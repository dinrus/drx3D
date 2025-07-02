// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef particleentity_h
#define particleentity_h
#pragma once

class CParticleEntity : public CPhysicalEntity {
 public:
	explicit CParticleEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CParticleEntity();
	virtual pe_type GetType() const { return PE_PARTICLE; }

	virtual i32 SetParams(pe_params*,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params*) const;
	virtual i32 GetStatus(pe_status*) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);
	virtual i32 Awake(i32 bAwake=1,i32 iSource=0);
	virtual i32 IsAwake(i32 ipart=-1) const;
	virtual i32 RayTrace(SRayTraceRes&);
	virtual void ComputeBBox(Vec3 *BBox, i32 flags=update_part_bboxes) {
		Vec3 sz(m_dim,m_dim,m_dim); BBox[0]=m_pos-sz; BBox[1]=m_pos+sz;
	}
	virtual float GetMass(i32 ipart) { return m_mass; }
	virtual void AlertNeighbourhoodND(i32 mode) { if (m_pColliderToIgnore) m_pColliderToIgnore->Release(); m_pColliderToIgnore=0; }

	enum snapver { SNAPSHOT_VERSION = 3 };
	virtual i32 GetStateSnapshot(class CStream &stm,float time_back=0,i32 flags=0);
	virtual i32 SetStateFromSnapshot(class CStream &stm, i32 flags);

	virtual i32 GetStateSnapshot(TSerialize ser, float time_back, i32 flags);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual i32 Step(float time_interval) { return DoStep(time_interval); }
	virtual i32 DoStep(float time_interval,i32 iCaller=get_iCaller_int());

	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	float m_mass,m_dim,m_rdim,m_dimLying;
	float m_kAirResistance,m_kWaterResistance, m_accThrust, m_kAccLift;
	Vec3 m_gravity,m_gravity0,m_waterGravity,m_rollax,m_normal;
	Vec3 m_heading,m_vel,m_wspin;
	quaternionf m_qspin;
	float m_minBounceVel;
	float m_minVel;
	i32 m_surface_idx;
	CPhysicalEntity *m_pColliderToIgnore;
	i32 m_collTypes;
	Vec3 m_slide_normal;
	float m_timeSurplus;
	float m_depth;
	Vec3 m_velMedium;

	float m_timeStepPerformed,m_timeStepFull;
	float m_timeForceAwake;
	float m_sleepTime;

	u32 m_areaCheckPeriod : 8;
	u32 m_nStepCount : 8;
	mutable u32 m_bHadCollisions : 1;
	u32 m_bRecentCollisions : 3;
	u32 m_bForceAwake : 2;
	u32 m_iPierceability : 5;
	u32 m_bSliding : 1;
	u32 m_bDontPlayHitEffect : 1;

	mutable  i32 m_lockParticle;
	static CParticleEntity *g_pCurParticle[MAX_PHYS_THREADS+1];
};

#endif