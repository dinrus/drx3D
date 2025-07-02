// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include "IShader.h"

struct SRenderViewport;

// Defines an output target for the Render View
// It could be an offscreen target or a back buffer
struct IRenderViewOutput
{

};

//! Describes a 3D Polygon to be rendered as a 3D Object and added to a Render View
struct SRenderPolygonDescription
{
	SShaderItem            shaderItem;
	i32                    numVertices = 0;
	const SVF_P3F_C4B_T2F* pVertices = nullptr;
	const SPipTangents*    pTangents = nullptr;
	u16*                pIndices = nullptr;
	i32                    numIndices = 0;
	i32                    afterWater = 0;
	i32                    renderListId = 0; // ERenderListID
	CRenderObject*         pRenderObject = nullptr;

	SRenderPolygonDescription() {}
	SRenderPolygonDescription(CRenderObject* pRendObj, SShaderItem& si, i32 numPts, const SVF_P3F_C4B_T2F* verts, const SPipTangents* tangs, u16* inds, i32 ninds, ERenderListID _renderListId, i32 nAW)
		: numVertices(numPts)
		, pVertices(verts)
		, pTangents(tangs)
		, pIndices(inds)
		, numIndices(ninds)
		, renderListId(_renderListId)
		, afterWater(nAW)
		, pRenderObject(pRendObj)
		, shaderItem(si)
	{}
};

//////////////////////////////////////////////////////////////////////
//! Enum for types of deferred lights.
enum eDeferredLightType
{
	eDLT_DeferredLight          = 0,

	eDLT_NumShadowCastingLights = eDLT_DeferredLight + 1,

	//! These lights cannot cast shadows.
	eDLT_DeferredCubemap = eDLT_NumShadowCastingLights,
	eDLT_DeferredAmbientLight,
	eDLT_DynamicLight,                                    // Not deferred light
	eDLT_NumLightTypes,
};

//! Global rendering fog description passed from 3D Engine to Render View
struct SRenderGlobalFogDescription
{
	bool   bEnable = false;
	ColorF color = {};
};

// Interface to the render view.
struct IRenderView : public CMultiThreadRefCount
{
	enum EViewType
	{
		eViewType_Default,
		eViewType_Recursive,
		eViewType_Shadow,
		eViewType_BillboardGen,
		eViewType_Count,
	};

	// View can be in either reading or writing modes.
	enum EUsageMode
	{
		eUsageModeUndefined,
		eUsageModeReading,
		eUsageModeReadingDone,
		eUsageModeWriting,
		eUsageModeWritingDone,
	};

	virtual void SetFrameId(i32 frameId) = 0;
	virtual i32  GetFrameId() const = 0;

	//! Set the start time of the frame
	virtual void       SetFrameTime(CTimeValue time) = 0;
	//! Get the start time of the frame
	virtual CTimeValue GetFrameTime() const = 0;

	/// @See SRenderingPassInfo::ESkipRenderingFlags
	virtual void   SetSkipRenderingFlags(u32 nFlags) = 0;
	virtual u32 GetSkipRenderingFlags() const = 0;

	/// @see EShaderRenderingFlags
	virtual void   SetShaderRenderingFlags(u32 nFlags) = 0;
	virtual u32 GetShaderRenderingFlags() const = 0;

	virtual void   SetCameras(const CCamera* pCameras, i32 cameraCount) = 0;
	virtual void   SetPreviousFrameCameras(const CCamera* pCameras, i32 cameraCount) = 0;

	virtual void   SwitchUsageMode(EUsageMode mode) = 0;

	// All jobs that write items to render view should share and use this synchronization mutex.
	virtual DrxJobState* GetWriteMutex() = 0;

	//! Enable global fog and provide the color when enabled.
	virtual void SetGlobalFog(const SRenderGlobalFogDescription& fogDescription) = 0;

	//! Enable clear of the back buffer to the desired color
	virtual void SetTargetClearColor(const ColorF& color, bool bEnableClear) = 0;

	//! Assign rendering viewport to be used in render view
	virtual void                   SetViewport(const SRenderViewport& viewport) = 0;
	//! Retrieve rendering viewport for this Render View
	virtual const SRenderViewport& GetViewport() const = 0;

	//////////////////////////////////////////////////////////////////////////
	// Access to dynamic lights
	virtual RenderLightIndex AddDeferredLight(const SRenderLight& pDL, float fMult, const SRenderingPassInfo& passInfo) = 0;

	virtual RenderLightIndex AddDynamicLight(const SRenderLight& light) = 0;
	virtual RenderLightIndex GetDynamicLightsCount() const = 0;
	virtual SRenderLight&    GetDynamicLight(RenderLightIndex nLightId) = 0;

	virtual RenderLightIndex AddLight(eDeferredLightType lightType, const SRenderLight& light) = 0;
	virtual RenderLightIndex GetLightsCount(eDeferredLightType lightType) const = 0;
	virtual SRenderLight&    GetLight(eDeferredLightType lightType, RenderLightIndex nLightId) = 0;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Interface for 3d engine
	//////////////////////////////////////////////////////////////////////////
	//! Adds a new render object to the view.
	virtual void AddRenderObject(CRenderElement* pRenderElement, SShaderItem& pShaderItem, CRenderObject* pRenderObject, const SRenderingPassInfo& passInfo, i32 list, i32 afterWater) = 0;

	//! Returns a temporary renderobject allocated only for this RenderView
	virtual CRenderObject* AllocateTemporaryRenderObject() = 0;

	//! Add one the previously created permanent render object.
	virtual void AddPermanentObject(CRenderObject* pRenderObject, const SRenderingPassInfo& passInfo) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Clip Volumes
	virtual u8 AddClipVolume(const IClipVolume* pClipVolume) = 0;
	virtual void  SetClipVolumeBlendInfo(const IClipVolume* pClipVolume, i32 blendInfoCount, IClipVolume** blendVolumes, Plane* blendPlanes) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Fog Volumes
	virtual void AddFogVolume(const class CREFogVolume* pFogVolume) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Cloud blockers
	virtual void AddCloudBlocker(const Vec3& pos, const Vec3& param, i32 flags) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Water ripples
	virtual void AddWaterRipple(const SWaterRippleInfo& waterRippleInfo) = 0;
	virtual void AddPolygon(const SRenderPolygonDescription& poly, const SRenderingPassInfo& passInfo) = 0;

	//! Pushes global Fog volume information
	virtual u16 PushFogVolumeContribution(const ColorF& fogVolumeContrib, const SRenderingPassInfo& passInfo) = 0;

	//! Return current used skinning pool index.
	virtual u32 GetSkinningPoolIndex() const = 0;

	//! Return associated shadow frustum
	virtual ShadowMapFrustum* GetShadowFrustumOwner() const = 0;

	//! Set associated shadow frustum
	virtual void              SetShadowFrustumOwner(ShadowMapFrustum* pOwner) = 0;
};

typedef _smart_ptr<IRenderView> IRenderViewPtr;

//! \endcond