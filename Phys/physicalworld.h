// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef physicalworld_h
#define physicalworld_h
#pragma once

#include "rigidbody.h"
#include "physicalentity.h"
#include "geoman.h"
#include <drx3D/CoreX/Thread/IThreadUpr.h>

i32k NSURFACETYPES = 512;
i32k PLACEHOLDER_CHUNK_SZLG2 = 8;
i32k PLACEHOLDER_CHUNK_SZ = 1<<PLACEHOLDER_CHUNK_SZLG2;
i32k QUEUE_SLOT_SZ = 8192;

i32k PENT_SETPOSED = 1<<16;
i32k PENT_QUEUED_BIT = 17;
i32k PENT_QUEUED = 1<<PENT_QUEUED_BIT;

class CPhysicalPlaceholder;
class CPhysicalEntity;
class CPhysArea;
struct pe_gridthunk;
struct le_precomp_entity;
struct le_precomp_part;
struct le_tmp_contact;
enum { pef_step_requested = 0x40000000 };

template <class T> T *CONTACT_END(T *const &pnext) { return (T*)((INT_PTR)&pnext-(INT_PTR)&((T*)0)->next); }

#if defined(ENTITY_PROFILER_ENABLED)
#define PHYS_ENTITY_PROFILER CPhysEntityProfiler ent_profiler(this);
#define PHYS_AREA_PROFILER(pArea) CPhysAreaProfiler ent_profiler(pArea);
#else
#define PHYS_ENTITY_PROFILER
#define PHYS_AREA_PROFILER(pArea)
#endif

#if defined(_RELEASE)
# define PHYS_FUNC_PROFILER_DISABLED 1
#endif

#ifndef PHYS_FUNC_PROFILER_DISABLED
# define PHYS_FUNC_PROFILER(name) CPhysFuncProfiler func_profiler((name),this);
# define BLOCK_PROFILER(count) CBlockProfiler block_profiler(count);
#else
# define PHYS_FUNC_PROFILER(name)
# define BLOCK_PROFILER(count)
#endif

struct EventClient {
    i32 (*OnEvent)(const EventPhys*);
    EventClient *next;
    float priority;
#ifndef PHYS_FUNC_PROFILER_DISABLED
    char tag[32];
    int64 ticks;
#endif
};

i32k EVENT_CHUNK_SZ = 8192-sizeof(uk );
struct EventChunk {
    EventChunk *next;
};

struct SExplosionShape {
    i32 id;
    IGeometry *pGeom;
    float size,rsize;
    i32 idmat;
    float probability;
    i32 iFirstByMat,nSameMat;
    i32 bCreateConstraint;
};

struct SRwiRequest : IPhysicalWorld::SRWIParams {
    i32 iCaller;
    i32 idSkipEnts[5];
};

#define max_def(a,b) (a) + ((b)-(a) & (a)-(b)>>31)
struct SPwiRequest : IPhysicalWorld::SPWIParams {
    char primbuf[max_def(max_def(max_def(max_def(sizeof(box),sizeof(cylinder)),sizeof(capsule)),sizeof(sphere)),sizeof(triangle))];
    i32 idSkipEnts[4];
};

struct SBreakRequest {
    pe_explosion expl;
    geom_world_data gwd[2];
    CPhysicalEntity *pent;
    i32 partid;
    Vec3 gravity;
};

struct pe_PODcell {
    i32 inextActive;
    float lifeTime;
    i32 nObjects;
    Vec2 zlim;
};

struct DRX_ALIGN(128) SPrecompPartBV {
    sphere partsph;
    box partbox;
    quaternionf qpart;
    Vec3 pospart;
    float scale;
    CPhysicalEntity* pent; geom *ppart;
    i32 ient; i32 ipart; i32 iparttype;
};

struct SThreadData {
    CPhysicalEntity **pTmpEntList;
    i32 szList;

    SPrecompPartBV *pTmpPartBVList;
    i32 szTmpPartBVList, nTmpPartBVs;
    CPhysicalEntity* pTmpPartBVListOwner;

    le_precomp_entity *pTmpPrecompEntsLE;
    le_precomp_part *pTmpPrecompPartsLE;
    le_tmp_contact *pTmpNoResponseContactLE;
    i32 nPrecompEntsAllocLE, nPrecompPartsAllocLE, nNoResponseAllocLE;

    i32 bGroupInvisible;
    float groupMass;
    float maxGroupFriction;
};

struct SThreadTaskRequest {
    float time_interval;
    i32 bSkipFlagged;
    i32 ipass;
    i32 iter;
    i32 *pbAllGroupsFinished;
};

#ifdef ENTGRID_2LEVEL
struct pe_entgrid {
    pe_entgrid &operator=(const pe_entgrid &src) {
        log2Stride=src.log2Stride;
        log2StrideXMask=src.log2StrideXMask;
        log2StrideYMask=src.log2StrideYMask;
        gridlod1=src.gridlod1;
        return *this;
    }
    pe_entgrid &operator=(i32) { gridlod1=0; return *this; }
    operator bool() const { return gridlod1!=0; }

    i32 operator[](i32 i) {
        i32 iy0=i>>log2Stride, ix0=i & log2StrideXMask;
        i32 *grid = gridlod1[ ((i & ~log2StrideYMask) >> 6) | ix0>>3], dummy=0;
        INT_PTR mask = -iszero((INT_PTR)grid);
        return ((i32*)((INT_PTR)grid+((INT_PTR)&dummy-(INT_PTR)grid & mask)))[(iy0 & 7)*8+(ix0 & 7) & ~(i32)mask];
    }
    i32 &operator[](u32 i) {
        i32 iy0=i>>log2Stride, ix0=i & log2StrideXMask;
        i32 *&grid = gridlod1[((i & ~log2StrideYMask) >> 6) | ix0>>3];
        if (!grid)
            memset(grid=new i32[64], 0, 64*sizeof(i32));
        // cppcheck-suppress memleak
        return grid[(iy0 & 7)*8+(ix0 & 7)];
    }

    i32 log2Stride;
    i32 log2StrideXMask;
    i32 log2StrideYMask;
    i32 **gridlod1;
};

inline void AllocateGrid(pe_entgrid &grid, const Vec2i &size)
{
    memset(grid.gridlod1 = new i32*[(size.x*size.y>>6)+1], 0, ((size.x*size.y>>6)+1)*sizeof(i32*));
    i32 bitCount = bitcount(size.x-1);
    grid.log2Stride             = bitCount;
    grid.log2StrideXMask    = (1 << bitCount) - 1;
    grid.log2StrideYMask    = (1 << (bitCount + 3)) - 1;
}
inline void DeallocateGrid(pe_entgrid &grid, const Vec2i &size)
{
    for(i32 i=size.x*size.y>>6;i>=0;i--) if (grid.gridlod1[i])
        delete[] grid.gridlod1[i];
    delete[] grid.gridlod1; grid.gridlod1=0;
}
inline i32 GetGridSize(pe_entgrid &grid, const Vec2i& size)
{
    if (!grid)
        return 0;
    i32 i,sz=0;
    for(i=size.x*size.y>>6; i>=0; i--)
        sz += iszero((INT_PTR)grid.gridlod1[i])^1;
    return sz*64*sizeof(i32)+((size.x*size.y>>6)+1)*sizeof(i32*);
}
#else
#define pe_entgrid i32*
inline void AllocateGrid(i32 *&grid, const Vec2i &size) { memset(grid = new i32[size.x*size.y+1], 0, (size.x*size.y+1)*sizeof(i32)); }
inline void DeallocateGrid(i32 *&grid, const Vec2i &size) { delete[] grid; }
inline i32 GetGridSize(i32 *&grid, const Vec2i& size) { return (size.x*size.y+1)*sizeof(i32); }
#endif

i32k TRIGGER_PORTAL = 1<<9;
i32k TRIGGER_PORTAL_INV = 1<<10;
inline i32 IsPortal(const CPhysicalPlaceholder *pobj) { return iszero(pobj->m_iSimClass-6) & pobj->m_iForeignFlags>>9; }
inline i32 IsPortal(const pe_gridthunk &thunk) { return thunk.iSimClass==6 && thunk.pent->m_iForeignFlags & TRIGGER_PORTAL; }

inline Vec3* transformBBox(const Vec3& ptmin, const Vec3& ptmax, Vec3* dstBBox, const QuatT& trans) {
    Matrix33 R = Matrix33(trans.q);
    Vec3 c = trans*((ptmin+ptmax)*0.5f), sz = R.Fabs()*(ptmax-ptmin)*0.5f;
    dstBBox[0] = c-sz; dstBBox[1] = c+sz;
    return dstBBox;
}


struct SEntityGrid : public CPhysicalPlaceholder, grid {
    i32 iup;
    float zGran,rzGran;
    pe_entgrid cells;
    pe_PODcell **pPODcells,dummyPODcell,*pDummyPODcell;
    Vec2i PODstride;
    i32 log2PODscale;
    i32 bHasPODGrid,iActivePODCell0;

    SEntityGrid *m_next,*m_prev;
    QuatT m_transInHost,m_transW;
    Vec3 m_velW;
    Vec3 m_v, m_w, m_com;
    float m_dvSleep2;
    CPhysicalEntity *m_host;
    QuatT GetUpdatedTransW();
    Vec3 GetUpdatedVelW();

    class CPhysicalWorld *m_pWorld = nullptr;
     mutable i32 m_refCount = 0;
    virtual i32 AddRef() { return DrxInterlockedIncrement(&m_refCount); }
    virtual i32 Release();
    virtual i32 SetParams(pe_params*, i32 bThreadSafe=1);
    virtual i32 GetParams(pe_params*) const;

    virtual pe_type GetType() const { return PE_GRID; }
    void Init();
    void Free() { if (cells) DeallocateGrid(cells,size); cells=0; if (m_host) m_host->Release(); m_host=0; }
    void Setup(i32 axisz, Vec3 org, i32 nx,i32 ny, float stepx,float stepy, i32 log2PODscale, i32 bCyclic);
    void DeactivateOnDemand();
    void GetPODGridCellBBox(i32 ix,i32 iy, Vec3 &center,Vec3 &size) const;
    void RegisterBBoxInPODGrid(const Vec3 *BBox, IPhysicsStreamer *pStreamer);
    void UnregisterBBoxInPODGrid(const Vec3 *BBox);
    i32 AddRefEntInPODGrid(IPhysicalEntity *_pent, const Vec3 *BBox);

    pe_PODcell *getPODcell(i32 ix,i32 iy)   const {
        i32 i, imask = ~negmask(ix) & ~negmask(iy) & negmask(ix - size.x) & negmask(iy - size.y);   // (x>=0 && x<m_entgrid.size.x) ? 0xffffffff : 0;
        ix >>= log2PODscale; iy >>= log2PODscale;
        i = (ix>>3)*PODstride.x + (iy>>3)*PODstride.y;
        i = i + ((-1-i) & ~imask) & -bHasPODGrid;
        INT_PTR pmask = -iszero((INT_PTR)pPODcells[i]);
        pe_PODcell *pcell0 = (pe_PODcell*)(((INT_PTR)pPODcells[i] & ~pmask) + ((INT_PTR)pDummyPODcell & pmask));
        imask &= -bHasPODGrid & ~pmask;
        return pcell0 + ((ix&7)+((iy&7)<<3) & imask);
    }
    Vec3 vecToGrid(const Vec3& src) const { return src.GetPermutated(iup); }
    Vec3 vecFromGrid(const Vec3& src) const { return src.GetPermutated(iup^1^iup>>1); }
    void BBoxToGrid(const Vec3& ptmin, const Vec3& ptmax, Vec3* dst) const {    dst[0]=vecToGrid(ptmin-origin); dst[1]=vecToGrid(ptmax-origin); }
    void BBoxFromGrid(const Vec3& ptmin, const Vec3& ptmax, Vec3* dst) const {  dst[0]=vecFromGrid(ptmin)+origin; dst[1]=vecFromGrid(ptmax)+origin; }
};


struct SPhysTask : public IThread {
    SPhysTask(CPhysicalWorld *pWorld,i32 idx) { m_pWorld=pWorld; m_idx=idx; bStop=0; }
    virtual void ThreadEntry();
    void SignalStopWork();
    void Wait();

    class CPhysicalWorld *m_pWorld;
    i32 m_idx;
     i32 bStop;
};

#ifndef MAIN_THREAD_NONWORKER
#define THREAD_TASK(a,b) \
    m_rq.ipass=a; \
    for(i32 ithread=0;ithread<m_nWorkerThreads;ithread++)   m_threadStart[ithread].Set();   \
    b; \
    for(i32 ithread=0;ithread<m_nWorkerThreads;ithread++)   m_threadDone[ithread].Wait();
#else
#define THREAD_TASK(a,b) \
    m_rq.ipass=a; \
    for(i32 ithread=0;ithread<m_nWorkerThreads;ithread++)   m_threadStart[ithread].Set();   \
    for(i32 ithread=0;ithread<m_nWorkerThreads;ithread++)   m_threadDone[ithread].Wait();
#endif

class CPhysicalWorld : public IPhysicalWorld, public IPhysUtils, public CGeomUpr {

  class CPhysicalEntityIt : public IPhysicalEntityIt {
  public:
    CPhysicalEntityIt(CPhysicalWorld* pWorld);

      virtual bool IsEnd();
      virtual IPhysicalEntity* Next();
      virtual IPhysicalEntity* This();
      virtual void MoveFirst();

      virtual void AddRef() {   ++m_refs;   }
        virtual void Release() { if (--m_refs<=0)   delete this; }
    private:
        CPhysicalWorld* m_pWorld;
        CPhysicalEntity *m_pCurEntity;
        i32 m_refs;
    };

public:
    CPhysicalWorld(ILog *pLog);
    ~CPhysicalWorld();

    virtual void Init();
    virtual void Shutdown(i32 bDeleteEntities = 1);
    virtual void Release() { delete this; }
    virtual IGeomUpr* GetGeomUpr() { return this; }
    virtual IPhysUtils* GetPhysUtils() { return this; }

    virtual IPhysicalEntity* SetupEntityGrid(i32 axisz, Vec3 org, i32 nx,i32 ny, float stepx,float stepy, i32 log2PODscale=0, i32 bCyclic=0,
        IPhysicalEntity* pHost=nullptr, const QuatT& posInHost=QuatT(IDENTITY));
    virtual void Cleanup();
    virtual void DeactivateOnDemandGrid();
    virtual void RegisterBBoxInPODGrid(const Vec3 *BBox);
    virtual void UnregisterBBoxInPODGrid(const Vec3 *BBox);
    virtual i32 AddRefEntInPODGrid(IPhysicalEntity *pent, const Vec3 *BBox=0);
    virtual IPhysicalEntity *SetHeightfieldData(const heightfield *phf,i32 *pMatMapping=0,i32 nMats=0);
    virtual IPhysicalEntity *GetHeightfieldData(heightfield *phf);
    virtual void SetHeightfieldMatMapping(i32 *pMatMapping, i32 nMats);
    virtual i32 SetSurfaceParameters(i32 surface_idx, float bounciness,float friction, u32 flags=0);
    virtual i32 GetSurfaceParameters(i32 surface_idx, float &bounciness,float &friction, u32 &flags);
    virtual i32 SetSurfaceParameters(i32 surface_idx, float bounciness,float friction,
        float damage_reduction, float ric_angle, float ric_dam_reduction,
        float ric_vel_reduction, u32 flags=0);
    virtual i32 GetSurfaceParameters(i32 surface_idx, float &bounciness,float &friction,
        float &damage_reduction, float &ric_angle, float &ric_dam_reduction,
        float &ric_vel_reduction, u32 &flags);
    virtual PhysicsVars *GetPhysVars() { return &m_vars; }

    void InitGThunksPool();
    void AllocGThunksPool( i32 nNewSize );
    void DeallocGThunksPool();
    void FlushOldThunks();
    void SortThunks();
    i32 GetFreeThunk();

    virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, pe_params* params=0, uk pForeignData=0,i32 iForeignData=0, i32 id=-1, IGeneralMemoryHeap* pHeap = NULL)
    { return CreatePhysicalEntity(type,0.0f,params,pForeignData,iForeignData,id, NULL, pHeap); }
    virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, float lifeTime, pe_params* params=0, uk pForeignData=0,i32 iForeignData=0,
        i32 id=-1,IPhysicalEntity *pHostPlaceholder=0, IGeneralMemoryHeap* pHeap = NULL);
    virtual IPhysicalEntity *CreatePhysicalPlaceholder(pe_type type, pe_params* params=0, uk pForeignData=0,i32 iForeignData=0, i32 id=-1);
    virtual i32 DestroyPhysicalEntity(IPhysicalEntity *pent, i32 mode=0, i32 bThreadSafe=0);
    virtual i32 SetPhysicalEntityId(IPhysicalEntity *pent, i32 id, i32 bReplace=1, i32 bThreadSafe=0);
    virtual i32 GetPhysicalEntityId(IPhysicalEntity *pent);
    i32 GetFreeEntId();
    virtual IPhysicalEntity* GetPhysicalEntityById(i32 id);
    i32 IsPlaceholder(const CPhysicalPlaceholder *pent) {
        if (!pent) return 0;
        i32 iChunk; for(iChunk=0; iChunk<m_nPlaceholderChunks && (u32)(pent-m_pPlaceholders[iChunk])>=(u32)PLACEHOLDER_CHUNK_SZ; iChunk++);
        return iChunk<m_nPlaceholderChunks ? (iChunk<<PLACEHOLDER_CHUNK_SZLG2 | (i32)(pent-m_pPlaceholders[iChunk]))+1 : 0;
    }

    virtual void TimeStep(float time_interval, i32 flags=ent_all|ent_deleted);
    virtual float GetPhysicsTime() { return m_timePhysics; }
    virtual i32 GetiPhysicsTime() { return m_iTimePhysics; }
    virtual void SetPhysicsTime(float time) {
        m_timePhysics = time;
        if (m_vars.timeGranularity>0)
            m_iTimePhysics = (i32)(m_timePhysics/m_vars.timeGranularity+0.5f);
    }
    virtual void SetiPhysicsTime(i32 itime) { m_timePhysics = (m_iTimePhysics=itime)*m_vars.timeGranularity; }
    virtual void SetSnapshotTime(float time_snapshot,i32 iType=0) {
        m_timeSnapshot[iType] = time_snapshot;
        if (m_vars.timeGranularity>0)
            m_iTimeSnapshot[iType] = (i32)(time_snapshot/m_vars.timeGranularity+0.5f);
    }
    virtual void SetiSnapshotTime(i32 itime_snapshot,i32 iType=0) {
        m_iTimeSnapshot[iType] = itime_snapshot; m_timeSnapshot[iType] = itime_snapshot*m_vars.timeGranularity;
    }

    // *important* if request RWIs queued iForeignData should be a EPhysicsForeignIds
    virtual i32 RayWorldIntersection(const Vec3& org,const Vec3& dir, i32 objtypes, u32 flags, ray_hit *hits,i32 nmaxhits,
        IPhysicalEntity **pSkipEnts=0,i32 nSkipEnts=0, uk pForeignData=0,i32 iForeignData=0,
        tukk pNameTag="RayWorldIntersection(Physics)", ray_hit_cached *phitLast=0, i32 iCaller=get_iCaller_int())
    {
        SRWIParams rp; rp.org=org; rp.dir=dir; rp.objtypes=objtypes; rp.flags=flags; rp.hits=hits; rp.nMaxHits=nmaxhits;
        rp.pForeignData=pForeignData; rp.iForeignData=iForeignData; rp.phitLast=phitLast; rp.pSkipEnts=pSkipEnts; rp.nSkipEnts=nSkipEnts;
        return RayWorldIntersection(rp, pNameTag, iCaller);
    }
    virtual i32 RayWorldIntersection(const SRWIParams &rp, tukk pNameTag="RayWorldIntersection(Physics)", i32 iCaller=get_iCaller_int());
    virtual i32 TracePendingRays(i32 bDoTracing=1);

    void RayHeightfield(const Vec3 &org,Vec3 &dir, ray_hit *hits, i32 flags, i32 iCaller);
    void RayWater(const Vec3 &org,const Vec3 &dir, struct entity_grid_checker &egc, i32 flags,i32 nMaxHits, ray_hit *hits);

    virtual void SimulateExplosion(pe_explosion *pexpl, IPhysicalEntity **pSkipEnts=0,i32 nSkipEnts=0,
        i32 iTypes=ent_rigid|ent_sleeping_rigid|ent_living|ent_independent, i32 iCaller=get_iCaller_int());
    virtual float IsAffectedByExplosion(IPhysicalEntity *pent, Vec3 *impulse=0);
    virtual float CalculateExplosionExposure(pe_explosion *pexpl, IPhysicalEntity *pient);
    virtual void ResetDynamicEntities();
    virtual void DestroyDynamicEntities();
    virtual void PurgeDeletedEntities();
    virtual i32 DeformPhysicalEntity(IPhysicalEntity *pent, const Vec3 &ptHit,const Vec3 &dirHit,float r, i32 flags=0);
    virtual void UpdateDeformingEntities(float time_interval);
    virtual i32 GetEntityCount(i32 iEntType) { return m_nTypeEnts[iEntType]; }
    virtual i32 ReserveEntityCount(i32 nNewEnts);

  virtual IPhysicalEntityIt* GetEntitiesIterator();

    virtual void DrawPhysicsHelperInformation(IPhysRenderer *pRenderer,i32 iCaller=0);
    virtual void DrawEntityHelperInformation(IPhysRenderer *pRenderer,i32 iEntityId,i32 iDrawHelpers);

    virtual void GetMemoryStatistics(IDrxSizer *pSizer);

    virtual i32 CollideEntityWithBeam(IPhysicalEntity *_pent, Vec3 org,Vec3 dir,float r, ray_hit *phit);
    virtual i32 RayTraceEntity(IPhysicalEntity *pient, Vec3 origin,Vec3 dir, ray_hit *pHit, pe_params_pos *pp=0,
        u32 geomFlagsAny = geom_colltype0|geom_colltype_player);
    virtual i32 CollideEntityWithPrimitive(IPhysicalEntity *_pent, i32 itype, primitive *pprim, Vec3 dir, ray_hit *phit, intersection_params* pip=0);

    virtual float PrimitiveWorldIntersection(const SPWIParams &pp, WriteLockCond *pLockContacts=0, tukk pNameTag="PrimitiveWorldIntersection");
    virtual void RasterizeEntities(const grid3d& grid, uchar *rbuf, i32 objtypes, float massThreshold, const Vec3& offsBBox, const Vec3& sizeBBox, i32 flags);

    virtual i32 GetEntitiesInBox(Vec3 ptmin,Vec3 ptmax, IPhysicalEntity **&pList, i32 objtypes, i32 szListPrealloc) {
        WriteLock lock(m_lockCaller[MAX_PHYS_THREADS]);
        return GetEntitiesAround(ptmin,ptmax, (CPhysicalEntity**&)pList, objtypes, 0, szListPrealloc, MAX_PHYS_THREADS);
    }
    i32 GetEntitiesAround(const Vec3 &ptmin,const Vec3 &ptmax, CPhysicalEntity **&pList, i32 objtypes, CPhysicalEntity *pPetitioner=0,
        i32 szListPrealoc=0, i32 iCaller=get_iCaller());
    i32 ChangeEntitySimClass(CPhysicalEntity *pent, i32 bGridLocked);
    i32 RepositionEntity(CPhysicalPlaceholder *pobj, i32 flags=3, Vec3 *BBox=0, i32 bQueued=0);
    void DetachEntityGridThunks(CPhysicalPlaceholder *pobj);
    void ScheduleForStep(CPhysicalEntity *pent, float time_interval);
    CPhysicalEntity *CheckColliderListsIntegrity();

    virtual i32 CoverPolygonWithCircles(strided_pointer<Vec2> pt,i32 npt,bool bConsecutive, const Vec2 &center,
        Vec2 *&centers,float *&radii, float minCircleRadius)
    { return ::CoverPolygonWithCircles(pt,npt,bConsecutive, center, centers,radii, minCircleRadius); }
    virtual void DeletePointer(uk pdata) { if (pdata) delete[] (tuk)pdata; }
    virtual i32 qhull(strided_pointer<Vec3> pts, i32 npts, index_t*& pTris, qhullmalloc qmalloc = 0) { return ::qhull(pts,npts,pTris, qmalloc); }
    virtual i32 qhull2d(ptitem2d *pts,i32 nVtx, edgeitem *edges, i32 nMaxEdges=0) { return ::qhull2d(pts,nVtx,edges,nMaxEdges); }
    virtual i32 TriangulatePoly(Vec2 *pVtx, i32 nVtx, i32 *pTris,i32 szTriBuf)
    { return ::TriangulatePoly(pVtx, nVtx, pTris, szTriBuf); }
    virtual void SetPhysicsStreamer(IPhysicsStreamer *pStreamer) { m_pPhysicsStreamer=pStreamer; }
    virtual void SetPhysicsEventClient(IPhysicsEventClient *pEventClient) { m_pEventClient=pEventClient; }
    virtual float GetLastEntityUpdateTime(IPhysicalEntity *pent) { return m_updateTimes[((CPhysicalPlaceholder*)pent)->m_iSimClass & 7]; }
    virtual  i32 *GetInternalLock(i32 idx) {
        switch (idx) {
            case PLOCK_WORLD_STEP: return &m_lockStep;
            case PLOCK_QUEUE: return &m_lockQueue;
            case PLOCK_AREAS: return &m_lockAreas;
            case PLOCK_TRACE_PENDING_RAYS: return &m_lockTPR;
            default:
                if ((u32)(idx-PLOCK_CALLER0)<=(u32)MAX_PHYS_THREADS)
                    return m_lockCaller+(idx-PLOCK_CALLER0);
        }
        return 0;
    }

    void AddEntityProfileInfo(CPhysicalEntity *pent,i32 nTicks);
    virtual i32 GetEntityProfileInfo(phys_profile_info *&pList) {   pList=m_pEntProfileData; return m_nProfiledEnts; }
    void AddFuncProfileInfo(tukk name,i32 nTicks);
    virtual i32 GetFuncProfileInfo(phys_profile_info *&pList)   {   pList=m_pFuncProfileData; return m_nProfileFunx; }
    virtual i32 GetGroupProfileInfo(phys_profile_info *&pList) { pList=m_grpProfileData; return DRX_ARRAY_COUNT(m_grpProfileData); }
    virtual i32 GetJobProfileInfo(phys_job_info *&pList) { pList = m_JobProfileInfo; return DRX_ARRAY_COUNT(m_JobProfileInfo); }
    phys_job_info& GetJobProfileInst(i32 ijob) { return m_JobProfileInfo[ijob]; }

    // *important* if provided callback function return 0, other registered listeners are not called anymore.
    virtual void AddEventClient(i32 type, i32 (*func)(const EventPhys*), i32 bLogged, float priority=1.0f);
    virtual i32 RemoveEventClient(i32 type, i32 (*func)(const EventPhys*), i32 bLogged);
    virtual void PumpLoggedEvents();
    virtual u32 GetPumpLoggedEventsTicks();
    virtual void ClearLoggedEvents();
    void CleanseEventsQueue();
    void PatchEventsQueue(IPhysicalEntity* pEntity, uk pForeignData, i32 iForeignData);
    EventPhys *AllocEvent(i32 id,i32 sz);
    template<class Etype> i32 OnEvent(u32 flags, Etype *pEvent, Etype **pEventLogged=0) {
        i32 res = 0;
        if ((flags & Etype::flagsCall)==Etype::flagsCall)
            res = SignalEvent(pEvent, 0);
        if ((flags & Etype::flagsLog)==Etype::flagsLog) {
            WriteLock lock(m_lockEventsQueue);
            Etype *pDst = (Etype*)AllocEvent(Etype::id,sizeof(Etype));
            memcpy(pDst,pEvent,sizeof(*pDst)); pDst->next = 0;
            (m_pEventLast ? m_pEventLast->next : m_pEventFirst) = pDst;
            m_pEventLast = pDst;
            if (pEventLogged)
                *pEventLogged = pDst;
            if (Etype::id==(i32k)EventPhysPostStep::id) {
                CPhysicalPlaceholder *ppc = (CPhysicalPlaceholder*)((EventPhysPostStep*)pEvent)->pEntity;
                if (ppc->m_bProcessed & PENT_SETPOSED)
                    AtomicAdd(&ppc->m_bProcessed, -PENT_SETPOSED);
            }
        }
        return res;
    }
    i32 SignalEvent(EventPhys *pEvent, i32 bLogged) {
        i32 nres = 0;
        EventClient *pClient;
        ReadLock lock(m_lockEventClients);
        for(pClient = m_pEventClients[pEvent->idval][bLogged]; pClient; pClient=pClient->next)
            nres += pClient->OnEvent(pEvent);
        return nres;
    }
    virtual i32 NotifyEventClients(EventPhys *pEvent, i32 bLogged) { return SignalEvent(pEvent,bLogged); }

    virtual i32 SerializeWorld(tukk fname, i32 bSave);
    virtual i32 SerializeGeometries(tukk fname, i32 bSave);

    virtual IPhysicalEntity *AddGlobalArea();
    virtual IPhysicalEntity *AddArea(Vec3 *pt,i32 npt, float zmin,float zmax, const Vec3 &pos=Vec3(0,0,0), const quaternionf &q=quaternionf(),
        float scale=1.0f, const Vec3& normal=Vec3(ZERO), i32 *pTessIdx=0,i32 nTessTris=0,Vec3 *pFlows=0);
    virtual IPhysicalEntity *AddArea(IGeometry *pGeom, const Vec3& pos,const quaternionf &q,float scale);
    virtual IPhysicalEntity *AddArea(Vec3 *pt,i32 npt, float r, const Vec3 &pos,const quaternionf &q,float scale);
    virtual void RemoveArea(IPhysicalEntity *pArea);
    virtual i32 CheckAreas(const Vec3 &ptc, Vec3 &gravity, pe_params_buoyancy *pb, i32 nMaxBuoys=1, i32 iMedium=-1, const Vec3 &vel=Vec3(ZERO),
        IPhysicalEntity *pent=0, i32 iCaller=get_iCaller_int());
    i32 CheckAreas(CPhysicalEntity *pent, Vec3 &gravity, pe_params_buoyancy *pb,i32 nMaxBuoys=1, const Vec3 &vel=Vec3(ZERO), i32 iCaller=get_iCaller_int()) {
        if (!m_pGlobalArea || pent->m_flags & pef_ignore_areas)
            return 0;
        return CheckAreas((pent->m_BBox[0]+pent->m_BBox[1])*0.5f, gravity, pb, nMaxBuoys, -1, vel, pent, iCaller);
    }
    void RepositionArea(CPhysArea *pArea, Vec3* pBoxPrev = 0);
    void ActivateArea(CPhysArea *pArea);
    virtual IPhysicalEntity *GetNextArea(IPhysicalEntity *pPrevArea=0);

    virtual void SetWaterMat(i32 imat);
    virtual i32 GetWaterMat() { return m_matWater; }
    virtual i32 SetWaterUprParams(pe_params *params);
    virtual i32 GetWaterUprParams(pe_params *params);
    virtual i32 GetWatermanStatus(pe_status *status);
    virtual void DestroyWaterUpr();

    virtual i32 AddExplosionShape(IGeometry *pGeom, float size,i32 idmat, float probability=1.0f);
    virtual void RemoveExplosionShape(i32 id);
    virtual void RemoveAllExplosionShapes(void (*OnRemoveGeom)(IGeometry *pGeom)=0) {
        for(i32 i=0;i<m_nExpl;i++) {
            if (OnRemoveGeom) OnRemoveGeom(m_pExpl[i].pGeom);
            m_pExpl[i].pGeom->Release();
        }   m_nExpl=m_idExpl = 0;
    }
    IGeometry *GetExplosionShape(float size,i32 idmat, float &scale, i32 &bCreateConstraint);
    i32 DeformEntityPart(CPhysicalEntity *pent,i32 i, pe_explosion *pexpl, geom_world_data *gwd,geom_world_data *gwd1, i32 iSource=0);
    void MarkEntityAsDeforming(CPhysicalEntity *pent);
    void UnmarkEntityAsDeforming(CPhysicalEntity *pent);
    void ClonePhysGeomInEntity(CPhysicalEntity *pent,i32 i,IGeometry *pNewGeom);

    void AllocRequestsQueue(i32 sz) {
        if (m_nQueueSlotSize+sz+1 > QUEUE_SLOT_SZ) {
            if (m_nQueueSlots==m_nQueueSlotsAlloc)
                ReallocateList(m_pQueueSlots, m_nQueueSlots,m_nQueueSlotsAlloc+=8, true);
            if (!m_pQueueSlots[m_nQueueSlots])
                m_pQueueSlots[m_nQueueSlots] = new char[max(sz+1,QUEUE_SLOT_SZ)];
            ++m_nQueueSlots; m_nQueueSlotSize = 0;
        }
    }
    uk QueueData(uk ptr, i32 sz) {
        uk storage = m_pQueueSlots[m_nQueueSlots-1]+m_nQueueSlotSize;
        memcpy(storage, ptr, sz);   m_nQueueSlotSize += sz;
        *(i32*)(m_pQueueSlots[m_nQueueSlots-1]+m_nQueueSlotSize) = -1;
        return storage;
    }
    template<class T> T* QueueData(const T& data) {
        T &storage = *(T*)(m_pQueueSlots[m_nQueueSlots-1]+m_nQueueSlotSize);
        storage = data;
        m_nQueueSlotSize += sizeof(data);
        *(i32*)(m_pQueueSlots[m_nQueueSlots-1]+m_nQueueSlotSize) = -1;
        return &storage;
    }

    float GetFriction(i32 imat0,i32 imat1,i32 bDynamic=0) {
        float *pTable = (float*)((intptr_t)m_FrictionTable&~(intptr_t)-bDynamic | (intptr_t)m_DynFrictionTable&(intptr_t)-bDynamic);
        return max(0.0f,pTable[imat0 & NSURFACETYPES-1]+pTable[imat1 & NSURFACETYPES-1])*0.5f;
    }
    float GetBounciness(i32 imat0,i32 imat1) {
        return (m_BouncinessTable[imat0 & NSURFACETYPES-1]+m_BouncinessTable[imat1 & NSURFACETYPES-1])*0.5f;
    }

    virtual void SavePhysicalEntityPtr(TSerialize ser, IPhysicalEntity* pIEnt);
    virtual IPhysicalEntity* LoadPhysicalEntityPtr(TSerialize ser);
    virtual void GetEntityMassAndCom(IPhysicalEntity* pIEnt, float& mass, Vec3& com);
    virtual uk GetInternalImplementation(i32 type, uk object = nullptr) { return nullptr; }


    IPhysicalWorld *GetIWorld() { return this; }

    virtual void SerializeGarbageTypedSnapshot( TSerialize ser, i32 iSnapshotType, i32 flags );

    i32 GetTmpEntList(CPhysicalEntity **&pEntList, i32 iCaller) {
        INT_PTR plist = (INT_PTR)m_threadData[iCaller].pTmpEntList;
        i32 is0=iCaller-1>>31, isN=MAX_PHYS_THREADS-iCaller-1>>31;
        plist += (INT_PTR)m_pTmpEntList -plist & is0;
        plist += (INT_PTR)m_pTmpEntList2-plist & isN;
        m_threadData[iCaller].pTmpEntList = (CPhysicalEntity**)plist;
        m_threadData[iCaller].szList += m_nEntsAlloc-m_threadData[iCaller].szList & (is0|isN);
        pEntList = m_threadData[iCaller].pTmpEntList;
        return m_threadData[iCaller].szList;
    }
    i32 ReallocTmpEntList(CPhysicalEntity **&pEntList, i32 iCaller, i32 szNew);

    void ProcessNextEntityIsland(float time_interval, i32 ipass, i32 iter, i32 &bAllGroupsFinished, i32 iCaller);
    void ProcessIslandSolverResults(i32 i, i32 iter, float groupTimeStep,float Ebefore, i32 nEnts,float fixedDamping, i32 &bAllGroupsFinished,
        entity_contact **pContacts,i32 nContacts,i32 nBodies, i32 iCaller,int64 iticks0);
    i32 ReadDelayedSolverResults(CMemStream &stm, float &dt,float &Ebefore,i32 &nEnts,float &fixedDamping, entity_contact **pContacts,RigidBody **pBodies);
    void ProcessNextEngagedIndependentEntity(i32 iCaller);
    void ProcessNextLivingEntity(float time_interval, i32 bSkipFlagged, i32 iCaller);
    void ProcessNextIndependentEntity(float time_interval, i32 bSkipFlagged, i32 iCaller);
    void ProcessBreakingEntities(float time_interval);
    void ThreadProc(i32 ithread, SPhysTask *pTask);

    template<class T> void ReallocQueue(T *&pqueue, i32 sz,i32 &szAlloc, i32 &head,i32 &tail, i32 nGrow) {
        if (sz==szAlloc) {
            T *pqueueOld = pqueue;
            pqueue = new T[szAlloc+nGrow];
            memcpy(pqueue, pqueueOld, (head+1)*sizeof(T));
            memcpy(pqueue+head+nGrow+1, pqueueOld+head+1, (sz-head-1)*sizeof(T));
            if (tail) tail += nGrow;
            szAlloc += nGrow;
            delete[] pqueueOld;
        }
        head = head+1 - (szAlloc & szAlloc-2-head>>31);
    }

    virtual EventPhys *AddDeferredEvent( i32 type, EventPhys *event )
    {
        switch (type) {
            case EventPhysBBoxOverlap::id       : { EventPhysBBoxOverlap *pLogged; OnEvent(EventPhysBBoxOverlap::flagsLog,(EventPhysBBoxOverlap*)event,&pLogged); return pLogged; }
            case EventPhysCollision::id         : { EventPhysCollision *pLogged; OnEvent(EventPhysCollision::flagsLog,(EventPhysCollision*)event,&pLogged); return pLogged; }
            case EventPhysStateChange::id       : { EventPhysStateChange *pLogged; OnEvent(EventPhysStateChange::flagsLog,(EventPhysStateChange*)event,&pLogged); return pLogged; }
            case EventPhysEnvChange::id         : { EventPhysEnvChange *pLogged; OnEvent(EventPhysEnvChange::flagsLog,(EventPhysEnvChange*)event,&pLogged); return pLogged; }
            case EventPhysPostStep::id          : { EventPhysPostStep *pLogged; OnEvent(EventPhysPostStep::flagsLog,(EventPhysPostStep*)event,&pLogged); return pLogged; }
            case EventPhysUpdateMesh::id        : { EventPhysUpdateMesh *pLogged; OnEvent(EventPhysUpdateMesh::flagsLog,(EventPhysUpdateMesh*)event,&pLogged); return pLogged; }
            case EventPhysCreateEntityPart::id  : { EventPhysCreateEntityPart *pLogged; OnEvent(EventPhysCreateEntityPart::flagsLog,(EventPhysCreateEntityPart*)event,&pLogged); return pLogged; }
            case EventPhysRemoveEntityParts::id : { EventPhysRemoveEntityParts *pLogged; OnEvent(EventPhysRemoveEntityParts::flagsLog,(EventPhysRemoveEntityParts*)event,&pLogged); return pLogged; }
            case EventPhysRevealEntityPart::id  : { EventPhysRevealEntityPart *pLogged; OnEvent(EventPhysRevealEntityPart::flagsLog,(EventPhysRevealEntityPart*)event,&pLogged); return pLogged; }
            case EventPhysJointBroken::id       : { EventPhysJointBroken *pLogged; OnEvent(EventPhysJointBroken::flagsLog,(EventPhysJointBroken*)event,&pLogged); return pLogged; }
            case EventPhysRWIResult::id         : { EventPhysRWIResult *pLogged; OnEvent(EventPhysRWIResult::flagsLog,(EventPhysRWIResult*)event,&pLogged); return pLogged; }
            case EventPhysPWIResult::id         : { EventPhysPWIResult *pLogged; OnEvent(EventPhysPWIResult::flagsLog,(EventPhysPWIResult*)event,&pLogged); return pLogged; }
            case EventPhysArea::id              : { EventPhysArea *pLogged; OnEvent(EventPhysArea::flagsLog,(EventPhysArea*)event,&pLogged); return pLogged; }
            case EventPhysAreaChange::id        : { EventPhysAreaChange *pLogged; OnEvent(EventPhysAreaChange::flagsLog,(EventPhysAreaChange*)event,&pLogged); return pLogged; }
            case EventPhysEntityDeleted::id     : { EventPhysEntityDeleted *pLogged; OnEvent(EventPhysEntityDeleted::flagsLog,(EventPhysEntityDeleted*)event,&pLogged); return pLogged; }
        }
        return 0;
    }

    template <class T> T *AllocPooledObj(T *&pFirstFree, i32 &countFree,i32 &countAlloc,  i32 &ilock)
    {
        WriteLock lock(ilock);
        if (pFirstFree->next==pFirstFree) {
            T *pChunk = new T[64];
            memset(pChunk, 0, sizeof(T)*64);
            for(i32 i=0;i<64;i++) {
                pChunk[i].next = pChunk+i+1; pChunk[i].prev = pChunk+i-1;
                pChunk[i].bChunkStart = 0;
            }
            pChunk[0].prev = pChunk[63].next = CONTACT_END(pFirstFree);
            (pFirstFree = pChunk)->bChunkStart = 1;
            countFree += 64; countAlloc += 64;
        }
        T *pObj = pFirstFree;
        pFirstFree->next->prev = pFirstFree->prev;
        pFirstFree->prev->next = pFirstFree->next;
        pObj->next=pObj->prev = pObj;
        countFree--;
        return pObj;
    }
    template <class T> void FreePooledObj(T *pObj, T *&pFirstFree, i32 &countFree,  i32 &ilock)
    {
        WriteLock lock(ilock);
        pObj->prev = pFirstFree->prev; pObj->next = pFirstFree;
        pFirstFree->prev = pObj; pFirstFree = pObj;
        countFree++;
    }
    template <class T> void FreeObjPool(T *&pFirstFree, i32 countFree,i32 &countAlloc)
    {
        T *pObj,*pObjNext;
        for(pObj=pFirstFree; pObj!=CONTACT_END(pFirstFree); pObj=pObj->next) if (!pObj->bChunkStart) {
            pObj->prev->next=pObj->next; pObj->next->prev=pObj->prev;
        }

        // workaround for a aliasing problem with pObj->next pointing into CPhysicalWorld;
        // the compiler assumes that pFristFree doesn't changein the first loop, thus it spares
        // a second load in the second loop, which is wrong.
        // the MEMORY_RW_REORDERING_BARRIER ensures a reload of pFirstFree
        MEMORY_RW_REORDERING_BARRIER;

        for(pObj=pFirstFree; pObj!=CONTACT_END(pFirstFree); pObj=pObjNext) {
            pObjNext = pObj->next; delete[] pObj;
        }
        pFirstFree = CONTACT_END(pFirstFree); pFirstFree->prev = pFirstFree;
        countAlloc = countFree = 0;
    }

    entity_contact *AllocContact() { return AllocPooledObj(m_pFreeContact,m_nFreeContacts,m_nContactsAlloc,m_lockContacts); }
    void FreeContact(entity_contact *pContact) { FreePooledObj(pContact, m_pFreeContact,m_nFreeContacts,m_lockContacts); }

    geom *AllocEntityPart() { return AllocPooledObj(m_pFreeEntPart,m_nFreeEntParts,m_nEntPartsAlloc,m_lockEntParts); }
    void FreeEntityPart(geom *pPart) { FreePooledObj(pPart, m_pFreeEntPart,m_nFreeEntParts,m_lockEntParts); }

    geom *AllocEntityParts(i32 count) { return count==1 ? AllocEntityPart() : new geom[count]; }
    void FreeEntityParts(geom *pParts, i32 count) {
        if (pParts!=&CPhysicalEntity::m_defpart)
            if (count==1)
                FreeEntityPart(pParts);
            else
                delete[] pParts;
    }

    PhysicsVars m_vars;
    ILog *m_pLog;
    IPhysicsStreamer *m_pPhysicsStreamer;
    IPhysicsEventClient *m_pEventClient;
    IPhysRenderer *m_pRenderer;

    CPhysicalEntity *m_pTypedEnts[8],*m_pTypedEntsPerm[8];
    CPhysicalEntity **m_pTmpEntList,**m_pTmpEntList1,**m_pTmpEntList2;
    CPhysicalEntity *m_pHiddenEnts;
    float *m_pGroupMass,*m_pMassList;
    i32 *m_pGroupIds,*m_pGroupNums;

    SEntityGrid m_entgrid;
    SEntityGrid *m_pDeletedGrids = nullptr;
#ifdef MULTI_GRID
    SEntityGrid* GetGrid(const CPhysicalPlaceholder *pobj) { return pobj->m_pGrid; }
    SEntityGrid* GetGrid(const IPhysicalEntity *pIEnt) { return pIEnt!=WORLD_ENTITY ? GetGrid((CPhysicalPlaceholder*)pIEnt) : &m_entgrid; }
    SEntityGrid* SetGrid(CPhysicalPlaceholder *pobj, SEntityGrid *pgrid) { SEntityGrid *pgridPrev=GetGrid(pobj); pobj->m_pGrid=pgrid; return pgridPrev; }
    SEntityGrid* GetHostedGrid(const CPhysicalEntity *pent) {
        return pent->m_pOuterEntity && pent->m_pOuterEntity->GetType()==PE_GRID ? static_cast<SEntityGrid*>((IPhysicalEntity*)pent->m_pOuterEntity) : nullptr;
    }
    template<typename Rot,typename Tdst> QuatT TransformToGrid(const CPhysicalPlaceholder *src, const Tdst *dst, Vec3 &offs, Rot &rot) {
        if (GetGrid(src) != GetGrid(dst)) {
            QuatT diff = GetGrid(dst)->m_transW.GetInverted()*GetGrid(src)->m_transW;
            rot = Rot(diff.q)*rot;
            offs = diff*offs;
            return diff;
        }
        return QuatT(IDENTITY);
    }
#else
    template<typename T> SEntityGrid* GetGrid(const T *pobj) { return &m_entgrid; }
    SEntityGrid* SetGrid(CPhysicalPlaceholder *pobj, SEntityGrid *pgrid) { return &m_entgrid; }
    SEntityGrid* GetHostedGrid(const CPhysicalEntity *pent) { return nullptr; }
    template<typename Rot,typename Tdst> QuatT TransformToGrid(const CPhysicalPlaceholder *src, const Tdst *dst, Vec3 &offs, Rot &rot) { return QuatT(IDENTITY); }
#endif
    void UnlockGrid(CPhysicalPlaceholder *pobj, i32 val=-WRITE_LOCK_VAL) { AtomicAdd(&m_lockGrid, val); }
    void SyncPortal(CPhysicalPlaceholder *portal);
    SEntityGrid* DestroyGrid(SEntityGrid *pgrid);   // returns pgrid->m_next

    pe_gridthunk *m_gthunks;
    i32 m_thunkPoolSz,m_iFreeGThunk0;
    pe_gridthunk *m_oldThunks;
     i32 m_lockOldThunks;
    i32 m_nEnts,m_nEntsAlloc;
    i32 m_nDynamicEntitiesDeleted;
    CPhysicalPlaceholder **m_pEntsById;
    i32 m_nIdsAlloc, m_iNextId;
    i32 m_iNextIdDown,m_lastExtId,m_nExtIds;
    i32 m_bGridThunksChanged;
    i32 m_bUpdateOnlyFlagged;
    i32 m_nTypeEnts[10];
    i32 m_bEntityCountReserved;
#ifndef _RELEASE
     i32 m_nGEA[MAX_PHYS_THREADS+1];
#endif
    i32 m_nEntListAllocs;
    i32 m_nOnDemandListFailures;
    i32 m_iLastPODUpdate;
    Vec3 m_prevGEABBox[MAX_PHYS_THREADS+1][2];
    i32 m_prevGEAobjtypes[MAX_PHYS_THREADS+1];
    i32 m_nprevGEAEnts[MAX_PHYS_THREADS+1];

    i32 m_nPlaceholders,m_nPlaceholderChunks,m_iLastPlaceholder;
    CPhysicalPlaceholder **m_pPlaceholders;
    i32 *m_pPlaceholderMap;
    CPhysicalEntity *m_pEntBeingDeleted;

    SOcclusionCubeMap m_cubeMapStatic;
    SOcclusionCubeMap m_cubeMapDynamic;
    Vec3 m_lastEpicenter,m_lastEpicenterImp,m_lastExplDir;
    CPhysicalEntity **m_pExplVictims;
    float *m_pExplVictimsFrac;
    Vec3 *m_pExplVictimsImp;
    i32 m_nExplVictims,m_nExplVictimsAlloc;

    CPhysicalEntity *m_pHeightfield[MAX_PHYS_THREADS+2];
    Matrix33 m_HeightfieldBasis;
    Vec3 m_HeightfieldOrigin;

    float m_timePhysics,m_timeSurplus,m_timeSnapshot[4];
    i32 m_iTimePhysics,m_iTimeSnapshot[4];
    float m_updateTimes[8];
    i32 m_iSubstep,m_bWorldStep;
    float m_curGroupMass;
    CPhysicalEntity *m_pAuxStepEnt;
    phys_profile_info m_pEntProfileData[64];
    i32 m_nProfiledEnts;
    phys_profile_info *m_pFuncProfileData;
    i32 m_nProfileFunx,m_nProfileFunxAlloc;
     i32 m_lockEntProfiler,m_lockFuncProfiler;
    phys_profile_info m_grpProfileData[16];
    phys_job_info m_JobProfileInfo[6];
    float m_lastTimeInterval;
    i32 m_nSlowFrames;
    u32 m_nPumpLoggedEventsHits;
     threadID m_idThread;
     threadID m_idPODThread;
    i32 m_bMassDestruction;

    float m_BouncinessTable[NSURFACETYPES];
    float m_FrictionTable[NSURFACETYPES];
    float m_DynFrictionTable[NSURFACETYPES];
    float m_DamageReductionTable[NSURFACETYPES];
    float m_RicochetAngleTable[NSURFACETYPES];
    float m_RicDamReductionTable[NSURFACETYPES];
    float m_RicVelReductionTable[NSURFACETYPES];
    u32 m_SurfaceFlagsTable[NSURFACETYPES];
    i32 m_matWater,m_bCheckWaterHits;
    class CWaterMan *m_pWaterMan;
    Vec3 m_posViewer;

    char **m_pQueueSlots,**m_pQueueSlotsAux;
    i32 m_nQueueSlots,m_nQueueSlotsAlloc;
    i32 m_nQueueSlotsAux,m_nQueueSlotsAllocAux;
    i32 m_nQueueSlotSize,m_nQueueSlotSizeAux;

    CPhysArea *m_pGlobalArea;
    i32 m_nAreas,m_nBigAreas;
    CPhysArea *m_pDeletedAreas;
    CPhysArea *m_pTypedAreas[16];
    CPhysArea *m_pActiveArea;
    i32 m_numNonWaterAreas;
    i32 m_numGravityAreas;
    i32 m_numAreaTriggers;

    EventPhys *m_pEventFirst,*m_pEventLast,*m_pFreeEvents[EVENT_TYPES_NUM];
    EventClient *m_pEventClients[EVENT_TYPES_NUM][2];
    EventChunk *m_pFirstEventChunk,*m_pCurEventChunk;
    i32 m_szCurEventChunk;
    i32 m_nEvents[EVENT_TYPES_NUM];
     i32 m_idStep;

    entity_contact *m_pFreeContact,*m_pLastFreeContact;
    i32 m_nFreeContacts;
    i32 m_nContactsAlloc;

    geom *m_pFreeEntPart,*m_pLastFreeEntPart;
    geom *m_oldEntParts;
    i32 m_nFreeEntParts;
    i32 m_nEntPartsAlloc;

    SExplosionShape *m_pExpl;
    i32 m_nExpl,m_nExplAlloc,m_idExpl;
    CPhysicalEntity **m_pDeformingEnts;
    i32 m_nDeformingEnts,m_nDeformingEntsAlloc;
    SBreakRequest *m_breakQueue;
    i32 m_breakQueueHead,m_breakQueueTail;
    i32 m_breakQueueSz,m_breakQueueAlloc;

    SRwiRequest *m_rwiQueue;
    i32 m_rwiQueueHead,m_rwiQueueTail;
    i32 m_rwiQueueSz,m_rwiQueueAlloc;
    ray_hit *m_pRwiHitsHead,*m_pRwiHitsTail, *m_pRwiHitsPool;
    i32 m_rwiHitsPoolSize;
    i32 m_rwiPoolEmpty;

    SPwiRequest *m_pwiQueue;
    i32 m_pwiQueueHead,m_pwiQueueTail;
    i32 m_pwiQueueSz,m_pwiQueueAlloc;

    SThreadTaskRequest m_rq;
    DrxEvent m_threadStart[MAX_PHYS_THREADS],m_threadDone[MAX_PHYS_THREADS];
    SThreadData m_threadData[MAX_PHYS_THREADS+1];
    SPhysTask *m_threads[MAX_PHYS_THREADS];
    Vec3 m_BBoxPlayerGroup[MAX_PHYS_THREADS+1][2];
    i32 m_nGroups;
    float m_maxGroupMass;
     i32 m_nWorkerThreads;
     i32 m_iCurGroup;
     CPhysicalEntity *m_pCurEnt;
     CPhysicalEntity *m_pMovedEnts;
     i32 m_lockNextEntityGroup;
     i32 m_lockMovedEntsList;
     i32 m_lockPlayerGroups;

     i32 m_lockGrid;
     i32 m_lockPODGrid;
     i32 m_lockEntIdList;
     i32 m_lockStep, m_lockCaller[MAX_PHYS_THREADS+1],m_lockQueue,m_lockList;
     i32 m_lockAreas;
     i32 m_lockActiveAreas;
     i32 m_lockEventsQueue,m_iLastLogPump, m_lockEventClients;
     i32 m_lockDeformingEntsList;
     i32 m_lockRwiQueue;
     i32 m_lockRwiHitsPool;
     i32 m_lockTPR;
     i32 m_lockPwiQueue;
     i32 m_lockContacts;
     i32 m_lockEntParts;
     i32 m_lockBreakQueue;
     i32 m_lockAuxStepEnt;
     i32 m_lockWaterMan;
};

#ifdef MULTI_GRID
template<typename Tdst> inline QuatT GridTrans(const CPhysicalPlaceholder *pentSrc, const Tdst *pentDst) {
    return pentDst->m_pWorld->GetGrid(pentDst)->m_transW.GetInverted() * pentDst->m_pWorld->GetGrid(pentSrc)->m_transW;
}
#else
template<typename Tdst> inline QuatT GridTrans(const CPhysicalPlaceholder *pentSrc, const Tdst *pentDst) { return QuatT(IDENTITY); }
#endif

template<typename Rot> inline void CPhysicalEntity::GetPartTransform(i32 ipart, Vec3 &offs, Rot &R, float &scale, const CPhysicalPlaceholder *trg) const
{
    R = Rot(m_pNewCoords->q*m_parts[ipart].pNewCoords->q);
    offs = m_pNewCoords->q*m_parts[ipart].pNewCoords->pos + m_pNewCoords->pos;
    scale = m_parts[ipart].scale;
    m_pWorld->TransformToGrid(this,trg, offs,R);
}

template<typename CTrg> inline Vec3* CPhysicalEntity::GetPartBBox(i32 ipart, Vec3* BBox, const CTrg *trg) const
{
    if (m_pWorld->GetGrid(this)==m_pWorld->GetGrid(trg))
        return m_parts[ipart].BBox;
    QuatT trans = GridTrans(this,trg);
    return transformBBox(m_parts[ipart].BBox[0], m_parts[ipart].BBox[1], BBox, trans);
}

inline RigidBody *CPhysicalEntity::GetRigidBodyTrans(RigidBody *pbody, i32 ipart, CPhysicalEntity *trg, i32 type, bool needIinv)
{
    RigidBody *pbodySrc = type==2 ? GetRigidBodyData(pbody,ipart) : GetRigidBody(ipart,type&1);
    if (m_pWorld->GetGrid(this)==m_pWorld->GetGrid(trg))
        return pbodySrc;
    QuatT trans = GridTrans(this,trg);
    pbody->v = trans.q*pbodySrc->v;
    pbody->w = trans.q*pbodySrc->w;
    pbody->pos = trans*pbodySrc->pos;
    pbody->M=pbodySrc->M; pbody->Minv=pbodySrc->Minv;
    if (needIinv)   {
        pbody->flags = 0;
        pbody->Iinv = Matrix33(trans.q)*pbodySrc->Iinv*Matrix33(!trans.q);
    }
    return pbody;
}


i32k PHYS_FOREIGN_ID_PHYS_AREA = 12;

class DRX_ALIGN(128) CPhysArea : public CPhysicalPlaceholder {
public:
    CPhysArea(CPhysicalWorld *pWorld);
    ~CPhysArea();
    virtual pe_type GetType() const { return PE_AREA; }
    virtual bool IsPlaceholder() const { return false; }
    virtual CPhysicalEntity *GetEntity();
    virtual CPhysicalEntity *GetEntityFast() { return GetEntity(); }
    virtual i32 AddRef() { return DrxInterlockedIncrement(&m_lockRef); }
    virtual i32 Release() { return DrxInterlockedDecrement(&m_lockRef); }

    virtual i32 SetParams(pe_params* params,i32 bThreadSafe=1);
    virtual i32 Action(pe_action* action,i32 bThreadSafe=1);
    virtual i32 GetParams(pe_params* params) const;
    virtual i32 GetStatus(pe_status* status) const;
    virtual IPhysicalWorld *GetWorld() const { return (IPhysicalWorld*)m_pWorld; }
    i32 CheckPoint(const Vec3& pttest, float radius=0.0f) const;
    i32 ApplyParams(const Vec3& pt, Vec3& gravity,const Vec3 &vel, pe_params_buoyancy *pbdst,i32 nBuoys,i32 nMaxBuoys,i32 &iMedium0, IPhysicalEntity *pent) const;
    float FindSplineClosestPt(const Vec3 &ptloc, i32 &iClosestSeg,float &tClosest) const;
    i32 FindSplineClosestPt(const Vec3 &org, const Vec3 &dir, Vec3 &ptray, Vec3 &ptspline) const;
    virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);
    i32 RayTraceInternal(const Vec3 &org, const Vec3 &dir, ray_hit *pHit, pe_params_pos *pp);
    i32 RayTrace(const Vec3 &org, const Vec3 &dir, ray_hit *pHit, pe_params_pos *pp=(pe_params_pos*)0)
    {
        return RayTraceInternal(org, dir, pHit, pp);
    }
    void Update(float dt);
    void ProcessBorder();
    void DeleteWaterMan();
    static i32 OnBBoxOverlap(const EventPhysBBoxOverlap*);

    float GetExtent(EGeomForm eForm) const;
    void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;

    virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

    i32 m_bDeleted;
    CPhysArea *m_next,*m_nextBig,*m_nextTyped,*m_nextActive;
    CPhysicalWorld *m_pWorld;

    Vec3 m_offset;
    Matrix33 m_R;
    float m_scale,m_rscale;

    Vec3 m_offset0;
    Matrix33 m_R0;

    IGeometry *m_pGeom;
    Vec2 *m_pt;
    i32 m_npt;
    float m_zlim[2];
    i32 *m_idxSort[2];
    Vec3 *m_pFlows;
    u32 *m_pMask;
    Vec3 m_size0;
    Vec3 *m_ptSpline;
    float m_V;
    float m_accuracyV;
    class CTriMesh *m_pContainer;
    CPhysicalEntity *m_pContainerEnt;
    i32 m_idpartContainer;
    QuatTS m_qtsContainer;
    i32 *m_pContainerParts;
    i32 m_nContainerParts;
    i32 m_bConvexBorder;
    IPhysicalEntity *m_pTrigger;
    float m_moveAccum;
    i32 m_nSleepFrames;
    float m_sleepVel;
    float m_borderPad;
    float m_szCell;
    params_wavesim m_waveSim;
    float m_sizeReserve;
    float m_minObjV;
    class CWaterMan *m_pWaterMan;
    class CTriMesh *m_pSurface;
    i32 m_nptAlloc;
    phys_geometry *m_pMassGeom;
    mutable CGeomExtents m_Extents;
    mutable Vec3 m_ptLastCheck;
    mutable i32 m_iClosestSeg;
    mutable float m_tClosest,m_mindist;
    mutable indexed_triangle m_trihit;

    pe_params_buoyancy m_pb;
    Vec3 m_gravity;
    Vec3 m_rsize;
    i32 m_bUniform;
    float m_falloff0,m_rfalloff0;
    float m_damping;
    i32 m_bUseCallback;
    i32 m_debugGeomHash;
    mutable  i32 m_lockRef;
};

extern i32 g_nPhysWorlds;
extern CPhysicalWorld *g_pPhysWorlds[];

inline i32 IsPODThread(CPhysicalWorld *pWorld) { return iszero((i32)(DrxGetCurrentThreadId()-(pWorld->m_idPODThread))); }
inline void MarkAsPODThread(CPhysicalWorld *pWorld) { pWorld->m_idPODThread = DrxGetCurrentThreadId(); }
inline void UnmarkAsPODThread(CPhysicalWorld *pWorld) { pWorld->m_idPODThread = THREADID_NULL; }
extern i32 g_idxThisIsPODThread;

struct CPhysEntityProfilerBase {
     int64 m_iStartTime;
    CPhysEntityProfilerBase() {
        m_iStartTime = DrxGetTicks();
    }
};
struct CPhysEntityProfiler : CPhysEntityProfilerBase {
    CPhysicalEntity *m_pEntity;
    CPhysEntityProfiler(CPhysicalEntity *pent) { m_pEntity=pent; }
    ~CPhysEntityProfiler() {
        if (m_pEntity->m_pWorld->m_vars.bProfileEntities)
            m_pEntity->m_pWorld->AddEntityProfileInfo(m_pEntity,(i32)((DrxGetTicks()-m_iStartTime)));
    }
};
struct CPhysAreaProfiler : CPhysEntityProfilerBase {
    CPhysArea *m_pArea;
    CPhysAreaProfiler(CPhysArea *pArea) { m_pArea=pArea; }
    ~CPhysAreaProfiler() {
        if (m_pArea && m_pArea->m_pWorld->m_vars.bProfileEntities)
            m_pArea->m_pWorld->AddEntityProfileInfo((CPhysicalEntity*)m_pArea,(i32)((DrxGetTicks()-m_iStartTime)));
    }
};

struct CPhysFuncProfiler {
     int64 m_iStartTime;
    tukk m_name;
    CPhysicalWorld *m_pWorld;

    CPhysFuncProfiler(tukk name, CPhysicalWorld *pWorld) {
        m_name = name; m_pWorld = pWorld;
        m_iStartTime = DrxGetTicks();
    }
    ~CPhysFuncProfiler() {
        if (m_pWorld->m_vars.bProfileFunx)
            m_pWorld->AddFuncProfileInfo(m_name, (i32)(DrxGetTicks()-m_iStartTime));
    }
};

struct CBlockProfiler {
     int64 &m_time;
    CBlockProfiler(int64 &time) : m_time(time) { m_time=DrxGetTicks(); }
    ~CBlockProfiler() { m_time = DrxGetTicks()-m_time; }
};

template<class T> inline QuatT InputGridTrans(const T* input, const CPhysicalEntity *trg) {
    return !is_unused(input->pGridRefEnt) && input->pGridRefEnt && trg->m_pWorld->GetGrid(trg)!=trg->m_pWorld->GetGrid(input->pGridRefEnt) ?
        trg->m_pWorld->GetGrid(trg)->m_transW.GetInverted() * trg->m_pWorld->GetGrid(input->pGridRefEnt)->m_transW :
        QuatT(IDENTITY);
}

template<class T> inline bool StructChangesPos(T*) { return false; }
inline bool StructChangesPos(pe_params *params) {
    pe_params_pos *pp = (pe_params_pos*)params;
    return params->type==pe_params_pos::type_id && (!is_unused(pp->pos) || !is_unused(pp->pMtx3x4) && pp->pMtx3x4) && !(pp->bRecalcBounds & 16);
}

template<class T> inline void OnStructQueued(T *params, CPhysicalWorld *pWorld, uk ptrAux,i32 iAux) {}
inline void OnStructQueued(pe_geomparams *params, CPhysicalWorld *pWorld, uk ptrAux,i32 iAux) { pWorld->AddRefGeometry((phys_geometry*)ptrAux); }

template<class T> struct ChangeRequest {
    CPhysicalWorld *m_pWorld;
    i32 m_bQueued,m_bLocked,m_bLockedCaller;
    T *m_pQueued;

    ChangeRequest(CPhysicalPlaceholder *pent, CPhysicalWorld *pWorld, T *params, i32 bInactive, uk ptrAux=0,i32 iAux=0) :
    m_pWorld(pWorld),m_bQueued(0),m_bLocked(0),m_bLockedCaller(0)
    {
        if (pent->m_iSimClass==-1 && ((CPhysicalEntity*)pent)->m_iDeletionTime==3)
            bInactive |= bInactive-1>>31;
        if ((u32)pent->m_iSimClass>=7u && bInactive>=0 && !AllowChangesOnDeleted(params)) {
            extern i32 g_dummyBuf[16];
            m_bQueued = 1; m_pQueued = (T*)(g_dummyBuf+1); return;
        }
        if (bInactive<=0) {
            i32 isPODthread;
            i32 isProcessed = 0;
            while (!isProcessed) {
                if (m_pWorld->m_lockStep || m_pWorld->m_lockTPR || pent->m_bProcessed>=PENT_QUEUED || bInactive<0 ||
                        !(isPODthread=IsPODThread(m_pWorld)) && m_pWorld->m_lockCaller[MAX_PHYS_THREADS])
                {
                    subref *psubref;
                    i32 szSubref,szTot;
                    WriteLock lock(m_pWorld->m_lockQueue);
                    AtomicAdd(&pent->m_bProcessed, PENT_QUEUED);
                    for(psubref=GetSubref(params),szSubref=0; psubref; psubref=psubref->next)
                        if (*(tuk*)((tuk)params+psubref->iPtrOffs) && !is_unused(*(tuk*)((tuk)params+psubref->iPtrOffs)))
                            szSubref += ((max(0,*(i32*)((tuk)params+max(0,psubref->iSzOffs))) & -psubref->iSzOffs>>31) + psubref->iSzFixed)*psubref->iSzUnit;
                    szTot = sizeof(i32)*2+sizeof(uk )+GetStructSize(params)+szSubref;
                    if (StructUsesAuxData(params))
                        szTot += sizeof(uk )+sizeof(i32);
                    m_pWorld->AllocRequestsQueue(szTot);
                    m_pWorld->QueueData(GetStructId(params));
                    m_pWorld->QueueData(szTot);
                    m_pWorld->QueueData(pent);
                    if (StructUsesAuxData(params)) {
                        m_pWorld->QueueData(ptrAux);
                        m_pWorld->QueueData(iAux);
                    }
                    m_pQueued = (T*)m_pWorld->QueueData(params, GetStructSize(params));
                    for(psubref=GetSubref(params); psubref; psubref=psubref->next) {
                        szSubref = ((*(i32*)((tuk)params+max(0,psubref->iSzOffs)) & -psubref->iSzOffs>>31) + psubref->iSzFixed)*psubref->iSzUnit;
                        char *pParams = *(tuk*)((tuk)params+psubref->iPtrOffs);
                        if (pParams && !is_unused(pParams) && szSubref>=0)
                            *(uk *)((tuk)m_pQueued+psubref->iPtrOffs) = m_pWorld->QueueData(pParams, szSubref);
                    }
                    OnStructQueued(params,pWorld,ptrAux,iAux);
                    m_bQueued = 1;
                    isProcessed = 1;
                }   else {
                    if (!isPODthread) {
                        if (AtomicCAS(&m_pWorld->m_lockCaller[MAX_PHYS_THREADS], WRITE_LOCK_VAL, 0)) {
                            continue;
                        }
                        m_bLockedCaller = WRITE_LOCK_VAL;
                    }
                    if (AtomicCAS(&m_pWorld->m_lockStep, WRITE_LOCK_VAL, 0)) {
                        if (!isPODthread) {
                            AtomicAdd(&m_pWorld->m_lockCaller[MAX_PHYS_THREADS], -m_bLockedCaller);
                            m_bLockedCaller=0;
                        }
                        continue;
                    }
                    m_bLocked = WRITE_LOCK_VAL;
                    isProcessed = 1;
                }
            }
        }
        if (StructChangesPos(params) && !(pent->m_bProcessed & PENT_SETPOSED))
            AtomicAdd(&pent->m_bProcessed, PENT_SETPOSED);
    }
    ~ChangeRequest() { AtomicAdd(&m_pWorld->m_lockStep,-m_bLocked);AtomicAdd(&m_pWorld->m_lockCaller[MAX_PHYS_THREADS],-m_bLockedCaller); }
    i32 IsQueued() { return m_bQueued; }
    T *GetQueuedStruct() { return m_pQueued; }
};

inline void InitEventBase(EventPhysMono *ev, CPhysicalEntity *pent) {
    ev->pEntity = pent; ev->pForeignData = pent->m_pForeignData; ev->iForeignData = pent->m_iForeignData;
}
inline void InitEvent(EventPhysPostStep *ev, CPhysicalEntity *pent, i32 iCaller)    {
    InitEventBase((EventPhysMono*)ev, pent);
    ev->pGrid = pent->m_pWorld->GetGrid(pent);
    ev->iCaller = iCaller;
}

class CRayGeom;
struct geom_contact;

//do not process lock
struct SRayTraceRes
{
    const CRayGeom *pRay;
    geom_contact *pcontacts;
    ILINE SRayTraceRes(const CRayGeom *const ray,geom_contact *contacts)
        : pRay(ray),pcontacts(contacts){}
    ILINE void SetLock( i32*){}
};
#endif
