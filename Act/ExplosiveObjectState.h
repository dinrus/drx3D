// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: state of an object pre-explosion
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __EXPLOSIVEOBJECTSTATE_H__
#define __EXPLOSIVEOBJECTSTATE_H__

#pragma once

struct SExplosiveObjectState
{
	bool     isEnt;
	// for entity
	EntityId entId;
	Vec3     entPos;
	Quat     entRot;
	Vec3     entScale;
	// for statobj
	Vec3     eventPos;
	u32   hash;
};

bool ExplosiveObjectStateFromPhysicalEntity(SExplosiveObjectState& out, IPhysicalEntity* pEnt);

struct SDeclareExplosiveObjectState : public SExplosiveObjectState
{
	SDeclareExplosiveObjectState() : breakId(-1) {}
	SDeclareExplosiveObjectState(i32 bid, const SExplosiveObjectState& eos) : breakId(bid), SExplosiveObjectState(eos) {}
	i32 breakId;

	void SerializeWith(TSerialize ser);
};

#endif
