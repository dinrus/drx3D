// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "EffectAsset.h"

#include <AssetSystem/EditableAsset.h>

namespace DrxParticleEditor
{

CEffectAsset::CEffectAsset()
	: m_pAsset(nullptr)
{
}

tukk CEffectAsset::GetName() const
{
	/*
	   if (m_pExplorerEntry && !m_pExplorerEntry->name.empty())
	   return m_pExplorerEntry->name.c_str();
	   else
	   return m_pEffect->GetName();
	 */
	return "STUFF";
}

bool CEffectAsset::IsModified() const
{
	return m_pAsset->IsModified();
}

CAsset* CEffectAsset::GetAsset()
{
	return m_pAsset;
}

pfx2::IParticleEffectPfx2* CEffectAsset::GetEffect()
{
	return m_pEffect;
}

DrxParticleEditor::CParticleGraphModel* CEffectAsset::GetModel()
{
	return m_pModel.get();
}

void CEffectAsset::SetAsset(CAsset* pAsset)
{
	m_pAsset = pAsset;
}

void CEffectAsset::SetEffect(pfx2::IParticleEffectPfx2* pEffect)
{
	m_pEffect = pEffect;
	m_pModel = pEffect ? stl::make_unique<CParticleGraphModel>(*pEffect) : nullptr;
}

bool CEffectAsset::MakeNewComponent(tukk szTemplateName)
{
	if (m_pModel)
	{
		return m_pModel->CreateNode(szTemplateName, QPointF()) != nullptr;
	}
	return false;
}

void CEffectAsset::SetModified(bool bModified)
{
	m_pAsset->SetModified(bModified);
}

}

