// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef ropeentity_h
#define ropeentity_h

struct rope_vtx {
	Vec3 pt,pt0;
	Vec3 vel;
	Vec3 dir;
	Vec3 ncontact;
	Vec3 vcontact;
	float dP;
	CPhysicalEntity *pContactEnt;
	u32 kdP : 1;
	u32 iContactPart : 31;
};

struct rope_segment : rope_vtx {
	//~rope_segment() { if (pContactEnt) pContactEnt->Release(); }
	Vec3 vel_ext;
	Vec3 ptdst;
	i32 bRecheckContact : 1;
	i32 bRecalcDir : 1;
	i32 iCheckPart : 30;
	float tcontact;
	float vreq;
	i32 iPrim,iFeature;
	i32 iVtx0;
};

struct rope_vtx_attach {
	~rope_vtx_attach() { pent->Release(); }
	i32 ivtx;
	CPhysicalEntity *pent;
	i32 ipart;
	Vec3 ptloc;
	Vec3 pt,v;
};

struct SRopeCheckPart {
	Vec3 offset;
	Matrix33 R;
	float scale,rscale;
	box bbox;
	CPhysicalEntity *pent;
	i32 ipart;
	Vec3 pos0;
	quaternionf q0;
	CGeometry *pGeom;
	i32 bProcess;
	i32 bVtxUnproj;
	Vec3 v,w;
};

#ifdef DEBUG_ROPES
template<class T> struct safe_array {
	safe_array(i32 *_psizeDyn, i32 _sizeConst=0) { data=0; psizeDyn=_psizeDyn; sizeConst=_sizeConst; }
	safe_array& operator=(T* pdata) { data=pdata; return *this; }
	// cppcheck-suppress operatorEqVarError
	safe_array& operator=(const safe_array<T> &op) { data=op.data; return *this; }
	T& operator[](i32 idx) {
		if (idx<0 || idx>=*psizeDyn+sizeConst)
			__asm i32 3;
		return data[idx];
	}
	const T& operator[](i32 idx) const { return data[idx]; }
	operator T*() { return data; }
	T* data;
	i32 *psizeDyn,sizeConst;
};
#define ROPE_SAFE_ARRAY(T) safe_array<T>
#else
#define ROPE_SAFE_ARRAY(T) T*
#endif



class CRopeEntity : public CPhysicalEntity {
 public:
	explicit CRopeEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CRopeEntity();
	virtual pe_type GetType() const { return PE_ROPE; }

	virtual i32 SetParams(pe_params*,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params*) const;
	virtual i32 GetStatus(pe_status*) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual i32 Step(float time_interval);
	virtual i32 Awake(i32 bAwake=1,i32 iSource=0);
	virtual i32 IsAwake(i32 ipart=-1) const { return m_bPermanent; }
	virtual void AlertNeighbourhoodND(i32 mode);
	virtual i32 RayTrace(SRayTraceRes& rtr);
	virtual float GetMass(i32 ipart) { return m_mass/m_nSegs; }
	virtual float GetMassInv() { return 1E26f; }
	virtual RigidBody *GetRigidBodyData(RigidBody *pbody, i32 ipart=-1);
	virtual void GetLocTransform(i32 ipart, Vec3 &offs, quaternionf &q, float &scale, const CPhysicalPlaceholder *trg) const;
	void EnforceConstraints(float seglen, const quaternionf& qtv,const Vec3& offstv,float scaletv, i32 bTargetPoseActive, float dt=0);
	virtual void OnNeighbourSplit(CPhysicalEntity *pentOrig, CPhysicalEntity *pentNew);
	virtual i32 RegisterContacts(float time_interval,i32 nMaxPlaneContacts);
	virtual i32 Update(float time_interval, float damping);
	virtual float GetDamping(float time_interval) { return max(0.0f,1.0f-m_damping*time_interval); }
	virtual float CalcEnergy(float time_interval) { return time_interval>0 ? m_energy:0.0f; }
	virtual float GetLastTimeStep(float time_interval) { return m_lastTimeStep; }
	virtual void ApplyVolumetricPressure(const Vec3 &epicenter, float kr, float rmin);
	void RecalcBBox();

	void CheckCollisions(i32 iDir, SRopeCheckPart *checkParts,i32 nCheckParts, float seglen,float rseglen, const Vec3 &hingeAxis);
	void StepSubdivided(float time_interval, SRopeCheckPart *checkParts,i32 nCheckParts, float seglen);
	void ZeroLengthStraighten(float time_interval);
	float Solver(float time_interval, float seglen);
	void ApplyStiffness(float time_interval, i32 bTargetPoseActive, const quaternionf &qtv,const Vec3 &offstv,float scaletv);

	enum snapver { SNAPSHOT_VERSION = 8 };
	virtual i32 GetStateSnapshot(CStream &stm, float time_back=0,i32 flags=0);
	virtual i32 SetStateFromSnapshot(CStream &stm, i32 flags);
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0,i32 flags=0);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags);

	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	i32 GetVertices(strided_pointer<Vec3>& verts) const;
	virtual float GetExtent(EGeomForm eForm) const;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;

	Vec3 m_gravity,m_gravity0;
	float m_damping;
	float m_maxAllowedStep;
	float m_Emin;
	float m_timeStepPerformed,m_timeStepFull;
	i32 m_nSlowFrames;
	float m_lastTimeStep;

	i32 m_nSleepingNeighboursFrames;
	i32 m_bHasContacts;
	i32 m_bContactsRegistered;
	mutable  i32 m_lockVtx;
	 i32 m_lockAwake;

	float m_length;

	//Grouping these values as they are all used by CRopeEntity::GetStatus(). This avoids L2 cache line pollution
	i32 m_nSegs;
	ROPE_SAFE_ARRAY(rope_segment) m_segs;
	float m_timeLastActive;
	i32 m_bTargetPoseActive;
	float m_stiffnessAnim;
	Vec3 m_lastposHost;
	quaternionf m_lastqHost;

	float m_mass;
	float m_collDist;
	i32 m_surface_idx;
	float m_friction;
	float m_stiffness;
	float m_dampingAnim,m_stiffnessDecayAnim;
	Vec3 m_wind,m_wind0,m_wind1;
  float m_airResistance,m_windVariance,m_windTimer;
	float m_waterResistance,m_rdensity;
	float m_jointLimit,m_jointLimitDecay;
	float m_szSensor;
	float m_maxForce;
	i32 m_flagsCollider;
	i32 m_collTypes;
	float m_penaltyScale;
	i32 m_maxIters;
	float m_attachmentZone;
	float m_minSegLen;
	float m_unprojLimit;
	float m_noCollDist;

	CPhysicalEntity *m_pTiedTo[2];
	Vec3 m_ptTiedLoc[2];
	i32 m_iTiedPart[2];
	i32 m_idConstraint;
	i32 m_iConstraintClient;
	Vec3 m_posBody[2][2];
	quaternionf m_qBody[2][2];
	Vec3 m_dir0dst;
	Vec3 m_collBBox[2];

	ROPE_SAFE_ARRAY(rope_vtx) m_vtx;
	ROPE_SAFE_ARRAY(rope_vtx) m_vtx1;
	ROPE_SAFE_ARRAY(rope_solver_vtx) m_vtxSolver;
	i32 m_nVtx,m_nVtxAlloc,m_nVtx0;
	i32 m_nFragments;
	//class CTriMesh *m_pMesh;
	//Vec3 m_lastMeshOffs;
	i32 m_nMaxSubVtx;
	ROPE_SAFE_ARRAY(i32) m_idx;
	i32 m_bStrained;
	float m_frictionPull;
	float m_energy;
	entity_contact *m_pContact;

	rope_vtx_attach *m_attach;
	i32 m_nAttach;

	void MeshVtxUpdated();
	void AllocSubVtx();
	void FillVtxContactData(rope_vtx *pvtx,i32 iseg, SRopeCheckPart &cp, geom_contact *pcontact);
};

#endif
