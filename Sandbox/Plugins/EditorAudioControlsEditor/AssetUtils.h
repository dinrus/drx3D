// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <SharedData.h>
#include <drx3D/CoreX/String/DrxString.h>

namespace ACE
{
class CAsset;

namespace AssetUtils
{
string      GenerateUniqueName(string const& name, EAssetType const type, CAsset* const pParent);
string      GenerateUniqueLibraryName(string const& name);
string      GenerateUniqueControlName(string const& name, EAssetType const type);
CAsset*     GetParentLibrary(CAsset* const pAsset);
char const* GetTypeName(EAssetType const type);
void        SelectTopLevelAncestors(Assets const& source, Assets& dest);
} //endns AssetUtils
} //endns ACE
