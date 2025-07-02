// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <type_traits>

#include <drx3D/CoreX/Extension/TypeList.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/String/StringUtils.h>

#include <drx3D/Schema/PreprocessorUtils.h>

namespace sxema
{
namespace HasOperator
{

template<typename TYPE, typename = void>
struct SEquals : std::false_type {};

template<typename TYPE>
struct SEquals<
  TYPE,
  decltype(void(std::declval<const TYPE&>() == std::declval<const TYPE&>()))
  > : std::true_type {};

} // HasOperator

// Test to determine whether types in first list can be converted to respective types in second.
////////////////////////////////////////////////////////////////////////////////////////////////////

template<class LHS, class RHS> struct STypeListConversion;

template<> struct STypeListConversion<TL::NullType, TL::NullType>
{
	static const bool value = true;
};

template<class HEAD, class TAIL> struct STypeListConversion<TL::Typelist<HEAD, TAIL>, TL::NullType>
{
	static const bool value = false;
};

template<class HEAD, class TAIL> struct STypeListConversion<TL::NullType, TL::Typelist<HEAD, TAIL>>
{
	static const bool value = false;
};

template<class LHS_HEAD, class LHS_TAIL, class RHS_HEAD, class RHS_TAIL> struct STypeListConversion<TL::Typelist<LHS_HEAD, LHS_TAIL>, TL::Typelist<RHS_HEAD, RHS_TAIL>>
{
	static const bool value = std::is_convertible<LHS_HEAD, RHS_HEAD>::value && STypeListConversion<LHS_TAIL, RHS_TAIL>::value;
};

// Get function name.
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename FUNCTION_PTR_TYPE, FUNCTION_PTR_TYPE FUNCTION_PTR> tukk GetFunctionName();

namespace Private
{
template<typename FUNCTION_PTR_TYPE, FUNCTION_PTR_TYPE FUNCTION_PTR> struct SExtractFunctionName
{
	inline string operator()() const
	{
		string functionName;
		tukk szFunctionName = SXEMA_FUNCTION_NAME;
		tukk szPrefix = "sxema::Private::SExtractFunctionName<";
		tukk szStartOfFunctionSignature = strstr(szFunctionName, szPrefix);
		if (szStartOfFunctionSignature)
		{
			static tukk szSuffix = ">::operator";
			tukk szEndOfFunctionName = strstr(szStartOfFunctionSignature + strlen(szPrefix), szSuffix);
			if (szEndOfFunctionName)
			{
				i32 scope = 0;
				while (true)
				{
					--szEndOfFunctionName;
					if (*szEndOfFunctionName == ')')
					{
						++scope;
					}
					else if (*szEndOfFunctionName == '(')
					{
						--scope;
						if (scope == 0)
						{
							break;
						}
					}
				}
				tukk szStartOfFunctionName = szEndOfFunctionName - 1;
				while (*(szStartOfFunctionName - 1) != ' ')
				{
					--szStartOfFunctionName;
				}
				functionName.assign(szStartOfFunctionName, 0, static_cast<string::size_type>(szEndOfFunctionName - szStartOfFunctionName));
			}
		}
		DRX_ASSERT_MESSAGE(!functionName.empty(), "Failed to extract function name!");
		return functionName;
	}
};

} // Private

template<typename FUNCTION_PTR_TYPE, FUNCTION_PTR_TYPE FUNCTION_PTR> inline tukk GetFunctionName()
{
	static const string functionName = Private::SExtractFunctionName<FUNCTION_PTR_TYPE, FUNCTION_PTR>()();
	return functionName.c_str();
}

// Get type name.
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Private
{
template<typename TYPE> struct SExtractTypeName
{
	inline string operator()() const
	{
		string typeName;
		tukk szFunctionName = SXEMA_FUNCTION_NAME;
		tukk szPrefix = "sxema::Private::SExtractTypeName<";
		tukk szStartOfTypeName = strstr(szFunctionName, szPrefix);
		if (szStartOfTypeName)
		{
			szStartOfTypeName += strlen(szPrefix);
			tukk keyWords[] = { "struct ", "class ", "enum " };
			for (u32 iKeyWord = 0; iKeyWord < DRX_ARRAY_COUNT(keyWords); ++iKeyWord)
			{
				tukk keyWord = keyWords[iKeyWord];
				u32k keyWordLength = strlen(keyWord);
				if (strncmp(szStartOfTypeName, keyWord, keyWordLength) == 0)
				{
					szStartOfTypeName += keyWordLength;
					break;
				}
			}
			static tukk szSffix = ">::operator";
			tukk szEndOfTypeName = strstr(szStartOfTypeName, szSffix);
			if (szEndOfTypeName)
			{
				while (*(szEndOfTypeName - 1) == ' ')
				{
					--szEndOfTypeName;
				}
				typeName.assign(szStartOfTypeName, 0, static_cast<string::size_type>(szEndOfTypeName - szStartOfTypeName));
			}
		}
		DRX_ASSERT_MESSAGE(!typeName.empty(), "Failed to extract type name!");
		return typeName;
	}
};

} // Private
} // sxema
