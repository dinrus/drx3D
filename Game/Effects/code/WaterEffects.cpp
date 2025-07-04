// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/WaterEffects.h>


CWaterGameEffects::CWaterGameEffects()
	: m_waterDropletsAmount(-1.0f)
{

}

CWaterGameEffects::~CWaterGameEffects()
{
	gEnv->p3DEngine->SetPostEffectParam( "WaterDroplets_Amount", 0.0f );
}

void CWaterGameEffects::Update( float frameTime )
{
	if (m_waterDropletsAmount > 0.0f)
	{
		const float maxScreenTimeInv = (float)__fres(1.5f);
		const float newWaterDropletsAmount = m_waterDropletsAmount - (frameTime * maxScreenTimeInv);

		m_waterDropletsAmount = newWaterDropletsAmount;

		gEnv->p3DEngine->SetPostEffectParam( "WaterDroplets_Amount", newWaterDropletsAmount );
	}
	else
	{
		gEnv->p3DEngine->SetPostEffectParam( "WaterDroplets_Amount", 0.0f );

		m_waterDropletsAmount = -1.0f;
		SetActive( false );
	}
}

tukk CWaterGameEffects::GetName( ) const
{
	return "WaterEffects";
}

void CWaterGameEffects::GetMemoryUsage( IDrxSizer *pSizer ) const
{
	pSizer->AddObject(this, sizeof(*this));
}

void CWaterGameEffects::ResetRenderParameters()
{
	gEnv->p3DEngine->SetPostEffectParam( "WaterDroplets_Amount", 0.0f );
}

void CWaterGameEffects::OnCameraComingOutOfWater( )
{
	IViewSystem *pViewSystem = g_pGame->GetIGameFramework()->GetIViewSystem();
	if (pViewSystem)
	{
		if (pViewSystem->IsClientActorViewActive())
		{
			m_waterDropletsAmount = 0.7f;

			if(IsFlagSet(GAME_EFFECT_ACTIVE) == false)
			{
				SetActive( true );
			}
		}
	}
}