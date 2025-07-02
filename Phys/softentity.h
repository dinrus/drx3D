// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef softentity_h
#define softentity_h
#pragma once

enum sentity_flags_int {
	sef_volumetric = 0x08
};

struct se_vertex_base {
	Vec3 pos,vel;
	Vec3 pos0,vel0;
	float massinv,mass;
	Vec3 n,ncontact;
	i32 idx,idx0;
	float area;
	i32 iStartEdge : 16;
	i32 iEndEdge : 16;
	i32 bAttached : 8;
	i32 bFullFan : 2;
	i32 iCheckPart : 6;
	i32 iContactPart : 16;
	float rnEdges;
	CPhysicalEntity *pContactEnt;
	i32 iContactNode;
	Vec3 vcontact;
	float vreq;
	two_ints_in_one surface_idx;
	float angle0;
	Vec3 nmesh;
};

struct se_vertex : se_vertex_base {
	~se_vertex() { if (pContactEnt) pContactEnt->Release(); }
	Vec3 ptAttach;
	Vec3 posorg;
	i32 iSorted;
	//Vec3 P,dv,r,d;
};

struct se_edge {
	i32 ivtx[2];
	float len0;
	float len,rlen;
	float kmass;
	float angle0[2];
};

struct check_part {
	Vec3 offset;
	Matrix33 R;
	float scale,rscale;
	box bbox;
	CPhysicalEntity *pent;
	i32 ipart;
	Vec3 vbody,wbody;
	CGeometry *pGeom;
	i32 bPrimitive;
	i32 surface_idx;
	Vec3 P,L;
	Vec3 posBody;
	plane contPlane[8];
	float contRadius[8];
	float contDepth[8];
	i32 nCont;
};


class CSoftEntity : public CPhysicalEntity {
 public:
	explicit CSoftEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CSoftEntity();
  virtual pe_type GetType() const { return PE_SOFT; }

	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);
	virtual i32 SetParams(pe_params *_params,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params *_params) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);
	virtual i32 GetStatus(pe_status*) const;

	virtual i32 Awake(i32 bAwake=1,i32 iSource=0);
	virtual i32 IsAwake(i32 ipart=-1) const { return m_bPermanent; }
	virtual void AlertNeighbourhoodND(i32 mode);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual i32 Step(float time_interval);
	virtual i32 RayTrace(SRayTraceRes&);
	virtual void ApplyVolumetricPressure(const Vec3 &epicenter, float kr, float rmin);
	virtual float GetMass(i32 ipart) { return m_parts[0].mass/m_nVtx; }
	void StepInner(float time_interval, i32 bCollMode, check_part *checkParts,i32 nCheckParts,
		const plane &waterPlane,const Vec3 &waterFlow,float waterDensity, const Vec3 &lastposHost, const quaternionf &lastqHost, se_vertex *pvtx);

	void BakeCurrentPose();
	void AttachPoints(pe_action_attach_points *action, CPhysicalEntity *pent,i32 ipart, float rvtxmass,float vtxmass, i32 bAttached, const Vec3 &offs,const quaternionf &q);

	enum snapver { SNAPSHOT_VERSION = 10 };
	virtual i32 GetStateSnapshot(CStream &stm, float time_back=0,i32 flags=0);
	virtual i32 SetStateFromSnapshot(CStream &stm, i32 flags);
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags=0);

	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	void RemoveCore();

	se_vertex *m_vtx;
	se_edge *m_edges;
	i32 *m_pVtxEdges;
	i32 m_nVtx,m_nEdges;
	i32 m_nConnectedVtx;
	i32 m_nAttachedVtx;
	Vec3 m_offs0;
	quaternionf m_qrot0;
	i32 m_bMeshUpdated;
	Vec3 m_lastposHost;
	quaternionf m_lastqHost;
	i32 *m_pTetrEdges;
	i32 *m_pTetrQueue;
	Vec3 m_lastPos;

	float m_timeStepFull;
	float m_timeStepPerformed;
	float m_timeStepSurplus;

	Vec3 m_gravity;
	float m_Emin;
	float m_maxAllowedStep;
	i32 m_nSlowFrames;
	float m_damping;
	float m_accuracy;
	i32 m_nMaxIters;
	float m_prevTimeInterval;
	i32 m_bSkinReady;
	float m_maxMove;
	float m_maxAllowedDist;

	float m_thickness;
	float m_ks;
	float m_maxSafeStep;
	float m_density;
	float m_coverage;
	float m_friction;
	float m_impulseScale;
	float m_explosionScale;
	float m_collImpulseScale;
	float m_maxCollImpulse;
	i32 m_collTypes;
	float m_massDecay;
	float m_kShapeStiffnessNorm,m_kShapeStiffnessTang;
	float m_vtxvol;
	float m_stiffnessAnim,m_stiffnessDecayAnim,m_dampingAnim,m_maxDistAnim;
	float m_kRigid;
	float m_maxLevelDenom;

	float m_waterResistance;
	float m_airResistance;
	Vec3 m_wind;
	Vec3 m_wind0,m_wind1;
	float m_windTimer;
	float m_windVariance;

	class CRigidEntity *m_pCore;
	Vec3 m_pos0core;
	i32 *m_corevtx,m_nCoreVtx;

	i32 m_iLastLog;
	EventPhysPostStep *m_pEvent;

	mutable  i32 m_lockSoftBody;
};

#endif
