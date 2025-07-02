// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _3DENGINE_BRUSH_H_
#define _3DENGINE_BRUSH_H_

#include <drx3D/Eng3D/ObjMan.h>
#include <drx3D/Eng3D/DeformableNode.h>

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#include <drx3D/CoreX/Platform/platform.h>
#endif

class CBrush
	: public IBrush
	  , public DinrusX3dEngBase
{
	friend class COctreeNode;

public:
	CBrush();
	virtual ~CBrush();

	virtual tukk         GetEntityClassName() const final;
	virtual Vec3                GetPos(bool bWorldOnly = true) const final;
	virtual float               GetScale() const;
	virtual tukk         GetName() const final;
	virtual bool                HasChanged();
	virtual void                Render(const struct SRendParams& EntDrawParams, const SRenderingPassInfo& passInfo) final;
	virtual CLodValue           ComputeLod(i32 wantedLod, const SRenderingPassInfo& passInfo) final;
	void                        Render(const CLodValue& lodValue, const SRenderingPassInfo& passInfo, SSectorTextureSet* pTerrainTexInfo, PodArray<SRenderLight*>* pAffectingLights);

	virtual struct IStatObj*    GetEntityStatObj(u32 nSubPartId = 0, Matrix34A* pMatrix = NULL, bool bReturnOnlyVisible = false) final;

	virtual bool                GetLodDistances(const SFrameLodInfo& frameLodInfo, float* distances) const final;

	virtual void                SetEntityStatObj(IStatObj* pStatObj, const Matrix34A* pMatrix = NULL) final;

	virtual IRenderNode*        Clone() const final;

	virtual void                SetCollisionClassIndex(i32 tableIndex) final { m_collisionClassIdx = tableIndex; }

	virtual void                SetLayerId(u16 nLayerId) final;
	virtual u16              GetLayerId() final                           { return m_nLayerId; }
	virtual struct IRenderMesh* GetRenderMesh(i32 nLod) final;

	virtual IPhysicalEntity*    GetPhysics() const final;
	virtual void                SetPhysics(IPhysicalEntity* pPhys) final;
	static bool                 IsMatrixValid(const Matrix34& mat);
	virtual void                Dephysicalize(bool bKeepIfReferenced = false) final;
	virtual void                Physicalize(bool bInstant = false) final;
	void                        PhysicalizeOnHeap(IGeneralMemoryHeap* pHeap, bool bInstant = false);
	virtual bool                PhysicalizeFoliage(bool bPhysicalize = true, i32 iSource = 0, i32 nSlot = 0) final;
	virtual IPhysicalEntity*    GetBranchPhys(i32 idx, i32 nSlot = 0) final { IFoliage* pFoliage = GetFoliage(nSlot); return pFoliage ? pFoliage->GetBranchPhysics(idx) : 0; }
	virtual struct IFoliage*    GetFoliage(i32 nSlot = 0) final;

	//! Assign final material to this entity.
	virtual void       SetMaterial(IMaterial* pMat) final;
	virtual IMaterial* GetMaterial(Vec3* pHitPos = NULL) const final;
	virtual IMaterial* GetMaterialOverride() final { return m_pMaterial; };
	virtual void       CheckPhysicalized() final;

	virtual float      GetMaxViewDist() final;

	virtual EERType    GetRenderNodeType();

	void               SetStatObj(IStatObj* pStatObj);

	void               SetMatrix(const Matrix34& mat) final;
	const Matrix34& GetMatrix() const final        { return m_Matrix; }
	virtual void    SetDrawLast(bool enable) final { m_bDrawLast = enable; }
	bool            GetDrawLast() const            { return m_bDrawLast; }

	void            Dematerialize() final;
	virtual void    GetMemoryUsage(IDrxSizer* pSizer) const final;
	static PodArray<SExportedBrushMaterial> m_lstSExportedBrushMaterials;

	virtual const AABB GetBBox() const final;
	virtual void       SetBBox(const AABB& WSBBox) final { m_WSBBox = WSBBox; }
	virtual void       FillBBox(AABB& aabb) final;
	virtual void       OffsetPosition(const Vec3& delta) final;

	virtual void SetCameraSpacePos( Vec3* pCameraSpacePos ) final;
	virtual void SetSubObjectHideMask( hidemask subObjHideMask ) final;

	virtual bool       CanExecuteRenderAsJob() final;
	
	virtual void       DisablePhysicalization(bool bDisable) final;

	//private:
	void CalcBBox();
	void UpdatePhysicalMaterials(i32 bThreadSafe = 0);

	virtual void OnRenderNodeBecomeVisibleAsync(SRenderNodeTempData* pTempData, const SRenderingPassInfo& passInfo) final;

	bool HasDeformableData() const { return m_pDeform != NULL; }


private:
	void CalcNearestTransform( Matrix34 &transformMatrix,const SRenderingPassInfo& passInfo );
	void InvalidatePermanentRenderObjectMatrix();

public:
	// Transformation Matrix
	Matrix34         m_Matrix;

	// Physical Entity, when this node is physicalized.
	IPhysicalEntity* m_pPhysEnt = nullptr;

	//! Override material, will override default material assigned to the Geometry.
	_smart_ptr<IMaterial> m_pMaterial;

	u16                m_collisionClassIdx = 0;
	u16                m_nLayerId = 0;

	// Geometry referenced and rendered by this node.
	_smart_ptr<CStatObj>  m_pStatObj;

	CDeformableNode*      m_pDeform = nullptr;
	IFoliage*             m_pFoliage = nullptr;

	u32 m_bVehicleOnlyPhysics : 1;
	u32 m_bDrawLast : 1;
	u32 m_bNoPhysicalize : 1;

	// World space bounding box for this node.
	AABB m_WSBBox;

	// Last frame this object moved
	u32 m_lastMoveFrameId = 0;
	// Hide mask disable individual sub-objects rendering in the compound static objects
	hidemask m_nSubObjHideMask;

	Vec3*    m_pCameraSpacePos = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
inline const AABB CBrush::GetBBox() const
{
	return m_WSBBox;
}

///////////////////////////////////////////////////////////////////////////////
class CMovableBrush : public CBrush
{
	virtual void     SetOwnerEntity(struct IEntity* pEntity) final { m_pOwnerEntity = pEntity; }
	virtual IEntity* GetOwnerEntity() const final                  { return m_pOwnerEntity; }
	virtual EERType  GetRenderNodeType() final                     { return eERType_MovableBrush; }
	virtual bool     IsAllocatedOutsideOf3DEngineDLL()             { return GetOwnerEntity() != nullptr; }

private:
	// When render node is created by the entity, pointer to the owner entity.
	IEntity* m_pOwnerEntity = 0;
};

#endif // _3DENGINE_BRUSH_H_
