// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "EffectAssetModel.h"
#include "EffectAsset.h"

#include <AssetSystem/Asset.h>

#include <DrxParticleSystem/IParticlesPfx2.h>

namespace DrxParticleEditor {

CEffectAssetModel::CEffectAssetModel()
	: m_nextUntitledAssetId(1)
{
}

void CEffectAssetModel::MakeNewAsset()
{
	string untitledName = "Untitled";
	untitledName += '0' + (m_nextUntitledAssetId / 10) % 10;
	untitledName += '0' + m_nextUntitledAssetId % 10;
	untitledName += "*";
	++m_nextUntitledAssetId;

	pfx2::PParticleEffect pNewEffect = GetParticleSystem()->CreateEffect();
	GetParticleSystem()->RenameEffect(pNewEffect, untitledName.c_str());

	signalBeginEffectAssetChange();

	m_pEffectAsset.reset(new CEffectAsset());
	m_pEffectAsset->SetEffect(pNewEffect); // XXX Pointer type?
	m_pEffectAsset->MakeNewComponent("%ENGINE%/EngineAssets/Particles/Default.pfxp");

	signalEndEffectAssetChange();
}

bool CEffectAssetModel::OpenAsset(CAsset* pAsset)
{
	DRX_ASSERT(pAsset);

	if (!pAsset->GetFilesCount())
	{
		return false;
	}

	signalBeginEffectAssetChange();

	const string pfxFilePath = pAsset->GetFile(0);

	pfx2::PParticleEffect pEffect = GetParticleSystem()->FindEffect(pfxFilePath.c_str());
	if (!pEffect)
	{
		pEffect = GetParticleSystem()->FindEffect(pfxFilePath.c_str(), true);
		if (!pEffect)
			return false;
	}
	else
	{
		// Reload effect from file every time it is opened, since it might be that the effect has changed
		// in memory. Opening means reading the current state from disk.
		Serialization::LoadJsonFile(*pEffect, pfxFilePath.c_str());
	}

	m_pEffectAsset.reset(new CEffectAsset());
	m_pEffectAsset->SetAsset(pAsset);
	m_pEffectAsset->SetEffect(pEffect.get()); // XXX Pointer type?

	signalEndEffectAssetChange();

	return true;
}

void CEffectAssetModel::ClearAsset()
{
	signalBeginEffectAssetChange();
	m_pEffectAsset.reset();
	signalEndEffectAssetChange();
}

CEffectAsset* CEffectAssetModel::GetEffectAsset()
{
	return m_pEffectAsset.get();
}

}

