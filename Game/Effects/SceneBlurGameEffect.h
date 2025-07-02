// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SCENEBLUR_GAME_EFFECT_
#define _SCENEBLUR_GAME_EFFECT_

#pragma once

// Includes
#include <drx3D/Game/Effects/GameEffect.h>
#include <drx3D/Game/Effects/GameEffectsSystem.h>

// Forward declarations

//==================================================================================================
// Name: CSceneBlurGameEffect
// Desc: Manages effects for scene blur - The effect needs to be managed in 1 global place to stop
//		   different game features fighting over setting the values
// Author: Dean Claassen
//==================================================================================================
class CSceneBlurGameEffect : public CGameEffect
{
public:
	enum EGameEffectUsage
	{
		eGameEffectUsage_InterestVideos = 0,
		eGameEffectUsage_SmokeUpr,
		eGameEffectUsage_NUMTYPES,
	};

	#define NUMBLENDEDSCENEBLURUSAGES (1)

public:
	CSceneBlurGameEffect();
	~CSceneBlurGameEffect();

	// CGameEffect
	virtual void	Initialise(const SGameEffectParams* gameEffectParams = NULL) override;
	virtual void	Release() override;
	virtual void	SetActive(bool isActive) override;

	virtual void	Update(float frameTime) override;

	virtual tukk GetName() const override;
	virtual void ResetRenderParameters() override;

#if DEBUG_GAME_FX_SYSTEM
	static void  DebugOnInputEvent(i32 keyId);
	static void	 DebugDisplay(const Vec2& textStartPos,float textSize,float textYStep);
#endif

	static void LoadStaticData(IItemParamsNode* pRootNode);
	static void ReloadStaticData(IItemParamsNode* pRootNode);
	static void ReleaseStaticData();
	// ~CGameEffect

	void SetBlurAmount(const float fAmount, const EGameEffectUsage usage);
	void SetBlurAmountFromData(const bool bEnable, const EGameEffectUsage usage);

private:
	enum EGameEffectBlendableState
	{
		eGameEffectBlendableState_None = 0,
		eGameEffectBlendableState_InitialDelay,
		eGameEffectBlendableState_BlendingTo,
		eGameEffectBlendableState_Hold,
		eGameEffectBlendableState_BlendingFrom,
	};

	struct SGameEffectExecutionData
	{
		SGameEffectExecutionData()
		: m_fBlurAmount(0.0f)
		, m_fUpdateTime(0.0f)
		, m_effectState(eGameEffectBlendableState_None)
		{
		}

		float m_fBlurAmount;
		float m_fUpdateTime;
		EGameEffectBlendableState m_effectState;
	};

	typedef DrxFixedArray<SGameEffectExecutionData, eGameEffectUsage_NUMTYPES> TGameEffectExecutionData;
	TGameEffectExecutionData m_executionData;
};

#endif // _SCENEBLUR_GAME_EFFECT_
