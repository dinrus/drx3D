// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IBreakableUpr.h>

class CEntitySystem;
class CEntity;
struct GeomRef;
//////////////////////////////////////////////////////////////////////////
//
// BreakableUpr manager handles all the code for breaking/destroying entity geometry.
//
//////////////////////////////////////////////////////////////////////////
class CBreakableUpr : public IBreakableUpr
{
public:
	CBreakableUpr() = default;
	virtual ~CBreakableUpr() {}

	// actual breaking function.
	void BreakIntoPieces(GeomRef& geoOrig, const Matrix34& srcObjTM,
	                     IStatObj* pPiecesObj, const Matrix34& piecesObjTM,	IStatObj *pRemovedObj,
	                     BreakageParams const& Breakage, i32 nMatLayers);

	//////////////////////////////////////////////////////////////////////////
	// IBreakableUpr implementation
	//////////////////////////////////////////////////////////////////////////
	virtual void BreakIntoPieces(IEntity* pEntity, i32 nOrigSlot, i32 nPiecesSlot, BreakageParams const& Breakage);

	// Breakable Plane
	virtual EProcessImpactResult ProcessPlaneImpact(const SProcessImpactIn& in, SProcessImpactOut& out);
	virtual void                 ExtractPlaneMeshIsland(const SExtractMeshIslandIn& in, SExtractMeshIslandOut& out);
	virtual void                 GetPlaneMemoryStatistics(uk pPlaneRaw, IDrxSizer* pSizer);
	virtual bool                 IsGeometryBreakable(IPhysicalEntity* pEntity, IStatObj* pStatObj, IMaterial* pMaterial);

	// Check if this stat object can shatter.
	// Check if its materials support shattering.
	virtual bool CanShatter(IStatObj* pStatObj);
	virtual bool CanShatterEntity(IEntity* pEntity, i32 nSlot = -1);

	// Attach the effect & params specified by material of object in slot.
	virtual void AttachSurfaceEffect(IEntity* pEntity, i32 nSlot, tukk sType, SpawnParams const& paramsIn);
	virtual void CreateSurfaceEffect(IStatObj* pStatObj, const Matrix34& tm, tukk sType);
	//////////////////////////////////////////////////////////////////////////

	bool                  CanShatterRenderMesh(IRenderMesh* pMesh, IMaterial* pMtl);

	virtual void          AddBreakEventListener(IBreakEventListener* pListener);
	virtual void          RemoveBreakEventListener(IBreakEventListener* pListener);

	virtual void          EntityDrawSlot(CEntity* pEntity, i32 slot, i32 flags);

	virtual ISurfaceType* GetFirstSurfaceType(IStatObj* pStatObj);
	virtual ISurfaceType* GetFirstSurfaceType(ICharacterInstance* pCharacter);

	virtual void          ReplayRemoveSubPartsEvent(const EventPhysRemoveEntityParts* pRemoveEvent);

	void                  HandlePhysicsCreateEntityPartEvent(const EventPhysCreateEntityPart* pCreateEvent);
	void                  HandlePhysicsRemoveSubPartsEvent(const EventPhysRemoveEntityParts* pRemoveEvent);
	void                  HandlePhysicsRevealSubPartEvent(const EventPhysRevealEntityPart* pRevealEvent);
	i32                   HandlePhysics_UpdateMeshEvent(const EventPhysUpdateMesh* pEvent);

	void                  FakePhysicsEvent(EventPhys* pEvent);

	// Resets broken objects
	void ResetBrokenObjects();

	// Freeze/unfreeze render node.
	void                                            FreezeRenderNode(IRenderNode* pRenderNode, bool bEnable);

	void                                            CreateObjectAsParticles(IStatObj* pStatObj, IPhysicalEntity* pPhysEnt, IBreakableUpr::SCreateParams& createParams);
	virtual IEntity*                                CreateObjectAsEntity(IStatObj* pStatObj, IPhysicalEntity* pPhysEnt, IPhysicalEntity* pSrcPhysEnt, IBreakableUpr::SCreateParams& createParams, bool bCreateSubstProxy = false);
	bool                                            CheckForPieces(IStatObj* pStatObjSrc, IStatObj::SSubObject* pSubObj, const Matrix34& worldTM, i32 nMatLayers, IPhysicalEntity* pPhysEnt);
	IStatObj::SSubObject*                           CheckSubObjBreak(IStatObj* pStatObj, IStatObj::SSubObject* pSubObj, const EventPhysCreateEntityPart* epcep);

	virtual const IBreakableUpr::SBrokenObjRec* GetPartBrokenObjects(i32& brokenObjectCount) { brokenObjectCount = m_brokenObjs.size(); return &m_brokenObjs[0]; }

	virtual void                                    GetBrokenObjectIndicesForCloning(i32* pPartRemovalIndices, i32& iNumPartRemovalIndices,
	                                                                                 i32* pOutIndiciesForCloning, i32& iNumEntitiesForCloning,
	                                                                                 const EventPhysRemoveEntityParts* BreakEvents);
	virtual void ClonePartRemovedEntitiesByIndex(i32* pBrokenObjectIndices, i32 iNumBrokenObjectIndices,
	                                             EntityId* pOutClonedEntities, i32& iNumClonedBrokenEntities,
	                                             const EntityId* pRecordingEntities, i32 iNumRecordingEntities,
	                                             SRenderNodeCloneLookup& nodeLookup);

	virtual void HideBrokenObjectsByIndex(i32k* pBrokenObjectIndices, i32k iNumBrokenObjectIndices);
	virtual void UnhidePartRemovedObjectsByIndex(i32k* pPartRemovalIndices, i32k iNumPartRemovalIndices, const EventPhysRemoveEntityParts* pBreakEvents);
	virtual void ApplySinglePartRemovalFromEventIndex(i32 iPartRemovalEventIndex, const SRenderNodeCloneLookup& renderNodeLookup, const EventPhysRemoveEntityParts* pBreakEvents);
	virtual void ApplyPartRemovalsUntilIndexToObjectList(i32 iFirstEventIndex, const SRenderNodeCloneLookup& renderNodeLookup, const EventPhysRemoveEntityParts* pBreakEvents);

private:
	void CreateObjectCommon(IStatObj* pStatObj, IPhysicalEntity* pPhysEnt, IBreakableUpr::SCreateParams& createParams);

	void ApplyPartBreakToClonedObjectFromEvent(const SRenderNodeCloneLookup& renderNodeLookup, const EventPhysRemoveEntityParts& OriginalEvent);

	// Remove Parts
	bool RemoveStatObjParts(IStatObj*& pStatObj);

	IBreakEventListener* m_pBreakEventListener = nullptr;
	//////////////////////////////////////////////////////////////////////////

	std::vector<IBreakableUpr::SBrokenObjRec> m_brokenObjs;
	std::vector<IPhysicalEntity*>                 m_brokenObjsParticles;
};
