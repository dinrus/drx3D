// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

// Mesh
// Contain geometry data(grouped triangles, vertex attributes like tangent space or vertex color, optional physics data).
class CMeshType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CMeshType);

	virtual tukk                       GetTypeName() const override { return "Mesh"; }
	virtual tukk                       GetUiTypeName() const override { return QT_TR_NOOP("Mesh"); }
	virtual tukk GetFileExtension() const override { return "cgf"; }
	virtual bool IsImported() const override { return true; }
	virtual bool CanBeEdited() const override { return true; }
	virtual CAssetEditor* Edit(CAsset* asset) const override;
	virtual bool HasThumbnail() const override { return true; }
	virtual void GenerateThumbnail(const CAsset* pAsset) const override;
	virtual tukk GetObjectClassName() const { return "Brush"; }
	virtual std::vector<CItemModelAttribute*> GetDetails() const override;
	virtual QVariant                          GetDetailValue(const CAsset* pAsset, const CItemModelAttribute* pDetail) const override;
	virtual bool MoveAsset(CAsset* pAsset, tukk szNewPath, bool bMoveSourcefile) const override;

private:
	virtual DrxIcon GetIconInternal() const override;

public:
	//! May be referenced by other asset types, too.
	static CItemModelAttribute s_vertexCountAttribute;
	static CItemModelAttribute s_triangleCountAttribute;
	static CItemModelAttribute s_materialCountAttribute;
};

