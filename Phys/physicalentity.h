// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef physicalentity_h
#define physicalentity_h
#pragma once

#include <drx3D/CoreX/Math/GeomQuery.h>

struct SRayTraceRes;

#include <drx3D/Network/ISerialize.h>

enum phentity_flags_int {
	pef_use_geom_callbacks = 0x20000000,
	sef_skeleton = 0x04
};

struct coord_block {
	Vec3 pos;
	quaternionf q;
};

struct coord_block_BBox {
	Vec3 pos;
	quaternionf q;
	float scale;
	Vec3 BBox[2];
};

enum cbbx_flags { part_added=1, update_part_bboxes=2 };	// for ComputeBBox

enum geom_flags_aux { geom_car_wheel=0x40000000, geom_invalid=0x20000000, geom_removed=0x10000000, geom_will_be_destroyed=0x8000000,
											geom_constraint_on_break=geom_proxy };

class CTetrLattice;

struct geom {
	Vec3 pos;
	quaternionf q;
	float scale;
	Vec3 BBox[2];
	phys_geometry *pPhysGeom,*pPhysGeomProxy;
	i32 id;
	float mass;
	u32 flags,flagsCollider;
	float maxdim;
	float minContactDist;
	i32 surface_idx : 10;
	i32 idmatBreakable : 14;
	u32 nMats : 7;
	i32 bChunkStart : 1;
	i32 *pMatMapping;
	union {
		CTetrLattice *pLattice;
		geom *next;
	};
	union {
		coord_block_BBox *pNewCoords;
		geom *prev;
	};
	CPhysicalPlaceholder *pPlaceholder;
};

struct SStructuralJoint {
	i32 id;
	i32 ipart[2];
	Vec3 pt;
	Vec3 n,axisx;
	float maxForcePush,maxForcePull,maxForceShift;
	float maxTorqueBend,maxTorqueTwist;
  float damageAccum, damageAccumThresh;
	Vec3 limitConstr;
	float dampingConstr;
	i32 bBreakable,bBroken;
	float size;
	float tension;
	i32 itens;
	Vec3 Paccum,Laccum;
};

struct SSkinInfo {
	i32 itri;
	float w[4];
};

struct SSkelInfo {
	SSkinInfo *pSkinInfo;
	CPhysicalEntity *pSkelEnt;
	i32 idSkel;
	float timeStep;
	i32 nSteps;
	float maxImpulse;
	float lastUpdateTime;
	Vec3 lastUpdatePos;
	Quat lastUpdateq;
	float lastCollTime;
	float lastCollImpulse;
};

struct SPartInfo {
	Vec3 Pext,Lext;
	Vec3 Fext,Text;
	Vec3 initialVel; // override for clients
	Vec3 initialAngVel;
	i32 iParent;
	i32 bGroup;
	i32 flags0,flagsCollider0;
};

struct SStructureInfo {
	SStructuralJoint *pJoints;
	SPartInfo *pParts;
	i32 nJoints,nJointsAlloc;
	i32 nLastBrokenJoints;
	i32 bModified;
	i32 idpartBreakOrg;
	i32 nPartsAlloc;
	float timeLastUpdate;
	i32 nPrevJoints;
	i32 nLastUsedJoints;
	float prevdt;
	float minSnapshotTime;
	float autoDetachmentDist;
	Vec3 lastExplPos;
	Vec3 *Pexpl,*Lexpl;
	SSkelInfo *defparts;
	i32 bTestRun;
	i32 bHasDirectBreaks;
};

struct SPartHelper {
	i32 idx;
	float Minv;
	Matrix33 Iinv;
	Vec3 v,w;
	Vec3 org;
	i32 bProcessed;
	i32 ijoint0;
	i32 isle;
	CPhysicalEntity *pent;
};

struct SStructuralJointHelper {
	i32 idx;
	Vec3 r0,r1;
	Matrix33 vKinv,wKinv;
	Vec3 P,L;
	i32 bBroken;
};
struct SStructuralJointDebugHelper {
	i32 itens;
	quotientf tension;
};

struct SExplosionInfo {
	Vec3 center;
	Vec3 dir;
	float rmin,kr;
};

class CPhysicalWorld;
class CRayGeom;
class CGeometry;

class CPhysicalEntity : public CPhysicalPlaceholder {
public:
	static void CleanupGlobalState();

public:
	uk operator new (size_t sz, uk p) { return p; }
	void operator delete (uk p, uk q) {}

	// Not supported, use Create/Delete instead
	uk operator new (size_t sz) throw() { return NULL; }
	void operator delete (uk p) { __debugbreak(); }

	template <typename T>
	static T* Create(CPhysicalWorld* pWorld, IGeneralMemoryHeap* pHeap) {
		uk p = NULL;
		if (pHeap)
			p = pHeap->Malloc(sizeof(T), "Physical Entity");
		else
			p = DrxModuleMalloc(sizeof(T));
		return p
			? new (p) T(pWorld, pHeap)
			: NULL;
	}

public:
	explicit CPhysicalEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CPhysicalEntity();
	virtual pe_type GetType() const { return PE_STATIC; }
	virtual bool IsPlaceholder() const { return false; }

	virtual void Delete();

	virtual i32 AddRef();
	virtual i32 Release();

	virtual i32 SetParams(pe_params*,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params*) const;
	virtual i32 GetStatus(pe_status*) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);
	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);
	virtual float GetExtent(EGeomForm eForm) const;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;
	virtual IPhysicalWorld *GetWorld() const { return (IPhysicalWorld*)m_pWorld; }
  virtual CPhysicalEntity *GetEntity();
	virtual CPhysicalEntity *GetEntityFast() { return this; }

	virtual void StartStep(float time_interval) {}
	virtual float GetMaxTimeStep(float time_interval) { return time_interval; }
	virtual float GetLastTimeStep(float time_interval) { return time_interval; }
	virtual i32 Step(float time_interval) { return 1; }
	virtual i32 DoStep(float time_interval, i32 iCaller=0) { return Step(time_interval); }
	virtual void StepBack(float time_interval) {}
	virtual i32 GetContactCount(i32 nMaxPlaneContacts) { return 0; }
	virtual i32 RegisterContacts(float time_interval,i32 nMaxPlaneContacts) { return 0; }
	virtual i32 Update(float time_interval, float damping) { return 1; }
	virtual float CalcEnergy(float time_interval) { return 0; }
	virtual float GetDamping(float time_interval) { return 1.0f; }
	virtual float GetMaxFriction() { return 100.0f; }
	virtual bool IgnoreCollisionsWith(const CPhysicalEntity *pent, i32 bCheckConstraints=0) const { return false; }
	virtual void GetSleepSpeedChange(i32 ipart, Vec3 &v,Vec3 &w) { v.zero(); w.zero(); }

	virtual i32 AddCollider(CPhysicalEntity *pCollider);
	virtual i32 AddColliderNoLock(CPhysicalEntity *pCollider) { return AddCollider(pCollider); }
	virtual i32 RemoveCollider(CPhysicalEntity *pCollider, bool bAlwaysRemove=true);
	virtual i32 RemoveColliderNoLock(CPhysicalEntity *pCollider, bool bAlwaysRemove=true) { return RemoveCollider(pCollider,bAlwaysRemove); }
	i32 RemoveColliderMono(CPhysicalEntity *pCollider, bool bAlwaysRemove=true) { WriteLock lock(m_lockColliders); return RemoveColliderNoLock(pCollider,bAlwaysRemove); }
	virtual i32 RemoveContactPoint(CPhysicalEntity *pCollider, const Vec3 &pt, float mindist2) { return -1; }
	virtual i32 HasContactsWith(CPhysicalEntity *pent) { return 0; }
	virtual i32 HasPartContactsWith(CPhysicalEntity *pent, i32 ipart, i32 bGreaterOrEqual=0) { return 1; }
	virtual i32 HasCollisionContactsWith(CPhysicalEntity *pent) { return 0; }
	virtual i32 HasConstraintContactsWith(const CPhysicalEntity *pent, i32 flagsIgnore=0) const { return 0; }
	virtual void AlertNeighbourhoodND(i32 mode);
	virtual i32 Awake(i32 bAwake=1,i32 iSource=0) { return 0; }
	virtual i32 IsAwake(i32 ipart=-1) const { return 0; }
	i32 GetColliders(CPhysicalEntity **&pentlist) { pentlist=m_pColliders; return m_nColliders; }
  virtual i32 RayTrace(SRayTraceRes&);
	virtual void ApplyVolumetricPressure(const Vec3 &epicenter, float kr, float rmin) {}
	virtual void OnContactResolved(struct entity_contact *pContact, i32 iop, i32 iGroupId);
	virtual void DelayedIntersect(geom_contact *pcontacts, i32 ncontacts, CPhysicalEntity **pColliders, i32 (*iCollParts)[2]) {}

	virtual void OnNeighbourSplit(CPhysicalEntity *pentOrig, CPhysicalEntity *pentNew) {}
	virtual void OnStructureUpdate() {}

	virtual class RigidBody *GetRigidBody(i32 ipart=-1,i32 bWillModify=0);
	virtual class RigidBody *GetRigidBodyData(RigidBody *pbody, i32 ipart=-1) { return GetRigidBody(ipart); }
	inline class RigidBody *GetRigidBodyTrans(RigidBody *pbody, i32 ipart, CPhysicalEntity *trg, i32 type=0, bool needIinv=false);
	virtual float GetMass(i32 ipart) { return m_parts[ipart].mass; }
	virtual void GetSpatialContactMatrix(const Vec3 &pt, i32 ipart, float Ibuf[][6]) {}
	virtual float GetMassInv() { return 0; }
	virtual i32 IsPointInside(Vec3 pt) const;
	template<typename Rot> void GetPartTransform(i32 ipart, Vec3 &offs, Rot &R, float &scale, const CPhysicalPlaceholder *trg) const;
	template<typename CTrg> Vec3* GetPartBBox(i32 ipart, Vec3* BBox, const CTrg *trg) const;
	virtual void GetLocTransform(i32 ipart, Vec3 &offs, quaternionf &q, float &scale, const CPhysicalPlaceholder *trg) const;
	virtual void GetLocTransformLerped(i32 ipart, Vec3 &offs, quaternionf &q, float &scale, float timeBack, const CPhysicalPlaceholder *trg) const { GetLocTransform(ipart,offs,q,scale,trg); }
	virtual void DetachPartContacts(i32 ipart,i32 iop0, CPhysicalEntity *pent,i32 iop1, i32 bCheckIfEmpty=1) {}
	i32 TouchesSphere(const Vec3 &center, float r);

	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);
	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	virtual i32 GetStateSnapshot(class CStream &stm, float time_back=0,	i32 flags=0) { return 0; }
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
	virtual i32 SetStateFromSnapshot(class CStream &stm, i32 flags=0) { return 0; }
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags=0);
	virtual i32 SetStateFromTypedSnapshot(TSerialize ser, i32 iSnapshotType, i32 flags=0);
	virtual i32 PostSetStateFromSnapshot() { return 1; }
	virtual u32 GetStateChecksum() { return 0; }
	virtual i32 GetStateSnapshotTxt(char *txtbuf,i32 szbuf, float time_back=0);
	virtual void SetStateFromSnapshotTxt(tukk txtbuf,i32 szbuf);
	virtual void SetNetworkAuthority(i32 authoritive, i32 paused) {}

	void AllocStructureInfo();
	i32 GenerateJoints();
	i32 UpdateStructure(float time_interval, pe_explosion *pexpl, i32 iCaller=0, Vec3 gravity=Vec3(0));
	void RemoveBrokenParent(i32 i, i32 nIsles);
	i32 MapHitPointFromParent(i32 i, const Vec3 &pt);
	virtual void RecomputeMassDistribution(i32 ipart=-1,i32 bMassChanged=1) {}

	virtual void ComputeBBox(Vec3 *BBox, i32 flags=update_part_bboxes);
	void WriteBBox(Vec3 *BBox) {
		m_BBox[0] = BBox[0]; m_BBox[1] = BBox[1];
		if (m_pEntBuddy) {
			m_pEntBuddy->m_BBox[0] = BBox[0];
			m_pEntBuddy->m_BBox[1] = BBox[1];
		}
	}

	bool OccupiesEntityGridSquare(const AABB &bbox);

	void UpdatePartIdmatBreakable(i32 ipart, i32 nParts=-1);
	i32 CapsulizePart(i32 ipart);

	i32 GetMatId(i32 id,i32 ipart) {
		if (ipart<m_nParts) {
			id += m_parts[ipart].surface_idx-id & id>>31;
			intptr_t mask = iszero_mask(m_parts[ipart].pMatMapping);
			i32 idummy=0, *pMatMapping = (i32*)((intptr_t)m_parts[ipart].pMatMapping & ~mask | (intptr_t)&idummy & mask), nMats;
			nMats = m_parts[ipart].nMats + (65536 & (i32)mask);
			return id & (i32)mask | pMatMapping[id & ~(i32)mask & (id & ~(i32)mask)-nMats>>31];
		} else
			return id;
	}

	bool MakeDeformable(i32 ipart, i32 iskel, float r=0.0f);
	void UpdateDeformablePart(i32 ipart);

	static  i32 g_lockProcessedAux;
	i32 GetCheckPart(i32 ipart)	{
		i32 i,j;
		if (m_nParts<=24) {
			i = (m_bProcessed_aux&0xFFFFFF) & (1<<ipart)-1;
			return (m_bProcessed_aux>>24) + bitcount(i) | (m_bProcessed_aux>>ipart & 1)-1;
		}	else {
			if (!(m_parts[ipart].flags & geom_removed))
				return -1;
			for(i=j=0;i<ipart;i++)
				j += iszero((i32)m_parts[i].flags & geom_removed)^1;
			return (m_bProcessed_aux>>24)+j;
		}
	}

	void RepositionParts();
	i32 GetUsedPartsCount(i32 iCaller) { i32 n = m_nUsedParts>>iCaller*4 & 15; return n + (m_nParts-n & (14-n|n-1)>>31); }
	i32 GetUsedPart(i32 iCaller,i32 i) {
		i32 n = m_nUsedParts>>iCaller*4 & 15;
		return (14-n|n-1)<0 ? i : m_pUsedParts[iCaller][i & 15];
	}
	CPhysicalPlaceholder *ReleasePartPlaceholder(i32 i);
	i32 m_iDeletionTime;
	 i32 m_nRefCount;
	u32 m_flags;
	CPhysicalEntity *m_next,*m_prev;
	CPhysicalWorld *m_pWorld;
	IGeneralMemoryHeap* m_pHeap;
	i32 m_nRefCountPOD   : 16;
	i32 m_iLastPODUpdate : 16;

	i32 m_iPrevSimClass : 16;
	i32 m_bMoved        : 8;
	i32 m_bPermanent		: 4;
	i32 m_bPrevPermanent: 4;
	i32 m_iGroup;
	CPhysicalEntity *m_next_coll,*m_next_coll1,*m_next_coll2;

	Vec3 m_pos;
	quaternionf m_qrot;
	coord_block *m_pNewCoords;

	CPhysicalEntity **m_pColliders;
	i32 m_nColliders,m_nCollidersAlloc;
	mutable  i32 m_lockColliders;

	CPhysicalEntity *m_pOuterEntity;
	i32 m_bProcessed_aux;

	SCollisionClass m_collisionClass;

	float m_timeIdle,m_maxTimeIdle;

	float m_timeStructUpdate;
	i32 m_updStage,m_nUpdTicks;
	SExplosionInfo *m_pExpl;

	geom *m_parts;
	static geom m_defpart;
	i32 m_nParts,m_nPartsAlloc;
	i32 m_iLastIdx;
	 i32 m_lockPartIdx;
	mutable CGeomExtents m_Extents;

	plane *m_ground;
	i32 m_nGroundPlanes;

	i32 (*m_pUsedParts)[16];
	 u32 m_nUsedParts;

	CPhysicalPlaceholder *m_pLastPortal = nullptr;
	CPhysicalPlaceholder *GetLastPortal() const { return m_pLastPortal; }
	void SetLastPortal(CPhysicalPlaceholder *portal) { m_pLastPortal = portal; }

	SStructureInfo *m_pStructure;
	static SPartHelper *g_parts;
	static SStructuralJointHelper *g_joints;
	static SStructuralJointDebugHelper *g_jointsDbg;
	static i32 *g_jointidx;
	static i32 g_nPartsAlloc,g_nJointsAlloc;
};

extern RigidBody g_StaticRigidBodies[];
#define g_StaticRigidBody (g_StaticRigidBodies[0])
extern CPhysicalEntity g_StaticPhysicalEntity;

template<class T> i32 GetStructSize(T *pstruct);
template<class T> subref *GetSubref(T *pstruct);
template<class T> i32 GetStructId(T *pstruct);
template<class T> bool AllowChangesOnDeleted(T *pstruct) { return false; }

extern i32 g_szParams[],g_szAction[],g_szGeomParams[];
extern subref *g_subrefParams[],*g_subrefAction[],*g_subrefGeomParams[];

inline i32 GetStructSize(pe_params *params) { return g_szParams[params->type]; }
inline i32 GetStructSize(pe_action *action) { return g_szAction[action->type]; }
inline i32 GetStructSize(pe_geomparams *geomparams) { return g_szGeomParams[geomparams->type]; }
inline i32 GetStructSize(uk ) { return 0; }
inline subref *GetSubref(pe_params *params) { return g_subrefParams[params->type]; }
inline subref *GetSubref(pe_action *action) { return g_subrefAction[action->type]; }
inline subref *GetSubref(pe_geomparams *geomparams) { return g_subrefGeomParams[geomparams->type]; }
inline subref *GetSubref(uk ) { return 0; }
inline i32 GetStructId(pe_params*) { return 0; }
inline i32 GetStructId(pe_action*) { return 1; }
inline i32 GetStructId(pe_geomparams*) { return 2; }
inline i32 GetStructId(uk ) { return 3; }
inline bool StructUsesAuxData(pe_params*) { return false; }
inline bool StructUsesAuxData(pe_action*) { return false; }
inline bool StructUsesAuxData(pe_geomparams*) { return true; }
inline bool StructUsesAuxData(uk ) { return true; }
inline bool AllowChangesOnDeleted(pe_params *params) { return params->type==pe_params_foreign_data::type_id || params->type==pe_player_dynamics::type_id || params->type==pe_player_dimensions::type_id || params->type==pe_params_collision_class::type_id; }
inline bool AllowChangesOnDeleted(pe_geomparams *geomparams) { return geomparams->type==pe_articgeomparams::type_id; }

#endif
