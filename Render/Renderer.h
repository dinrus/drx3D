// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/Pool/PoolAlloc.h>
#include <drx3D/Plugins/concqueue/concqueue.hpp>
#include <drx3D/Render/TextMessages.h>                             // CTextMessages
#include <drx3D/Render/RenderAuxGeom.h>
#include <drx3D/Render/RenderPipeline.h>
#include <drx3D/Render/RenderThread.h>                             // SRenderThread
#include <drx3D/Render/Scaleform/ScaleformRender.h>
#include <drx3D/Render/D3D/DeviceUpr/D3D11/DeviceSubmissionQueue_D3D11.h> // CSubmissionQueue_DX11
#include <drx3D/Render/ElementPool.h>

typedef void (PROCRENDEF)(SShaderPass* l, i32 nPrimType);

#define USE_NATIVE_DEPTH 1

enum eAntialiasingType
{
	eAT_NOAA = 0,
	eAT_SMAA_1X,
	eAT_SMAA_1TX,
	eAT_SMAA_2TX,
	eAT_TSAA,
	eAT_AAMODES_COUNT,

	eAT_DEFAULT_AA                  = eAT_SMAA_1TX,

	eAT_NOAA_MASK                   = (1 << eAT_NOAA),
	eAT_SMAA_1X_MASK                = (1 << eAT_SMAA_1X),
	eAT_SMAA_1TX_MASK               = (1 << eAT_SMAA_1TX),
	eAT_SMAA_2TX_MASK               = (1 << eAT_SMAA_2TX),
	eAT_TSAA_MASK                   = (1 << eAT_TSAA),

	eAT_SMAA_MASK                   = (eAT_SMAA_1X_MASK | eAT_SMAA_1TX_MASK | eAT_SMAA_2TX_MASK),

	eAT_REQUIRES_PREVIOUSFRAME_MASK = (eAT_SMAA_1TX_MASK | eAT_SMAA_2TX_MASK | eAT_TSAA_MASK),
	eAT_REQUIRES_SUBPIXELSHIFT_MASK = (eAT_SMAA_2TX_MASK | eAT_TSAA_MASK)
};

static tukk s_pszAAModes[eAT_AAMODES_COUNT] =
{
	"NO AA",
	"SMAA 1X",
	"SMAA 1TX",
	"SMAA 2TX",
	"TSAA"
};

extern u32 ColorMasks[(ColorMask::GS_NOCOLMASK_COUNT >> GS_COLMASK_SHIFT)][4];
extern std::array<u32, (ColorMask::GS_NOCOLMASK_COUNT >> GS_COLMASK_SHIFT)> AvailableColorMasks;

struct ShadowMapFrustum;
struct IStatObj;
struct SShaderPass;
class CREParticle;
class CD3DStereoRenderer;
class CTextureUpr;
class CIntroMovieRenderer;
class IOpticsUpr;
struct SDynTexture2;
class CDeviceResourceSet;
class CVertexBuffer;
class CIndexBuffer;
class CStandardGraphicsPipeline;
class CBaseResource;

namespace compute_skinning {
struct IComputeSkinningStorage;
}

typedef i32 (* pDrawModelFunc)(void);

//=============================================================

#define D3DRGBA(r, g, b, a)                                \
  ((((i32)((a) * 255)) << 24) | (((i32)((r) * 255)) << 16) \
   | (((i32)((g) * 255)) << 8) | (i32)((b) * 255)          \
  )

struct alloc_info_struct
{
	i32         ptr;
	i32         bytes_num;
	bool        busy;
	tukk szSource;
	unsigned Size()                                  { return sizeof(*this); }

	void     GetMemoryUsage(IDrxSizer* pSizer) const {}
};

const float TANGENT30_2 = 0.57735026918962576450914878050196f * 2;   // 2*tan(30)

// Assuming 24 bits of depth precision
#define DBT_SKY_CULL_DEPTH                    0.99999994f

#define DEF_SHAD_DBT_DEFAULT_VAL              1

#define TEXSTREAMING_DEFAULT_VAL              2

#define GEOM_INSTANCING_DEFAULT_VAL           0
#define COLOR_GRADING_DEFAULT_VAL             1
#define SUNSHAFTS_DEFAULT_VAL                 2
#define HDR_RANGE_ADAPT_DEFAULT_VAL           0
#define HDR_RENDERING_DEFAULT_VAL             1
#define TEXPREALLOCATLAS_DEFAULT_VAL          0
#define TEXMAXANISOTROPY_DEFAULT_VAL          8
#if DRX_PLATFORM_DESKTOP
	#define TEXNOANISOALPHATEST_DEFAULT_VAL     0
	#define SHADERS_ALLOW_COMPILATION_DEFAULT_VAL 1
#else
	#define TEXNOANISOALPHATEST_DEFAULT_VAL     1
	#define SHADERS_ALLOW_COMPILATION_DEFAULT_VAL 0
#endif
#define ENVTEXRES_DEFAULT_VAL                 3
#define WATERREFLQUAL_DEFAULT_VAL             4
#define DOF_DEFAULT_VAL                       2
#define SHADERS_PREACTIVATE_DEFAULT_VAL       3
#define CUSTOMVISIONS_DEFAULT_VAL             3
#define FLARES_DEFAULT_VAL                    1
#define WATERVOLCAUSTICS_DEFAULT_VAL          1
#define FLARES_HQSHAFTS_DEFAULT_VAL           1
#define DEF_SHAD_DBT_STENCIL_DEFAULT_VAL      1
#define DEF_SHAD_SSS_DEFAULT_VAL              1

#define MULTITHREADED_DEFAULT_VAL             1
#define ZPASS_DEPTH_SORT_DEFAULT_VAL          1
#define TEXSTREAMING_UPDATETYPE_DEFAULT_VAL   1

#define MAX_PREDICTION_ZONES                  MAX_STREAM_PREDICTION_ZONES

#define MAX_SHADOWMAP_FRUSTUMS                1024
#define MAX_DEFERRED_LIGHTS                   256

#define TEMP_REND_OBJECTS_POOL                (2048)

#define MAX_REND_LIGHTS                       32

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#define CBUFFER_NATIVE_DEPTH_DEAFULT_VAL 1
#else
	#define CBUFFER_NATIVE_DEPTH_DEAFULT_VAL 0
#endif

#include <drx3D/Render/RendererResources.h>
#include <drx3D/Render/RendererCVars.h>                    // Can only be included after the default values are defined.

struct SSpriteInfo
{
	SDynTexture2*             m_pTex;
	struct SSectorTextureSet* m_pTerrainTexInfo;
	Vec3                      m_vPos;
	float                     m_fDX;
	float                     m_fDY;
	float                     m_fScaleV;
	UCol                      m_Color;
	i32                       m_nVI;
	u8                     m_ucTexCoordMinX; // 0..128 used for the full range (0..1) in the texture (to fit in byte)
	u8                     m_ucTexCoordMinY; // 0..128 used for the full range (0..1) in the texture (to fit in byte)
	u8                     m_ucTexCoordMaxX; // 0..128 used for the full range (0..1) in the texture (to fit in byte)
	u8                     m_ucTexCoordMaxY; // 0..128 used for the full range (0..1) in the texture (to fit in byte)
};

struct SSpriteGenInfo
{
	float          fAngle;                      // horizontal rotation in degree
	float          fGenDist;
	float          fBrightness;
	i32            nMaterialLayers;
	IMaterial*     pMaterial;
	float*         pMipFactor;
	u8*         pTexturesAreStreamedIn;
	SDynTexture2** ppTexture;
	IStatObj*      pStatObj;
	i32            nSP;
};

class CMatrixStack
{
public:
	CMatrixStack(u32 maxDepth, u32 dirtyFlag);
	~CMatrixStack();

	// Pops the top of the stack, returns the current top
	// *after* popping the top.
	bool Pop();

	// Pushes the stack by one, duplicating the current matrix.
	bool Push();

	// Loads identity in the current matrix.
	bool LoadIdentity();

	// Loads the given matrix into the current matrix
	bool LoadMatrix(const Matrix44* pMat);

	// Right-Multiplies the given matrix to the current matrix.
	// (transformation is about the current world origin)
	bool MultMatrix(const Matrix44* pMat);

	// Left-Multiplies the given matrix to the current matrix
	// (transformation is about the local origin of the object)
	bool MultMatrixLocal(const Matrix44* pMat);

	// Right multiply the current matrix with the computed rotation
	// matrix, counterclockwise about the given axis with the given angle.
	// (rotation is about the current world origin)
	bool RotateAxis(const Vec3* pV, f32 Angle);

	// Left multiply the current matrix with the computed rotation
	// matrix, counterclockwise about the given axis with the given angle.
	// (rotation is about the local origin of the object)
	bool RotateAxisLocal(const Vec3* pV, f32 Angle);

	// Right multiply the current matrix with the computed rotation
	// matrix. All angles are counterclockwise. (rotation is about the
	// current world origin)

	// Right multiply the current matrix with the computed scale
	// matrix. (transformation is about the current world origin)
	bool Scale(f32 x, f32 y, f32 z);

	// Left multiply the current matrix with the computed scale
	// matrix. (transformation is about the local origin of the object)
	bool ScaleLocal(f32 x, f32 y, f32 z);

	// Right multiply the current matrix with the computed translation
	// matrix. (transformation is about the current world origin)
	bool Translate(f32 x, f32 y, f32 z);

	// Left multiply the current matrix with the computed translation
	// matrix. (transformation is about the local origin of the object)
	bool TranslateLocal(f32 x, f32 y, f32 z);

	// Obtain the current matrix at the top of the stack
	inline Matrix44A* GetTop()
	{
		assert(m_pTop != NULL);
		return m_pTop;
	}

	inline i32 GetDepth() { return m_nDepth; }

private:
	Matrix44A* m_pTop;        //top of the stack
	Matrix44A* m_pStack;      // array of Matrix44
	u32     m_nDepth;
	u32     m_nMaxDepth;
	u32     m_nDirtyFlag; //flag for new matrices
};

//////////////////////////////////////////////////////////////////////
// Class to manage memory for Skinning Renderer Data
class CSkinningDataPool
{
public:
	CSkinningDataPool()
		: m_pPool(NULL)
		, m_pPages(NULL)
		, m_nPoolSize(0)
		, m_nPoolUsed(0)
		, m_nPageAllocated(0)
	{}

	~CSkinningDataPool()
	{
		// free temp pages
		SPage* pPage = m_pPages;
		while (pPage)
		{
			SPage* pPageToDelete = pPage;
			pPage = pPage->pNext;

			DrxModuleMemalignFree(pPageToDelete);
		}

		// free pool
		DrxModuleMemalignFree(m_pPool);
	}

	byte* Allocate(size_t nBytes)
	{
		// If available use allocated page space
		u32 nPoolUsed = ~0;
		do
		{

			nPoolUsed = *const_cast< size_t*>(&m_nPoolUsed);
			size_t nPoolFree = m_nPoolSize - nPoolUsed;
			if (nPoolFree < nBytes)
				break; // not enough memory, use fallback
			if (DrxInterlockedCompareExchange(alias_cast< LONG*>(&m_nPoolUsed), nPoolUsed + nBytes, nPoolUsed) == nPoolUsed)
				return &m_pPool[nPoolUsed];

		}
		while (true);

		// Create memory
		byte* pMemory = alias_cast<byte*>(DrxModuleMemalign(Align(nBytes, 16) + 16, 16));
		SPage* pNewPage = alias_cast<SPage*>(pMemory);

		// Assign page
		 SPage* pPages = 0;
		do
		{
			pPages = *(const_cast< SPage**>(&m_pPages));
			pNewPage->pNext = alias_cast<SPage*>(pPages);

		}
		while (DrxInterlockedCompareExchangePointer(alias_cast<uk *>(&m_pPages), pNewPage, (uk )pPages) != pPages);

		DrxInterlockedAdd(( i32*)&m_nPageAllocated, nBytes);

		return pMemory + 16;
	}

	void ClearPool()
	{
		m_nPoolUsed = 0;
		if (m_nPageAllocated)
		{
			// free temp pages
			SPage* pPage = m_pPages;
			while (pPage)
			{
				SPage* pPageToDelete = pPage;
				pPage = pPage->pNext;

				DrxModuleMemalignFree(pPageToDelete);
			}

			// adjust pool size
			DrxModuleMemalignFree(m_pPool);
			m_nPoolSize += m_nPageAllocated;
			m_pPool = alias_cast<byte*>(DrxModuleMemalign(m_nPoolSize, 16));

			// reset state
			m_pPages = NULL;
			m_nPageAllocated = 0;
		}
	}

	void FreePoolMemory()
	{
		// free temp pages
		SPage* pPage = m_pPages;
		while (pPage)
		{
			SPage* pPageToDelete = pPage;
			pPage = pPage->pNext;

			DrxModuleMemalignFree(pPageToDelete);
		}

		// free pool
		DrxModuleMemalignFree(m_pPool);

		m_pPool = NULL;
		m_pPages = NULL;
		m_nPoolSize = 0;
		m_nPoolUsed = 0;
		m_nPageAllocated = 0;
	}

	size_t AllocatedMemory()
	{
		return m_nPoolSize + m_nPageAllocated;
	}
private:
	struct SPage
	{
		SPage* pNext;
	};

	byte*  m_pPool;
	SPage* m_pPages;
	size_t m_nPoolSize;
	size_t m_nPoolUsed;
	size_t m_nPageAllocated;
};

struct SSkinningDataPoolInfo
{
	i32                          poolIndex;
	CSkinningDataPool*           pDataCurrentFrame;
	CSkinningDataPool*           pDataPreviousFrame;
	std::vector<SSkinningData*>* pDataComputeSkinning;
};

//////////////////////////////////////////////////////////////////////
class CFillRateUpr : private stl::PSyncMultiThread
{
public:
	CFillRateUpr()
		: m_fTotalPixels(0.f), m_fMaxPixels(1e9f)
	{}
	void Reset()
	{
		m_fTotalPixels = 0.f;
		m_afPixels.resize(0);
	}
	float GetMaxPixels() const
	{
		return m_fMaxPixels;
	}
	void AddPixelCount(float fPixels);
	void ComputeMaxPixels();

private:
	float               m_fTotalPixels;
	float               m_fMaxPixels;
	FastDynArray<float> m_afPixels;
};

//////////////////////////////////////////////////////////////////////
// 3D engine duplicated data for non-thread safe data
namespace N3DEngineCommon
{

struct SOceanInfo
{
	SOceanInfo() : m_nOceanRenderFlags(0), m_fWaterLevel(0.0f), m_vCausticsParams(0.0f, 0.0f, 0.0f, 0.0f), m_vMeshParams(0.0f, 0.0f, 0.0f, 0.0f) {};
	Vec4  m_vCausticsParams;
	Vec4  m_vMeshParams;
	float m_fWaterLevel;
	u8 m_nOceanRenderFlags;
};

struct SVisAreaInfo
{
	SVisAreaInfo() : nFlags(0)
	{
	};
	u32 nFlags;
};

struct SRainOccluder
{
	SRainOccluder() : m_RndMesh(0), m_WorldMat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0) {}
	_smart_ptr<IRenderMesh> m_RndMesh;
	Matrix34                m_WorldMat;
};

typedef std::vector<SRainOccluder> ArrOccluders;

struct SRainOccluders
{
	ArrOccluders m_arrOccluders;
	ArrOccluders m_arrCurrOccluders[RT_COMMAND_BUF_COUNT];
	size_t       m_nNumOccluders;
	bool         m_bProcessed[MAX_GPU_NUM];

	SRainOccluders() : m_nNumOccluders(0) { for (i32 i = 0; i < MAX_GPU_NUM; ++i) m_bProcessed[i] = true; }
	~SRainOccluders()                                                  { Release(); }
	void Release(bool bAll = false)
	{
		stl::free_container(m_arrOccluders);
		m_nNumOccluders = 0;
		if (bAll)
		{
			for (i32 i = 0; i < RT_COMMAND_BUF_COUNT; i++)
				stl::free_container(m_arrCurrOccluders[i]);
		}
		for (i32 i = 0; i < MAX_GPU_NUM; ++i) m_bProcessed[i] = true;
	}
};

struct SCausticInfo
{
	SCausticInfo() : m_pCausticQuadMesh(0), m_nCausticMeshWidth(0), m_nCausticMeshHeight(0), m_nCausticQuadTaps(0), m_nVertexCount(0), m_nIndexCount(0),
		m_mCausticMatr(IDENTITY), m_mCausticViewMatr(IDENTITY)
	{
	}

	~SCausticInfo() { Release(); }
	void Release()
	{
		m_pCausticQuadMesh = NULL;
	}

	_smart_ptr<IRenderMesh> m_pCausticQuadMesh;
	u32                  m_nCausticMeshWidth;
	u32                  m_nCausticMeshHeight;
	u32                  m_nCausticQuadTaps;
	u32                  m_nVertexCount;
	u32                  m_nIndexCount;

	Matrix44A               m_mCausticMatr;
	Matrix34                m_mCausticViewMatr;
};
}

struct S3DEngineCommon
{
	enum EVisAreaFlags
	{
		VAF_EXISTS_FOR_POSITION    = (1 << 0),
		VAF_CONNECTED_TO_OUTDOOR   = (1 << 1),
		VAF_AFFECTED_BY_OUT_LIGHTS = (1 << 2),
		VAF_MASK                   = VAF_EXISTS_FOR_POSITION | VAF_CONNECTED_TO_OUTDOOR | VAF_AFFECTED_BY_OUT_LIGHTS
	};

	N3DEngineCommon::SVisAreaInfo   m_pCamVisAreaInfo;
	N3DEngineCommon::SOceanInfo     m_OceanInfo;
	N3DEngineCommon::SRainOccluders m_RainOccluders;
	N3DEngineCommon::SCausticInfo   m_CausticInfo;
	SRainParams                     m_RainInfo;
	SSnowParams                     m_SnowInfo;

	void Update(const SRenderingPassInfo& passInfo);
	void UpdateRainInfo(const SRenderingPassInfo& passInfo);
	void UpdateRainOccInfo(const SRenderingPassInfo& passInfo);
	void UpdateSnowInfo(const SRenderingPassInfo& passInfo);
};

struct SVolumetricCloudTexInfo
{
	i32 cloudTexId;
	i32 cloudNoiseTexId;
	i32 edgeNoiseTexId;
};

// Structure that describe properties of the current rendering quality
struct SRenderQuality
{
	EShaderQuality shaderQuality = eSQ_Max;
	ERenderQuality renderQuality = eRQ_Max;

	Vec2           downscaleFactor;

	//////////////////////////////////////////////////////////////////////////
	SRenderQuality() : downscaleFactor(Vec2(1.0f,1.0f)) {}
};

struct SMSAA
{
	SMSAA() : Type(0), Quality(0), m_pZTexture(nullptr) {}
	void Clear()
	{
		Type = 0;
		Quality = 0;
		m_pZTexture = nullptr;
	}

	u32    Type;
	u32    Quality;
	CTexture* m_pZTexture;
};

struct SRTargetStat
{
	string      m_Name;
	u32      m_nSize;
	u32      m_nWidth;
	u32      m_nHeight;
	ETEX_Format m_eTF;

	void        GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_Name);
	}
};

struct DRX_ALIGN(128) SRenderStatistics
{
	static SRenderStatistics& Write() { return *s_pCurrentOutput; }

	i32 m_NumRendHWInstances;
	i32 m_RendHWInstancesPolysAll;
	i32 m_RendHWInstancesPolysOne;
	i32 m_RendHWInstancesDIPs;
	i32 m_NumTextChanges;
	i32 m_NumRTChanges;
	i32 m_NumStateChanges;
	i32 m_NumRendSkinnedObjects;
	i32 m_NumVShadChanges;
	i32 m_NumPShadChanges;
	i32 m_NumGShadChanges;
	i32 m_NumDShadChanges;
	i32 m_NumHShadChanges;
	i32 m_NumCShadChanges;
	i32 m_NumVShaders;
	i32 m_NumPShaders;
	i32 m_NumGShaders;
	i32 m_NumDShaders;
	i32 m_NumHShaders;
	i32 m_NumRTs;
	i32 m_NumSprites;
	i32 m_NumSpriteDIPS;
	i32 m_NumSpritePolys;
	i32 m_NumSpriteUpdates;
	i32 m_NumSpriteAltasesUsed;
	i32 m_NumSpriteCellsUsed;
	i32 m_NumQIssued;
	i32 m_NumQOccluded;
	i32 m_NumQNotReady;
	i32 m_NumQStallTime;
	i32 m_NumImpostersUpdates;
	i32 m_NumCloudImpostersUpdates;
	i32 m_NumImpostersDraw;
	i32 m_NumCloudImpostersDraw;
	i32 m_NumTextures;

#if defined(ENABLE_PROFILING_CODE)
	u32 m_NumShadowPoolFrustums;
	u32 m_NumShadowPoolAllocsThisFrame;
	u32 m_NumShadowMaskChannels;
	u32 m_NumTiledShadingSkippedLights;
#endif

	i32 m_NumPSInstructions;
	i32 m_NumVSInstructions;
	i32 m_RTCleared;
	i32 m_RTClearedSize;
	i32 m_RTCopied;
	i32 m_RTCopiedSize;
	i32 m_RTSize;

	CHWShader* m_pMaxPShader;
	CHWShader* m_pMaxVShader;
	uk m_pMaxPSInstance;
	uk m_pMaxVSInstance;

	size_t m_ManagedTexturesStreamSysSize;
	size_t m_ManagedTexturesStreamVidSize;
	size_t m_ManagedTexturesSysMemSize;
	size_t m_ManagedTexturesVidMemSize;
	size_t m_DynTexturesSize;
	size_t m_MeshUpdateBytes;
	size_t m_DynMeshUpdateBytes;
	float m_fOverdraw;
	float m_fSkinningTime;
	float m_fPreprocessTime;
	float m_fSceneTimeMT;
	float m_fTexUploadTime;
	float m_fTexRestoreTime;
	float m_fOcclusionTime;
	float m_fRenderTime;
	float m_fEnvCMapUpdateTime;
	float m_fEnvTextUpdateTime;

	float m_fRefractionPartialResolveEstimatedCost;
	i32 m_refractionPartialResolveCount;
	i32 m_refractionPartialResolvePixelCount;

	i32 m_NumRendMaterialBatches;
	i32 m_NumRendGeomBatches;
	i32 m_NumRendInstances;

#if defined(ENABLE_PROFILING_CODE)
	// Synchronously recorded stats
	i32 m_nNumInsts;
	i32 m_nNumInstCalls;

	i32 m_nNumPSOSwitches;
	i32 m_nNumLayoutSwitches;
	i32 m_nNumResourceSetSwitches;
	i32 m_nNumInlineSets;
	i32 m_nNumTopologySets;
	i32 m_nDIPs[EFSLIST_NUM];
	i32 m_nPolygons[EFSLIST_NUM];
	i32 m_nPolygonsByTypes[EFSLIST_NUM][EVCT_NUM][2];

	// Asynchronously recorded stats
	i32 m_nAsynchNumInsts;
	i32 m_nAsynchNumInstCalls;

	i32 m_nAsynchNumPSOSwitches;
	i32 m_nAsynchNumLayoutSwitches;
	i32 m_nAsynchNumResourceSetSwitches;
	i32 m_nAsynchNumInlineSets;
	i32 m_nAsynchNumTopologySets;
	i32 m_nAsynchDIPs[EFSLIST_NUM];
	i32 m_nAsynchPolygons[EFSLIST_NUM];
	i32 m_nAsynchPolygonsByTypes[EFSLIST_NUM][EVCT_NUM][2];
#endif

	i32 m_nModifiedCompiledObjects;
	i32 m_nTempCompiledObjects;
	i32 m_nIncompleteCompiledObjects;

	i32 m_nNumBoundVertexBuffers[2];   // Local=0,PCIe=1 - or in tech-speak, L1=0 and L0=1
	i32 m_nNumBoundIndexBuffers[2];    // Local=0,PCIe=1 - or in tech-speak, L1=0 and L0=1
	i32 m_nNumBoundConstBuffers[2];    // Local=0,PCIe=1 - or in tech-speak, L1=0 and L0=1
	i32 m_nNumBoundInlineBuffers[2];   // Local=0,PCIe=1 - or in tech-speak, L1=0 and L0=1
	i32 m_nNumBoundUniformBuffers[2];  // Local=0,PCIe=1 - or in tech-speak, L1=0 and L0=1
	i32 m_nNumBoundUniformTextures[2]; // Local=0,PCIe=1 - or in tech-speak, L1=0 and L0=1

	static SRenderStatistics* s_pCurrentOutput;

	void Begin(const SRenderStatistics* prevData);
	void Finish();

#if defined(ENABLE_PROFILING_CODE)
	i32  GetNumGeomInstances() const;
	i32  GetNumGeomInstanceDrawCalls() const;

	i32  GetNumberOfDrawCalls() const;
	i32  GetNumberOfDrawCalls(u32k EFSListMask) const;
	i32  GetNumberOfPolygons() const;
	i32  GetNumberOfPolygons(u32k EFSListMask) const;
#endif
};

//////////////////////////////////////////////////////////////////////
class CRenderer : public IRenderer, public CRendererResources, public CRendererCVars
{
	friend class CRendererResources;
	friend class CRendererCVars;

public:

	CRenderer();
	virtual ~CRenderer();

	virtual void InitRenderer();

	virtual void PostInit() override;

	virtual void StartRenderIntroMovies() override;
	virtual void StopRenderIntroMovies(bool bWaitForFinished) override;
	virtual bool IsRenderingIntroMovies() const override;

	virtual void PostLevelLoading() override;

	void         PreShutDown();
	void         PostShutDown();

	// Multithreading support
#if DRX_PLATFORM_DURANGO
	virtual void SuspendDevice();
	virtual void ResumeDevice();

	virtual void RT_SuspendDevice() = 0;
	virtual void RT_ResumeDevice() = 0;

	bool m_bDeviceSuspended;
#endif

	virtual void SyncComputeVerticesJobs() override;

	virtual void RT_PresentFast() = 0;

	virtual i32  CurThreadList() override;
	virtual void RT_BeginFrame(const SDisplayContextKey& displayContextKey) = 0;
	virtual void RT_EndFrame() = 0;

	virtual void RT_Init() = 0;
	virtual void RT_ShutDown(u32 nFlags) = 0;
	virtual bool RT_CreateDevice() = 0;
	virtual void RT_Reset() = 0;

	virtual void RT_RenderScene(CRenderView* pRenderView) = 0;

	virtual void RT_ReleaseRenderResources(u32 nFlags) = 0;

	virtual void RT_CreateRenderResources() = 0;
	virtual void RT_PrecacheDefaultShaders() = 0;
	virtual bool RT_ReadTexture(uk pDst, i32 destinationWidth, i32 destinationHeight, EReadTextureFormat dstFormat, CTexture* pSrc) = 0;
	virtual bool RT_StoreTextureToFile(tukk szFilePath, CTexture* pSrc) = 0;
	virtual void FlashRenderPlayer(std::shared_ptr<IFlashPlayer> &&pPlayer) override;
	virtual void FlashRender(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer) override;
	virtual void FlashRenderPlaybackLockless(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, i32 cbIdx, bool finalPlayback) override;
	virtual void FlashRemoveTexture(ITexture* pTexture) override;

	virtual void RT_RenderDebug(bool bRenderStats = true) = 0;

	virtual void RT_FlashRenderInternal(std::shared_ptr<IFlashPlayer> &&pPlayer) = 0;
	virtual void RT_FlashRenderInternal(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, bool doRealRender) = 0;
	virtual void RT_FlashRenderPlaybackLocklessInternal(std::shared_ptr<IFlashPlayer_RenderProxy> &&pPlayer, i32 cbIdx, bool finalPlayback, bool doRealRender) = 0;
	virtual bool FlushRTCommands(bool bWait, bool bImmediatelly, bool bForce) override;
	virtual bool ForceFlushRTCommands();
	virtual void WaitForParticleBuffer(i32 frameId) = 0;

	virtual void RequestFlushAllPendingTextureStreamingJobs(i32 nFrames) override { m_nFlushAllPendingTextureStreamingJobs = nFrames; }
	virtual void SetTexturesStreamingGlobalMipFactor(float fFactor) override      { m_fTexturesStreamingGlobalMipFactor = fFactor; }

	virtual void SetRendererCVar(ICVar* pCVar, tukk pArgText, const bool bSilentMode = false) override = 0;

	//===============================================================================

	virtual float*      PinOcclusionBuffer(Matrix44A& camera) override = 0;
	virtual void        UnpinOcclusionBuffer() override = 0;

	virtual void        AddListener(IRendererEventListener* pRendererEventListener) override;
	virtual void        RemoveListener(IRendererEventListener* pRendererEventListener) override;

	virtual ERenderType GetRenderType() const override;

	virtual WIN_HWND    Init(i32 x, i32 y, i32 width, i32 height, u32 cbpp, i32 zbpp, i32 sbits, WIN_HWND Glhwnd = 0, bool bReInit = false, bool bShaderCacheGen = false) override = 0;

	virtual WIN_HWND    GetCurrentContextHWND() { return GetHWND(); }

	virtual i32         GetFeatures() override { return m_Features; }

	virtual i32         GetNumGeomInstances() override;

	virtual i32 GetNumGeomInstanceDrawCalls() override;

	virtual i32  GetCurrentNumberOfDrawCalls() override;

	virtual void GetCurrentNumberOfDrawCalls(i32& nGeneral, i32& nShadowGen) override;

	virtual i32  GetCurrentNumberOfDrawCalls(u32k EFSListMask) override;

	virtual void SetDebugRenderNode(IRenderNode* pRenderNode) override;

	virtual bool IsDebugRenderNode(IRenderNode* pRenderNode) const override;

	virtual bool         CheckDeviceLost() { return false; };

	EScreenAspectRatio   GetScreenAspect(i32 nWidth, i32 nHeight);

	virtual Vec2         SetViewportDownscale(float xscale, float yscale) override;

	virtual void         Release() override;
	virtual void         FreeSystemResources(i32 nFlags) override;
	virtual void         InitSystemResources(i32 nFlags) override;

	virtual void         BeginFrame(const SDisplayContextKey& displayContextKey) override = 0;
	virtual void         FillFrame(ColorF clearColor) override = 0;
	virtual void         RenderDebug(bool bRenderStats = true) override = 0;
	virtual void         EndFrame() override = 0;
	virtual void         LimitFramerate(i32k maxFPS, const bool bUseSleep) = 0;

	virtual void         TryFlush() override = 0;

	virtual void         Reset(void) = 0;

	float                GetDrawNearestFOV() const { return m_drawNearFov; }

	virtual void         EnableVSync(bool enable) override = 0;

	virtual bool         SaveTga(u8* sourcedata, i32 sourceformat, i32 w, i32 h, tukk filename, bool flip) const override;

	//download an image to video memory. 0 in case of failure
	virtual u32 UploadToVideoMemory(u8* data, i32 w, i32 h, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) override = 0;
	virtual void         UpdateTextureInVideoMemory(u32 tnum, u8* newdata, i32 posx, i32 posy, i32 w, i32 h, ETEX_Format eTFSrc = eTF_R8G8B8A8, i32 posz = 0, i32 sizez = 1) override = 0;
	virtual u32 UploadToVideoMemory3D(u8* data, i32 w, i32 h, i32 d, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) override = 0;
	virtual u32 UploadToVideoMemoryCube(u8* data, i32 w, i32 h, ETEX_Format eTFSrc, ETEX_Format eTFDst, i32 nummipmap, bool repeat = true, i32 filter = FILTER_BILINEAR, i32 Id = 0, tukk szCacheName = NULL, i32 flags = 0, EEndian eEndian = eLittleEndian, RectI* pRegion = NULL, bool bAsynDevTexCreation = false) override = 0;

	virtual bool         DXTCompress(byte* raw_data, i32 nWidth, i32 nHeight, ETEX_Format eTF, bool bUseHW, bool bGenMips, i32 nSrcBytesPerPix, MIPDXTcallback callback) override;
	virtual bool         DXTDecompress(byte* srcData, const size_t srcFileSize, byte* dstData, i32 nWidth, i32 nHeight, i32 nMips, ETEX_Format eSrcTF, bool bUseHW, i32 nDstBytesPerPix) override;

	virtual bool         SetGammaDelta(const float fGamma) override = 0;

	virtual void         RemoveTexture(u32 TextureId) override = 0;

	virtual i32          GetWhiteTextureId() const override;

	CTextureUpr*     GetTextureUpr() { return m_pTextureUpr; }

	virtual void         PrintResourcesLeaks() = 0;

	inline float         ScaleCoordXInternal(float value, const SRenderViewport& vp) const        { value *= float(vp.width) / 800.0f; return (value); }
	inline float         ScaleCoordYInternal(float value, const SRenderViewport& vp) const        { value *= float(vp.height) / 600.0f; return (value); }
	inline void          ScaleCoordInternal(float& x, float& y, const SRenderViewport& vp) const  { x = ScaleCoordXInternal(x, vp); y = ScaleCoordYInternal(y, vp); }

	virtual float        ScaleCoordX(float value) const override;
	virtual float        ScaleCoordY(float value) const override;
	virtual void         ScaleCoord(float& x, float& y) const override;

//	void                 SetWidth(i32 nW)                              { ChangeRenderResolution(nW, CRendererResources::s_renderHeight); }
//	void                 SetHeight(i32 nH)                             { ChangeRenderResolution(CRendererResources::s_renderWidth, nH); }
//	void                 SetPixelAspectRatio(float fPAR)               { m_pixelAspectRatio = fPAR; }

	virtual i32          GetWidth() const override                     { return CRendererResources::s_renderWidth; }
	virtual i32          GetHeight() const override                    { return CRendererResources::s_renderHeight; }

	virtual i32          GetOverlayWidth() const override;
	virtual i32          GetOverlayHeight() const override;

	virtual float        GetPixelAspectRatio() const override          { return (m_pixelAspectRatio); }

	virtual bool         IsStereoEnabled() const override              { return false; }

	virtual float        GetNearestRangeMax() const override           { return (CV_r_DrawNearZRange); }

	virtual i32          GetWireframeMode()                            { return(m_wireframe_mode); }

	virtual CRenderView* GetOrCreateRenderView(IRenderView::EViewType Type = IRenderView::eViewType_Default) threadsafe final;
	virtual void         ReturnRenderView(CRenderView* pRenderView) threadsafe final;
	void                 DeleteRenderViews();

	void             GetPolyCount(i32& nPolygons, i32& nShadowPolys) override;
	i32              GetPolyCount() override;

	virtual bool     WriteDDS(byte* dat, i32 wdt, i32 hgt, i32 Size, tukk name, ETEX_Format eF, i32 NumMips) override;
	virtual bool     WriteTGA(byte* dat, i32 wdt, i32 hgt, tukk name, i32 src_bits_per_pixel, i32 dest_bits_per_pixel) override;
	virtual bool     WriteJPG(byte* dat, i32 wdt, i32 hgt, tuk name, i32 src_bits_per_pixel, i32 nQuality = 100) override;

	virtual void     GetMemoryUsage(IDrxSizer* Sizer) override;

	virtual void     GetBandwidthStats(float* fBandwidthRequested) override;

	virtual void     SetTextureStreamListener(ITextureStreamListener* pListener) override;

#if defined(DRX_ENABLE_RC_HELPER)
	virtual void AddAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener);
	virtual void RemoveAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener);
#endif

	virtual void GetLogVBuffers() = 0;

	virtual i32  GetFrameID(bool bIncludeRecursiveCalls = true) override;

	// GPU being updated
	i32 RT_GetCurrGpuID() const;

	// Project/UnProject.  Returns true if successful.
	virtual bool ProjectToScreen(float ptx, float pty, float ptz, float* sx, float* sy, float* sz) override = 0;
	virtual i32  UnProject(float sx, float sy, float sz,
	                       float* px, float* py, float* pz,
	                       const float modelMatrix[16],
	                       const float projMatrix[16],
	                       i32k viewport[4]) override = 0;
	virtual i32  UnProjectFromScreen(float sx, float sy, float sz, float* px, float* py, float* pz) override = 0;

	// Shadow Mapping
	virtual void OnEntityDeleted(IRenderNode* pRenderNode) override;

	virtual void SetHighlightColor(ColorF color) override { m_highlightColor = color; }
	virtual void SetSelectionColor(ColorF color) override { m_SelectionColor = color; }
	virtual void SetHighlightParams(float outlineThickness, float fGhostAlpha) override
	{ 
		m_highlightParams = Vec4(outlineThickness, fGhostAlpha, 0.0f, 0.0f); 
	}

	ColorF& GetHighlightColor()         { return m_highlightColor; }
	ColorF& GetSelectionColor()         { return m_SelectionColor; }
	Vec4&   GetHighlightParams()        { return m_highlightParams; }

	//misc
	virtual bool                ScreenShot(tukk filename = NULL, const SDisplayContextKey& displayContextKey = {}) override = 0;
	virtual bool                ReadFrameBuffer(u32* pDstRGBA8, i32 destinationWidth, i32 destinationHeight, bool readPresentedBackBuffer = true, EReadTextureFormat format = EReadTextureFormat::RGB8) override = 0;

	virtual i32                 GetColorBpp() override   { return m_cbpp; }
	virtual i32                 GetDepthBpp() override   { return m_zbpp; }
	virtual i32                 GetStencilBpp() override { return m_sbpp; }

	virtual void                Set2DMode(bool enable, i32 ortox, i32 ortoy, float znear = -1e10f, float zfar = 1e10f) override = 0;

	virtual void                LockParticleVideoMemory(i32 frameId) override           {}
	virtual void                UnLockParticleVideoMemory(i32 frameId) override         {}

	virtual void                ActivateLayer(tukk pLayerName, bool activate) override {}

	virtual void                FlushPendingTextureTasks() override;
	virtual void                FlushPendingUploads() override;

	virtual void                SetCloakParams(const SRendererCloakParams& cloakParams) override;
	virtual float               GetCloakFadeLightScale() const override;
	virtual void                SetCloakFadeLightScale(float fColorScale) override;
	virtual void                SetShadowJittering(float shadowJittering) override;
	virtual float               GetShadowJittering() const override;

	void                        EF_AddClientPolys(const SRenderingPassInfo& passInfo);

	virtual uk               FX_AllocateCharInstCB(SSkinningData*, u32) { return NULL; }
	virtual void                FX_ClearCharInstCB(u32)                    {}

	virtual EShaderQuality      EF_GetShaderQuality(EShaderType eST) final;
	virtual ERenderQuality      EF_GetRenderQuality() const final             { return m_renderQuality.renderQuality; };

	virtual void                EF_SubmitWind(const SWindGrid* pWind) override;

	void                        RefreshSystemShaders();

	virtual float               EF_GetWaterZElevation(float fX, float fY) override;

	virtual IOpticsElementBase* CreateOptics(EFlareType type) const override;

	virtual bool                EF_PrecacheResource(IShader* pSH, float fMipFactor, float fTimeToReady, i32 Flags) override;
	virtual bool                EF_PrecacheResource(ITexture* pTP, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter) override = 0;
	virtual bool                EF_PrecacheResource(IRenderMesh* pPB, IMaterial* pMaterial, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId) override;
	virtual bool                EF_PrecacheResource(SRenderLight* pLS, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId) override;

	// functions for handling particle jobs which cull particles and generate their vertices/indices
	virtual void EF_AddMultipleParticlesToScene(const SAddParticlesToSceneJob* jobs, size_t numJobs, const SRenderingPassInfo& passInfo) override;

	//==========================================================
	// external interface for shaders
	//==========================================================
	// Shaders management
	virtual void        EF_SetShaderMissCallback(ShaderCacheMissCallback callback) override;
	virtual tukk EF_GetShaderMissLogPath() override;
	virtual string*     EF_GetShaderNames(i32& nNumShaders) override;
	virtual IShader*    EF_LoadShader(tukk name, i32 flags = 0, uint64 nMaskGen = 0) override;
	virtual SShaderItem EF_LoadShaderItem(tukk name, bool bShare, i32 flags = 0, SInputShaderResources* Res = NULL, uint64 nMaskGen = 0, const SLoadShaderItemArgs* pArgs = 0) override;
	virtual uint64      EF_GetRemapedShaderMaskGen(tukk name, uint64 nMaskGen = 0, bool bFixup = 0) override;

	virtual uint64      EF_GetShaderGlobalMaskGenFromString(tukk szShaderName, tukk szShaderGen, uint64 nMaskGen = 0) override;
	virtual tukk EF_GetStringFromShaderGlobalMaskGen(tukk szShaderName, uint64 nMaskGen = 0) override;

	// reload file
	virtual bool                   EF_ReloadFile(tukk szFileName) override;
	virtual bool                   EF_ReloadFile_Request(tukk szFileName) override;
	virtual void                   EF_ReloadShaderFiles(i32 nCategory) override;
	virtual void                   EF_ReloadTextures() override;
	virtual i32                    EF_LoadLightmap(tukk nameTex) override;
	virtual DynArray<uint16_t>     EF_RenderEnvironmentCubeHDR(std::size_t size, const Vec3& Pos) override;
	virtual bool                   WriteTIFToDisk(ukk pData, i32 width, i32 height, i32 bytesPerChannel, i32 numChannels, bool bFloat, tukk szPreset, tukk szFileName) override;
	virtual ITexture*              EF_GetTextureByID(i32 Id) override;
	virtual ITexture*              EF_GetTextureByName(tukk name, u32 flags = 0) override;
	virtual ITexture*              EF_LoadTexture(tukk nameTex, u32k flags = 0) override;
	virtual IDynTextureSource*     EF_LoadDynTexture(tukk dynsourceName, bool sharedRT = false) override;
	virtual const SShaderProfile&  GetShaderProfile(EShaderType eST) const override;
	virtual void                   EF_SetShaderQuality(EShaderType eST, EShaderQuality eSQ) override;

	virtual _smart_ptr<IImageFile> EF_LoadImage(tukk szFileName, u32 nFlags) override;

	// Create new RE of type (edt)
	virtual CRenderElement*          EF_CreateRE(EDataType edt) override;

	// Begin using shaders
	virtual void EF_StartEf(const SRenderingPassInfo& passInfo) override;

	// Get Object for RE transformation
	virtual CRenderObject* EF_DuplicateRO(CRenderObject* pObj, const SRenderingPassInfo& passInfo) final;
	virtual CRenderObject* EF_GetObject() final;
	virtual void           EF_FreeObject(CRenderObject* pObj) final;

	// Draw all shaded REs in the list
	virtual void EF_EndEf3D(i32k nFlags, i32k nPrecacheUpdateId, i32k nNearPrecacheUpdateId, const SRenderingPassInfo& passInfo) override = 0;

	virtual void EF_InvokeShadowMapRenderJobs(const SRenderingPassInfo& passInfo, i32k nFlags) override {}
	virtual IRenderView* GetNextAvailableShadowsView(IRenderView* pMainRenderView, ShadowMapFrustum* pOwnerFrustum) override;
	void PrepareShadowFrustumForShadowPool(ShadowMapFrustum* pFrustum, u32 frameID, const SRenderLight& light, u32 *timeSlicedShadowsUpdated) override final;

	// 2d interface for shaders
	virtual void EF_EndEf2D(const bool bSort) override = 0;

	// Dynamic lights
	virtual bool                   EF_IsFakeDLight(const SRenderLight* Source) override;
	virtual void                   EF_ADDDlight(SRenderLight* Source, const SRenderingPassInfo& passInfo) override;
	virtual bool                   EF_AddDeferredDecal(const SDeferredDecal& rDecal, const SRenderingPassInfo& passInfo) override;

	virtual i32                    EF_AddDeferredLight(const SRenderLight& pLight, float fMult, const SRenderingPassInfo& passInfo) override;

	virtual void                   EF_ReleaseDeferredData() override;
	virtual SInputShaderResources* EF_CreateInputShaderResource(IRenderShaderResources* pOptionalCopyFrom = nullptr) override;
	virtual void                   ClearPerFrameData(const SRenderingPassInfo& passInfo);
	virtual bool                   EF_UpdateDLight(SRenderLight* pDL) override;
	void                           EF_CheckLightMaterial(SRenderLight* pLight, u16 nRenderLightID, const SRenderingPassInfo& passInfo);

	virtual void EF_QueryImpl(ERenderQueryTypes eQuery, uk pInOut0, u32 nInOutSize0, uk pInOut1, u32 nInOutSize1) override;

	//////////////////////////////////////////////////////////////////////////
	// Deferred ambient passes
	virtual void Ef_AddDeferredGIClipVolume(const IRenderMesh* pClipVolume, const Matrix34& mxTransform) override;

	//////////////////////////////////////////////////////////////////////////
	// Post processing effects interfaces

	virtual void  EF_SetPostEffectParam(tukk pParam, float fValue, bool bForceValue = false) override;
	virtual void  EF_SetPostEffectParamVec4(tukk pParam, const Vec4& pValue, bool bForceValue = false) override;
	virtual void  EF_SetPostEffectParamString(tukk pParam, tukk pszArg) override;

	virtual void  EF_GetPostEffectParam(tukk pParam, float& fValue) override;
	virtual void  EF_GetPostEffectParamVec4(tukk pParam, Vec4& pValue) override;
	virtual void  EF_GetPostEffectParamString(tukk pParam, tukk & pszArg) override;

	virtual i32 EF_GetPostEffectID(tukk pPostEffectName) override;

	virtual void  EF_ResetPostEffects(bool bOnSpecChange = false) override;

	virtual void  EF_DisableTemporalEffects() override;

	virtual void  ForceGC() override;

	// create/delete RenderMesh object
	virtual _smart_ptr<IRenderMesh> CreateRenderMesh(
	  tukk szType
	  , tukk szSourceName
	  , IRenderMesh::SInitParamerers* pInitParams = NULL
	  , ERenderMeshType eBufType = eRMT_Static
	  ) override;

	virtual _smart_ptr<IRenderMesh> CreateRenderMeshInitialized(
	  ukk pVertBuffer, i32 nVertCount, InputLayoutHandle eVF,
	  const vtx_idx* pIndices, i32 nIndices,
	  const PublicRenderPrimitiveType nPrimetiveType, tukk szType, tukk szSourceName, ERenderMeshType eBufType = eRMT_Static,
	  i32 nMatInfoCount = 1, i32 nClientTextureBindID = 0,
	  bool (* PrepareBufferCallback)(IRenderMesh*, bool) = NULL,
	  uk CustomData = NULL,
	  bool bOnlyVideoBuffer = false, bool bPrecache = true, const SPipTangents* pTangents = NULL, bool bLockForThreadAcc = false, Vec3* pNormals = NULL) override;

	virtual i32 GetMaxActiveTexturesARB() { return 0; }

	//////////////////////////////////////////////////////////////////////
	// Replacement functions for the Font engine ( vlad: for font can be used old functions )
	virtual bool FontUploadTexture(class CFBitmap*, ETEX_Format eSrcFormat = eTF_R8G8B8A8) override = 0;
	virtual i32  FontCreateTexture(i32 Width, i32 Height, byte* pSrcData, ETEX_Format eSrcFormat = eTF_R8G8B8A8, bool genMips = false) override = 0;
	virtual bool FontUpdateTexture(i32 nTexId, i32 X, i32 Y, i32 USize, i32 VSize, byte* pSrcData) override = 0;
	virtual void FontReleaseTexture(class CFBitmap* pBmp) override = 0;

	//////////////////////////////////////////////////////////////////////
	// Used for pausing timer related stuff (eg: for texture animations, and shader 'time' parameter)
	void                         PauseTimer(bool bPause) override { m_bPauseTimer = bPause; }
	virtual IShaderPublicParams* CreateShaderPublicParams() override;

	virtual void                 SetLevelLoadingThreadId(threadID threadId) override;
	virtual void                 GetThreadIDs(threadID& mainThreadID, threadID& renderThreadID) const override;

#if RENDERER_SUPPORT_SCALEFORM
	void SF_ConfigMask(i32 st, u32 ref);
	virtual i32 SF_CreateTexture(i32 width, i32 height, i32 numMips, u8k* pSrcData, ETEX_Format eSrcFormat, i32 flags) override;
	virtual void SF_GetMeshMaxSize(i32& numVertices, i32& numIndices) const override;

	virtual IScaleformPlayback* SF_CreatePlayback() const override;
	virtual void SF_Playback(IScaleformPlayback* pRenderer, GRendererCommandBufferReadOnly* pBuffer) const override;
	virtual void SF_Drain(GRendererCommandBufferReadOnly* pBuffer) const override;
#else // #if RENDERER_SUPPORT_SCALEFORM
	// These dummy functions are required when the feature is disabled, do not remove without testing the RENDERER_SUPPORT_SCALEFORM=0 case!
	virtual i32 SF_CreateTexture(i32 width, i32 height, i32 numMips, u8k* pSrcData, ETEX_Format eSrcFormat, i32 flags) override { return 0; }
	virtual void SF_GetMeshMaxSize(i32& numVertices, i32& numIndices) const override { numVertices = 0; numIndices = 0; }
	virtual IScaleformPlayback* SF_CreatePlayback() const override;
	virtual void SF_Playback(IScaleformPlayback* pRenderer, GRendererCommandBufferReadOnly* pBuffer) const override {}
	virtual void SF_Drain(GRendererCommandBufferReadOnly* pBuffer) const override {}
#endif // #if RENDERER_SUPPORT_SCALEFORM

	virtual ITexture* CreateTexture(tukk name, i32 width, i32 height, i32 numMips, u8* pSrcData, ETEX_Format eSrcFormat, i32 flags) override;
	virtual ITexture* CreateTextureArray(tukk name, ETEX_Type eType, u32 nWidth, u32 nHeight, u32 nArraySize, i32 nMips, u32 nFlags, ETEX_Format eSrcFormat, i32 nCustomID) override;

	enum ESPM {ESPM_PUSH = 0, ESPM_POP = 1};
	virtual void   SetProfileMarker(tukk label, ESPM mode) const                              {};

	virtual i32    GetMaxTextureSize() override                                                               { return m_MaxTextureSize; }

	virtual void   SetCloudShadowsParams(i32 nTexID, const Vec3& speed, float tiling, bool invert, float brightness) override;
	i32            GetCloudShadowTextureId() const { return m_cloudShadowTexId; }
	bool GetCloudShadowsEnabled() const;

	virtual void                                      SetVolumetricCloudParams(i32 nTexID) override;
	virtual void                                      SetVolumetricCloudNoiseTex(i32 cloudNoiseTexId, i32 edgeNoiseTexId) override;
	void                                              GetVolumetricCloudTextureInfo(SVolumetricCloudTexInfo& info) const;

	virtual ShadowFrustumMGPUCache*                   GetShadowFrustumMGPUCache() override                                                          { return &m_ShadowFrustumMGPUCache; }

	virtual const StaticArray<i32, MAX_GSM_LODS_NUM>& GetCachedShadowsResolution() const override                                                   { return m_CachedShadowsResolution; }
	virtual void                                      SetCachedShadowsResolution(const StaticArray<i32, MAX_GSM_LODS_NUM>& arrResolutions) override { m_CachedShadowsResolution = arrResolutions; }
	virtual void                                      UpdateCachedShadowsLodCount(i32 nGsmLods) const override;

	virtual bool                                      IsPost3DRendererEnabled() const override;

	virtual void                                      ExecuteAsyncDIP() override;

	alloc_info_struct*                                GetFreeChunk(i32 bytes_count, i32 nBufSize, PodArray<alloc_info_struct>& alloc_info, tukk szSource);
	bool                                              ReleaseChunk(i32 p, PodArray<alloc_info_struct>& alloc_info);

	virtual tukk                               GetTextureFormatName(ETEX_Format eTF) override;
	virtual i32                                       GetTextureFormatDataSize(i32 nWidth, i32 nHeight, i32 nDepth, i32 nMips, ETEX_Format eTF) override;
	virtual void                                      SetDefaultMaterials(IMaterial* pDefMat, IMaterial* pTerrainDefMat) override                          { m_pDefaultMaterial = pDefMat; m_pTerrainDefaultMaterial = pTerrainDefMat; }
	virtual byte*                                     GetTextureSubImageData32(byte* pData, i32 nDataSize, i32 nX, i32 nY, i32 nW, i32 nH, CTexture* pTex) { return 0; }

	virtual void                                      PrecacheTexture(ITexture* pTP, float fMipFactor, float fTimeToReady, i32 Flags, i32 nUpdateId, i32 nCounter = 1);

	virtual SSkinningData*                            EF_CreateSkinningData(IRenderView* pRenderView, u32 nNumBones, bool bNeedJobSyncVar) override;
	virtual SSkinningData*                            EF_CreateRemappedSkinningData(IRenderView* pRenderView, u32 nNumBones, SSkinningData* pSourceSkinningData, u32 nCustomDataSize, u32 pairGuid) override;
	virtual void                                      EF_EnqueueComputeSkinningData(IRenderView* pRenderView, SSkinningData* pData) override;
	virtual i32                                       EF_GetSkinningPoolID() override;
	SSkinningDataPoolInfo                             GetSkinningDataPools();
	void                                              ClearSkinningDataPool();

	virtual void                                      UpdateShaderItem(SShaderItem* pShaderItem, IMaterial* pMaterial) override;
	virtual void                                      ForceUpdateShaderItem(SShaderItem* pShaderItem, IMaterial* pMaterial) override;
	virtual void                                      RefreshShaderResourceConstants(SShaderItem* pShaderItem, IMaterial* pMaterial) override;

	virtual bool                                      LoadShaderStartupCache() override;

	virtual void                                      UnloadShaderStartupCache() override;

	virtual void                                      CopyTextureRegion(ITexture* pSrc, RectI srcRegion, ITexture* pDst, RectI dstRegion, ColorF& color, i32k renderStateFlags) override;

	virtual bool                                      LoadShaderLevelCache() override            { return false; }
	virtual void                                      UnloadShaderLevelCache() override          {}

	virtual void                                      RegisterSyncWithMainListener(ISyncMainWithRenderListener* pListener) override;
	virtual void                                      RemoveSyncWithMainListener(const ISyncMainWithRenderListener* pListener) override;

	virtual void                                      SetCurDownscaleFactor(Vec2 sf) = 0;

	virtual IGraphicsDeviceConstantBufferPtr          CreateGraphiceDeviceConstantBuffer() override                                              { assert(0);  return 0; };

	virtual void                                      MakeMatrix(const Vec3& pos, const Vec3& angles, const Vec3& scale, Matrix34* mat) override { assert(0); }

public:
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC HELPER METHODS
	//////////////////////////////////////////////////////////////////////////
	void   SyncMainWithRender();

	void   ReadPerFrameShaderConstants(const SRenderingPassInfo& passInfo, bool bSecondaryViewport);

	u32 GetActiveGPUCount() const override { return CV_r_multigpu > 0 ? m_nGPUs : 1; }

	void   Logv(tukk format, ...);
	void   LogStrv(tuk format, ...);
	void   LogShv(tuk format, ...);
	void   Log(tuk str);

	// Add shader to the list
	void              EF_AddEf_NotVirtual(CRenderElement* pRE, SShaderItem& pSH, CRenderObject* pObj, const SRenderingPassInfo& passInfo, i32 nList, i32 nAW);

	void              EF_TransformDLights();
	void              EF_IdentityDLights();

	void              EF_AddRTStat(CTexture* pTex, i32 nFlags = 0, i32 nW = -1, i32 nH = -1);
	void              EF_PrintRTStats(tukk szName);

	static inline eAntialiasingType FX_GetAntialiasingType () { return (eAntialiasingType)((u32)1 << min(CV_r_AntialiasingMode, eAT_AAMODES_COUNT - 1)); }
	static inline bool              IsHDRDisplayEnabled    () { return (CV_r_HDRSwapChain && !CV_r_measureoverdraw) ? true : false; }
	static inline bool              IsPostProcessingEnabled() { return (CV_r_PostProcess && !CV_r_measureoverdraw) ? true : false; }

	void              UpdateRenderingModesInfo();
	bool              IsCustomRenderModeEnabled(u32 nRenderModeMask);

	bool              IsEditorMode() const
	{
#if DRX_PLATFORM_DESKTOP
		return (m_bEditor != 0);
#else
		return false;
#endif
	}

	bool              IsShaderCacheGenMode() const
	{
#if DRX_PLATFORM_DESKTOP
		return (m_bShaderCacheGen != 0);
#else
		return false;
#endif
	}

	//	Get mipmap distance factor (depends on screen width, screen height and aspect ratio)
	float                GetMipDistFactor(u32 twidth, u32 theight) { float ratio = std::max(twidth, theight) / float(CRendererResources::s_renderMinDim); return ratio * ratio; }
//	float                GetMipDistFactor(u32 twidth, u32 theight) { return ((TANGENT30_2 * TANGENT30_2) / (m_rheight * m_rheight)) * std::max(twidth, theight) * std::max(twidth, theight); }

	static i32           GetTexturesStreamPoolSize();

protected:
	void EF_AddParticle(CREParticle* pParticle, SShaderItem& shaderItem, CRenderObject* pRO, const SRenderingPassInfo& passInfo);
	void EF_RemoveParticlesFromScene();
	void PrepareParticleRenderObjects(Array<const SAddParticlesToSceneJob> aJobs, i32 nREStart, const SRenderingPassInfo& passInfo);
	void EF_GetParticleListAndBatchFlags(u32& nBatchFlags, i32& nList, CRenderObject* pRenderObject, const SShaderItem& shaderItem, const SRenderingPassInfo& passInfo);

	void FreePermanentRenderObjects(i32 bufferId);

public:
	uk operator new(size_t Size)
	{
		uk pPtrRes = DrxModuleMemalign(Size, 16);
		memset(pPtrRes, 0, Size);
		return pPtrRes;
	}
	void operator delete(uk Ptr)
	{
		DrxModuleMemalignFree(Ptr);
	}

	virtual WIN_HWND GetHWND() override = 0;

	void             SetTextureAlphaChannelFromRGB(byte* pMemBuffer, i32 nTexSize);

	void             EnableSwapBuffers(bool bEnable) override { m_bSwapBuffers = bEnable; }
	bool m_bSwapBuffers;

	virtual bool StopRendererAtFrameEnd(uint timeoutMilliseconds) override;
	virtual void ResumeRendererFromFrameEnd() override;
	 bool m_bStopRendererAtFrameEnd;

	virtual void                   SetTexturePrecaching(bool stat) override;

	virtual const RPProfilerStats*                   GetRPPStats(ERenderPipelineProfilerStats eStat, bool bCalledFromMainThread = true) override  { return nullptr; }
	virtual const RPProfilerStats*                   GetRPPStatsArray(bool bCalledFromMainThread = true) override                                 { return nullptr; }
	virtual const DynArray<RPProfilerDetailedStats>* GetRPPDetailedStatsArray(bool bCalledFromMainThread = true ) override                        { return nullptr; }

	virtual i32                    GetPolygonCountByType(u32 EFSList, EVertexCostTypes vct, u32 z, bool bCalledFromMainThread = true) override { return 0; }

	//platform specific
	virtual void  RT_InsertGpuCallback(u32 context, GpuCallbackFunc callback) override {}
	virtual void  EnablePipelineProfiler(bool bEnable) override = 0;

	virtual float GetGPUFrameTime() override;
	virtual void  GetRenderTimes(SRenderTimes& outTimes) override;
	virtual void  LogShaderImportMiss(const CShader* pShader) {}

#if !defined(_RELEASE)
	//Get debug draw call stats stat
	virtual RNDrawcallsMapMesh& GetDrawCallsInfoPerMesh(bool mainThread = true) override;
	virtual i32                 GetDrawCallsPerNode(IRenderNode* pRenderNode) override;

	//Routine to perform an emergency flush of a particular render node from the stats, as not all render node holders are delay-deleted
	virtual void ForceRemoveNodeFromDrawCallsMap(IRenderNode* pNode) override;

	void ClearDrawCallsInfo();
#endif
#ifdef ENABLE_PROFILING_CODE
	void         AddRecordedProfilingStats(const struct SProfilingStats& stats, ERenderListID renderList, bool bAsynchronous);
#endif

	virtual void                CollectDrawCallsInfo(bool status) override;
	virtual void                CollectDrawCallsInfoPerNode(bool status) override;
	virtual void                EnableLevelUnloading(bool enable) override;
	virtual void                EnableBatchMode(bool enable) override;
	virtual bool                IsStereoModeChangePending() override { return false; }
		
	virtual void QueryActiveGpuInfo(SGpuInfo& info) const override;

	virtual compute_skinning::IComputeSkinningStorage* GetComputeSkinningStorage() = 0;

	i32   GetStreamZoneRoundId( i32 zone ) const { assert(zone >=0 && zone < MAX_PREDICTION_ZONES); return m_streamZonesRoundId[zone]; };

	// Only should be used to get current frame id internally in the render thread.
	i32 GetRenderFrameID() const;
	i32 GetMainFrameID()   const;

	threadID GetMainThreadID() const { return m_nFillThreadID; }
	threadID GetRenderThreadID() const { return m_nProcessThreadID; }
	SRenderObjectAccessThreadConfig GetObjectAccessorThreadConfig() const
	{
		DRX_ASSERT(m_pRT->IsRenderThread());
		return SRenderObjectAccessThreadConfig(GetRenderThreadID());
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Query Anti-Aliasing information.
	bool                  IsMSAAEnabled() const { return m_MSAAData.Type > 0; }
	const SMSAA&          GetMSAA() const { return m_MSAAData; }

	void                  SetRenderQuality( const SRenderQuality &quality );
	const SRenderQuality& GetRenderQuality() const { return m_renderQuality; };

	// Animation time is used for rendering animation effects and can be paused if CRenderer::m_bPauseTimer is true
	void                  SetAnimationTime(CTimeValue time) { m_animationTime = time; }
	CTimeValue            GetAnimationTime() const { return m_animationTime; }

	// Time of the last main to renderer thread sync
	void                  SetFrameSyncTime(CTimeValue time) { m_frameSyncTime = time; }
	CTimeValue            GetFrameSyncTime() const { return m_frameSyncTime; }
		
	// Return current graphics pipeline
	CStandardGraphicsPipeline&     GetGraphicsPipeline() { return *m_pGraphicsPipeline; }

	template<typename RenderThreadCallback>
	void ExecuteRenderThreadCommand(RenderThreadCallback&& callback, ERenderCommandFlags flags = ERenderCommandFlags::None)
	{
		m_pRT->ExecuteRenderThreadCommand(std::forward<RenderThreadCallback>(callback), flags);
	}

	// Called every frame from the Render Thread to reclaim deleted resources.
	void                   ScheduleResourceForDelete(CBaseResource* pResource);
	void                   RT_DelayedDeleteResources(bool bAllResources=false);

public:
	Matrix44A m_IdentityMatrix;

	byte           m_bDeviceLost;
	byte           m_bSystemResourcesInit;
	byte           m_bSystemTargetsInit;
	bool           m_bAquireDeviceThread;
	bool           m_bInitialized;

	SRenderThread* m_pRT;

	// Shaders pipeline states
	//=============================================================================================================
	CSubmissionQueue_DX11 m_DevMan;
	CDeviceBufferUpr m_DevBufMan;
	//=============================================================================================================

	CIntroMovieRenderer* m_pIntroMovieRenderer;

	float                m_fTimeWaitForMain[RT_COMMAND_BUF_COUNT];
	float                m_fTimeWaitForRender[RT_COMMAND_BUF_COUNT];
	float                m_fTimeProcessedRT[RT_COMMAND_BUF_COUNT];
	float                m_fTimeProcessedGPU[RT_COMMAND_BUF_COUNT];
	float                m_fTimeWaitForGPU[RT_COMMAND_BUF_COUNT];
	float                m_fTimeGPUIdlePercent[RT_COMMAND_BUF_COUNT];

	float                m_fRTTimeEndFrame;
	float                m_fRTTimeFlashRender;
	float                m_fRTTimeSceneRender;
	float                m_fRTTimeMiscRender;

	i32                  m_CurVertBufferSize;
	i32                  m_CurIndexBufferSize;

	i32                  m_nGPU;
	i32                  m_VSync;
	i32                  m_Predicated;

	i32                  m_nGraphicsPipeline;

#if defined(SUPPORT_DEVICE_INFO_USER_DISPLAY_OVERRIDES)
	float m_overrideRefreshRate;
	i32   m_overrideScanlineOrder;
#endif

	i32       m_nStencilMaskRef;

	byte      m_bDeviceSupportsInstancing;

	u32    m_bDeviceSupports_AMDExt;
	u32    m_bDeviceSupports_NVDBT          : 1;
	u32    m_bDeviceSupportsTessellation    : 1;
	u32    m_bDeviceSupportsGeometryShaders : 1;

	u32    m_bEditor                        : 1; // Render instance created from editor
	u32    m_bShaderCacheGen                : 1; // Render instance create from shader cache gen mode
	u32    m_bUseHWSkinning                 : 1;
	u32    m_bShadersPresort                : 1;
	u32    m_bEndLevelLoading               : 1;
	u32    m_bLevelUnloading                : 1;
	u32    m_bStartLevelLoading             : 1;
	u32    m_bInLevel                       : 1;
	u32    m_bUseWaterTessHW                : 1;
	u32    m_bUseSilhouettePOM              : 1;
	u32    m_bWaterCaustics                 : 1;
	u32    m_bIsWindowActive                : 1;
	u32    m_bInShutdown                    : 1;
	u32    m_bDeferredDecals                : 1;
	u32    m_bShadowsEnabled                : 1;
	u32    m_bCloudShadowsEnabled           : 1;
#if defined(VOLUMETRIC_FOG_SHADOWS)
	u32    m_bVolFogShadowsEnabled          : 1;
	u32    m_bVolFogCloudShadowsEnabled     : 1;
#endif
	u32    m_bVolumetricFogEnabled          : 1;
	u32    m_bVolumetricCloudsEnabled       : 1;
	u32    m_bDeferredRainEnabled           : 1;
	u32    m_bDeferredRainOcclusionEnabled  : 1;
	u32    m_bDeferredSnowEnabled           : 1;

	u8     m_nDisableTemporalEffects;
	bool      m_bUseGPUFriendlyBatching[2];
	u32    m_nGPULimited;           // How many frames we are GPU limited
	int8      m_nCurMinAniso;
	int8      m_nCurMaxAniso;
	float     m_fCurMipLodBias;

	u32    m_nShadowPoolHeight;
	u32    m_nShadowPoolWidth;

	ColorF    m_CurFontColor;

	DWORD     m_Features;
	i32       m_MaxTextureSize;
	size_t    m_MaxTextureMemory;
	i32       m_nShadowTexSize;

	float     m_fLastGamma;
	float     m_fLastBrightness;
	float     m_fLastContrast;
	float     m_fDeltaGamma;

	float     m_fogCullDistance;

	enum { nMeshPoolMaxTimeoutCounter = 150 /*150 ms*/};
	i32 m_nMeshPoolTimeoutCounter;

	// Cached verts/inds used for sprites
	SVF_P3F_C4B_T2F* m_pSpriteVerts;
	u16*          m_pSpriteInds;

	// Custom render modes states
	u32 m_nThermalVisionMode : 2;
	u32 m_nSonarVisionMode   : 2;
	u32 m_nNightVisionMode   : 2;

	i32    m_nFlushAllPendingTextureStreamingJobs;
	float  m_fTexturesStreamingGlobalMipFactor;

	SGpuInfo m_adapterInfo = {};
public:
	// these ids can be used for tripple (or more) buffered structures
	// they are incremented in RenderWorld on the mainthread
	// use m_nPoolIndex from the mainthread (or jobs which are synced before Renderworld)
	// and m_nPoolIndexRT from the renderthread
	// right now the skinning data pool and particle are using this id
	u32  m_nPoolIndex = 0;

	bool    m_bVendorLibInitialized;

	u32  m_nFrameLoad;
	u32  m_nFrameReset;
	u32  m_nFrameSwapID;             // without recursive calls, access through GetFrameID(false)

	ColorF  m_cClearColor;
	i32     m_NumResourceSlots;
	i32     m_NumSamplerSlots;

	////////////////////////////////////////////////////////
	// downscaling viewport information.

	// Set from DinrusSystem via IRenderer interface
	Vec2 m_ReqViewportScale;

	// Updated in RT_EndFrame. Fixed across the whole frame.
	Vec2 m_CurViewportScale;
	Vec2 m_PrevViewportScale;

	////////////////////////////////////////////////////////

	class CPostEffectsMgr* m_pPostProcessMgr;
	class CWater*          m_pWaterSimMgr;

	CTextureUpr*       m_pTextureUpr;

	// Used for pausing timer related stuff (eg: for texture animations, and shader 'time' parameter)
	bool  m_bPauseTimer;
	float m_fPrevTime;
	u8 m_nUseZpass : 2;
	bool  m_bCollectDrawCallsInfo;
	bool  m_bCollectDrawCallsInfoPerNode;

	S3DEngineCommon m_p3DEngineCommon;

	ShadowFrustumMGPUCache  m_ShadowFrustumMGPUCache;

	//Debug Gun
	IRenderNode*     m_pDebugRenderNode;

	const SWindGrid* m_pCurWindGrid;

	//=====================================================================
	// Shaders interface
	CShaderMan            m_cEF;
	_smart_ptr<IMaterial> m_pDefaultMaterial;
	_smart_ptr<IMaterial> m_pTerrainDefaultMaterial;

	i32                   m_TexGenID;

	IFFont*               m_pDefaultFont;

	static i32            m_iGeomInstancingThreshold; // internal value, auto mapped depending on GPU hardware, 0 means not set yet

	// Limit for local sorting array
	static i32k          nMaxParticleContainer = 8 * 1024;

	i32                       m_nCREParticleCount[RT_COMMAND_BUF_COUNT];
	JobUpr::SJobState     m_ComputeVerticesJobState;

	CFillRateUpr          m_FillRateUpr;

	FILE*                     m_LogFile;
	FILE*                     m_LogFileStr;
	FILE*                     m_LogFileSh;

protected:
	//================================================================================
	i32                                        m_cbpp, m_zbpp, m_sbpp;
	i32                                        m_wireframe_mode, m_wireframe_mode_prev;
	u32                                     m_nGPUs;                      // Use GetActiveGPUCount() to read
	float                                      m_drawNearFov;
	float                                      m_pixelAspectRatio;
	float                                      m_shadowJittering;
	StaticArray<i32, MAX_GSM_LODS_NUM>         m_CachedShadowsResolution;

	CSkinningDataPool                          m_SkinningDataPool[3];        // Tripple Buffered for motion blur
	std::array<std::vector<SSkinningData*>, 3> m_computeSkinningData;

	i32                                        m_cloudShadowTexId;
	Vec3                                       m_cloudShadowSpeed;
	float                                      m_cloudShadowTiling;
	bool                                       m_cloudShadowInvert;
	float                                      m_cloudShadowBrightness;
	i32                                        m_volumetricCloudTexId;
	i32                                        m_volumetricCloudNoiseTexId;
	i32                                        m_volumetricCloudEdgeNoiseTexId;

	// Shaders/Shaders support
	// RE - RenderElement
	bool m_bTimeProfileUpdated;
	i32  m_PrevProfiler;
	i32  m_nCurSlotProfiler;

	i32  m_beginFrameCount;

	typedef std::list<IRendererEventListener*> TListRendererEventListeners;
	TListRendererEventListeners               m_listRendererEventListeners;

	std::vector<ISyncMainWithRenderListener*> m_syncMainWithRenderListeners;

	DrxMutex             m_mtxStopAtRenderFrameEnd;
	DrxConditionVariable m_condStopAtRenderFrameEnd;

	ColorF m_highlightColor;
	ColorF m_SelectionColor;
	Vec4  m_highlightParams;

	// Separate render views per recursion
	SElementPool<CRenderView> m_pRenderViewPool[IRenderView::eViewType_Count];
	void InitRenderViewPool();

	// Temporary render objects storage
	struct STempObjects
	{
		std::shared_ptr<class CRenderObjectsPools> m_renderObjectsPools;
		// Array of render objects that need to be deleted next frame
		CThreadSafeRendererContainer<class CPermanentRenderObject*> m_persistentRenderObjectsToDelete[RT_COMMAND_BUF_COUNT];
	};
	STempObjects m_tempRenderObjects;

	// Resource deletion is delayed for at least 3 frames.
	CThreadSafeRendererContainer<CBaseResource*> m_resourcesToDelete[RT_COMMAND_BUF_COUNT];
	 i32 m_currentResourceDeleteBuffer = 0;

	// rounds ID from 3D engine, useful for texture streaming
	i32 m_streamZonesRoundId[MAX_PREDICTION_ZONES];

	i32 m_nRenderThreadFrameID = 0;

	struct SWaterUpdateInfo
	{
		float m_fLastWaterFOVUpdate;
		Vec3  m_LastWaterViewdirUpdate;
		Vec3  m_LastWaterUpdirUpdate;
		Vec3  m_LastWaterPosUpdate;
		float m_fLastWaterUpdate;
		i32   m_nLastWaterFrameID;
	};
	SWaterUpdateInfo m_waterUpdateInfo;

	// Antialiasing data.
	SMSAA    m_MSAAData;

	// Render frame statistics
	SRenderStatistics m_frameRenderStats[RT_COMMAND_BUF_COUNT];

	// Render target statistics
	std::vector<SRTargetStat> m_renderTargetStats;

	// Rendering Quality
	SRenderQuality    m_renderQuality;

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Animation time is used for rendering animation effects and can be paused if CRenderer::m_bPauseTimer is true
	CTimeValue     m_animationTime;
	// Time of the last main to renderer thread sync
	CTimeValue     m_frameSyncTime;
	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<CStandardGraphicsPipeline> m_pGraphicsPipeline;

public: // TEMPORARY PUBLIC
	friend struct SRenderThread;
	//////////////////////////////////////////////////////////////////////////
	// Render Thread support
	threadID     m_nFillThreadID;
	threadID     m_nProcessThreadID;
	//////////////////////////////////////////////////////////////////////////
	
private:
};

inline i32 CRenderer::RT_GetCurrGpuID() const
{
	return gRenDev->m_nFrameSwapID % gRenDev->GetActiveGPUCount();
}

inline i32 CRenderer::GetRenderFrameID() const
{
	assert(!m_pRT->IsMultithreaded() || m_pRT->IsRenderThread());
	return m_nRenderThreadFrameID;
}

inline i32 CRenderer::GetMainFrameID() const
{
	assert(!m_pRT->IsMultithreaded() || !m_pRT->IsRenderThread());
	return gEnv->nMainFrameID;
}

#include <drx3D/Render/CommonRender.h>

#define SKY_BOX_SIZE 32.f
