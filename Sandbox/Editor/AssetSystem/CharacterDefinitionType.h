// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

// Character Definition
// The character definition file is created in the Character Editor in Sandbox. It contains the reference to a .chr file and its attachments (can be .chr or .cgf).
class CCharacterDefinitionType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CCharacterDefinitionType);

	virtual tukk GetTypeName() const override { return "Character"; }
	virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Character"); }
	virtual tukk GetFileExtension() const override { return "cdf"; }
	virtual bool        IsImported() const override { return true; }
	virtual bool        CanBeEdited() const override { return false; }
	virtual bool        HasThumbnail() const override { return false; } // once QPreviewWidget supports character preview, change this to return true to generate the icons
	virtual void        GenerateThumbnail(const CAsset* pAsset) const override;

private:
	virtual DrxIcon GetIconInternal() const override;
};

