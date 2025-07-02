// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: DebugGun Implementation

-------------------------------------------------------------------------
История:
- 09:01:2006   14:00 : Created by Michael Rauh

*************************************************************************/
#ifndef __DebugGun_H__
#define __DebugGun_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Act/IItemSystem.h>
#include <drx3D/Game/Weapon.h>


class CDebugGun :
  public CWeapon
{
public:
  CDebugGun();
  void OnAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value);
  void Update(SEntityUpdateContext& ctx, i32 update);
  void Shoot( bool bPrimary);
	virtual void GetMemoryUsage(IDrxSizer * s) const
	{
		s->AddObject(this, sizeof(*this));		
		s->AddContainer(m_fireModes);
		CWeapon::GetInternalMemoryUsage(s); // collect memory of parent class
	}

  virtual void Select(bool select);

private:
  ICVar* m_pAIDebugDraw;
  i32 m_aiDebugDrawPrev;
  
  typedef std::pair<string, float> TFmPair;
  std::vector<TFmPair> m_fireModes;    
  i32 m_fireMode;
};

#endif // __DebugGun_H__