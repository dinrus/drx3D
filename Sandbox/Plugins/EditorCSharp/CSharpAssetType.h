// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <AssetSystem/AssetType.h>
#include <AssetSystem/EditableAsset.h>

class CSharpSourcefileAssetType : public CAssetType
{
private:
	struct SCreateParams;

public:
	DECLARE_ASSET_TYPE_DESC(CSharpSourcefileAssetType);

	virtual tukk   GetTypeName() const           { return "CSharpScript"; }
	virtual tukk   GetUiTypeName() const         { return QT_TR_NOOP("C# script"); }
	virtual tukk GetFileExtension() const { return "cs"; }
	virtual bool CanBeCreated() const override { return true; }
	virtual bool IsImported() const { return false; }
	virtual bool CanBeEdited() const { return true; }
	virtual CAssetEditor* Edit(CAsset* asset) const override;

protected:
	virtual bool OnCreate(CEditableAsset& editAsset, ukk pTypeSpecificParameter) const override;

private:
	virtual DrxIcon GetIconInternal() const override;
	string GetCleanName(const string& name) const;
};

