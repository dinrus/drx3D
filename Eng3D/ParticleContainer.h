// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleContainer.h
//  Version:     v1.00
//  Created:     11/03/2010 by Corey (split out from other files).
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particlecontainer_h__
#define __particlecontainer_h__
#pragma once

#include <drx3D/Eng3D/ParticleEffect.h>
#include <drx3D/Eng3D/ParticleEnviron.h>
#include <drx3D/Eng3D/ParticleUtils.h>
#include <drx3D/Eng3D/Particle.h>
#include <drx3D/CoreX/Renderer/RendElements/CREParticle.h>

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/Eng3D/ParticleCommon.h>
#include <drx3D/Eng3D/ParticleDataTypes.h>
#include <drx3D/Eng3D/ParticleMath.h>
#include <drx3D/Eng3D/ParticleDataStreams.h>

namespace pfx2
{

class CParticleContainer;

typedef TIStream<UCol>         IColorStream;
typedef TIStream<u32>       IUintStream;
typedef TIOStream<u32>      IOUintStream;
typedef TIStream<TParticleId>  IPidStream;
typedef TIOStream<TParticleId> IOPidStream;

class CParticleContainer
{
public:
    CParticleContainer();
    CParticleContainer(const CParticleContainer& copy);
    ~CParticleContainer();

    void                              Resize(u32 newSize);
    void                              ResetUsedData();
    void                              AddParticleData(EParticleDataType type);
    bool                              HasData(EParticleDataType type) const { return m_useData[type]; }
    void                              AddParticle();
    void                              Trim();
    void                              Clear();

    template<typename T> void         FillData(TDataType<T> type, const T& data, SUpdateRange range);
    void                              CopyData(EParticleDataType dstType, EParticleDataType srcType, SUpdateRange range);

    template<typename T> TIStream<T>  IStream(TDataType<T> type, const T& defaultVal = T()) const                           { return TIStream<T>(Data(type), defaultVal);   }
    template<typename T> TIOStream<T> IOStream(TDataType<T> type)                                                           { return TIOStream<T>(Data(type));  }

    TIStream<Vec3>                    IStream(TDataType<Vec3> type, Vec3 defaultVal = Vec3(0)) const;
    TIOStream<Vec3>                   IOStream(TDataType<Vec3> type);
    TIStream<Quat>                    IStream(TDataType<Quat> type, Quat defaultVal = Quat(IDENTITY)) const;
    TIOStream<Quat>                   IOStream(TDataType<Quat> type);

    // Legacy compatibility
    IFStream                          GetIFStream(TDataType<float> type, float defaultVal = 0.0f) const                      { return IStream(type, defaultVal); }
    IOFStream                         GetIOFStream(TDataType<float> type)                                                    { return IOStream(type); }
    IVec3Stream                       GetIVec3Stream(TDataType<Vec3> type, Vec3 defaultVal = Vec3(ZERO)) const               { return IStream(type, defaultVal); }
    IOVec3Stream                      GetIOVec3Stream(TDataType<Vec3> type)                                                  { return IOStream(type); }
    IQuatStream                       GetIQuatStream(TDataType<Quat> type, Quat defaultVal = Quat(IDENTITY)) const           { return IStream(type, defaultVal); }
    IOQuatStream                      GetIOQuatStream(TDataType<Quat> type)                                                  { return IOStream(type); }
    IColorStream                      GetIColorStream(TDataType<UCol> type, UCol defaultVal = UCol()) const                  { return IStream(type, defaultVal); }
    IOColorStream                     GetIOColorStream(TDataType<UCol> type)                                                 { return IOStream(type); }
    IPidStream                        GetIPidStream(TDataType<TParticleId> type, TParticleId defaultVal = gInvalidId) const  { return IStream(type, defaultVal); }
    IOPidStream                       GetIOPidStream(TDataType<TParticleId> type)                                            { return IOStream(type); }

    u32                            GetNumParticles() const         { DRX_PFX2_ASSERT(!HasNewBorns()); return m_lastId; }
    u32                            GetRealNumParticles() const     { return m_lastId + HasNewBorns() * GetNumSpawnedParticles(); }
    u32                            GetMaxParticles() const         { return m_maxParticles; }
    u32                            GetNumSpawnedParticles() const  { return m_lastSpawnId - m_firstSpawnId; }
    bool                              HasNewBorns() const             { return m_lastSpawnId > m_lastId; }
    bool                              IsNewBorn(TParticleId id) const { return id >= m_firstSpawnId && id < m_lastSpawnId; }
    TParticleId                       GetRealId(TParticleId pId) const;
    SUpdateRange                      GetFullRange() const            { DRX_PFX2_ASSERT(!HasNewBorns()); return SUpdateRange(0, m_lastId); }
    SUpdateRange                      GetSpawnedRange() const         { return SUpdateRange(m_firstSpawnId, m_lastSpawnId); }
    SUpdateRange                      GetNonSpawnedRange() const      { DRX_PFX2_ASSERT(!HasNewBorns()); return SUpdateRange(0, m_firstSpawnId); }

    void                              AddParticles(TConstArray<SSpawnEntry> spawnEntries);
    void                              ResetSpawnedParticles();
    void                              RemoveParticles(TVarArray<TParticleId> toRemove, TVarArray<TParticleId> swapIds);

private:
    template<typename T> T*           Data(TDataType<T> type)                { return reinterpret_cast<T*>(m_pData[type]); }
    template<typename T> const T*     Data(TDataType<T> type) const          { return reinterpret_cast<const T*>(m_pData[type]); }

    void                              MakeSwapIds(TVarArray<TParticleId> toRemove, TVarArray<TParticleId> swapIds);

    StaticEnumArray<uk , EParticleDataType> m_pData;
    StaticEnumArray<bool, EParticleDataType>  m_useData;
    TParticleId m_nextSpawnId;
    TParticleId m_maxParticles;

    TParticleId m_lastId;
    TParticleId m_firstSpawnId;
    TParticleId m_lastSpawnId;
};

}

#include <drx3D/Eng3D/ParticleContainerImpl.h>
#include <drx3D/Eng3D/ParticleList.h>

class CParticleEmitter;
class CParticleSubEmitter;
struct SParticleVertexContext;

//////////////////////////////////////////////////////////////////////////
struct DRX_ALIGN(16) SParticleUpdateContext
{
    float fUpdateTime;
    u32 nEnvFlags;
    Vec3 vEmitBox;
    Vec3 vEmitScale;
    AABB* pbbDynamicBounds;
    bool bHasTarget;

    // Iteration support.
    float fMaxLinearDeviation;
    float fMaxLinearStepTime;
    float fMinStepTime;

    // Sorting support.
    Vec3 vMainCamPos;
    float fCenterCamDist;
    i32 nSortQuality;
    bool b3DRotation;

    // Updated per emitter.
    float fDensityAdjust;

    struct SSortElem
    {
        const CParticle* pPart;
        float            fDist;

        bool operator<(const SSortElem& r) const
        { return fDist < r.fDist; }
    };
    Array<SSortElem> aParticleSort;

    struct SSpaceLoop
    {
        Vec3 vCenter, vSize;
        Vec3 vScaledAxes[3];
    };
    SSpaceLoop SpaceLoop;
};

//////////////////////////////////////////////////////////////////////////
// Particle rendering helper params.
struct DRX_ALIGN(16) SPartRenderParams
{
    float m_fCamDistance;
    float m_fMainBoundsScale;
    float m_fMaxAngularDensity;
    u32 m_nRenFlags;
    TrinaryFlags<uint64> m_nRenObjFlags;
    u16 m_nFogVolumeContribIdx;
    float m_fHDRDynamicMultiplier;
    u16 m_nDeferredLightVolumeId;
};

//////////////////////////////////////////////////////////////////////////
// Contains, updates, and renders a list of particles, of a single effect

class CParticleContainer : public IParticleVertexCreator, public DinrusX3dEngBase
{
public:

    CParticleContainer(CParticleContainer* pParent, CParticleEmitter* pMain, CParticleEffect const* pEffect);
    ~CParticleContainer();

    // Associated structures.
    ILINE const CParticleEffect*        GetEffect() const { return m_pEffect; }
    ILINE CParticleEmitter&             GetMain()         { return *m_pMainEmitter; }
    ILINE const CParticleEmitter&       GetMain() const   { return *m_pMainEmitter; }

    ILINE ResourceParticleParams const& GetParams() const
    {
        return *m_pParams;
    }

    float      GetAge() const;

    ILINE bool IsIndirect() const
    {
        return m_pParentContainer != 0;
    }

    ILINE uk GetDirectEmitter()
    {
        if (!m_pParentContainer && !m_Emitters.empty())
        {
            assert(m_Emitters.size() == 1);
            return reinterpret_cast<uk> (m_Emitters.front());
        }
        return NULL;
    }

    // If indirect container.
    ILINE CParticleContainer* GetParent() const
    {
        return m_pParentContainer;
    }

    void                 Reset();

    CParticleSubEmitter* AddEmitter(CParticleSource* pSource);
    CParticle*           AddParticle(SParticleUpdateContext& context, const CParticle& part);
    void                 UpdateParticles();
    void                 SyncUpdateParticles();
    void                 UpdateEmitters(SParticleUpdateContext* pEmitContext = NULL);

    void                 EmitParticle(const EmitParticleData* pData);

    u16               GetNextEmitterSequence()
    {
        return m_nEmitterSequence++;
    }
    void  UpdateEffects();
    float InvalidateStaticBounds();
    void  Render(SRendParams const& RenParams, SPartRenderParams const& PRParams, const SRenderingPassInfo& passInfo);
    void  RenderGeometry(const SRendParams& RenParams, const SRenderingPassInfo& passInfo);
    void  RenderDecals(const SRenderingPassInfo& passInfo);
    void  RenderLights(const SRendParams& RenParams, const SRenderingPassInfo& passInfo);
    void  ResetRenderObjects();

    // Bounds functions.
    void   UpdateState();
    void   SetDynamicBounds();

    u32 NeedsDynamicBounds() const
    {
#ifndef _RELEASE
        if (GetCVars()->e_ParticlesDebug & AlphaBit('d'))
            // Force all dynamic bounds computation and culling.
            return EFF_DYNAMIC_BOUNDS;
#endif
        return (m_nEnvFlags | m_nChildFlags) & EFF_DYNAMIC_BOUNDS;
    }
    bool StaticBoundsStable() const
    {
        return GetAge() >= m_fAgeStaticBoundsStable;
    }
    AABB const& GetBounds() const
    {
        return m_bbWorld;
    }
    AABB const& GetStaticBounds() const
    {
        return m_bbWorldStat;
    }
    AABB const& GetDynamicBounds() const
    {
        return m_bbWorldDyn;
    }

    u32 GetEnvironmentFlags() const;

    size_t GetNumParticles() const
    {
        return m_Particles.size();
    }

    u32 GetChildFlags() const
    {
        return m_nChildFlags;
    }

    float GetContainerLife() const
    {
        return m_fContainerLife;
    }
    void  UpdateContainerLife(float fAgeAdjust = 0.f);

    bool  GetTarget(ParticleTarget& target, const CParticleSubEmitter* pSubEmitter) const;

    float GetTimeToUpdate() const
    {
        return GetAge() - m_fAgeLastUpdate;
    }

    // Temp functions to update edited effects.
    inline bool IsUsed() const
    {
        return !m_bUnused;
    }

    void SetUsed(bool b)
    {
        m_bUnused = !b;
    }

    i32 GetHistorySteps() const
    {
        return m_nHistorySteps;
    }
    u32 NeedsCollisionInfo() const
    {
        return GetEnvironmentFlags() & ENV_COLLIDE_INFO;
    }
    float GetMaxParticleFullLife() const
    {
        return m_fMaxParticleFullLife;
    }

    void OnEffectChange();

    void ComputeUpdateContext(SParticleUpdateContext& context, float fUpdateTime);

    // Stat/profile functions.
    SContainerCounts& GetCounts()
    {
        return m_Counts;
    }

    void GetCounts(SParticleCounts& counts) const;
    void ClearCounts()
    {
        ZeroStruct(m_Counts);
    }

    void GetMemoryUsage(IDrxSizer* pSizer) const;

    //////////////////////////////////////////////////////////////////////////
    // IParticleVertexCreator methods

    virtual void ComputeVertices(const SCameraInfo& camInfo, CREParticle* pRE, uint64 uVertexFlags, float fMaxPixels);

    // Other methods.
    bool NeedsUpdate() const;

    // functions for particle generate and culling of vertices directly into Video Memory
    i32  CullParticles(SParticleVertexContext& Context, i32& nVertices, i32& nIndices, u8 auParticleAlpha[]);
    void WriteVerticesToVMEM(SParticleVertexContext& Context, SRenderVertices& RenderVerts, u8k auParticleAlpha[]);

    void SetNeedJobUpdate(u8 n)
    {
        m_nNeedJobUpdate = n;
    }
    u8 NeedJobUpdate() const
    {
        return m_nNeedJobUpdate;
    }
    bool WasRenderedPrevFrame() const
    {
        return m_nNeedJobUpdate > 0;
    }

    i32 GetEmitterCount() const
    {
        return m_Emitters.size();
    }

    void OffsetPosition(const Vec3& delta);
    void OnHidden();
    void OnUnhidden();

private:
    const ResourceParticleParams*      m_pParams;     // Pointer to particle params (effect or code).
    _smart_ptr<CParticleEffect>        m_pEffect;     // Particle effect used for this emitter.
    u32                             m_nEnvFlags;
    u32                             m_nChildFlags; // Summary of rendering/environment info for child containers.

    ParticleList<CParticleSubEmitter>  m_Emitters;                    // All emitters into this container.
    ParticleList<EmitParticleData>     m_DeferredEmitParticles;       // Data for client EmitParticles calls, deferred till next update.
    ParticleList<_smart_ptr<IStatObj>> m_ExternalStatObjs;            // Any StatObjs from EmitParticle; released on destruction.

    u16                             m_nHistorySteps;
    u16                             m_nEmitterSequence;

    ParticleList<CParticle>            m_Particles;

    CRenderObject*                     m_pBeforeWaterRO[RT_COMMAND_BUF_COUNT];
    CRenderObject*                     m_pAfterWaterRO[RT_COMMAND_BUF_COUNT];
    CRenderObject*                     m_pRecursiveRO[RT_COMMAND_BUF_COUNT];

    // Last time when emitter updated, and static bounds validated.
    float m_fAgeLastUpdate;
    float m_fAgeStaticBoundsStable;
    float m_fContainerLife;

    // Final bounding volume for rendering. Also separate static & dynamic volumes for sub-computation.
    AABB  m_bbWorld, m_bbWorldStat, m_bbWorldDyn;

    bool  m_bUnused;                                            // Temp var used during param editing.
    u8 m_nNeedJobUpdate;                                     // Mark container as needing sprite rendering this frame.
                                                                // Can be set to >1 to prime for threaded update next frame.

    // Associated structures.
    CParticleContainer* m_pParentContainer;                     // Parent container, if indirect.
    CParticleEmitter*   m_pMainEmitter;                         // Emitter owning this container.
    float               m_fMaxParticleFullLife;                 // Cached value indicating max update time necessary.

    SContainerCounts    m_Counts;                               // Counts for stats.

    void  ComputeStaticBounds(AABB& bb, bool bWithSize = true, float fMaxLife = fHUGE);

    float GetEmitterLife() const;
    float GetMaxParticleScale() const;
    i32   GetMaxParticleCount(const SParticleUpdateContext& context) const;
    void  UpdateParticleStates(SParticleUpdateContext& context);
    void  SetScreenBounds(const CCamera& cam, TRect_tpl<u16> &bounds);

    CRenderObject* CreateRenderObject(uint64 nObjFlags, const SRenderingPassInfo& passInfo);
};

#endif // __particlecontainer_h__
