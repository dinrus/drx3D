// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/DrxVariant.h>
#include <drx3D/CoreX/Extension/IDrxUnknown.h>
#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/Sys/IFlashPlayer.h>
#include <drx3D/FlowGraph/IFlowSystem.h>
#include <drx3D/CoreX/String/DrxName.h>
#include <drx3D/CoreX/functor.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// UI variant data /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//! \cond INTERNAL
typedef DrxVariant<
	i32,
	float,
	EntityId,
	Vec3,
	string,
	wstring,
	bool
	> TUIDataVariant;

//! Default conversion uses C++ rules.
template<class From, class To>
struct SUIConversion
{
	static ILINE bool ConvertValue(const From& from, To& to)
	{
		to = static_cast<To>(from);
		return true;
	}
};
namespace drx_variant
{
	template<class To, size_t I = 0>
	ILINE bool ConvertVariant(const TUIDataVariant& from, To& to)
	{
		if (from.index() == I)
		{
			return SUIConversion<typename stl::variant_alternative<I, TUIDataVariant>::type, To>::ConvertValue(stl::get<I>(from), to);
		}
		else
		{
			return ConvertVariant<To, I + 1>(from, to);
		}
	}

	template<size_t I = 0>
	ILINE bool ConvertVariant(const TUIDataVariant& from, Vec3& to)
	{
		if (stl::holds_alternative<Vec3>(from))
		{
			to = stl::get<Vec3>(from);
		}
		else
		{
			float temp;
			if (from.index() == I)
			{
				if (!SUIConversion<typename stl::variant_alternative<I, TUIDataVariant>::type, float>::ConvertValue(stl::get<I>(from), temp))
					return false;
			}
			else
			{
				return ConvertVariant<float, I + 1>(from, temp);
			}
			to.x = to.y = to.z = temp;
		}
		return true;
	}

#define FLASHUI_CONVERTVARIANT_SPECIALIZATION(T) \
template<> \
ILINE bool ConvertVariant<T, stl::variant_size<TUIDataVariant>::value>(const TUIDataVariant&, T&) \
{ \
	DRX_ASSERT_MESSAGE(false, "Invalid variant index."); \
	return false; \
}
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(i32);
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(float);
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(EntityId);
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(Vec3);
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(string);
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(wstring);
	FLASHUI_CONVERTVARIANT_SPECIALIZATION(bool);
#undef FLOWSYSTEM_CONVERTVARIANT_SPECIALIZATION
}
template<class To>
struct SUIConversion<TUIDataVariant, To>
{
	static ILINE bool ConvertValue(const TUIDataVariant& from, To& to)
	{
		return drx_variant::ConvertVariant(from, to);
	}
};
template<>
struct SUIConversion<TUIDataVariant, bool>
{
	static ILINE bool ConvertValue(const TUIDataVariant& from, bool& to)
	{
		return drx_variant::ConvertVariant(from, to);
	}
};
template<>
struct SUIConversion<TUIDataVariant, Vec3>
{
	static ILINE bool ConvertValue(const TUIDataVariant& from, Vec3& to)
	{
		return drx_variant::ConvertVariant(from, to);
	}
};

//! Same type conversation.
#define UIDATA_NO_CONVERSION(T)                                                       \
  template<> struct SUIConversion<T, T> {                                             \
    static ILINE bool ConvertValue(const T &from, T & to) { to = from; return true; } \
  }
UIDATA_NO_CONVERSION(i32);
UIDATA_NO_CONVERSION(float);
UIDATA_NO_CONVERSION(EntityId);
UIDATA_NO_CONVERSION(Vec3);
UIDATA_NO_CONVERSION(string);
UIDATA_NO_CONVERSION(wstring);
UIDATA_NO_CONVERSION(bool);
#undef FLOWSYSTEM_NO_CONVERSION

//! Specialization for converting to bool to avoid compiler warnings.
template<class From>
struct SUIConversion<From, bool>
{
	static ILINE bool ConvertValue(const From& from, bool& to)
	{
		to = (from != 0);
		return true;
	}
};

//! Strict conversation from float to i32.
template<>
struct SUIConversion<float, i32>
{
	static ILINE bool ConvertValue(const float& from, i32& to)
	{
		i32 tmp = (i32) from;
		if (fabs(from - (float) tmp) < FLT_EPSILON)
		{
			to = tmp;
			return true;
		}
		return false;
	}
};

//! Vec3 conversions.
template<class To>
struct SUIConversion<Vec3, To>
{
	static ILINE bool ConvertValue(const Vec3& from, To& to)
	{
		return SUIConversion<float, To>::ConvertValue(from.x, to);
	}
};

template<class From>
struct SUIConversion<From, Vec3>
{
	static ILINE bool ConvertValue(const From& from, Vec3& to)
	{
		float temp;
		if (!SUIConversion<From, float>::ConvertValue(from, temp))
			return false;
		to.x = to.y = to.z = temp;
		return true;
	}
};

template<>
struct SUIConversion<Vec3, bool>
{
	static ILINE bool ConvertValue(const Vec3& from, bool& to)
	{
		to = from.GetLengthSquared() > 0;
		return true;
	}
};

//! String conversions.
#define UIDATA_STRING_CONVERSION(strtype, type, fmt, fct)                      \
  template<>                                                                   \
  struct SUIConversion<type, DrxStringT<strtype>>                              \
  {                                                                            \
    static ILINE bool ConvertValue(const type &from, DrxStringT<strtype> &to)  \
    {                                                                          \
      DrxStringT<char> tmp;                                                    \
      tmp.Format(fmt, from);                                                   \
      Unicode::Convert(to, tmp);                                               \
      return true;                                                             \
    }                                                                          \
  };                                                                           \
  template<>                                                                   \
  struct SUIConversion<DrxStringT<strtype>, type>                              \
  {                                                                            \
    static ILINE bool ConvertValue(const DrxStringT<strtype> &from, type & to) \
    {                                                                          \
      strtype* pEnd;                                                           \
      to = fct;                                                                \
      return from.size() > 0 && *pEnd == '\0';                                 \
    }                                                                          \
  };

#define SINGLE_FCT(fct) (float) fct(from.c_str(), &pEnd)
#define DOUBLE_FCT(fct) fct(from.c_str(), &pEnd, 10)

UIDATA_STRING_CONVERSION(char, i32, "%d", DOUBLE_FCT(strtol));
UIDATA_STRING_CONVERSION(wchar_t, i32, "%d", DOUBLE_FCT(wcstol));

UIDATA_STRING_CONVERSION(char, float, "%f", SINGLE_FCT(strtod));
UIDATA_STRING_CONVERSION(wchar_t, float, "%f", SINGLE_FCT(wcstod));

UIDATA_STRING_CONVERSION(char, EntityId, "%u", DOUBLE_FCT(strtoul));
UIDATA_STRING_CONVERSION(wchar_t, EntityId, "%u", DOUBLE_FCT(wcstoul));

#undef UIDATA_STRING_CONVERSION
#undef SINGLE_FCT
#undef DOUBLE_FCT

template<>
struct SUIConversion<bool, string>
{
	static ILINE bool ConvertValue(const bool& from, string& to)
	{
		to.Format("%d", from);
		return true;
	}
};

template<>
struct SUIConversion<string, bool>
{
	static ILINE bool ConvertValue(const string& from, bool& to)
	{
		float to_i;
		if (SUIConversion<string, float>::ConvertValue(from, to_i))
		{
			to = to_i != 0;
			return true;
		}
		if (0 == stricmp(from.c_str(), "true"))
		{
			to = true;
			return true;
		}
		if (0 == stricmp(from.c_str(), "false"))
		{
			to = false;
			return true;
		}
		return false;
	}
};

template<>
struct SUIConversion<Vec3, string>
{
	static ILINE bool ConvertValue(const Vec3& from, string& to)
	{
		to.Format("%f,%f,%f", from.x, from.y, from.z);
		return true;
	}
};

template<>
struct SUIConversion<string, Vec3>
{
	static ILINE bool ConvertValue(const string& from, Vec3& to)
	{
		return 3 == sscanf(from.c_str(), "%f,%f,%f", &to.x, &to.y, &to.z);
	}
};

template<>
struct SUIConversion<bool, wstring>
{
	static ILINE bool ConvertValue(const bool& from, wstring& to)
	{
		string tmp;
		tmp.Format("%d", from);
		Unicode::Convert(to, tmp);
		return true;
	}
};

template<>
struct SUIConversion<wstring, bool>
{
	static ILINE bool ConvertValue(const wstring& from, bool& to)
	{
		i32 to_i;
		if (SUIConversion<wstring, i32>::ConvertValue(from, to_i))
		{
			to = !!to_i;
			return true;
		}
		if (0 == wcsicmp(from.c_str(), L"true"))
		{
			to = true;
			return true;
		}
		if (0 == wcsicmp(from.c_str(), L"false"))
		{
			to = false;
			return true;
		}
		return false;
	}
};

template<>
struct SUIConversion<Vec3, wstring>
{
	static ILINE bool ConvertValue(const Vec3& from, wstring& to)
	{
		string tmp;
		tmp.Format("%f,%f,%f", from.x, from.y, from.z);
		Unicode::Convert(to, tmp);
		return true;
	}
};

template<>
struct SUIConversion<wstring, Vec3>
{
	static ILINE bool ConvertValue(const wstring& from, Vec3& to)
	{
		return 3 == swscanf(from.c_str(), L"%f,%f,%f", &to.x, &to.y, &to.z);
	}
};

template<>
struct SUIConversion<string, wstring>
{
	static ILINE bool ConvertValue(const string& from, wstring& to)
	{
		Unicode::Convert(to, from);
		return true;
	}
};

template<>
struct SUIConversion<wstring, string>
{
	static ILINE bool ConvertValue(const wstring& from, string& to)
	{
		Unicode::Convert(to, from);
		return true;
	}
};

enum EUIDataTypes
{
	eUIDT_Any      = -1,
	eUIDT_Bool     = drx_variant::get_index<bool, TUIDataVariant>::value,
	eUIDT_Int      = drx_variant::get_index<i32, TUIDataVariant>::value,
	eUIDT_Float    = drx_variant::get_index<float, TUIDataVariant>::value,
	eUIDT_EntityId = drx_variant::get_index<EntityId, TUIDataVariant>::value,
	eUIDT_Vec3     = drx_variant::get_index<Vec3, TUIDataVariant>::value,
	eUIDT_String   = drx_variant::get_index<string, TUIDataVariant>::value,
	eUIDT_WString  = drx_variant::get_index<wstring, TUIDataVariant>::value,
};

class TUIData
{
	class ExtractType
	{
	public:
		EUIDataTypes operator()(const TUIDataVariant& variant)
		{
			return static_cast<EUIDataTypes>(variant.index());
		}
	};

	template<typename To>
	class ConvertType_Get
	{
	public:
		ConvertType_Get(To& to_) : to(to_) {}

		template<typename From>
		bool operator()(const From& from) const
		{
			return SUIConversion<From, To>::ConvertValue(from, to);
		}

		To& to;
	};

public:
	TUIData()
		: m_variant()
	{}

	TUIData(const TUIData& rhs)
		: m_variant(rhs.m_variant)
	{}

	template<typename T>
	explicit TUIData(const T& v, bool locked = false)
		: m_variant(v)
	{}

	TUIData& operator=(const TUIData& rhs)
	{
		m_variant = rhs.m_variant;
		return *this;
	}

	template<typename T>
	bool Set(const T& value)
	{
		m_variant = value;
		return true;
	}

	template<typename T>
	bool GetValueWithConversion(T& value) const
	{
		return stl::visit(ConvertType_Get<T>(value), m_variant);
	}

	EUIDataTypes                  GetType() const { return stl::visit(ExtractType(), m_variant); }

	template<typename T> T*       GetPtr()        { return stl::get_if<T>(&m_variant); }
	template<typename T> const T* GetPtr() const  { return stl::get_if<const T>(&m_variant); }

private:
	TUIDataVariant m_variant;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// UI Arguments //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#define UIARGS_DEFAULT_DELIMITER      "|"
#define UIARGS_DEFAULT_DELIMITER_NAME "pipe" // for flownode descriptions

template<class T> struct SUIParamTypeHelper
{
	static EUIDataTypes GetType(const T&)
	{
		return eUIDT_Any;
	}
};
template<> struct SUIParamTypeHelper<bool>
{
	static EUIDataTypes GetType(const bool&)
	{
		return eUIDT_Bool;
	}
};
template<> struct SUIParamTypeHelper<i32>
{
	static EUIDataTypes GetType(i32k&)
	{
		return eUIDT_Int;
	}
};
template<> struct SUIParamTypeHelper<EntityId>
{
	static EUIDataTypes GetType(const EntityId&)
	{
		return eUIDT_EntityId;
	}
};
template<> struct SUIParamTypeHelper<float>
{
	static EUIDataTypes GetType(const float&)
	{
		return eUIDT_Float;
	}
};
template<> struct SUIParamTypeHelper<Vec3>
{
	static EUIDataTypes GetType(const Vec3&)
	{
		return eUIDT_Vec3;
	}
};
template<> struct SUIParamTypeHelper<string>
{
	static EUIDataTypes GetType(const string&)
	{
		return eUIDT_String;
	}
};
template<> struct SUIParamTypeHelper<wstring>
{
	static EUIDataTypes GetType(const wstring&)
	{
		return eUIDT_WString;
	}
};
template<> struct SUIParamTypeHelper<TUIData>
{
	static EUIDataTypes GetType(const TUIData& d)
	{
		return (EUIDataTypes) d.GetType();
	}
};
//! \endcond

struct SUIArguments
{
	SUIArguments() : m_cDelimiter(UIARGS_DEFAULT_DELIMITER), m_Dirty(eBDF_Delimiter) {};
	template<class T>
	SUIArguments(const T* argStringList, bool bufferStr = false) : m_cDelimiter(UIARGS_DEFAULT_DELIMITER) { SetArguments(argStringList, bufferStr); }
	SUIArguments(const SFlashVarValue* vArgs, i32 iNumArgs) : m_cDelimiter(UIARGS_DEFAULT_DELIMITER) { SetArguments(vArgs, iNumArgs); }
	SUIArguments(const TUIData& data) : m_cDelimiter(UIARGS_DEFAULT_DELIMITER) { AddArgument(data); }

	template<class T>
	void SetArguments(const T* argStringList, bool bufferStr = false)
	{
		Clear();
		AddArguments(argStringList, bufferStr);
	}

	template<class T>
	void AddArguments(const T* argStringList, bool bufferStr = false)
	{
		const DrxStringT<T>& delimiter = GetDelimiter<T>();
		const T* delimiter_str = delimiter.c_str();
		i32k delimiter_len = delimiter.length();
		const size_t s = StrLenTmpl(argStringList) + 1;
		T* buffer = new T[s];
		memcpy(buffer, argStringList, s * sizeof(T));
		T* found = buffer;
		while (*found)
		{
			T* next = StrStrTmpl(found, delimiter_str);
			if (next)
			{
				next[0] = 0;
				AddArgument(DrxStringT<T>(found), eUIDT_Any);
				next[0] = delimiter_str[0];
				found = next + delimiter_len;
			}
			if (!next || (next && !*found))
			{
				AddArgument(DrxStringT<T>(found), eUIDT_Any);
				break;
			}
		}
		if (bufferStr)
			setStringBuffer(argStringList);
		delete[] buffer;
	}

	void SetArguments(const SFlashVarValue* vArgs, i32 iNumArgs)
	{
		Clear();
		AddArguments(vArgs, iNumArgs);
	}

	void AddArguments(const SFlashVarValue* vArgs, i32 iNumArgs)
	{
		m_ArgList.reserve(m_ArgList.size() + iNumArgs);
		for (i32 i = 0; i < iNumArgs; ++i)
		{
			switch (vArgs[i].GetType())
			{
			case SFlashVarValue::eBool:
				AddArgument(vArgs[i].GetBool());
				break;
			case SFlashVarValue::eInt:
				AddArgument(vArgs[i].GetInt());
				break;
			case SFlashVarValue::eUInt:
				AddArgument(vArgs[i].GetUInt());
				break;
			case SFlashVarValue::eFloat:
				AddArgument(vArgs[i].GetFloat());
				break;
			case SFlashVarValue::eDouble:
				AddArgument((float) vArgs[i].GetDouble());
				break;
			case SFlashVarValue::eConstStrPtr:
				AddArgument(vArgs[i].GetConstStrPtr());
				break;
			case SFlashVarValue::eConstWstrPtr:
				AddArgument(vArgs[i].GetConstWstrPtr());
				break;
			case SFlashVarValue::eNull:
				AddArgument("NULL");
				break;
			case SFlashVarValue::eObject:
				AddArgument("OBJECT");
				break;
			case SFlashVarValue::eUndefined:
				AddArgument("UNDEFINED");
				break;
			}
		}
	}

	void AddArguments(const SUIArguments& args)
	{
		i32k iNumArgs = args.GetArgCount();
		m_ArgList.reserve(m_ArgList.size() + iNumArgs);
		for (i32 i = 0; i < iNumArgs; ++i)
			AddArgument(args.GetArg(i), args.GetArgType(i));
	}

	template<class T>
	inline void AddArgument(const T& arg, EUIDataTypes type)
	{
		m_ArgList.push_back(SUIData(type, TUIData(arg)));
		m_Dirty = eBDF_ALL;
	}

	template<class T>
	inline void AddArgument(const T& arg)
	{
		AddArgument(arg, SUIParamTypeHelper<T>::GetType(arg));
	}

	template<class T>
	inline void AddArgument(const T* str)
	{
		AddArgument(DrxStringT<T>(str));
	}

	inline void Clear()
	{
		m_ArgList.clear();
		m_Dirty = eBDF_ALL;
	}

	template<class T>
	static SUIArguments Create(const T& arg)
	{
		SUIArguments args;
		args.AddArgument(arg);
		return args;
	}

	template<class T>
	static SUIArguments Create(const T* str)
	{
		SUIArguments args;
		args.AddArgument(str);
		return args;
	}

	inline i32            GetArgCount() const  { return m_ArgList.size(); }

	tukk           GetAsString() const  { return updateStringBuffer(m_sArgStringBuffer, eBDF_String); }
	const wchar_t*        GetAsWString() const { return updateStringBuffer(m_sArgWStringBuffer, eBDF_WString); }
	const SFlashVarValue* GetAsList() const    { return updateFlashBuffer(); }

	inline const TUIData& GetArg(i32 index) const
	{
		assert(index >= 0 && index < m_ArgList.size());
		return m_ArgList[index].Data;
	}

	inline EUIDataTypes GetArgType(i32 index) const
	{
		assert(index >= 0 && index < m_ArgList.size());
		return m_ArgList[index].Type;
	}

	template<class T>
	inline bool GetArg(i32 index, T& val) const
	{
		if (index >= 0 && index < m_ArgList.size())
			return m_ArgList[index].Data.GetValueWithConversion(val);
		return false;
	}

	template<class T>
	inline void GetArgNoConversation(i32 index, T& val) const
	{
		assert(index >= 0 && index < m_ArgList.size());
		const T* valPtr = m_ArgList[index].Data.GetPtr<T>();
		assert(valPtr);
		val = *valPtr;
	}

	inline void SetDelimiter(const string& delimiter)
	{
		if (delimiter != m_cDelimiter)
		{
			m_Dirty |= eBDF_String | eBDF_WString | eBDF_Delimiter;
		}
		m_cDelimiter = delimiter;
	}

	template<class T>
	inline T* StrStrTmpl(T* str1, const T* str2)
	{
		return strstr(str1, str2);
	}

	template<class T>
	inline size_t StrLenTmpl(const T* str)
	{
		return strlen(str);
	}

private:
	struct SUIData
	{
		SUIData(EUIDataTypes type, const TUIData& data) : Type(type), Data(data) {}
		EUIDataTypes Type;
		TUIData      Data;
	};
	DynArray<SUIData>                m_ArgList;
	mutable string                   m_sArgStringBuffer;    //!< Buffer for tukk GetAsString().
	mutable wstring                  m_sArgWStringBuffer;   //!< Buffer for const wchar_t* GetAsWString().
	mutable DynArray<SFlashVarValue> m_FlashValueBuffer;    //!< buffer for const SFlashVarValue* GetAsList().
	string                           m_cDelimiter;

	enum EBufferDirtyFlag
	{
		eBDF_None      = 0x00,
		eBDF_String    = 0x01,
		eBDF_WString   = 0x02,
		eBDF_FlashVar  = 0x04,
		eBDF_Delimiter = 0x08,
		eBDF_ALL       = 0xFF,
	};
	mutable uint           m_Dirty;

	inline SFlashVarValue* updateFlashBuffer() const
	{
		if (m_Dirty & eBDF_FlashVar)
		{
			m_Dirty &= ~eBDF_FlashVar;
			m_FlashValueBuffer.clear();
			for (DynArray<SUIData>::const_iterator it = m_ArgList.begin(); it != m_ArgList.end(); ++it)
			{
				bool bConverted = false;
				switch (it->Type)
				{
				case eUIDT_Bool:
					AddValue<bool>(it->Data);
					break;
				case eUIDT_Int:
					AddValue<i32>(it->Data);
					break;
				case eUIDT_EntityId:
					AddValue<EntityId>(it->Data);
					break;
				case eUIDT_Float:
					AddValue<float>(it->Data);
					break;
				case eUIDT_String:
					AddValue<string>(it->Data);
					break;
				case eUIDT_WString:
					AddValue<wstring>(it->Data);
					break;
				case eUIDT_Any:
					{
						bool bRes = TryAddValue<i32>(it->Data)
						            || TryAddValue<float>(it->Data)
						            || (it->Data.GetType() == eUIDT_String && AddValue<string>(it->Data))
						            || (it->Data.GetType() == eUIDT_WString && AddValue<wstring>(it->Data));
						assert(bRes);
					}
					break;
				default:
					assert(false);
					break;
				}
			}
		}
		return m_FlashValueBuffer.size() > 0 ? &m_FlashValueBuffer[0] : NULL;
	}

	template<class T>
	inline bool AddValue(const TUIData& data) const
	{
		const T* val = data.GetPtr<T>();
		assert(val);
		m_FlashValueBuffer.push_back(SFlashVarValue(*val));
		return true;
	}

	template<class T>
	inline bool TryAddValue(const TUIData& data) const
	{
		T val;
		if (data.GetValueWithConversion(val))
		{
			m_FlashValueBuffer.push_back(SFlashVarValue(val));
			return true;
		}
		return false;
	}

	template<class T>
	inline const T* updateStringBuffer(DrxStringT<T>& buffer, uint flag) const
	{
		if (m_Dirty & flag)
		{
			m_Dirty &= ~flag;
			DrxStringT<T> delimiter_str = GetDelimiter<T>();
			buffer.clear();
			for (DynArray<SUIData>::const_iterator it = m_ArgList.begin(); it != m_ArgList.end(); ++it)
			{
				if (buffer.size() > 0) buffer += delimiter_str;
				DrxStringT<T> val;
				bool bRes = it->Data.GetValueWithConversion(val);
				assert(bRes && "try to convert to tuk string list but some of the values are unsupported wchar_t*");
				buffer += val;
			}
		}
		return buffer.c_str();
	}

	template<class T>
	inline const DrxStringT<T>& GetDelimiter() const
	{
		static DrxStringT<T> delimiter_str;
		if (m_Dirty & eBDF_Delimiter)
		{
			m_Dirty &= ~eBDF_Delimiter;
			TUIData delimiter(m_cDelimiter);
			delimiter.GetValueWithConversion(delimiter_str);
		}
		return delimiter_str;
	}

	template<class T>
	inline void setStringBuffer(const T* str) { assert(false); }
};

//! Specialize in global scope.
template<>
inline const DrxStringT<char>& SUIArguments::GetDelimiter() const
{
	return m_cDelimiter;
}

template<>
inline void SUIArguments::setStringBuffer(tukk str)
{
	m_sArgStringBuffer = str;
	m_Dirty &= ~eBDF_String;
}

template<>
inline void SUIArguments::setStringBuffer(const wchar_t* str)
{
	m_sArgWStringBuffer = str;
	m_Dirty &= ~eBDF_WString;
}

template<>
inline wchar_t* SUIArguments::StrStrTmpl(wchar_t* str1, const wchar_t* str2)
{
	return wcsstr(str1, str2);
}

template<>
inline size_t SUIArguments::StrLenTmpl(const wchar_t* str)
{
	return wcslen(str);
}

template<>
inline bool SUIArguments::GetArg(i32 index, TUIData& val) const
{
	if (index >= 0 && index < m_ArgList.size())
	{
		val = GetArg(index);
		return true;
	}
	return false;
}

//! Should be replaced by SUIArguments&& on platforms with Cx11 support.
typedef SUIArguments SUIArgumentsRet;

/////////////////////////////////////////// Lookup Table ///////////////////////////////////////////

//! \cond INTERNAL
//! This type is not implemented!
template<class T> struct SUIItemLookupIDD
{
	static inline tukk GetId(const T* p)
	{
		assert(false);
		return "";
	}
};

template<class Base>
struct SUIItemLookupSet_Impl
{
	struct CILess
	{
		bool operator() (const CDrxName& lhs, const CDrxName& rhs) const
		{
			return stricmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};

	typedef DynArray<Base*>                 TITems;
	typedef std::map<CDrxName, i32, CILess> TLookup;
	typedef typename TITems::iterator       iterator;
	typedef typename TITems::const_iterator const_iterator;
	typedef typename TITems::size_type      size_type;
	typedef typename TITems::value_type     value_type;

	inline Base* operator[](i32 __n)
	{
		return m_Items[__n];
	}

	inline Base* operator()(tukk __name)
	{
		typename TLookup::iterator it = m_Lookup.find(CDrxName(__name));
		return it != m_Lookup.end() ? m_Items[it->second] : NULL;
	}

	inline const Base* operator[](i32 __n) const
	{
		return m_Items[__n];
	}

	inline const Base* operator()(tukk __name) const
	{
		typename TLookup::const_iterator it = m_Lookup.find(CDrxName(__name));
		return it != m_Lookup.end() ? m_Items[it->second] : NULL;
	}

	void push_back(Base* item)
	{
		assert(m_Lookup[CDrxName(SUIItemLookupIDD < Base > ::GetId(item))] == 0);
		m_Lookup[CDrxName(SUIItemLookupIDD < Base > ::GetId(item))] = m_Items.size();
		m_Items.push_back(item);
	}

	void remove(Base* item)
	{
		typename TLookup::iterator it = m_Lookup.find(CDrxName(SUIItemLookupIDD<Base>::GetId(item)));
		if (it != m_Lookup.end())
		{
			for (typename TLookup::iterator nextIt = it; nextIt != m_Lookup.end(); ++nextIt)
				nextIt->second--;
			m_Lookup.erase(it);
			for (typename TITems::iterator it = m_Items.begin(); it != m_Items.end(); ++it)
			{
				if (*it == item)
				{
					m_Items.erase(it);
					return;
				}
			}
		}
		assert(false);
	}

	void clear()
	{
		m_Items.clear();
		m_Lookup.clear();
	}

	inline size_type get_alloc_size() const
	{
		return m_Items.get_alloc_size() + stl::size_of_map(m_Lookup) + m_Items.size() * sizeof(Base);
	}

	inline size_type      size() const     { return m_Items.size(); }
	inline iterator       begin()          { return m_Items.begin(); }
	inline iterator       end()            { return m_Items.end(); }
	inline const_iterator begin() const    { return m_Items.begin(); }
	inline const_iterator end() const      { return m_Items.end(); }
	inline size_type      capacity() const { return m_Items.capacity(); }

protected:
	TITems  m_Items;
	TLookup m_Lookup;
};

struct IUIElement;
struct IUIAction;
struct SUIParameterDesc;
struct SUIMovieClipDesc;
struct SUIEventDesc;

#if defined(_LIB)
template<class Base> struct SUIItemLookupSet : public SUIItemLookupSet_Impl<Base> {};
#else
struct SUIItemLookupSetFactory
{
	static inline SUIItemLookupSet_Impl<SUIParameterDesc>* CreateLookupParameter();
	static inline SUIItemLookupSet_Impl<SUIMovieClipDesc>* CreateLookupMovieClip();
	static inline SUIItemLookupSet_Impl<SUIEventDesc>*     CreateLookupEvent();
};

template<class Base>
struct SUIItemLookupSet_DllSafeImpl
{
	typedef DynArray<Base*>                 TITems;
	typedef std::map<CDrxName, i32>         TLookup;
	typedef typename TITems::iterator       iterator;
	typedef typename TITems::const_iterator const_iterator;
	typedef typename TITems::size_type      size_type;
	typedef typename TITems::value_type     value_type;

	SUIItemLookupSet_DllSafeImpl() { m_pImpl = CreateLookupImpl(); }
	~SUIItemLookupSet_DllSafeImpl() { delete m_pImpl; }

	inline Base*          operator[](i32 __n)                  { return m_pImpl->operator[](__n); }
	inline Base*          operator()(tukk __name)       { return m_pImpl->operator()(__name); }
	inline const Base*    operator[](i32 __n) const            { return m_pImpl->operator[](__n); }
	inline const Base*    operator()(tukk __name) const { return m_pImpl->operator()(__name); }
	void                  push_back(Base* item)                { m_pImpl->push_back(item); }
	void                  remove(Base* item)                   { m_pImpl->remove(item); }
	void                  clear()                              { m_pImpl->clear(); }
	inline size_type      get_alloc_size() const               { return m_pImpl->get_alloc_size(); }
	inline size_type      size() const                         { return m_pImpl->size(); }
	inline iterator       begin()                              { return m_pImpl->begin(); }
	inline iterator       end()                                { return m_pImpl->end(); }
	inline const_iterator begin() const                        { return m_pImpl->begin(); }
	inline const_iterator end() const                          { return m_pImpl->end(); }
	inline size_type      capacity() const                     { return m_pImpl->capacity(); }

protected:
	inline SUIItemLookupSet_Impl<Base>* CreateLookupImpl() { assert(false); return NULL; }

protected:
	SUIItemLookupSet_Impl<Base>* m_pImpl;
};
template<> inline SUIItemLookupSet_Impl<SUIParameterDesc>* SUIItemLookupSet_DllSafeImpl<SUIParameterDesc >::CreateLookupImpl() { return SUIItemLookupSetFactory::CreateLookupParameter(); }
template<> inline SUIItemLookupSet_Impl<SUIMovieClipDesc>* SUIItemLookupSet_DllSafeImpl<SUIMovieClipDesc >::CreateLookupImpl() { return SUIItemLookupSetFactory::CreateLookupMovieClip(); }
template<> inline SUIItemLookupSet_Impl<SUIEventDesc>* SUIItemLookupSet_DllSafeImpl<SUIEventDesc         >::CreateLookupImpl() { return SUIItemLookupSetFactory::CreateLookupEvent(); }

template<class Base> struct SUIItemLookupSet : public SUIItemLookupSet_DllSafeImpl<Base> {};
#endif

typedef SUIItemLookupSet<SUIParameterDesc> TUIParamsLookup;
typedef SUIItemLookupSet<SUIMovieClipDesc> TUIMovieClipLookup;
typedef SUIItemLookupSet<SUIEventDesc>     TUIEventsLookup;

//! Since those are not shared between DLL boundaries we can use the impl directly.
typedef SUIItemLookupSet_Impl<IUIElement> TUIElementsLookup;
typedef SUIItemLookupSet_Impl<IUIAction>  TUIActionsLookup;

//! \endcond

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// UI Descriptions /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
struct SUIParameterDesc
{
	enum EUIParameterType
	{
		eUIPT_Any,
		eUIPT_Bool,
		eUIPT_Int,
		eUIPT_Float,
		eUIPT_Vec3,
		eUIPT_String,
		eUIPT_WString
	};

	SUIParameterDesc() : sName("undefined"), sDisplayName("undefined"), sDesc("undefined"), eType(eUIPT_Any), pParent(NULL) {}
	SUIParameterDesc(tukk name, tukk displ, tukk desc, EUIParameterType type = eUIPT_Any, const SUIParameterDesc* parent = NULL) : sName(name), sDisplayName(displ), sDesc(desc), eType(type), pParent(parent) { DRX_ASSERT_MESSAGE(strstr(sName, " ") == NULL, "Name with whitespaces not allowed! Use display name for descriptive names!"); }
	tukk             sName;
	tukk             sDisplayName;
	tukk             sDesc;
	const SUIParameterDesc* pParent;
	EUIParameterType        eType;

	inline void             GetInstanceString(string& instanceStr, const SUIParameterDesc* pTempl = NULL) const
	{
		if (pParent)
		{
			pParent->GetInstanceString(instanceStr, pTempl);
			instanceStr += ".";
			instanceStr += sName;
		}
		else if (pTempl && pTempl != this)
		{
			pTempl->GetInstanceString(instanceStr);
			instanceStr += ".";
			instanceStr += sName;
		}
		else
		{
			instanceStr = sName;
		}
	}

	inline bool operator==(const SUIParameterDesc& other) const
	{
		return strcmp(sName, other.sName) == 0;
	}
};
typedef DynArray<SUIParameterDesc> TUIParams;

struct SUIEventDesc : public SUIParameterDesc
{
	struct SEvtParamsDesc
	{
		SEvtParamsDesc(bool isDyn = false, tukk dynName = "Array", tukk dynDesc = "") : IsDynamic(isDyn), sDynamicName(dynName), sDynamicDesc(dynDesc)
		{
			DRX_ASSERT_MESSAGE(strstr(sDynamicName, " ") == NULL, "DynamicName with whitespaces not allowed!");
		}

		bool        IsDynamic;
		tukk sDynamicName;
		tukk sDynamicDesc;
	};

	SUIEventDesc() {}
	SUIEventDesc(tukk name, tukk desc) : SUIParameterDesc(name, name, desc) {}
	SUIEventDesc(tukk name, tukk displ, tukk desc, SEvtParamsDesc inputs = SEvtParamsDesc(), SEvtParamsDesc outputs = SEvtParamsDesc(), const SUIParameterDesc* parent = NULL)
		: SUIParameterDesc(name, displ, desc, eUIPT_Any, parent)
		, InputParams(inputs)
		, OutputParams(outputs)
	{
	}

	struct SEvtParams : public SEvtParamsDesc
	{
		SEvtParams() {}
		SEvtParams(const SEvtParamsDesc& desc) : SEvtParamsDesc(desc) {}
		TUIParams Params;

		inline bool operator==(const SEvtParams& other) const
		{
			bool res = IsDynamic == other.IsDynamic && Params.size() == other.Params.size();
			for (i32 i = 0; i < Params.size() && res; ++i)
			{
				res &= Params[i] == other.Params[i];
			}
			return res;
		}

		inline void SetDynamic(tukk name, tukk desc)
		{
			DRX_ASSERT_MESSAGE(strstr(name, " ") == NULL, "DynamicName with whitespaces not allowed!");
			IsDynamic = true;
			sDynamicName = name;
			sDynamicDesc = desc;
		}

		template<EUIParameterType T>
		inline void AddParam(tukk name, tukk desc)
		{
			Params.push_back(SUIParameterDesc(name, name, desc, T));
		}

		template<EUIParameterType T>
		inline void AddParam(tukk name, tukk displ, tukk desc)
		{
			Params.push_back(SUIParameterDesc(name, displ, desc, T));
		}
	};

	inline void SetDynamic(tukk name, tukk desc)
	{
		InputParams.SetDynamic(name, desc);
	}

	template<EUIParameterType T>
	inline void AddParam(tukk name, tukk desc)
	{
		InputParams.AddParam<T>(name, desc);
	}

	template<EUIParameterType T>
	inline void AddParam(tukk name, tukk displ, tukk desc)
	{
		InputParams.AddParam<T>(name, displ, desc);
	}

	inline bool operator==(const SUIEventDesc& other) const
	{
		return strcmp(sName, other.sName) == 0 && InputParams == other.InputParams && OutputParams == other.OutputParams;
	}

	inline bool operator==(tukk name) const
	{
		return stricmp(sDisplayName, name) == 0;
	}

	SEvtParams InputParams;
	SEvtParams OutputParams;
};

struct SUIMovieClipDesc : public SUIParameterDesc
{
	SUIMovieClipDesc() {}
	SUIMovieClipDesc(tukk name, tukk displ, tukk desc, EUIParameterType type = eUIPT_Any, const SUIParameterDesc* parent = NULL) : SUIParameterDesc(name, displ, desc, type, parent) {}

	const SUIParameterDesc* GetVariableDesc(i32 index) const                   { return index < m_variables.size() ? m_variables[index] : NULL; }
	const SUIParameterDesc* GetVariableDesc(tukk sVarName) const        { return m_variables(sVarName); }
	i32                     GetVariableCount() const                           { return m_variables.size(); }

	const SUIParameterDesc* GetArrayDesc(i32 index) const                      { return index < m_arrays.size() ? m_arrays[index] : NULL; }
	const SUIParameterDesc* GetArrayDesc(tukk sArrayName) const         { return m_arrays(sArrayName); }
	i32                     GetArrayCount() const                              { return m_arrays.size(); }

	const SUIMovieClipDesc* GetMovieClipDesc(i32 index) const                  { return index < m_displayObjects.size() ? m_displayObjects[index] : NULL; }
	const SUIMovieClipDesc* GetMovieClipDesc(tukk sMovieClipName) const { return m_displayObjects(sMovieClipName); }
	i32                     GetMovieClipCount() const                          { return m_displayObjects.size(); }

	const SUIEventDesc*     GetFunctionDesc(i32 index) const                   { return index < m_functions.size() ? m_functions[index] : NULL; }
	const SUIEventDesc*     GetFunctionDesc(tukk sFunctionName) const   { return m_functions(sFunctionName); }
	i32                     GetFunctionCount() const                           { return m_functions.size(); }

	TUIParamsLookup    m_variables;
	TUIParamsLookup    m_arrays;
	TUIMovieClipLookup m_displayObjects;
	TUIEventsLookup    m_functions;
};

template<> inline tukk SUIItemLookupIDD<SUIParameterDesc >::GetId(const SUIParameterDesc* p) { return p->sDisplayName; }
template<> inline tukk SUIItemLookupIDD<SUIMovieClipDesc >::GetId(const SUIMovieClipDesc* p) { return p->sDisplayName; }
template<> inline tukk SUIItemLookupIDD<SUIEventDesc     >::GetId(const SUIEventDesc* p)     { return p->sDisplayName; }

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// UI Element ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
struct IUIElement;

struct IUIElementEventListener
{
	virtual void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)                {}
	virtual void OnUIEventEx(IUIElement* pSender, tukk fscommand, const SUIArguments& args, uk pUserData) {}

	virtual void OnInit(IUIElement* pSender, IFlashPlayer* pFlashPlayer)                                            {}
	virtual void OnUnload(IUIElement* pSender)                                                                      {}
	virtual void OnSetVisible(IUIElement* pSender, bool bVisible)                                                   {}

	virtual void OnInstanceCreated(IUIElement* pSender, IUIElement* pNewInstance)                                   {}
	virtual void OnInstanceDestroyed(IUIElement* pSender, IUIElement* pDeletedInstance)                             {}
protected:
	virtual ~IUIElementEventListener() {};
};

struct IUIElementIterator
{
	virtual ~IUIElementIterator() {}

	virtual void        AddRef() = 0;
	virtual void        Release() = 0;

	virtual IUIElement* Next() = 0;
	virtual i32         GetCount() const = 0;
};
TYPEDEF_AUTOPTR(IUIElementIterator);
typedef IUIElementIterator_AutoPtr IUIElementIteratorPtr;

struct IUIElement
{
	struct SUIConstraints
	{
		enum EPositionType
		{
			ePT_Fixed,
			ePT_Fullscreen,
			ePT_Dynamic,
			ePT_FixedDynTexSize
		};

		enum EPositionAlign
		{
			ePA_Lower,
			ePA_Mid,
			ePA_Upper,
		};

		SUIConstraints()
			: eType(ePT_Dynamic)
			, iLeft(0)
			, iTop(0)
			, iWidth(1024)
			, iHeight(768)
			, eHAlign(ePA_Mid)
			, eVAlign(ePA_Mid)
			, bScale(true)
		{
		}

		SUIConstraints(EPositionType type, i32 left, i32 top, i32 width, i32 height, EPositionAlign halign, EPositionAlign valign, bool scale, bool ismax)
			: eType(type)
			, iLeft(left)
			, iTop(top)
			, iWidth(width)
			, iHeight(height)
			, eHAlign(halign)
			, eVAlign(valign)
			, bScale(scale)
			, bMax(ismax)
		{
		}

		EPositionType  eType;
		i32            iLeft;
		i32            iTop;
		i32            iWidth;
		i32            iHeight;
		EPositionAlign eHAlign;
		EPositionAlign eVAlign;
		bool           bScale;
		bool           bMax;
	};

	enum EFlashUIFlags
	{
		//! Flags per instance.
		eFUI_MASK_PER_INSTANCE = 0x0FFFF,
		eFUI_NOT_CHANGEABLE    = 0x01000,
		eFUI_HARDWARECURSOR    = 0x00001,
		eFUI_MOUSEEVENTS       = 0x00002,
		eFUI_KEYEVENTS         = 0x00004,
		eFUI_CONSOLE_MOUSE     = 0x00008,
		eFUI_CONSOLE_CURSOR    = 0x00010,
		eFUI_CONTROLLER_INPUT  = 0x00020,
		eFUI_EVENTS_EXCLUSIVE  = 0x00040,
		eFUI_RENDER_LOCKLESS   = 0x00080,
		eFUI_FIXED_PROJ_DEPTH  = 0x00100,
		eFUI_IS_HUD            = 0x00200,
		eFUI_SHARED_RT         = 0x00400,
		eFUI_LAZY_UPDATE       = 0x00800 | eFUI_NOT_CHANGEABLE,
		eFUI_NO_AUTO_UPDATE    = 0x02000,

		//! Flags per UIElement.
		eFUI_MASK_PER_ELEMENT = 0xF0000,
		eFUI_FORCE_NO_UNLOAD  = 0x10000,
	};

	enum EControllerInputEvent
	{
		eCIE_Up = 0,
		eCIE_Down,
		eCIE_Left,
		eCIE_Right,
		eCIE_Action,
		eCIE_Back,
		eCIE_Start,
		eCIE_Select,
		eCIE_ShoulderL1,
		eCIE_ShoulderL2,
		eCIE_ShoulderR1,
		eCIE_ShoulderR2,
		eCIE_Button3,
		eCIE_Button4,
		eCIE_DpadUp,
		eCIE_DpadDown,
		eCIE_DpadLeft,
		eCIE_DpadRight,
	};

	enum EControllerInputState
	{
		eCIS_OnPress = 0,
		eCIS_OnRelease,
	};

	virtual ~IUIElement() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;

	// instances
	virtual uint                  GetInstanceID() const = 0;
	virtual IUIElement*           GetInstance(uint instanceID) = 0;
	virtual IUIElementIteratorPtr GetInstances() const = 0;
	virtual bool                  DestroyInstance(uint instanceID) = 0;
	virtual bool                  DestroyThis() = 0;

	// common
	virtual void        SetName(tukk name) = 0;
	virtual tukk GetName() const = 0;

	virtual void        SetGroupName(tukk groupName) = 0;
	virtual tukk GetGroupName() const = 0;

	virtual void        SetFlashFile(tukk flashFile) = 0;
	virtual tukk GetFlashFile() const = 0;

	virtual bool        Init(bool bLoadAsset = true) = 0;
	virtual void        Unload(bool bAllInstances = false) = 0;
	virtual void        Reload(bool bAllInstances = false) = 0;
	virtual bool        IsInit() const = 0;

	//! Request an unload on the next frame, in case the unload is requested by a UI event.
	virtual void RequestUnload(bool bAllInstances = false) = 0;

	virtual bool IsValid() const = 0;

	virtual void UnloadBootStrapper() = 0;
	virtual void ReloadBootStrapper() = 0;

	virtual void Update(float fDeltaTime) = 0;
	virtual void Render() = 0;
	virtual void RenderLockless() = 0;

	// visibility
	virtual void                              RequestHide() = 0;
	virtual bool                              IsHiding() const = 0;

	virtual void                              SetVisible(bool bVisible) = 0;
	virtual bool                              IsVisible() const = 0;

	virtual void                              SetFlag(EFlashUIFlags flag, bool bSet) = 0;
	virtual bool                              HasFlag(EFlashUIFlags flag) const = 0;

	virtual float                             GetAlpha() const = 0;
	virtual void                              SetAlpha(float fAlpha) = 0;

	virtual i32                               GetLayer() const = 0;
	virtual void                              SetLayer(i32 iLayer) = 0;

	virtual void                              SetConstraints(const SUIConstraints& newConstraints) = 0;
	virtual const IUIElement::SUIConstraints& GetConstraints() const = 0;

	//! For lazy update.
	virtual void ForceLazyUpdate() = 0;
	virtual void LazyRendered() = 0;
	virtual bool NeedLazyRender() const = 0;

	//! Raw IFlashPlayer.
	virtual std::shared_ptr<IFlashPlayer> GetFlashPlayer() = 0;

	// definitions
	virtual const SUIParameterDesc* GetVariableDesc(i32 index) const = 0;
	virtual const SUIParameterDesc* GetVariableDesc(tukk varName) const = 0;
	virtual i32                     GetVariableCount() const = 0;

	virtual const SUIParameterDesc* GetArrayDesc(i32 index) const = 0;
	virtual const SUIParameterDesc* GetArrayDesc(tukk arrayName) const = 0;
	virtual i32                     GetArrayCount() const = 0;

	virtual const SUIMovieClipDesc* GetMovieClipDesc(i32 index) const = 0;
	virtual const SUIMovieClipDesc* GetMovieClipDesc(tukk movieClipName) const = 0;
	virtual i32                     GetMovieClipCount() const = 0;

	virtual const SUIMovieClipDesc* GetMovieClipTmplDesc(i32 index) const = 0;
	virtual const SUIMovieClipDesc* GetMovieClipTmplDesc(tukk movieClipTmplName) const = 0;
	virtual i32                     GetMovieClipTmplCount() const = 0;

	virtual const SUIEventDesc*     GetEventDesc(i32 index) const = 0;
	virtual const SUIEventDesc*     GetEventDesc(tukk eventName) const = 0;
	virtual i32                     GetEventCount() const = 0;

	virtual const SUIEventDesc*     GetFunctionDesc(i32 index) const = 0;
	virtual const SUIEventDesc*     GetFunctionDesc(tukk functionName) const = 0;
	virtual i32                     GetFunctionCount() const = 0;

	virtual void                    UpdateViewPort() = 0;
	virtual void                    GetViewPort(i32& x, i32& y, i32& width, i32& height, float& aspectRatio) = 0;

	virtual bool                    Serialize(XmlNodeRef& xmlNode, bool bIsLoading) = 0;

	//! Event listener.
	virtual void AddEventListener(IUIElementEventListener* pListener, tukk name) = 0;
	virtual void RemoveEventListener(IUIElementEventListener* pListener) = 0;

	//! Functions and objects.
	virtual bool                  CallFunction(tukk fctName, const SUIArguments& args = SUIArguments(), TUIData* pDataRes = NULL, tukk pTmplName = NULL) = 0;
	virtual bool                  CallFunction(const SUIEventDesc* pFctDesc, const SUIArguments& args = SUIArguments(), TUIData* pDataRes = NULL, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual IFlashVariableObject* GetMovieClip(tukk movieClipName, tukk pTmplName = NULL) = 0;
	virtual IFlashVariableObject* GetMovieClip(const SUIMovieClipDesc* pMovieClipDesc, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual IFlashVariableObject* CreateMovieClip(const SUIMovieClipDesc*& pNewInstanceDesc, tukk movieClipTemplate, tukk mcParentName = NULL, tukk mcInstanceName = NULL) = 0;
	virtual IFlashVariableObject* CreateMovieClip(const SUIMovieClipDesc*& pNewInstanceDesc, const SUIMovieClipDesc* pMovieClipTemplateDesc, const SUIMovieClipDesc* pParentMC = NULL, tukk mcInstanceName = NULL) = 0;

	virtual void                  RemoveMovieClip(tukk movieClipName) = 0;
	virtual void                  RemoveMovieClip(const SUIParameterDesc* pMovieClipDesc) = 0;
	virtual void                  RemoveMovieClip(IFlashVariableObject* pVarObject) = 0;

	virtual bool                  SetVariable(tukk varName, const TUIData& value, tukk pTmplName = NULL) = 0;
	virtual bool                  SetVariable(const SUIParameterDesc* pVarDesc, const TUIData& value, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual bool                  GetVariable(tukk varName, TUIData& valueOut, tukk pTmplName = NULL) = 0;
	virtual bool                  GetVariable(const SUIParameterDesc* pVarDesc, TUIData& valueOut, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual bool                  CreateVariable(const SUIParameterDesc*& pNewDesc, tukk varName, const TUIData& value, tukk pTmplName = NULL) = 0;
	virtual bool                  CreateVariable(const SUIParameterDesc*& pNewDesc, tukk varName, const TUIData& value, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual bool                  SetArray(tukk arrayName, const SUIArguments& values, tukk pTmplName = NULL) = 0;
	virtual bool                  SetArray(const SUIParameterDesc* pArrayDesc, const SUIArguments& values, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual bool                  GetArray(tukk arrayName, SUIArguments& valuesOut, tukk pTmplName = NULL) = 0;
	virtual bool                  GetArray(const SUIParameterDesc* pArrayDesc, SUIArguments& valuesOut, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual bool                  CreateArray(const SUIParameterDesc*& pNewDesc, tukk arrayName, const SUIArguments& values, tukk pTmplName = NULL) = 0;
	virtual bool                  CreateArray(const SUIParameterDesc*& pNewDesc, tukk arrayName, const SUIArguments& values, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	template<class T>
	inline bool SetVar(tukk varName, const T& value, tukk pTmplName = NULL)
	{
		return SetVariable(varName, TUIData(value), pTmplName);
	}

	template<class T>
	inline T GetVar(tukk varName, tukk pTmplName = NULL)
	{
		TUIData out;
		if (GetVariable(varName, out, pTmplName))
		{
			T res;
			if (out.GetValueWithConversion(res))
				return res;
		}
		assert(false);
		return T();
	}

	// ITexture
	virtual void LoadTexIntoMc(tukk movieClip, ITexture* pTexture, tukk pTmplName = NULL) = 0;
	virtual void LoadTexIntoMc(const SUIParameterDesc* pMovieClipDesc, ITexture* pTexture, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	virtual void UnloadTexFromMc(tukk movieClip, ITexture* pTexture, tukk pTmplName = NULL) = 0;
	virtual void UnloadTexFromMc(const SUIParameterDesc* pMovieClipDesc, ITexture* pTexture, const SUIMovieClipDesc* pTmplDesc = NULL) = 0;

	//! Converts screenpos (0-1) to flash pos.
	virtual void ScreenToFlash(const float& px, const float& py, float& rx, float& ry, bool bStageScaleMode = false) const = 0;

	//! Converts world position to flash position.
	virtual void WorldToFlash(const Matrix34& camMat, const Vec3& worldpos, Vec3& flashpos, Vec2& borders, float& scale, bool bStageScaleMode = false) const = 0;

	//! Dynamic textures.
	virtual void AddTexture(IDynTextureSource* pDynTexture) = 0;
	virtual void RemoveTexture(IDynTextureSource* pDynTexture) = 0;
	virtual i32  GetNumExtTextures() const = 0;
	virtual bool GetDynTexSize(i32& width, i32& height) const = 0;

	//! Input events.
	virtual void                                            SendCursorEvent(SFlashCursorEvent::ECursorState evt, i32 iX, i32 iY, i32 iButton = 0, float fWheel = 0.f) = 0;
	virtual void                                            SendKeyEvent(const SFlashKeyEvent& evt) = 0;
	virtual void                                            SendCharEvent(const SFlashCharEvent& charEvent) = 0;
	virtual void                                            SendControllerEvent(EControllerInputEvent event, EControllerInputState state, float value) = 0;

	virtual void                                            GetMemoryUsage(IDrxSizer* s) const = 0;

	virtual void                                            RemoveFlashVarObj(const SUIParameterDesc* pDesc) = 0;
};
template<> inline tukk SUIItemLookupIDD<IUIElement >::GetId(const IUIElement* p) { return p->GetName(); }

////////////////////////////////////////////// UI Action ///////////////////////////////////////////

struct IUIAction
{
	enum EUIActionType
	{
		eUIAT_FlowGraph,
		eUIAT_LuaScript,
	};

	virtual ~IUIAction() {}

	virtual IUIAction::EUIActionType                       GetType() const = 0;

	virtual tukk                                    GetName() const = 0;
	virtual void                                           SetName(tukk name) = 0;

	virtual bool                                           Init() = 0;
	virtual bool                                           IsValid() const = 0;

	virtual void                                           SetEnabled(bool bEnabled) = 0;
	virtual bool                                           IsEnabled() const = 0;

	virtual IFlowGraphPtr                                  GetFlowGraph() const = 0;
	virtual SmartScriptTable                               GetScript() const = 0;

	virtual bool                                           Serialize(XmlNodeRef& xmlNode, bool bIsLoading) = 0;
	virtual bool                                           Serialize(tukk scriptFile, bool bIsLoading) = 0;

	virtual void                                           GetMemoryUsage(IDrxSizer* s) const = 0;
};
template<> inline tukk SUIItemLookupIDD<IUIAction >::GetId(const IUIAction* p) { return p->GetName(); }

struct IUIActionListener
{
	virtual void OnStart(IUIAction* pAction, const SUIArguments& args) = 0;
	virtual void OnEnd(IUIAction* pAction, const SUIArguments& args) = 0;
protected:
	virtual ~IUIActionListener() {}
};

struct IUIActionUpr
{
	virtual ~IUIActionUpr() {}

	virtual void StartAction(IUIAction* pAction, const SUIArguments& args) = 0;
	virtual void EndAction(IUIAction* pAction, const SUIArguments& args) = 0;
	virtual void EnableAction(IUIAction* pAction, bool bEnable) = 0;

	virtual void AddListener(IUIActionListener* pListener, tukk name) = 0;
	virtual void RemoveListener(IUIActionListener* pListener) = 0;

	virtual void GetMemoryUsage(IDrxSizer* s) const = 0;
};

////////////////////////////////////////////// UI Events ///////////////////////////////////////////

struct SUIEvent
{
	SUIEvent(uint evt, SUIArguments agruments) : event(evt), args(agruments) {}
	SUIEvent(uint evt) : event(evt) {}
	uint         event;
	SUIArguments args;
};

struct IUIEventSystem;
struct IUIEventListener
{
	virtual SUIArgumentsRet OnEvent(const SUIEvent& event) = 0;
	virtual void            OnEventSystemDestroyed(IUIEventSystem* pEventSystem) = 0;
protected:
	virtual ~IUIEventListener() {}
};

struct IUIEventSystem
{
	enum EEventSystemType
	{
		eEST_UI_TO_SYSTEM,
		eEST_SYSTEM_TO_UI,
	};

	virtual ~IUIEventSystem() {}

	virtual tukk                      GetName() const = 0;
	virtual IUIEventSystem::EEventSystemType GetType() const = 0;

	virtual uint                             RegisterEvent(const SUIEventDesc& eventDesc) = 0;

	virtual void                             RegisterListener(IUIEventListener* pListener, tukk name) = 0;
	virtual void                             UnregisterListener(IUIEventListener* pListener) = 0;

	virtual SUIArgumentsRet                  SendEvent(const SUIEvent& event) = 0;

	virtual const SUIEventDesc*              GetEventDesc(i32 index) const = 0;
	virtual const SUIEventDesc*              GetEventDesc(tukk eventName) const = 0;
	virtual i32                              GetEventCount() const = 0;

	virtual uint                             GetEventId(tukk sEventName) = 0;

	virtual void                             GetMemoryUsage(IDrxSizer* s) const = 0;
};

struct IUIEventSystemIterator
{
	virtual ~IUIEventSystemIterator() {}

	virtual void            AddRef() = 0;
	virtual void            Release() = 0;
	virtual IUIEventSystem* Next(string& name) = 0;
};

TYPEDEF_AUTOPTR(IUIEventSystemIterator);
typedef IUIEventSystemIterator_AutoPtr IUIEventSystemIteratorPtr;

/////////////////////////////////////////// UI Interface ///////////////////////////////////////////

struct IVirtualKeyboardEvents;

struct IUIModule
{
	virtual ~IUIModule() {};

	//! Called once on initialization of the UISystem.
	virtual void Init() {};

	//! Called once on shutdown of the UISystem.
	virtual void Shutdown() {};

	//! Called if gfx_reload_all command was issued.
	virtual void Reload() {};

	//! Called on FlashUI reset (unload level etc.)
	virtual void Reset() {};

	//! Called on FlashUI update.
	virtual void UpdateModule(float fDelta) {};

	/////////////////////////////////////////////////////////
	// Sandbox only.
	//! Called before reloading is issued, if return false reloading will not happen.
	virtual bool EditorAllowReload() { return true; }

	//! Called on reload all.
	virtual void EditorReload() {};
	/////////////////////////////////////////////////////////
};

struct IFlashUI : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IFlashUI, "e1161004-da5b-4f04-9dff-8fc0eace3bd4"_drx_guid);

public:
	//! Init the Flash UI system.
	virtual void Init() = 0;
	virtual bool PostInit() = 0;

	//! Update the ui system.
	virtual void Update(float fDeltatime) = 0;

	//! Reload UI xml files.
	virtual void Reload() = 0;

	//! Clear UI Actions
	virtual void ClearUIActions() = 0;

	//! Shut down.
	virtual void Shutdown() = 0;

	virtual bool LoadElementsFromFile(tukk fileName) = 0;
	virtual bool LoadActionFromFile(tukk sFileName, IUIAction::EUIActionType type) = 0;

	//! Access for IUIElements.
	virtual IUIElement* GetUIElement(tukk name) const = 0;
	virtual IUIElement* GetUIElement(i32 index) const = 0;
	virtual i32         GetUIElementCount() const = 0;

	virtual                        IUIElement*  GetUIElementByInstanceStr(tukk UIInstanceStr) const = 0;
	virtual std::pair<IUIElement*, IUIElement*> GetUIElementsByInstanceStr(tukk UIInstanceStr) const = 0;
	virtual std::pair<string, i32>              GetUIIdentifiersByInstanceStr(tukk sUIInstanceStr) const = 0;

	//! Access for IUIActions.
	virtual IUIAction*        GetUIAction(tukk name) const = 0;
	virtual IUIAction*        GetUIAction(i32 index) const = 0;
	virtual i32               GetUIActionCount() const = 0;

	virtual IUIActionUpr* GetUIActionUpr() const = 0;

	//! Updates all UIAction flowgraphs.
	//! Will update all FGs in a loop until all event stacks are flushed.
	virtual void UpdateFG() = 0;

	virtual void RegisterModule(IUIModule* pModule, tukk name) = 0;
	virtual void UnregisterModule(IUIModule* pModule) = 0;

	virtual void SetHudElementsVisible(bool bVisible) = 0;

	//! Only valid in editor, also only used by UI Editor to prevent stack overflow while FG is not updated.
	virtual void EnableEventStack(bool bEnable) = 0;

	//! Event system to auto create flownodes for communication between flash and c++.
	virtual IUIEventSystem*           CreateEventSystem(tukk name, IUIEventSystem::EEventSystemType eType) = 0;
	virtual IUIEventSystem*           GetEventSystem(tukk name, IUIEventSystem::EEventSystemType eType) = 0;
	virtual IUIEventSystemIteratorPtr CreateEventSystemIterator(IUIEventSystem::EEventSystemType eType) = 0;

	//! Input events.
	virtual void DispatchControllerEvent(IUIElement::EControllerInputEvent event, IUIElement::EControllerInputState state, float value) = 0;
	virtual void SendFlashMouseEvent(SFlashCursorEvent::ECursorState evt, i32 iX, i32 iY, i32 iButton = 0, float fWheel = 0.f, bool bFromController = false) = 0;
	virtual bool DisplayVirtualKeyboard(u32 flags, tukk title, tukk initialInput, i32 maxInputLength, IVirtualKeyboardEvents* pInCallback) = 0;
	virtual bool IsVirtualKeyboardRunning() = 0;
	virtual bool CancelVirtualKeyboard() = 0;

	//! Used by scaleform to validate if texture is preloaded (only debug/profile).
	virtual void CheckPreloadedTexture(ITexture* pTexture) const = 0;

	//! Used by the renderer to check if an UIElement should be used on dyn texture.
	virtual bool UseSharedRT(tukk instanceStr, bool defVal) const = 0;

	//! Memory statistics.
	virtual void GetMemoryStatistics(IDrxSizer* s) const = 0;

	//! Screen size stuff for UI Emulator (Sandbox) only!
	virtual void GetScreenSize(i32& width, i32& height) = 0;
	typedef Functor2<i32&, i32&> TEditorScreenSizeCallback;
	virtual void SetEditorScreenSizeCallback(TEditorScreenSizeCallback& cb) = 0;
	virtual void RemoveEditorScreenSizeCallback() = 0;

	//! Platform stuff for UI Emulator (Sandbox) only!
	enum EPlatformUI
	{
		ePUI_PC = 0,
		ePUI_Durango,
		ePUI_Orbis,
		ePUI_Android,
	};
	typedef Functor0wRet<EPlatformUI> TEditorPlatformCallback;
	virtual void SetEditorPlatformCallback(TEditorPlatformCallback& cb) = 0;
	virtual void RemoveEditorPlatformCallback() = 0;

	//! Logging listener for UI Emulator (Sandbox) only!
	enum ELogEventLevel
	{
		eLEL_Log = 0,
		eLEL_Warning,
		eLEL_Error,
	};

	struct SUILogEvent
	{
		string         Message;
		ELogEventLevel Level;
	};

	typedef Functor1<const SUILogEvent&> TEditorUILogEventCallback;
	virtual void SetEditorUILogEventCallback(TEditorUILogEventCallback& cb) = 0;
	virtual void RemoveEditorUILogEventCallback() = 0;

#if !defined(_LIB)
	virtual SUIItemLookupSet_Impl<SUIParameterDesc>* CreateLookupParameter() = 0;
	virtual SUIItemLookupSet_Impl<SUIMovieClipDesc>* CreateLookupMovieClip() = 0;
	virtual SUIItemLookupSet_Impl<SUIEventDesc>*     CreateLookupEvent() = 0;
#endif
};

#if !defined(_LIB)
inline SUIItemLookupSet_Impl<SUIParameterDesc>* SUIItemLookupSetFactory::CreateLookupParameter() { assert(gEnv->pFlashUI); return gEnv->pFlashUI->CreateLookupParameter(); }
inline SUIItemLookupSet_Impl<SUIMovieClipDesc>* SUIItemLookupSetFactory::CreateLookupMovieClip() { assert(gEnv->pFlashUI); return gEnv->pFlashUI->CreateLookupMovieClip(); }
inline SUIItemLookupSet_Impl<SUIEventDesc>*     SUIItemLookupSetFactory::CreateLookupEvent()     { assert(gEnv->pFlashUI); return gEnv->pFlashUI->CreateLookupEvent(); }
#endif

DECLARE_SHARED_POINTERS(IFlashUI);

static IFlashUIPtr GetIFlashUIPtr()
{
	IFlashUIPtr pFlashUI;
	if (gEnv && gEnv->pSystem)
		DrxCreateClassInstanceForInterface<IFlashUI>(drxiidof<IFlashUI>(), pFlashUI);
	return pFlashUI;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
enum EUIObjectType
{
	eUOT_Variable,
	eUOT_Array,
	eUOT_MovieClip,
	eUOT_MovieClipTmpl,
	eUOT_Functions,
	eUOT_Events,
};

//! \cond INTERNAL
template<EUIObjectType type> struct SUIDescTypeOf
{
	typedef i32 TType;
};                                                                           //!< Dummy type.
template<> struct SUIDescTypeOf<eUOT_Variable>
{
	typedef SUIParameterDesc TType;
};
template<> struct SUIDescTypeOf<eUOT_Array>
{
	typedef SUIParameterDesc TType;
};
template<> struct SUIDescTypeOf<eUOT_MovieClip>
{
	typedef SUIMovieClipDesc TType;
};
template<> struct SUIDescTypeOf<eUOT_MovieClipTmpl>
{
	typedef SUIMovieClipDesc TType;
};
template<> struct SUIDescTypeOf<eUOT_Functions>
{
	typedef SUIEventDesc TType;
};
template<> struct SUIDescTypeOf<eUOT_Events>
{
	typedef SUIEventDesc TType;
};

template<EUIObjectType type, class Item, class Idx> struct SUIGetDesc
{
	static const typename SUIDescTypeOf<type>::TType * GetDesc(Item * pItem, Idx i)
	{
		assert(false);
		return NULL;
	} static i32 GetCount(Item* pItem)
	{
		assert(false);
		return 0;
	}
};
template<EUIObjectType type, class Idx> struct SUIGetDesc<type, const SUIParameterDesc, Idx>
{
	static const typename SUIDescTypeOf<type>::TType * GetDesc(const SUIParameterDesc * pItem, Idx idx)
	{
		return NULL;
	} static i32 GetCount(const SUIParameterDesc* pItem)
	{
		return 0;
	}
};
template<class Item, class Idx> struct SUIGetDesc<eUOT_Variable, Item, Idx>
{
	static const typename SUIDescTypeOf<eUOT_Variable>::TType * GetDesc(Item * pItem, Idx idx)
	{
		return pItem->GetVariableDesc(idx);
	}  static i32 GetCount(Item* pItem)
	{
		return pItem->GetVariableCount();
	}
};
template<class Item, class Idx> struct SUIGetDesc<eUOT_Array, Item, Idx>
{
	static const typename SUIDescTypeOf<eUOT_Array>::TType * GetDesc(Item * pItem, Idx idx)
	{
		return pItem->GetArrayDesc(idx);
	}  static i32 GetCount(Item* pItem)
	{
		return pItem->GetArrayCount();
	}
};
template<class Item, class Idx> struct SUIGetDesc<eUOT_MovieClip, Item, Idx>
{
	static const typename SUIDescTypeOf<eUOT_MovieClip>::TType * GetDesc(Item * pItem, Idx idx)
	{
		return pItem->GetMovieClipDesc(idx);
	}  static i32 GetCount(Item* pItem)
	{
		return pItem->GetMovieClipCount();
	}
};
template<class Item, class Idx> struct SUIGetDesc<eUOT_Functions, Item, Idx>
{
	static const typename SUIDescTypeOf<eUOT_Functions>::TType * GetDesc(Item * pItem, Idx idx)
	{
		return pItem->GetFunctionDesc(idx);
	}  static i32 GetCount(Item* pItem)
	{
		return pItem->GetFunctionCount();
	}
};
template<class Item, class Idx> struct SUIGetDesc<eUOT_Events, Item, Idx>
{
	static const typename SUIDescTypeOf<eUOT_Events>::TType * GetDesc(Item * pItem, Idx idx)
	{
		return pItem->GetEventDesc(idx);
	}  static i32 GetCount(Item* pItem)
	{
		return pItem->GetEventCount();
	}
};
template<class Idx> struct SUIGetDesc<eUOT_MovieClipTmpl, const SUIMovieClipDesc, Idx>
{
	static const typename SUIDescTypeOf<eUOT_MovieClipTmpl>::TType * GetDesc(const SUIMovieClipDesc * pItem, Idx idx)
	{
		return NULL;
	}  static i32 GetCount(const SUIMovieClipDesc* pItem)
	{
		return 0;
	}
};
template<class Idx> struct SUIGetDesc<eUOT_MovieClipTmpl, IUIElement, Idx>
{
	static const typename SUIDescTypeOf<eUOT_MovieClipTmpl>::TType * GetDesc(IUIElement * pItem, Idx idx)
	{
		return pItem->GetMovieClipTmplDesc(idx);
	}  static i32 GetCount(IUIElement* pItem)
	{
		return pItem->GetMovieClipTmplCount();
	}
};
template<class Idx> struct SUIGetDesc<eUOT_MovieClipTmpl, const IUIElement, Idx>
{
	static const typename SUIDescTypeOf<eUOT_MovieClipTmpl>::TType * GetDesc(const IUIElement * pItem, Idx idx)
	{
		return pItem->GetMovieClipTmplDesc(idx);
	}  static i32 GetCount(const IUIElement* pItem)
	{
		return pItem->GetMovieClipTmplCount();
	}
};

template<EUIObjectType type> struct SUIGetTypeStr
{
	static tukk GetTypeName()
	{
		return "UNDEFINED";
	}
};
template<> struct SUIGetTypeStr<eUOT_Variable>
{
	static tukk GetTypeName()
	{
		return "variable";
	}
};
template<> struct SUIGetTypeStr<eUOT_Array>
{
	static tukk GetTypeName()
	{
		return "array";
	}
};
template<> struct SUIGetTypeStr<eUOT_MovieClip>
{
	static tukk GetTypeName()
	{
		return "movieclip";
	}
};
template<> struct SUIGetTypeStr<eUOT_MovieClipTmpl>
{
	static tukk GetTypeName()
	{
		return "movieclip template";
	}
};
template<> struct SUIGetTypeStr<eUOT_Functions>
{
	static tukk GetTypeName()
	{
		return "function";
	}
};
template<> struct SUIGetTypeStr<eUOT_Events>
{
	static tukk GetTypeName()
	{
		return "event";
	}
};
//! \endcond

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// UIEvent Dispatch helper //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(_DEBUG) || !defined(NDEBUG)
	#define UIEVENT_CHECK_RETURN_VALUE(type) \
	  case SUIParameterDesc::eUIPT_ ## type: \
	    DRX_ASSERT_MESSAGE(r.GetArg(i).GetType() == eUIDT_ ## type, "Argument not compatible!"); break;

	#define UIEVENT_RETURN_DISPATCH_SAFE                                                                                                                     \
	  SUIArguments r = it->second->Dispatch((uk ) pThis, event);                                                                                           \
	  const SUIEventDesc* pEventDesc = m_pEventSystem->GetEventDesc(it->first);                                                                              \
	  DRX_ASSERT_MESSAGE(pEventDesc, "No matching event registered!");                                                                                       \
	  DRX_ASSERT_MESSAGE(pEventDesc->OutputParams.IsDynamic || pEventDesc->OutputParams.Params.size() == r.GetArgCount(), "Wrong number of return values!"); \
	  if (!pEventDesc->OutputParams.IsDynamic) {                                                                                                             \
	    for (i32 i = 0; i < r.GetArgCount(); ++i) {                                                                                                          \
	      switch (pEventDesc->OutputParams.Params[i].eType) {                                                                                                \
	        UIEVENT_CHECK_RETURN_VALUE(Bool)                                                                                                                 \
	        UIEVENT_CHECK_RETURN_VALUE(Int)                                                                                                                  \
	        UIEVENT_CHECK_RETURN_VALUE(Float)                                                                                                                \
	        UIEVENT_CHECK_RETURN_VALUE(Vec3)                                                                                                                 \
	        UIEVENT_CHECK_RETURN_VALUE(String)                                                                                                               \
	        UIEVENT_CHECK_RETURN_VALUE(WString)                                                                                                              \
	      }                                                                                                                                                  \
	    }                                                                                                                                                    \
	  }                                                                                                                                                      \
	  return r;

#else
	#define UIEVENT_RETURN_DISPATCH_SAFE return it->second->Dispatch((uk ) pThis, event);
#endif

#define UIEVENT_GETARGSAVE(index)                             \
  deref_t<T ## index> arg ## index;                           \
  { const bool ok = event.args.GetArg(index, arg ## index.v); \
    DRX_ASSERT_MESSAGE(ok, "Value is not compatible for arg " # index); }

#define UIEVENT_CHECKARGCOUNT(count) \
  DRX_ASSERT_MESSAGE(event.args.GetArgCount() == count, "Wrong argument count, expected " # count);

#define UIEVENT_ASSERT_COUNT(count) \
  DRX_ASSERT_MESSAGE(event.InputParams.Params.size() == count, "Wrong argument count, expected " # count);

#define UIEVENT_ASSERT_ARG(index) \
  DRX_ASSERT_MESSAGE(SUIEventArgumentCheck<T ## index>::Check(event.InputParams.Params[index].eType), "Template argument not compatible! Index: " # index);

//! \cond INTERNAL
//! Deref for T&, const T& and special case for tukk and const wchar_t*.
template<class T> struct deref_t
{
	typedef T type;
	type v;
	T    g()
	{
		return v;
	}
};
template<class T> struct deref_t<T&>
{
	typedef T type;
	type v;
	T&   g()
	{
		return v;
	}
};
template<class T> struct deref_t<const T&>
{
	typedef T type;
	type     v;
	const T& g()
	{
		return v;
	}
};
template<class T> struct deref_t<const T*>
{
	typedef DrxStringT<T> type;
	type v;
	const T* g()
	{
		return v.c_str();
	}
};
//! \endcond

// Argument check
template<class T> struct SUIEventArgumentCheck
{
	static inline bool Check(SUIParameterDesc::EUIParameterType type)
	{
		return false;
	}
};
template<class T> struct SUIEventArgumentCheck<T&>
{
	static inline bool Check(SUIParameterDesc::EUIParameterType type)
	{
		return SUIEventArgumentCheck<T>::Check(type);
	}
};
template<class T> struct SUIEventArgumentCheck<const T&>
{
	static inline bool Check(SUIParameterDesc::EUIParameterType type)
	{
		return SUIEventArgumentCheck<T>::Check(type);
	}
};
template<> inline bool SUIEventArgumentCheck<bool          >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_Bool; }
template<> inline bool SUIEventArgumentCheck<EntityId      >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_Int; }
template<> inline bool SUIEventArgumentCheck<i32           >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_Int; }
template<> inline bool SUIEventArgumentCheck<float         >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_Float; }
template<> inline bool SUIEventArgumentCheck<Vec3          >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_Vec3; }
template<> inline bool SUIEventArgumentCheck<Ang3          >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_Vec3; }
template<> inline bool SUIEventArgumentCheck<tukk   >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_String; }
template<> inline bool SUIEventArgumentCheck<const wchar_t*>::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_WString; }
template<> inline bool SUIEventArgumentCheck<string        >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_String; }
template<> inline bool SUIEventArgumentCheck<wstring       >::Check(SUIParameterDesc::EUIParameterType type) { return type == SUIParameterDesc::eUIPT_WString; }
template<> inline bool SUIEventArgumentCheck<TUIData       >::Check(SUIParameterDesc::EUIParameterType type) { return true; }

//! \cond INTERNAL
//! Dispatcher function interface.
struct IUIEventDispatchFct
{
	virtual ~IUIEventDispatchFct() {};
	virtual SUIArgumentsRet Dispatch(uk pThis, const SUIEvent& event) = 0;
};

template<class R, class C>
struct SUIEventDispatchFctReturn
{
	static inline SUIArgumentsRet Dispatch(C& impl, uk pThis, const SUIEvent& event)
	{
		return impl.Dispatch(pThis, event);
	}
};

template<class C>
struct SUIEventDispatchFctReturn<void, C>
{
	static inline SUIArgumentsRet Dispatch(C& impl, uk pThis, const SUIEvent& event)
	{
		impl.Dispatch(pThis, event);
		return SUIArguments();
	}
};

template<class R, class C>
struct SUIEventDispatchFctImpl : public IUIEventDispatchFct
{
	SUIEventDispatchFctImpl(typename C::TFct fct) : Impl(fct) {}
	virtual ~SUIEventDispatchFctImpl() {};
	virtual SUIArgumentsRet Dispatch(uk pThis, const SUIEvent& event) { return SUIEventDispatchFctReturn<R, C>::Dispatch(Impl, pThis, event); }
	C Impl;
};

//! Dispatcher functions.
template<class C, class R>
struct SUIEventDispatchFct0
{
	typedef R (C::* TFct) (void);
	SUIEventDispatchFct0(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event) { return (((C*)pThis)->*m_fct)(); }
	TFct m_fct;
};

template<class C, class R, class T0>
struct SUIEventDispatchFct1
{
	typedef R (C::* TFct) (T0);
	SUIEventDispatchFct1(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(1);
		UIEVENT_GETARGSAVE(0);
		return (((C*)pThis)->*m_fct)(arg0.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1>
struct SUIEventDispatchFct2
{
	typedef R (C::* TFct) (T0, T1);
	SUIEventDispatchFct2(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(2);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2>
struct SUIEventDispatchFct3
{
	typedef R (C::* TFct) (T0, T1, T2);
	SUIEventDispatchFct3(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(3);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2, class T3>
struct SUIEventDispatchFct4
{
	typedef R (C::* TFct) (T0, T1, T2, T3);
	SUIEventDispatchFct4(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(4);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		UIEVENT_GETARGSAVE(3);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g(), arg3.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2, class T3, class T4>
struct SUIEventDispatchFct5
{
	typedef R (C::* TFct) (T0, T1, T2, T3, T4);
	SUIEventDispatchFct5(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(5);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		UIEVENT_GETARGSAVE(3);
		UIEVENT_GETARGSAVE(4);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g(), arg3.g(), arg4.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2, class T3, class T4, class T5>
struct SUIEventDispatchFct6
{
	typedef R (C::* TFct) (T0, T1, T2, T3, T4, T5);
	SUIEventDispatchFct6(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(6);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		UIEVENT_GETARGSAVE(3);
		UIEVENT_GETARGSAVE(4);
		UIEVENT_GETARGSAVE(5);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g(), arg3.g(), arg4.g(), arg5.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2, class T3, class T4, class T5, class T6>
struct SUIEventDispatchFct7
{
	typedef R (C::* TFct) (T0, T1, T2, T3, T4, T5, T6);
	SUIEventDispatchFct7(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(7);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		UIEVENT_GETARGSAVE(3);
		UIEVENT_GETARGSAVE(4);
		UIEVENT_GETARGSAVE(5);
		UIEVENT_GETARGSAVE(6);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g(), arg3.g(), arg4.g(), arg5.g(), arg6.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
struct SUIEventDispatchFct8
{
	typedef R (C::* TFct) (T0, T1, T2, T3, T4, T5, T6, T7);
	SUIEventDispatchFct8(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(8);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		UIEVENT_GETARGSAVE(3);
		UIEVENT_GETARGSAVE(4);
		UIEVENT_GETARGSAVE(5);
		UIEVENT_GETARGSAVE(6);
		UIEVENT_GETARGSAVE(7);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g(), arg3.g(), arg4.g(), arg5.g(), arg6.g(), arg7.g());
	}
	TFct m_fct;
};

template<class C, class R, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
struct SUIEventDispatchFct9
{
	typedef R (C::* TFct) (T0, T1, T2, T3, T4, T5, T6, T7, T8);
	SUIEventDispatchFct9(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event)
	{
		UIEVENT_CHECKARGCOUNT(9);
		UIEVENT_GETARGSAVE(0);
		UIEVENT_GETARGSAVE(1);
		UIEVENT_GETARGSAVE(2);
		UIEVENT_GETARGSAVE(3);
		UIEVENT_GETARGSAVE(4);
		UIEVENT_GETARGSAVE(5);
		UIEVENT_GETARGSAVE(6);
		UIEVENT_GETARGSAVE(7);
		UIEVENT_GETARGSAVE(8);
		return (((C*)pThis)->*m_fct)(arg0.g(), arg1.g(), arg2.g(), arg3.g(), arg4.g(), arg5.g(), arg6.g(), arg7.g(), arg8.g());
	}
	TFct m_fct;
};

//! Special cases for legacy and dynamic arg count.
template<class C, class R>
struct SUIEventDispatchFct1<C, R, const SUIEvent&>
{
	typedef R (C::* TFct) (const SUIEvent&);
	SUIEventDispatchFct1(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event) { return (((C*)pThis)->*m_fct)(event); }
	TFct m_fct;
};

template<class C, class R>
struct SUIEventDispatchFct1<C, R, const SUIArguments&>
{
	typedef R (C::* TFct) (const SUIArguments&);
	SUIEventDispatchFct1(TFct fct) : m_fct(fct) {}
	R Dispatch(uk pThis, const SUIEvent& event) { return (((C*)pThis)->*m_fct)(event.args); }
	TFct m_fct;
};

//! Helper to dispatch received events to callback functions with variable arguments and arg count.
template<class C>
struct SUIEventReceiverDispatcher : public IUIEventListener
{
	SUIEventReceiverDispatcher() : m_pEventSystem(NULL), m_pThis(NULL) {}

	//! If init with valid pThis pointer the using class don't need to implement IUIEventListener, the dispatcher will auto dispatch!
	void Init(IUIEventSystem* pEventSystem, C* pThis = NULL, tukk listenerName = "SUIEventReceiverDispatcher")
	{
		DRX_ASSERT_MESSAGE(!m_pEventSystem, "Already initialized!");
		m_pEventSystem = pEventSystem;
		m_pThis = pThis;
		if (pThis)
			m_pEventSystem->RegisterListener(this, listenerName);
	}

	template<class R>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(void))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(0);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct0<C, R>>(fct);
	}

	template<class R, class T0>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(1);
		UIEVENT_ASSERT_ARG(0);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct1<C, R, T0>>(fct);
	}

	template<class R, class T0, class T1>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(2);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct2<C, R, T0, T1>>(fct);
	}

	template<class R, class T0, class T1, class T2>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(3);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct3<C, R, T0, T1, T2>>(fct);
	}

	template<class R, class T0, class T1, class T2, class T3>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2, T3))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(4);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		UIEVENT_ASSERT_ARG(3);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct4<C, R, T0, T1, T2, T3>>(fct);
	}

	template<class R, class T0, class T1, class T2, class T3, class T4>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2, T3, T4))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(5);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		UIEVENT_ASSERT_ARG(3);
		UIEVENT_ASSERT_ARG(4);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct5<C, R, T0, T1, T2, T3, T4>>(fct);
	}

	template<class R, class T0, class T1, class T2, class T3, class T4, class T5>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2, T3, T4, T5))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(6);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		UIEVENT_ASSERT_ARG(3);
		UIEVENT_ASSERT_ARG(4);
		UIEVENT_ASSERT_ARG(5);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct6<C, R, T0, T1, T2, T3, T4, T5>>(fct);
	}

	template<class R, class T0, class T1, class T2, class T3, class T4, class T5, class T6>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2, T3, T4, T5, T6))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(7);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		UIEVENT_ASSERT_ARG(3);
		UIEVENT_ASSERT_ARG(4);
		UIEVENT_ASSERT_ARG(5);
		UIEVENT_ASSERT_ARG(6);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct7<C, R, T0, T1, T2, T3, T4, T5, T6>>(fct);
	}

	template<class R, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2, T3, T4, T5, T6, T7))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(8);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		UIEVENT_ASSERT_ARG(3);
		UIEVENT_ASSERT_ARG(4);
		UIEVENT_ASSERT_ARG(5);
		UIEVENT_ASSERT_ARG(6);
		UIEVENT_ASSERT_ARG(7);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct8<C, R, T0, T1, T2, T3, T4, T5, T6, T7>>(fct);
	}

	template<class R, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(T0, T1, T2, T3, T4, T5, T6, T7, T8))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		UIEVENT_ASSERT_COUNT(9);
		UIEVENT_ASSERT_ARG(0);
		UIEVENT_ASSERT_ARG(1);
		UIEVENT_ASSERT_ARG(2);
		UIEVENT_ASSERT_ARG(3);
		UIEVENT_ASSERT_ARG(4);
		UIEVENT_ASSERT_ARG(5);
		UIEVENT_ASSERT_ARG(6);
		UIEVENT_ASSERT_ARG(7);
		UIEVENT_ASSERT_ARG(8);
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct9<C, R, T0, T1, T2, T3, T4, T5, T6, T7, T8>>(fct);
	}

	//! Special cases for legacy and dyn arg count.
	template<class R>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(const SUIEvent&))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct1<C, R, const SUIEvent&>>(fct);
	}

	template<class R>
	inline void RegisterEvent(const SUIEventDesc& event, R (C::* fct)(const SUIArguments&))
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventReceiver not initialized!");
		mFunctionMap[m_pEventSystem->RegisterEvent(event)] = new SUIEventDispatchFctImpl<R, SUIEventDispatchFct1<C, R, const SUIArguments&>>(fct);
	}

	inline SUIArgumentsRet Dispatch(C* pThis, const SUIEvent& event)
	{
		DRX_ASSERT_MESSAGE(pThis, "pThis pointer not valid!");
		TFunctionMapIter it = mFunctionMap.find(event.event);
		if (it != mFunctionMap.end())
		{
			UIEVENT_RETURN_DISPATCH_SAFE;
		}
		return SUIArguments();
	}

	// IUIEventListener
	virtual SUIArgumentsRet OnEvent(const SUIEvent& event)
	{
		return Dispatch(m_pThis, event);
	}

	virtual void OnEventSystemDestroyed(IUIEventSystem* pEventSystem)
	{
		m_pEventSystem = NULL;
	}

	virtual ~SUIEventReceiverDispatcher()
	{
		TFunctionMapIter it = mFunctionMap.begin();
		TFunctionMapIter end = mFunctionMap.end();
		for (; it != end; ++it) delete it->second;
		if (m_pThis && m_pEventSystem)
			m_pEventSystem->UnregisterListener(this);
	}

private:
	typedef std::map<uint, IUIEventDispatchFct*> TFunctionMap;
	typedef typename TFunctionMap::iterator      TFunctionMapIter;
	TFunctionMap    mFunctionMap;
	IUIEventSystem* m_pEventSystem;
	C*              m_pThis;
};

/////////////////////////////////////// UIEvent Send helper ////////////////////////////////////////

#if defined(_DEBUG) || !defined(NDEBUG)
	#define UIEVENT_GETINFO                                                       \
	  const SUIEventDesc * pEventDesc = m_pEventSystem->GetEventDesc(it->second); \
	  DRX_ASSERT_MESSAGE(pEventDesc, "No matching event registered!");

	#define UIEVENT_CHECKARGCOUNT_SEND(count)                                                           \
	  DRX_ASSERT_MESSAGE(pEventDesc->InputParams.Params.size() == count, "Wrong number of arguments!"); \
	  DRX_ASSERT_MESSAGE(pEventDesc->OutputParams.Params.size() == 0, "Output params not allowed!");

	#define UIEVENT_ADDARGSAFE(index)                                                                                                                        \
	  DRX_ASSERT_MESSAGE(SUIEventArgumentCheck<T ## index>::Check(pEventDesc->InputParams.Params[index].eType), "Argument not compatible! Index: " # index); \
	  event.args.AddArgument(arg ## index);

	#define UIEVENT_CHECKDYNARGS \
	  DRX_ASSERT_MESSAGE(pEventDesc->InputParams.IsDynamic || pEventDesc->InputParams.Params.size() == args.GetArgCount(), "Wrong number of arguments!");

#else
	#define UIEVENT_GETINFO
	#define UIEVENT_CHECKARGCOUNT_SEND(count)
	#define UIEVENT_ADDARGSAFE(index) event.args.AddArgument(arg ## index);
	#define UIEVENT_CHECKDYNARGS
#endif

#define UIEVENT_GETEVENT                                              \
  DRX_ASSERT_MESSAGE(m_pEventSystem, "EventSender not initialized!"); \
  TEventMapIter it = m_EventMap.find(type);                           \
  DRX_ASSERT_MESSAGE(it != m_EventMap.end(), "Unknown event!");       \
  UIEVENT_GETINFO;                                                    \
  SUIEvent event(it->second);

#define UIEVENT_GETEVENT_DYN                                          \
  DRX_ASSERT_MESSAGE(m_pEventSystem, "EventSender not initialized!"); \
  TEventMapIter it = m_EventMap.find(type);                           \
  DRX_ASSERT_MESSAGE(it != m_EventMap.end(), "Unknown event!");       \
  UIEVENT_GETINFO;                                                    \
  SUIEvent event(it->second, args);

//! Helper to send events with variable arguments and arg count.
template<class T>
struct SUIEventSenderDispatcher
{
	SUIEventSenderDispatcher() : m_pEventSystem(NULL) {}

	void Init(IUIEventSystem* pEventSystem)
	{
		DRX_ASSERT_MESSAGE(!m_pEventSystem && m_EventMap.size() == 0, "Already initialized!");
		m_pEventSystem = pEventSystem;
	}

	template<T type>
	inline void RegisterEvent(const SUIEventDesc& event)
	{
		DRX_ASSERT_MESSAGE(m_pEventSystem, "EventSender not initialized!");
		DRX_ASSERT_MESSAGE(m_EventMap.find(type) == m_EventMap.end(), "Event already registered for this type!");
		m_EventMap[type] = m_pEventSystem->RegisterEvent(event);
	}

	template<T type>
	inline void SendEvent()
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(0);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0>
	inline void SendEvent(T0 arg0)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(1);
		UIEVENT_ADDARGSAFE(0);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1>
	inline void SendEvent(T0 arg0, T1 arg1)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(2);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(3);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2, class T3>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2, T3 arg3)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(4);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		UIEVENT_ADDARGSAFE(3);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2, class T3, class T4>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(5);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		UIEVENT_ADDARGSAFE(3);
		UIEVENT_ADDARGSAFE(4);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2, class T3, class T4, class T5>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(6);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		UIEVENT_ADDARGSAFE(3);
		UIEVENT_ADDARGSAFE(4);
		UIEVENT_ADDARGSAFE(5);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2, class T3, class T4, class T5, class T6>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(7);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		UIEVENT_ADDARGSAFE(3);
		UIEVENT_ADDARGSAFE(4);
		UIEVENT_ADDARGSAFE(5);
		UIEVENT_ADDARGSAFE(6);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(8);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		UIEVENT_ADDARGSAFE(3);
		UIEVENT_ADDARGSAFE(4);
		UIEVENT_ADDARGSAFE(5);
		UIEVENT_ADDARGSAFE(6);
		UIEVENT_ADDARGSAFE(7);
		m_pEventSystem->SendEvent(event);
	}

	template<T type, class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	inline void SendEvent(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
	{
		UIEVENT_GETEVENT;
		UIEVENT_CHECKARGCOUNT_SEND(9);
		UIEVENT_ADDARGSAFE(0);
		UIEVENT_ADDARGSAFE(1);
		UIEVENT_ADDARGSAFE(2);
		UIEVENT_ADDARGSAFE(3);
		UIEVENT_ADDARGSAFE(4);
		UIEVENT_ADDARGSAFE(5);
		UIEVENT_ADDARGSAFE(6);
		UIEVENT_ADDARGSAFE(7);
		UIEVENT_ADDARGSAFE(8);
		m_pEventSystem->SendEvent(event);
	}

	//! Special case for dyn args.
	template<T type>
	inline void SendEvent(const SUIArguments& args)
	{
		UIEVENT_GETEVENT_DYN;
		UIEVENT_CHECKDYNARGS;
		m_pEventSystem->SendEvent(event);
	}

private:
	typedef typename std::map<T, uint>         TEventMap;
	typedef typename TEventMap::const_iterator TEventMapIter;
	TEventMap       m_EventMap;
	IUIEventSystem* m_pEventSystem;
};

//! Per-instance call
template<class T>
struct SPerInstanceCallBase
{
	typedef Functor2<IUIElement*, T> TCallback;

	bool Execute(IUIElement* pBaseElement, i32 instanceId, const TCallback& cb, T data, bool bAllowMultiInstances = true)
	{
		DRX_ASSERT_MESSAGE(pBaseElement, "NULL pointer passed!");
		if (pBaseElement)
		{
			if (instanceId < 0)
			{
				DRX_ASSERT_MESSAGE(bAllowMultiInstances, "SPerInstanceCallBase is used on multiple instances, but bAllowMultiInstances is set to false!");
				if (!bAllowMultiInstances)
					return false;

				IUIElementIteratorPtr instances = pBaseElement->GetInstances();
				while (IUIElement* pInstance = instances->Next())
				{
					// instanceId < -1 == only call instances that are initialized
					if (instanceId < -1 && !pInstance->IsInit())
						continue;

					cb(pInstance, data);
				}
			}
			else
			{
				IUIElement* pInstance = pBaseElement->GetInstance((uint) instanceId);
				if (pInstance)
					cb(pInstance, data);
			}
			return true;
		}
		return false;
	}
};

struct SPerInstanceCall0
{
	typedef Functor1<IUIElement*> TCallback;

	bool Execute(IUIElement* pBaseElement, i32 instanceId, const TCallback& cb, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase<const TCallback&> base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall0::_cb), cb, bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const TCallback& cb) { cb(pInstance); }
};

template<class T>
struct SPerInstanceCall1 : public SPerInstanceCallBase<T>
{
};

template<class T1, class T2>
struct SPerInstanceCall2
{
	typedef Functor3<IUIElement*, T1, T2> TCallback;

	struct SCallData
	{
		SCallData(const TCallback& _cb, T1 _arg1, T2 _arg2) : cb(_cb), arg1(_arg1), arg2(_arg2) {}
		const TCallback& cb;
		T1               arg1;
		T2               arg2;
	};

	bool Execute(IUIElement* pBaseElement, i32 instanceId, const TCallback& cb, T1 arg1, T2 arg2, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase<const SCallData&> base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall2::_cb), SCallData(cb, arg1, arg2), bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const SCallData& data) { data.cb(pInstance, data.arg1, data.arg2); }
};

template<class T1, class T2, class T3>
struct SPerInstanceCall3
{
	typedef Functor4<IUIElement*, T1, T2, T3> TCallback;

	struct SCallData
	{
		SCallData(const TCallback& _cb, T1 _arg1, T2 _arg2, T3 _arg3) : cb(_cb), arg1(_arg1), arg2(_arg2), arg3(_arg3) {}
		const TCallback& cb;
		T1               arg1;
		T2               arg2;
		T3               arg3;
	};

	bool Execute(IUIElement* pBaseElement, i32 instanceId, const TCallback& cb, T1 arg1, T2 arg2, T3 arg3, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase<const SCallData&> base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall3::_cb), SCallData(cb, arg1, arg2, arg3), bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const SCallData& data) { data.cb(pInstance, data.arg1, data.arg2, data.arg3); }
};

template<class T1, class T2, class T3, class T4>
struct SPerInstanceCall4
{
	typedef Functor5<IUIElement*, T1, T2, T3, T4> TCallback;

	struct SCallData
	{
		SCallData(const TCallback& _cb, T1 _arg1, T2 _arg2, T3 _arg3, T4 _arg4) : cb(_cb), arg1(_arg1), arg2(_arg2), arg3(_arg3), arg4(_arg4) {}
		const TCallback& cb;
		T1               arg1;
		T2               arg2;
		T3               arg3;
		T4               arg4;
	};

	bool Execute(IUIElement* pBaseElement, i32 instanceId, const TCallback& cb, T1 arg1, T2 arg2, T3 arg3, T4 arg4, bool bAllowMultiInstances = true)
	{
		SPerInstanceCallBase<const SCallData&> base;
		return base.Execute(pBaseElement, instanceId, functor(*this, &SPerInstanceCall4::_cb), SCallData(cb, arg1, arg2, arg3, arg4), bAllowMultiInstances);
	}

private:
	void _cb(IUIElement* pInstance, const SCallData& data) { data.cb(pInstance, data.arg1, data.arg2, data.arg3, data.arg4); }
};
//! \endcond