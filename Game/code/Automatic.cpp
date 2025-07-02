// Разработка 2018-2023 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 11:9:2005   15:00 : Created by M�rcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Automatic.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/ScreenEffects.h>

#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/Game.h>


DRX_IMPLEMENT_GTI(CAutomatic, CSingle);


//------------------------------------------------------------------------
CAutomatic::CAutomatic()
{
}

//------------------------------------------------------------------------
CAutomatic::~CAutomatic()
{
}

//------------------------------------------------------------------------
void CAutomatic::StartReload(i32 zoomed)
{
	m_firing = false;
	BaseClass::StartReload(zoomed);
}

//------------------------------------------------------------------------
void CAutomatic::StartFire()
{
	BaseClass::StartFire();

	if (m_firing)
		m_pWeapon->PlayAction(GetFragmentIds().spin_down_tail);

	m_pWeapon->RequestStartFire();
}
//------------------------------------------------------------------------
void CAutomatic::Update(float frameTime, u32 frameId)
{
	BaseClass::Update(frameTime, frameId);

	if (m_firing && CanFire(false))
	{
		m_firing = Shoot(true, m_fireParams->fireparams.autoReload);

		if(!m_firing)
		{
			StopFire();
		}
	}
}

//------------------------------------------------------------------------
void CAutomatic::StopFire()
{
	if(m_firing)
		SmokeEffect();

	m_firing = false;

	m_pWeapon->RequestStopFire();

	BaseClass::StopFire();
}

//---------------------------------------------------
void CAutomatic::GetMemoryUsage(IDrxSizer * s) const
{
	s->AddObject(this, sizeof(*this));	
	GetInternalMemoryUsage(s); 
}

void CAutomatic::GetInternalMemoryUsage(IDrxSizer * s) const
{
	CSingle::GetInternalMemoryUsage(s);		// collect memory of parent class
}
