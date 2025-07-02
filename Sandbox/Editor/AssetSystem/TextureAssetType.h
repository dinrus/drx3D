// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "AssetSystem/AssetType.h"

class CTextureType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CTextureType);

	virtual tukk                       GetTypeName() const override { return "Texture"; }
	virtual tukk                       GetUiTypeName() const override { return QT_TR_NOOP("Texture"); }
	virtual tukk GetFileExtension() const override { return "dds"; }
	virtual bool IsImported() const override { return true; }
	virtual bool CanBeEdited() const override { return true; }
	virtual bool HasThumbnail() const override { return true; }
	virtual void GenerateThumbnail(const CAsset* pAsset) const override;
	virtual QWidget* CreateBigInfoWidget(const CAsset* pAsset) const override;
	virtual CAssetEditor* Edit(CAsset* asset) const override;
	virtual std::vector<CItemModelAttribute*> GetDetails() const override;
	virtual QVariant                          GetDetailValue(const CAsset* pAsset, const CItemModelAttribute* pDetail) const override;

private:
	virtual DrxIcon GetIconInternal() const override;

public:
	//! May be referenced by other asset types, too.
	static CItemModelAttribute s_widthAttribute;
	static CItemModelAttribute s_heightAttribute;
	static CItemModelAttribute s_mipCountAttribute;
};

