// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef LIGHT_ENTITY_H
#define LIGHT_ENTITY_H

const float LIGHT_PROJECTOR_MAX_FOV = 175.f;

struct ShadowMapInfo
{
	i32 GetLodCount() const { return dynamicLodCount + cachedLodCount; }

	ShadowMapFrustumPtr pGSM[MAX_GSM_LODS_NUM];

	i32                 dynamicLodCount = 0;
	i32                 cachedLodCount = 0;
};

class CLightEntity : public ILightSource, public DinrusX3dEngBase
{
public:
	CLightEntity();
	~CLightEntity();
	static void                          StaticReset();
	virtual EERType                      GetRenderNodeType();
	virtual tukk                  GetEntityClassName(void) const { return "LightEntityClass"; }
	virtual tukk                  GetName(void) const;
	virtual void                         GetLocalBounds(AABB& bbox);
	virtual Vec3                         GetPos(bool) const;
	virtual void                         Render(const SRendParams&, const SRenderingPassInfo& passInfo);
	virtual void                         Hide(bool bHide);
	virtual IPhysicalEntity*             GetPhysics(void) const                  { return 0; }
	virtual void                         SetPhysics(IPhysicalEntity*)            {}
	virtual void                         SetMaterial(IMaterial* pMat)            { m_pMaterial = pMat; }
	virtual IMaterial*                   GetMaterial(Vec3* pHitPos = NULL) const { return m_pMaterial; }
	virtual IMaterial*                   GetMaterialOverride()                   { return m_pMaterial; }
	virtual float                        GetMaxViewDist();
	virtual void                         SetLightProperties(const SRenderLight& light);
	virtual SRenderLight&                GetLightProperties()       { return m_light; };
	virtual const SRenderLight&          GetLightProperties() const { return m_light; };
	virtual void                         Release(bool);
	virtual void                         SetMatrix(const Matrix34& mat);
	virtual const Matrix34&              GetMatrix() { return m_Matrix; }
	virtual struct ShadowMapFrustum*     GetShadowFrustum(i32 nId = 0);
	virtual void                         GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual const AABB                   GetBBox() const             { return m_WSBBox; }
	virtual void                         SetBBox(const AABB& WSBBox) { m_WSBBox = WSBBox; }
	virtual void                         FillBBox(AABB& aabb);
	virtual void                         OffsetPosition(const Vec3& delta);
	virtual void                         SetCastingException(IRenderNode* pNotCaster) { m_pNotCaster = pNotCaster; }
	IRenderNode*                         GetCastingException()                        { return m_pNotCaster; }
	virtual bool                         IsLightAreasVisible();
	static const PodArray<SPlaneObject>& GetCastersHull()                             { return s_lstTmpCastersHull; }
	virtual void                         SetViewDistRatio(i32 nViewDistRatio);
#if defined(FEATURE_SVO_GI)
	virtual EGIMode                      GetGIMode() const;
#endif
	virtual void                         SetOwnerEntity(IEntity* pEntity);
	virtual IEntity*                     GetOwnerEntity() const final      { return m_pOwnerEntity; }
	virtual bool                         IsAllocatedOutsideOf3DEngineDLL() { return GetOwnerEntity() != nullptr; }
	void                                 InitEntityShadowMapInfoStructure(i32 dynamicLods, i32 cachedLods);
	void                                 UpdateGSMLightSourceShadowFrustum(const SRenderingPassInfo& passInfo);
	i32                                  UpdateGSMLightSourceDynamicShadowFrustum(i32 nDynamicLodCount, i32 nDistanceLodCount, float& fDistanceFromViewLastDynamicLod, float& fGSMBoxSizeLastDynamicLod, bool bFadeLastCascade, const SRenderingPassInfo& passInfo);
	i32                                  UpdateGSMLightSourceCachedShadowFrustum(i32 nFirstLod, i32 nLodCount, bool isHeightMapAOEnabled, float& fDistFromViewDynamicLod, float fRadiusDynamicLod, const SRenderingPassInfo& passInfo);
	i32                                  UpdateGSMLightSourceNearestShadowFrustum(i32 nFrustumIndex, const SRenderingPassInfo& passInfo);
	bool                                 ProcessFrustum(i32 nLod, float fCamBoxSize, float fDistanceFromView, PodArray<struct SPlaneObject>& lstCastersHull, const SRenderingPassInfo& passInfo);
	void                                 CollectShadowCascadeForOnePassTraversal(ShadowMapFrustum* pFr);
	static bool                          IsOnePassTraversalFrustum(const ShadowMapFrustum* pFr);
	static void                          SetShadowFrustumsCollector(std::vector<std::pair<ShadowMapFrustum*, const CLightEntity*>>* p) { s_pShadowFrustumsCollector = p; }
	static void                          ProcessPerObjectFrustum(ShadowMapFrustum* pFr, struct SPerObjectShadow* pPerObjectShadow, ILightSource* pLightSource, const SRenderingPassInfo& passInfo);
	void                                 InitShadowFrustum_SUN_Conserv(ShadowMapFrustum* pFr, i32 dwAllowedTypes, float fGSMBoxSize, float fDistance, i32 nLod, const SRenderingPassInfo& passInfo);
	void                                 InitShadowFrustum_PROJECTOR(ShadowMapFrustum* pFr, i32 dwAllowedTypes, const SRenderingPassInfo& passInfo);
	void                                 InitShadowFrustum_OMNI(ShadowMapFrustum* pFr, i32 dwAllowedTypes, const SRenderingPassInfo& passInfo);
	void                                 SetupShadowFrustumCamera_SUN(ShadowMapFrustum* pFr, i32 dwAllowedTypes, i32 nRenderNodeFlags, PodArray<struct SPlaneObject>& lstCastersHull, i32 nLod, const SRenderingPassInfo& passInfo);
	void                                 SetupShadowFrustumCamera_PROJECTOR(ShadowMapFrustum* pFr, i32 dwAllowedTypes, const SRenderingPassInfo& passInfo);
	void                                 SetupShadowFrustumCamera_OMNI(ShadowMapFrustum* pFr, i32 dwAllowedTypes, const SRenderingPassInfo& passInfo);
	void                                 CheckValidFrustums_OMNI(ShadowMapFrustum* pFr, const SRenderingPassInfo& passInfo);
	bool                                 CheckFrustumsIntersect(CLightEntity* lightEnt);
	bool                                 GetGsmFrustumBounds(const CCamera& viewFrustum, ShadowMapFrustum* pShadowFrustum);
	void                                 OnCasterDeleted(IShadowCaster* pCaster);
	i32                                  MakeShadowCastersHull(PodArray<SPlaneObject>& lstCastersHull, const SRenderingPassInfo& passInfo);
	static Vec3                          GSM_GetNextScreenEdge(float fPrevRadius, float fPrevDistanceFromView, const SRenderingPassInfo& passInfo);
	static float                         GSM_GetLODProjectionCenter(const Vec3& vEdgeScreen, float fRadius);
	static bool                          FrustumIntersection(const CCamera& viewFrustum, const CCamera& shadowFrustum);
	void                                 UpdateCastShadowFlag(float fDistance, const SRenderingPassInfo& passInfo);
	void                                 CalculateShadowBias(ShadowMapFrustum* pFr, i32 nLod, float fGSMBoxSize) const;
	void                                 SetLayerId(u16 nLayerId);
	u16                               GetLayerId()       { return m_layerId; }
	ShadowMapInfo*                       GetShadowMapInfo() { return m_pShadowMapInfo.get(); }

private:
	static PodArray<SPlaneObject>  s_lstTmpCastersHull;
	IEntity*                       m_pOwnerEntity = 0;
	_smart_ptr<IMaterial>          m_pMaterial;
	Matrix34                       m_Matrix;
	std::shared_ptr<ShadowMapInfo> m_pShadowMapInfo;
	SRenderLight                   m_light;
	IRenderNode*                   m_pNotCaster;
	AABB                           m_WSBBox;
	u16                         m_layerId;
	bool                           m_bShadowCaster : 1;

	static std::vector<std::pair<ShadowMapFrustum*, const CLightEntity*>>* s_pShadowFrustumsCollector;
};
#endif
