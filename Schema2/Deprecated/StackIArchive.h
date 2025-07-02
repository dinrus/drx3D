// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Deprecated/Stack.h>

namespace sxema2
{
	template <typename TYPE> inline void Pop(CStack& stack, TYPE& value);

	inline void Pop(CStack& stack, bool& value)
	{
		value = stack.Top().AsBool();
		stack.Pop();
	}

	inline void Pop(CStack& stack, int8& value)
	{
		value = stack.Top().AsInt8();
		stack.Pop();
	}

	inline void Pop(CStack& stack, u8& value)
	{
		value = stack.Top().AsUInt8();
		stack.Pop();
	}

	inline void Pop(CStack& stack, i16& value)
	{
		value = stack.Top().AsInt16();
		stack.Pop();
	}

	inline void Pop(CStack& stack, u16& value)
	{
		value = stack.Top().AsUInt16();
		stack.Pop();
	}

	inline void Pop(CStack& stack, i32& value)
	{
		value = stack.Top().AsInt32();
		stack.Pop();
	}

	inline void Pop(CStack& stack, u32& value)
	{
		value = stack.Top().AsUInt32();
		stack.Pop();
	}

	inline void Pop(CStack& stack, int64& value)
	{
		value = stack.Top().AsInt64();
		stack.Pop();
	}

	inline void Pop(CStack& stack, uint64& value)
	{
		value = stack.Top().AsUInt64();
		stack.Pop();
	}

	inline void Pop(CStack& stack, float& value)
	{
		value = stack.Top().AsFloat();
		stack.Pop();
	}

	inline void Pop(CStack& stack, CPoolString& value)
	{
		value = stack.Top().c_str();
		stack.Pop();
	}

	template <class TYPE> inline void Pop(CStack& stack, DrxStringT<TYPE>& value)
	{
		value = stack.Top().c_str();
		stack.Pop();
	}

	template <class TYPE, size_t SIZE> inline void Pop(CStack& stack, DrxStackStringT<TYPE, SIZE>& value)
	{
		value = stack.Top().c_str();
		stack.Pop();
	}

	class CStackIArchive : public Serialization::IArchive
	{
	public:

		inline CStackIArchive(CStack& stack)
			: Serialization::IArchive(Serialization::IArchive::INPUT | Serialization::IArchive::INPLACE)
			, m_stack(stack)
		{}

		// Serialization::IArchive

		virtual bool operator () (bool& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (int8& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (u8& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (i32& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (u32& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (int64& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (uint64& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (float& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (Serialization::IString& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Pop(m_stack, value);
			return true;
		}

		virtual bool operator () (const Serialization::SStruct& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			value(*this);
			return true;
		}

		virtual bool operator () (Serialization::IContainer& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			if(value.isFixedSize())
			{
				if(value.size())
				{
					do
					{
						value(*this, szName, szLabel);
					} while(value.next());
				}
				return true;
			}
			else
			{
				DrxFatalError("Serialization of dynamic containers is not yet supported!");
				return false;
			}
		}

		using Serialization::IArchive::operator ();

		// ~Serialization::IArchive

		inline void operator () (tukk & value)
		{
			Pop(m_stack, value);
		}

	private:

		CStack& m_stack;
	};

	template <typename TYPE> inline void Pop(CStack& stack, TYPE& value)
	{
		CStackIArchive archive(stack);
		archive(value);
	}
}
