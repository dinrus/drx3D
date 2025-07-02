// Разработка 2018-2023 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 5:5:2006   15:26 : Created by M�rcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/Chaff.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/CoreX/Game/IGameTokens.h>

#include <drx3D/Game/WeaponSystem.h>

VectorSet<CChaff*> CChaff::s_chaffs;

//------------------------------------------------------------------------
CChaff::CChaff()
{
	s_chaffs.insert(this);
	if(s_chaffs.size()>MAX_SPAWNED_CHAFFS)
	{
		if(s_chaffs[0]!=this)
			s_chaffs[0]->Destroy();
		else
			s_chaffs[1]->Destroy(); //Just in case...??
	}
}

//------------------------------------------------------------------------
CChaff::~CChaff()
{
	s_chaffs.erase(this);
}

//------------------------------------------------------------------------

void CChaff::HandleEvent(const SGameObjectEvent &event)
{
	CProjectile::HandleEvent(event);
}

Vec3 CChaff::GetPosition(void)
{
	return m_last;
}