// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Folder.h"

namespace ACE
{
//////////////////////////////////////////////////////////////////////////
CFolder::CFolder(string const& name)
	: CAsset(name, EAssetType::Folder)
{}
} //endns ACE
