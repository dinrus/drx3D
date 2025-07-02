// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

// Skeleton
// Contains the skeleton data and physics proxies used for hit detection and ragdoll simulation which is driven by animations.
class CSkeletonType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CSkeletonType);

	virtual tukk                       GetTypeName() const override { return "Skeleton"; }
	virtual tukk                       GetUiTypeName() const override { return QT_TR_NOOP("Skeleton"); }
	virtual tukk GetFileExtension() const override { return "chr"; }
	virtual bool IsImported() const override { return true; }
	virtual bool CanBeEdited() const override { return true; }
	virtual CAssetEditor* Edit(CAsset* asset) const override;
	virtual bool HasThumbnail() const override { return false; }
	virtual void GenerateThumbnail(const CAsset* pAsset) const override;
	virtual std::vector<CItemModelAttribute*> GetDetails() const override;
	virtual QVariant                          GetDetailValue(const CAsset* pAsset, const CItemModelAttribute* pDetail) const override;

private:
	virtual DrxIcon GetIconInternal() const override;

public:
	//! May be referenced by other asset types, too.
	static CItemModelAttribute s_bonesCountAttribute;
};

