// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Declare specialized cast functions for shared pointers?

#pragma once

#include <drx3D/Schema/TypeDesc.h>

namespace sxema
{

template<typename TYPE> bool ToString(IString& output, const TYPE& input) // #SchematycTODO : Move/remove?
{
	const STypeOperators& typeOperators = GetTypeDesc<TYPE>().GetOperators();
	if (typeOperators.toString)
	{
		(*typeOperators.toString)(output, &input);
		return true;
	}
	return false;
}

namespace Helpers
{

inline uk DynamicCast(const CCommonTypeDesc& fromTypeDesc, uk pFrom, const CCommonTypeDesc& toTypeDesc)
{
	DRX_ASSERT(pFrom);
	if (fromTypeDesc == toTypeDesc)
	{
		return pFrom;
	}
	else
	{
		const ETypeCategory fromTypeCategory = fromTypeDesc.GetCategory();
		if ((fromTypeCategory == ETypeCategory::Class) || (fromTypeCategory == ETypeCategory::CustomClass))
		{
			for (const CClassBaseDesc& base : static_cast<const CClassDesc&>(fromTypeDesc).GetBases())
			{
				uk pCastValue = DynamicCast(base.GetTypeDesc(), pFrom, toTypeDesc);
				if (pCastValue)
				{
					return static_cast<u8*>(pCastValue) + base.GetOffset();
				}
			}
		}
	}
	return nullptr;
}

inline ukk DynamicCast(const CCommonTypeDesc& fromTypeDesc, ukk pFrom, const CCommonTypeDesc& toTypeDesc)
{
	DRX_ASSERT(pFrom);
	if (fromTypeDesc == toTypeDesc)
	{
		return pFrom;
	}
	else
	{
		const ETypeCategory fromTypeCategory = fromTypeDesc.GetCategory();
		if ((fromTypeCategory == ETypeCategory::Class) || (fromTypeCategory == ETypeCategory::CustomClass))
		{
			for (const CClassBaseDesc& base : static_cast<const CClassDesc&>(fromTypeDesc).GetBases())
			{
				ukk pCastValue = DynamicCast(base.GetTypeDesc(), pFrom, toTypeDesc);
				if (pCastValue)
				{
					return static_cast<u8k*>(pCastValue) + base.GetOffset();
				}
			}
		}
	}
	return nullptr;
}

template<typename TO_TYPE> TO_TYPE* DynamicCast(const CCommonTypeDesc& fromTypeDesc, uk pFrom)
{
	return static_cast<TO_TYPE*>(DynamicCast(fromTypeDesc, pFrom, GetTypeDesc<TO_TYPE>()));
}

template<typename TO_TYPE> const TO_TYPE* DynamicCast(const CCommonTypeDesc& fromTypeDesc, ukk pFrom)
{
	return static_cast<const TO_TYPE*>(DynamicCast(fromTypeDesc, pFrom, GetTypeDesc<TO_TYPE>()));
}

} // Helpers

template<typename TO_TYPE, typename FROM_TYPE> TO_TYPE& DynamicCast(FROM_TYPE& from)
{
	uk pResult = Helpers::DynamicCast(GetTypeDesc<FROM_TYPE>(), &from, GetTypeDesc<TO_TYPE>());
	DRX_ASSERT(pResult);
	return *static_cast<TO_TYPE*>(pResult);
}

template<typename TO_TYPE, typename FROM_TYPE> const TO_TYPE* DynamicCast(const FROM_TYPE& from)
{
	ukk pResult = Helpers::DynamicCast(GetTypeDesc<FROM_TYPE>(), &from, GetTypeDesc<TO_TYPE>());
	DRX_ASSERT(pResult);
	return *static_cast<const TO_TYPE*>(pResult);
}

template<typename TO_TYPE, typename FROM_TYPE> TO_TYPE* DynamicCast(FROM_TYPE* pFrom)
{
	return pFrom ? static_cast<TO_TYPE*>(Helpers::DynamicCast(GetTypeDesc<FROM_TYPE>(), pFrom, GetTypeDesc<TO_TYPE>())) : nullptr;
}

template<typename TO_TYPE, typename FROM_TYPE> const TO_TYPE* DynamicCast(const FROM_TYPE* pFrom)
{
	return pFrom ? static_cast<const TO_TYPE*>(Helpers::DynamicCast(GetTypeDesc<FROM_TYPE>(), pFrom, GetTypeDesc<TO_TYPE>())) : nullptr;
}

} // sxema
