// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//  предварительные объявления.
struct SEntityEvent;
struct IPhysicalEntity;
struct IPhysicalWorld;

#if 0 // uncomment if more than 64 parts are needed and the mask can no longer fit into an int64
	#include <drx3D/CoreX/BitMask.h>
typedef bitmask_t<bitmaskPtr, 4>    attachMask;
typedef bitmask_t<bitmaskBuf<4>, 4> attachMaskLoc;
	#define attachMask1 (bitmask_t<bitmaskOneBit, 4>(1))
#else
typedef u32 attachMask, attachMaskLoc;
	#define attachMask1 1
#endif

// Implements physical behavior of the entity.
class CEntityPhysics final : public ISimpleEntityEventListener
{
public:
	CEntityPhysics() = default;
	virtual ~CEntityPhysics();

	// ISimpleEntityEventListener
	virtual void ProcessEvent(const SEntityEvent& event) override;
	// ~ISimpleEntityEventListener

	void RegisterEventListeners(IEntityComponent::ComponentEventPriority priority);

	void SerializeXML(XmlNodeRef& entityNode, bool bLoading);
	bool NeedNetworkSerialize();
	void SerializeTyped(TSerialize ser, i32 type, i32 flags);
	void EnableNetworkSerialization(bool enable);
	//////////////////////////////////////////////////////////////////////////

	void             Serialize(TSerialize ser);

	void             GetLocalBounds(AABB& bbox) const;
	void             GetWorldBounds(AABB& bbox) const;

	void             Physicalize(SEntityPhysicalizeParams& params);
	IPhysicalEntity* GetPhysicalEntity() const { return m_pPhysicalEntity; }
	void             EnablePhysics(bool bEnable);
	bool             IsPhysicsEnabled() const;
	void             AddImpulse(i32 ipart, const Vec3& pos, const Vec3& impulse, bool bPos, float fAuxScale, float fPushScale = 1.0f);

	i32              GetPartId0(i32 nSlot = 0);
	i32              GetPhysAttachId();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Called from physics events.
	//////////////////////////////////////////////////////////////////////////
	// Called back by physics PostStep event for the owned physical entity.
	void OnPhysicsPostStep(EventPhysPostStep* pEvent = 0);
	void AttachToPhysicalEntity(IPhysicalEntity* pPhysEntity);
	void CreateRenderGeometry(i32 nSlot, IGeometry* pFromGeom, bop_meshupdate* pLastUpdate = 0);
	//////////////////////////////////////////////////////////////////////////

	void              UpdateSlotGeometry(i32 nSlot, IStatObj* pStatObjNew = 0, float mass = -1.0f, i32 bNoSubslots = 1);
	void              AssignPhysicalEntity(IPhysicalEntity* pPhysEntity, i32 nSlot = -1);

	virtual bool      PhysicalizeFoliage(i32 iSlot);
	virtual void      DephysicalizeFoliage(i32 iSlot);
	virtual IFoliage* GetFoliage(i32 iSlot);

	i32               AddSlotGeometry(i32 nSlot, SEntityPhysicalizeParams& params, i32 bNoSubslots = 1);
	void              RemoveSlotGeometry(i32 nSlot);

	void              MovePhysics(CEntityPhysics* dstPhysics);

	virtual void      GetMemoryUsage(IDrxSizer* pSizer) const;
	void              ReattachSoftEntityVtx(IPhysicalEntity* pAttachToEntity, i32 nAttachToPart);

#if !defined(_RELEASE)
	static void EnableValidation();
	static void DisableValidation();
#endif

private:
	IPhysicalWorld*  PhysicalWorld() const { return gEnv->pPhysicalWorld; }
	void             OnEntityXForm(const SEntityEvent& event);
	void             OnChangedPhysics(bool bEnabled);
	void             DestroyPhysicalEntity(bool bDestroyCharacters = true, i32 iMode = 0);

	void             PhysicalizeSimple(SEntityPhysicalizeParams& params);
	void             PhysicalizeLiving(SEntityPhysicalizeParams& params);
	void             PhysicalizeParticle(SEntityPhysicalizeParams& params);
	void             PhysicalizeSoft(SEntityPhysicalizeParams& params);
	void             AttachSoftVtx(IRenderMesh* pRM, IPhysicalEntity* pAttachToEntity, i32 nAttachToPart);
	void             PhysicalizeArea(SEntityPhysicalizeParams& params);
	bool             PhysicalizeGeomCache(SEntityPhysicalizeParams& params);
	bool             PhysicalizeCharacter(SEntityPhysicalizeParams& params);
	bool             ConvertCharacterToRagdoll(SEntityPhysicalizeParams& params, const Vec3& velInitial);

	void             CreatePhysicalEntity(SEntityPhysicalizeParams& params);
	phys_geometry*   GetSlotGeometry(i32 nSlot);
	void             SyncCharacterWithPhysics();

	void             MoveChildPhysicsParts(IPhysicalEntity* pSrcAdam, CEntity* pChild, pe_action_move_parts& amp, uint64 usedRanges);

	IPhysicalEntity* QueryPhyscalEntity(IEntity* pEntity) const;
	CEntity*         GetCEntity(IPhysicalEntity* pPhysEntity);

	void             ReleasePhysicalEntity();

	void             RemapChildAttachIds(CEntity* pent, attachMask& idmaskSrc, attachMask& idmaskDst, i32* idmap);

	bool             TriggerEventIfStateChanged(IPhysicalEntity* pPhysEntity, const pe_status_pos* pPrevStatus) const;
	// Figures out render material at slot nSlot, and fills necessary data into the ppart output structure
	void             UpdateParamsFromRenderMaterial(i32 nSlot, IPhysicalEntity* pPhysEntity);

	void             AwakeOnRender(bool vRender);
	void             OnTimer(i32 id);

private:
	CEntity* GetEntity() const;

private:
	friend class CEntity;

	// Pointer to physical object.
	IPhysicalEntity* m_pPhysicalEntity = nullptr;
	i32 m_timerId = IEntity::CREATE_NEW_UNIQUE_TIMER_ID;
};
