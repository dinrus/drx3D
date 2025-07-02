// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// TODO : Replace type traits with std::traits?

#pragma once

#include <drx3D/CoreX/Extension/TypeList.h>

#include <drx3D/Schema2/TemplateUtils_PreprocessorUtils.h>
#include <drx3D/CoreX/String/StringUtils.h>

namespace TemplateUtils
{
	template <bool SELECT, typename LHS, typename RHS> struct Select;

	template <typename LHS, typename RHS> struct Select<true, LHS, RHS>
	{
		typedef LHS Result;
	};

	template <typename LHS, typename RHS> struct Select<false, LHS, RHS>
	{
		typedef RHS Result;
	};

	template <typename TYPE> struct ReferenceTraits
	{
		static const bool IsReference = false;

		typedef TYPE RemoveReference;
	};

	template <typename TYPE> struct ReferenceTraits<TYPE &>
	{
		static const bool IsReference = true;

		typedef TYPE RemoveReference;
	};

	template <typename TYPE> struct PointerTraits
	{
		static const bool IsPointer = false;

		typedef TYPE RemovePointer;
	};

	template <typename TYPE> struct PointerTraits<TYPE *>
	{
		static const bool IsPointer = true;

		typedef TYPE RemovePointer;
	};
/*
	template <typename TYPE> struct VolatileTraits
	{
		static const bool IsVolatile = false;

		typedef TYPE RemoveVolatile;
	};

	template <typename TYPE> struct VolatileTraits<volatile TYPE>
	{
		static const bool IsVolatile = true;

		typedef TYPE RemoveVolatile;
	};
*/
	template <typename TYPE> struct ConstTraits
	{
		static const bool IsConst = false;

		typedef TYPE RemoveConst;
	};

	template <typename TYPE> struct ConstTraits<const TYPE>
	{
		static const bool IsConst = true;

		typedef TYPE RemoveConst;
	};

	template <typename TYPE> struct RemoveReference
	{
		typedef typename ReferenceTraits<TYPE>::RemoveReference Result;
	};

	template <typename TYPE> struct RemoveReferenceAndConst
	{
		typedef typename ConstTraits<typename ReferenceTraits<TYPE>::RemoveReference>::RemoveConst Result;
	};

	template <typename TYPE> struct RemovePointer
	{
		typedef typename PointerTraits<TYPE>::RemovePointer Result;
	};

	template <typename TYPE> struct RemovePointerAndConst
	{
		typedef typename ConstTraits<typename PointerTraits<TYPE>::RemovePointer>::RemoveConst Result;
	};

	template <typename TYPE> struct GetUnqualifiedType
	{
		typedef typename ConstTraits<typename PointerTraits<typename ReferenceTraits<TYPE>::RemoveReference>::RemovePointer>::RemoveConst Result;
	};

	template <typename LHS_TYPE, typename RHS_TYPE> struct Conversion
	{
	private:

		typedef char True;
		typedef long False;

		static LHS_TYPE MakeLHSType();
		static True Test(RHS_TYPE);
		static False Test(...);

	public:

		static const bool Exists         = sizeof((Test(MakeLHSType()))) == sizeof(True);
		static const bool ExistsBothWays = Exists && Conversion<LHS_TYPE, RHS_TYPE>::Exists;
		static const bool SameType       = false;
	};

	template <class TYPE> struct Conversion<TYPE, TYPE>
	{
		static const bool Exists         = true;
		static const bool ExistsBothWays = true;
		static const bool SameType       = true;
	};

	template <class TYPE> struct Conversion<TYPE, void>
	{
		static const bool Exists         = false;
		static const bool ExistsBothWays = false;
		static const bool SameType       = false;
	};

	template <class TYPE> struct Conversion<void, TYPE>
	{
		static const bool Exists         = false;
		static const bool ExistsBothWays = false;
		static const bool SameType       = false;
	};

	template <> struct Conversion<void, void>    
	{
		static const bool Exists         = true;
		static const bool ExistsBothWays = true;
		static const bool SameType       = true;
	};

	template <typename TYPE> struct IsClass
	{
	private:

		typedef char True;
		typedef long False;

		template <class U> static True Test(void (U::*)());
		template <class U> static False Test(...);

	public:

		static const bool Result = (sizeof(Test<TYPE>(0)) == sizeof(True));
	};

	template <class LHS, class RHS> struct ListConversion;

	template <> struct ListConversion<TL::NullType, TL::NullType> 
	{
		static const bool Exists = true;
	};

	template <class HEAD, class TAIL> struct ListConversion<TL::Typelist<HEAD, TAIL>,TL::NullType>
	{
		static const bool Exists = false;
	};

	template <class HEAD, class TAIL> struct ListConversion<TL::NullType, TL::Typelist<HEAD, TAIL> >
	{
		static const bool Exists = false;
	};
	
	template <class LHS_HEAD, class LHS_TAIL, class RHS_HEAD, class RHS_TAIL> struct ListConversion<TL::Typelist<LHS_HEAD, LHS_TAIL>, TL::Typelist<RHS_HEAD, RHS_TAIL> >
	{
		static const bool Exists = (Conversion<LHS_HEAD, RHS_HEAD>::Exists && ListConversion<LHS_TAIL, RHS_TAIL>::Exists);
	};

	namespace Private
	{
		// MSVC's macro __FUNCSIG__ produces something like this:
		//tukk __cdecl TemplateUtils::Private::GetFunctionName<void (__cdecl*)(i32),&void __cdecl SchematycLegacy::Entity::TestFunction(i32)>::operator ()(void) const
		inline tukk GetFunctionNameImplMSVC(tukk szFunctionName, size_t& outLength)
		{
			tukk szPrefix = "TemplateUtils::Private::GetFunctionName<";
			tukk szStartOfFunctionSignature = strstr(szFunctionName, szPrefix);

			if (szStartOfFunctionSignature)
			{
				static tukk szSuffix = ">::operator";
				tukk        szEndOfFunctionName = strstr(szStartOfFunctionSignature + strlen(szPrefix), szSuffix);

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

					outLength = (size_t)(szEndOfFunctionName - szStartOfFunctionName);
					return szStartOfFunctionName;
				}
			}

			DRX_ASSERT_MESSAGE(false, "Failed to extract function name!");

			outLength = 0;
			return nullptr;
		}



		// Use GetFunctionNameImplGCC() or GetFunctionNameImplClang() below
		inline tukk GetFunctionNameImplClangGCC(tukk szFunctionName, tukk szPrefix, size_t& outLength)
		{
			tukk szStartOfFunctionSignature = strstr(szFunctionName, szPrefix);

			if (szStartOfFunctionSignature)
			{
				tukk szStartOfFunctionName = szStartOfFunctionSignature + strlen(szPrefix);

				size_t len = ::strlen(szStartOfFunctionName);
				if (len > 1 && szStartOfFunctionName[len - 1] == ']')
				{
					outLength = len - 1;
					return szStartOfFunctionName;
				}
			}

			DRX_ASSERT_MESSAGE(false, "Failed to extract function name!");

			outLength = 0;
			return nullptr;
		}

		// GCC's macro __PRETTY_FUNCTION__ produces something like this:
		// tukk TemplateUtils::Private::GetFunctionName<FUNCTION_PTR_TYPE, FUNCTION_PTR>::operator()() const [with FUNCTION_PTR_TYPE = void (*)(i32); FUNCTION_PTR_TYPE FUNCTION_PTR = SchematycLegacy::Entity::TestFunction]
		inline tukk GetFunctionNameImplGCC(tukk szFunctionName, size_t& outLength)
		{
			tukk szPrefix = "FUNCTION_PTR = ";
			return GetFunctionNameImplClangGCC(szFunctionName, szPrefix, outLength);
		}

		// Clang's macro __PRETTY_FUNCTION__ produces something like this:
		// tukk TemplateUtils::Private::GetFunctionName<void (*)(i32), &SchematycLegacy::Entity::TestFunction>::operator()() const [FUNCTION_PTR_TYPE = void (*)(i32), FUNCTION_PTR = &SchematycLegacy::Entity::TestFunction]
		inline tukk GetFunctionNameImplClang(tukk szFunctionName, size_t& outLength)
		{
			tukk szPrefix = "FUNCTION_PTR = &";
			return GetFunctionNameImplClangGCC(szFunctionName, szPrefix, outLength);
		}

		// Structure for extracting function name from function pointer.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		template <typename FUNCTION_PTR_TYPE, FUNCTION_PTR_TYPE FUNCTION_PTR> struct GetFunctionName
		{
			inline tukk operator () () const
			{
				// TODO pavloi 2017.09.04: would be nice to be able to store and return string_view here
				static tukk szResult = "";
				if (!szResult[0])
				{
					tukk szFunctionName = COMPILE_TIME_FUNCTION_NAME;
					static char szStorage[256] = "";

					size_t length = 0;
#if DRX_COMPILER_MSVC
					tukk szNameBegin = GetFunctionNameImplMSVC(szFunctionName, *&length);
#elif DRX_COMPILER_CLANG
					tukk szNameBegin = GetFunctionNameImplClang(szFunctionName, *&length);
#elif DRX_COMPILER_GCC
					tukk szNameBegin = GetFunctionNameImplGCC(szFunctionName, *&length);
#else
					tukk szNameBegin = nullptr;
#endif 

					if (length > 0)
					{
						DRX_ASSERT_MESSAGE(length < sizeof(szStorage), "Not enough space for function name storage - name will be clamped");

						drx_strcpy(szStorage, szNameBegin, length);
						szResult = szStorage;
					}
				}

				return szResult;
			}
		};
	} //endns Private

	// Get function name.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename FUNCTION_PTR_TYPE, FUNCTION_PTR_TYPE FUNCTION_PTR> inline tukk GetFunctionName()
	{
		return Private::GetFunctionName<FUNCTION_PTR_TYPE, FUNCTION_PTR>()();
	}

	namespace Private
	{
		// Structure for extracting type name from type.
		////////////////////////////////////////////////////////////////////////////////////////////////////
		template <typename TYPE> struct GetTypeName
		{
			inline tukk operator () () const
			{
				static tukk szResult = "";
				if(!szResult[0])
				{
					tukk szFunctionName = COMPILE_TIME_FUNCTION_NAME;
					tukk szPrefix = "TemplateUtils::Private::GetTypeName<";
					tukk szStartOfTypeName = strstr(szFunctionName, szPrefix);
					if(szStartOfTypeName)
					{
						szStartOfTypeName += strlen(szPrefix);
						tukk keyWords[] = { "struct ", "class ", "enum "	};
						for(size_t iKeyWord = 0; iKeyWord < DRX_ARRAY_COUNT(keyWords); ++ iKeyWord)
						{
							tukk 		keyWord = keyWords[iKeyWord];
							const size_t	keyWordLength = strlen(keyWord);
							if(strncmp(szStartOfTypeName, keyWord, keyWordLength) == 0)
							{
								szStartOfTypeName += keyWordLength;
								break;
							}
						}
						static tukk szSffix = ">::operator";
						tukk        szEndOfTypeName = strstr(szStartOfTypeName, szSffix);
						if(szEndOfTypeName)
						{
							while(*(szEndOfTypeName - 1) == ' ')
							{
								-- szEndOfTypeName;
							}
							static char storage[8192] = "";
							drx_strcpy(storage, szStartOfTypeName, (size_t)(szEndOfTypeName - szStartOfTypeName));
							szResult = storage;
						}
					}
					DRX_ASSERT_MESSAGE(szResult[0], "Failed to extract type name!");
				}
				return szResult;
			}
		};
	}

	// Get type name.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename TYPE> inline tukk GetTypeName()
	{
		return Private::GetTypeName<TYPE>()();
	}

	// Get member offset.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename TYPE, typename MEMBER_TYPE> inline ptrdiff_t GetMemberOffset(MEMBER_TYPE TYPE::* pMember)
	{
		return reinterpret_cast<u8k*>(&(reinterpret_cast<const TYPE*>(0x1)->*pMember)) - reinterpret_cast<u8k*>(0x1);
	}
}