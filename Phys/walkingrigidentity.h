// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef walkingrigidentity_h
#define walkingrigidentity_h
#pragma once

#include "rigidentity.h"

class CWalkingRigidEntity : public CRigidEntity {
public:
	explicit CWalkingRigidEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CWalkingRigidEntity() { delete[] m_legs;	}
	virtual pe_type GetType() const { return PE_WALKINGRIGID; }

	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id, i32 bThreadSafe=1);
	virtual i32 SetParams(pe_params *_params,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params *_params) const;
	virtual i32 GetStatus(pe_status*) const;

	virtual void RecomputeMassDistribution(i32 ipart=-1,i32 bMassChanged=1);
	virtual i32 Step(float dt);
	virtual float CalcEnergy(float dt) { return m_Eaux + CRigidEntity::CalcEnergy(dt); }
	virtual i32 GetPotentialColliders(CPhysicalEntity **&pentlist, float dt=0);
	virtual bool OnSweepHit(geom_contact &cnt, i32 icnt, float &dt, Vec3 &vel, i32 &nsweeps);
	virtual void CheckAdditionalGeometry(float dt);
	virtual i32 RegisterContacts(float dt,i32 nMaxPlaneContacts);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	Vec3 *m_legs = nullptr;
	i32 m_nLegs = 0;
	i32 m_colltypeLegs = geom_colltype_player;
	float m_minLegsCollMass = 1.0f;
	float m_velStick = 0.5f;
	float m_friction = 1.0f;
	float m_unprojScale = 10.0f;

	i32 m_ilegHit = -1;
	float m_distHit;
	CPhysicalEntity *m_pentGround = nullptr;
	i32 m_ipartGround;
	Vec3 m_ptlocGround, m_nlocGround;
	i32 m_matidGround;

	i32 m_nCollEnts = -1;
	CPhysicalEntity **m_pCollEnts;
	float m_moveDist = 0;
	float m_Eaux = 0;

	float m_massLegacy = 80;
	i32 m_matidLegacy = 0;
};

#endif
