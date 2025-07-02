// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

//! A sample asset type. This type of asset references a single text file storing a string.
//! \sa CSampleAssetEditor
class CSampleAssetType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CSampleAssetType);

	virtual tukk GetTypeName() const { return "SampleAssetType"; }
	virtual tukk GetUiTypeName() const { return "Sample Asset Type"; }
	virtual tukk GetFileExtension() const { return "smpl"; }
	virtual bool CanBeCreated() const { return true; }
	virtual bool CanBeEdited() const { return true; }
	virtual CAssetEditor* Edit(CAsset* pAsset) const override;

	virtual bool OnCreate(CEditableAsset& editAsset, ukk pCreateParams) const override;
};
