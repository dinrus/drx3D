// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _HACK_DRX3D_H_
#define _HACK_DRX3D_H_

#include <drx3D/Game/Projectile.h>



class CHackBullet : public CProjectile
{
private:
	virtual void HandleEvent(const SGameObjectEvent& event);
};



#endif
