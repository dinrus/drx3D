// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "AnimationType.h"

#include <AssetSystem/AssetEditor.h>

REGISTER_ASSET_TYPE(CAnimationType)

CAssetEditor* CAnimationType::Edit(CAsset* asset) const
{
	return CAssetEditor::OpenAssetForEdit("Animation", asset);
}

DrxIcon CAnimationType::GetIconInternal() const
{
	return DrxIcon("icons:common/assets_animation.ico");
}

