// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

// Skinned Render mesh
// Contains skinned character data. It can be any asset that is animated with bone - weighted vertices like humans, aliens, ropes, lamps, heads, and parachutes.The.skin file includes the mesh, vertex weighting, vertex colors, and morph targets.
class CSkinnedMeshType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CSkinnedMeshType);

	virtual tukk                       GetTypeName() const override { return "SkinnedMesh"; }
	virtual tukk                       GetUiTypeName() const override { return QT_TR_NOOP("Skinned Mesh"); }
	virtual tukk GetFileExtension() const override { return "skin"; }
	virtual bool IsImported() const override { return true; }
	virtual bool CanBeEdited() const override { return true; }
	virtual CAssetEditor* Edit(CAsset* asset) const override;
	virtual bool HasThumbnail() const override { return false; }
	virtual void GenerateThumbnail(const CAsset* pAsset) const override;
	virtual std::vector<CItemModelAttribute*> GetDetails() const override;
	virtual QVariant GetDetailValue(const CAsset* pAsset, const CItemModelAttribute* pDetail) const override;

private:
	virtual DrxIcon GetIconInternal() const override;
};

