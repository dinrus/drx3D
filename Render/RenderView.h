// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Thread/DrxThreadSafeRendererContainer.h>
#include <drx3D/CoreX/Thread/DrxThreadSafeWorkerContainer.h>
#include <drx3D/CoreX/Math/Range.h>

#include <drx3D/Render/RendererResources.h>

#include <drx3D/Render/RenderPipeline.h>
#include <drx3D/Render/RenderItemDrawer.h>

#include <drx3D/Render/LockFreeAddVector.h>
#include <drx3D/Render/RenderOutput.h>

class CSceneRenderPass;
class CPermanentRenderObject;
struct SGraphicsPipelinePassContext;
class CRenderPolygonDataPool;
class CREClientPoly;
enum EObjectCompilationOptions : u8;

//////////////////////////////////////////////////////////////////////////
// Contain information about view need to render
struct SRenderViewInfo
{
	enum EFlags
	{
		eFlags_ReverseDepth  = BIT(0),
		eFlags_MirrorCull    = BIT(1),
		eFlags_SubpixelShift = BIT(2),
		eFlags_MirrorCamera  = BIT(3),
		eFlags_DrawToTexure  = BIT(4),

		eFlags_None          = 0
	};
	enum EFrustumCorner
	{
		eFrustum_RT = 0, //!< Top Right
		eFrustum_LT = 1, //!< Top Left
		eFrustum_LB = 2, //!< Bottom Left
		eFrustum_RB = 3, //!< Bottom Right
	};

	const CCamera*  pCamera;
	const Plane*    pFrustumPlanes;

	Vec3            cameraOrigin;
	Vec3            cameraVX;
	Vec3            cameraVY;
	Vec3            cameraVZ;

	float           nearClipPlane;
	float           farClipPlane;

	Matrix44A       cameraProjZeroMatrix;
	Matrix44A       cameraProjMatrix;
	Matrix44A       cameraProjNearestMatrix;
	Matrix44A       projMatrix;
	Matrix44A       unjitteredProjMatrix;
	Matrix44A       viewMatrix;
	Matrix44A       invCameraProjMatrix;
	Matrix44A       invViewMatrix;
	Matrix44A       prevCameraMatrix;
	Matrix44A       prevCameraProjMatrix;
	Matrix44A       prevCameraProjNearestMatrix;

	Vec3            m_frustumCorners[4];

	SRenderViewport viewport;
	Vec4            downscaleFactor;

	EFlags          flags;

	SRenderViewInfo();
	void     SetCamera(const CCamera& cam, const CCamera& previousCam, Vec2 subpixelShift, float drawNearestFov, float drawNearestFarPlane);

	Matrix44 GetNearestProjection(float nearestFOV, float farPlane, Vec2 subpixelShift);
	void     ExtractViewMatrices(const CCamera& cam, Matrix44& view, Matrix44& viewZero, Matrix44& invView) const;

	float    WorldToCameraZ(const Vec3& wP) const;
};
DEFINE_ENUM_FLAG_OPERATORS(SRenderViewInfo::EFlags);

// This class encapsulate all information required to render a camera view.
// It stores list of render items added by 3D engine
class CRenderView : public IRenderView
{
public:
	typedef TRange<i32>          ItemsRange;
	typedef _smart_ptr<CTexture> TexSmartPtr;

	enum eShadowFrustumRenderType
	{
		// NOTE: DO NOT reorder, shadow gen depends on values
		eShadowFrustumRenderType_SunCached   = 0,
		eShadowFrustumRenderType_HeightmapAO = 1,
		eShadowFrustumRenderType_SunDynamic  = 2,
		eShadowFrustumRenderType_LocalLight  = 3,
		eShadowFrustumRenderType_Custom      = 4,

		eShadowFrustumRenderType_Count,
		eShadowFrustumRenderType_First = eShadowFrustumRenderType_SunCached
	};

	struct STransparentSegment
	{
		std::vector<TRect_tpl<std::uint16_t>> resolveRects;
		TRange<i32>                           rendItems = { 0 };
	};
	using STransparentSegments = std::vector<STransparentSegment>;

	//typedef CThreadSafeWorkerContainer<SRendItem> RenderItems;
	typedef lockfree_add_vector<SRendItem>       RenderItems;
	typedef std::vector<SShadowFrustumToRender>  ShadowFrustums;
	typedef std::vector<SShadowFrustumToRender*> ShadowFrustumsPtr;
	typedef lockfree_add_vector<CRenderObject*>  ModifiedObjects;

	static i32k MaxFogVolumeNum = 64;

	static i32k CloudBlockerTypeNum = 2;
	static i32k MaxCloudBlockerWorldSpaceNum = 4;
	static i32k MaxCloudBlockerScreenSpaceNum = 1;

public:
	//////////////////////////////////////////////////////////////////////////
	// IRenderView
	//////////////////////////////////////////////////////////////////////////
	virtual void       SetFrameId(i32 frameId) override;
	virtual i32        GetFrameId() const final                        { return m_frameId; };

	virtual void       SetFrameTime(CTimeValue frameTime) final        { m_frameTime = frameTime; };
	virtual CTimeValue GetFrameTime() const final                      { return m_frameTime; };

	virtual void       SetSkipRenderingFlags(u32 nFlags) override   { m_skipRenderingFlags = nFlags; }
	virtual u32     GetSkipRenderingFlags() const final             { return m_skipRenderingFlags; };

	virtual void       SetShaderRenderingFlags(u32 nFlags) override { m_shaderRenderingFlags = nFlags; }
	virtual u32     GetShaderRenderingFlags() const final           { return m_shaderRenderingFlags; };

	virtual void       SetCameras(const CCamera* pCameras, i32 cameraCount) final;
	virtual void       SetPreviousFrameCameras(const CCamera* pCameras, i32 cameraCount) final;

	// Begin/End writing items to the view from 3d engine traversal.
	virtual void                 SwitchUsageMode(EUsageMode mode) override;

	virtual DrxJobState*         GetWriteMutex() override { return &m_jobstate_Write; };

	virtual void                 AddRenderObject(CRenderElement* pRenderElement, SShaderItem& pShaderItem, CRenderObject* pRenderObject, const SRenderingPassInfo& passInfo, i32 list, i32 afterWater) threadsafe final;
	virtual void                 AddPermanentObject(CRenderObject* pObject, const SRenderingPassInfo& passInfo) final;

	virtual void                 SetGlobalFog(const SRenderGlobalFogDescription& fogDescription) final { m_globalFogDescription = fogDescription; };

	virtual void                 SetTargetClearColor(const ColorF& color, bool bEnableClear) override;

	virtual CRenderObject*       AllocateTemporaryRenderObject() final;

	void                         SetSkinningDataPools(const SSkinningDataPoolInfo& skinningData) { m_SkinningData = skinningData;   }
	const SSkinningDataPoolInfo& GetSkinningDataPools() const                                    { return m_SkinningData;           }
	virtual u32               GetSkinningPoolIndex() const final                              { return m_SkinningData.poolIndex; }

	//! HDR and Z Depth render target
	CTexture*            GetColorTarget() const;
	CTexture*            GetDepthTarget() const;

	void                 AssignRenderOutput(CRenderOutputPtr pRenderOutput);
	void                 InspectRenderOutput();
	void                 UnsetRenderOutput();
	const CRenderOutput* GetRenderOutput() const { return m_pRenderOutput.get(); }
	CRenderOutput*       GetRenderOutput()       { return m_pRenderOutput.get(); }

	//! Retrieve rendering viewport for this Render View
	virtual void                   SetViewport(const SRenderViewport& viewport) final;
	virtual const SRenderViewport& GetViewport() const final;

	//! Get resolution of the render target surface(s)
	//! Note that Viewport can be smaller then this.
	Vec2_tpl<uint32_t> GetRenderResolution() const { return { m_RenderWidth, m_RenderHeight }; }
	Vec2_tpl<uint32_t> GetOutputResolution() const  { return m_pRenderOutput ? m_pRenderOutput->GetOutputResolution() : GetRenderResolution(); }
	Vec2_tpl<uint32_t> GetDisplayResolution() const { return m_pRenderOutput ? m_pRenderOutput->GetDisplayResolution() : GetRenderResolution(); }

	void  ChangeRenderResolution(uint32_t  renderWidth, uint32_t  renderHeight, bool bForce);
	//////////////////////////////////////////////////////////////////////////

public:
	CRenderView(tukk name, EViewType type, CRenderView* pParentView = nullptr, ShadowMapFrustum* pShadowFrustumOwner = nullptr);
	~CRenderView();

	EViewType    GetType() const                         { return m_viewType;  }

	void         SetParentView(CRenderView* pParentView) { m_pParentView = pParentView; };
	CRenderView* GetParentView() const                   { return m_pParentView;  };

	//////////////////////////////////////////////////////////////////////////
	// View flags
	//////////////////////////////////////////////////////////////////////////
	void                          SetViewFlags(SRenderViewInfo::EFlags flags)         { m_viewFlags = flags; }
	const SRenderViewInfo::EFlags GetViewFlags() const                                { return m_viewFlags; }
	bool                          IsViewFlag(SRenderViewInfo::EFlags viewFlags) const { return 0 != (m_viewFlags & viewFlags); }

	//////////////////////////////////////////////////////////////////////////
	// Camera access
	const CCamera& GetCamera(CCamera::EEye eye)         const { DRX_ASSERT(eye == CCamera::eEye_Left || eye == CCamera::eEye_Right); return m_camera[eye]; }
	const CCamera& GetPreviousCamera(CCamera::EEye eye) const { DRX_ASSERT(eye == CCamera::eEye_Left || eye == CCamera::eEye_Right); return m_previousCamera[eye]; }

	CCamera::EEye  GetCurrentEye() const;

	RenderItems& GetRenderItems(i32 nRenderList);
	u32       GetBatchFlags(i32 nRenderList) const;

	void         AddRenderItem(CRenderElement* pElem, CRenderObject* RESTRICT_POINTER pObj, const SShaderItem& shaderItem, u32 nList, u32 nBatchFlags, const SRenderingPassInfo& passInfo,
							   SRendItemSorter sorter, bool bShadowPass, bool bForceOpaqueForward) threadsafe;

	bool       CheckPermanentRenderObjects() const { return !m_permanentObjects.empty(); }
	void       AddPermanentObjectImpl(CPermanentRenderObject* pObject, const SRenderingPassInfo& passInfo);

	ItemsRange GetItemsRange(ERenderListID renderList);

	// Find render item index in the sorted list, after when object flag changes.
	i32 FindRenderListSplit(ERenderListID list, u32 objFlag);

	// Find render item index in the sorted list according to arbitrary criteria.
	template<typename T>
	i32        FindRenderListSplit(T predicate, ERenderListID list, i32 first, i32 last);

	void       PrepareForRendering();
	void       PrepareForWriting();

	bool       AllowsHDRRendering() const;
	bool       IsPostProcessingEnabled() const;
	bool       IsRecursive() const        { return m_viewType == eViewType_Recursive; }
	bool       IsShadowGenView() const    { return m_viewType == eViewType_Shadow; }
	bool       IsBillboardGenView() const { return m_viewType == eViewType_BillboardGen; }
	EUsageMode GetUsageMode() const       { return m_usageMode; }
	//////////////////////////////////////////////////////////////////////////
	// Shadows related
	void AddShadowFrustumToRender(const SShadowFrustumToRender& frustum);

	// Get render view used for given frustum.
	CRenderView*                         GetShadowsView(ShadowMapFrustum* pFrustum);

	ItemsRange                           GetShadowItemsRange(ShadowMapFrustum* pFrustum, i32 nFrustumSide);
	RenderItems&                         GetShadowItems(ShadowMapFrustum* pFrustum, i32 nFrustumSide);
	std::vector<SShadowFrustumToRender>& GetFrustumsToRender() { return m_shadows.m_renderFrustums; };

	ShadowFrustumsPtr& GetShadowFrustumsForLight(i32 lightId);
	ShadowFrustumsPtr& GetShadowFrustumsByType(eShadowFrustumRenderType type)             { return m_shadows.m_frustumsByType[type]; }
	const ShadowFrustumsPtr& GetShadowFrustumsByType(eShadowFrustumRenderType type) const { return m_shadows.m_frustumsByType[type]; }

	// Can start executing post write jobs on shadow views
	void                      PostWriteShadowViews();
	void                      PrepareShadowViews(); // Sync all outstanding shadow preparation jobs
	virtual void              SetShadowFrustumOwner(ShadowMapFrustum* pOwner) final { m_shadows.m_pShadowFrustumOwner = pOwner; }
	virtual ShadowMapFrustum* GetShadowFrustumOwner() const final                   { return m_shadows.m_pShadowFrustumOwner; }

	//////////////////////////////////////////////////////////////////////////
	const SRenderGlobalFogDescription& GetGlobalFog() const        { return m_globalFogDescription; };
	bool                               IsGlobalFogEnabled() const  { return m_globalFogDescription.bEnable; }

	ColorF                             GetTargetClearColor() const { return m_targetClearColor; }
	bool                               IsClearTarget()       const { return m_bClearTarget; }

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Lights
	//////////////////////////////////////////////////////////////////////////

	virtual RenderLightIndex AddDeferredLight(const SRenderLight& pDL, float fMult, const SRenderingPassInfo& passInfo) final;

	virtual RenderLightIndex AddDynamicLight(const SRenderLight& light) final;
	virtual RenderLightIndex GetDynamicLightsCount() const final;
	virtual SRenderLight&    GetDynamicLight(RenderLightIndex nLightId) final;

	virtual RenderLightIndex AddLight(eDeferredLightType lightType, const SRenderLight& light) final;
	virtual RenderLightIndex GetLightsCount(eDeferredLightType lightType) const final;
	virtual SRenderLight&    GetLight(eDeferredLightType lightType, RenderLightIndex nLightId) final;

	RenderLightsList&        GetLightsArray(eDeferredLightType lightType);
	SRenderLight*            AddLightAtIndex(eDeferredLightType lightType, const SRenderLight& light, RenderLightIndex index = -1);

	bool                     HaveSunLight() { return GetSunLight() != nullptr; }
	const SRenderLight*      GetSunLight() const;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Clip Volumes
	//////////////////////////////////////////////////////////////////////////
	u8                                   AddClipVolume(const IClipVolume* pClipVolume) final;
	void                                    SetClipVolumeBlendInfo(const IClipVolume* pClipVolume, i32 blendInfoCount, IClipVolume** blendVolumes, Plane* blendPlanes) final;
	const std::vector<SDeferredClipVolume>& GetClipVolumes() const { return m_clipVolumes; }
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Fog Volumes
	//////////////////////////////////////////////////////////////////////////
	void                               AddFogVolume(const CREFogVolume* pFogVolume) final;
	const std::vector<SFogVolumeInfo>& GetFogVolumes(IFogVolumeRenderNode::eFogVolumeType volumeType) const;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Cloud Blockers
	//////////////////////////////////////////////////////////////////////////
	void                              AddCloudBlocker(const Vec3& pos, const Vec3& param, i32 flags) final;
	const std::vector<SCloudBlocker>& GetCloudBlockers(u32 blockerType) const;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Water Ripples
	//////////////////////////////////////////////////////////////////////////
	void                                 AddWaterRipple(const SWaterRippleInfo& waterRippleInfo) final;
	const std::vector<SWaterRippleInfo>& GetWaterRipples() const;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Polygons.
	virtual void            AddPolygon(const SRenderPolygonDescription& poly, const SRenderingPassInfo& passInfo) final;
	CRenderPolygonDataPool* GetPolygonDataPool() { return m_polygonDataPool.get(); };
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	SDeferredDecal*              AddDeferredDecal(const SDeferredDecal& source);
	std::vector<SDeferredDecal>& GetDeferredDecals();
	size_t                       GetDeferredDecalsCount() const       { return m_deferredDecals.size(); }
	void                         SetDeferredNormalDecals(bool bValue) { m_bDeferrredNormalDecals = bValue; };
	bool                         GetDeferredNormalDecals() const      { return m_bDeferrredNormalDecals; }
	//////////////////////////////////////////////////////////////////////////

	const SRenderViewShaderConstants& GetShaderConstants() const;
	SRenderViewShaderConstants&       GetShaderConstants();

	//////////////////////////////////////////////////////////////////////////
	virtual u16 PushFogVolumeContribution(const ColorF& fogVolumeContrib, const SRenderingPassInfo& passInfo) threadsafe final;
	void           GetFogVolumeContribution(u16 idx, ColorF& rColor) const;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Operation on items.
	//////////////////////////////////////////////////////////////////////////
	void                   Clear();

	const SRenderViewInfo& GetViewInfo(CCamera::EEye eye) const { return m_viewInfo[eye]; };
	size_t                 GetViewInfoCount() const             { return m_viewInfoCount; };

	void                   CompileModifiedRenderObjects();
	void                   CalculateViewInfo();

	void                   StartOptimizeTransparentRenderItemsResolvesJob();
	void                   WaitForOptimizeTransparentRenderItemsResolvesJob() const;
	bool                   HasResolveForList(ERenderListID list) const
	{
		const auto refractionMask = FB_REFRACTION | FB_RESOLVE_FULL;
		const auto flags = GetBatchFlags(list);
		return (list == EFSLIST_TRANSP_BW || list == EFSLIST_TRANSP_AW || list == EFSLIST_TRANSP_NEAREST) && 
			!!(flags & refractionMask) && CRendererCVars::CV_r_Refraction;
	}
	const STransparentSegments& GetTransparentSegments(ERenderListID list) const
	{
		DRX_ASSERT_MESSAGE(list == EFSLIST_TRANSP_BW || list == EFSLIST_TRANSP_AW || list == EFSLIST_TRANSP_NEAREST, "'list' does not name a transparent list");
		i32k idx = static_cast<i32>(list - EFSLIST_TRANSP_BW);
		return m_transparentSegments[idx];
	}

private:
	void                   DeleteThis() const override;

	void                   Job_PostWrite();
	void                   Job_SortRenderItemsInList(ERenderListID list);
	void                   SortLights();
	void                   ExpandPermanentRenderObjects();
	void                   UpdateModifiedShaderItems();
	void                   ClearTemporaryCompiledObjects();
	void                   CheckAndScheduleForUpdate(const SShaderItem& shaderItem) threadsafe;
	template<bool bConcurrent>
	void                   AddRenderItemToRenderLists(const SRendItem& ri, i32 nRenderList, CRenderObject* RESTRICT_POINTER pObj, const SShaderItem& shaderItem) threadsafe;

	CCompiledRenderObject* AllocCompiledObject(CRenderObject* pObj, CRenderElement* pElem, const SShaderItem& shaderItem);

	std::size_t            OptimizeTransparentRenderItemsResolves(STransparentSegments &segments, RenderItems &renderItems, std::size_t total_resolve_count) const;
	TRect_tpl<u16>      ComputeResolveViewport(const AABB &aabb, bool forceFullscreenUpdate) const;
	STransparentSegments&  GetTransparentSegments(ERenderListID list)
	{
		DRX_ASSERT_MESSAGE(list == EFSLIST_TRANSP_BW || list == EFSLIST_TRANSP_AW || list == EFSLIST_TRANSP_NEAREST, "'list' does not name a transparent list");
		i32k idx = static_cast<i32>(list - EFSLIST_TRANSP_BW);
		return m_transparentSegments[idx];
	}
	
private:
	EUsageMode       m_usageMode;
	const EViewType  m_viewType;
	string           m_name;
	/// @See SRenderingPassInfo::ESkipRenderingFlags
	u32           m_skipRenderingFlags;
	/// @see EShaderRenderingFlags
	u32           m_shaderRenderingFlags;

	i32              m_frameId;
	CTimeValue       m_frameTime;

	// For shadows or recursive view parent will be main rendering view
	CRenderView*     m_pParentView;

	RenderItems      m_renderItems[EFSLIST_NUM];

	 u32  m_BatchFlags[EFSLIST_NUM];
	// For general passes initialized as a pointers to the m_BatchFlags
	// But for shadow pass it will be a pointer to the shadow frustum side mask
	// u32* m_pFlagsPointer[EFSLIST_NUM];

	// Resolve passes information
	STransparentSegments m_transparentSegments[3];
	mutable DrxJobState  m_optimizeTransparentRenderItemsResolvesJobStatus;

	// Temporary render objects storage
	struct STempObjects
	{
		CThreadSafeWorkerContainer<CRenderObject*> tempObjects;
		CRenderObject*                             pRenderObjectsPool = nullptr;
		u32                                     numObjectsInPool = 0;
		DrxCriticalSection                         accessLock;
	};
	STempObjects m_tempRenderObjects;

	// Light sources affecting the view.
	RenderLightsList m_lights[eDLT_NumLightTypes];

	//////////////////////////////////////////////////////////////////////////
	// Clip Volumes
	std::vector<SDeferredClipVolume> m_clipVolumes;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Fog Volumes
	std::vector<SFogVolumeInfo> m_fogVolumes[IFogVolumeRenderNode::eFogVolumeType_Count];
	lockfree_add_vector<ColorF> m_fogVolumeContributions;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Cloud Blockers
	std::vector<SCloudBlocker> m_cloudBlockers[CloudBlockerTypeNum];
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Water Ripples
	std::vector<SWaterRippleInfo> m_waterRipples;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Simple polygons storage
	std::vector<CREClientPoly*>             m_polygonsPool;
	i32                                     m_numUsedClientPolygons;
	std::shared_ptr<CRenderPolygonDataPool> m_polygonDataPool;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Deferred Decals
	std::vector<SDeferredDecal> m_deferredDecals;
	bool                        m_bDeferrredNormalDecals;

	SRenderGlobalFogDescription m_globalFogDescription;

	ColorF                      m_targetClearColor = {};
	bool                        m_bClearTarget = false;
	bool                        m_bRenderToSwapChain = false;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Skinning Data
	SSkinningDataPoolInfo m_SkinningData;
	//////////////////////////////////////////////////////////////////////////

	u32           m_RenderWidth = -1;
	u32           m_RenderHeight = -1;

	CRenderOutputPtr m_pRenderOutput; // Output render target (currently used for recursive pass and secondary viewport)
	TexSmartPtr      m_pColorTarget = nullptr;
	TexSmartPtr      m_pDepthTarget = nullptr;

	SRenderViewport  m_viewport;

	bool             m_bTrackUncompiledItems;
	bool             m_bAddingClientPolys;

	u32           m_skinningPoolIndex = 0;

	// Render objects modified by this view.
	struct SPermanentRenderObjectCompilationData
	{
		CPermanentRenderObject*    pObject;
		EObjectCompilationOptions  compilationFlags;
	};
	lockfree_add_vector<SPermanentRenderObjectCompilationData> m_permanentRenderObjectsToCompile;

	// Temporary compiled objects for this frame
	struct STemporaryRenderObjectCompilationData
	{
		CCompiledRenderObject* pObject;
		AABB                   localAABB;
	};
	lockfree_add_vector<STemporaryRenderObjectCompilationData> m_temporaryCompiledObjects;

	// shader items that need to be updated
	lockfree_add_vector<std::pair<CShaderResources*, CShader*>> m_shaderItemsToUpdate;

	// Persistent objects added to the view.
	struct SPermanentObjectRecord
	{
		CPermanentRenderObject* pRenderObject;
		u32                  itemSorter;
		i32                     shadowFrustumSide;
		bool                    requiresInstanceDataUpdate;
	};
	lockfree_add_vector<SPermanentObjectRecord> m_permanentObjects;

	//////////////////////////////////////////////////////////////////////////
	// Camera and view information required to render this RenderView
	CCamera         m_camera[CCamera::eEye_eCount];           // Current camera
	CCamera         m_previousCamera[CCamera::eEye_eCount];   // Previous frame render camera

	SRenderViewInfo m_viewInfo[CCamera::eEye_eCount];         // Calculated View Information
	size_t          m_viewInfoCount;                          // Number of current m_viewInfo structures.

	// Internal job states to control when view job processing is done.
	DrxJobState                    m_jobstate_Sort;
	DrxJobState                    m_jobstate_PostWrite;
	DrxJobStateLambda              m_jobstate_Write;
	DrxJobStateLambda              m_jobstate_ShadowGen;

	DrxCriticalSectionNonRecursive m_lock_UsageMode;
	DrxCriticalSectionNonRecursive m_lock_PostWrite;

	std::atomic<bool>              m_bPostWriteExecuted;

	// Constants to pass to shaders.
	SRenderViewShaderConstants m_shaderConstants;

	//@see EFlags
	SRenderViewInfo::EFlags m_viewFlags = SRenderViewInfo::eFlags_None;

	struct SShadows
	{
		using TiledShadingFrustumCoveragePair =    std::pair<SShadowFrustumToRender*, Vec4>;
		using TiledShadingFrustumListByMaskSlice = std::vector<std::vector<TiledShadingFrustumCoveragePair>>;

		// Shadow frustums needed for a view.
		ShadowMapFrustum*                                             m_pShadowFrustumOwner;
		std::vector<SShadowFrustumToRender>                           m_renderFrustums;

		std::map<i32, ShadowFrustumsPtr>                              m_frustumsByLight;
		std::array<ShadowFrustumsPtr, eShadowFrustumRenderType_Count> m_frustumsByType;
		TiledShadingFrustumListByMaskSlice                            m_frustumsPerTiledShadingSlice;

		CThreadSafeRendererContainer<AABB>                            m_nearestCasterBoxes;

		void Clear();
		void AddNearestCaster(CRenderObject* pObj, const SRenderingPassInfo& passInfo);
		void CreateFrustumGroups();
		void PrepareNearestShadows();
		void GenerateSortedFrustumsForTiledShadingByScreenspaceOverlap();
	};

public:// temp
	SShadows m_shadows;

	Vec2     m_vProjMatrixSubPixoffset = Vec2(0.0f, 0.0f);

private:
	CRenderItemDrawer m_RenderItemDrawer;

public:
	void                     DrawCompiledRenderItems(const SGraphicsPipelinePassContext& passContext) const;

	CRenderItemDrawer&       GetDrawer()       { return m_RenderItemDrawer; }
	const CRenderItemDrawer& GetDrawer() const { return m_RenderItemDrawer; }
};

template<typename T>
inline i32 CRenderView::FindRenderListSplit(T predicate, ERenderListID list, i32 first, i32 last)
{
	FUNCTION_PROFILER_RENDERER();

	// Binary search, assumes that the sub-region of the list is sorted by the predicate already
	RenderItems& renderItems = GetRenderItems(list);

	assert(first >= 0 && (first < renderItems.size()));
	assert(last >= 0 && (last <= renderItems.size()));

	--last;
	while (first <= last)
	{
		i32 middle = (first + last) / 2;

		if (predicate(renderItems[middle]))
			first = middle + 1;
		else
			last = middle - 1;
	}

	return last + 1;
}

typedef _smart_ptr<CRenderView> CRenderViewPtr;
