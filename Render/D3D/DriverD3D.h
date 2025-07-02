// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/intrusive_list.hpp>
#include <drx3D/CoreX/Math/Drx_XOptimise.h>
#include <drx3D/Sys/IWindowMessageHandler.h>
#include <drx3D/Input/IHardwareMouse.h>
#include <drx3D/Render/ElementPool.h>
#include <drx3D/Render/RenderDisplayContext.h>

#if defined(_DEBUG)
	#define ENABLE_CONTEXT_THREAD_CHECKING 1
#else
	#define ENABLE_CONTEXT_THREAD_CHECKING 0
#endif

/*
   ===========================================
   The DXRenderer interface Class
   ===========================================
 */

#define VERSION_D3D 3.0

struct SPixFormat;
struct SGraphicsPipelinePassContext;

#include <drx3D/Render/D3DRenderAuxGeom.h>
#include <drx3D/Render/D3DColorGradingController.h>
#include <drx3D/Render/D3DStereo.h>
#include <drx3D/Render/D3DMultiResRendering.h>
#include <drx3D/Render/ShadowTextureGroupUpr.h>    // CShadowTextureGroupUpr

#include <drx3D/Render/D3DDeferredShading.h>
#include <drx3D/Render/PipelineProfiler.h>
#if defined(DX11_ALLOW_D3D_DEBUG_RUNTIME)
	#include <drx3D/Render/D3DDebug.h>
#endif
#include <drx3D/Render/DeviceInfo.h>
#include <memory>
#include <drx3D/Render/RenderView.h>
#include <drx3D/Render/OcclQuery.h>               // COcclusionQuery
#include <drx3D/Render/DeferredRenderUtils.h>     // t_arrDeferredMeshIndBuff
#include <drx3D/Render/Textures/Texture.h>        // CTexture
#include <drx3D/Render/ShadowUtils.h>

#if RENDERER_SUPPORT_SCALEFORM
	#include <drx3D/Render/Scaleform/ScaleformPlayback.h>
	#include <drx3D/Render/Scaleform/ScaleformRender.h>
#endif

#include <drx3D/Sys/FrameProfiler_JobSystem.h>

#include <drx3D/Render/RenderOutput.h>
#include <drx3D/Render/ElementPool.h>

//=====================================================

#define MAXFRAMECAPTURECALLBACK 1

//======================================================================
/// Forward declared classes

struct IStatoscopeDataGroup;

namespace gpu_pfx2 {
class CUpr;
}

#ifdef ENABLE_BENCHMARK_SENSOR
namespace BenchmarkFramework
{
class IBenchmarkRendererSensor;
};
#endif
//======================================================================
/// Direct3D Render driver class

#ifdef SHADER_ASYNC_COMPILATION
class CAsyncShaderTask;
#endif

#if !defined(ENABLE_RENDER_AUX_GEOM)
class CAuxGeomCBCollector;
#endif

enum class EWindowState
{
	//! Normal window with borders
	Windowed,
	//! Rendered in a window without borders, showing desktop behind if the window does not cover the entire screen
	BorderlessWindow,
	//! Rendered in a full screen window without borders, allowing alt-tab to other applications without any delay
	BorderlessFullscreen,
	//! Rendered in exclusive full screen
	Fullscreen,
};

class CD3D9Renderer final : public CRenderer, public IWindowMessageHandler
{
	friend struct SPixFormat;
	friend class CD3DStereoRenderer;
	friend class CTexture;
	friend class CShadowMapStage;
	friend class CSceneRenderPass;
	friend struct IScaleformPlayback;
	friend class CScaleformPlayback;

public:
	struct SCharacterInstanceCB
	{
		CConstantBufferPtr               boneTransformsBuffer;
		// contains only the bone ids and the weighs [0-100] of the bones that move a morph for a particular frame
		CGpuBuffer                       activeMorphsBuffer;
		SSkinningData*                   m_pSD;
		util::list<SCharacterInstanceCB> list;
		bool                             updated;

		SCharacterInstanceCB()
			: boneTransformsBuffer()
			, m_pSD()
			, list()
			, updated()
		{
		}

		~SCharacterInstanceCB() { list.erase(); }
	};

	struct SGammaRamp
	{
		u16 red[256];
		u16 green[256];
		u16 blue[256];
	};

public:
	CD3D9Renderer();
	~CD3D9Renderer();

	//////////////////////////////////////////////////////////////////////////
	// Remove pointer indirection.
	ILINE CD3D9Renderer* operator->()              { return this; }
	ILINE const bool     operator!() const         { return false; }
	ILINE                operator bool() const     { return true; }
	ILINE                operator CD3D9Renderer*() { return this; }
	//////////////////////////////////////////////////////////////////////////

	virtual void           InitRenderer() override;
	virtual void           Release() override;

	//	static u32 GetNumBackBufferIndices(const DXGI_SWAP_CHAIN_DESC& scDesc);
	//	static u32 GetCurrentBackBufferIndex(SDisplayContext* pContext);

	virtual void     LockParticleVideoMemory(i32 frameId) override;
	virtual void     UnLockParticleVideoMemory(i32 frameId) override;
	virtual void     ActivateLayer(tukk pLayerName, bool activate) override;

	void             SetDefaultTexParams(bool bUseMips, bool bRepeat, bool bLoad);

public:
#ifdef USE_PIX_DURANGO
	ILINE ID3DUserDefinedAnnotation* GetPixProfiler() { return m_pPixPerf; }
#endif

	/////////////////////////////////////////////////////////////////////////////
	// Functions to access the device and associated objects
	const CDrxDeviceWrapper&        GetDevice() const;
	CDrxDeviceWrapper&              GetDevice();
	ILINE CDrxDeviceWrapper&        GetDevice_Unsynchronized()        { return m_DeviceWrapper; }
	CDrxDeviceContextWrapper&       GetDeviceContext();
	ILINE CDrxDeviceContextWrapper& GetDeviceContext_Unsynchronized() { return m_DeviceContextWrapper; }
	ILINE CDrxDeviceContextWrapper& GetDeviceContext_ForMapAndUnmap()
	{
#if (DRX_RENDERER_DIRECT3D >= 120)
		// "ID3D12Resource::Map": Map and Unmap can be called by multiple threads safely.
		return GetDeviceContext_Unsynchronized();
#else
		return GetDeviceContext();
#endif
	}
#if defined(DEVICE_SUPPORTS_PERFORMANCE_DEVICE)
	CDrxPerformanceDeviceWrapper&              GetPerformanceDevice();
	ILINE CDrxPerformanceDeviceWrapper&        GetPerformanceDevice_Unsynchronized()        { return m_PerformanceDeviceWrapper; }
	CDrxPerformanceDeviceContextWrapper&       GetPerformanceDeviceContext();
	ILINE CDrxPerformanceDeviceContextWrapper& GetPerformanceDeviceContext_Unsynchronized() { return m_PerformanceDeviceContextWrapper; }
#endif

	/////////////////////////////////////////////////////////////////////////////
	// Debug Functions to check that the D3D DeviceContext is only accessed
	// by its owning thread
	void  BindContextToThread(DWORD threadID);
	DWORD GetBoundThreadID() const;
	void  CheckContextThreadAccess() const;

	/////////////////////////////////////////////////////////////////////////////
	// Util function to support synchronization if a asynchrounous device is used
	void          WaitForAsynchronousDevice() const;
	 i32* GetAsynchronousDeviceState();

	/////////////////////////////////////////////////////////////////////////////
	// Util function to bind Device Hooks to all devices/contextes
	void                             RegisterDeviceWrapperHook(IDrxDeviceWrapperHook* pDeviceWrapperHook);
	void                             UnregisterDeviceWrapperHook(tukk pDeviceHookName);

	void                             EF_PrepareShadowGenRenderList(const SRenderingPassInfo& passInfo);
	bool                             EF_PrepareShadowGenForLight(CRenderView* pRenderView, SRenderLight* pLight, i32 nLightID);
	bool                             PrepareShadowGenForFrustum(CRenderView* pRenderView, ShadowMapFrustum* pCurFrustum, const SRenderLight* pLight, i32 nLightID, i32 nLOD = 0);

	virtual void                     EF_InvokeShadowMapRenderJobs(const SRenderingPassInfo& passInfo, i32k nFlags) override;
	void                             InvokeShadowMapRenderJobs(ShadowMapFrustum* pCurFrustum, SRenderingPassInfo passInfo);
	void                             StartInvokeShadowMapRenderJobs(ShadowMapFrustum* pCurFrustum, const SRenderingPassInfo& passInfo);

	void                             GetReprojectionMatrix(Matrix44A& matReproj, const Matrix44A& matView, const Matrix44A& matProj, const Matrix44A& matPrevView, const Matrix44A& matPrevProj, float fFarPlane) const;

	void                             CreateDeferredUnitBox(t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);
	const t_arrDeferredMeshIndBuff&  GetDeferredUnitBoxIndexBuffer() const;
	const t_arrDeferredMeshVertBuff& GetDeferredUnitBoxVertexBuffer() const;
	CShadowUtils::SShadowsSetupInfo  ConfigShadowTexgen(CRenderView* pRenderView, const ShadowMapFrustum* pFr, i32 nFrustNum = -1, bool bScreenToLocalBasis = false);
	void                             DrawAllDynTextures(tukk szFilter, const bool bLogNames, const bool bOnlyIfUsedThisFrame);

	void                             GetDeviceGamma();
	void                             SetDeviceGamma(SGammaRamp*);

	HRESULT                          AdjustWindowForChange(i32k displayWidth, i32k displayHeight, EWindowState previousWindowState);

	bool                             IsFullscreen()           { return m_windowState == EWindowState::Fullscreen; }

#if defined(SUPPORT_DEVICE_INFO)
	static HWND CreateWindowCallback();
	DeviceInfo& DevInfo() { return m_devInfo; }
#endif

private:
	void                    DebugShowRenderTarget();

	bool                    CreateDeviceDurango();
	bool                    CreateDeviceOrbis();
	bool                    CreateDeviceMobile();
	bool                    CreateDeviceDesktop();
	bool                    CreateDevice();

	bool                    SetWindow(i32 width, i32 height);
	void                    UnSetRes();
	void                    DisplaySplash(); //!< Load a bitmap from a file, blit it to the windowdc and free it

	void                    DestroyWindow(void);
	virtual void            RestoreGamma(void) override;
	void                    SetGamma(float fGamma, float fBrigtness, float fContrast, bool bForce);

	i32                     FindTextureInRegistry(tukk filename, i32* tex_type);
	i32                     RegisterTextureInRegistry(tukk filename, i32 tex_type, i32 tid, i32 low_tid);
	u32            MakeTextureREAL(tukk filename, i32* tex_type, u32 load_low_res);
	u32            CheckTexturePlus(tukk filename, tukk postfix);

	static HRESULT CALLBACK OnD3D11CreateDevice(D3DDevice* pd3dDevice);

	void                    PostDeviceReset();

public:
	static HRESULT CALLBACK OnD3D11PostCreateDevice(D3DDevice* pd3dDevice);

public:
	// Multithreading support
	virtual void RT_BeginFrame(const SDisplayContextKey& displayContextKey) override;
	virtual void RT_EndFrame() override;

	virtual void RT_Init() override;
	virtual bool RT_CreateDevice() override;
	virtual void RT_Reset() override;
	virtual void RT_PreRenderScene(CRenderView* pRenderView);
	virtual void RT_RenderScene(CRenderView* pRenderView) override;
	virtual void RT_PostRenderScene(CRenderView* pRenderView);

	virtual void RT_ReleaseRenderResources(u32 nFlags) override;

	virtual void RT_CreateRenderResources() override;
	virtual void RT_PrecacheDefaultShaders() override;
	virtual bool RT_ReadTexture(uk pDst, i32 destinationWidth, i32 destinationHeight, EReadTextureFormat dstFormat, CTexture* pSrc) override;
	virtual bool RT_StoreTextureToFile(tukk szFilePath, CTexture* pSrc) override;
	
	virtual void RT_RenderDebug(bool bRenderStats = true) override;

	virtual void RT_PresentFast() override;

	//===============================================================================

	virtual void RT_FlashRenderInternal(std::shared_ptr<IFlashPlayer> &&pPlayer) override;
	virtual void RT_FlashRenderInternal(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, bool bDoRealRender) override;
	virtual void RT_FlashRenderPlaybackLocklessInternal(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, i32 cbIdx, bool bFinalPlayback, bool bDoRealRender) override;

	//===============================================================================

	virtual WIN_HWND Init(i32 x, i32 y, i32 width, i32 height, u32 cbpp, i32 zbpp, i32 sbits, WIN_HWND Glhwnd = 0, bool bReInit = false, bool bShaderCacheGen = false) override;

	virtual void     GetVideoMemoryUsageStats(size_t& vidMemUsedThisFrame, size_t& vidMemUsedRecently, bool bGetPoolsSizes = false) override;

	/////////////////////////////////////////////////////////////////////////////////
	// Render-context management
	/////////////////////////////////////////////////////////////////////////////////
	CRenderDisplayContext*                GetActiveDisplayContext() const;
	CSwapChainBackedRenderDisplayContext* GetBaseDisplayContext() const;
	CRenderDisplayContext*                FindDisplayContext(const SDisplayContextKey& key) threadsafe const;
	
	// Returns a pair with a success flag, and the previous active context key.
	// In case of failure the key is the current active context key.
	std::pair<bool, SDisplayContextKey>   SetCurrentContext(const SDisplayContextKey& key) threadsafe;
	virtual WIN_HWND                      GetCurrentContextHWND() override;

	uint32_t                   GenerateUniqueContextId() { return m_uniqueDisplayContextId++; }
	SDisplayContextKey         AddCustomContext(const CRenderDisplayContextPtr &context) threadsafe;
	virtual SDisplayContextKey CreateSwapChainBackedContext(const SDisplayContextDescription& desc) threadsafe final;
	void                       ResizeContext(CRenderDisplayContext *, i32 width, i32 height) threadsafe;
	virtual void               ResizeContext(const SDisplayContextKey& key, i32 width, i32 height) threadsafe final;
	virtual bool               DeleteContext(const SDisplayContextKey& key) threadsafe final;

#ifdef DRX_PLATFORM_WINDOWS
	virtual RectI    GetDefaultContextWindowCoordinates() final;
#endif
	bool             IsCurrentContextMainVP();
	/////////////////////////////////////////////////////////////////////////////////

	//! Changes resolution of the window/device (doesn't require to reload the level)
	bool         ChangeRenderResolution(i32 nNewRenderWidth, i32 nNewRenderHeight, CRenderView* pRenderView);
	bool         ChangeOutputResolution(i32 nNewOutputWidth, i32 nNewOutputHeight, CRenderOutput* pRenderOutput);
	bool         ChangeDisplayResolution(i32 nNewDisplayWidth, i32 nNewHDisplayeight, i32 nNewColDepth, i32 nNewRefreshHZ, EWindowState previousWindowState, bool bForceReset, CRenderDisplayContext* displayContext);
	bool         ChangeDisplayResolution(i32 nNewDisplayWidth, i32 nNewHDisplayeight, i32 nNewColDepth, i32 nNewRefreshHZ, EWindowState previousWindowState, bool bForceReset, const SDisplayContextKey& displayContextKey);

	void         CalculateResolutions(i32 displayWidthRequested, i32 displayHeightRequested, bool bUseNativeRes, bool findClosestMatching, i32* pRenderWidth, i32* pRenderHeight, i32* pOutputWidth, i32* pOutputHeight, i32* pDisplayWidth, i32* pDisplayHeight);

	bool         ChangeDisplay(CRenderDisplayContext* pDC, u32 width, u32 height, u32 cbpp);
	void         ChangeViewport(CRenderDisplayContext* pDC, u32 viewPortOffsetX, u32 viewPortOffsetY, u32 viewportWidth, u32 viewportHeight);

	virtual i32  EnumDisplayFormats(SDispFormat* formats) override;

	virtual bool IsTextureFormatSupported(ETEX_Format eTF) final { return CRendererResources::s_hwTexFormatSupport.IsFormatSupported(eTF); }

	virtual void Reset(void) override;

	virtual void BeginFrame(const SDisplayContextKey& displayContextKey) override;
	virtual void FillFrame(ColorF clearColor) override;
	virtual void ShutDown(bool bReInit = false) override;
	virtual void ShutDownFast() override;
	virtual void RenderDebug(bool bRenderStats = true) override;
	virtual void RT_ShutDown(u32 nFlags) override;

	//////////////////////////////////////////////////////////////////////////
	// Debug functions
	//////////////////////////////////////////////////////////////////////////
	void                 DebugPrintShader(class CHWShader_D3D* pSH, uk pInst, i32 nX, i32 nY, ColorF colSH);
	void                 DebugPerfBars(const SRenderStatistics &RStats,i32 nX, i32 nY);
	void                 DebugVidResourcesBars(i32 nX, i32 nY);
	void                 DebugDrawStats1(const SRenderStatistics& RStats);
	void                 DebugDrawStats2(const SRenderStatistics& RStats);
	void                 DebugDrawStats8(const SRenderStatistics& RStats);
	void                 DebugDrawStats20(const SRenderStatistics& RStats);
	void                 DebugDrawStats(const SRenderStatistics& RStats);
	void                 VidMemLog();
	//////////////////////////////////////////////////////////////////////////

	void                 RenderAux();
	void                 RenderAux_RT();

	virtual void         EndFrame() override;
	virtual void         LimitFramerate(i32k maxFPS, const bool bUseSleep) override;
	virtual void         GetMemoryUsage(IDrxSizer* Sizer) override;
	virtual void         GetLogVBuffers() override;

	virtual void         TryFlush() override;

	virtual i32          GetDetailedRayHitInfo(IPhysicalEntity* pCollider, const Vec3& vOrigin, const Vec3& vDirection, const float maxRayDist, float* pUOut, float* pVOut) override;
	virtual Vec3         UnprojectFromScreen(i32 x, i32 y) override;

	virtual void         PushProfileMarker(tukk label) override;
	virtual void         PopProfileMarker(tukk label) override;

	u32         UploadToVideoMemory(u8* data, i32 w, i32 h, i32 d, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, ETEX_Type eTT, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false);
	virtual u32 UploadToVideoMemory(u8* data, i32 w, i32 h, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) final;
	virtual u32 UploadToVideoMemoryCube(u8* data, i32 w, i32 h, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) final;
	virtual u32 UploadToVideoMemory3D(u8* data, i32 w, i32 h, i32 d, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) final;
	virtual void         UpdateTextureInVideoMemory(u32 tnum, u8* newdata, i32 posx, i32 posy, i32 w, i32 h, ETEX_Format eTF = eTF_R8G8B8A8, i32 posz = 0, i32 sizez = 1) override;
	virtual bool         EF_PrecacheResource(SShaderItem* pSI, i32 iScreenTexels, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter) override;
	virtual bool         EF_PrecacheResource(SShaderItem* pSI, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter) override;
	virtual bool         EF_PrecacheResource(ITexture* pTP, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter) override;
	virtual void         RemoveTexture(u32 TextureId) override;

	virtual void         PostLevelUnload() override;

	virtual void         Graph(byte* g, i32 x, i32 y, i32 wdt, i32 hgt, i32 nC, i32 type, tukk text, ColorF& color, float fScale) override;

	virtual void         PrintResourcesLeaks() override;

	virtual void         EnableVSync(bool enable) override;

	virtual bool         ProjectToScreen(float ptx, float pty, float ptz, float* sx, float* sy, float* sz) override;
	virtual i32          UnProject(float sx, float sy, float sz, float* px, float* py, float* pz, const float modelMatrix[16], const float projMatrix[16], i32k viewport[4]) override;
	virtual i32          UnProjectFromScreen(float sx, float sy, float sz, float* px, float* py, float* pz) override;

	virtual void SetRendererCVar(ICVar* pCVar, tukk pArgText, const bool bSilentMode = false) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Continuous frame capture interface
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	// This routines uses 2 destination surfaces.  It triggers a backbuffer copy to one of its surfaces,
	// and then copies the other surface to system memory.  This hopefully will remove any
	// CPU stalls due to the rect lock call since the buffer will already be in system
	// memory when it is called
	// Inputs :
	//			pDstARGBA8			:	Pointer to a buffer that will hold the captured frame (should be at least 4*dstWidth*dstHieght for RGBA surface)
	//			destinationWidth	:	Width of the frame to copy
	//			destinationHeight	:	Height of the frame to copy
	//
	//			Note :	If dstWidth or dstHeight is larger than the current surface dimensions, the dimensions
	//					of the surface are used for the copy
	//
	virtual bool CaptureFrameBufferFast(u8* pDstRGBA8, i32 destinationWidth, i32 destinationHeight) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Copy a captured surface to a buffer
	//
	// Inputs :
	//			pDstARGBA8			:	Pointer to a buffer that will hold the captured frame (should be at least 4*dstWidth*dstHieght for RGBA surface)
	//			destinationWidth	:	Width of the frame to copy
	//			destinationHeight	:	Height of the frame to copy
	//
	//			Note :	If dstWidth or dstHeight is larger than the current surface dimensions, the dimensions
	//					of the surface are used for the copy
	//
	virtual bool CopyFrameBufferFast(u8* pDstRGBA8, i32 destinationWidth, i32 destinationHeight) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// This routine registers a callback address that is called when a new frame is available
	// Inputs :
	//			pCapture			:	Address of the ICaptureFrameListener object
	//
	// Outputs : returns true if successful, otherwise false
	//
	virtual bool RegisterCaptureFrame(ICaptureFrameListener* pCapture) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// This routine unregisters a callback address that was previously registered
	// Inputs :
	//			pCapture			:	Address of the ICaptureFrameListener object to unregister
	//
	// Outputs : returns true if successful, otherwise false
	//
	virtual bool UnRegisterCaptureFrame(ICaptureFrameListener* pCapture) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// This routine initializes 2 destination surfaces for use by the CaptureFrameBufferFast routine
	// It also, captures the current backbuffer into one of the created surfaces
	//
	// Inputs :
	//			bufferWidth	: Width of capture buffer, on consoles the scaling is done on the GPU. Pass in 0 (the default) to use backbuffer dimensions
	//			bufferHeight	: Height of capture buffer.
	//
	// Outputs : returns true if surfaces were created otherwise returns false
	//
	virtual bool InitCaptureFrameBufferFast(u32 bufferWidth, u32 bufferHeight) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// This routine releases the 2 surfaces used for frame capture by the CaptureFrameBufferFast routine
	//
	// Inputs : None
	//
	// Returns : None
	//
	virtual void CloseCaptureFrameBufferFast(void) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// This routine checks for any frame buffer callbacks that are needed and calls them
	//
	// Inputs : None
	//
	//	Outputs : None
	//
	virtual void CaptureFrameBufferCallBack(void) override;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// This routine checks for any frame buffer callbacks that are needed and checks their flags, calling preparation functions if required
	//
	// Inputs : None
	//
	//	Outputs : None
	//
	void CaptureFrameBufferPrepare(void);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Single frame capture interface
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	bool ScreenShot(tukk filename = NULL);
	bool ScreenShot(tukk filename, CRenderDisplayContext *pDC);
	virtual bool ScreenShot(tukk filename = NULL, const SDisplayContextKey& displayContextKey = {}) override;
	void         CaptureFrameBuffer();
	virtual bool ReadFrameBuffer(u32* pDstRGBA8, i32 destinationWidth, i32 destinationHeight, bool readPresentedBackBuffer = true, EReadTextureFormat format = EReadTextureFormat::RGB8) override;

	//misc
	void         UnloadOldTextures() {};

	virtual void Set2DMode(bool enable, i32 ortox, i32 ortoy, float znear = -1e10f, float zfar = 1e10f) override;

	virtual bool SetGammaDelta(const float fGamma) override;

	//////////////////////////////////////////////////////////////////////
	// Replacement functions for the Font engine
	virtual bool FontUploadTexture(class CFBitmap*, ETEX_Format eTF = eTF_R8G8B8A8) override;
	virtual i32  FontCreateTexture(i32 Width, i32 Height, byte* pData, ETEX_Format eTF = eTF_R8G8B8A8, bool genMips = false) override;
	virtual bool FontUpdateTexture(i32 nTexId, i32 X, i32 Y, i32 USize, i32 VSize, byte* pData) override;
	virtual void FontReleaseTexture(class CFBitmap* pBmp) override;

#if RENDERER_SUPPORT_SCALEFORM
	void                     SF_CreateResources();
	void                     SF_PrecacheShaders();
	void                     SF_DestroyResources();
	void                     SF_ResetResources();
	struct SSF_ResourcesD3D& SF_GetResources();

	void                     SF_HandleClear(const SSF_GlobalDrawParams& __restrict params);

	void                     SF_DrawIndexedTriList(i32 baseVertexIndex, i32 minVertexIndex, i32 numVertices, i32 startIndex, i32 triangleCount, const SSF_GlobalDrawParams& __restrict params);
	void                     SF_DrawLineStrip(i32 baseVertexIndex, i32 lineCount, const SSF_GlobalDrawParams& __restrict params);
	void                     SF_DrawGlyphClear(const IScaleformPlayback::DeviceData* vtxData, i32 baseVertexIndex, const SSF_GlobalDrawParams& __restrict params);
	void                     SF_DrawBlurRect(const IScaleformPlayback::DeviceData* vtxData, const SSF_GlobalDrawParams& __restrict params);

	virtual bool             SF_UpdateTexture(i32 texId, i32 mipLevel, i32 numRects, const SUpdateRect* pRects, u8k* pData, size_t pitch, size_t size, ETEX_Format eTF) override;
	virtual bool             SF_ClearTexture(i32 texId, i32 mipLevel, i32 numRects, const SUpdateRect* pRects, u8k* pData) override;
#else // #if RENDERER_SUPPORT_SCALEFORM
	// These dummy functions are required when the feature is disabled, do not remove without testing the RENDERER_SUPPORT_SCALEFORM=0 case!
	virtual bool SF_UpdateTexture(i32 texId, i32 mipLevel, i32 numRects, const SUpdateRect* pRects, u8k* pData, size_t pitch, size_t size, ETEX_Format eTF) override { return false; }
	virtual bool SF_ClearTexture(i32 texId, i32 mipLevel, i32 numRects, const SUpdateRect* pRects, u8k* pData) override                                              { return false; }
#endif // #if RENDERER_SUPPORT_SCALEFORM

	virtual void SetProfileMarker(tukk label, ESPM mode) const override;
	
	//! Render a frame from the RenderView
	//! nSceneRenderingFlags @see EShaderRenderingFlags
	void RenderFrame(i32 nSceneRenderingFlags, const SRenderingPassInfo& passInfo);

	bool CheckSSAAChange();

	bool FX_DrawToRenderTarget(CShader* pShader, CShaderResources* pRes, CRenderObject* pObj, SShaderTechnique* pTech, SHRenderTarget* pTarg, i32 nPreprType, CRenderElement* pRE,const SRenderingPassInfo& passInfo);

#if DRX_PLATFORM_ORBIS
	bool BakeMesh(const SMeshBakingInputParams* pInputParams, SMeshBakingOutput* pReturnValues) override { return false; }
#else
	bool BakeMesh(const SMeshBakingInputParams* pInputParams, SMeshBakingOutput* pReturnValues) override;
#endif

	virtual float* PinOcclusionBuffer(Matrix44A& camera) override;
	virtual void   UnpinOcclusionBuffer() override;

	virtual void   WaitForParticleBuffer(i32 frameId) override;
	void           InsertParticleVideoDataFence(i32 frameId);

	void           RT_UpdateSkinningConstantBuffers(CRenderView* pRenderView);

	virtual uk  FX_AllocateCharInstCB(SSkinningData*, u32) override;
	virtual void   FX_ClearCharInstCB(u32) override;

	bool CreateAuxiliaryMeshes();
	bool ReleaseAuxiliaryMeshes();

	bool CreateUnitVolumeMesh(t_arrDeferredMeshIndBuff& arrDeferredInds, t_arrDeferredMeshVertBuff& arrDeferredVerts, D3DIndexBuffer*& pUnitFrustumIB, D3DVertexBuffer*& pUnitFrustumVB);

	void FX_DeferredShadowsNearFrustum(i32 maskRTWidth, i32 maskRTHeight);
	void FX_SetDeferredShadows();

	void PrepareShadowPool(CRenderView* pRenderView) const override final;

	bool FX_HDRScene(CRenderView *pRenderView, bool bClear = true);

	// Performance queries
	//=======================================================================

	virtual float GetGPUFrameTime() override;
	virtual void  GetRenderTimes(SRenderTimes& outTimes) override;

	// Shaders pipeline
	//=======================================================================
	void         ChangeLog();

	virtual void SetCurDownscaleFactor(Vec2 sf) final;

	void         SetLogFuncs(bool bEnable);
	void         MemReplayWrapD3DDevice();

	void         DrawTexelsPerMeterInfo();
	virtual bool CheckDeviceLost() override;

#if DRX_PLATFORM_DURANGO
	virtual void RT_SuspendDevice() override;
	virtual void RT_ResumeDevice() override;
#endif

	//========================================================================================

	void EF_Init();
	void EF_Exit(bool bFastShutdown = false);

	void RT_GraphicsPipelineBootup();
	void RT_GraphicsPipelineShutdown();

private:
	//friend class CStandardGraphicsPipeline;

	bool RT_ScreenShot(tukk filename, CRenderDisplayContext*);

public:
	bool        ShouldTrackStats();

	i32  EF_Preprocess(SRendItem* ri, u32 nums, u32 nume, const SRenderingPassInfo& passInfo);

	// This method takes CRenderView prepared by 3D engine after it fully finished,and send it to the Renderer for drawing.
	void             SubmitRenderViewForRendering(i32 nFlags, const SRenderingPassInfo& passInfo);

	virtual void     EF_EndEf3D(i32k nFlags, i32k nPrecacheUpdateId, i32k nNearPrecacheUpdateId, const SRenderingPassInfo& passInfo) override;
	virtual void     EF_EndEf2D(const bool bSort) override; // 2d only

	virtual WIN_HWND GetHWND() override { return m_hWnd; }
	virtual bool     SetWindowIcon(tukk path) override;
	static void      SetWindowIconCVar(ICVar* pVar);

	static void      SetMouseCursorIconCVar(ICVar* pVar);

	virtual bool     StoreGBufferToAtlas(const RectI& rcDst, i32 nSrcWidth, i32 nSrcHeight, i32 nDstWidth, i32 nDstHeight, ITexture* pDataD, ITexture* pDataN) override;

	//////////////////////////////////////////////////////////////////////////
public:
	bool IsDeviceLost() { return(m_bDeviceLost != 0); }
	void InitNVAPI();
	void InitAMDAPI();

#if defined(FEATURE_SVO_GI)
	virtual ISvoRenderer* GetISvoRenderer() override;
#endif
	
	IRenderAuxGeom*      GetIRenderAuxGeom() override;
	IRenderAuxGeom*      GetOrCreateIRenderAuxGeom(const CCamera* pCustomCamera = nullptr) override;
	void                 UpdateAuxDefaultCamera(const CCamera& systemCamera) override;
	void                 DeleteAuxGeom(IRenderAuxGeom* pRenderAuxGeom) override;
	void                 SubmitAuxGeom(IRenderAuxGeom* pRenderAuxGeom, bool merge = true) override;
	void                 DeleteAuxGeomCBs();
	void                 SetCurrentAuxGeomCollector(CAuxGeomCBCollector* auxGeomCollector);
	
	CAuxGeomCBCollector* GetOrCreateAuxGeomCollector(const CCamera &defaultCamera) threadsafe;
	void ReturnAuxGeomCollector(CAuxGeomCBCollector* auxGeomCollector) threadsafe;
	void DeleteAuxGeomCollectors();
	

private:
#if defined(ENABLE_RENDER_AUX_GEOM)
	CAuxGeomCBCollector* m_currentAuxGeomCBCollector;

	// Aux Geometry Collector Pool
	SElementPool<CAuxGeomCBCollector> m_auxGeometryCollectorPool{
		[] { return new CAuxGeomCBCollector; }, 
		[](CAuxGeomCBCollector* pAuxGeomCBCollector) { pAuxGeomCBCollector->FreeMemory(); }
	};

	// Aux Geometry Command Buffer Pool
	SElementPool<CAuxGeomCB> m_auxGeomCBPool{
		[] { return new CAuxGeomCB; }, 
		[](CAuxGeomCB* pAuxGeomCB) { pAuxGeomCB->FreeMemory(); }
	};

	CAuxGeomCB	m_renderThreadAuxGeom;	
#endif

public:
	
	virtual IColorGradingController* GetIColorGradingController() override;

	virtual IStereoRenderer*         GetIStereoRenderer() const override;

	virtual void                     StartLoadtimeFlashPlayback(ILoadtimeCallback* pCallback) override;
	virtual void                     StopLoadtimeFlashPlayback() override;

	CD3DStereoRenderer&            GetS3DRend() const    { return *m_pStereoRenderer; }
	virtual bool                   IsStereoEnabled() const override;

	virtual const RPProfilerStats*                   GetRPPStats(ERenderPipelineProfilerStats eStat, bool bCalledFromMainThread = true) override;
	virtual const RPProfilerStats*                   GetRPPStatsArray(bool bCalledFromMainThread = true) override;
	virtual const DynArray<RPProfilerDetailedStats>* GetRPPDetailedStatsArray(bool bCalledFromMainThread = true) override;

	virtual i32                    GetPolygonCountByType(u32 EFSList, EVertexCostTypes vct, u32 z, bool bCalledFromMainThread = true) override;

	virtual void                   LogShaderImportMiss(const CShader* pShader) override;

	void                           BeginRenderDocCapture();
	void                           EndRenderDocCapture();

#ifdef SUPPORT_HW_MOUSE_CURSOR
	virtual IHWMouseCursor* GetIHWMouseCursor() override;
#endif

	virtual IGraphicsDeviceConstantBufferPtr           CreateGraphiceDeviceConstantBuffer() final;

	virtual gpu_pfx2::IUpr*                        GetGpuParticleUpr() override;
	virtual compute_skinning::IComputeSkinningStorage* GetComputeSkinningStorage() override;

private:
	void                                               HandleDisplayPropertyChanges();
	EWindowState                                       CalculateWindowState() const;
	tukk                                        GetWindowStateName() const;

	void                                               ResolveSupersampledRendering();
	void                                               ResolveSubsampledOutput();
	void                                               ResolveHighDynamicRangeDisplay();
	virtual void                                       EnablePipelineProfiler(bool bEnable) override;

	void                                               UpdateActiveContext(const std::shared_ptr<CRenderDisplayContext> &ctx, const SDisplayContextKey &key);

	// Called before starting drawing new frame
	virtual void ClearPerFrameData(const SRenderingPassInfo& passInfo) override;
#if DRX_PLATFORM_WINDOWS
public:
	// Called to inspect window messages sent to this renderer's windows
	virtual bool HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
#endif

public:
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC MEMBERS
	//////////////////////////////////////////////////////////////////////////

#if !defined(_RELEASE) || defined(ENABLE_STATOSCOPE_RELEASE)
	// sprite cells referenced this frame (particular sprite in particular atlas)
	std::set<u32> m_SpriteCellsUsed;
	// sprite atlases referenced this frame
	std::set<CTexture*> m_SpriteAtlasesUsed;
#endif

#if ENABLE_STATOSCOPE
	IStatoscopeDataGroup * m_pGPUTimesDG;
	IStatoscopeDataGroup* m_pDetailedRenderTimesDG;
	IStatoscopeDataGroup* m_pGraphicsDG;
	IStatoscopeDataGroup* m_pPerformanceOverviewDG;
#endif

#if defined(DX11_ALLOW_D3D_DEBUG_RUNTIME)
	CD3DDebug m_d3dDebug;
	bool m_bUpdateD3DDebug;
#endif

	DWORD m_DeviceOwningthreadID;           // thread if of thread who is allows to access the D3D Context
	 i32       m_nAsyncDeviceState; // counter of how many jobs are currently executed in parallel on the device

#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	ID3DXboxPerformanceDevice*  m_pPerformanceDevice;
	ID3DXboxPerformanceContext* m_pPerformanceDeviceContext;
#endif

	DXGI_SURFACE_DESC        m_d3dsdBackBuffer; // Surface desc of the BackBuffer

#ifdef USE_PIX_DURANGO
	ID3DUserDefinedAnnotation* m_pPixPerf;
#endif

	// x = average luminance, y = max luminance, z = min luminance,
	Vec4               m_vSceneLuminanceInfo;

	float              m_fScotopicSceneScale;
	float              m_fAdaptedSceneScale;
	float              m_fAdaptedSceneScaleLBuffer;

	i32                m_MaxAnisotropyLevel;
	SamplerStateHandle m_nMaterialAnisoHighSampler;
	SamplerStateHandle m_nMaterialAnisoLowSampler;
	SamplerStateHandle m_nMaterialAnisoSamplerBorder;


	//////////////////////////////////////////////////////////////////////////
	enum { kUnitObjectIndexSizeof = 2 };

	D3DVertexBuffer* m_pUnitFrustumVB[SHAPE_MAX];
	D3DIndexBuffer*  m_pUnitFrustumIB[SHAPE_MAX];

	i32              m_UnitFrustVBSize[SHAPE_MAX];
	i32              m_UnitFrustIBSize[SHAPE_MAX];

	D3DVertexBuffer* m_pQuadVB;
	i16            m_nQuadVBSize;

	//////////////////////////////////////////////////////////////////////////
	byte                        m_GammmaTable[256];

	i32                         m_fontBlendMode;

	CDrxNameTSCRC               m_LevelShaderCacheMissIcon;

	CColorGradingController*    m_pColorGradingControllerD3D;

	CRenderPipelineProfiler*    m_pPipelineProfiler;

#ifdef SHADER_ASYNC_COMPILATION
	std::vector<CAsyncShaderTask*> m_AsyncShaderTasks;
#endif

	Matrix44                 m_matPsmWarp;
	Matrix44                 m_matViewInv;
	i32                      m_MatDepth;

	string                   m_Description;
	EWindowState             m_windowState = EWindowState::Windowed;
	bool                     m_isChangingResolution = false;
	bool					 m_bWindowRestored = false; // Dirty-flag set when the window was restored from minimized state

	static constexpr i32         MAX_RT_STACK = 8;

#if DRX_PLATFORM_WINDOWS || DRX_RENDERER_OPENGL
	static constexpr i32         RT_STACK_WIDTH = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
#else
	static constexpr i32         RT_STACK_WIDTH = 4;
#endif

private:
	//////////////////////////////////////////////////////////////////////////
	// PRIVATE MEMBERS
	//////////////////////////////////////////////////////////////////////////
	bool m_bInitialized = false;

	// Windows context
	char   m_WinTitle[80];
	HWND   m_hWnd;                 // The main app window
	HWND   m_hWndDesktop;          // The desktop window
#if DRX_PLATFORM_WINDOWS
	HICON  m_hIconBig;             // Icon currently being used on the taskbar
	HICON  m_hIconSmall;           // Icon currently being used on the window
	string m_iconPath;             // Path to the icon currently loaded
#endif

	u32                           m_uLastBlendFlagsPassGroup;

	CD3DStereoRenderer*              m_pStereoRenderer;

	 i32                     m_lockCharCB;
	util::list<SCharacterInstanceCB> m_CharCBFreeList;
	util::list<SCharacterInstanceCB> m_CharCBActiveList[3];
	 i32                     m_CharCBFrameRequired[3];
	 i32                     m_CharCBAllocated;

	std::map<SDisplayContextKey, std::shared_ptr<CRenderDisplayContext>> m_displayContexts;

	u32                                                m_uniqueDisplayContextId = 0;
	std::shared_ptr<CRenderDisplayContext>                m_pActiveContext;
	SDisplayContextKey                                    m_activeContextKey;
	std::shared_ptr<CSwapChainBackedRenderDisplayContext> m_pBaseDisplayContext;

	enum PresentStatus
	{
		epsOccluded     = 1 << 0,
		epsNonExclusive = 1 << 1,
	};
	DWORD           m_dwPresentStatus; // Indicate present status

	DWORD           m_dwCreateFlags;      // Indicate sw or hw vertex processing
	char            m_strDeviceStats[90]; // String to hold D3D device stats

	i32             m_SceneRecurseCount = 0;

#if DRX_PLATFORM_WINDOWS
	uint m_nConnectedMonitors;  // The number of monitors currently connected to the system
	bool m_bDisplayChanged;     // Dirty-flag set when the number of monitors in the system changes
#endif

#if defined(ENABLE_PROFILING_CODE)
	CTexture* m_pSaveTexture[2];

	// Surfaces used to capture the current screen
	u32           m_captureFlipFlop;
	// Variables used for frame buffer capture and callback
	ICaptureFrameListener* m_pCaptureCallBack[MAXFRAMECAPTURECALLBACK];
	u32           m_frameCaptureRegisterNum;
	i32                    m_nScreenCaptureRequestFrame[RT_COMMAND_BUF_COUNT];
	i32                    m_screenCapTexHandle[RT_COMMAND_BUF_COUNT];
#endif

	// fields to access the D3D Driver: Device, Context and other related objects
	// all should be accessed through the provided getters:
	// GetDevice(), GetDeviceContext() and so on
	// To access the device without synchronization (which happens in presentce of a asynchrounous device)
	// please use GetDevice_Unsynchronized(), GetDeviceContext_Unsynchronized() etc
	CDrxDeviceWrapper        m_DeviceWrapper;
	CDrxDeviceContextWrapper m_DeviceContextWrapper;

#if defined(DEVICE_SUPPORTS_PERFORMANCE_DEVICE)
	CDrxPerformanceDeviceWrapper        m_PerformanceDeviceWrapper;
	CDrxPerformanceDeviceContextWrapper m_PerformanceDeviceContextWrapper;
#endif

	t_arrDeferredMeshIndBuff  m_arrDeferredInds;
	t_arrDeferredMeshVertBuff m_arrDeferredVerts;

#if defined(SUPPORT_DEVICE_INFO)
	DeviceInfo m_devInfo;
#endif

#if RENDERER_SUPPORT_SCALEFORM
	struct SSF_ResourcesD3D* m_pSFResD3D;
#endif

#if defined(ENABLE_RENDER_AUX_GEOM)
	CRenderAuxGeomD3D*         m_pRenderAuxGeomD3D;
#endif
	CAuxGeomCB_Null            m_renderAuxGeomNull;

	CShadowTextureGroupUpr m_ShadowTextureGroupUpr;           // to combine multiple shadowmaps into one texture

	u32                     m_nTimeSlicedShadowsUpdatedThisFrame;

#ifdef ENABLE_BENCHMARK_SENSOR
public:
	BenchmarkFramework::IBenchmarkRendererSensor* m_benchmarkRendererSensor;
private:
#endif
};

///////////////////////////////////////////////////////////////////////////////
inline void CD3D9Renderer::BindContextToThread(DWORD threadID)
{
#if ENABLE_CONTEXT_THREAD_CHECKING
	m_DeviceOwningthreadID = threadID;
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline void CD3D9Renderer::CheckContextThreadAccess() const
{
#if ENABLE_CONTEXT_THREAD_CHECKING
	if (m_DeviceOwningthreadID != DrxGetCurrentThreadId())
		DrxFatalError("accessing d3d11 immediate context from unbound thread!");
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline DWORD CD3D9Renderer::GetBoundThreadID() const
{
	return m_DeviceOwningthreadID;
}

///////////////////////////////////////////////////////////////////////////////
inline void CD3D9Renderer::WaitForAsynchronousDevice() const
{
#if DURANGO_ENABLE_ASYNC_DIPS
	if (m_nAsyncDeviceState)
	{
		DRX_PROFILE_REGION_WAITING(PROFILE_RENDERER, "Sync Async DIPS");
		DRXPROFILE_SCOPE_PROFILE_MARKER("Sync Async DIPS");

		while (m_nAsyncDeviceState)
		{
#if DRX_PLATFORM_ORBIS || DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
			DrxSleep(0);
#else
			SwitchToThread();
#endif
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
inline  i32* CD3D9Renderer::GetAsynchronousDeviceState()
{
	DrxInterlockedIncrement(&m_nAsyncDeviceState);
	return &m_nAsyncDeviceState;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline CDrxDeviceWrapper& CD3D9Renderer::GetDevice()
{
	return m_DeviceWrapper;
}

inline const CDrxDeviceWrapper& CD3D9Renderer::GetDevice() const
{
	return m_DeviceWrapper;
}

///////////////////////////////////////////////////////////////////////////////
inline CDrxDeviceContextWrapper& CD3D9Renderer::GetDeviceContext()
{
	CheckContextThreadAccess();
	WaitForAsynchronousDevice();
	return m_DeviceContextWrapper;
}

#if defined(DEVICE_SUPPORTS_PERFORMANCE_DEVICE)
///////////////////////////////////////////////////////////////////////////////
inline CDrxPerformanceDeviceWrapper& CD3D9Renderer::GetPerformanceDevice()
{
	return m_PerformanceDeviceWrapper;
}

///////////////////////////////////////////////////////////////////////////////
inline CDrxPerformanceDeviceContextWrapper& CD3D9Renderer::GetPerformanceDeviceContext()
{
	CheckContextThreadAccess();
	WaitForAsynchronousDevice();
	return m_PerformanceDeviceContextWrapper;
}
#endif // DEVICE_SUPPORTS_PERFORMANCE_DEVICE

extern CD3D9Renderer gcpRendD3D;

//=========================================================================================

#include <drx3D/Render/D3DHWShader.h>

#include <drx3D/Render/ParticleBuffer.h>

//=========================================================================================

void EnableCloseButton(uk hWnd, bool enabled);
