// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Rock

-------------------------------------------------------------------------
История:
- 5:5:2006   15:24 : Created by M�rcio Martins

*************************************************************************/
#ifndef __CHAFF_H__
#define __CHAFF_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Game/Projectile.h>
#include <drx3D/CoreX/Containers/VectorSet.h>

#define MAX_SPAWNED_CHAFFS	16

class CChaff : public CProjectile
{
public:
	CChaff();
	virtual ~CChaff();

	virtual void HandleEvent(const SGameObjectEvent &);

	virtual Vec3 GetPosition(void);

	static VectorSet<CChaff*> s_chaffs;
};


#endif // __CHAFF_H__