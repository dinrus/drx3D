// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//  File:Renderer.h - Независимый от API
//
//  История:
//  -Jan 31,2001:Originally created by Marco Corbetta
//  -: Taken over by Andrey Khonich
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Renderer/IFlares.h> // <> required for Interfuscator
#include <drx3D/CoreX/Thread/IJobUpr.h>

#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/CoreX/BaseTypes.h>
#include <drx3D/CoreX/DrxVariant.h>
#include "IRenderView.h"

#include <vector>

//  предварительные объявления
struct SRenderingPassInfo;
struct IFoliage;
struct SRenderLight;
struct SWaterRippleInfo;

class CRenderView;

//! Callback used for DXTCompress.
typedef void (* MIPDXTcallback)(ukk buffer, size_t count, uk userData);

typedef void (* GpuCallbackFunc)(DWORD context);

//! Callback for shadercache miss.
typedef void (* ShaderCacheMissCallback)(tukk acShaderRequest);

struct ICaptureFrameListener
{
	virtual ~ICaptureFrameListener(){}

	virtual bool OnNeedFrameData(u8*& pConvertedTextureBuf) = 0;
	virtual void OnFrameCaptured(void) = 0;
	virtual i32  OnGetFrameWidth(void) = 0;
	virtual i32  OnGetFrameHeight(void) = 0;
	virtual i32  OnCaptureFrameBegin(i32* pTexHandle) = 0;

	enum ECaptureFrameFlags
	{
		eCFF_NoCaptureThisFrame = (0 << 1),
		eCFF_CaptureThisFrame   = (1 << 1),
	};
};

// Forward declarations.
//////////////////////////////////////////////////////////////////////
typedef uk WIN_HWND;
typedef uk WIN_HINSTANCE;
typedef uk WIN_HDC;
typedef uk WIN_HGLRC;

class CREMesh;
class CMesh;
//class   CImage;
struct  CStatObj;
class CVegetation;
struct  ShadowMapFrustum;
struct  IStatObj;
class CObjUpr;
struct  SPrimitiveGroup;
struct  ICharacterInstance;
class CRenderElement;
class CRenderObject;
class CTexMan;
//class   ColorF;
class CShadowVolEdge;
class CCamera;
struct SDeferredLightVolume;
struct  ILog;
struct  IConsole;
struct  ICVar;
struct  ITimer;
struct  ISystem;
class IDrxSizer;
struct IRenderAuxGeom;
struct SREPointSpriteCreateParams;
struct SPointSpriteVertex;
struct RenderLMData;
struct SShaderParam;
struct SSkyLightRenderParams;
struct SParticleRenderInfo;
struct SParticleAddJobCompare;
struct IFlashPlayer_RenderProxy;
struct IColorGradingController;
struct IStereoRenderer;
struct STextDrawContext;
struct IRenderMesh;
class IOpticsUpr;
struct SRendererCloakParams;
struct ShadowFrustumMGPUCache;
struct IAsyncTextureCompileListener;
struct IClipVolume;
struct SClipVolumeBlendInfo;
class IImageFile;
class CVrProjectionUpr;

//////////////////////////////////////////////////////////////////////
struct IScaleformPlayback;
struct GRendererCommandBufferReadOnly;
struct IFlashLoadMovieImage;

//////////////////////////////////////////////////////////////////////
namespace DrxOVR
{
struct RenderInitParams;
}

//////////////////////////////////////////////////////////////////////
typedef u8 bvec4[4];
typedef float         vec4_t[4];
typedef u8 byte;
typedef float         vec2_t[2];

#include <drx3D/CoreX/Math/Drx_Color.h>
#include "Tarray.h"

#include <drx3D/Font/IFont.h>

#define MAX_NUM_VIEWPORTS 7

//! Query types for DrxInd editor (used in EF_Query() function).
enum ERenderQueryTypes
{
	EFQ_DeleteMemoryArrayPtr = 1,
	EFQ_DeleteMemoryPtr,
	EFQ_GetShaderCombinations,
	EFQ_SetShaderCombinations,
	EFQ_CloseShaderCombinations,

	EFQ_MainThreadList,
	EFQ_RenderThreadList,
	EFQ_RenderMultithreaded,

	EFQ_RecurseLevel,
	EFQ_IncrementFrameID,
	EFQ_DeviceLost,

	EFQ_Alloc_APITextures,
	EFQ_Alloc_APIMesh,

	//! Memory allocated by meshes in system memory.
	EFQ_Alloc_Mesh_SysMem,
	EFQ_Mesh_Count,

	EFQ_HDRModeEnabled,
	EFQ_ParticlesTessellation,
	EFQ_WaterTessellation,
	EFQ_MeshTessellation,
	EFQ_GetShadowPoolFrustumsNum,
	EFQ_GetShadowPoolAllocThisFrameNum,
	EFQ_GetShadowMaskChannelsNum,
	EFQ_GetTiledShadingSkippedLightsNum,

	//! Query will return all textures in the renderer,
	//! pass pointer to an SRendererQueryGetAllTexturesParam instance
	EFQ_GetAllTextures,

	//! Release resources allocated by GetAllTextures query.
	//! Pass pointer to an SRendererQueryGetAllTexturesParam instance, populated by EFQ_GetAllTextures
	EFQ_GetAllTexturesRelease,

	//! Query will return all IRenderMesh objects in the renderer.
	//! Pass an array pointer to be allocated and filled with the IRendermesh pointers. The calling function is responsible for freeing this memory.
	//! This was originally a two pass process, but proved to be non-thread-safe, leading to buffer overruns and underruns.
	EFQ_GetAllMeshes,

	//! Multigpu (crossfire/sli) is enabled.
	EFQ_MultiGPUEnabled,
	EFQ_SetDrawNearFov,
	EFQ_GetDrawNearFov,
	EFQ_TextureStreamingEnabled,
	EFQ_MSAAEnabled,
	EFQ_AAMode,

	EFQ_Fullscreen,
	EFQ_GetTexStreamingInfo,
	EFQ_GetMeshPoolInfo,

	//! True when shading is done in linear space, de-gamma on texture lookup, gamma on frame buffer writing (sRGB), false otherwise.
	EFQ_sLinearSpaceShadingEnabled,

	//! The percentages of overscan borders for left/right and top/bottom to adjust the title safe area.
	EFQ_OverscanBorders,

	//! Get num active post effects.
	EFQ_NumActivePostEffects,

	//! Get size of textures memory pool.
	EFQ_TexturesPoolSize,
	EFQ_RenderTargetPoolSize,

	EFQ_GetShaderCacheInfo,

	EFQ_GetFogCullDistance,
	EFQ_GetMaxRenderObjectsNum,

	EFQ_IsRenderLoadingThreadActive,

	EFQ_GetParticleVertexBufferSize,
	EFQ_GetParticleIndexBufferSize,
	EFQ_GetMaxParticleContainer,
	EFQ_GetSkinningDataPoolSize,

	EFQ_SetDynTexSourceLayerInfo,
	EFQ_SetDynTexSourceSharedRTDim,
	EFQ_GetViewportDownscaleFactor,
	EFQ_ReverseDepthEnabled,

	EFQ_GetLastD3DDebugMessage
};

// Interface to the graphics constant buffers
struct IGraphicsDeviceConstantBuffer : public CMultiThreadRefCount
{
	// Set contents of the device buffer
	virtual void SetData(u8k* data, size_t size) = 0;
};
typedef _smart_ptr<IGraphicsDeviceConstantBuffer> IGraphicsDeviceConstantBufferPtr;

struct SWindGrid
{
	i32       m_nWidth;
	i32       m_nHeight;
	float     m_fCellSize;

	Vec3      m_vCentr;
	DrxHalf2* m_pData;

	SWindGrid()
	{
		m_pData = NULL;
	}
};

struct ID3DDebugMessage
{
public:
	virtual void        Release() = 0;
	virtual tukk GetMsg() const = 0;

protected:
	ID3DDebugMessage() {}
	virtual ~ID3DDebugMessage() {}
};

enum EScreenAspectRatio
{
	eAspect_Unknown,
	eAspect_4_3,
	eAspect_16_9,
	eAspect_16_10,
};

enum class EReadTextureFormat
{
	RGBA8,
	RGB8,
	BGR8
};

namespace gpu_pfx2 {
class IUpr;
}

//////////////////////////////////////////////////////////////////////
#define R_CULL_DISABLE 0
#define R_CULL_NONE    0
#define R_CULL_FRONT   1
#define R_CULL_BACK    2

//////////////////////////////////////////////////////////////////////
#define R_DEFAULT_LODBIAS 0

//////////////////////////////////////////////////////////////////////
#define R_SOLID_MODE      0
#define R_WIREFRAME_MODE  1
#define R_POINT_MODE      2

//////////////////////////////////////////////////////////////////////
#define R_DX11_RENDERER   0
#define R_DX12_RENDERER   1
#define R_GL_RENDERER     2
#define R_VK_RENDERER     3

#define STR_DX11_RENDERER   "DX11"
#define STR_DX12_RENDERER   "DX12"
#define STR_GL_RENDERER     "GL"
#define STR_VK_RENDERER     "VK"
#define STR_AUTO_RENDERER   "Auto"

#define STR_ORBIS_SHADER_TARGET   "ORBIS"
#define STR_DURANGO_SHADER_TARGET "DURANGO"
#define STR_D3D11_SHADER_TARGET   "D3D11"
#define STR_GL4_SHADER_TARGET     "GL4"
#define STR_GLES3_SHADER_TARGET   "GLES3"
#define STR_VULKAN_SHADER_TARGET  "VULKAN"

//////////////////////////////////////////////////////////////////////
#define STR_VK_SHADER_COMPILER_HLSLCC    "HLSLCC"
#define STR_VK_SHADER_COMPILER_GLSLANG   "GLSLANG"
#define STR_VK_SHADER_COMPILER_DXC       "DXC"

//////////////////////////////////////////////////////////////////////
// Render features

#define RFT_FREE_0x1         0x1
#define RFT_ALLOW_RECTTEX    0x2
#define RFT_OCCLUSIONQUERY   0x4
#define RFT_FREE_0x8         0x8
#define RFT_HWGAMMA          0x10
#define RFT_FREE_0x20        0x20
#define RFT_COMPRESSTEXTURE  0x40
#define RFT_FREE_0x80        0x80
#define RFT_ALLOWANISOTROPIC 0x100       // Allows anisotropic texture filtering.
#define RFT_SUPPORTZBIAS     0x200
#define RFT_FREE_0x400       0x400
#define RFT_FREE_0x800       0x800
#define RFT_FREE_0x1000      0x1000
#define RFT_FREE_0x2000      0x2000
#define RFT_FREE_0x4000      0x4000
#define RFT_OCCLUSIONTEST    0x8000      // Support hardware occlusion test.

#define RFT_HW_INTEL         0x10000     // Unclassified intel hardware.
#define RFT_HW_ATI           0x20000     // Unclassified ATI hardware.
#define RFT_HW_NVIDIA        0x40000     // Unclassified NVidia hardware.
#define RFT_HW_MASK          0x70000     // Graphics chip mask.

#define RFT_HW_HDR           0x80000     // Hardware supports high dynamic range rendering.

#define RFT_HW_SM20          0x100000    // Shader model 2.0
#define RFT_HW_SM2X          0x200000    // Shader model 2.X
#define RFT_HW_SM30          0x400000    // Shader model 3.0
#define RFT_HW_SM40          0x800000    // Shader model 4.0
#define RFT_HW_SM50          0x1000000   // Shader model 5.0

#define RFT_FREE_0x2000000   0x2000000
#define RFT_FREE_0x4000000   0x4000000
#define RFT_FREE_0x8000000   0x8000000
#define RFT_FREE_0x10000000  0x10000000

#define RFT_RGBA             0x20000000  // RGBA order (otherwise BGRA).
#define RFT_FREE_0x40000000  0x40000000
#define RFT_FREE_0x80000000  0x80000000

//====================================================================
// PrecacheResources flags

#define FPR_NEEDLIGHT                    1
#define FPR_2D                           2
#define FPR_HIGHPRIORITY                 4
#define FPR_SYNCRONOUS                   8
#define FPR_STARTLOADING                 16
#define FPR_SINGLE_FRAME_PRIORITY_UPDATE 32

//=====================================================================
// SetRenderTarget flags
#define SRF_SCREENTARGET           1
#define SRF_USE_ORIG_DEPTHBUF      2
#define SRF_USE_ORIG_DEPTHBUF_MSAA 4

//====================================================================
// Draw shaders flags (EF_EndEf3d)
enum EShaderRenderingFlags
{
	SHDF_ALLOWHDR           = BIT(0),
	SHDF_CUBEMAPGEN         = BIT(1),
	SHDF_ZPASS              = BIT(2),
	SHDF_STEREO_LEFT_EYE    = BIT(3),
	SHDF_STEREO_RIGHT_EYE   = BIT(4),
	SHDF_ALLOWPOSTPROCESS   = BIT(5),
	SHDF_BILLBOARDS         = BIT(6),
	SHDF_ALLOW_AO           = BIT(8),
	SHDF_ALLOW_WATER        = BIT(9),
	SHDF_NOASYNC            = BIT(10),
	SHDF_NO_DRAWNEAR        = BIT(11),
	SHDF_NO_DRAWCAUSTICS    = BIT(14),
	SHDF_NO_SHADOWGEN       = BIT(15),
	SHDF_SECONDARY_VIEWPORT = BIT(16),
	SHDF_FORWARD_MINIMAL    = BIT(17),
};

//////////////////////////////////////////////////////////////////////
// Virtual screen size
const float VIRTUAL_SCREEN_WIDTH  = 800.0f;
const float VIRTUAL_SCREEN_HEIGHT = 600.0f;

//////////////////////////////////////////////////////////////////////
// Object states
#define OS_ALPHA_BLEND         0x1
#define OS_ADD_BLEND           0x2
#define OS_MULTIPLY_BLEND      0x4
#define OS_TRANSPARENT         (OS_ALPHA_BLEND | OS_ADD_BLEND | OS_MULTIPLY_BLEND)
#define OS_NODEPTH_TEST        0x8
#define OS_NODEPTH_WRITE       0x10
#define OS_ANIM_BLEND          0x20
#define OS_ENVIRONMENT_CUBEMAP 0x40

// Render State flags (u32)
#define GS_BLSRC_ZERO              0x01
#define GS_BLSRC_ONE               0x02
#define GS_BLSRC_DSTCOL            0x03
#define GS_BLSRC_ONEMINUSDSTCOL    0x04
#define GS_BLSRC_SRCALPHA          0x05
#define GS_BLSRC_ONEMINUSSRCALPHA  0x06
#define GS_BLSRC_DSTALPHA          0x07
#define GS_BLSRC_ONEMINUSDSTALPHA  0x08
#define GS_BLSRC_ALPHASATURATE     0x09
#define GS_BLSRC_SRCALPHA_A_ZERO   0x0A  // separate alpha blend state
#define GS_BLSRC_SRC1ALPHA         0x0B  // dual source blending
#define GS_BLSRC_MASK              0x0F
#define GS_BLSRC_SHIFT             0

#define GS_BLDST_ZERO              0x10
#define GS_BLDST_ONE               0x20
#define GS_BLDST_SRCCOL            0x30
#define GS_BLDST_ONEMINUSSRCCOL    0x40
#define GS_BLDST_SRCALPHA          0x50
#define GS_BLDST_ONEMINUSSRCALPHA  0x60
#define GS_BLDST_DSTALPHA          0x70
#define GS_BLDST_ONEMINUSDSTALPHA  0x80
#define GS_BLDST_ONE_A_ZERO        0x90 // separate alpha blend state
#define GS_BLDST_ONEMINUSSRC1ALPHA 0xA0 // dual source blending
#define GS_BLDST_MASK              0xF0
#define GS_BLDST_SHIFT             4

#define GS_DEPTHWRITE              0x00000100
#define GS_NODEPTHTEST             0x00000200
#define GS_STENCIL                 0x00000400
#define GS_ALPHATEST               0x00000800

#define GS_COLMASK_SHIFT           12
#define GS_COLMASK_MASK            0x0000F000

// Color Exclusion Mask
// Note: Do not concatenate!
enum ColorMask
{
	GS_NOCOLMASK_NONE				= 0,
	GS_NOCOLMASK_R					= 0x00001000,
	GS_NOCOLMASK_G					= 0x00002000,
	GS_NOCOLMASK_B					= 0x00003000,
	GS_NOCOLMASK_A					= 0x00004000,
	GS_NOCOLMASK_GBA				= 0x00005000,
	GS_NOCOLMASK_RBA				= 0x00006000,
	GS_NOCOLMASK_RGA				= 0x00007000,
	GS_NOCOLMASK_RGB				= 0x00008000,
	GS_NOCOLMASK_RGBA				= 0x00009000,
	GS_NOCOLMASK_GBUFFER_OVERLAY	= 0x0000A000,

	GS_NOCOLMASK_COUNT				= 0x0000B000,
};
static_assert(GS_NOCOLMASK_COUNT <= GS_COLMASK_MASK, "Exceeded count limit of possible color masks (16)");

#define GS_WIREFRAME               0x00010000
#define GS_POINTRENDERING          0x00020000

#define GS_DEPTHFUNC_LEQUAL        0x00000000
#define GS_DEPTHFUNC_EQUAL         0x00040000
#define GS_DEPTHFUNC_GREAT         0x00080000
#define GS_DEPTHFUNC_LESS          0x000C0000
#define GS_DEPTHFUNC_GEQUAL        0x00100000
#define GS_DEPTHFUNC_NOTEQUAL      0x00140000
#define GS_DEPTHFUNC_MASK          0x001C0000 // enum
#define GS_DEPTHFUNC_SHIFT         18

#define GS_BLEND_OP_ADD            0x00000000
#define GS_BLEND_OP_MAX            0x00200000
#define GS_BLEND_OP_MIN            0x00400000
#define GS_BLEND_OP_SUB            0x00600000
#define GS_BLEND_OP_SUBREV         0x00800000
#define GS_BLEND_OP_MASK           0x00E00000 // enum
#define GS_BLEND_OP_SHIFT          21

// Separate alpha blend mode
#define GS_BLALPHA_NONSEP 0x00000000          // same alpha blend op as color blend op (non-separate)
#define GS_BLALPHA_MAX    0x02000000
#define GS_BLALPHA_MIN    0x04000000
#define GS_BLALPHA_MASK   0x0E000000          // enum
#define GS_BLALPHA_SHIFT  25

#define GS_BLEND_MASK     (GS_BLSRC_MASK | GS_BLDST_MASK | GS_BLEND_OP_MASK | GS_BLALPHA_MASK)

#define FORMAT_8_BIT      8
#define FORMAT_24_BIT     24
#define FORMAT_32_BIT     32

//! Read FrameBuffer type.
enum ERB_Type
{
	eRB_BackBuffer,
	eRB_FrontBuffer,
	eRB_ShadowBuffer
};

enum EVertexCostTypes
{
	EVCT_STATIC = 0,
	EVCT_VEGETATION,
	EVCT_SKINNED,
	EVCT_NUM
};

//////////////////////////////////////////////////////////////////////

struct SDispFormat
{
	i32 m_Width;
	i32 m_Height;
	i32 m_BPP;
};

struct SAAFormat
{
	char szDescr[64];
	i32  nSamples;
	i32  nQuality;
};

//! \cond INTERNAL
//! Info about Terrain sector texturing.
struct SSectorTextureSet
{
	SSectorTextureSet()
	{
		ZeroStruct(*this);
		fTexScale = 1.f;
		nSlot0 = nSlot1 = nSlot2 = -1;
	}

	unsigned short nTex0, nTex1, nTex2, nSlot0, nSlot1, nSlot2;
	float          fTexOffsetX, fTexOffsetY, fTexScale;
};
//! \endcond

struct IRenderNode;
struct SShaderItem;
struct IParticleVertexCreator;
struct IParticleComponentInstance;
namespace gpu_pfx2
{
	class IParticleComponentRuntime;
}

struct DRX_ALIGN(16) SAddParticlesToSceneJob
{
	void GetMemoryUsage(IDrxSizer* pSizer) const {}

	SShaderItem* pShaderItem;
	CRenderObject* pRenderObject;
	IParticleVertexCreator* pVertexCreator = nullptr;
	gpu_pfx2::IParticleComponentRuntime* pGpuRuntime = nullptr;
	i16 nCustomTexId;
	AABB aabb;
};

#ifdef SUPPORT_HW_MOUSE_CURSOR
class IHWMouseCursor
{
public:
	virtual ~IHWMouseCursor() {}
	virtual void SetPosition(i32 x, i32 y) = 0;
	virtual void Show() = 0;
	virtual void Hide() = 0;
};
#endif

//////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Renderer/IShader.h> // <> required for Interfuscator
#include <drx3D/CoreX/Renderer/IRenderMesh.h>
#include "IMeshBaking.h"

//! Flags passed in function FreeResources.
#define FRR_DELETED_MESHES           BIT(0)
#define FRR_FLUSH_TEXTURESTREAMING   BIT(1)
#define FRR_RP_BUFFERS               BIT(2)
#define FRR_POST_EFFECTS             BIT(3)
#define FRR_OBJECTS                  BIT(4)
#define FRR_PERMANENT_RENDER_OBJECTS BIT(5)
#define FRR_SYSTEM                   BIT(6) // requires also FRR_OBJECTS
#define FRR_SYSTEM_RESOURCES         BIT(7) // requires also FRR_DELETED_MESHES |  FRR_FLUSH_TEXTURESTREAMING | FRR_RP_BUFFERS | FRR_POST_EFFECTS | FRR_OBJECTS | FRR_PERMANENT_RENDER_OBJECTS | FRR_SVOGI
#define FRR_TEXTURES                 BIT(8) // requires also FRR_DELETED_MESHES |  FRR_FLUSH_TEXTURESTREAMING | FRR_RP_BUFFERS | FRR_POST_EFFECTS | FRR_OBJECTS | FRR_PERMANENT_RENDER_OBJECTS | FRR_SYSTEM_RESOURCES | FRR_SVOGI
#define FRR_ALL                      -1

// Free resource flag combinations for commonly used operations
#define FRR_LEVEL_UNLOAD_SANDBOX  (FRR_DELETED_MESHES | FRR_FLUSH_TEXTURESTREAMING | FRR_OBJECTS | FRR_RP_BUFFERS | FRR_POST_EFFECTS | FRR_PERMANENT_RENDER_OBJECTS)
#define FRR_LEVEL_UNLOAD_LAUNCHER (FRR_DELETED_MESHES | FRR_FLUSH_TEXTURESTREAMING | FRR_OBJECTS | FRR_RP_BUFFERS | FRR_POST_EFFECTS | FRR_PERMANENT_RENDER_OBJECTS | FRR_SYSTEM_RESOURCES)
#define FRR_SHUTDOWN              (FRR_ALL)

// Refresh render resources flags.
// Flags passed in function RefreshResources.
#define FRO_SHADERS                     BIT(0)
#define FRO_SHADERTEXTURES              BIT(1)
#define FRO_TEXTURES                    BIT(2)
#define FRO_GEOMETRY                    BIT(3)
#define FRO_FORCERELOAD                 BIT(4)

//=============================================================================
// Shaders render target stuff.

#define FRT_CLEAR_DEPTH                 BIT(0)  // equals CLEAR_ZBUFFER and D3D11_CLEAR_DEPTH
#define FRT_CLEAR_STENCIL               BIT(1)  // equals CLEAR_STENCIL and D3D11_CLEAR_STENCIL
#define FRT_CLEAR_COLOR                 BIT(2)  // equals CLEAR_RTARGET
#define FRT_CLEAR_FOGCOLOR              BIT(3)

#define FRT_OVERLAY_DEPTH               BIT(4)  // The overlay rendering uses a persistent depth buffer (and gets the rendering depth too)
#define FRT_OVERLAY_STENCIL             BIT(5)  // The overlay rendering uses a persistent stencil buffer)

#define FRT_CAMERA_REFLECTED_WATERPLANE BIT(6)
#define FRT_CAMERA_REFLECTED_PLANE      BIT(7)
#define FRT_CAMERA_CURRENT              BIT(8)

#define FRT_USE_FRONTCLIPPLANE          BIT(9)
#define FRT_USE_BACKCLIPPLANE           BIT(10)

#define FRT_GENERATE_MIPS               BIT(11)
#define FRT_TEMPORARY_DEPTH             BIT(12) // The rendering uses a temporary depth buffer and the overlay rendering won't get any depth

#define FRT_RENDTYPE_CURSCENE           BIT(13)
#define FRT_RENDTYPE_RECURSIVECURSCENE  BIT(14)
#define FRT_RENDTYPE_COPYSCENE          BIT(15)

#define FRT_CLEAR                       (FRT_CLEAR_COLOR | FRT_CLEAR_DEPTH | FRT_CLEAR_STENCIL)

//! Flags used in DrawText function.
enum EDrawTextFlags
{
	eDrawText_Default,
	eDrawText_Center         = BIT(0),  //!< Centered alignment, otherwise right or left.
	eDrawText_Right          = BIT(1),  //!< Right alignment, otherwise center or left.
	eDrawText_CenterV        = BIT(2),  //!< Center vertically, oterhwise top.
	eDrawText_Bottom         = BIT(3),  //!< Bottom alignment.

	eDrawText_2D             = BIT(4),  //!< 3-component vector is used for xy screen position, otherwise it's 3d world space position.

	eDrawText_FixedSize      = BIT(5),  //!< Font size is defined in the actual pixel resolution, otherwise it's in the virtual 800x600.
	eDrawText_800x600        = BIT(6),  //!< Position are specified in the virtual 800x600 resolution, otherwise coordinates are in pixels.

	eDrawText_Monospace      = BIT(7),  //!< Non proportional font rendering (Font width is same for all characters).

	eDrawText_Framed         = BIT(8),  //!< Draw a transparent, rectangular frame behind the text to ease readability independent from the background.

	eDrawText_DepthTest      = BIT(9),  //!< Text should be occluded by world geometry using the depth buffer.
	eDrawText_IgnoreOverscan = BIT(10), //!< Ignore the overscan borders, text should be drawn at the location specified.
	eDrawText_LegacyBehavior = BIT(11)  //!< Reserved for internal system use.
};

//! \cond INTERNAL
//! This structure used in DrawText method of renderer.
//! It provide all necessary information of how to render text on screen.
struct SDrawTextInfo
{
	//! One of EDrawTextFlags flags.
	i32 flags;

	//! Text color, (r,g,b,a) all members must be specified.
	float color[4];
	Vec2  scale;
	IFFont* pFont;

	SDrawTextInfo()
	{
		flags = 0;
		color[0] = color[1] = color[2] = color[3] = 1;
		scale = ZERO;
		pFont = nullptr;
	}
};
//! \endcond

#define UIDRAW_TEXTSIZEFACTOR (12.0f)

#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_ORBIS
	#define MAX_GPU_NUM 1
#else
//! SLI/CROSSFIRE GPU maximum count.
	#define MAX_GPU_NUM 4
#endif

#define MAX_FRAME_ID_STEP_PER_FRAME 20
i32k MAX_GSM_LODS_NUM = 16;
i32k MAX_GSM_CACHED_LODS_NUM = 3;

const f32 DRAW_NEAREST_MIN = 0.03f;
const f32 DRAW_NEAREST_MAX = 40.0f;

//===================================================================

//////////////////////////////////////////////////////////////////////
struct IRendererEventListener
{
	virtual void OnPostCreateDevice() = 0;
	virtual void OnPostResetDevice() = 0;
	virtual ~IRendererEventListener(){}
};

//////////////////////////////////////////////////////////////////////
struct ILoadtimeCallback
{
	virtual void LoadtimeUpdate(float fDeltaTime) = 0;
	virtual void LoadtimeRender() = 0;
	virtual ~ILoadtimeCallback(){}
};

//////////////////////////////////////////////////////////////////////
struct ISyncMainWithRenderListener
{
	virtual void SyncMainWithRender() = 0;
	virtual ~ISyncMainWithRenderListener(){}
};

//////////////////////////////////////////////////////////////////////
enum class ERenderType : u8
{
	Undefined,
	Direct3D11,
	Direct3D12,
	OpenGL,
	Vulkan
};

//! Scale factor between photometric and internal light units.
const float RENDERER_LIGHT_UNIT_SCALE = 10000.0f;

#if DRX_PLATFORM_ANDROID
enum {CULL_SIZEX = 128};
enum {CULL_SIZEY = 64};
#else
enum {CULL_SIZEX = 256};
enum {CULL_SIZEY = 128};
#endif

class ITextureStreamListener
{
public:
	virtual void OnCreatedStreamedTexture(uk pHandle, tukk name, i32 nMips, i32 nMinMipAvailable) = 0;
	virtual void OnUploadedStreamedTexture(uk pHandle) = 0;
	virtual void OnDestroyedStreamedTexture(uk pHandle) = 0;
	virtual void OnTextureWantsMip(uk pHandle, i32 nMinMip) = 0;
	virtual void OnTextureHasMip(uk pHandle, i32 nMinMip) = 0;
	virtual void OnBegunUsingTextures(uk * pHandles, size_t numHandles) = 0;
	virtual void OnEndedUsingTextures(uk * pHandle, size_t numHandles) = 0;

protected:
	virtual ~ITextureStreamListener() {}
};

#if defined(DRX_ENABLE_RC_HELPER)
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>
//! Listener for asynchronous texture compilation.
//! Connects the listener to the task-queue of pending compilation requests.
enum ERcExitCode;
struct IAsyncTextureCompileListener
{
public:
	virtual void OnCompilationStarted(tukk source, tukk target, i32 nPending) = 0;
	virtual void OnCompilationFinished(tukk source, tukk target, ERcExitCode eReturnCode) = 0;

	virtual void OnCompilationQueueTriggered(i32 nPending) = 0;
	virtual void OnCompilationQueueDepleted() = 0;

protected:
	virtual ~IAsyncTextureCompileListener() {}
};
#endif

enum ERenderPipelineProfilerStats
{
	eRPPSTATS_OverallFrame = 0,
	eRPPSTATS_Recursion,

	// Scene
	eRPPSTATS_SceneOverall,
	eRPPSTATS_SceneDecals,
	eRPPSTATS_SceneForward,
	eRPPSTATS_SceneWater,

	// Shadows
	eRPPSTATS_ShadowsOverall,
	eRPPSTATS_ShadowsSun,
	eRPPSTATS_ShadowsSunCustom,
	eRPPSTATS_ShadowsLocal,

	// Lighting
	eRPPSTATS_LightingOverall,
	eRPPSTATS_LightingGI,

	// VFX
	eRPPSTATS_VfxOverall,
	eRPPSTATS_VfxTransparent,
	eRPPSTATS_VfxFog,
	eRPPSTATS_VfxFlares,

	// Individual Total Illumination stats
	eRPPSTATS_TI_INJECT_CLEAR,
	eRPPSTATS_TI_VOXELIZE,
	eRPPSTATS_TI_INJECT_AIR,
	eRPPSTATS_TI_INJECT_LIGHT,
	eRPPSTATS_TI_INJECT_REFL0,
	eRPPSTATS_TI_INJECT_REFL1,
	eRPPSTATS_TI_INJECT_DYNL,
	eRPPSTATS_TI_NID_DIFF,
	eRPPSTATS_TI_GEN_DIFF,
	eRPPSTATS_TI_GEN_SPEC,
	eRPPSTATS_TI_GEN_AIR,
	eRPPSTATS_TI_DEMOSAIC_DIFF,
	eRPPSTATS_TI_DEMOSAIC_SPEC,
	eRPPSTATS_TI_UPSCALE_DIFF,
	eRPPSTATS_TI_UPSCALE_SPEC,

	RPPSTATS_NUM
};

struct RPProfilerStats
{
	float  gpuTime;
	float  gpuTimeSmoothed;
	float  gpuTimeMax;
	float  cpuTime;
	u32 numDIPs;
	u32 numPolys;

	// Internal
	float _gpuTimeMaxNew;
};

// See CRenderPipelineProfiler::SProfilerSection
struct RPProfilerDetailedStats
{
	char            name[31];
	float           gpuTime;
	float           gpuTimeSmoothed;
	float           cpuTime;
	float           cpuTimeSmoothed;
	CTimeValue      startTimeCPU, endTimeCPU;
	uint64          startTimeGPU, endTimeGPU;
	i32             numDIPs, numPolys;
	int8            recLevel;   // Negative value means error in stack
	u8           flags;
};

struct ISvoRenderer
{
	virtual bool IsShaderItemUsedForVoxelization(SShaderItem& rShaderItem, IRenderNode* pRN) { return false; }
	virtual void SetEditingHelper(const Sphere& sp)                                          {}
	virtual void Release()                                                                   {}
	virtual void InitCVarValues()                                                            {}
};

//! \cond INTERNAL
//! Describes the key used to create display context
struct SDisplayContextKey
{
	struct SInvalidKey
	{
		bool operator<(const SInvalidKey&) const { return false; }
		bool operator==(const SInvalidKey&) const { return true; }
		bool operator!=(const SInvalidKey&) const { return false; }
	};

	DrxVariant<SInvalidKey, uint32_t, HWND> key;
	bool operator<(const SDisplayContextKey &o) const { return key < o.key; }
	bool operator==(const SDisplayContextKey &o) const { return key == o.key; }
	bool operator!=(const SDisplayContextKey &o) const { return !(*this == o); }

	bool operator==(const uint32_t &o) const { return key == o; }
	bool operator!=(const uint32_t &o) const { return !(*this == o); }
	bool operator==(const HWND &o) const { return key == o; }
	bool operator!=(const HWND &o) const { return !(*this == o); }
};

//! \cond INTERNAL
//! Describes rendering viewport dimensions
struct SRenderViewport
{
	i32   x      = 0;
	i32   y      = 0;
	i32   width  = 0;
	i32   height = 0;
	float zmin   = 0.f;
	float zmax   = 1.f;

	SRenderViewport() {}

	SRenderViewport(i32 newX, i32 newY, i32 newWidth, i32 newHeight)
		: x(newX)
		, y(newY)
		, width(newWidth)
		, height(newHeight)
		, zmin(0.0f)
		, zmax(1.0f)
	{}

	bool operator==(const SRenderViewport& v)
	{
		return x == v.x && y == v.y && width == v.width && height == v.height && zmin == v.zmin && zmax == v.zmax;
	}
	bool operator!=(const SRenderViewport& v)
	{
		return !(*this == v);
	}
};
//! \endcond

#include "IRenderView.h"

struct IRendererEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IRendererEngineModule, "516d044b-d626-4283-b08d-9ad9558204e0"_drx_guid);
};

//! Main interface to the renderer implementation, wrapping the low-level graphics API
struct IRenderer//: public IRendererCallbackServer
{
	enum EViewportType : u8
	{
		eViewportType_Default,
		eViewportType_Secondary,
	};

	struct SDisplayContextDescription
	{
		HWND handle  = 0; // WIN_HWND

		ColorF clearColor               = Clr_Empty;
		ColorF clearDepthStencil        = Clr_FarPlane_Rev;
		Vec2i  superSamplingFactor      = { 1, 1 };
		Vec2i  screenResolution         = { 0, 0 };

		EViewportType type              = eViewportType_Default;
		u16 renderFlags              = FRT_CLEAR | FRT_OVERLAY_DEPTH | FRT_OVERLAY_STENCIL;

		bool vsync                      = true;
	};

	virtual ~IRenderer(){}
	virtual void        AddListener(IRendererEventListener* pRendererEventListener) = 0;
	virtual void        RemoveListener(IRendererEventListener* pRendererEventListener) = 0;

	virtual ERenderType GetRenderType() const = 0;

	//! Initializes the renderer, parameters are self-explanatory.
	virtual WIN_HWND Init(i32 x, i32 y, i32 width, i32 height, u32 cbpp, i32 zbpp, i32 sbits, WIN_HWND Glhwnd = 0, bool bReInit = false, bool bShaderCacheGen = false) = 0;
	virtual void     PostInit() = 0;

	//! Start active rendering of the intro movies while initializing the rest of the engine.
	virtual void StartRenderIntroMovies() = 0;
	virtual void StopRenderIntroMovies(bool bWaitForFinished) = 0;
	virtual bool IsRenderingIntroMovies() const = 0;

	virtual bool IsPost3DRendererEnabled() const { return false; }

	virtual void ExecuteAsyncDIP() = 0;

	virtual i32  GetFeatures() = 0;
	virtual void GetVideoMemoryUsageStats(size_t& vidMemUsedThisFrame, size_t& vidMemUsedRecently, bool bGetPoolsSizes = false) = 0;
	virtual i32  GetNumGeomInstances() = 0;
	virtual i32  GetNumGeomInstanceDrawCalls() = 0;
	virtual i32  GetCurrentNumberOfDrawCalls() = 0;
	virtual void GetCurrentNumberOfDrawCalls(i32& nGeneral, i32& nShadowGen) = 0;

	//! Sums DIP counts for the EFSLIST_* passes that match the submitted mask.
	//! Compose the mask with bitwise arithmetic, use (1 << EFSLIST_*) per list.
	//! E.g. to sum general and transparent, pass in ( (1 << EFSLIST_GENERAL) | (1 << EFSLIST_TRANSP) ).
	virtual i32  GetCurrentNumberOfDrawCalls(u32k EFSListMask) = 0;

	virtual void SetDebugRenderNode(IRenderNode* pRenderNode) = 0;
	virtual bool IsDebugRenderNode(IRenderNode* pRenderNode) const = 0;

	/////////////////////////////////////////////////////////////////////////////////
	// Render-context management
	/////////////////////////////////////////////////////////////////////////////////
	virtual SDisplayContextKey     CreateSwapChainBackedContext(const SDisplayContextDescription& desc) = 0;
	virtual void                   ResizeContext(const SDisplayContextKey& key, i32 width, i32 height) = 0;
	virtual bool                   DeleteContext(const SDisplayContextKey& key) = 0;

#if DRX_PLATFORM_WINDOWS
	virtual RectI    GetDefaultContextWindowCoordinates() = 0;
#endif
	/////////////////////////////////////////////////////////////////////////////////

#if DRX_PLATFORM_DURANGO
	virtual void SuspendDevice() = 0;
	virtual void ResumeDevice() = 0;
#endif

	//! Shuts down the renderer.
	virtual void ShutDown(bool bReInit = false) = 0;
	virtual void ShutDownFast() = 0;

	//! Creates array of all supported video formats (except low resolution formats).
	//! \return Number of formats in memory.
	virtual i32 EnumDisplayFormats(SDispFormat* Formats) = 0;

	//! Should be called at the beginning of every frame.
	virtual void BeginFrame(const SDisplayContextKey& key) = 0;

	//! Should be called at the beginning of every frame.
	virtual void FillFrame(ColorF clearColor) = 0;

	//! Creates default system shaders and textures.
	virtual void InitSystemResources(i32 nFlags) = 0;

	//! Frees the allocated resources.
	virtual void FreeSystemResources(i32 nFlags) = 0;

	//! Shuts down the renderer.
	virtual void Release() = 0;

	//! \see r_ShowDynTextures
	virtual void RenderDebug(bool bRenderStats = true) = 0;

	//! Should be called at the end of every frame.
	virtual void EndFrame() = 0;

	//! Try to flush the render thread commands to keep the render thread active during level loading, but simpy return if the render thread is still busy
	virtual void               TryFlush() = 0;

	virtual Vec2               SetViewportDownscale(float xscale, float yscale) = 0;

	virtual CRenderView*   GetOrCreateRenderView(IRenderView::EViewType Type = IRenderView::eViewType_Default) = 0;
	virtual void           ReturnRenderView(CRenderView* pRenderView) = 0;

	//! Set delta gamma.
	virtual bool SetGammaDelta(const float fGamma) = 0;

	//! Reset gamma setting if not in fullscreen mode.
	virtual void RestoreGamma(void) = 0;

	// Should not be here.
	//! Saves source data to a Tga file.
	virtual bool SaveTga(u8* sourcedata, i32 sourceformat, i32 w, i32 h, tukk filename, bool flip) const = 0;

	//! Gets the white texture Id.
	virtual i32 GetWhiteTextureId() const = 0;

	//! Used for calculating the hit uv-coordinates from a raycast.
	virtual i32	GetDetailedRayHitInfo(IPhysicalEntity* pCollider, const Vec3& vOrigin, const Vec3& vDirection, const float maxRayDist, float* pUOut, float* pVOut) = 0;

	//! Turns screen point into vector that is projected into scene, from the current view
	virtual Vec3 UnprojectFromScreen(i32 x, i32 y) = 0;

	//! Gets height of the main rendering resolution.
	virtual i32 GetHeight() const = 0;

	//! Gets width of the main rendering resolution.
	virtual i32 GetWidth() const = 0;

	//! Gets Pixel Aspect Ratio.
	virtual float GetPixelAspectRatio() const = 0;

	//! Gets width of the height of the overlay viewport where UI and debug output are rendered.
	virtual i32 GetOverlayHeight() const = 0;

	//! Gets width of the width of the overlay viewport where UI and debug output are rendered.
	virtual i32 GetOverlayWidth() const = 0;

	//! Gets memory status information
	virtual void GetMemoryUsage(IDrxSizer* Sizer) = 0;

	//! Gets textures streaming bandwidth information
	virtual void GetBandwidthStats(float* fBandwidthRequested) = 0;

	//! Sets an event listener for texture streaming updates
	virtual void SetTextureStreamListener(ITextureStreamListener* pListener) = 0;

#if defined(DRX_ENABLE_RC_HELPER)
	//! Sets an event listener for triggered asynchronous resource compilation.
	//! The listener is called asynchronously as well, thus doesn't block the renderer/streaming,
	//! but it does block the worker-queue, no resources will be compiled for as long as the listener executes.
	//! The listener doesn't need to be thread-safe, there is always just one notified.
	virtual void AddAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener) = 0;
	virtual void RemoveAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener) = 0;
#endif

	//! Pins an occlusion buffer into CPU memory, from where it can be used until UnpinOcclusionBuffer is called.
	//! The occlusion buffer is of type 'float[CULL_SIZEX * CULL_SIZEY]', and contains non-linearized depth values from 0 (near) to 1 (far).
	//! While at least one client has a buffer pinned, all other clients are guaranteed to pin the same buffer.
	//! Note: This implies all pins have to be released at some point or all clients will forever be stuck with old data.
	//! (Un)pinning can be arbitrarily nested from any thread and will never block forward progress of the render thread or GPU.
	//! The function may fail (by returning nullptr) if no data is available, in which case you must not call UnpinOcclusionBuffer.
	virtual float* PinOcclusionBuffer(Matrix44A& camera) = 0;
	virtual void   UnpinOcclusionBuffer() = 0;

	//! Get current bpp.
	virtual i32 GetColorBpp() = 0;

	//! Get current z-buffer depth.
	virtual i32 GetDepthBpp() = 0;

	//! Get current stencil bits.
	virtual i32 GetStencilBpp() = 0;

	//! Returns true if stereo rendering is enabled.
	virtual bool IsStereoEnabled() const = 0;

	//! Returns values of nearest rendering z-range max
	virtual float GetNearestRangeMax() const = 0;

	//! Projects to screen.
	//! Returns true if successful.
	virtual bool ProjectToScreen(
	  float ptx, float pty, float ptz,
	  float* sx, float* sy, float* sz) = 0;

	//! Unprojects to screen.
	virtual i32 UnProject(
	  float sx, float sy, float sz,
	  float* px, float* py, float* pz,
	  const float modelMatrix[16],
	  const float projMatrix[16],
	  i32k viewport[4]) = 0;

	//! Unprojects from screen.
	virtual i32 UnProjectFromScreen(
	  float sx, float sy, float sz,
	  float* px, float* py, float* pz) = 0;

	virtual bool WriteDDS(byte* dat, i32 wdt, i32 hgt, i32 Size, tukk name, ETEX_Format eF, i32 NumMips) = 0;
	virtual bool WriteTGA(byte* dat, i32 wdt, i32 hgt, tukk name, i32 src_bits_per_pixel, i32 dest_bits_per_pixel) = 0;
	virtual bool WriteJPG(byte* dat, i32 wdt, i32 hgt, tuk name, i32 src_bits_per_pixel, i32 nQuality = 100) = 0;

	/////////////////////////////////////////////////////////////////////////////////
	//Replacement functions for Font

	virtual bool FontUploadTexture(class CFBitmap*, ETEX_Format eTF = eTF_R8G8B8A8) = 0;
	virtual i32  FontCreateTexture(i32 Width, i32 Height, byte* pData, ETEX_Format eTF = eTF_R8G8B8A8, bool genMips = false) = 0;
	virtual bool FontUpdateTexture(i32 nTexId, i32 X, i32 Y, i32 USize, i32 VSize, byte* pData) = 0;
	virtual void FontReleaseTexture(class CFBitmap* pBmp) = 0;

	virtual bool FlushRTCommands(bool bWait, bool bImmediatelly, bool bForce) = 0;
	virtual i32  CurThreadList() = 0;

	virtual void FlashRender(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer) = 0;
	virtual void FlashRenderPlaybackLockless(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, i32 cbIdx, bool finalPlayback) = 0;
	virtual void FlashRenderPlayer(std::shared_ptr<IFlashPlayer> &&pPlayer) = 0;
	virtual void FlashRemoveTexture(ITexture* pTexture) = 0;

	/////////////////////////////////////////////////////////////////////////////////
	// External interface for shaders
	/////////////////////////////////////////////////////////////////////////////////
	virtual bool           EF_PrecacheResource(SShaderItem *pSI, i32 iScreenTexels, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter = 1) = 0;
	virtual bool           EF_PrecacheResource(SShaderItem* pSI, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter = 1) = 0;
	virtual bool           EF_PrecacheResource(IShader* pSH, float fMipFactor, float fTimeToReady, i32 Flags) = 0;
	virtual bool           EF_PrecacheResource(ITexture* pTP, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter = 1) = 0;
	virtual bool           EF_PrecacheResource(IRenderMesh* pPB, IMaterial* pMaterial, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId) = 0;
	virtual bool           EF_PrecacheResource(SRenderLight* pLS, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId) = 0;

	virtual void           PostLevelLoading() = 0;
	virtual void           PostLevelUnload() = 0;

	virtual void           EF_AddMultipleParticlesToScene(const SAddParticlesToSceneJob* jobs, size_t numJobs, const SRenderingPassInfo& passInfo) = 0;
	virtual void           GetMemoryUsageParticleREs(IDrxSizer* pSizer) {}

	/////////////////////////////////////////////////////////////////////////////////
	// Shaders/Shaders management /////////////////////////////////////////////////////////////////////////////////

	virtual void        EF_SetShaderMissCallback(ShaderCacheMissCallback callback) = 0;
	virtual tukk EF_GetShaderMissLogPath() = 0;

	/////////////////////////////////////////////////////////////////////////////////
	virtual string* EF_GetShaderNames(i32& nNumShaders) = 0;

	//! Reloads file
	virtual bool EF_ReloadFile(tukk szFileName) = 0;

	//! Reloads file at any time the renderer feels to do so (no guarantees, but likely on next frame update).
	//! \note This function is is thread safe.
	virtual bool                   EF_ReloadFile_Request(tukk szFileName) = 0;

	virtual _smart_ptr<IImageFile> EF_LoadImage(tukk szFileName, u32 nFlags) = 0;

	//! Remaps shader gen mask to common global mask.
	virtual uint64                EF_GetRemapedShaderMaskGen(tukk name, uint64 nMaskGen = 0, bool bFixup = 0) = 0;

	virtual uint64                EF_GetShaderGlobalMaskGenFromString(tukk szShaderName, tukk szShaderGen, uint64 nMaskGen = 0) = 0;
	virtual tukk           EF_GetStringFromShaderGlobalMaskGen(tukk szShaderName, uint64 nMaskGen = 0) = 0;

	virtual const SShaderProfile& GetShaderProfile(EShaderType eST) const = 0;
	virtual void                  EF_SetShaderQuality(EShaderType eST, EShaderQuality eSQ) = 0;

	//! Submit window texture info.
	virtual void EF_SubmitWind(const SWindGrid* pWind) = 0;

	//! Get renderer quality.
	virtual ERenderQuality EF_GetRenderQuality() const = 0;

	//! Get shader type quality.
	virtual EShaderQuality EF_GetShaderQuality(EShaderType eST) = 0;

	//! Load shader item for name (name).
	struct SLoadShaderItemArgs
	{
		SLoadShaderItemArgs(IMaterial* pMtlSrc, IMaterial* pMtlSrcParent) : m_pMtlSrc(pMtlSrc), m_pMtlSrcParent(pMtlSrcParent) {}
		IMaterial* m_pMtlSrc;
		IMaterial* m_pMtlSrcParent;
	};
	virtual SShaderItem EF_LoadShaderItem(tukk szName, bool bShare, i32 flags = 0, SInputShaderResources* Res = NULL, uint64 nMaskGen = 0, const SLoadShaderItemArgs* pArgs = 0) = 0;

	//! Load shader for name (name).
	virtual IShader* EF_LoadShader(tukk name, i32 flags = 0, uint64 nMaskGen = 0) = 0;

	//! Reinitialize all shader files (build hash tables).
	virtual void EF_ReloadShaderFiles(i32 nCategory) = 0;

	//! Reload all texture files.
	virtual void EF_ReloadTextures() = 0;

	//! Get texture object by ID.
	virtual ITexture* EF_GetTextureByID(i32 Id) = 0;

	//! Get texture object by Name.
	virtual ITexture* EF_GetTextureByName(tukk name, u32 flags = 0) = 0;

	//! Load the texture for name(nameTex).
	virtual ITexture* EF_LoadTexture(tukk nameTex, u32k flags = 0) = 0;

	//! Load the texture for name(nameTex).
	virtual IDynTextureSource* EF_LoadDynTexture(tukk dynsourceName, bool sharedRT = false) = 0;

	//! Load lightmap for name.
	virtual i32  EF_LoadLightmap(tukk name) = 0;
	virtual DynArray<uint16_t> EF_RenderEnvironmentCubeHDR(size_t size, const Vec3& Pos) = 0;
	// Writes a TIF file to the system with the requested preset
	// Intended for use with EF_RenderEnvironmentCubeHDR
	virtual bool WriteTIFToDisk(ukk pData, i32 width, i32 height, i32 bytesPerChannel, i32 numChannels, bool bFloat, tukk szPreset, tukk szFileName) = 0;

	//! Stores GBuffers region to atlas textures.
	virtual bool StoreGBufferToAtlas(const RectI& rcDst, i32 nSrcWidth, i32 nSrcHeight, i32 nDstWidth, i32 nDstHeight, ITexture *pDataD, ITexture *pDataN) = 0;

	//! Create new RE (RenderElement) of type (edt).
	virtual CRenderElement*  EF_CreateRE(EDataType edt) = 0;

	//! Start using of the shaders (return first index for allow recursions).
	virtual void EF_StartEf(const SRenderingPassInfo& passInfo) = 0;

	//! Get permanent RenderObject
	virtual CRenderObject* EF_GetObject() = 0;

	//! Release permanent RenderObject
	virtual void EF_FreeObject(CRenderObject* pObj) = 0;

	//Get permanent RenderObject
	virtual CRenderObject* EF_DuplicateRO(CRenderObject* pObj, const SRenderingPassInfo& passInfo) = 0;

	//! Draw all shaded REs in the list
	virtual void EF_EndEf3D(i32k nFlags, i32k nPrecacheUpdateId, i32k nNearPrecacheUpdateId, const SRenderingPassInfo& passInfo) = 0;

	virtual void EF_InvokeShadowMapRenderJobs(const SRenderingPassInfo& passInfo, i32k nFlags) = 0;
	virtual IRenderView* GetNextAvailableShadowsView(IRenderView* pMainRenderView, ShadowMapFrustum* pOwnerFrustum) = 0;
	virtual void PrepareShadowFrustumForShadowPool(ShadowMapFrustum* pFrustum, u32 frameID, const SRenderLight& light, u32 *timeSlicedShadowsUpdated) = 0;
	virtual void PrepareShadowPool(CRenderView* pRenderView) const = 0;

	// Dynamic lights.
	virtual bool EF_IsFakeDLight(const SRenderLight* Source) = 0;
	virtual void EF_ADDDlight(SRenderLight* Source, const SRenderingPassInfo& passInfo) = 0;
	virtual bool EF_UpdateDLight(SRenderLight* pDL) = 0;
	virtual bool EF_AddDeferredDecal(const SDeferredDecal& rDecal, const SRenderingPassInfo& passInfo) { return true; }

	// Deferred lights/vis areas

	virtual i32 EF_AddDeferredLight(const SRenderLight& pLight, float fMult, const SRenderingPassInfo& passInfo) = 0;

	// Deferred clip volumes
	virtual void Ef_AddDeferredGIClipVolume(const IRenderMesh* pClipVolume, const Matrix34& mxTransform) = 0;

	//! Called in between levels to free up memory.
	virtual void EF_ReleaseDeferredData() = 0;

	//! Called in between levels to free up memory.
	virtual SInputShaderResources* EF_CreateInputShaderResource(IRenderShaderResources* pOptionalCopyFrom = 0) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Post-processing effects interfaces.

	virtual void  EF_SetPostEffectParam(tukk pParam, float fValue, bool bForceValue = false) = 0;
	virtual void  EF_SetPostEffectParamVec4(tukk pParam, const Vec4& pValue, bool bForceValue = false) = 0;
	virtual void  EF_SetPostEffectParamString(tukk pParam, tukk pszArg) = 0;

	virtual void  EF_GetPostEffectParam(tukk pParam, float& fValue) = 0;
	virtual void  EF_GetPostEffectParamVec4(tukk pParam, Vec4& pValue) = 0;
	virtual void  EF_GetPostEffectParamString(tukk pParam, tukk & pszArg) = 0;

	virtual i32 EF_GetPostEffectID(tukk pPostEffectName) = 0;

	virtual void  EF_ResetPostEffects(bool bOnSpecChange = false) = 0;

	virtual void  EF_DisableTemporalEffects() = 0;

	/////////////////////////////////////////////////////////////////////////////////
	// 2D interface for the shaders.
	/////////////////////////////////////////////////////////////////////////////////
	virtual void EF_EndEf2D(const bool bSort) = 0;

	//! Returns various Renderer Settings, see ERenderQueryTypes.
	//! \param Query e.g. EFQ_GetShaderCombinations.
	//! \param rInOut Input/Output Parameter, depends on the query if written to/read from, or both.
	void EF_Query(ERenderQueryTypes eQuery)
	{
		EF_QueryImpl(eQuery, NULL, 0, NULL, 0);
	}
	template<typename T>
	void EF_Query(ERenderQueryTypes eQuery, T& rInOut)
	{
		EF_QueryImpl(eQuery, static_cast<uk>(&rInOut), sizeof(T), NULL, 0);
	}
	template<typename T0, typename T1>
	void EF_Query(ERenderQueryTypes eQuery, T0& rInOut0, T1& rInOut1)
	{
		EF_QueryImpl(eQuery, static_cast<uk>(&rInOut0), sizeof(T0), static_cast<uk>(&rInOut1), sizeof(T1));
	}

	//! Toggles render mesh garbage collection.
	virtual void ForceGC() = 0;

	//! \note For stats.
	virtual i32  GetPolyCount() = 0;
	virtual void GetPolyCount(i32& nPolygons, i32& nShadowVolPolys) = 0;

	//! Creates/deletes RenderMesh object.
	virtual _smart_ptr<IRenderMesh> CreateRenderMesh(
	  tukk szType
	  , tukk szSourceName
	  , IRenderMesh::SInitParamerers* pInitParams = NULL
	  , ERenderMeshType eBufType = eRMT_Static
	  ) = 0;

	virtual _smart_ptr<IRenderMesh> CreateRenderMeshInitialized(
	  ukk pVertBuffer, i32 nVertCount, InputLayoutHandle eVF,
	  const vtx_idx* pIndices, i32 nIndices,
	  const PublicRenderPrimitiveType nPrimetiveType, tukk szType, tukk szSourceName, ERenderMeshType eBufType = eRMT_Static,
	  i32 nMatInfoCount = 1, i32 nClientTextureBindID = 0,
	  bool (* PrepareBufferCallback)(IRenderMesh*, bool) = NULL,
	  uk CustomData = NULL,
	  bool bOnlyVideoBuffer = false,
	  bool bPrecache = true,
	  const SPipTangents* pTangents = NULL, bool bLockForThreadAcc = false, Vec3* pNormals = NULL) = 0;

	virtual i32  GetFrameID(bool bIncludeRecursiveCalls = true) = 0;

	virtual void MakeMatrix(const Vec3& pos, const Vec3& angles, const Vec3& scale, Matrix34* mat) = 0;

	//////////////////////////////////////////////////////////////////////

	virtual float ScaleCoordX(float value) const = 0;
	virtual float ScaleCoordY(float value) const = 0;
	virtual void  ScaleCoord(float& x, float& y) const = 0;

	virtual void  PushProfileMarker(tukk label) = 0;
	virtual void  PopProfileMarker(tukk label) = 0;

	//! For one frame allows disabling limit of texture streaming requests.
	virtual void RequestFlushAllPendingTextureStreamingJobs(i32 nFrames) {}

	//! Allow dynamic adjustment texture streaming load depending on game conditions.
	virtual void SetTexturesStreamingGlobalMipFactor(float fFactor) {}

	//////////////////////////////////////////////////////////////////////
	//! Interface for auxiliary geometry (for debugging, editor purposes, etc.)
	virtual IRenderAuxGeom* GetIRenderAuxGeom() = 0;
	virtual IRenderAuxGeom* GetOrCreateIRenderAuxGeom(const CCamera* pCustomCamera = nullptr) = 0;
	virtual void            DeleteAuxGeom(IRenderAuxGeom* pRenderAuxGeom) = 0;
	virtual void            SubmitAuxGeom(IRenderAuxGeom* pRenderAuxGeom, bool merge = true) = 0;
	virtual void            UpdateAuxDefaultCamera(const CCamera& systemCamera) = 0;
	//////////////////////////////////////////////////////////////////////

	//! Interface for renderer side SVO.
	virtual ISvoRenderer*            GetISvoRenderer() { return 0; }

	virtual IColorGradingController* GetIColorGradingController() = 0;
	virtual IStereoRenderer*         GetIStereoRenderer() const = 0;

	virtual void                     Graph(byte* g, i32 x, i32 y, i32 wdt, i32 hgt, i32 nC, i32 type, tukk text, ColorF& color, float fScale) = 0;

	// NB: The following functions will be removed.
	virtual void         EnableVSync(bool enable) = 0;

	virtual u32 UploadToVideoMemory(u8* data, i32 w, i32 h, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) = 0;
	virtual u32 UploadToVideoMemory3D(u8* data, i32 w, i32 h, i32 d, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) = 0;
	virtual u32 UploadToVideoMemoryCube(u8* data, i32 w, i32 h, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) = 0;
	virtual void         UpdateTextureInVideoMemory(u32 tnum, u8* newdata, i32 posx, i32 posy, i32 w, i32 h, ETEX_Format eTFSrc = eTF_B8G8R8, i32 posz = 0, i32 sizez = 1) = 0;

	// NB: Without header.
	virtual bool DXTCompress(byte* raw_data, i32 nWidth, i32 nHeight, ETEX_Format eTF, bool bUseHW, bool bGenMips, i32 nSrcBytesPerPix, MIPDXTcallback callback) = 0;
	virtual bool DXTDecompress(byte* srcData, const size_t srcFileSize, byte* dstData, i32 nWidth, i32 nHeight, i32 nMips, ETEX_Format eSrcTF, bool bUseHW, i32 nDstBytesPerPix) = 0;
	virtual void RemoveTexture(u32 TextureId) = 0;

	virtual bool BakeMesh(const SMeshBakingInputParams* pInputParams, SMeshBakingOutput* pReturnValues) = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Continuous frame capture interface
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//! This routines uses 2 destination surfaces.  It triggers a backbuffer copy to one of its surfaces, and then copies the other surface to system memory.
	//! This hopefully will remove any CPU stalls due to the rect lock call since the buffer will already be in system memory when it is called.
	//! \param pDstARGBA8			Pointer to a buffer that will hold the captured frame (should be at least 4*dstWidth*dstHieght for RGBA surface).
	//! \param destinationWidth	Width of the frame to copy.
	//! \param destinationHeight	Height of the frame to copy.
	//! \note If dstWidth or dstHeight is larger than the current surface dimensions, the dimensions of the surface are used for the copy.
	virtual bool CaptureFrameBufferFast(u8* pDstRGBA8, i32 destinationWidth, i32 destinationHeight) = 0;

	//! Copy a captured surface to a buffer.
	//! \param pDstARGBA8			Pointer to a buffer that will hold the captured frame (should be at least 4*dstWidth*dstHieght for RGBA surface).
	//! \param destinationWidth	Width of the frame to copy.
	//! \param destinationHeight	Height of the frame to copy.
	//! \note If dstWidth or dstHeight is larger than the current surface dimensions, the dimensions of the surface are used for the copy.
	virtual bool CopyFrameBufferFast(u8* pDstRGBA8, i32 destinationWidth, i32 destinationHeight) = 0;

	//! This routine registers a callback address that is called when a new frame is available.
	//! \param pCapture			Address of the ICaptureFrameListener object.
	//! \return true if successful, otherwise false.
	virtual bool RegisterCaptureFrame(ICaptureFrameListener* pCapture) = 0;

	//! This routine unregisters a callback address that was previously registered.
	//! \param pCapture			Address of the ICaptureFrameListener object to unregister.
	//! \returns true if successful, otherwise false.
	virtual bool UnRegisterCaptureFrame(ICaptureFrameListener* pCapture) = 0;

	//! This routine initializes 2 destination surfaces for use by the CaptureFrameBufferFast routine.
	//! It also captures the current backbuffer into one of the created surfaces.
	//! \param bufferWidth	Width of capture buffer, on consoles the scaling is done on the GPU. Pass in 0 (the default) to use backbuffer dimensions.
	//! \param bufferHeight	Height of capture buffer.
	//! \return true if surfaces were created otherwise returns false.
	virtual bool InitCaptureFrameBufferFast(u32 bufferWidth = 0, u32 bufferHeight = 0) = 0;

	//! This routine releases the 2 surfaces used for frame capture by the CaptureFrameBufferFast routine.
	virtual void CloseCaptureFrameBufferFast(void) = 0;

	//! This routine checks for any frame buffer callbacks that are needed and calls them.
	virtual void                CaptureFrameBufferCallBack(void) = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Single frame capture interface
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Take a screenshot and save it to a file
	//! \return true on success
	virtual bool ScreenShot(tukk filename = nullptr, const SDisplayContextKey& displayContextKey = {}) = 0;

	//! Copy frame buffer into destination memory buffer.
	virtual bool ReadFrameBuffer(u32* pDstRGBA8, i32 destinationWidth, i32 destinationHeight, bool readPresentedBackBuffer = true, EReadTextureFormat format = EReadTextureFormat::RGB8) = 0;

	virtual gpu_pfx2::IUpr* GetGpuParticleUpr() = 0;

	virtual void                RegisterSyncWithMainListener(ISyncMainWithRenderListener* pListener) = 0;
	virtual void                RemoveSyncWithMainListener(const ISyncMainWithRenderListener* pListener) = 0;

	virtual void                Set2DMode(bool enable, i32 ortox, i32 ortoy, float znear = -1e10f, float zfar = 1e10f) = 0;

	virtual void                EnableSwapBuffers(bool bEnable) = 0;

	//! Stop renderer at the end of the frame.
	//! E.g. ensures the renderer is not holding a lock when crash handler is trying to take a screenshot.
	virtual bool StopRendererAtFrameEnd(uint timeoutMilliseconds) = 0;

	//! Resume renderer that was stopped at the end of the frame with StopRendererAtFrameEnd().
	virtual void     ResumeRendererFromFrameEnd() = 0;

	virtual WIN_HWND GetHWND() = 0;

	//! Set the window icon to be displayed on the output window.
	//! The parameter is the path to a DDS texture file to be used as the icon.
	//! For best results, pass a square power-of-two sized texture, with a mip-chain.
	virtual bool                SetWindowIcon(tukk path) = 0;

	virtual void                OnEntityDeleted(struct IRenderNode* pRenderNode) = 0;

	virtual float               EF_GetWaterZElevation(float fX, float fY) = 0;

	virtual IOpticsElementBase* CreateOptics(EFlareType type) const = 0;

	//! Used for pausing timer related stuff.
	//! Example: For texture animations, and shader 'time' parameter.
	virtual void PauseTimer(bool bPause) = 0;

	//! Creates an Interface to the public params container.
	//! \return Created IShaderPublicParams interface.
	virtual IShaderPublicParams* CreateShaderPublicParams() = 0;

	virtual void                 SetLevelLoadingThreadId(threadID threadId) = 0;

	virtual void                 GetThreadIDs(threadID& mainThreadID, threadID& renderThreadID) const = 0;

	struct SUpdateRect
	{
		u32 dstX, dstY;
		u32 srcX, srcY;
		u32 width, height;

		void   Set(u32 dx, u32 dy, u32 sx, u32 sy, u32 w, u32 h)
		{
			dstX = dx;
			dstY = dy;
			srcX = sx;
			srcY = sy;
			width = w;
			height = h;
		}
	};

	virtual IScaleformPlayback*                         SF_CreatePlayback() const = 0;
	virtual i32                                         SF_CreateTexture(i32 width, i32 height, i32 numMips, u8k* pData, ETEX_Format eTF, i32 flags) = 0;
	virtual bool                                        SF_UpdateTexture(i32 texId, i32 mipLevel, i32 numRects, const SUpdateRect* pRects, u8k* pData, size_t pitch, size_t size, ETEX_Format eTF) = 0;
	virtual bool                                        SF_ClearTexture(i32 texId, i32 mipLevel, i32 numRects, const SUpdateRect* pRects, u8k* pData) = 0;
	virtual void                                        SF_Playback(IScaleformPlayback* pRenderer, GRendererCommandBufferReadOnly* pBuffer) const = 0;
	virtual void                                        SF_Drain(GRendererCommandBufferReadOnly* pBuffer) const = 0;
	virtual void                                        SF_GetMeshMaxSize(i32& numVertices, i32& numIndices) const = 0;

	virtual ITexture*                                   CreateTexture(tukk name, i32 width, i32 height, i32 numMips, u8* pData, ETEX_Format eTF, i32 flags) = 0;
	virtual ITexture*                                   CreateTextureArray(tukk name, ETEX_Type eType, u32 nWidth, u32 nHeight, u32 nArraySize, i32 nMips, u32 nFlags, ETEX_Format eTF, i32 nCustomID) = 0;

	virtual const RPProfilerStats*                      GetRPPStats(ERenderPipelineProfilerStats eStat, bool bCalledFromMainThread = true) = 0;
	virtual const RPProfilerStats*                      GetRPPStatsArray(bool bCalledFromMainThread = true) = 0;
	virtual const DynArray<RPProfilerDetailedStats>*    GetRPPDetailedStatsArray(bool bCalledFromMainThread = true) = 0;

	virtual i32                                         GetPolygonCountByType(u32 EFSList, EVertexCostTypes vct, u32 z, bool bCalledFromMainThread = true) = 0;

	virtual void                                        StartLoadtimeFlashPlayback(ILoadtimeCallback* pCallback) = 0;
	virtual void                                        StopLoadtimeFlashPlayback() = 0;
				                                        
	virtual void                                        SetCloudShadowsParams(i32 nTexID, const Vec3& speed, float tiling, bool invert, float brightness) = 0;
	virtual void                                        SetVolumetricCloudParams(i32 nTexID) = 0;
	virtual void                                        SetVolumetricCloudNoiseTex(i32 cloudNoiseTexId, i32 edgeNoiseTexId) = 0;

	virtual i32                                         GetMaxTextureSize() = 0;

	virtual tukk                                 GetTextureFormatName(ETEX_Format eTF) = 0;
	virtual i32                                         GetTextureFormatDataSize(i32 nWidth, i32 nHeight, i32 nDepth, i32 nMips, ETEX_Format eTF) = 0;
	virtual bool                                        IsTextureFormatSupported(ETEX_Format eTF) = 0;

	virtual void                                        SetDefaultMaterials(IMaterial* pDefMat, IMaterial* pTerrainDefMat) = 0;

	virtual u32                                      GetActiveGPUCount() const = 0;
	virtual ShadowFrustumMGPUCache*                     GetShadowFrustumMGPUCache() = 0;
	virtual const StaticArray<i32, MAX_GSM_LODS_NUM>&   GetCachedShadowsResolution() const = 0;
	virtual void                                        SetCachedShadowsResolution(const StaticArray<i32, MAX_GSM_LODS_NUM>& arrResolutions) = 0;
	virtual void                                        UpdateCachedShadowsLodCount(i32 nGsmLods) const = 0;
				                                        
	virtual void                                        SetTexturePrecaching(bool stat) = 0;

	//! Platform-specific.
	virtual void RT_InsertGpuCallback(u32 context, GpuCallbackFunc callback) = 0;
	virtual void EnablePipelineProfiler(bool bEnable) = 0;

	struct SGpuInfo
	{
		tukk name;
		u32 nNodeCount;
		UINT VendorId;
		UINT DeviceId;
		UINT SubSysId;
		UINT Revision;
	};

	virtual void QueryActiveGpuInfo(SGpuInfo& info) const = 0;

	struct SRenderTimes
	{
		float fWaitForMain;
		float fWaitForRender;
		float fWaitForGPU;
		float fTimeProcessedRT;
		float fTimeProcessedRTScene;  //!< The part of the render thread between the "SCENE" profiler labels.
		float fTimeProcessedGPU;
		float fTimeGPUIdlePercent;
	};
	virtual void  GetRenderTimes(SRenderTimes& outTimes) = 0;
	virtual float GetGPUFrameTime() = 0;

	//! Enable the batch mode if the meshpools are used to enable quick and dirty flushes.
	virtual void EnableBatchMode(bool enable) = 0;

	//! Flag level unloading in progress to disable f.i. rendermesh creation requests.
	virtual void EnableLevelUnloading(bool enable) = 0;

	struct SDrawCallCountInfo
	{
		static u32k MESH_NAME_LENGTH = 32;
		static u32k TYPE_NAME_LENGTH = 16;

		SDrawCallCountInfo() : pPos(0, 0, 0), nZpass(0), nShadows(0), nGeneral(0), nTransparent(0), nMisc(0)
		{
			meshName[0] = '\0';
			typeName[0] = '\0';
		}

#if defined(DO_RENDERSTATS)
		void Update(CRenderObject* pObj, IRenderMesh* pRM, EShaderTechniqueID techniqueID);
#endif

		Vec3  pPos;
		u8 nZpass, nShadows, nGeneral, nTransparent, nMisc;
		char  meshName[MESH_NAME_LENGTH];
		char  typeName[TYPE_NAME_LENGTH];
	};

	//! Debug draw call info (per mesh).
	typedef std::map<IRenderMesh*, IRenderer::SDrawCallCountInfo> RNDrawcallsMapMesh;

#if !defined(_RELEASE)
	//! Get draw call info for frame.
	virtual RNDrawcallsMapMesh& GetDrawCallsInfoPerMesh(bool mainThread = true) = 0;
	virtual i32                 GetDrawCallsPerNode(IRenderNode* pRenderNode) = 0;
	virtual void                ForceRemoveNodeFromDrawCallsMap(IRenderNode* pNode) = 0;
#endif

	virtual void CollectDrawCallsInfo(bool status) = 0;
	virtual void CollectDrawCallsInfoPerNode(bool status) = 0;

	// Summary:
	virtual SSkinningData* EF_CreateSkinningData(IRenderView* pRenderView, u32 nNumBones, bool bNeedJobSyncVar) = 0;
	virtual SSkinningData* EF_CreateRemappedSkinningData(IRenderView* pRenderView, u32 nNumBones, SSkinningData* pSourceSkinningData, u32 nCustomDataSize, u32 pairGuid) = 0;
	virtual void           EF_EnqueueComputeSkinningData(IRenderView* pRenderView, SSkinningData* pData) = 0;
	virtual i32            EF_GetSkinningPoolID() = 0;

	virtual void           UpdateShaderItem(SShaderItem* pShaderItem, IMaterial* pMaterial) = 0;
	virtual void           ForceUpdateShaderItem(SShaderItem* pShaderItem, IMaterial* pMaterial) = 0;
	virtual void           RefreshShaderResourceConstants(SShaderItem* pShaderItem, IMaterial* pMaterial) = 0;

	//! Used for editor highlights, sets the highlight color
	virtual void           SetHighlightColor(ColorF color) = 0;
	//! Used for editor highlights, sets the selection color
	virtual void           SetSelectionColor(ColorF color) = 0;
	//! Used for editor highlights, sets outline thickness and ghost alpha parameters for the effect
	virtual void           SetHighlightParams(float outlineThickness, float fGhostAlpha) = 0;

	//! Determine if a switch to stereo mode will occur at the start of the next frame.
	virtual bool IsStereoModeChangePending() = 0;

	//! Wait for all particle ComputeVertices jobs to finish.
	virtual void SyncComputeVerticesJobs() = 0;

	//! Lock/Unlock the video memory buffer used by particles when using the jobsystem.
	virtual void  LockParticleVideoMemory(i32 frameId) = 0;
	virtual void  UnLockParticleVideoMemory(i32 frameId) = 0;

	virtual void  ActivateLayer(tukk pLayerName, bool activate) = 0;

	virtual void  FlushPendingTextureTasks() = 0;
	virtual void  FlushPendingUploads() = 0;

	virtual void  SetCloakParams(const SRendererCloakParams& cloakParams) = 0;
	virtual float GetCloakFadeLightScale() const = 0;
	virtual void  SetCloakFadeLightScale(float fColorScale) = 0;
	virtual void  SetShadowJittering(float fShadowJittering) = 0;
	virtual float GetShadowJittering() const = 0;

	virtual bool  LoadShaderStartupCache() = 0;
	virtual void  UnloadShaderStartupCache() = 0;

	//! Copy a region from source texture to destination texture and apply a tint color. If blend is false, the source will override the destination area. Otherwise it will perform an alpha blend.
	virtual void  CopyTextureRegion(ITexture* pSrc, RectI srcRegion, ITexture* pDst, RectI dstRegion, ColorF& color, i32k renderStateFlags) = 0;

	virtual bool  LoadShaderLevelCache() = 0;
	virtual void  UnloadShaderLevelCache() = 0;

	virtual void  SetRendererCVar(ICVar* pCVar, tukk pArgText, const bool bSilentMode = false) = 0;

#ifdef SUPPORT_HW_MOUSE_CURSOR
	virtual IHWMouseCursor* GetIHWMouseCursor() = 0;
#endif

	// Creates a constant buffer on the device.
	virtual IGraphicsDeviceConstantBufferPtr CreateGraphiceDeviceConstantBuffer() = 0;

private:
	//! Use private for EF_Query to prevent client code to submit arbitary combinations of output data/size.
	virtual void EF_QueryImpl(ERenderQueryTypes eQuery, uk pInOut0, u32 nInOutSize0, uk pInOut1, u32 nInOutSize1) = 0;
};

//! Util class to change wireframe mode.
//! \cond INTERNAL
class CScopedWireFrameMode
{
public:
	CScopedWireFrameMode(IRenderer* pRenderer, i32 nMode) : m_nMode(nMode)
	{
	}
	~CScopedWireFrameMode()
	{
	}
private:
	i32        m_nMode;
};

struct SShaderCacheStatistics
{
	size_t m_nTotalLevelShaderCacheMisses;
	size_t m_nGlobalShaderCacheMisses;
	size_t m_nNumShaderAsyncCompiles;
	bool   m_bShaderCompileActive;

	SShaderCacheStatistics() : m_nTotalLevelShaderCacheMisses(0),
		m_nGlobalShaderCacheMisses(0),
		m_nNumShaderAsyncCompiles(0), m_bShaderCompileActive(false)
	{}
};

//! The statistics about the pool for render mesh data.
struct SMeshPoolStatistics
{
	//! The size of the mesh data size in bytes.
	size_t nPoolSize;

	//! The amount of memory currently in use in the pool.
	size_t nPoolInUse;

	//! The highest amount of memory allocated within the mesh data pool.
	size_t nPoolInUsePeak;

	//! The size of the mesh data size in bytes.
	size_t nInstancePoolSize;

	//! The amount of memory currently in use in the pool.
	size_t nInstancePoolInUse;

	//! The highest amount of memory allocated within the mesh data pool.
	size_t nInstancePoolInUsePeak;

	size_t nFallbacks;
	size_t nInstanceFallbacks;
	size_t nFlushes;

	SMeshPoolStatistics()
		: nPoolSize(),
		nPoolInUse(),
		nInstancePoolSize(),
		nInstancePoolInUse(),
		nInstancePoolInUsePeak(),
		nFallbacks(),
		nInstanceFallbacks(),
		nFlushes()
	{}
};

struct SRendererQueryGetAllTexturesParam
{
	SRendererQueryGetAllTexturesParam()
		: pTextures(NULL)
		, numTextures(0)
	{
	}

	_smart_ptr<ITexture>* pTextures;
	u32                numTextures;
};
//! \endcond

//////////////////////////////////////////////////////////////////////

#define STRIPTYPE_NONE           0
#define STRIPTYPE_ONLYLISTS      1
#define STRIPTYPE_SINGLESTRIP    2
#define STRIPTYPE_MULTIPLESTRIPS 3
#define STRIPTYPE_DEFAULT        4

/////////////////////////////////////////////////////////////////////

struct IRenderMesh;

#include "VertexFormats.h"

struct SRestLightingInfo
{
	SRestLightingInfo()
	{
		averDir.zero();
		averCol = Col_Black;
		refPoint.zero();
	}
	Vec3   averDir;
	ColorF averCol;
	Vec3   refPoint;
};

class CLodValue
{
public:
	CLodValue()
	{
		m_nLodA = -1;
		m_nLodB = -1;
		m_nDissolveRef = 0;
	}

	CLodValue(i32 nLodA)
	{
		m_nLodA = nLodA;
		m_nLodB = -1;
		m_nDissolveRef = 0;
	}

	CLodValue(i32 nLodA, u8 nDissolveRef, i32 nLodB)
	{
		m_nLodA = nLodA;
		m_nLodB = nLodB;
		m_nDissolveRef = nDissolveRef;
	}

	i32  LodA() const         { return m_nLodA; }
	i32  LodSafeA() const     { return m_nLodA == -1 ? 0 : m_nLodA; }
	i32  LodB() const         { return m_nLodB; }

	uint DissolveRefA() const { return m_nDissolveRef; }
	uint DissolveRefB() const { return 255 - m_nDissolveRef; }

private:
	i16 m_nLodA;
	i16 m_nLodB;
	u8 m_nDissolveRef;
};

//! \cond INTERNAL
//! Structure used to pass render parameters to Render() functions of IStatObj and ICharInstance.
struct SRendParams
{
	SRendParams()
	{
		memset(this, 0, sizeof(SRendParams));
		fAlpha = 1.f;
		nAfterWater = 1;
	}

	// object transformations.
	Matrix34*                 pMatrix;

	Matrix34*                 pPrevMatrix; //!< object previous transformations - motion blur specific.

	Matrix34*				  pNearestMatrix; //!< object transformations - nearest camera specific.

	IVisArea*                 m_pVisArea; //!< VisArea that contains this object, used for RAM-ambient cube query.

	IMaterial*                pMaterial; //!< Override material.

	IFoliage*                 pFoliage; //!< Skeleton implementation for bendable foliage.

	struct IRenderNode*       pRenderNode; //!< Object Id for objects identification in renderer.

	uk                     pInstance; //!< Unique object Id for objects identification in renderer.

	struct SSectorTextureSet* pTerrainTexInfo;   //!< TerrainTexInfo for grass.

	DynArray<SShaderParam>*   pShaderParams; //!< dynamic render data object which can be set by the game

	ColorF                    AmbientColor; //!< Ambient color for the object.

	float                     fAlpha; //!< Object alpha.

	float                     fDistance; //!< Distance from camera.

	uint64                    dwFObjFlags; //!< Approximate information about the lights not included into nDLightMask.

	u32                    nMaterialLayersBlend; //!< Material layers blending amount

	u32                    nVisionParams; //!< Vision modes params

	u32                    nHUDSilhouettesParams; //!< Vision modes params

	u32                    pLayerEffectParams; //!< Layer effects.

	hidemask                  nSubObjHideMask; //!< Defines what peaces of pre-broken geometry has to be rendered.

	float                     fCustomData[4]; //!< Defines per object float custom data.

	i16                     nTextureID; //!< Custom TextureID.

	u16                    nCustomFlags; //!< Defines per object custom flags.

	CLodValue                 lodValue; //!< The LOD value compute for rendering.

	u8                     nCustomData; //!< Defines per object custom data.

	u8                     nDissolveRef; //!< Defines per object DissolveRef value if used by shader.

	u8                     nClipVolumeStencilRef; //!< Per-instance vis area stencil ref id.

	u8                     nAfterWater; //!< Custom offset for sorting by distance.

	u8                     nMaterialLayers; //!< Material layers bitmask -> which material layers are active.

	u32                    nEditorSelectionID; //!< Selection ID and information for editor
};
//! \endcond

struct SRendererCloakParams
{
	SRendererCloakParams()
	{
		memset(this, 0, sizeof(SRendererCloakParams));
	}

	float fCloakLightScale;
	float fCloakTransitionLightScale;
	i32   cloakFadeByDist;
	float fCloakFadeLightScale;
	float fCloakFadeMinDistSq;
	float fCloakFadeMaxDistSq;
	float fCloakFadeMinValue;
	i32   cloakRefractionFadeByDist;
	float fCloakRefractionFadeMinDistSq;
	float fCloakRefractionFadeMaxDistSq;
	float fCloakRefractionFadeMinValue;
	float fCloakMinLightValue;
	float fCloakHeatScale;
	i32   cloakRenderInThermalVision;
	float fCloakMinAmbientOutdoors;
	float fCloakMinAmbientIndoors;
	float fCloakSparksAlpha;
	float fCloakInterferenceSparksAlpha;
	float fCloakHighlightStrength;
	float fThermalVisionViewDistance;
	float fArmourPulseSpeedMultiplier;
	float fMaxSuitPulseSpeedMultiplier;
};