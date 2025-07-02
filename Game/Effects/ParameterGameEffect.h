// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _PARAMETER_GAME_EFFECT_
#define _PARAMETER_GAME_EFFECT_

#pragma once



// Includes
#include <drx3D/Game/Effects/GameEffect.h>
#include <drx3D/Game/Effects/GameEffectsSystem.h>

// Forward declarations

//==================================================================================================
// Name: CParameterGameEffect
// Desc: Manages the value of the game effects' parameters when more than one module is trying to
//			modify it at the same time
// Author: Sergi Juarez
//==================================================================================================
class CParameterGameEffect : public CGameEffect
{
public:
	CParameterGameEffect();

	
	virtual void	SetActive(bool isActive) override;
	virtual void	Update(float frameTime) override;
	virtual tukk GetName() const override;
	        void	Reset();
	virtual void ResetRenderParameters() override;

//************************************************************************************
//		SATURATION EFFECT--
//************************************************************************************
public:
	enum ESaturationEffectUsage
	{
		eSEU_PreMatch = 0,
		eSEU_LeavingBattleArea,
		eSEU_PlayerHealth,
		eSEU_Intro,
		eSEU_NUMTYPES,
	};

	void SetSaturationAmount(const float fAmount, const ESaturationEffectUsage usage);

private:
	bool UpdateSaturation(float frameTime);
	void ResetSaturation();

private:
	struct SSaturationData 
	{
		float m_amount;

		SSaturationData(): m_amount(1.f){}
	};

	typedef DrxFixedArray<SSaturationData, eSEU_NUMTYPES> TSaturationEffectExecutionData;
	TSaturationEffectExecutionData m_saturationExecutionData;
//************************************************************************************
//		--SATURATION EFFECT
//************************************************************************************


};

#endif // _SCENEBLUR_GAME_EFFECT_