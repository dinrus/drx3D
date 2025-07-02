// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef physicalworld_h
#define physicalworld_h
#pragma once

#include "../../DinrusXPhys/geoman.h"
#include <drx3D/Network/ISerialize.h>


class PhysXEnt;
class PhysXProjectile;

i32k NSURFACETYPES = 512;

class PhysXWorld :public IPhysicalWorld, public IPhysUtils, public CGeomUpr, public PxSimulationEventCallback
{

public:

    PhysXWorld(ILog* pLog);
    virtual ~PhysXWorld() {}

    virtual i32 CoverPolygonWithCircles(strided_pointer<Vec2> pt,i32 npt,bool bConsecutive, const Vec2 &center, Vec2 *&centers,float *&radii, float minCircleRadius)
    { return ::CoverPolygonWithCircles(pt,npt,bConsecutive, center, centers,radii, minCircleRadius); }
    virtual void DeletePointer(uk pdata) { if (pdata) delete[] (tuk)pdata; }
    virtual i32 qhull(strided_pointer<Vec3> pts, i32 npts, index_t*& pTris, qhullmalloc qmalloc = 0) { return ::qhull(pts,npts,pTris, qmalloc); }
    virtual i32 qhull2d(ptitem2d *pts, i32 nVtx, edgeitem *edges, i32 nMaxEdges = 0) { return ::qhull2d(pts, nVtx, edges, nMaxEdges); }
    virtual i32 TriangulatePoly(Vec2 *pVtx, i32 nVtx, i32 *pTris,i32 szTriBuf) { return ::TriangulatePoly(pVtx, nVtx, pTris, szTriBuf); }

    virtual void          Init() {}
    virtual void          Shutdown(i32 bDeleteGeometries = 1) {}
    virtual void          Release();

    virtual IGeomUpr* GetGeomUpr() { return this; }
    virtual IPhysUtils*   GetPhysUtils() { return this; }

    virtual IPhysicalEntity* SetupEntityGrid(i32 axisz, Vec3 org, i32 nx,i32 ny, float stepx,float stepy, i32 log2PODscale=0, i32 bCyclic=0,
        IPhysicalEntity* pHost=nullptr, const QuatT& posInHost=QuatT(IDENTITY));
    virtual void Cleanup() {}
    virtual void RegisterBBoxInPODGrid(const Vec3* BBox) {}
    virtual void UnregisterBBoxInPODGrid(const Vec3* BBox) {}
    virtual void DeactivateOnDemandGrid() {}
    virtual i32 AddRefEntInPODGrid(IPhysicalEntity* pent, const Vec3* BBox = 0) { return 0; }

    virtual IPhysicalEntity* SetHeightfieldData(const primitives::heightfield* phf, i32* pMatMapping = 0, i32 nMats = 0);
    virtual IPhysicalEntity* GetHeightfieldData(primitives::heightfield* phf) { DRX_PHYSX_LOG_FUNCTION; _RETURN_PTR_DUMMY_; }
    virtual void             SetHeightfieldMatMapping(i32* pMatMapping, i32 nMats) { DRX_PHYSX_LOG_FUNCTION; }
    virtual PhysicsVars*     GetPhysVars() { return &m_vars; }

    virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, pe_params* params = 0, uk pforeigndata = 0, i32 iforeigndata = 0, i32 id = -1, IGeneralMemoryHeap* pHeap = NULL)
    {
        return CreatePhysicalEntity(type, 0.0f, params, pforeigndata, iforeigndata, id, NULL, pHeap);
    }
    virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, float lifeTime, pe_params* params = 0, uk pForeignData = 0, i32 iForeignData = 0, i32 id = -1, IPhysicalEntity* pHostPlaceholder = 0, IGeneralMemoryHeap* pHeap = NULL);
    virtual IPhysicalEntity* CreatePhysicalPlaceholder(pe_type type, pe_params* params = 0, uk pForeignData = 0, i32 iForeignData = 0, i32 id = -1);
    virtual i32              DestroyPhysicalEntity(IPhysicalEntity* pent, i32 mode = 0, i32 bThreadSafe = 0);

    virtual i32              SetPhysicalEntityId(IPhysicalEntity* pent, i32 id, i32 bReplace = 1, i32 bThreadSafe = 0) { return 0; }
    virtual i32              GetPhysicalEntityId(IPhysicalEntity* pent);
    virtual IPhysicalEntity* GetPhysicalEntityById(i32 id);
    i32                      GetFreeEntId();

    virtual i32 SetSurfaceParameters(i32 surface_idx, float bounciness, float friction, u32 flags = 0);
    virtual i32 GetSurfaceParameters(i32 surface_idx, float& bounciness, float& friction, u32& flags);
    virtual i32 SetSurfaceParameters(i32 surface_idx, float bounciness, float friction, float damage_reduction, float ric_angle, float ric_dam_reduction, float ric_vel_reduction, u32 flags = 0)
    { return SetSurfaceParameters(surface_idx,bounciness,friction,flags); }
    virtual i32 GetSurfaceParameters(i32 surface_idx, float& bounciness, float& friction, float& damage_reduction, float& ric_angle, float& ric_dam_reduction, float& ric_vel_reduction, u32& flags)
    { return GetSurfaceParameters(surface_idx,bounciness,friction,flags); }

    virtual void  TimeStep(float time_interval, i32 flags = ent_all | ent_deleted);

    virtual float GetPhysicsTime() { return m_time; }
    virtual i32   GetiPhysicsTime() { return float2int(m_time/m_vars.timeGranularity); }
    virtual void  SetPhysicsTime(float time) { m_time=time; }
    virtual void  SetiPhysicsTime(i32 itime) { DRX_PHYSX_LOG_FUNCTION; }

    virtual void  SetSnapshotTime(float time_snapshot, i32 iType = 0) { DRX_PHYSX_LOG_FUNCTION; }
    virtual void  SetiSnapshotTime(i32 itime_snapshot, i32 iType = 0) { DRX_PHYSX_LOG_FUNCTION; }

    virtual i32 GetEntitiesInBox(Vec3 ptmin, Vec3 ptmax, IPhysicalEntity**& pList, i32 objtypes, i32 szListPrealloc = 0);

    virtual i32 RayWorldIntersection(const SRWIParams& rp, tukk pNameTag = RWI_NAME_TAG, i32 iCaller = MAX_PHYS_THREADS);
    virtual i32  TracePendingRays(i32 bDoActualTracing = 1);

    virtual void ResetDynamicEntities();
    virtual void               DestroyDynamicEntities() { DRX_PHYSX_LOG_FUNCTION; }

    virtual void               PurgeDeletedEntities() { DRX_PHYSX_LOG_FUNCTION; } //!< Forces immediate physical deletion of all entities marked as deleted
    virtual i32                GetEntityCount(i32 iEntType) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; } //!< iEntType is of pe_type
    virtual i32                ReserveEntityCount(i32 nExtraEnts) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; } //!< can prevent excessive internal lists re-allocations

    virtual IPhysicalEntityIt* GetEntitiesIterator() { DRX_PHYSX_LOG_FUNCTION; _RETURN_PTR_DUMMY_; }

    virtual void               SimulateExplosion(pe_explosion* pexpl, IPhysicalEntity** pSkipEnts = 0, i32 nSkipEnts = 0, i32 iTypes = ent_rigid | ent_sleeping_rigid | ent_living | ent_independent, i32 iCaller = MAX_PHYS_THREADS);

    virtual void RasterizeEntities(const primitives::grid3d& grid, uchar* rbuf, i32 objtypes, float massThreshold, const Vec3& offsBBox, const Vec3& sizeBBox, i32 flags) { DRX_PHYSX_LOG_FUNCTION; }

    virtual i32   DeformPhysicalEntity(IPhysicalEntity* pent, const Vec3& ptHit, const Vec3& dirHit, float r, i32 flags = 0) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; }
    virtual void  UpdateDeformingEntities(float time_interval = 0.01f) { DRX_PHYSX_LOG_FUNCTION; } //!< normally this happens during TimeStep
    virtual float CalculateExplosionExposure(pe_explosion* pexpl, IPhysicalEntity* pient) { DRX_PHYSX_LOG_FUNCTION; _RETURN_FLOAT_DUMMY_; }

    virtual float IsAffectedByExplosion(IPhysicalEntity* pent, Vec3* impulse = 0) { DRX_PHYSX_LOG_FUNCTION; _RETURN_FLOAT_DUMMY_; }

    virtual i32  AddExplosionShape(IGeometry* pGeom, float size, i32 idmat, float probability = 1.0f) { return 0; }
    virtual void RemoveExplosionShape(i32 id) { DRX_PHYSX_LOG_FUNCTION; }
    virtual void RemoveAllExplosionShapes(void(*OnRemoveGeom)(IGeometry* pGeom) = 0) { DRX_PHYSX_LOG_FUNCTION; }

    virtual void DrawPhysicsHelperInformation(IPhysRenderer* pRenderer, i32 iCaller = MAX_PHYS_THREADS);
    virtual void DrawEntityHelperInformation(IPhysRenderer* pRenderer, i32 iEntityId, i32 iDrawHelpers);

    virtual i32   CollideEntityWithBeam(IPhysicalEntity* _pent, Vec3 org, Vec3 dir, float r, ray_hit* phit) {
        sphere sph; sph.center=org; sph.r=r;
        return CollideEntityWithPrimitive(_pent, sphere::type,&sph, dir, phit);
    }
    virtual i32   CollideEntityWithPrimitive(IPhysicalEntity* _pent, i32 itype, primitives::primitive* pprim, Vec3 dir, ray_hit* phit, intersection_params* pip = 0);
    virtual i32   RayTraceEntity(IPhysicalEntity* pient, Vec3 origin, Vec3 dir, ray_hit* pHit, pe_params_pos* pp = 0, u32 geomFlagsAny = geom_colltype0 | geom_colltype_player);

    virtual float PrimitiveWorldIntersection(const SPWIParams& pp, WriteLockCond* pLockContacts = 0, tukk pNameTag = PWI_NAME_TAG);
    virtual void  GetMemoryStatistics(IDrxSizer* pSizer) { DRX_PHYSX_LOG_FUNCTION; }

    virtual void  SetPhysicsStreamer(IPhysicsStreamer* pStreamer) { DRX_PHYSX_LOG_FUNCTION; }
    virtual void  SetPhysicsEventClient(IPhysicsEventClient* pEventClient) { DRX_PHYSX_LOG_FUNCTION; }
    virtual float GetLastEntityUpdateTime(IPhysicalEntity* pent) { DRX_PHYSX_LOG_FUNCTION; _RETURN_FLOAT_DUMMY_; } //!< simulation class-based, not actually per-entity
    virtual i32   GetEntityProfileInfo(phys_profile_info*& pList) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; }
    virtual i32   GetFuncProfileInfo(phys_profile_info*& pList) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; }
    virtual i32   GetGroupProfileInfo(phys_profile_info*& pList) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; }
    virtual i32   GetJobProfileInfo(phys_job_info*& pList) { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; }

    virtual void             AddEventClient(i32 type, i32(*func)(const EventPhys*), i32 bLogged, float priority = 1.0f);
    virtual i32              RemoveEventClient(i32 type, i32(*func)(const EventPhys*), i32 bLogged);
    virtual i32              NotifyEventClients(EventPhys* pEvent, i32 bLogged) { SendEvent(*pEvent,bLogged); return 1; }
    virtual void             PumpLoggedEvents();
    virtual u32           GetPumpLoggedEventsTicks() { DRX_PHYSX_LOG_FUNCTION; _RETURN_INT_DUMMY_; }
    virtual void             ClearLoggedEvents();

    virtual IPhysicalEntity* AddGlobalArea();
    virtual IPhysicalEntity* AddArea(Vec3* pt, i32 npt, float zmin, float zmax, const Vec3& pos = Vec3(0, 0, 0), const quaternionf& q = quaternionf(IDENTITY), float scale = 1.0f, const Vec3& normal = Vec3(ZERO), i32* pTessIdx = 0, i32 nTessTris = 0, Vec3* pFlows = 0) { DRX_PHYSX_LOG_FUNCTION; _RETURN_PTR_DUMMY_; }
    virtual IPhysicalEntity* AddArea(IGeometry* pGeom, const Vec3& pos, const quaternionf& q, float scale);
    virtual IPhysicalEntity* AddArea(Vec3* pt, i32 npt, float r, const Vec3& pos = Vec3(0, 0, 0), const quaternionf& q = quaternionf(IDENTITY), float scale = 1) { DRX_PHYSX_LOG_FUNCTION; _RETURN_PTR_DUMMY_; }
    virtual IPhysicalEntity* GetNextArea(IPhysicalEntity* pPrevArea = 0);
    virtual i32              CheckAreas(const Vec3& ptc, Vec3& gravity, pe_params_buoyancy* pb, i32 nMaxBuoys = 1, i32 iMedium = -1, const Vec3& vec = Vec3(ZERO), IPhysicalEntity* pent = 0, i32 iCaller = MAX_PHYS_THREADS);

    virtual void             SetWaterMat(i32 imat) {}
    virtual i32              GetWaterMat() { return 0; }
    virtual i32              SetWaterUprParams(pe_params* params) { return 0; }
    virtual i32              GetWaterUprParams(pe_params* params) { return 0; }
    virtual i32              GetWatermanStatus(pe_status* status) { return 0; }
    virtual void             DestroyWaterUpr() {}

    virtual  i32*    GetInternalLock(i32 idx) { DRX_PHYSX_LOG_FUNCTION; _RETURN_PTR_DUMMY_; } //!< returns one of phys_lock locks

    virtual i32              SerializeWorld(tukk fname, i32 bSave) { return 0; }
    virtual i32              SerializeGeometries(tukk fname, i32 bSave) { return 0; }

    virtual void             SerializeGarbageTypedSnapshot(TSerialize ser, i32 iSnapshotType, i32 flags) { DRX_PHYSX_LOG_FUNCTION; }

    virtual void             SavePhysicalEntityPtr(TSerialize ser, IPhysicalEntity* pent) { DRX_PHYSX_LOG_FUNCTION; }
    virtual IPhysicalEntity* LoadPhysicalEntityPtr(TSerialize ser) { DRX_PHYSX_LOG_FUNCTION; _RETURN_PTR_DUMMY_; }
    virtual void             GetEntityMassAndCom(IPhysicalEntity* pIEnt, float& mass, Vec3& com) { DRX_PHYSX_LOG_FUNCTION; }

    virtual EventPhys*       AddDeferredEvent(i32 type, EventPhys* event);

    virtual uk            GetInternalImplementation(i32 type, uk object = nullptr); // type: EPE_InternalImplementation

    // private:

    PhysicsVars m_vars;
    ILog* m_pLog;
    IPhysRenderer *m_pRenderer;

    /////////////////////////// PhysX //////////////////////////////////
    PhysXEnt *m_phf = nullptr;
    PhysXEnt *m_pentStatic = nullptr;
    std::vector<PhysXEnt*> m_entsById;
    i32 m_idFree = -1;
    uint m_idWorldRgn = 0;
    PxMaterial *m_mats[NSURFACETYPES];
     i32 m_lockMask=0, m_lockCollEvents=0, m_lockEntDelete=0, m_lockEntList=0;
     uint m_maskUsed = 0x1f; // lower mask bits are reserved
    std::vector<PhysXEnt*> m_entList;
    PhysXEnt *m_activeEntList[2], *m_prevActiveEntList[2];
    PhysXEnt *m_addEntList[2], *m_delEntList[2];
    PhysXProjectile *m_projectilesList[2];
    PhysXEnt *m_auxEntList[2];
    pe_params_buoyancy m_pbGlob;
    Vec3 m_wind = Vec3(0);
    float m_dt=0, m_dtSurplus=0;
    i32 m_idStep = 0;
     i32 m_updated=0;
    double m_time = 0;
    std::vector<char> m_scratchBuf;

    PxBatchQuery *m_batchQuery[2];
    i32 m_nqRWItouches=0;
    std::vector<PxRaycastQueryResult> m_rwiResPx;
    std::vector<PxRaycastHit> m_rwiTouchesPx;
    std::vector<EventPhysRWIResult> m_rwiRes[3];
    std::vector<ray_hit> m_rwiHits;
     i32 m_lockRWIqueue=0, m_lockRWIres=0;

    struct SEventClient {
        i32 (*OnEvent)(const EventPhys*);
        float priority;
        bool operator<(const SEventClient& op) const { return priority > op.priority; }
    };
    std::multiset<SEventClient> m_eventClients[EVENT_TYPES_NUM][2];
    i32 m_nCollEvents = 0;
    std::vector<EventPhysCollision> m_collEvents[2];

    void SendEvent(EventPhys &evt, i32 bLogged) {
        auto &list = m_eventClients[evt.idval][bLogged];
        std::find_if(list.begin(),list.end(), [&](auto client)->bool { return !client.OnEvent(&evt); });
    }
    EventPhys *AllocEvent(i32 id);

    virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) {}
    virtual void onWake(PxActor** actors, PxU32 count);
    virtual void onSleep(PxActor** actors, PxU32 count);
    virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
    virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) {}
    virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {}

    PxMaterial *GetSurfaceType(i32 i) { return m_mats[ (uint)i<(uint)NSURFACETYPES && m_mats[i] ? i : 0 ]; }
    void UpdateProjectileState(PhysXProjectile *pent);

    private:

    bool m_debugDraw;
    IPhysRenderer *m_renderer;
    std::vector<Vec3> m_debugDrawTriangles;
    std::vector<ColorB> m_debugDrawTrianglesColors;
    std::vector<Vec3> m_debugDrawLines;
    std::vector<ColorB> m_debugDrawLinesColors;

};

namespace cpx {
    namespace Helper {
        inline i32 MatId(const physx::PxMaterial *mtl) { return mtl ? ((i32)(INT_PTR)mtl->userData & 0xffff) : 0; }
    }
}

#endif
