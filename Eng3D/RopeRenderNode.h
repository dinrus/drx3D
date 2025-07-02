// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/ISplines.h>
#include <drx3D/Phys/IDeferredCollisionEvent.h>

class CRopeRenderNode : public IRopeRenderNode, public DinrusX3dEngBase
{
public:
	static void StaticReset();
	static i32  OnPhysStateChange(EventPhys const* pEvent);

public:
	//////////////////////////////////////////////////////////////////////////
	// implements IRenderNode
	virtual void             GetLocalBounds(AABB& bbox) override;
	virtual void             SetMatrix(const Matrix34& mat) override;

	virtual EERType          GetRenderNodeType() override;
	virtual tukk      GetEntityClassName() const override;
	virtual tukk      GetName() const override;
	virtual Vec3             GetPos(bool bWorldOnly = true) const override;
	virtual void             Render(const SRendParams& rParam, const SRenderingPassInfo& passInfo) override;
	virtual IPhysicalEntity* GetPhysics() const override;
	virtual void             SetPhysics(IPhysicalEntity*) override;
	virtual void             Physicalize(bool bInstant = false) override;
	virtual void             Dephysicalize(bool bKeepIfReferenced = false) override;
	virtual void             SetMaterial(IMaterial* pMat) override;
	virtual IMaterial*       GetMaterial(Vec3* pHitPos = 0) const override;
	virtual IMaterial*       GetMaterialOverride() override { return m_pMaterial; }
	virtual float            GetMaxViewDist() override;
	virtual void             Precache() override;
	virtual void             GetMemoryUsage(IDrxSizer* pSizer) const override;
	virtual void             LinkEndPoints() override;
	virtual const AABB       GetBBox() const override             { return m_WSBBox; }
	virtual void             SetBBox(const AABB& WSBBox) override { m_WSBBox = WSBBox; m_bNeedToReRegister = true; }
	virtual void             FillBBox(AABB& aabb) override;
	virtual void             OffsetPosition(const Vec3& delta) override;

	// Set a new owner entity
	virtual void             SetOwnerEntity(IEntity* pEntity) final { m_pEntity = pEntity; }
	// Retrieve a pointer to the entity who owns this render node.
	virtual IEntity*         GetOwnerEntity() const final { return m_pEntity; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IRopeRenderNode implementation
	//////////////////////////////////////////////////////////////////////////
	virtual void                                SetName(tukk sNodeName) override;
	virtual void                                SetParams(const SRopeParams& params) override;
	virtual const IRopeRenderNode::SRopeParams& GetParams() const override;

	virtual void                                SetPoints(const Vec3* pPoints, i32 nCount) override;
	virtual i32                                 GetPointsCount() const override;
	virtual const Vec3*                         GetPoints() const override;

	virtual u32                              GetLinkedEndsMask() override { return m_nLinkedEndsMask; };
	virtual void                                OnPhysicsPostStep() override;

	virtual void                                ResetPoints() override;

	virtual void                                LinkEndEntities(IPhysicalEntity* pStartEntity, IPhysicalEntity* pEndEntity) override;
	virtual void                                GetEndPointLinks(SEndPointLink* links) override;

	virtual void                                DisableAudio() override;
	virtual void                                SetAudioParams(SRopeAudioParams const& audioParams) override;
	//////////////////////////////////////////////////////////////////////////

public:
	CRopeRenderNode();

	void CreateRenderMesh();
	void UpdateRenderMesh();
	void AnchorEndPoints(pe_params_rope& pr);
	void SyncWithPhysicalRope(bool bForce);
	bool RenderDebugInfo(const SRendParams& rParams, const SRenderingPassInfo& passInfo);

private:
	~CRopeRenderNode();

	void UpdateAudio();

private:
	string                  m_sName;
	IEntity*                m_pEntity = nullptr;
	Vec3                    m_pos;
	AABB                    m_localBounds;
	Matrix34                m_worldTM;
	Matrix34                m_InvWorldTM;
	_smart_ptr<IMaterial>   m_pMaterial;
	_smart_ptr<IRenderMesh> m_pRenderMesh;
	IPhysicalEntity*        m_pPhysicalEntity;

	u32                  m_nLinkedEndsMask;

	// Flags
	u32            m_bModified                 : 1;
	u32            m_bRopeCreatedInsideVisArea : 1;
	u32            m_bNeedToReRegister         : 1;
	u32            m_bStaticPhysics            : 1;

	std::vector<Vec3> m_points;
	std::vector<Vec3> m_physicsPoints;

	SRopeParams       m_params;

	typedef spline::CatmullRomSpline<Vec3> SplineType;
	SplineType         m_spline;

	AABB               m_WSBBox;

	DrxAudio::IObject* m_pIAudioObject;
	SRopeAudioParams   m_audioParams;
};
