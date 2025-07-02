// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: K-Volt bullet

-------------------------------------------------------------------------
История:
- 13:05:2009   15:00 : Created by Claire Allan

*************************************************************************/
  
#include <drx3D/Game/Projectile.h>

class CKVoltBullet: public CProjectile
{
public:
	CKVoltBullet();
	virtual ~CKVoltBullet();

	// CProjectile
	virtual void HandleEvent(const SGameObjectEvent& event);
	virtual bool Init(IGameObject *pGameObject);
	// ~CProjectile

protected:

	void	Trigger(Vec3 collisionPos);
	void	DamageEnemiesInRange(float range, Vec3 pos, EntityId ignoreId);
	void	ProcessDamage(EntityId targetId, float damage, i32 hitPartId, Vec3 pos, Vec3 dir);

private:
	typedef CProjectile inherited;

};
