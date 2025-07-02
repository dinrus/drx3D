// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/Schema/IString.h>

namespace sxema
{

struct STypeOperators
{
	typedef void (* DefaultConstruct)(uk pPlacement);
	typedef void (* Destruct)(uk pPlacement);
	typedef void (* CopyConstruct)(uk pPlacement, ukk pRHS);
	typedef void (* CopyAssign)(uk pLHS, ukk pRHS);
	typedef bool (* Equals)(ukk pLHS, ukk pRHS);
	typedef bool (* Serialize)(Serialization::IArchive& archive, uk pValue, tukk szName, tukk szLabel);
	typedef void (* ToString)(IString& output, ukk pInput);

	DefaultConstruct defaultConstruct = nullptr;
	Destruct         destruct = nullptr;
	CopyConstruct    copyConstruct = nullptr;
	CopyAssign       copyAssign = nullptr;
	Equals           equals = nullptr;
	Serialize        serialize = nullptr;
	ToString         toString = nullptr;
};

struct SStringOperators
{
	typedef tukk (* GetChars)(ukk pString);

	GetChars getChars = nullptr;
};

struct SArrayOperators
{
	typedef u32 (*     Size)(ukk pArray);
	typedef uk (*       At)(uk pArray, u32 pos);
	typedef ukk (* AtConst)(ukk pArray, u32 pos);
	typedef void (*       PushBack)(uk pArray, ukk pValue);

	Size     size = nullptr;
	At       at = nullptr;
	AtConst  atConst = nullptr;
	PushBack pushBack = nullptr;
};

} // sxema

#include <drx3D/Schema/TypeOperators.inl>
