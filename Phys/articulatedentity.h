// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef articulatedentity_h
#define articulatedentity_h
#pragma once

struct featherstone_data {
	Vec3i qidx2axidx,axidx2qidx;
	Vec3 Ya_vec[2];
	Vec3 dv_vec[2];
	Vec3 s_vec[3][2];
	Vec3 Q;
	float qinv[9];
	float qinv_down[9];
	float s[18];
	DRX_ALIGN(16) float Ia[6][6];
	float Ia_s[18];
	DRX_ALIGN(16) float Ia_s_qinv_sT[6][6];
	DRX_ALIGN(16) float s_qinv_sT[6][6];
	DRX_ALIGN(16) float s_qinv_sT_Ia[6][6];
	float qinv_sT[3][6];
	float qinv_sT_Ia[3][6];
	DRX_ALIGN(16) float Iinv[6][6];
	i32 useTree;
	i32 iparent;
	i32 jointSize;
	Matrix33 *pM0inv;
};

enum joint_flags_aux { joint_rotate_pivot=010000000 };

struct ae_joint : ArticulatedBody {
	ae_joint() {
		memset(this,0,sizeof(*this));
		iParent=-2; idbody=-1;
		MARK_UNUSED dq_req.x, dq_req.y, dq_req.z;
		limits[0].Set(-1E10,-1E10,-1E10);
		limits[1].Set(1E10,1E10,1E10);
		flags=all_angles_locked;
		quat0.SetIdentity();
		prev_qrot.SetIdentity();
		quat.SetIdentity();
		body.q.SetIdentity(); body.qfb.SetIdentity();
	}
	~ae_joint() {
		if (fsbuf) delete[] fsbuf;
	}

	Ang3 q;
	Ang3 qext;
	Vec3 dq;
	Vec3 dqext;
	Vec3 dq_req;
	Vec3 dq_limit;
	Vec3 ddq;
	quaternionf quat;

	Ang3 prev_q;
	Vec3 prev_dq;
	Vec3 prev_pos,prev_v,prev_w;
	quaternionf prev_qrot;
	Ang3 q0;
	Vec3 Fcollision,Tcollision;
	Vec3 vSleep,wSleep;

	u32 flags;
	quaternionf quat0;
	Vec3 limits[2];
	Vec3 bounciness;
	Vec3 ks,kd;
	Vec3 qdashpot,kdashpot;
	Vec3 pivot[2];

	i32 iStartPart,nParts;
	i32 iLevel;
	masktype selfCollMask;
	struct entity_contact *pContact;
	i32 bAwake;
	i32 bQuat0Changed;
	i32 bHasExtContacts;

	i32 idbody;
	Vec3 dv_body,dw_body;
	Vec3 Pimpact,Limpact;
	i32 nActiveAngles;
	Vec3 rotaxes[3];
	Matrix33 I;

	char *fsbuf;
};

struct ae_part_info {
	Vec3 pos;
	quaternionf q;
	float scale;
	Vec3 BBox[2];
	quaternionf q0;
	Vec3 pos0;
	i32 iJoint;
	i32 idbody;
	Vec3 posHist[2];
	quaternionf qHist[2];
};


class CArticulatedEntity : public CRigidEntity {
 public:
	explicit CArticulatedEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CArticulatedEntity();
	virtual pe_type GetType() const { return PE_ARTICULATED; }
	virtual void AlertNeighbourhoodND(i32 mode);

	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);
	virtual i32 SetParams(pe_params *_params,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params *_params) const;
	virtual i32 GetStatus(pe_status *_status) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);

	virtual RigidBody *GetRigidBody(i32 ipart=-1,i32 bWillModify=0);
	virtual RigidBody *GetRigidBodyData(RigidBody *pbody, i32 ipart=-1);
	virtual void GetLocTransformLerped(i32 ipart, Vec3 &offs, quaternionf &q, float &scale, float timeBack, const CPhysicalPlaceholder *trg) const;
	virtual void OnContactResolved(entity_contact *pcontact, i32 iop, i32 iGroupId);

	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	enum snapver { SNAPSHOT_VERSION = 6 };
	virtual i32 GetStateSnapshot(CStream &stm, float time_back=0,i32 flags=0);
	virtual i32 SetStateFromSnapshot(CStream &stm, i32 flags);
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags);

	virtual float GetMaxTimeStep(float time_interval);
	virtual i32 Step(float time_interval);
	virtual void StepBack(float time_interval);
	virtual i32 RegisterContacts(float time_interval,i32 nMaxPlaneContacts);
	virtual i32 Update(float time_interval, float damping);
	virtual float CalcEnergy(float time_interval);
	virtual float GetDamping(float time_interval);
	virtual void GetSleepSpeedChange(i32 ipart, Vec3 &v,Vec3 &w) { i32 i=m_infos[ipart].iJoint; v=m_joints[i].vSleep; w=m_joints[i].wSleep; }

	virtual i32 GetPotentialColliders(CPhysicalEntity **&pentlist, float dt=0);
	virtual i32 CheckSelfCollision(i32 ipart0,i32 ipart1);
	virtual i32 IsAwake(i32 ipart=-1) const;
	virtual void RecomputeMassDistribution(i32 ipart=-1,i32 bMassChanged=1);
	virtual void BreakableConstraintsUpdated();
	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);

	i32 SyncWithHost(i32 bRecalcJoints,float time_interval);
	void SyncBodyWithJoint(i32 idx, i32 flags=3);
	void SyncJointWithBody(i32 idx, i32 flags=1);
	void UpdateJointRotationAxes(i32 idx);
	void CheckForGimbalLock(i32 idx);
	i32 GetUnprojAxis(i32 idx, Vec3 &axis);

	i32 StepJoint(i32 idx, float time_interval,i32 &bBounced, i32 bFlying, i32 iCaller);
	void JointListUpdated();
	void StepFeatherstone(float time_interval, i32 bBounced, Matrix33 &M0host);
	i32 CalcBodyZa(i32 idx, float time_interval, vectornf &Za_change);
	i32 CalcBodyIa(i32 idx, matrixf& Ia_change, i32 bIncludeLimits=1);
	void CalcBodiesIinv(i32 bLockLimits);
	i32 CollectPendingImpulses(i32 idx,i32 &bNotZero,i32 bBounce=1);
	void PropagateImpulses(const Vec3 &dv,i32 bLockLimits=0,i32 bApplyVel=1,const Vec3 &dw=Vec3(ZERO));
	void CalcVelocityChanges(float time_interval, const Vec3 &dv,const Vec3 &dw);
	void GetJointTorqueResponseMatrix(i32 idx, Matrix33 &K);
	void UpdatePosition(i32 bGridLocked);
	void UpdateJointDyn();
	void AssignContactsToJoints();
	i32 UpdateHistory(i32 bStepDone) {
		if (bStepDone) {
			m_posHist[0]=m_posHist[1];m_posHist[1]=m_pos; m_qHist[0]=m_qHist[1];m_qHist[1]=m_qrot;
			for(i32 i=0;i<m_nParts;i++) {
				m_infos[i].posHist[0] = m_infos[i].posHist[1]; m_infos[i].posHist[1] = m_infos[i].pos;
				m_infos[i].qHist[0] = m_infos[i].qHist[1]; m_infos[i].qHist[1] = m_infos[i].q;
			}
			float rhistTime0 = m_rhistTime;
			m_rhistTime = 1.0f/max(m_timeStepFull,0.0001f);
			if (rhistTime0==0)
				UpdateHistory(1);
		}
		return bStepDone;
	}

	i32 IsChildOf(i32 idx, i32 iParent) { return isnonneg(iParent) & isneg(iParent-idx) & isneg(idx-iParent-m_joints[iParent].nChildrenTree-1); }
	entity_contact *CreateConstraintContact(i32 idx);
	void AllocFSData();

	ae_part_info *m_infos;
	ae_joint *m_joints;
	i32 m_nJoints, m_nJointsAlloc;
	Vec3 m_posPivot, m_offsPivot;
	Vec3 m_acc,m_wacc;
	Matrix33 m_M0inv;
	Vec3 m_Ya_vec[2];
	float m_simTime,m_simTimeAux;
	float m_scaleBounceResponse;
	i32 m_bGrounded;
	i32 m_nRoots;
	i32 m_bInheritVel;
	CPhysicalEntity *m_pHost;
	Vec3 m_posHostPivot;
	quaternionf m_qHostPivot;
	Vec3 m_velHost;
	Vec3 m_rootImpulse;
	i32 m_bCheckCollisions;
	i32 m_bFeatherstone;
	i32 m_bExertImpulse;
	i32 m_iSimType,m_iSimTypeLyingMode;
	i32 m_iSimTypeCur;
	i32 m_iSimTypeOverride;
	i32 m_bIaReady;
	i32 m_bPartPosForced;
	i32 m_bFastLimbs;
	float m_maxPenetrationCur;
	i32 m_bUsingUnproj;
	Vec3 m_prev_pos,m_prev_vel;
	i32 m_bUpdateBodies;
	i32 m_nDynContacts,m_bInGroup;
	i32 m_bIgnoreCommands;
	float m_Ejoints;

	i32 m_nCollLyingMode;
	Vec3 m_gravityLyingMode;
	float m_dampingLyingMode;
	float m_EminLyingMode;
	i32 m_nBodyContacts;

	Vec3 m_posHist[2];
	quaternionf m_qHist[2];
	float m_rhistTime;
	i32 m_bContactsAssigned;

	mutable  i32 m_lockJoints;

	CPhysicalEntity **m_pCollEntList;
	i32 m_nCollEnts;
};

#endif
