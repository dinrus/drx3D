// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   IShader.h : Shaders common interface.

   Revision история:
* Created by Anton Kaplanyan

   =============================================================================*/

#ifndef _ITEXTURE_H_
#define _ITEXTURE_H_

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include "Tarray.h"

class CTexture;

//! Texture types. Stored in files, can't change numeric value!
enum ETEX_Type : u8
{
	// regular hardware supported/native types
	eTT_1D          = 0,
	eTT_2D          = 1,
	eTT_2DArray     = 8,
	eTT_2DMS        = 9,
	eTT_3D          = 2,
	eTT_Cube        = 3,
	eTT_CubeArray   = 4,

	// custom types
	eTT_Dyn2D       = 5,
	eTT_Auto2D      = 10,
	eTT_User        = 6,
	eTT_NearestCube = 7,

	eTT_MaxTexType  = 11,   //!< Not used.
};

//! Texture formats.
enum ETEX_Format : u8
{
	eTF_Unknown  = 0,
	eTF_R8G8B8A8S,
	eTF_R8G8B8A8 = 2, //!< May be saved into file.

	eTF_R1,
	eTF_R8,
	eTF_R8S,
	eTF_R16,
	eTF_R16S,
	eTF_R16F,
	eTF_R32F,
	eTF_R8G8,
	eTF_R8G8S,
	eTF_R16G16,
	eTF_R16G16S,
	eTF_R16G16F,
	eTF_R32G32F,
	eTF_R11G11B10F,
	eTF_R10G10B10A2,
	eTF_R16G16B16A16,
	eTF_R16G16B16A16S,
	eTF_R16G16B16A16F,
	eTF_R32G32B32A32F,

	eTF_BC1 = 22, //!< May be saved into file.
	eTF_BC2 = 23, //!< May be saved into file.
	eTF_BC3 = 24, //!< May be saved into file.
	eTF_BC4U,     //!< 3Dc+.
	eTF_BC4S,
	eTF_BC5U,     //!< 3Dc.
	eTF_BC5S,
	eTF_BC6UH,
	eTF_BC6SH,
	eTF_BC7,
	eTF_CTX1,
	eTF_R9G9B9E5,
	eTF_A8,

	//! Hardware depth/stencil buffers.
	eTF_S8,
	eTF_D16,
	eTF_D16S8,
	eTF_D24,
	eTF_D24S8,
	eTF_D32F,
	eTF_D32FS8,

	//! Only available as hardware format under DX11.1 with DXGI 1.2.
	eTF_B5G6R5,
	eTF_B5G5R5A1,
	eTF_B4G4R4A4,

	//! Only available as hardware format under Vulkan or XBO.
	eTF_R4G4,
	eTF_R4G4B4A4,

	//! Only available as hardware format under OpenGL and Vulkan.
	eTF_EAC_R11,
	eTF_EAC_R11S,
	eTF_EAC_RG11,
	eTF_EAC_RG11S,
	eTF_ETC2,
	eTF_ETC2A,

	eTF_ASTC_LDR_4x4,

	//! Only available as hardware format under DX9.
	eTF_A8L8,
	eTF_L8,
	eTF_L8V8U8,
	eTF_B8G8R8,
	eTF_L8V8U8X8,
	eTF_B8G8R8X8,
	eTF_B8G8R8A8,

	eTF_YUV,
	eTF_YUVA,

	eTF_MaxFormat       //!< Unused, but must be always the last in the list.
};

enum ETEX_TileMode : u8
{
	eTM_None = 0,
	eTM_LinearPadded,
	eTM_Optimal,
	eTM_Unspecified = 0xFF
};

//! T = applies to texture objects read from disk.
//! R = applies to texture objects allocated for render-targets.
enum ETextureFlags : u32
{
	FT_NOMIPS                  = BIT(0),  // TR: don't allocate or use any mip-maps (even if they exist)
	FT_TEX_NORMAL_MAP          = BIT(1),  // T: indicator that a texture contains normal vectors (used for tracking statistics, debug messages and the default texture)
	FT______________________00 = BIT(2),  // UNUSED
	FT_USAGE_DEPTHSTENCIL      = BIT(3),  // R: use as depth-stencil render-target
	FT_USAGE_ALLOWREADSRGB     = BIT(4),  // TR: allows the renderer to cast the texture-format to a sRGB type if available
	FT_FILESINGLE              = BIT(5),  // T: suppress loading of additional files like _DDNDIF (faster, RC can tag the file for that)
	FT_TEX_FONT                = BIT(6),  // T: indicator that a texture contains font glyphs (used solely for tracking statistics!)
	FT_HAS_ATTACHED_ALPHA      = BIT(7),  // T: indicator that the texture has another texture attached (see FT_ALPHA)
	FT_USAGE_UNORDERED_ACCESS  = BIT(8),  // R: allow write-only UAVs for the texture object
	FT______________________01 = BIT(9),  // UNUSED
	FT_USAGE_MSAA              = BIT(10), // R: use as MSAA render-target
	FT_FORCE_MIPS              = BIT(11), // TR: always allocate mips (even if normally this would be optimized away)
	FT_USAGE_RENDERTARGET      = BIT(12), // R: use as render-target
	FT_USAGE_TEMPORARY         = BIT(13), // TR: indicate that the resource is used for just one or at most a couple of frames
	FT_STAGE_READBACK          = BIT(14), // R: allow read-back of the texture contents by the CPU through a persistent staging texture (otherwise the staging is dynamic)
	FT_STAGE_UPLOAD            = BIT(15), // R: allow up-load of the texture contents by the CPU through a persistent staging texture (otherwise the staging is dynamic)
	FT_DONT_RELEASE            = BIT(16), // TR: texture will not be freed automatically when ref counter goes to 0. Use ReleaseForce() to free the texture.
	FT_ASYNC_PREPARE           = BIT(17), // T: run the streaming preparation of this texture asynchronously
	FT_DONT_STREAM             = BIT(18), // T: prevent progressive streaming-in/out of the texture, it's fully loaded or not at all
	FT_DONT_READ               = BIT(19), // TR: the texture is write-only (backbuffers fe.)
	FT_FAILED                  = BIT(20), // TR: indicator that the allocation of the texture failed the last time it has been tried, for render-targets this is fatal
	FT_FROMIMAGE               = BIT(21), // T: indicator that the textures originates from disk
	FT_STATE_CLAMP             = BIT(22), // T: set the sampling mode to clamp in the corresponding sampler-state
	FT_USAGE_ATLAS             = BIT(23), // R: indicator that the texture's contents form an atlas or flip-book (used solely for tracking statistics!)
	FT_ALPHA                   = BIT(24), // T: indicator that this is a texture attached to another one (see FT_HAS_ATTACHED_ALPHA)
	FT_REPLICATE_TO_ALL_SIDES  = BIT(25), // T: Replicate single texture to all cube-map sides on allocation and on file-load
	FT_KEEP_LOWRES_SYSCOPY     = BIT(26), // ?: keep low res copy in system memory for voxelization on CPU
	FT_SPLITTED                = BIT(27), // T: indicator that the texture is available splitted on disk
	FT_STREAMED_PREPARE        = BIT(28), // REMOVE
	FT_STREAMED_FADEIN         = BIT(29), // T: smoothly fade the texture in after MIPs have been added
	FT_USAGE_UAV_OVERLAP       = BIT(30), // R: disable compute-serialization when concurrently using this UAV
	FT_USAGE_UAV_RWTEXTURE     = BIT(31), // R: enable RW usage for the UAV, otherwise UAVs are write-only (see FT_USAGE_UNORDERED_ACCESS)
};

DEFINE_ENUM_FLAG_OPERATORS(ETextureFlags);

//////////////////////////////////////////////////////////////////////
enum class EFilterPreset : u8
{
	Unspecified    = 255,

	Point          = 0,
	Linear         = 1,
	Bilinear       = 2,
	Trilinear      = 3,
	Anisotropic2x  = 4,
	Anisotropic4x  = 5,
	Anisotropic8x  = 6,
	Anisotropic16x = 7
};
static_assert(EFilterPreset::Unspecified == EFilterPreset(~0), "Bad unspecified value");

#define FILTER_NONE      -1
#define FILTER_POINT     0
#define FILTER_LINEAR    1
#define FILTER_BILINEAR  2
#define FILTER_TRILINEAR 3
#define FILTER_ANISO2X   4
#define FILTER_ANISO4X   5
#define FILTER_ANISO8X   6
#define FILTER_ANISO16X  7

struct SamplerStateHandle
{
	typedef u16 ValueType;
	ValueType value;

	constexpr SamplerStateHandle() : value(Unspecified) { }
	constexpr SamplerStateHandle(ValueType v) : value(v) { }

	// Test operators
	template<typename T> bool operator ==(const T other) const { return value == other; }
	template<typename T> bool operator !=(const T other) const { return value != other; }
	// Range operators
	template<typename T> bool operator <=(const T other) const { return value <= other; }
	template<typename T> bool operator >=(const T other) const { return value >= other; }
	// Sorting operators
	template<typename T> bool operator < (const T other) const { return value <  other; }
	template<typename T> bool operator > (const T other) const { return value >  other; }

	// Auto cast for array access operator []
	operator ValueType() const { return value; }

	// Not an enum, because of SWIG
	static constexpr ValueType Unspecified = ValueType(~0);
};

//////////////////////////////////////////////////////////////////////
struct SDepthTexture;

struct STextureStreamingStats
{
	STextureStreamingStats(bool bComputeTexturesPerFrame) : bComputeReuquiredTexturesPerFrame(bComputeTexturesPerFrame)
	{
		nMaxPoolSize = 0;
		nCurrentPoolSize = 0;
		nStreamedTexturesSize = 0;
		nStaticTexturesSize = 0;
		nNumStreamingRequests = 0;
		nThroughput = 0;
		nNumTexturesPerFrame = 0;
		nRequiredStreamedTexturesSize = 0;
		nRequiredStreamedTexturesCount = 0;
		bPoolOverflow = false;
		bPoolOverflowTotally = false;
		fPoolFragmentation = 0.0f;
	}
	size_t     nMaxPoolSize;
	size_t     nCurrentPoolSize;
	size_t     nStreamedTexturesSize;
	size_t     nStaticTexturesSize;
	size_t     nNumStreamingRequests;
	u32     nNumTexturesPerFrame;
	size_t     nThroughput;
	size_t     nRequiredStreamedTexturesSize;
	u32     nRequiredStreamedTexturesCount;
	float      fPoolFragmentation;
	u32     bPoolOverflow        : 1;
	u32     bPoolOverflowTotally : 1;
	const bool bComputeReuquiredTexturesPerFrame;
};

struct STexData
{
	u8*      m_pData[6];
	u16      m_nWidth;
	u16      m_nHeight;
	u16      m_nDepth;
protected:
	u8       m_reallocated;
public:
	ETEX_Format m_eFormat;
	u8       m_nMips;
	i32         m_nFlags;
	float       m_fAvgBrightness;
	ColorF      m_cMinColor;
	ColorF      m_cMaxColor;
	tukk m_pFilePath;

	STexData()
	{
		m_pData[0] = m_pData[1] = m_pData[2] = m_pData[3] = m_pData[4] = m_pData[5] = 0;
		m_nWidth = 0;
		m_nHeight = 0;
		m_nDepth = 1;
		m_reallocated = 0;
		m_eFormat = eTF_Unknown;
		m_nMips = 0;
		m_nFlags = 0;
		m_fAvgBrightness = 1.0f;
		m_cMinColor = 0.0f;
		m_cMaxColor = 1.0f;
		m_pFilePath = 0;
	}
	void AssignData(u32 i, u8* pNewData)
	{
		assert(i < 6);
		if (WasReallocated(i))
			delete[] m_pData[i];
		m_pData[i] = pNewData;
		SetReallocated(i);
	}
	bool WasReallocated(u32 i) const
	{
		return (m_reallocated & (1 << i)) != 0;
	}
	void SetReallocated(u32 i)
	{
		m_reallocated |= (1 << i);
	}
};

//! Texture object interface.
class ITexture
{
protected:
	virtual ~ITexture() {}
public:
	// <interfuscator:shuffle>
	virtual i32             AddRef() = 0;
	virtual i32             Release() = 0;
	virtual i32             ReleaseForce() = 0;

	virtual tukk     GetName() const = 0;
	virtual i32k       GetWidth() const = 0;
	virtual i32k       GetHeight() const = 0;
	virtual i32k       GetDepth() const = 0;
	virtual i32k       GetTextureID() const = 0;
	virtual u32k    GetFlags() const = 0;
	virtual i32k       GetNumMips() const = 0;
	virtual i32k       GetRequiredMip() const = 0;
	virtual i32k       GetDeviceDataSize() const = 0;
	virtual i32k       GetDataSize() const = 0;
	virtual const ETEX_Type GetTextureType() const = 0;
	virtual const bool      IsTextureLoaded() const = 0;
	virtual void            PrecacheAsynchronously(float fMipFactor, i32 nFlags, i32 nUpdateId, i32 nCounter = 1) = 0;
	virtual u8*          GetData32(i32 nSide = 0, i32 nLevel = 0, u8* pDst = NULL, ETEX_Format eDstFormat = eTF_R8G8B8A8) = 0;
	virtual bool            SetFilter(i32 nFilter) = 0; //!< FILTER_ flags.
	virtual void            SetClamp(bool bEnable) = 0; //!< Texture addressing set.
	virtual float           GetAvgBrightness() const = 0;

	virtual bool            Clear() = 0;
	virtual bool            Clear(const ColorF& color) = 0;

	virtual i32             StreamCalculateMipsSigned(float fMipFactor) const = 0;
	virtual i32             GetStreamableMipNumber() const = 0;
	virtual i32             GetStreamableMemoryUsage(i32 nStartMip) const = 0;
	virtual i32             GetMinLoadedMip() const = 0;

	//! Used for debugging/profiling.
	virtual tukk       GetFormatName() const = 0;
	virtual tukk       GetTypeName() const = 0;
	virtual const bool        IsStreamedVirtual() const = 0;
	virtual const bool        IsShared() const = 0;
	virtual const bool        IsStreamable() const = 0;
	virtual bool              IsStreamedIn(i32k nMinPrecacheRoundIds[2]) const = 0;
	virtual i32k         GetAccessFrameId() const = 0;
	virtual i32k         GetCustomID() const = 0;
	virtual void              SetCustomID(i32 nID) = 0;

	virtual const ETEX_Format GetTextureDstFormat() const = 0;
	virtual const ETEX_Format GetTextureSrcFormat() const = 0;

	virtual bool              IsPostponed() const = 0;
	virtual const bool        IsParticularMipStreamed(float fMipFactor) const = 0;

	//! Get low res system memory (used for CPU voxelization).
	virtual const ColorB* GetLowResSystemCopy(u16& nWidth, u16& nHeight, i32** ppLowResSystemCopyAtlasId) { return 0; }

	virtual void UpdateData(STexData &td, i32 flags) = 0;
	// </interfuscator:shuffle>

	void         GetMemoryUsage(IDrxSizer* pSizer) const
	{
		static_assert(eTT_MaxTexType <= 255, "Constant value is too high!");
		static_assert(eTF_MaxFormat <= 255, "Constant value is too high!");
		/*LATER*/
	}

	virtual void SetKeepSystemCopy(const bool bKeepSystemCopy) = 0;
};

//=========================================================================================

struct IDynTextureSource
{
	enum EDynTextureSource
	{
		DTS_UNKNOWN,

		DTS_I_FLASHPLAYER
	};

	enum EDynTextureRTType
	{
		DTS_RT_SHARED,
		DTS_RT_UNIQUE
	};

	// <interfuscator:shuffle>
	virtual void              AddRef() = 0;
	virtual void              Release() = 0;
	virtual uk             GetSourceTemp(EDynTextureSource type) const = 0;
	virtual uk             GetSourcePerm(EDynTextureSource type) = 0;
	virtual tukk       GetSourceFilePath() const = 0;
	virtual EDynTextureSource GetSourceType() const = 0;
	virtual EDynTextureRTType GetRTType() const = 0;
	virtual ITexture*         GetTexture() const = 0;

	virtual void              EnablePerFrameRendering(bool enable) = 0;
	virtual void              Activate(bool activate) = 0;
	// </interfuscator:shuffle>
#if defined(ENABLE_DYNTEXSRC_PROFILING)
	virtual string GetProfileInfo() const = 0;
#endif

protected:
	virtual ~IDynTextureSource() {}
};

//=========================================================================================

class IDynTexture
{
public:
	enum
	{
		fNeedRegenerate = 1ul << 0,
	};
	// <interfuscator:shuffle>
	virtual ~IDynTexture(){}
	virtual void      Release() = 0;
	virtual void      GetSubImageRect(u32& nX, u32& nY, u32& nWidth, u32& nHeight) = 0;
	virtual void      GetImageRect(u32& nX, u32& nY, u32& nWidth, u32& nHeight) = 0;
	virtual i32       GetTextureID() = 0;
	virtual void      Lock() = 0;
	virtual void      UnLock() = 0;
	virtual i32       GetWidth() = 0;
	virtual i32       GetHeight() = 0;
	virtual bool      IsValid() = 0;
	virtual u8     GetFlags() const = 0;
	virtual void      SetFlags(u8 flags) {}
	virtual bool      Update(i32 nNewWidth, i32 nNewHeight) = 0;
	virtual bool      SetRectStates() = 0;
	virtual ITexture* GetTexture() = 0;
	virtual void      SetUpdateMask() = 0;
	virtual void      ResetUpdateMask() = 0;
	virtual bool      IsSecondFrame() = 0;
	virtual bool      GetImageData32(u8* pData, i32 nDataSize) { return 0; }
	// </interfuscator:shuffle>
};

//! \cond INTERNAL
//! Animating Texture sequence definition.
struct STexAnim
{
	i32               m_nRefCount;
	TArray<CTexture*> m_TexPics;
	i32               m_Rand;
	i32               m_NumAnimTexs;
	bool              m_bLoop;
	float             m_Time;

	i32               Size()
	{
		i32 nSize = sizeof(STexAnim);
		nSize += m_TexPics.GetMemoryUsage();
		return nSize;
	}
	void Release()
	{
		long refCnt = DrxInterlockedDecrement(&m_nRefCount);
		if (refCnt > 0)
			return;
		delete this;
	}
	void AddRef()
	{
		DrxInterlockedIncrement(&m_nRefCount);
	}

	STexAnim()
	{
		m_nRefCount = 1;
		m_Rand = 0;
		m_NumAnimTexs = 0;
		m_bLoop = false;
		m_Time = 0.0f;
	}

	~STexAnim()
	{
		for (u32 i = 0; i < m_TexPics.Num(); i++)
		{
			ITexture* pTex = (ITexture*) m_TexPics[i];
			SAFE_RELEASE(pTex);
		}
		m_TexPics.Free();
	}

	STexAnim& operator=(const STexAnim& sl)
	{
		// make sure not same object
		if (this == &sl)
		{
			return *this;
		}

		for (u32 i = 0; i < m_TexPics.Num(); i++)
		{
			ITexture* pTex = (ITexture*)m_TexPics[i];
			SAFE_RELEASE(pTex);
		}
		m_TexPics.Free();

		for (u32 i = 0; i < sl.m_TexPics.Num(); i++)
		{
			ITexture* pTex = (ITexture*)sl.m_TexPics[i];
			if (pTex)
			{
				pTex->AddRef();
			}

			m_TexPics.AddElem(sl.m_TexPics[i]);
		}

		m_Rand = sl.m_Rand;
		m_NumAnimTexs = sl.m_NumAnimTexs;
		m_bLoop = sl.m_bLoop;
		m_Time = sl.m_Time;

		return *this;
	}
};
//! \endcond

#endif// _ITEXTURE_H_
