// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "AssetSystem/AssetType.h"

class CGeometryCacheType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CGeometryCacheType)

	virtual tukk   GetTypeName() const override      { return GetTypeNameStatic(); }
	virtual tukk   GetUiTypeName() const override    { return QT_TR_NOOP("Geometry Cache"); }
	virtual tukk   GetFileExtension() const override { return "cax"; }
	virtual bool          IsImported() const override       { return true; }
	virtual bool          CanBeEdited() const override      { return true; }
	virtual CAssetEditor* Edit(CAsset* pAsset) const override;

	static tukk    GetTypeNameStatic() { return "GeometryCache"; }

private:
	virtual DrxIcon GetIconInternal() const override;
};

