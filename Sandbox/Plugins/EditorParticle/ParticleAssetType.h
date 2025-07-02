// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "AssetSystem/AssetType.h"

class CEditableAsset;

//PFX assets
class CParticlesType : public CAssetType
{
private:
	struct SCreateParams;

public:
	DECLARE_ASSET_TYPE_DESC(CParticlesType);

	bool CreateForExistingEffect(tukk szFilePath) const;

	virtual tukk   GetTypeName() const override		{ return "Particles"; }
	virtual tukk   GetUiTypeName() const override	{ return QT_TR_NOOP("Particles"); }
	virtual tukk GetFileExtension() const override	{ return "pfx"; }
	virtual bool CanBeCreated() const override				{ return true; }
	virtual bool IsImported() const	override				{ return false; }
	virtual bool CanBeEdited() const override				{ return true; }
	virtual CAssetEditor* Edit(CAsset* asset) const override;
	virtual tukk GetObjectClassName() const override	{ return "EntityWithParticleComponent"; }

	//Particle needs to support legacy editor still
	virtual bool IsUsingGenericPropertyTreePicker() const override { return false; }

protected:
	virtual bool OnCreate(CEditableAsset& editAsset, ukk pTypeSpecificParameter) const override;

private:
	virtual DrxIcon GetIconInternal() const override;
};

