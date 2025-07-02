// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ParticleEmitter.h
//  Version:     v1.00
//  Created:     18/7/2003 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio.NET
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particleemitter_h__
#define __particleemitter_h__
#pragma once

#include <drx3D/Eng3D/ParticleEffect.h>
#include <drx3D/Eng3D/ParticleEnviron.h>
#include <drx3D/Eng3D/ParticleContainer.h>
#include <drx3D/Eng3D/ParticleSubEmitter.h>
#include <drx3D/Eng3D/ParticleUpr.h>

#include <drx3D/Entity/IEntity.h>

#include <drx3D/Eng3D/ParticleComponentRuntime.h>
#include <drx3D/Eng3D/ParticleAttributes.h>
#include <drx3D/CoreX/Renderer/IGpuParticles.h>

#undef PlaySound

class CParticle;

//////////////////////////////////////////////////////////////////////////
// A top-level emitter system, interfacing to 3D engine
class CParticleEmitter : public IParticleEmitter, public CParticleSource
{
public:

	CParticleEmitter(const IParticleEffect* pEffect, const QuatTS& loc, const SpawnParams* pSpawnParams = NULL);
	~CParticleEmitter();

	//////////////////////////////////////////////////////////////////////////
	// IRenderNode implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void        ReleaseNode(bool bImmediate) { DRX_ASSERT((m_dwRndFlags & ERF_PENDING_DELETE) == 0); Register(false, bImmediate); Kill(); }
	virtual EERType     GetRenderNodeType()          { return eERType_ParticleEmitter; }

	virtual char const* GetName() const              { return m_pTopEffect ? m_pTopEffect->GetName() : ""; }
	virtual char const* GetEntityClassName() const   { return "ParticleEmitter"; }
	virtual string      GetDebugString(char type = 0) const;

	virtual Vec3        GetPos(bool bWorldOnly = true) const { return GetLocation().t; }

	virtual const AABB  GetBBox() const                      { return m_bbWorld; }
	virtual void        SetBBox(const AABB& WSBBox)          { m_bbWorld = WSBBox; }
	virtual void        FillBBox(AABB& aabb)                 { aabb = GetBBox(); }

	virtual void        GetLocalBounds(AABB& bbox);
	virtual void        SetViewDistRatio(i32 nViewDistRatio)
	{
		// Override to cache normalized value.
		IRenderNode::SetViewDistRatio(nViewDistRatio);
		m_fViewDistRatio = GetViewDistRatioNormilized();
	}
	ILINE float              GetViewDistRatioFloat() const { return m_fViewDistRatio; }
	virtual float            GetMaxViewDist();
	virtual void             UpdateStreamingPriority(const SUpdateStreamingPriorityContext& context);

	virtual void             SetMatrix(Matrix34 const& mat)    { if (mat.IsValid()) SetLocation(QuatTS(mat)); }

	virtual void             SetMaterial(IMaterial* pMaterial) { m_pMaterial = pMaterial; }
	virtual IMaterial*       GetMaterial(Vec3* pHitPos = NULL) const;
	virtual IMaterial*       GetMaterialOverride()             { return m_pMaterial; }

	virtual IPhysicalEntity* GetPhysics() const                { return 0; }
	virtual void             SetPhysics(IPhysicalEntity*)      {}

	virtual void             Render(SRendParams const& rParam, const SRenderingPassInfo& passInfo);

	virtual void             Hide(bool bHide);

	virtual void             GetMemoryUsage(IDrxSizer* pSizer) const;

	//////////////////////////////////////////////////////////////////////////
	// IParticleEmitter implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual i32                    GetVersion() const { return 1; }
	virtual void                   SetEffect(IParticleEffect const* pEffect);
	virtual const IParticleEffect* GetEffect() const
	{ return m_pTopEffect.get(); };

	virtual QuatTS        GetLocation() const
	{ return CParticleSource::GetLocation(); }
	virtual void          SetLocation(const QuatTS& loc);

	ParticleTarget const& GetTarget() const
	{ return m_Target; }
	virtual void          SetTarget(ParticleTarget const& target)
	{
		if ((i32)target.bPriority >= (i32)m_Target.bPriority)
			m_Target = target;
	}
	virtual void                 SetEmitGeom(const GeomRef& geom);
	virtual void                 SetSpawnParams(const SpawnParams& spawnParams);
	virtual void                 GetSpawnParams(SpawnParams& sp) const { sp = m_SpawnParams; }
	const SpawnParams&           GetSpawnParams() const                { return m_SpawnParams; }

	virtual bool                 IsAlive() const;
	virtual void                 Activate(bool bActive);
	virtual void                 Kill();
	virtual void                 Restart();
	virtual void                 Update();
	virtual void                 EmitParticle(const EmitParticleData* pData = NULL);

	virtual void                 SetEntity(IEntity* pEntity, i32 nSlot);
	virtual void                 InvalidateCachedEntityData() final;
	virtual void                 OffsetPosition(const Vec3& delta);
	virtual EntityId             GetAttachedEntityId();
	virtual i32                  GetAttachedEntitySlot()
	{ return m_nEntitySlot; }
	virtual IParticleAttributes& GetAttributes();

	virtual void   SetOwnerEntity(IEntity* pEntity) final { m_pOwnerEntity = pEntity; }
	virtual IEntity* GetOwnerEntity() const final         { return m_pOwnerEntity; }

	//////////////////////////////////////////////////////////////////////////
	// Other methods.
	//////////////////////////////////////////////////////////////////////////

	bool IsActive() const
	// Has particles
	{ return m_fAge <= m_fDeathAge; }

	void        UpdateEmitCountScale();
	float       GetNearestDistance(const Vec3& vPos, float fBoundsScale) const;

	void        SerializeState(TSerialize ser);
	void        Register(bool b, bool bImmediate = false);
	ILINE float GetEmitCountScale() const
	{ return m_fEmitCountScale; }

	void                RefreshEffect();

	void                UpdateEffects();

	void                UpdateState();
	void                UpdateResetAge();

	void                CreateIndirectEmitters(CParticleSource* pSource, CParticleContainer* pCont);

	SPhysEnviron const& GetPhysEnviron() const
	{
		return m_PhysEnviron;
	}
	SVisEnviron const& GetVisEnviron() const
	{
		return m_VisEnviron;
	}
	void OnVisAreaDeleted(IVisArea* pVisArea)
	{
		m_VisEnviron.OnVisAreaDeleted(pVisArea);
	}

	void GetDynamicBounds(AABB& bb) const
	{
		bb.Reset();
		for (const auto& c : m_Containers)
			bb.Add(c.GetDynamicBounds());
	}
	ILINE u32 GetEnvFlags() const
	{
		return m_nEnvFlags & CParticleUpr::Instance()->GetAllowedEnvironmentFlags();
	}
	void AddEnvFlags(u32 nFlags)
	{
		m_nEnvFlags |= nFlags;
	}

	float GetParticleScale() const
	{
		// Somewhat obscure. But top-level emitters spawned from entities,
		// and not attached to other objects, should apply the entity scale to their particles.
		if (!GetEmitGeom())
			return m_SpawnParams.fSizeScale * GetLocation().s;
		else
			return m_SpawnParams.fSizeScale;
	}

	void InvalidateStaticBounds()
	{
		m_bbWorld.Reset();
		for (auto& c : m_Containers)
		{
			float fStableTime = c.InvalidateStaticBounds();
			m_fBoundsStableAge = max(m_fBoundsStableAge, fStableTime);
		}
	}
	void     RenderDebugInfo();
	void     UpdateFromEntity();
	bool     IsIndependent() const
	{
		return  Unique();
	}
	bool NeedSerialize() const
	{
		return IsIndependent() && !m_pTopEffect->IsTemporary();
	}
	float TimeNotRendered() const
	{
		return GetAge() - m_fAgeLastRendered;
	}

	void GetCounts(SParticleCounts& counts, bool bClear = false) const
	{
		DRX_PROFILE_FUNCTION(PROFILE_PARTICLE);

		counts.emitters.alloc += 1.f;
		if (IsActive())
		{
			counts.emitters.alive += 1.f;
			counts.emitters.updated += 1.f;
		}
		if (TimeNotRendered() == 0.f)
			counts.emitters.rendered += 1.f;

		for (const auto& c : m_Containers)
		{
			c.GetCounts(counts);
			if (bClear)
				non_const(c).ClearCounts();
		}
	}
	void GetAndClearCounts(SParticleCounts& counts)
	{
		GetCounts(counts, true);
	}
	
	ParticleList<CParticleContainer> const& GetContainers() const
	{
		return m_Containers;
	}

	bool IsEditSelected() const
	{
#if DRX_PLATFORM_DESKTOP
		if (gEnv->IsEditing())
		{
			if (m_pOwnerEntity)
			{
				if (IEntityRender* pIEntityRender = m_pOwnerEntity->GetRenderInterface())
				{
					if (IRenderNode* pRenderNode = pIEntityRender->GetRenderNode())
						return (pRenderNode->GetRndFlags() & ERF_SELECTED) != 0;
				}
			}
		}
#endif
		return false;
	}

	void AddUpdateParticlesJob();
	void SyncUpdateParticlesJob();
	void UpdateAllParticlesJob();

	void Reset()
	{
		SetUpdateParticlesJobState(nullptr);
		Register(false);

		// Free unneeded memory.
		m_Containers.clear();

		// Release and remove external geom refs.
		GeomRef::operator=(GeomRef());
	}

	void SetUpdateParticlesJobState(JobUpr::SJobState* pJobState)
	{
		m_pUpdateParticlesJobState = pJobState;
	}

private:

	// Internal emitter flags, extend EParticleEmitterFlags
	enum EFlags
	{
		ePEF_HasPhysics        = BIT(6),
		ePEF_HasTarget         = BIT(7),
		ePEF_HasAttachment     = BIT(8),
		ePEF_NeedsEntityUpdate = BIT(9),
		ePEF_Registered        = BIT(10),
	};

	JobUpr::SJobState* m_pUpdateParticlesJobState;

	// Constant values, effect-related.
	_smart_ptr<CParticleEffect> m_pTopEffect;
	_smart_ptr<IMaterial>       m_pMaterial;          // Override material for this emitter.

	// Cache values derived from the main effect.
	float       m_fMaxParticleSize;

	CTimeValue  m_timeLastUpdate;                     // Track this to automatically update age.
	SpawnParams m_SpawnParams;                        // External settings modifying emission.
	float       m_fEmitCountScale;                    // Composite particle count scale.
	float       m_fViewDistRatio;                     // Normalised value of IRenderNode version.

	ParticleList<CParticleContainer>
	               m_Containers;
	u32         m_nEnvFlags;                       // Union of environment flags affecting emitters.
	uint64         m_nRenObjFlags;                    // Union of render feature flags.
	ParticleTarget m_Target;                          // Target set from external source.

	AABB           m_bbWorld;                         // World bbox.

	float          m_fAgeLastRendered;
	float          m_fBoundsStableAge;                // Next age at which bounds stable.
	float          m_fResetAge;                       // Age to purge unseen particles.
	float          m_fStateChangeAge;                 // Next age at which a container's state changes.
	float          m_fDeathAge;                       // Age when all containers (particles) dead.

	// Entity connection params.
	i32          m_nEntitySlot;
	IEntity*     m_pOwnerEntity = 0;
	u32       m_nEmitterFlags;

	SPhysEnviron m_PhysEnviron;                       // Common physical environment (uniform forces only) for emitter.
	SVisEnviron  m_VisEnviron;

	// Functions.
	void                ResetUnseen();
	void                AllocEmitters();
	void                UpdateContainers();
	void                UpdateTimes(float fAgeAdjust = 0.f);

	void                AddEffect(CParticleContainer* pParentContainer, CParticleEffect const* pEffect, bool bUpdate = true);
	CParticleContainer* AddContainer(CParticleContainer* pParentContainer, const CParticleEffect* pEffect);
};


namespace pfx2
{

class CParticleEmitter : public IParticleEmitter, public DinrusX3dEngBase
{
public:
	using SRenderObjectMaterialPair = std::pair<CRenderObject*, _smart_ptr<IMaterial>>;

public:
	CParticleEmitter(CParticleEffect* pEffect, uint emitterId);
	~CParticleEmitter();

	using TRuntimes = TSmartArray<CParticleComponentRuntime>;

	// IRenderNode
	virtual EERType          GetRenderNodeType() override;
	virtual tukk      GetEntityClassName() const override;
	virtual tukk      GetName() const override;
	virtual void             SetMatrix(const Matrix34& mat) override;
	virtual Vec3             GetPos(bool bWorldOnly = true) const override;
	virtual void             Render(const struct SRendParams& rParam, const SRenderingPassInfo& passInfo) override;
	virtual IPhysicalEntity* GetPhysics() const override                   { return nullptr; }
	virtual void             SetPhysics(IPhysicalEntity*) override         {}
	virtual void             SetMaterial(IMaterial* pMat) override         {}
	virtual IMaterial*       GetMaterial(Vec3* pHitPos = 0) const override { return nullptr; }
	virtual IMaterial*       GetMaterialOverride() override                { return nullptr; }
	virtual float            GetMaxViewDist() override;
	virtual void             Precache() override                           {}
	virtual void             GetMemoryUsage(IDrxSizer* pSizer) const override;
	virtual const AABB       GetBBox() const override;
	virtual void             FillBBox(AABB& aabb) override;
	virtual void             SetBBox(const AABB& WSBBox) override          {}
	virtual void             OffsetPosition(const Vec3& delta) override    {}
	virtual bool             IsAllocatedOutsideOf3DEngineDLL() override    { return false; }
	virtual void             SetViewDistRatio(i32 nViewDistRatio) override;
	virtual void             ReleaseNode(bool bImmediate) override;
	virtual void             SetOwnerEntity(IEntity* pEntity) override     { SetEntity(pEntity, m_entitySlot); }
	virtual IEntity*         GetOwnerEntity() const override               { return m_entityOwner; }
	virtual void             UpdateStreamingPriority(const SUpdateStreamingPriorityContext& streamingContext) override;

	// pfx1 CPArticleEmitter
	virtual i32                    GetVersion() const override                        { return 2; }
	virtual bool                   IsAlive() const override                           { return m_alive; }
	virtual bool                   IsActive() const;
	virtual void                   Activate(bool activate) override;
	virtual void                   Kill() override;
	virtual void                   Restart() override;
	virtual void                   SetEffect(const IParticleEffect* pEffect) override {}
	virtual const IParticleEffect* GetEffect() const override                         { return m_pEffectOriginal; }
	virtual void                   SetEmitGeom(const GeomRef& geom) override;
	virtual void                   SetSpawnParams(const SpawnParams& spawnParams) override;
	virtual void                   GetSpawnParams(SpawnParams& sp) const override;
	using                          IParticleEmitter::GetSpawnParams;
	virtual void                   SetEntity(IEntity* pEntity, i32 nSlot) override;
	virtual void                   InvalidateCachedEntityData() override;
	virtual uint                   GetAttachedEntityId() override;
	virtual i32                    GetAttachedEntitySlot() override                   { return m_entitySlot; }
	virtual void                   SetLocation(const QuatTS& loc) override;
	virtual QuatTS                 GetLocation() const override                       { return m_location; }
	virtual void                   SetTarget(const ParticleTarget& target) override;
	virtual void                   Update() override;
	virtual void                   EmitParticle(const EmitParticleData* pData = NULL)  override;

	// pfx2 IParticleEmitter
	virtual IParticleAttributes& GetAttributes()  override { return m_attributeInstance; }
	virtual void                 SetEmitterFeatures(TParticleFeatures& features) override;

	// CParticleEmitter
	void                      InitSeed();
	void                      DebugRender(const SRenderingPassInfo& passInfo) const;
	void                      CheckUpdated();
	bool                      UpdateParticles();
	void                      SyncUpdateParticles();
	void                      PostUpdate();
	void                      RenderDeferred(const SRenderContext& renderContext);
	CParticleContainer&       GetParentContainer()         { return m_parentContainer; }
	const CParticleContainer& GetParentContainer() const   { return m_parentContainer; }
	const TRuntimes&          GetRuntimes() const          { return m_componentRuntimes; }
	CParticleComponentRuntime*GetRuntimeFor(CParticleComponent* pComponent) { return m_componentRuntimesFor[pComponent->GetComponentId()]; }
	const CParticleEffect*    GetCEffect() const           { return m_pEffect; }
	CParticleEffect*          GetCEffect()                 { return m_pEffect; }
	void                      Register();
	void                      Unregister();
	void                      ResetRenderObjects();
	void                      UpdateEmitGeomFromEntity();
	const SVisEnviron&        GetVisEnv() const            { return m_visEnviron; }
	const SPhysEnviron&       GetPhysicsEnv() const        { return m_physEnviron; }
	const SpawnParams&        GetSpawnParams() const       { return m_spawnParams; }
	const GeomRef&            GetEmitterGeometry() const   { return m_emitterGeometry; }
	QuatTS                    GetEmitterGeometryLocation() const;
	const CAttributeInstance& GetAttributeInstance() const { return m_attributeInstance; }
	TParticleFeatures&        GetFeatures()                { return m_emitterFeatures; }
	const ParticleTarget&     GetTarget() const            { return m_target; }
	float                     GetViewDistRatio() const     { return m_viewDistRatio; }
	float                     GetTimeScale() const         { return DinrusX3dEngBase::GetCVars()->e_ParticlesDebug & AlphaBit('z') ? 0.0f : m_spawnParams.fTimeScale; }
	CRenderObject*            GetRenderObject(uint threadId, uint renderObjectIdx);
	void                      SetRenderObject(CRenderObject* pRenderObject, _smart_ptr<IMaterial>&& material, uint threadId, uint renderObjectIdx);
	float                     GetDeltaTime() const         { return m_time - m_timeUpdated; }
	float                     GetTime() const              { return m_time; }
	float                     GetAge() const               { return m_time - m_timeCreated; }
	bool                      WasRenderedLastFrame() const { return m_unrendered <= 1 && !(GetRndFlags() & ERF_HIDDEN); }
	u32                    GetInitialSeed() const       { return m_initialSeed; }
	u32                    GetCurrentSeed() const       { return m_currentSeed; }
	uint                      GetEmitterId() const         { return m_emitterId; }
	ColorF                    GetProfilerColor() const     { return m_profilerColor; }
	uint                      GetParticleSpec() const;

	void                      SetChanged();
	bool                      IsStable() const             { return m_time > m_timeStable && !m_realBounds.IsReset(); }
	bool                      IsIndependent() const        { return Unique(); }
	bool                      HasParticles() const;
	bool                      HasBounds() const            { return m_bounds.GetVolume() > 0.0f; }
	bool                      NeedsUpdate() const          { return ThreadMode() < 3 || !IsStable() || WasRenderedLastFrame(); }

	struct EmitterStats
	{
		struct
		{
			uint alloc = 0, alive = 0;
		}
		components, particles;
	};
	EmitterStats&             GetStats() { return m_stats; }

private:
	void     UpdateBoundingBox();
	void     UpdateRuntimes();
	void     UpdateFromEntity();
	void     UpdateTargetFromEntity(IEntity* pEntity);
	IEntity* GetEmitGeometryEntity() const;

private:
	_smart_ptr<CParticleEffect>            m_pEffect;
	_smart_ptr<CParticleEffect>            m_pEffectOriginal;
	std::vector<SRenderObjectMaterialPair> m_pRenderObjects[RT_COMMAND_BUF_COUNT];
	SVisEnviron                            m_visEnviron;
	SPhysEnviron                           m_physEnviron;
	SpawnParams                            m_spawnParams;
	CAttributeInstance                     m_attributeInstance;
	TParticleFeatures                      m_emitterFeatures;
	AABB                                   m_realBounds;
	AABB                                   m_nextBounds;
	AABB                                   m_bounds;
	CParticleContainer                     m_parentContainer;
	TRuntimes                              m_componentRuntimesFor;
	TRuntimes                              m_componentRuntimes;
	QuatTS                                 m_location;
	IEntity*                               m_entityOwner;
	i32                                    m_entitySlot;
	ParticleTarget                         m_target;
	GeomRef                                m_emitterGeometry;
	i32                                    m_emitterGeometrySlot;
	ColorF                                 m_profilerColor;
	float                                  m_viewDistRatio;
	float                                  m_time;
	float                                  m_timeCreated;
	float                                  m_timeStable;
	float                                  m_timeUpdated;
	float                                  m_timeDeath;
	i32                                    m_emitterEditVersion;
	i32                                    m_effectEditVersion;
	uint                                   m_initialSeed;
	uint                                   m_currentSeed;
	uint                                   m_emitterId;
	bool                                   m_registered;
	bool                                   m_boundsChanged;
	bool                                   m_active;
	bool                                   m_alive;
	uint                                   m_addInstances;
	uint                                   m_unrendered;
	EmitterStats                           m_stats;
	stl::PSyncMultiThread                  m_lock;
};

typedef TSmartArray<CParticleEmitter> TParticleEmitters;

}



#endif // __particleemitter_h__
