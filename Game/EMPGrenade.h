// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Grenade that delivers multiple EMP pulses over a given time period

*************************************************************************/
#ifndef __EMP_GRENADE_H__
#define __EMP_GRENADE_H__

#include <drx3D/Game/Grenade.h>


class CEMPGrenade : public CGrenade
{
private:
	typedef CGrenade BaseClass;

public:
	CEMPGrenade();
	virtual ~CEMPGrenade();

	virtual void Update( SEntityUpdateContext &ctx, i32 updateSlot);
	virtual void HandleEvent(const SGameObjectEvent &event);

private:
	
	Vec3 m_pulsePos;

	float m_postExplosionLifetime;
	bool	m_bActive;
};

#endif //__EMP_GRENADE_H__
