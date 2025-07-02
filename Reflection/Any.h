// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Reflection/Serialization/Forward.h>

//#include <drx3D/Schema/ReflectionUtils.h>

#include <drx3D/Reflection/RegisteredType.h>

namespace Drx {
namespace Reflection {

// Forward declare classes.
class CAnyValue;
class CAnyRef;
class CAnyConstRef;
class CAnyPtr;
class CAnyConstPtr;

// Specialized allocator for CAnyValue, allocates additional memory for value storage immediately after object.
template<typename T>
struct SAnyValueAllocator
{
	typedef T        value_type;
	typedef T*       pointer;
	typedef const T& const_pointer;
	typedef T&       reference;
	typedef const T& const_reference;
	typedef size_t   size_type;

	template<class U> struct rebind
	{
		typedef SAnyValueAllocator<U> other;
	};

	SAnyValueAllocator(const CCommonTypeDesc& _typeDesc)
		: typeDesc(_typeDesc)
	{}

	template<typename OTHER_TYPE>
	SAnyValueAllocator(const SAnyValueAllocator<OTHER_TYPE>& rhs)
		: typeDesc(rhs.typeDesc)
	{}

	pointer address(reference x)
	{
		return &x;
	}

	const_pointer address(const_reference x) const
	{
		return &x;
	}

	pointer allocate(size_type n)
	{
		return static_cast<pointer>(DrxModuleMalloc((sizeof(value_type) + typeDesc.GetSize()) * n));
	}

	void deallocate(pointer p, size_type n)
	{
		DrxModuleFree(p);
	}

	size_type max_size() const
	{
		return INT_MAX;
	}

	template<class U>
	void destroy(U* p) noexcept
	{
		p->~U();
	}

	const CCommonTypeDesc& typeDesc;
};

// Owning container for value of any reflected type. It is assumed that
// value follows immediately after object in memory therefore instances
// must be allocated using Make functions provided.
class CAnyValue
{
public:
	~CAnyValue()
	{
		m_typeDesc.GetOperators().destruct(GetValue());
	}

	const CReflectedTypeDesc& GetTypeDesc() const
	{
		return m_typeDesc;
	}

	uk GetValue()
	{
		return this + 1;
	}

	ukk GetValue() const
	{
		return this + 1;
	}

	template<typename TYPE>
	CAnyValuePtr MakeShared(const TYPE& value)
	{
		SXEMA_VERIFY_TYPE_IS_REFLECTED(TYPE);
		static_assert(std::is_copy_constructible<TYPE>::value, "Type must provide copy constructor!");
		return MakeShared(sxema::GetTypeDesc<TYPE>(), &value);
	}

	CAnyValuePtr MakeShared(const CCommonTypeDesc& typeDesc, ukk pValue)
	{
		class CAnyValueImpl : public CAnyValue
		{
		public:

			inline CAnyValueImpl(const CCommonTypeDesc& typeDesc, ukk pValue)
				: CAnyValue(typeDesc, pValue)
			{}
		};

		DRX_ASSERT_MESSAGE(typeDesc.GetOperators().copyConstruct, "Type must provide copy constructor!");
		if (typeDesc.GetOperators().copyConstruct)
		{
			DRX_ASSERT(pValue);
			if (pValue)
			{
				SAnyValueAllocator<CAnyValueImpl> allocator(typeDesc);
				return std::static_pointer_cast<CAnyValue>(std::allocate_shared<CAnyValueImpl, SAnyValueAllocator<CAnyValueImpl>>(allocator, typeDesc, pValue));
			}
		}
		return CAnyValuePtr();
	}

	CAnyValuePtr MakeSharedDefault(const CCommonTypeDesc& typeDesc)
	{
		class CAnyValueImpl : public CAnyValue
		{
		public:

			inline CAnyValueImpl(const CCommonTypeDesc& typeDesc)
				: CAnyValue(typeDesc)
			{}
		};

		DRX_ASSERT_MESSAGE(typeDesc.GetOperators().defaultConstruct, "Type must provide default constructor!");
		if (typeDesc.GetOperators().defaultConstruct)
		{
			SAnyValueAllocator<CAnyValueImpl> allocator(typeDesc);
			return std::static_pointer_cast<CAnyValue>(std::allocate_shared<CAnyValueImpl, SAnyValueAllocator<CAnyValueImpl>>(allocator, typeDesc));
		}
		return CAnyValuePtr();
	}

	CAnyValuePtr CloneShared(const CAnyConstRef& value)
	{
		return MakeShared(value.GetTypeDesc(), value.GetValue());
	}

protected:
	CAnyValue(const CCommonTypeDesc& typeDesc)
		: m_typeDesc(typeDesc)
	{
		m_typeDesc.GetOperators().defaultConstruct(GetValue());
	}

	CAnyValue(const CCommonTypeDesc& typeDesc, ukk pValue)
		: m_typeDesc(typeDesc)
	{
		m_typeDesc.GetOperators().copyConstruct(GetValue(), pValue);
	}

	CAnyValue() = delete;
	CAnyValue(const CAnyValue& rhs) = delete;
	CAnyValue& operator=(const CAnyValue& rhs) = delete;

private:
	const CCommonTypeDesc& m_typeDesc;
};

// Reference to value of any reflected type.
class CAnyRef
{
public:
	CAnyRef(const CCommonTypeDesc& typeDesc, uk pValue)
		: m_pTypeDesc(&typeDesc)
		, m_pValue(pValue)
	{
		DRX_ASSERT(IsValid());
	}

	template<typename TYPE>
	CAnyRef(TYPE& value)
		: m_pTypeDesc(&sxema::GetTypeDesc<TYPE>())
		, m_pValue(&value)
	{}

	CAnyRef(CAnyValue& rhs)
		: m_pTypeDesc(&rhs.GetTypeDesc())
		, m_pValue(rhs.GetValue())
	{}

	CAnyRef(const CAnyRef& rhs)
		: m_pTypeDesc(rhs.m_pTypeDesc)
		, m_pValue(rhs.m_pValue)
	{}

	bool IsValid() const
	{
		return m_pValue != nullptr;
	}

	const CCommonTypeDesc& GetTypeDesc() const
	{
		DRX_ASSERT(IsValid());
		return *m_pTypeDesc;
	}

	uk GetValue() const
	{
		DRX_ASSERT(IsValid());
		return m_pValue;
	}

protected:
	friend class CAnyPtr;

	CAnyRef()
		: m_pTypeDesc(nullptr)
		, m_pValue(nullptr)
	{}

	void Reset(const CAnyRef& rhs)
	{
		m_pTypeDesc = rhs.m_pTypeDesc;
		m_pValue = rhs.m_pValue;
	}

private:
	CAnyRef& operator=(const CAnyRef& rhs) = delete;

private:
	const CRegisteredType* m_pTypeDesc;
	uk                  m_pValue;
};

// Reference to constant value of any reflected type.
class CAnyConstRef
{
public:
	CAnyConstRef(const CCommonTypeDesc& typeDesc, ukk pValue)
		: m_pTypeDesc(&typeDesc)
		, m_pValue(pValue)
	{
		DRX_ASSERT(IsValid());
	}

	template<typename TYPE>
	CAnyConstRef(const TYPE& value)
		: m_pTypeDesc(&sxema::GetTypeDesc<TYPE>())
		, m_pValue(&value)
	{}

	CAnyConstRef(const CAnyValue& rhs)
		: m_pTypeDesc(&rhs.GetTypeDesc())
		, m_pValue(rhs.GetValue())
	{}

	CAnyConstRef(const CAnyRef& rhs)
		: m_pTypeDesc(&rhs.GetTypeDesc())
		, m_pValue(rhs.GetValue())
	{}

	CAnyConstRef(const CAnyConstRef& rhs)
		: m_pTypeDesc(rhs.m_pTypeDesc)
		, m_pValue(rhs.m_pValue)
	{}

	bool IsValid() const
	{
		return m_pValue != nullptr;
	}

	const CCommonTypeDesc& GetTypeDesc() const
	{
		DRX_ASSERT(IsValid());
		return *m_pTypeDesc;
	}

	ukk GetValue() const
	{
		return m_pValue;
	}

protected:
	friend class CAnyConstPtr;

	CAnyConstRef()
		: m_pTypeDesc(nullptr)
		, m_pValue(nullptr)
	{}

	void Reset(const CAnyConstRef& rhs)
	{
		m_pTypeDesc = rhs.m_pTypeDesc;
		m_pValue = rhs.m_pValue;
	}

private:
	CAnyConstRef& operator=(const CAnyConstRef& rhs) = delete;

private:
	const CCommonTypeDesc* m_pTypeDesc;
	ukk            m_pValue;
};

// Pointer to value of any reflected type.
class CAnyPtr
{
public:
	CAnyPtr()
	{}

	CAnyPtr(const CCommonTypeDesc& typeDesc, uk pValue)
		: m_ref(typeDesc, pValue)
	{}

	template<typename TYPE>
	CAnyPtr(TYPE* pValue)
		: m_ref(sxema::GetTypeDesc<TYPE>(), pValue)
	{}

	CAnyPtr(CAnyValue* pValue)
		: m_ref(pValue ? CAnyRef(*pValue) : CAnyRef())
	{}

	CAnyPtr(const CAnyValuePtr& pValue)
		: m_ref(pValue ? CAnyRef(*pValue) : CAnyRef())
	{}

	CAnyPtr(const CAnyRef* rhs)
		: m_ref(rhs ? *rhs : CAnyRef())
	{}

	CAnyPtr(const CAnyPtr& rhs)
		: m_ref(rhs.m_ref)
	{}

	operator bool() const
	{
		return m_ref.IsValid();
	}

	const CAnyRef* operator->() const
	{
		DRX_ASSERT(m_ref.IsValid());
		return &m_ref;
	}

	const CAnyRef& operator*() const
	{
		DRX_ASSERT(m_ref.IsValid());
		return m_ref;
	}

	CAnyPtr& operator=(const CAnyPtr& rhs)
	{
		m_ref.Reset(rhs.m_ref);
		return *this;
	}

private:
	CAnyRef m_ref;
};

// Pointer to constant value of any reflected type.
class CAnyConstPtr
{
public:
	CAnyConstPtr()
	{}

	CAnyConstPtr(const CCommonTypeDesc& typeDesc, ukk pValue)
		: m_ref(typeDesc, pValue)
	{}

	template<typename TYPE>
	CAnyConstPtr(const TYPE* pValue)
		: m_ref(sxema::GetTypeDesc<TYPE>(), pValue)
	{}

	CAnyConstPtr(const CAnyValue* rhs)
		: m_ref(rhs ? CAnyConstRef(*rhs) : CAnyConstRef())
	{}

	CAnyConstPtr(const CAnyConstRef* pValue)
		: m_ref(pValue ? *pValue : CAnyConstRef())
	{}

	CAnyConstPtr(const CAnyValuePtr& pValue)
		: m_ref(pValue ? *pValue : CAnyConstRef())
	{}

	CAnyConstPtr(const CAnyValueConstPtr& pValue)
		: m_ref(pValue ? *pValue : CAnyConstRef())
	{}

	CAnyConstPtr(const CAnyPtr& rhs)
		: m_ref(*rhs)
	{}

	CAnyConstPtr(const CAnyConstPtr& rhs)
		: m_ref(rhs.m_ref)
	{}

	operator bool() const
	{
		return m_ref.IsValid();
	}

	const CAnyConstRef* operator->() const
	{
		DRX_ASSERT(m_ref.IsValid());
		return &m_ref;
	}

	const CAnyConstRef& operator*() const
	{
		DRX_ASSERT(m_ref.IsValid());
		return m_ref;
	}

	CAnyConstPtr& operator=(const CAnyConstPtr& rhs)
	{
		m_ref.Reset(rhs.m_ref);
		return *this;
	}

private:
	CAnyConstRef m_ref;
};

// Utility functions.
namespace Any {

inline bool CopyAssign(const CAnyRef& lhs, const CAnyConstRef& rhs)
{
	const CCommonTypeDesc& typeDesc = lhs.GetTypeDesc();
	DRX_ASSERT_MESSAGE(typeDesc.GetOperators().copyAssign, "Type must provide copy assignment operator!");
	if (typeDesc.GetOperators().copyAssign)
	{
		ukk pRHSValue = Helpers::DynamicCast(rhs.GetTypeDesc(), rhs.GetValue(), typeDesc);
		DRX_ASSERT_MESSAGE(pRHSValue, "Type mismatch!");
		if (pRHSValue)
		{
			(*typeDesc.GetOperators().copyAssign)(lhs.GetValue(), pRHSValue);
			return true;
		}
	}
	return false;
}

inline bool Equals(const CAnyConstRef& lhs, const CAnyConstRef& rhs)
{
	const CCommonTypeDesc& typeDesc = lhs.GetTypeDesc();
	DRX_ASSERT_MESSAGE(typeDesc.GetOperators().equals, "Type must provide equals operator!");
	if (typeDesc.GetOperators().equals)
	{
		ukk pRHSValue = Helpers::DynamicCast(rhs.GetTypeDesc(), rhs.GetValue(), typeDesc);
		DRX_ASSERT_MESSAGE(pRHSValue, "Type mismatch!");
		if (pRHSValue)
		{
			return (*typeDesc.GetOperators().equals)(lhs.GetValue(), pRHSValue);
		}
	}
	return false;
}

inline bool ToString(IString& output, const CAnyConstRef& input)
{
	const CCommonTypeDesc& typeDesc = input.GetTypeDesc();
	DRX_ASSERT_MESSAGE(typeDesc.GetOperators().toString, "Type must provide 'ToString' method!");
	if (typeDesc.GetOperators().toString)
	{
		(*typeDesc.GetOperators().toString)(output, input.GetValue());
		return true;
	}
	return false;
}

}   // ~Any namespace

// Serialization functions.
inline bool Serialize(Serialization::IArchive& archive, const CAnyRef& value, tukk szName, tukk szLabel)
{
	const CCommonTypeDesc& typeDesc = value.GetTypeDesc();
	DRX_ASSERT_MESSAGE(typeDesc.GetOperators().toString, "Type must provide 'Serialize' method!");
	if (typeDesc.GetOperators().serialize)
	{
		return (*typeDesc.GetOperators().serialize)(archive, value.GetValue(), szName, szLabel);
	}
	return false;
}

inline bool Serialize(Serialization::IArchive& archive, CAnyValue& value, tukk szName, tukk szLabel)
{
	const CCommonTypeDesc& typeDesc = value.GetTypeDesc();
	DRX_ASSERT_MESSAGE(typeDesc.GetOperators().serialize, "Type must provide 'Serialize' method!");
	if (typeDesc.GetOperators().serialize)
	{
		return (*typeDesc.GetOperators().serialize)(archive, value.GetValue(), szName, szLabel);
	}
	return false;
}

// Dynamic cast functions.
template<typename TO_TYPE>
TO_TYPE& DynamicCast(const CAnyRef& from)
{
	TO_TYPE* pResult = Helpers::DynamicCast<TO_TYPE>(from.GetTypeDesc(), from.GetValue());
	DRX_ASSERT_MESSAGE(pResult);
	return *pResult;
}

template<typename TO_TYPE>
const TO_TYPE& DynamicCast(const CAnyConstRef& from)
{
	const TO_TYPE* pResult = Helpers::DynamicCast<TO_TYPE>(from.GetTypeDesc(), from.GetValue());
	DRX_ASSERT_MESSAGE(pResult);
	return *pResult;
}

template<typename TO_TYPE>
TO_TYPE* DynamicCast(const CAnyPtr& pFrom)
{
	return pFrom ? Helpers::DynamicCast<TO_TYPE>(pFrom->GetTypeDesc(), pFrom->GetValue()) : nullptr;
}

template<typename TO_TYPE>
const TO_TYPE* DynamicCast(const CAnyConstPtr& pFrom)
{
	return pFrom ? Helpers::DynamicCast<TO_TYPE>(pFrom->GetTypeDesc(), pFrom->GetValue()) : nullptr;
}

} // ~Reflection namespace
} // ~Drx namespace
