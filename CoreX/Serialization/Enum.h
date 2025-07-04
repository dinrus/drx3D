// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Enum.h>
#include "NameGeneration.h"

///////////////////////////////////////////////////////////////////////
// Implement serialization for a seperately defined enum
#define SERIALIZATION_ENUM_BEGIN(Enum, label)                         YASLI_ENUM_BEGIN(Enum, label)
#define SERIALIZATION_ENUM_BEGIN_NESTED(Class, Enum, label)           YASLI_ENUM_BEGIN_NESTED(Class, Enum, label)
#define SERIALIZATION_ENUM_BEGIN_NESTED2(Class1, Class2, Enum, label) YASLI_ENUM_BEGIN_NESTED2(Class1, Class2, Enum, label)
#define SERIALIZATION_ENUM(value, name, label)                        YASLI_ENUM(value, name, label)
#define SERIALIZATION_ENUM_END()                                      YASLI_ENUM_END()

///////////////////////////////////////////////////////////////////////
// Define enum and serialization together.
// Enum has scoped names (enum class), and optional size.
// Each enum value has its serialization "name" equal to its code id,
// and its "label" automatically generated from the name,
// with automatic capitalization and spaces. Ex:
//		fixed      -> "Fixed"
//		blackHole  -> "Black Hole"
//		heat_death -> "Heat Death"

// Declare an enum and registration class, for use in header files. Ex:
//		SERIALIZATION_ENUM_DECLARE(EType,, one = 1, two, duece = two, three)  // assigned values are allowed
//		SERIALIZATION_ENUM_DECLARE(ESmallType, :u8, ...)  // size is optional
// To alias additional names to a value, and disable membership in display list (e.g. for backward compatibility), add an underscore:
//		SERIALIZATION_ENUM_DECLARE(EType,,
//			Mercury, Venus,
//			Earth, _Gaia = Earth,    // alias can come either before or after main name
//			Mars))

///////////////////////////////////////////////////////////////////////
// Implementation

namespace Serialization
{
using yasli::EnumDescription;
using yasli::getEnumDescription;

class EnumAliasDescription
{
public:
	EnumAliasDescription(yasli::EnumDescription& base)
		: base_(base) {}

	void add(i32 value, tukk name, tukk label = "")
	{
		base_.add(value, name, label);
		nameToValue_[name] = value;
	}

	void addAlias(i32 value, tukk alias)
	{
		nameToValue_[alias] = value;
	}

	bool serializeInt(yasli::Archive& ar, i32& value, tukk name, tukk label) const
	{
		if (ar.isInput() && !ar.isInPlace() && !ar.isEdit())
		{
			string str;
			if (ar(str, name, label))
			{
				auto it = nameToValue_.find(str);
				if (it != nameToValue_.end())
				{
					value = it->second;
					return true;
				}
			}
		}
		return base_.serialize(ar, value, name, label);
	}

	template<typename Enum>
	bool serialize(yasli::Archive& ar, Enum& value, tukk name, tukk label)
	{
		i32 i = check_cast<i32>(value);
		if (!serializeInt(ar, i, name, label))
			return false;
		if (ar.isInput())
			value = check_cast<Enum>(i);
		return true;
	}

private:

	struct LessStrCmpi : std::binary_function<tukk , tukk , bool>
	{
		bool operator()(tukk l, tukk r) const { return strcmpi(l, r) < 0; }
	};
	typedef std::map<cstr, i32, LessStrCmpi> NameToValue;

	yasli::EnumDescription& base_;
	NameToValue             nameToValue_;
};

// Helper object for tracking enum names and values
struct EnumInit
{
	EnumInit(EnumAliasDescription& desc, tuk names)
	{
		context().Init(desc, names);
	}
	EnumInit()
	{
		AddElem(context().nextValue);
	}
	EnumInit(EnumInit const& in)
	{
		AddElem(in.value);
	}
	EnumInit(i32 v)
	{
		AddElem(v);
	}

private:

	i32    value;
	string label;

	void   AddElem(i32 val)
	{
		value = val;
		cstr name = context().ParseNextEnum();
		if (*name == '_')
		{
			// Non-displayed alias, for compatibility
			name++;
			context().enumDesc->addAlias(value, name);
		}
		else
		{
			string labelStr = NameToLabel(name);
			if (labelStr == name)
			{
				context().enumDesc->add(value, name, name);
			}
			else
			{
				label = labelStr;
				context().enumDesc->add(value, name, label);
			}
		}
		context().nextValue = value + 1;
	}

	struct Context
	{
		EnumAliasDescription* enumDesc;
		tuk                 enumStr;
		i32                   nextValue;

		void                  Init(EnumAliasDescription& desc, tuk names)
		{
			enumDesc = &desc;
			enumStr = names;
			nextValue = 0;
		}

		cstr ParseNextEnum()
		{
			tuk s = enumStr;
			while (isspace(*s)) s++;
			if (!*s)
				return 0;

			cstr se = s;
			while (isalnum(*s) || *s == '_') s++;

			if (*s == ',')
			{
				* s++ = 0;
			}
			else if (*s)
			{
				* s++ = 0;
				while (*s && *s != ',') s++;
				if (*s) s++;
			}
			enumStr = s;
			return se;
		}
	};

	static Context& context()
	{
		static Context s;
		return s;
	}
};

template<typename Enum>
cstr getEnumName(Enum val)
{
	return getEnumDescription<Enum>().name((i32)val);
}
template<typename Enum>
cstr getEnumLabel(Enum val)
{
	return getEnumDescription<Enum>().label((i32)val);
}

}

///////////////////////////////////////////////////////////////////////
// Declare enums and serialization together, with scoped names.
// EnumAliasDescription createad on first call to Serialize.
// Serialization will only work if called from the same namespace.

#define SERIALIZATION_ENUM_IMPLEMENT(Enum, ...)                                                   \
  inline Serialization::EnumAliasDescription& makeEnumDescription(Enum*) {                        \
    static Serialization::EnumAliasDescription desc(yasli::getEnumDescription<Enum>());           \
    static char enum_str[] = # __VA_ARGS__;                                                       \
    static Serialization::EnumInit init(desc, enum_str), __VA_ARGS__;                             \
    return desc;                                                                                  \
  }                                                                                               \
  inline bool Serialize(yasli::Archive & ar, Enum & value, tukk name, tukk label) { \
    return makeEnumDescription(&value).serialize(ar, value, name, label);                         \
  }                                                                                               \

#define SERIALIZATION_ENUM_DECLARE(Enum, Base, ...)                                               \
  enum class Enum Base { __VA_ARGS__ };                                                           \
  SERIALIZATION_ENUM_IMPLEMENT(Enum, __VA_ARGS__)                                                 \

// Legacy macros
#define SERIALIZATION_ENUM_DEFINE               SERIALIZATION_ENUM_DECLARE
#define SERIALIZATION_DECLARE_ENUM(Enum, ...)   SERIALIZATION_ENUM_DECLARE(Enum, , __VA_ARGS__)
