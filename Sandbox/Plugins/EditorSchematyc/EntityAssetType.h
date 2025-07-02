// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

namespace DrxSchematycEditor {

class CEntityAssetType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CEntityAssetType);

	static tukk TypeName() { return "SchematycEntity"; }

	// CAssetType
	virtual tukk   GetTypeName() const override      { return TypeName(); }
	virtual tukk   GetUiTypeName() const override    { return QT_TR_NOOP("Schematyc Entity"); }

	virtual tukk   GetFileExtension() const override { return "schematyc_ent"; }

	virtual bool          CanBeCreated() const override     { return true; }
	virtual bool          IsImported() const override       { return false; }
	virtual bool          CanBeEdited() const override      { return true; }

	virtual CAssetEditor* Edit(CAsset* pAsset) const override;
	virtual bool          RenameAsset(CAsset* pAsset, tukk szNewName) const override;
	virtual bool          DeleteAssetFiles(const CAsset& asset, bool bDeleteSourceFile, size_t& numberOfFilesDeleted) const override;

	virtual tukk   GetObjectClassName() const override { return "Entity"; }
	virtual string GetObjectFilePath(const CAsset* pAsset) const override;

protected:
	virtual bool OnCreate(CEditableAsset& editAsset, ukk pTypeSpecificParameter) const override;

private:
	virtual DrxIcon GetIconInternal() const override;
	// ~CAssetType

	Schematyc::IScript* GetScript(const CAsset& asset) const;
};

}

