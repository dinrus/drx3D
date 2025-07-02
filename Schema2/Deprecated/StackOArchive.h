// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Deprecated/Stack.h>

namespace sxema2
{
	template <typename TYPE> inline void Push(CStack& stack, const TYPE& value);

	inline void Push(CStack& stack, const bool& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, const int8& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, u8k& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, i16k& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, u16k& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, i32k& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, u32k& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, const int64& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, const uint64& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, const float& value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, tukk value)
	{
		stack.Push(CVariant(value));
	}

	inline void Push(CStack& stack, const CPoolString& value)
	{
		stack.Push(CVariant(value.c_str()));
	}

	template <class TYPE> inline void Push(CStack& stack, const DrxStringT<TYPE>& value)
	{
		stack.Push(CVariant(value.c_str()));
	}

	template <class TYPE, size_t SIZE> inline void Push(CStack& stack, const DrxStackStringT<TYPE, SIZE>& value)
	{
		stack.Push(CVariant(value.c_str()));
	}

	class CStackOArchive : public Serialization::IArchive
	{
	public:

		inline CStackOArchive(CStack& stack)
			: Serialization::IArchive(Serialization::IArchive::OUTPUT | Serialization::IArchive::INPLACE)
			, m_stack(stack)
		{}

		// Serialization::IArchive

		virtual bool operator () (bool& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (int8& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (u8& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (i32& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (u32& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (int64& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (uint64& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (float& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value);
			return true;
		}

		virtual bool operator () (Serialization::IString& value, tukk szName = "", tukk szLabel = nullptr) override
		{
			Push(m_stack, value.get());
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
			Push(m_stack, value);
		}

	private:

		CStack& m_stack;
	};

	template <typename TYPE> inline void Push(CStack& stack, const TYPE& value)
	{
		CStackOArchive archive(stack);
		archive(const_cast<TYPE&>(value));
	}
}
