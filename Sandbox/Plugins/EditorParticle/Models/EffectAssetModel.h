// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <memory>

class CAsset;

namespace DrxParticleEditor {

class CEffectAsset;

class CEffectAssetModel
{
public:
	CEffectAssetModel();

	void MakeNewAsset();
	bool OpenAsset(CAsset* pAsset);
	void ClearAsset();

	CEffectAsset* GetEffectAsset();

public:
	CDrxSignal<void()> signalBeginEffectAssetChange;
	CDrxSignal<void()> signalEndEffectAssetChange;

private:
	std::unique_ptr<CEffectAsset> m_pEffectAsset; //!< There can be at most one active effect asset.
	i32 m_nextUntitledAssetId;
};

}
