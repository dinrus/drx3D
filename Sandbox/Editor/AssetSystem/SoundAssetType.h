// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "AssetSystem/AssetType.h"

//This is only for loose sounds (ex: mp3, ogg, wav files)
//Sounds from soundbanks are not handled though this asset type
class CSoundType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CSoundType);

	virtual tukk GetTypeName() const override		{ return "Sound"; }
	virtual tukk GetUiTypeName() const override		{ return QT_TR_NOOP("Sound"); }
	virtual tukk GetFileExtension() const override	{ return "wav"; } // TODO: At the moment, the RC produces .ogg.cryasset and .wav.cryasset files.
	virtual bool        IsImported() const override			{ return false; }
	virtual bool        CanBeEdited() const override		{ return false; }

	//This asset type almost could use the generic pickers 
	//but because it is allowing more extensions than wav to be picked with file picker
	//and we know that this asset type has problems handling various formats still
	virtual bool IsUsingGenericPropertyTreePicker() const override { return false; }

private:
	virtual DrxIcon GetIconInternal() const override
	{
		return DrxIcon("icons:common/assets_sound.ico");
	}
};

