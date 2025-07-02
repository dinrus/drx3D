// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Water related effects

-------------------------------------------------------------------------
История:
- 16:05:2012   Created by Benito Gangoso Rodriguez

*************************************************************************/
#pragma once

#ifndef _WATER_GAME_EFFECTS_
#define _WATER_GAME_EFFECTS_

#include <drx3D/Game/Effects/GameEffect.h>
#include <drx3D/Game/Effects/GameEffectsSystem.h>

class CWaterGameEffects : public CGameEffect
{
	typedef CGameEffect BaseEffectClass;

public:
	CWaterGameEffects( );
	virtual ~CWaterGameEffects( );

	virtual void	Update(float frameTime) override;
	virtual tukk GetName() const override;
	virtual void GetMemoryUsage( IDrxSizer *pSizer ) const override;
	virtual void ResetRenderParameters() override;

	void OnCameraComingOutOfWater( );

private:

	float m_waterDropletsAmount;
};

#endif // _WATER_GAME_EFFECTS_