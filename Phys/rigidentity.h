// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef rigidentity_h
#define rigidentity_h
#pragma once

typedef uint64 masktype;
#define getmask(i) ((uint64)1<<(i))
i32k NMASKBITS = 64;

enum constr_info_flags { constraint_limited_1axis=1, constraint_limited_2axes=2, constraint_rope=4, constraint_area=8, constraint_broken=0x10000 };

struct constraint_info {
	i32 id;
	quaternionf qframe_rel[2];
	Vec3 ptloc[2];
	float limits[2];
	u32 flags;
	float damping;
	float sensorRadius;
	CPhysicalEntity *pConstraintEnt;
	i32 bActive;
	float limit;
	float hardness;
};

struct checksum_item {
	i32 iPhysTime;
	u32 checksum;
};
i32k NCHECKSUMS = 1;

struct SRigidEntityNetSerialize
{
	Vec3 pos;
	Quat rot;
	Vec3 vel;
	Vec3 angvel;
	bool simclass;
#if USE_IMPROVED_RIGID_ENTITY_SYNCHRONISATION
	u8 sequenceNumber;
#endif

	void Serialize( TSerialize ser );
};

#if USE_IMPROVED_RIGID_ENTITY_SYNCHRONISATION

#define MAX_STATE_HISTORY_SNAPSHOTS 5
#define MAX_SEQUENCE_HISTORY_SNAPSHOTS 20

struct SRigidEntityNetStateHistory {
	SRigidEntityNetStateHistory()
		: numReceivedStates(0)
		, numReceivedSequences(0)
		, receivedStatesStart(0)
		, receivedSequencesStart(0)
		, paused(0)
		, sequenceDeltaAverage(0.0f)
	{
		ZeroArray(m_receivedStates);
		ZeroArray(m_receivedSequenceDeltas);
	}

	inline i32 GetNumReceivedStates() { return numReceivedStates; }
	inline i32 GetNumReceivedSequences() { return numReceivedSequences; }
	inline SRigidEntityNetSerialize& GetReceivedState(i32 index) { return m_receivedStates[(index + receivedStatesStart) % MAX_STATE_HISTORY_SNAPSHOTS]; }
	inline float GetAverageSequenceDelta() { return sequenceDeltaAverage; }
	void PushReceivedState(const SRigidEntityNetSerialize& item);
	void PushReceivedSequenceDelta(u8 delta);

	inline void Clear() {
		numReceivedStates = 0;
		numReceivedSequences = 0;
		receivedStatesStart = 0;
		receivedSequencesStart = 0;
		sequenceDeltaAverage = 0.0f;
	}
	inline int8 Paused() { return paused; }
	inline void SetPaused(int8 p) { paused = p; }

private:
	void UpdateSequenceDeltaAverage(u8 delta, i32 sampleCount);

	SRigidEntityNetSerialize m_receivedStates[MAX_STATE_HISTORY_SNAPSHOTS];
	u8 m_receivedSequenceDeltas[MAX_SEQUENCE_HISTORY_SNAPSHOTS];
	int8 numReceivedStates;
	int8 numReceivedSequences;
	int8 receivedStatesStart;
	int8 receivedSequencesStart;
	int8 paused;
	float sequenceDeltaAverage;
};
#endif

class CRigidEntity : public CPhysicalEntity {
 public:
	explicit CRigidEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CRigidEntity();
	virtual pe_type GetType() const { return PE_RIGID; }

	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);
	virtual i32 SetParams(pe_params *_params,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params *_params) const;
	virtual i32 GetStatus(pe_status*) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);

	virtual i32 AddCollider(CPhysicalEntity *pCollider);
	virtual i32 AddColliderNoLock(CPhysicalEntity *pCollider);
	virtual i32 RemoveCollider(CPhysicalEntity *pCollider, bool bRemoveAlways=true);
	virtual i32 RemoveColliderNoLock(CPhysicalEntity *pCollider, bool bRemoveAlways=true);
	virtual i32 RemoveContactPoint(CPhysicalEntity *pCollider, const Vec3 &pt, float mindist2);
	virtual i32 HasContactsWith(CPhysicalEntity *pent);
	virtual i32 HasPartContactsWith(CPhysicalEntity *pent, i32 ipart, i32 bGreaterOrEqual=0);
	virtual i32 HasCollisionContactsWith(CPhysicalEntity *pent);
	virtual i32 HasConstraintContactsWith(const CPhysicalEntity *pent, i32 flagsIgnore=0) const;
	virtual i32 Awake(i32 bAwake=1,i32 iSource=0);
	virtual i32 IsAwake(i32 ipart=-1) const { return m_bAwake; }
	virtual void AlertNeighbourhoodND(i32 mode);
	virtual void OnContactResolved(entity_contact *pContact, i32 iop, i32 iGroupId);

	virtual RigidBody *GetRigidBody(i32 ipart=-1,i32 bWillModify=0) { return &m_body; }
	virtual float GetMassInv() { return m_flags & aef_recorded_physics ? 0:m_body.Minv; }

	enum snapver { SNAPSHOT_VERSION = 9 };
	virtual i32 GetSnapshotVersion() { return SNAPSHOT_VERSION; }
	virtual i32 GetStateSnapshot(class CStream &stm, float time_back=0, i32 flags=0);
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
	virtual i32 SetStateFromSnapshot(class CStream &stm, i32 flags=0);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags);
	virtual i32 PostSetStateFromSnapshot();
	virtual u32 GetStateChecksum();
	virtual void SetNetworkAuthority(i32 authoritive, i32 paused);
	i32 WriteContacts(CStream &stm,i32 flags);
	i32 ReadContacts(CStream &stm,i32 flags);
	i32 WriteContacts(TSerialize ser);
	i32 ReadContacts(TSerialize ser);

	virtual void StartStep(float time_interval);
	virtual float GetMaxTimeStep(float time_interval);
	virtual float GetLastTimeStep(float time_interval) { return m_lastTimeStep; }
	virtual i32 Step(float time_interval);
	virtual void StepBack(float time_interval);
	virtual i32 GetContactCount(i32 nMaxPlaneContacts);
	virtual i32 RegisterContacts(float time_interval,i32 nMaxPlaneContacts);
	virtual i32 Update(float time_interval, float damping);
	virtual float CalcEnergy(float time_interval);
	virtual float GetDamping(float time_interval);
	virtual float GetMaxFriction() { return m_maxFriction; }
	virtual void GetSleepSpeedChange(i32 ipart, Vec3 &v,Vec3 &w) { v=m_vSleep; w=m_wSleep; }

	virtual bool OnSweepHit(geom_contact &cnt, i32 icnt, float &dt, Vec3 &vel, i32 &nsweeps) {
		if (m_nColliders)
			m_minFriction = 3.0f;
		return false;
	}
	virtual void CheckAdditionalGeometry(float time_interval) {}
	virtual void AddAdditionalImpulses(float time_interval) {}
	virtual void RecomputeMassDistribution(i32 ipart=-1,i32 bMassChanged=1);

	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	i32 RegisterConstraint(const Vec3 &pt0,const Vec3 &pt1, i32 ipart0, CPhysicalEntity *pBuddy,i32 ipart1, i32 flags,i32 flagsInfo=0, CPhysicalEntity* pConstraintEnt=0);
	i32 RemoveConstraint(i32 iConstraint);
	virtual void BreakableConstraintsUpdated();
	entity_contact *RegisterContactPoint(i32 idx, const Vec3 &pt, const geom_contact *pcontacts, i32 iPrim0,i32 iFeature0,
		i32 iPrim1,i32 iFeature1, i32 flags=contact_new, float penetration=0, i32 iCaller=get_iCaller_int(), const Vec3 &nloc=Vec3(ZERO));
	i32 CheckForNewContacts(geom_world_data *pgwd0,intersection_params *pip, i32 &itmax, Vec3 sweep=Vec3(0), i32 iStartPart=0,i32 nParts=-1, i32 *pFlagsAccum=0);
	virtual i32 GetPotentialColliders(CPhysicalEntity **&pentlist, float dt=0);
	virtual i32 CheckSelfCollision(i32 ipart0,i32 ipart1) { return 0; }
	void UpdatePenaltyContacts(float time_interval);
	i32 UpdatePenaltyContact(entity_contact *pContact, float time_interval);
	void VerifyExistingContacts(float maxdist);
	i32 EnforceConstraints(float time_interval);
	void UpdateConstraints(float time_interval);
	void UpdateContactsAfterStepBack(float time_interval);
	void ApplyBuoyancy(float time_interval,const Vec3 &gravity,pe_params_buoyancy *pb,i32 nBuoys);
	void ArchiveContact(entity_contact *pContact, float imp=0, i32 bLastInGroup=1, float r=0.0f);
	i32 CompactContactBlock(entity_contact *pContact,i32 endFlags, float maxPlaneDist, i32 nMaxContacts,i32 &nContacts,
		entity_contact *&pResContact, Vec3 &n,float &maxDist, const Vec3 &ptTest, const Vec3 &dirTest) const;
	void ComputeBBoxRE(coord_block_BBox *partCoord);
	void UpdatePosition(i32 bGridLocked);
	i32 PostStepNotify(float time_interval,pe_params_buoyancy *pb,i32 nMaxBuoys,i32 iCaller);
	masktype MaskIgnoredColliders(i32 iCaller, i32 bScheduleForStep=0);
	void UnmaskIgnoredColliders(masktype constraint_mask, i32 iCaller);
	void FakeRayCollision(CPhysicalEntity *pent, float dt);
	i32 ExtractConstraintInfo(i32 i, masktype constraintMask, pe_action_add_constraint &aac);
	EventPhysJointBroken &ReportConstraintBreak(EventPhysJointBroken &epjb, i32 i);
	virtual bool IgnoreCollisionsWith(const CPhysicalEntity *pent, i32 bCheckConstraints=0) const;
	virtual void OnNeighbourSplit(CPhysicalEntity *pentOrig, CPhysicalEntity *pentNew);

	void AttachContact(entity_contact *pContact, i32 i, CPhysicalEntity *pCollider);
	void DetachContact(entity_contact *pContact, i32 i=-1,i32 bCheckIfEmpty=1,i32 bAcquireContactLock=1);
	void DetachAllContacts();
	void MoveConstrainedObjects(const Vec3 &dpos, const quaternionf &dq);
	virtual void DetachPartContacts(i32 ipart,i32 iop0, CPhysicalEntity *pent,i32 iop1, i32 bCheckIfEmpty=1);
	void CapBodyVel();
	void CleanupAfterContactsCheck(i32 iCaller);
	void CheckContactConflicts(geom_contact *pcontacts, i32 ncontacts, i32 iCaller);
	void ProcessContactEvents(geom_contact* pcontact, i32 i, i32 iCaller);
	virtual void DelayedIntersect(geom_contact *pcontacts, i32 ncontacts, CPhysicalEntity **pColliders, i32 (*iCollParts)[2]);
	void ProcessCanopyContact(geom_contact *pcontacts, i32 i, float time_interval, i32 iCaller);
#if USE_IMPROVED_RIGID_ENTITY_SYNCHRONISATION
	float GetInterpSequenceNumber();
	bool GetInterpolatedState(float sequenceNumber, SRigidEntityNetSerialize &interpState);
	void UpdateStateFromNetwork();
	u8 GetLocalSequenceNumber() const;
#endif

	Vec3 m_posNew;
	quaternionf m_qNew;
	Vec3 m_BBoxNew[2];
	i32 m_iVarPart0;

	u32 m_bCollisionCulling     : 1;
	u32 m_bJustLoaded           : 8;
	u32 m_bStable               : 2;
	u32 m_bHadSeverePenetration : 1;
	u32 m_bSteppedBack          : 1;
	u32 m_nStepBackCount        : 4;
	u32 m_bCanSweep             : 1;
	u32 m_nNonRigidNeighbours   : 8;
	u32 m_bFloating             : 1;
	u32 m_bDisablePreCG				 : 1;
	u32 m_bSmallAndFastForced   : 1;

	u32 m_bAwake              : 8;
	u32 m_nSleepFrames        : 5;
	u32 m_nFutileUnprojFrames : 3;
	u32 m_nEvents             : 5;
	u32 m_nMaxEvents          : 5;
	u32 m_icollMin            : 5;
	u32 m_alwaysSweep         : 1;

	entity_contact **m_pColliderContacts;
	masktype *m_pColliderConstraints;
	entity_contact *m_pContactStart,*m_pContactEnd;
	i32 m_nContacts;
	entity_contact *m_pConstraints;
	constraint_info *m_pConstraintInfos;
	i32 m_nConstraintsAlloc;
	masktype m_constraintMask;
	u32 m_nRestMask;
	i32 m_nPrevColliders;
	i32 m_collTypes;

	float m_velFastDir,m_sizeFastDir;

	float m_timeStepFull;
	float m_timeStepPerformed;
	float m_lastTimeStep;
	float m_minAwakeTime;
	float m_nextTimeStep;

	Vec3 m_gravity,m_gravityFreefall;
	float m_Emin;
	float m_maxAllowedStep;
	Vec3 m_vAccum,m_wAccum;
	float m_damping,m_dampingFreefall;
	float m_dampingEx;
	float m_maxw;

	float m_minFriction,m_maxFriction;
	Vec3 m_vSleep,m_wSleep;
	entity_contact *m_pStableContact;

	RigidBody m_body;
	Vec3 m_Pext,m_Lext;
	Vec3 m_prevPos,m_prevv,m_prevw;
	quaternionf m_prevq;
	float m_E0,m_Estep;
	float m_timeCanopyFallen;
	u32 m_bCanopyContact : 8;
	u32 m_nCanopyContactsLost : 15;
	u32 m_sequenceOffset : 8;
	u32 m_hasAuthority : 1;
	Vec3 m_Psoft,m_Lsoft;
	Vec3 m_forcedMove;
	float m_sweepGap;

	EventPhysCollision **m_pEventsColl;
	i32 m_iLastLogColl;
	float m_vcollMin;
	i32 m_iLastLog;
	EventPhysPostStep *m_pEvent;

	float m_waterDamping;
	float m_kwaterDensity,m_kwaterResistance;
	float m_EminWater;
	float m_submergedFraction;

	i32 m_iLastConstraintIdx;
	 i32 m_lockConstraintIdx;
	 i32 m_lockContacts;
	 i32 m_lockStep;
	union {
		mutable  i32 m_lockNetInterp;
		i32 m_iLastChecksum;
	};
	union {
		checksum_item m_checksums[NCHECKSUMS];
		struct SRigidEntityNetStateHistory* m_pNetStateHistory;
	};
};

struct REdata {
	CPhysicalEntity *CurColliders[128];
	i32 CurCollParts[128][2];
	i32 idx0NoColl;
	i32 nLastContacts;
};
extern REdata g_REdata[];

#define g_CurColliders  g_REdata[iCaller].CurColliders
#define g_CurCollParts  g_REdata[iCaller].CurCollParts
#define g_idx0NoColl	  g_REdata[iCaller].idx0NoColl
#define g_nLastContacts g_REdata[iCaller].nLastContacts


#endif
