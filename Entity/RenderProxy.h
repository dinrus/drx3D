// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/EntitySystem.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Eng3D/IRenderNode.h>

//  предварительные объявления.
class CEntitySlot;
class CEntity;
struct SRendParams;
struct IShaderPublicParams;
class CEntityRender;
struct AnimEventInstance;
struct IRenderNode;

//////////////////////////////////////////////////////////////////////////
// Описание:
//    CEntityRender provide rendering capabilities implementation for the Entity.
//    It can contain multiple sub object slots that can have their own relative transformation, and
//    each slot can represent specific renderable node (IStatObj,ICharacterInstance,etc..)
///////////////////////////////////////////////////////////// /////////////
class CEntityRender final : public ISimpleEntityEventListener
{
public:
	CEntityRender();
	virtual ~CEntityRender();

	// ISimpleEntityEventListener
	virtual void ProcessEvent(const SEntityEvent& event) override;
	// ~ISimpleEntityEventListener

	// Must be called after constructor.
	void PostInit();
	void RegisterEventListeners(IEntityComponent::ComponentEventPriority priority);

	void Serialize(TSerialize ser);
	bool NeedNetworkSerialize();

	//////////////////////////////////////////////////////////////////////////
	// IEntityRender interface.
	//////////////////////////////////////////////////////////////////////////
	void         SetLocalBounds(const AABB& bounds, bool bDoNotRecalculate);
	void         GetWorldBounds(AABB& bounds) const;
	void         GetLocalBounds(AABB& bounds) const;
	void         InvalidateLocalBounds();

	IMaterial*   GetRenderMaterial(i32 nSlot = -1) const;
	IRenderNode* GetRenderNode(i32 nSlot = -1) const;

	void         SetSlotMaterial(i32 nSlot, IMaterial* pMaterial);
	IMaterial*   GetSlotMaterial(i32 nSlot) const;

	float        GetLastSeenTime() const { return m_fLastSeenTime; }
	//////////////////////////////////////////////////////////////////////////

	bool                IsSlotValid(i32 nIndex) const { return nIndex >= 0 && nIndex < (i32)m_slots.size() && m_slots[nIndex] != NULL; };
	i32                 GetSlotCount() const;
	bool                SetParentSlot(i32 nParentIndex, i32 nChildIndex);
	bool                GetSlotInfo(i32 nIndex, SEntitySlotInfo& slotInfo) const;

	void                SetSlotFlags(i32 nSlot, u32 nFlags);
	u32              GetSlotFlags(i32 nSlot) const;

	ICharacterInstance* GetCharacter(i32 nSlot);
	IStatObj*           GetStatObj(i32 nSlot);
	IParticleEmitter*   GetParticleEmitter(i32 nSlot);

	i32                 SetSlotRenderNode(i32 nSlot, IRenderNode* pRenderNode);
	i32                 SetSlotGeometry(i32 nSlot, IStatObj* pStatObj);
	i32                 SetSlotCharacter(i32 nSlot, ICharacterInstance* pCharacter);
	i32                 LoadGeometry(i32 nSlot, tukk sFilename, tukk sGeomName = NULL, i32 nLoadFlags = 0);
	i32                 LoadCharacter(i32 nSlot, tukk sFilename, i32 nLoadFlags = 0);
	i32                 LoadParticleEmitter(i32 nSlot, IParticleEffect* pEffect, SpawnParams const* params = NULL, bool bPrime = false, bool bSerialize = false);
	i32                 SetParticleEmitter(i32 nSlot, IParticleEmitter* pEmitter, bool bSerialize = false);
	i32                 LoadLight(i32 nSlot, SRenderLight* pLight, u16 layerId);
	i32                 LoadCloudBlocker(i32 nSlot, const SCloudBlockerProperties& properties);
	i32                 LoadFogVolume(i32 nSlot, const SFogVolumeProperties& properties);
	i32                 FadeGlobalDensity(i32 nSlot, float fadeTime, float newGlobalDensity);
#if defined(USE_GEOM_CACHES)
	i32                 LoadGeomCache(i32 nSlot, tukk sFilename);
#endif

	//////////////////////////////////////////////////////////////////////////
	// Slots.
	//////////////////////////////////////////////////////////////////////////
	ILINE CEntitySlot* GetSlot(i32 nIndex) const { return (nIndex >= 0 && nIndex < (i32)m_slots.size()) ? m_slots[nIndex] : NULL; }
	CEntitySlot*       GetParentSlot(i32 nIndex) const;

	// Allocate a new object slot.
	CEntitySlot* AllocSlot(i32 nIndex);
	// Frees existing object slot, also optimizes size of slot array.
	void         FreeSlot(i32 nIndex);
	void         FreeAllSlots();

	// Returns world transformation matrix of the object slot.
	Matrix34                          GetSlotWorldTM(i32 nIndex) const;
	// Returns local relative to host entity transformation matrix of the object slot.
	Matrix34                          GetSlotLocalTM(i32 nIndex, bool bRelativeToParent) const;
	// Set local transformation matrix of the object slot.
	void                              SetSlotLocalTM(i32 nIndex, const Matrix34& localTM, EntityTransformationFlagsMask transformReasons = EntityTransformationFlagsMask());
	// Set camera space position of the object slot
	void                              SetSlotCameraSpacePos(i32 nIndex, const Vec3& cameraSpacePos);
	// Get camera space position of the object slot
	void                              GetSlotCameraSpacePos(i32 nSlot, Vec3& cameraSpacePos) const;

	void                              SetSubObjHideMask(i32 nSlot, hidemask nSubObjHideMask);
	hidemask                          GetSubObjHideMask(i32 nSlot) const;

	void                              SetRenderNodeParams(const IEntity::SRenderNodeParams& params);
	IEntity::SRenderNodeParams&       GetRenderNodeParams()       { return m_renderNodeParams; };
	const IEntity::SRenderNodeParams& GetRenderNodeParams() const { return m_renderNodeParams; };
	//////////////////////////////////////////////////////////////////////////

	// Internal slot access function.
	ILINE CEntitySlot* Slot(i32 nIndex) const { assert(nIndex >= 0 && nIndex < (i32)m_slots.size()); return m_slots[nIndex]; }
	IStatObj*          GetCompoundObj() const;

	void               SetLastSeenTime(float fNewTime) { m_fLastSeenTime = fNewTime; }

	void               DebugDraw(const SGeometryDebugDrawInfo& info);
	// Calls UpdateRenderNode on every slot.
	void               UpdateRenderNodes();

	void               CheckLocalBoundsChanged();

public:
	static i32 AnimEventCallback(ICharacterInstance* pCharacter, uk userdata);

	// Check if any render nodes are rendered recently.
	bool IsRendered() const;
	void PreviewRender(SEntityPreviewContext& context);

private:
	void ComputeLocalBounds(bool bForce = false);
	void OnEntityXForm(i32 nWhyFlags);

	// Get existing slot or make a new slot if not exist.
	// Is nSlot is negative will allocate a new available slot and return it Index in nSlot parameter.
	CEntitySlot* GetOrMakeSlot(i32& nSlot);

	I3DEngine*   GetI3DEngine() { return gEnv->p3DEngine; }
	void         AnimationEvent(ICharacterInstance* pCharacter, const AnimEventInstance& event);

	CEntity*     GetEntity() const;

private:
	friend class CEntitySlot;
	friend class CEntity;

	// Object Slots.
	std::vector<CEntitySlot*> m_slots;

	// Local bounding box of render proxy.
	AABB m_localBBox = AABB(ZERO, ZERO);

	// Time passed since this entity was seen last time  (wrong: this is an absolute time, todo: fix float absolute time values)
	float m_fLastSeenTime;

	// Rendering related member variables, Passed to 3d engine render nodes.
	IEntity::SRenderNodeParams m_renderNodeParams;
};
