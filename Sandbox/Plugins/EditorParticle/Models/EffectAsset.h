// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ParticleGraphModel.h"

class CAsset;

namespace DrxParticleEditor {

class CEffectAsset
{
public:
	CEffectAsset();

	tukk GetName() const;
	bool IsModified() const;
	CAsset* GetAsset();

	pfx2::IParticleEffectPfx2* GetEffect();
	DrxParticleEditor::CParticleGraphModel* GetModel();

	void SetAsset(CAsset* pAsset);
	void SetEffect(pfx2::IParticleEffectPfx2* pEffect);

	bool MakeNewComponent(tukk szTemplateName);

	void SetModified(bool bModified);

private:
	CAsset* m_pAsset;

	std::unique_ptr<DrxParticleEditor::CParticleGraphModel> m_pModel;
	_smart_ptr<pfx2::IParticleEffectPfx2>                   m_pEffect;
	bool m_bModified;
};

}
