// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef livingentity_h
#define livingentity_h
#pragma once

i32k SZ_ACTIONS = 128;
i32k SZ_HISTORY = 128;

struct le_history_item {
    Vec3 pos;
    Quat q;
    Vec3 v;
    i32 bFlying;
    Vec3 nslope;
    float timeFlying;
    float minFlyTime;
    float timeUseLowCap;
    i32 idCollider;
    i32 iColliderPart;
    Vec3 posColl;
    float dt;
};

struct le_contact {
    CPhysicalEntity *pent;
    i32 ipart;
    Vec3 pt, ptloc;
    Vec3 n;
    float penetration;
    Vec3 center;
    entity_contact *pSolverContact[2];
};

struct DRX_ALIGN(16) le_tmp_contact {
    CPhysicalEntity* pent;
    Vec3 ptcontact;
    Vec3 ncontact;
    i32 ipart;
    i32 idmat;
    i32 iPrim;
    float tmin;
};

struct DRX_ALIGN(16) le_precomp_part {
    Vec3 BBox[2];
    Quat partrot;
    Vec3 partoff;
    float partscale;
    i32 partflags;
    i32 ipart;
    i32 surface_idx;
    IGeometry* pgeom;
};

struct DRX_ALIGN(16) le_precomp_entity {
    Vec3 BBox[2];
    Vec3 sz;
    i32 entflags;
    float massinv;
    i32 bCheckBBox;
    i32 entType;
    i32 nParts;
    i32 iSimClass;
    i32 ignoreCollisionsWith;
    i32 iPartsBegin, iPartsEnd;
    CPhysicalEntity* pent;
};

struct le_precomp_data {
    le_precomp_entity *pents;
    le_precomp_part *pparts;
    i32 nPrecompEntsAlloc, nPrecompPartsAlloc;
};

struct SLivingEntityNetSerialize {
    Vec3 pos;
    Vec3 vel;
    Vec3 velRequested;
    void Serialize( TSerialize ser );
};

class CLivingEntity : public CPhysicalEntity {
public:
    explicit CLivingEntity(CPhysicalWorld *pWorld, IGeneralMemoryHeap* pHeap = NULL);
    virtual ~CLivingEntity();
    virtual pe_type GetType() const { return PE_LIVING; }

    virtual i32 SetParams(pe_params*,i32 bThreadSafe=1);
    virtual i32 GetParams(pe_params*) const;
    virtual i32 GetStatus(pe_status*) const;
    virtual i32 Action(pe_action*,i32 bThreadSafe=1);
    virtual void StartStep(float time_interval);
    virtual float GetMaxTimeStep(float time_interval);
    virtual i32 Step(float time_interval);
    //i32 StepBackEx(float time_interval, bool bRollbackHistory=true);
    virtual void StepBack(float time_interval) { /*StepBackEx(time_interval);*/ }
    virtual float CalcEnergy(float time_interval);
    virtual i32 RegisterContacts(float time_interval,i32 nMaxPlaneContacts);
    virtual i32 Update(float time_interval, float damping);
    virtual i32 Awake(i32 bAwake=1,i32 iSource=0);
    virtual void AlertNeighbourhoodND(i32 mode) { ReleaseGroundCollider(); CPhysicalEntity::AlertNeighbourhoodND(mode); }
    virtual void ComputeBBox(Vec3 *BBox, i32 flags=update_part_bboxes);
    virtual RigidBody *GetRigidBody(i32 ipart=-1,i32 bWillModify=0);
    virtual RigidBody *GetRigidBodyData(RigidBody *pbody, i32 ipart=-1) {
        pbody->zero(); pbody->M=m_mass; pbody->Minv=m_massinv;
        pbody->v=m_vel; pbody->pos=m_pos; return pbody;
    }
    virtual void OnContactResolved(entity_contact *pContact, i32 iop, i32 iGroupId);

    virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
    virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);

    virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);

    enum snapver { SNAPSHOT_VERSION = 2 };
    virtual i32 GetStateSnapshot(class CStream &stm, float time_back=0, i32 flags=0);
    virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
    virtual i32 SetStateFromSnapshot(class CStream &stm, i32 flags=0);
    virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags=0);

    virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

    virtual float GetMassInv() { return m_massinv; }
    virtual void GetSpatialContactMatrix(const Vec3 &pt, i32 ipart, float Ibuf[][6]) {
        Ibuf[3][0]+=m_massinv; Ibuf[4][1]+=m_massinv; Ibuf[5][2]+=m_massinv;
    }
    float ShootRayDown(le_precomp_entity* pents, i32 nents, le_precomp_part *pparts, const Vec3 &pos,
        Vec3 &nslope, float time_interval=0, bool bUseRotation=false, bool bUpdateGroundCollider=false,bool bIgnoreSmallObjects=true);
    void AddLegsImpulse(const Vec3 &vel, const Vec3 &nslope, bool bInstantChange);
    void ReleaseGroundCollider();
    void SetGroundCollider(CPhysicalEntity *pCollider, i32 bAcceptStatic=0);
    Vec3 SyncWithGroundCollider(float time_interval);
    void RegisterContact(const Vec3 &posSelf, const Vec3& pt,const Vec3& n, CPhysicalEntity *pCollider, i32 ipart,i32 idmat,
        float imp=0, i32 bLegsContact=0, i32 iPrim=-1, i32 ipartMin=0);
    void RegisterUnprojContact(const le_contact &unproj);
    float UnprojectionNeeded(const Vec3 &pos,const quaternionf &qrot, float hCollider,float hPivot, const Vec3 &newdim,i32 bCapsule,
        Vec3 &dirUnproj, i32 iCaller=get_iCaller()) const;

    //void AllocateExtendedHistory();

    void UpdatePosition(const Vec3 &pos, const Vec3 *BBox, i32 bGridLocked);
    void Step_HandleFlying(Vec3 &vel, const Vec3& velGround, i32 bWasFlying, const Vec3& heightAdj, const float kInertia, const float time_interval);
    void Step_HandleWasFlying(Vec3& vel, i32& bFlying, const Vec3& axis, i32k bGroundContact);
    void Step_HandleLivingEntity();

    Vec3 m_vel,m_velRequested,m_gravity,m_nslope;
    float m_dtRequested;
    float m_kInertia,m_kInertiaAccel,m_kAirControl,m_kAirResistance, m_hCyl,m_hEye,m_hPivot;
    Vec3 m_size;
    float m_dh,m_dhSpeed,m_dhAcc,m_stablehTime,m_hLatest,m_nodSpeed;
    float m_mass,m_massinv;
    i32 m_surface_idx;
    i32 m_lastGroundSurfaceIdx,m_lastGroundSurfaceIdxAux;
    i32 m_lastGroundPrim;
    float m_timeFlying,m_timeForceInertia;
    float m_slopeSlide,m_slopeClimb,m_slopeJump,m_slopeFall;
    float m_maxVelGround;
    float m_groundContactEps;
    float m_timeImpulseRecover;
    CCylinderGeom *m_pCylinderGeom;
    float m_hHead;
    CSphereGeom *m_pHeadGeom;
    phys_geometry m_CylinderGeomPhys;
    float m_timeUseLowCap;
    float m_timeSinceStanceChange;
    float m_timeSinceImpulseContact;
    //float m_dhHist[2],m_timeOnStairs;
    float m_timeStepFull,m_timeStepPerformed;
    i32 m_iSnapshot;
    i32 m_iTimeLastSend;
    i32 m_collTypes;
    CPhysicalEntity *m_pLivingEntToIgnore;

    u32 m_bFlying : 1;
    u32 m_bJumpRequested : 1;
    u32 m_bSwimming : 1;
    u32 m_bUseCapsule : 1;
    u32 m_bIgnoreCommands : 1;
    u32 m_bStateReading : 1;
    u32 m_bActive : 1;
    u32 m_bActiveEnvironment : 1;
    u32 m_bStuck : 1;
    u32 m_bReleaseGroundColliderWhenNotActive : 1;
    mutable u32 m_bHadCollisions : 1;
    i32 m_bSquashed;

    CPhysicalEntity *m_pLastGroundCollider;
    i32 m_iLastGroundColliderPart;
    Vec3 m_posLastGroundColl;
    Vec3 m_velGround;
    Vec3 m_deltaPos,m_posLocal;
    Vec3 m_deltaV;
    Quat m_deltaQRot;
    float m_timeSmooth;
    float m_timeRotChanged;

//  le_history_item *m_history,m_history_buf[4];
//  i32 m_szHistory,m_iHist;
//  pe_action_move *m_actions,m_actions_buf[16];
//  i32 m_szActions,m_iAction;
//  i32 m_iCurTime,m_iRequestedTime;

    le_contact *m_pContacts;
    i32 m_nContacts,m_nContactsAlloc;
    RigidBody *m_pBody;

    //Vec3 m_posLogged;
    //i32 m_timeLogged;

    mutable  i32 m_lockLiving;
    mutable  i32 m_lockStep;

    bool m_forceFly;

    static le_precomp_data s_precompData[MAX_PHYS_THREADS+1];
};

#endif
