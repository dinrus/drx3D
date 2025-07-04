// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _DECAL_RENDERNODE_
#define _DECAL_RENDERNODE_

#pragma once

#include <drx3D/Eng3D/DecalUpr.h>

class CDecalRenderNode final : public IDecalRenderNode, public DinrusX3dEngBase
{
public:
	// IDecalRenderNode
	virtual void                    SetDecalProperties(const SDecalProperties& properties) override;
	virtual const SDecalProperties* GetDecalProperties() const override;
	virtual const Matrix34&  GetMatrix() override { return m_Matrix; }
	virtual void                    CleanUpOldDecals() override;
	// ~IDecalRenderNode

	// IRenderNode
	virtual IRenderNode*     Clone() const override;
	virtual void             SetMatrix(const Matrix34& mat) override;

	virtual EERType          GetRenderNodeType() override;
	virtual tukk      GetEntityClassName() const override;
	virtual tukk      GetName() const override;
	virtual Vec3             GetPos(bool bWorldOnly = true) const override;
	virtual void             Render(const SRendParams& rParam, const SRenderingPassInfo& passInfo) override;
	virtual IPhysicalEntity* GetPhysics() const override;
	virtual void             SetPhysics(IPhysicalEntity*) override;
	virtual void             SetMaterial(IMaterial* pMat) override;
	virtual IMaterial*       GetMaterial(Vec3* pHitPos = 0) const override;
	virtual IMaterial*       GetMaterialOverride() override { return m_pMaterial; }
	virtual float            GetMaxViewDist() override;
	virtual void             Precache() override;
	virtual void             GetMemoryUsage(IDrxSizer* pSizer) const override;

	virtual const AABB       GetBBox() const override { return m_WSBBox; }
	virtual void             SetBBox(const AABB& WSBBox) override { m_WSBBox = WSBBox; }
	virtual void             FillBBox(AABB& aabb) override;
	virtual void             OffsetPosition(const Vec3& delta) override;

	virtual u8            GetSortPriority() override { return m_decalProperties.m_sortPrio; }

	virtual void             SetLayerId(u16 nLayerId) override { m_nLayerId = nLayerId; Get3DEngine()->C3DEngine::UpdateObjectsLayerAABB(this); }
	virtual u16           GetLayerId() override { return m_nLayerId; }

	// Implementation not currently needed, but required due to assertion in IRenderNode::SetOwnerEntity default implementation
	virtual void SetOwnerEntity(IEntity* pEntity) override {}
	// ~IRenderNode

	static void              ResetDecalUpdatesCounter()  { CDecalRenderNode::m_nFillBigDecalIndicesCounter = 0; }

	// SetMatrix only supports changing position, this will do the full transform
	void SetMatrixFull(const Matrix34& mat);
public:
	CDecalRenderNode();
	void RequestUpdate() { m_updateRequested = true; DeleteDecals(); }
	void DeleteDecals();

private:
	~CDecalRenderNode();
	void CreateDecals();
	void ProcessUpdateRequest();
	void UpdateAABBFromRenderMeshes();
	bool CheckForceDeferred();

	void CreatePlanarDecal();
	void CreateDecalOnStaticObjects();
	void CreateDecalOnTerrain();

private:
	Vec3                  m_pos;
	AABB                  m_localBounds;
	_smart_ptr<IMaterial> m_pOverrideMaterial;
	_smart_ptr<IMaterial> m_pMaterial;
	bool                  m_updateRequested;
	SDecalProperties      m_decalProperties;
	std::vector<CDecal*>  m_decals;
	AABB                  m_WSBBox;
	Matrix34              m_Matrix;
	u32                m_nLastRenderedFrameId;
	u16                m_nLayerId;
	IPhysicalEntity*      m_physEnt;

public:
	static i32 m_nFillBigDecalIndicesCounter;
};

#endif // #ifndef _DECAL_RENDERNODE_
