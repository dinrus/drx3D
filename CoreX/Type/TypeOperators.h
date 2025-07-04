// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "TypeTraits.h"

namespace Drx {
namespace Type {

// Constructor: Default
class CDefaultConstructor
{
public:
	typedef void (* FuncPtr)(uk pPlacement);
	constexpr CDefaultConstructor(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	void operator()(uk pPlacement) { m_ptr(pPlacement); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = std::is_default_constructible<TYPE>::value>
class CAdapater_DefaultConstructor : public CDefaultConstructor
{

};

template<typename TYPE>
class CAdapater_DefaultConstructor<TYPE, true> : public CDefaultConstructor
{
public:
	constexpr CAdapater_DefaultConstructor()
		: CDefaultConstructor(&CAdapater_DefaultConstructor::Execute)
	{}

private:
	static void Execute(uk pPlacement)
	{
		new(pPlacement) TYPE();
	}
};

// Constructor: Copy
class CCopyConstructor
{
public:
	typedef void (* FuncPtr)(uk pPlacement, ukk pRHS);
	constexpr CCopyConstructor(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	void operator()(uk pPlacement, ukk pRHS) { m_ptr(pPlacement, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = std::is_copy_constructible<TYPE>::value>
class CAdapater_CopyConstructor : public CDefaultConstructor
{

};

template<typename TYPE>
class CAdapater_CopyConstructor<TYPE, true> : public CCopyConstructor
{
public:
	constexpr CAdapater_CopyConstructor()
		: CCopyConstructor(&CAdapater_CopyConstructor::Execute)
	{}

private:
	static void Execute(uk pPlacement, ukk pRHS)
	{
		const TYPE& value = *static_cast<const TYPE*>(pRHS);
		new(pPlacement) TYPE(value);
	}
};

// Constructor: Move
class CMoveConstructor
{
public:
	typedef void (* FuncPtr)(uk pPlacement, uk pRHS);
	constexpr CMoveConstructor(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	void operator()(uk pPlacement, uk pRHS) { m_ptr(pPlacement, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = std::is_move_constructible<TYPE>::value>
class CAdapter_MoveConstructor : public CMoveConstructor
{
};

template<typename TYPE>
class CAdapter_MoveConstructor<TYPE, true> : public CMoveConstructor
{
public:
	constexpr CAdapter_MoveConstructor()
		: CMoveConstructor(&CAdapter_MoveConstructor::Execute)
	{}

private:
	static void Execute(uk pPlacement, uk pRHS)
	{
		const TYPE& value = *static_cast<const TYPE*>(pRHS);
		new(pPlacement) TYPE(std::move(value));
	}
};

// Destructor
class CDestructor
{
public:
	typedef void (* FuncPtr)(uk pRHS);
	constexpr CDestructor(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	void operator()(uk pRHS) { m_ptr(pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = std::is_copy_constructible<TYPE>::value>
class CAdapter_Destructor : public CDestructor
{
};

template<typename TYPE>
class CAdapter_Destructor<TYPE, true> : public CDestructor
{
public:
	constexpr CAdapter_Destructor()
		: CDestructor(&CAdapter_Destructor::Execute)
	{}

private:
	static void Execute(uk pRHS)
	{
		TYPE* rhs = static_cast<TYPE*>(pRHS);
		rhs->~TYPE();
	}
};

// Operator: Copy assign
class CCopyAssignOperator
{
public:
	typedef void (* FuncPtr)(uk pLHS, ukk pRHS);
	constexpr CCopyAssignOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	void operator()(uk pLHS, ukk pRHS) { m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = std::is_copy_assignable<TYPE>::value>
class CAdapter_CopyAssign : public CCopyAssignOperator
{

};

template<typename TYPE>
class CAdapter_CopyAssign<TYPE, true> : public CCopyAssignOperator
{
public:
	constexpr CAdapter_CopyAssign()
		: CCopyAssignOperator(&CAdapter_CopyAssign::Execute)
	{}

private:
	static void Execute(uk pLHS, ukk pRHS)
	{
		TYPE& lhs = *static_cast<TYPE*>(pLHS);
		const TYPE& rhs = *static_cast<const TYPE*>(pRHS);
		lhs = rhs;
	}
};

// Operator: Equal
class CEqualOperator
{
public:
	typedef bool (* FuncPtr)(ukk pLHS, ukk pRHS);
	constexpr CEqualOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	bool operator()(ukk pLHS, ukk pRHS) { return m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = Traits::IsEqualComparable<TYPE>::value>
class CAdapter_Equal : public CEqualOperator
{

};

template<typename TYPE>
class CAdapter_Equal<TYPE, true> : public CEqualOperator
{
public:
	constexpr CAdapter_Equal()
		: CEqualOperator(&CAdapter_Equal::Execute)
	{}

private:
	static bool Execute(ukk pLHS, ukk pRHS)
	{
		const TYPE& lhs = *static_cast<const TYPE*>(pLHS);
		const TYPE& rhs = *static_cast<const TYPE*>(pRHS);
		return (lhs == rhs);
	}
};

// Operator: Greater or equal
class CGreaterOrEqualOperator
{
public:
	typedef bool (* FuncPtr)(ukk pLHS, ukk pRHS);
	constexpr CGreaterOrEqualOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	bool operator()(ukk pLHS, ukk pRHS) { return m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = Traits::IsGreaterOrEqualComparable<TYPE>::value>
class CAdapter_GreaterOrEqual : public CGreaterOrEqualOperator
{

};

template<typename TYPE>
class CAdapter_GreaterOrEqual<TYPE, true> : public CGreaterOrEqualOperator
{
public:
	constexpr CAdapter_GreaterOrEqual()
		: CGreaterOrEqualOperator(&CAdapter_GreaterOrEqual::Execute)
	{}

private:
	static bool Execute(ukk pLHS, ukk pRHS)
	{
		const TYPE& lhs = *static_cast<const TYPE*>(pLHS);
		const TYPE& rhs = *static_cast<const TYPE*>(pRHS);
		return (lhs >= rhs);
	}
};

// Operator: Less or equal
class CLessOrEqualOperator
{
public:
	typedef bool (* FuncPtr)(ukk pLHS, ukk pRHS);
	constexpr CLessOrEqualOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	bool operator()(ukk pLHS, ukk pRHS) { return m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = Traits::IsLessOrEqualComparable<TYPE>::value>
class CAdapter_LessOrEqual : public CLessOrEqualOperator
{

};

template<typename TYPE>
class CAdapter_LessOrEqual<TYPE, true> : public CLessOrEqualOperator
{
public:
	constexpr CAdapter_LessOrEqual()
		: CLessOrEqualOperator(&CAdapter_LessOrEqual::Execute)
	{}

private:
	static bool Execute(ukk pLHS, ukk pRHS)
	{
		const TYPE& lhs = *static_cast<const TYPE*>(pLHS);
		const TYPE& rhs = *static_cast<const TYPE*>(pRHS);
		return (lhs <= rhs);
	}
};

// Operator: Greater
class CGreaterOperator
{
public:
	typedef bool (* FuncPtr)(ukk pLHS, ukk pRHS);
	constexpr CGreaterOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	bool operator()(ukk pLHS, ukk pRHS) { return m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = Traits::IsGreaterComparable<TYPE>::value>
class CAdapter_Greater : public CGreaterOperator
{

};

template<typename TYPE>
class CAdapter_Greater<TYPE, true> : public CGreaterOperator
{
public:
	constexpr CAdapter_Greater()
		: CAdapter_Greater(& CAdapter_Greater::Execute)
	{}

private:
	static bool Execute(ukk pLHS, ukk pRHS)
	{
		const TYPE& lhs = *static_cast<const TYPE*>(pLHS);
		const TYPE& rhs = *static_cast<const TYPE*>(pRHS);
		return (lhs > rhs);
	}
};

// Operator: Less
class CLessOperator
{
public:
	typedef bool (* FuncPtr)(ukk pLHS, ukk pRHS);
	constexpr CLessOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	bool operator()(ukk pLHS, ukk pRHS) { return m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename TYPE, bool IS_SUPPORTED = Traits::IsLessComparable<TYPE>::value>
class CAdapter_Less : public CLessOperator
{

};

template<typename TYPE>
class CAdapter_Less<TYPE, true> : public CLessOperator
{
public:
	constexpr CAdapter_Less()
		: CLessOperator(&CAdapter_Less::Execute)
	{}

private:
	static bool Execute(ukk pLHS, ukk pRHS)
	{
		const TYPE& lhs = *static_cast<const TYPE*>(pLHS);
		const TYPE& rhs = *static_cast<const TYPE*>(pRHS);
		return (lhs <= rhs);
	}
};

// TODO: Result?
// Operator: Conversion
class CConversionOperator
{
public:
	typedef void (* FuncPtr)(uk pLHS, ukk pRHS);
	constexpr CConversionOperator(FuncPtr ptr = nullptr)
		: m_ptr(ptr)
	{}

	operator bool() const { return m_ptr != nullptr; }
	void operator()(uk pLHS, ukk pRHS) { m_ptr(pLHS, pRHS); }

private:
	FuncPtr m_ptr;
};

template<typename LHS_TYPE, typename RHS_TYPE, bool IS_SUPPORTED = Traits::IsConvertible<LHS_TYPE, RHS_TYPE>::value>
class CAdapter_Conversion : public CConversionOperator
{
};

template<typename LHS_TYPE, typename RHS_TYPE>
class CAdapter_Conversion<LHS_TYPE, RHS_TYPE, true> : public CConversionOperator
{
public:
	CAdapter_Conversion()
		: CConversionOperator(&CAdapter_Conversion::Execute)
	{}

private:
	static void Execute(uk pLHS, ukk pRHS)
	{
		LHS_TYPE& lhs = *static_cast<LHS_TYPE*>(pLHS);
		const RHS_TYPE& rhs = *static_cast<const RHS_TYPE*>(pRHS);
		lhs = rhs;
	}
};
// ~TODO

} // ~Drx namespace
} // ~Reflection namespace
