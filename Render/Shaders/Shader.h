// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   Shader.h : Shaders declarations.

   Revision история:
* Created by Honich Andrey

   =============================================================================*/

#ifndef __SHADER_H__
#define __SHADER_H__

#include <drx3D/Render/Defs.h>

#include <drx3D/CoreX/String/DrxName.h>
#include <drx3D/CoreX/Renderer/IShader.h>
#include <drx3D/Render/CommonRender.h> // CBaseResource
#include <drx3D/Render/Shaders/ShaderComponents.h>

#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/DrxVariant.h>

#include <drx3D/Render/ResFile.h>

#include <memory>
#include <string>

// bump this value up if you want to invalidate shader cache (e.g. changed some code or .ext file)
// #### VIP NOTE ####: DON'T USE MORE THAN ONE DECIMAL PLACE!!!! else it doesn't work...
#define FX_CACHE_VER     0.3
#define FX_SER_CACHE_VER 1.2    // Shader serialization version (FX_CACHE_VER + FX_SER_CACHE_VER)

// Maximum 1 digit here
// The version determines the parse logic in the shader cache gen, these values cannot overlap
#define SHADER_LIST_VER      2
#define SHADER_SERIALISE_VER (SHADER_LIST_VER + 1)

//#define SHADER_NO_SOURCES 1 // If this defined all binary shaders (.fxb) should be located in Game folder (not user)

#define SHADERS_SERIALIZING 1 // Enables shaders serializing (Export/Import) to/from .fxb files

#define CB_PER_DRAW         eConstantBufferShaderSlot_PerDraw           // Non-Scene-Pass only
#define CB_PER_MATERIAL     eConstantBufferShaderSlot_PerMaterial       // Scene-Pass only
#define CB_NUM              2

//====================================================================
// Fixed Per-Material constants
// Needs to match shader registers in FXConstantDefs

#define NUM_PM_CONSTANTS             (10 + EFTT_MAX * 2)
#define FIRST_REG_PM                 (0)

#define REG_PM_CHANNELS_SB           (0)                  // Scale-Bias for each texture slot
#define REG_PM_DIFFUSE_COL           (0 + EFTT_MAX * 2)
#define REG_PM_SPECULAR_COL          (1 + EFTT_MAX * 2)
#define REG_PM_EMISSIVE_COL          (2 + EFTT_MAX * 2)
#define REG_PM_TCM_MATRIX            (3 + EFTT_MAX * 2)
#define REG_PM_DEFORM_WAVE           (7 + EFTT_MAX * 2)
#define REG_PM_DETAILTILING_ALPHAREF (8 + EFTT_MAX * 2)
#define REG_PM_SILPOM_DETAIL_PARAMS  (9 + EFTT_MAX * 2)

// Shader.h
// Shader pipeline common declarations.

struct SShaderPass;
class CShader;
class CRenderElement;
class CResFile;
struct SEnvTexture;
struct SParserFrame;
struct SPreprocessTree;
struct SEmptyCombination;
struct SShaderCombination;
struct SCGParam;
struct SSFXParam;
struct SSFXSampler;
struct SSFXTexture;
class CShaderResources;
struct SShaderCombIdent;
struct SCacheCombination;

enum eCompareFunc
{
	eCF_Disable,
	eCF_Never,
	eCF_Less,
	eCF_Equal,
	eCF_LEqual,
	eCF_Greater,
	eCF_NotEqual,
	eCF_GEqual,
	eCF_Always
};

struct SPair
{
	string m_szMacroName;
	string m_szMacro;
	u32 m_nMask;
};

#if DRX_PLATFORM_MOBILE
	#define GEOMETRYSHADER_SUPPORT false
#else
	#define GEOMETRYSHADER_SUPPORT true
#endif

struct SFXSampler
{
	CDrxNameR           m_Name; // Parameter name
	std::vector<u32> m_dwName;
	u32              m_nFlags;
	short               m_nArray;      // Number of samplers
	CDrxNameR           m_Annotations; // Additional parameters (between <>)
	CDrxNameR           m_Semantic;    // Parameter semantic type (after ':')
	CDrxNameR           m_Values;      // Parameter values (after '=')
	byte                m_eType;       // ESamplerType
	short               m_nRegister[eHWSC_Num];
	SamplerStateHandle  m_nTexState;
	SFXSampler()
	{
		m_nTexState = EDefaultSamplerStates::Unspecified;
		m_nArray = 0;
		m_nFlags = 0;
		for (i32 i = 0; i < eHWSC_Num; i++)
		{
			m_nRegister[i] = 10000;
		}
	}
	~SFXSampler()
	{
	}
	u32 GetFlags() { return m_nFlags; }
	void   PostLoad(class CParserBin& Parser, SParserFrame& Name, SParserFrame& Annotations, SParserFrame& Values, SParserFrame& Assign);
	bool   Export(SShaderSerializeContext& SC);
	bool   Import(SShaderSerializeContext& SC, SSFXSampler* pPR);
	u32 Size()
	{
		u32 nSize = sizeof(SFXSampler);
		//nSize += m_Name.capacity();
		nSize += sizeofVector(m_dwName);
		//nSize += m_Values.capacity();
		return nSize;
	}
	inline bool operator==(const SFXSampler& m) const
	{
		if (m_Name == m.m_Name && m_Annotations == m.m_Annotations && m_Semantic == m.m_Semantic && m_Values == m.m_Values &&
		    m_nArray == m.m_nArray && m_nFlags == m.m_nFlags && m_nRegister[0] == m.m_nRegister[0] && m_nRegister[1] == m.m_nRegister[1] &&
		    m_eType == m.m_eType && m_nTexState == m.m_nTexState)
			return true;
		return false;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//pSizer->AddObject( m_Name );
		//pSizer->AddObject( m_Values );
		pSizer->AddObject(m_dwName);
	}
};

struct SFXTexture
{
	CDrxNameR           m_Name; // Texture name
	std::vector<u32> m_dwName;
	u32              m_nFlags;
	u32              m_nTexFlags;
	short               m_nArray;      // Number of textures
	CDrxNameR           m_Annotations; // Additional parameters (between <>)
	CDrxNameR           m_Semantic;    // Parameter semantic type (after ':')
	CDrxNameR           m_Values;      // Parameter values (after '=')
	string              m_szTexture;   // Texture source name
	SHRenderTarget*     m_pTarget = nullptr;
	string              m_szUIName;    // UI name
	string              m_szUIDesc;    // UI description
	bool                m_bSRGBLookup; // Lookup
	byte                m_eType;       // ETextureType
	byte                m_Type;        // Data type (float, float4, etc)
	short               m_nRegister[eHWSC_Num];
	SFXTexture()
	{
		m_bSRGBLookup = false;
		m_Type = 0;
		m_nTexFlags = 0;
		m_nArray = 0;
		m_nFlags = 0;
		for (i32 i = 0; i < eHWSC_Num; i++)
		{
			m_nRegister[i] = 10000;
		}
	}
	~SFXTexture()
	{
	}
	u32 GetFlags()    { return m_nFlags; }
	u32 GetTexFlags() { return m_nTexFlags; }
	void   PostLoad(class CParserBin& Parser, SParserFrame& Name, SParserFrame& Annotations, SParserFrame& Values, SParserFrame& Assign);
	bool   Export(SShaderSerializeContext& SC);
	bool   Import(SShaderSerializeContext& SC, SSFXTexture* pPR);
	u32 Size()
	{
		u32 nSize = sizeof(SFXTexture);
		//nSize += m_Name.capacity();
		nSize += sizeofVector(m_dwName);
		//nSize += m_Values.capacity();
		return nSize;
	}
	inline bool operator==(const SFXTexture& m) const
	{
		if (m_Name == m.m_Name && m_Annotations == m.m_Annotations && m_Semantic == m.m_Semantic && m_Values == m.m_Values &&
		    m_nArray == m.m_nArray && m_nFlags == m.m_nFlags && m_nRegister[0] == m.m_nRegister[0] && m_nRegister[1] == m.m_nRegister[1] &&
		    m_eType == m.m_eType && m_bSRGBLookup == m.m_bSRGBLookup && m_szTexture == m.m_szTexture)
			return true;
		return false;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//pSizer->AddObject( m_Name );
		//pSizer->AddObject( m_Values );
		pSizer->AddObject(m_dwName);
	}
};

// In Matrix 3x4: m_nParams = 3, m_nComps = 4
struct SFXParam
{
	CDrxNameR           m_Name; // Parameter name
	std::vector<u32> m_dwName;
	u32              m_nFlags;
	short               m_nParameters; // Number of parameters
	short               m_nComps;      // Number of components in single parameter
	CDrxNameR           m_Annotations; // Additional parameters (between <>)
	CDrxNameR           m_Semantic;    // Parameter semantic type (after ':')
	CDrxNameR           m_Values;      // Parameter values (after '=')
	byte                m_eType;       // EParamType
	int8                m_nCB;
	short               m_nRegister[eHWSC_Num];
	SFXParam()
	{
		m_nParameters = 0;
		m_nComps = 0;
		m_nCB = -1;
		m_nFlags = 0;
		m_nRegister[0] = 10000;
		m_nRegister[1] = 10000;
		m_nRegister[2] = 10000;
		m_nRegister[3] = 10000;
		m_nRegister[4] = 10000;
		m_nRegister[5] = 10000;
	}
	~SFXParam()
	{
		i32 nnn = 0;
	}
	u32 GetComponent(EHWShaderClass eSHClass);
	void   GetParamComp(u32 nOffset, DrxFixedStringT<128>& param);
	u32 GetParamFlags() { return m_nFlags; }
	void   GetCompName(u32 nId, DrxFixedStringT<128>& name);
	string GetValueForName(tukk szName, EParamType& eType);
	void   PostLoad(class CParserBin& Parser, SParserFrame& Name, SParserFrame& Annotations, SParserFrame& Values, SParserFrame& Assign);
	void   PostLoad();
	bool   Export(SShaderSerializeContext& SC);
	bool   Import(SShaderSerializeContext& SC, SSFXParam* pPR);
	u32 Size()
	{
		u32 nSize = sizeof(SFXParam);
		//nSize += m_Name.capacity();
		nSize += sizeofVector(m_dwName);
		//nSize += m_Values.capacity();
		return nSize;
	}
	inline bool operator==(const SFXParam& m) const
	{
		if (m_Name == m.m_Name && m_Annotations == m.m_Annotations && m_Semantic == m.m_Semantic && m_Values == m.m_Values &&
		    m_nParameters == m.m_nParameters && m_nComps == m.m_nComps && m_nFlags == m.m_nFlags && m_nRegister[0] == m.m_nRegister[0] && m_nRegister[1] == m.m_nRegister[1] &&
		    m_eType == m.m_eType)
			return true;
		return false;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//pSizer->AddObject( m_Name );
		//pSizer->AddObject( m_Values );
		pSizer->AddObject(m_dwName);
	}
};

struct STokenD
{
	//std::vector<i32> Offsets;
	u32   Token;
	string   SToken;
	unsigned Size()                                  { return sizeof(STokenD) /*+ sizeofVector(Offsets)*/ + SToken.capacity(); }
	void     GetMemoryUsage(IDrxSizer* pSizer) const { pSizer->AddObject(SToken); }
};
typedef std::vector<STokenD>    FXShaderToken;
typedef FXShaderToken::iterator FXShaderTokenItor;

struct SFXStruct
{
	string m_Name;
	string m_Struct;
	SFXStruct()
	{
	}
};

enum ETexFilter
{
	eTEXF_None,
	eTEXF_Point,
	eTEXF_Linear,
	eTEXF_Anisotropic,
};

//=============================================================================

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#pragma warning( push )               //AMD Port
	#pragma warning( disable : 4267 )
#endif

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#pragma warning( pop )                //AMD Port
#endif

//=============================================================================
// Vertex programms / Vertex shaders (VP/VS)

//=====================================================================

static inline float* sfparam(Vec3 param)
{
	static float sparam[4];
	sparam[0] = param.x;
	sparam[1] = param.y;
	sparam[2] = param.z;
	sparam[3] = 1.0f;

	return &sparam[0];
}

static inline float* sfparam(float param)
{
	static float sparam[4];
	sparam[0] = param;
	sparam[1] = 0;
	sparam[2] = 0;
	sparam[3] = 1.0f;
	return &sparam[0];
}

static inline float* sfparam(float param0, float param1, float param2, float param3)
{
	static float sparam[4];
	sparam[0] = param0;
	sparam[1] = param1;
	sparam[2] = param2;
	sparam[3] = param3;
	return &sparam[0];
}

inline tuk sGetFuncName(tukk pFunc)
{
	static char func[128];
	tukk b = pFunc;
	if (*b == '[')
	{
		tukk s = strchr(b, ']');
		if (s)
			b = s + 1;
		while (*b <= 0x20)
		{
			b++;
		}
	}
	while (*b > 0x20)
	{
		b++;
	}
	while (*b <= 0x20)
	{
		b++;
	}
	i32 n = 0;
	while (*b > 0x20 && *b != '(')
	{
		func[n++] = *b++;
	}
	func[n] = 0;

	return func;
}

enum ERenderOrder
{
	eRO_PreProcess,
	eRO_PostProcess,
	eRO_PreDraw,
	eRO_Managed
};

enum ERTUpdate
{
	eRTUpdate_Unknown,
	eRTUpdate_Always,
	eRTUpdate_WaterReflect
};

struct SHRenderTarget : public IRenderTarget
{
	i32          m_nRefCount;
	ERenderOrder m_eOrder;
	i32          m_nProcessFlags; // FSPR_ flags
	string       m_TargetName;
	i32          m_nWidth;
	i32          m_nHeight;
	ETEX_Format  m_eTF;
	i32          m_nIDInPool;
	ERTUpdate    m_eUpdateType;
	CTexture*    m_pTarget;
	bool         m_bTempDepth;
	ColorF       m_ClearColor;
	float        m_fClearDepth;
	u32       m_nFlags;
	u32       m_nFilterFlags;
	i32          m_refSamplerID;

	SHRenderTarget()
	{
		m_nRefCount = 1;
		m_eOrder = eRO_PreProcess;
		m_pTarget = nullptr;
		m_bTempDepth = true;
		m_ClearColor = Col_Black;
		m_fClearDepth = 1.f;
		m_nFlags = 0;
		m_nFilterFlags = 0xffffffff;
		m_nProcessFlags = 0;
		m_nIDInPool = -1;
		m_nWidth = 256;
		m_nHeight = 256;
		m_eTF = eTF_R8G8B8A8;
		m_eUpdateType = eRTUpdate_Unknown;
		m_refSamplerID = -1;
	}
	virtual void Release()
	{
		m_nRefCount--;
		if (m_nRefCount)
			return;
		delete this;
	}
	virtual void AddRef()
	{
		m_nRefCount++;
	}
	SEnvTexture* GetEnv2D();
	SEnvTexture* GetEnvCM();

	void         GetMemoryUsage(IDrxSizer* pSizer) const;
};

//=============================================================================
// Hardware shaders

#define SHADER_BIND_TEXTURE 0x2000
#define SHADER_BIND_SAMPLER 0x4000

//=============================================================================

struct SShaderCacheHeaderItem
{
	byte   m_nVertexFormat = 0;
	byte   m_Class = 0;
	byte   m_nInstBinds = 0;
	byte   m_StreamMask_Stream = 0;
	u32 m_CRC32 = 0;
	u16 m_StreamMask_Decl = 0;
	i16  m_nInstructions = 0;

	AUTO_STRUCT_INFO;
};

#define MAX_VAR_NAME 512
struct SShaderCacheHeaderItemVar
{
	i32   m_Reg;
	short m_nCount;
	char  m_Name[MAX_VAR_NAME];
	SShaderCacheHeaderItemVar()
	{
		memset(this, 0, sizeof(SShaderCacheHeaderItemVar));
	}
};

struct SCompressedData
{
	byte*  m_pCompressedShader;
	u32 m_nSizeCompressedShader;
	u32 m_nSizeDecompressedShader;

	SCompressedData()
	{
		m_pCompressedShader = NULL;
		m_nSizeCompressedShader = 0;
		m_nSizeDecompressedShader = 0;
	}
	i32 Size()
	{
		i32 nSize = sizeof(SCompressedData);
		if (m_pCompressedShader)
			nSize += m_nSizeCompressedShader;
		return nSize;
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		//pSizer->AddObject(this, sizeof(SCompressedData));
		if (m_pCompressedShader)
			pSizer->AddObject(m_pCompressedShader, m_nSizeCompressedShader);
	}
};

enum class cacheSource : uint8_t
{
	readonly = 0,       // Shader cache from readonly paks
	user = 1,           // Writeable shader cache
};

struct SOptimiseStats
{
	i32 nEntries;
	i32 nUniqueEntries;
	i32 nSizeUncompressed;
	i32 nSizeCompressed;
	i32 nTokenDataSize;
	i32 nDirDataSize;
	SOptimiseStats()
	{
		nEntries = 0;
		nUniqueEntries = 0;
		nSizeUncompressed = 0;
		nSizeCompressed = 0;
		nTokenDataSize = 0;
		nDirDataSize = 0;
	}
};

//====================================================================
// HWShader run-time flags
// Note:we are limited to a maximum of 64, check HWSR_MAX before adding

enum EHWSRMaskBit
{
	HWSR_FOG = 0,

	HWSR_ALPHATEST,
	HWSR_ALPHABLEND,

	HWSR_MSAA_QUALITY,
	HWSR_MSAA_QUALITY1,
	HWSR_MSAA_SAMPLEFREQ_PASS,

	HWSR_SECONDARY_VIEW,

	HWSR_VERTEX_VELOCITY,
	HWSR_SKELETON_SSD,
	HWSR_SKELETON_SSD_LINEAR,
	HWSR_COMPUTE_SKINNING,

	HWSR_OBJ_IDENTITY,
	HWSR_NEAREST,
	HWSR_DISSOLVE,
	HWSR_NO_TESSELLATION,

	HWSR_QUALITY,
	HWSR_QUALITY1,

	HWSR_SAMPLE0,
	HWSR_SAMPLE1,
	HWSR_SAMPLE2,
	HWSR_SAMPLE3,
	HWSR_SAMPLE4,
	HWSR_SAMPLE5,
	HWSR_SAMPLE6,

	HWSR_DEBUG0,
	HWSR_DEBUG1,
	HWSR_DEBUG2,
	HWSR_DEBUG3,

	HWSR_CUBEMAP0,

	HWSR_DECAL_TEXGEN_2D,

	HWSR_SHADOW_MIXED_MAP_G16R16,
	HWSR_HW_PCF_COMPARE,
	HWSR_SHADOW_JITTERING,
	HWSR_POINT_LIGHT,
	HWSR_LIGHT_TEX_PROJ,

	HWSR_BLEND_WITH_TERRAIN_COLOR,
	HWSR_AMBIENT_OCCLUSION,

	HWSR_PARTICLE_SHADOW,
	HWSR_SOFT_PARTICLE,
	HWSR_OCEAN_PARTICLE,
	HWSR_ANIM_BLEND,
	HWSR_ENVIRONMENT_CUBEMAP,
	HWSR_MOTION_BLUR,

	HWSR_SPRITE,

	HWSR_LIGHTVOLUME0,
	HWSR_LIGHTVOLUME1,

	HWSR_TILED_SHADING,

	HWSR_VOLUMETRIC_FOG,

	HWSR_REVERSE_DEPTH,

	HWSR_PROJECTION_MULTI_RES,
	HWSR_PROJECTION_LENS_MATCHED,
	HWSR_MAX
};

extern uint64 g_HWSR_MaskBit[HWSR_MAX];

// HWShader global flags (m_Flags)
#define HWSG_SUPPORTS_LIGHTING    0x20
#define HWSG_SUPPORTS_MULTILIGHTS 0x40
#define HWSG_SUPPORTS_MODIF       0x80
#define HWSG_SUPPORTS_VMODIF      0x100
#define HWSG_WASGENERATED         0x200
#define HWSG_NOSPECULAR           0x400
#define HWSG_SYNC                 0x800
#define HWSG_CACHE_USER           0x1000
//#define HWSG_AUTOENUMTC      0x1000
#define HWSG_UNIFIEDPOS           0x2000
#define HWSG_DEFAULTPOS           0x4000
#define HWSG_PROJECTED            0x8000
#define HWSG_NOISE                0x10000
#define HWSG_FP_EMULATION         0x40000
#define HWSG_GS_MULTIRES          0x80000

// HWShader per-instance Modificator flags (SHWSInstance::m_MDMask)
// Vertex shader specific

// Texture projected flags
#define HWMD_TEXCOORD_PROJ              0x1
// Texture transform flag
#define HWMD_TEXCOORD_MATRIX            0x100
// Object linear texgen flag
#define HWMD_TEXCOORD_GEN_OBJECT_LINEAR 0x1000

#define HWMD_TEXCOORD_FLAG_MASK         (0xfffff000 | 0xf00)

// HWShader per-instance vertex modificator flags (SHWSInstance::m_MDVMask)
// Texture projected flags (4 bits)
#define HWMDV_TYPE 0

// HWShader input flags (passed via mfSet function)
#define HWSF_SETPOINTERSFORSHADER 1
#define HWSF_SETPOINTERSFORPASS   2
#define HWSF_PRECACHE             4
#define HWSF_SETTEXTURES          8
#define HWSF_FAKE                 0x10

#define HWSF_INSTANCED            0x20
#define HWSF_NEXT                 0x100
#define HWSF_PRECACHE_INST        0x200
#define HWSF_STORECOMBINATION     0x400
#define HWSF_STOREDATA            0x800

class CHWShader;
struct SDiskShaderCache
{
	struct recreateUserCacheTag {};

	SDiskShaderCache(tukk name, cacheSource cacheType);
	SDiskShaderCache(recreateUserCacheTag, tukk name, uint32_t CRC32, float cacheVer);
	~SDiskShaderCache() noexcept;

	CResFile* m_pRes = nullptr;

	std::pair<std::unique_ptr<byte[]>, u32> DecompressResource(CResFileOpenScope &scope, size_t offset, size_t size) const;

	void GetMemoryUsage(IDrxSizer* pSizer) const;
	cacheSource GetType() const { return cacheType; }

#if DRX_PLATFORM_DESKTOP
	bool mfOptimiseCacheFile(SOptimiseStats* Stats);
#endif

private:
	const cacheSource cacheType = cacheSource::readonly;

	bool OpenCacheFileImpl(cacheSource cacheType, CResFile* pRF);
	bool OpenCacheFile(tukk szName, cacheSource src);
};

struct SDeviceShaderEntry
{
	SShaderCacheHeaderItem header;
	std::vector<SCGBind> bindVars;

	std::unique_ptr<byte[]> m_pVertexShaderBinary;
	std::size_t             m_VertexShaderBinarySize;
#if DRX_RENDERER_VULKAN
	std::vector<SVertexInputStream>  m_VSInputStreams;
#endif

	_smart_ptr<class SD3DShader> shader;

	operator bool() const noexcept { return !!shader; }
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(bindVars.data(), bindVars.size() * sizeof(SCGBind));
		pSizer->AddObject(m_pVertexShaderBinary.get(), m_VertexShaderBinarySize * sizeof(byte));
#if DRX_RENDERER_VULKAN
		pSizer->AddObject(m_VSInputStreams.data(), m_VSInputStreams.size() * sizeof(SVertexInputStream));
#endif
		pSizer->AddObject(shader.get());
	}
};

struct SHWShaderCache : CBaseResource
{
private:
	string m_Name;

public:
	using deviceShaderCacheKey = uint32_t;
	// Value might be an entry or a duplicate, in which case it is a pointer to duplicated entry.
	using deviceShaderCacheValue = DrxVariant<const SDeviceShaderEntry*, SDeviceShaderEntry>;
	using shaderCache = std::unordered_map<deviceShaderCacheKey, deviceShaderCacheValue>;

	shaderCache                       m_shaders;
	std::unique_ptr<SDiskShaderCache> m_pDiskShaderCache[2];

	SHWShaderCache(string name) noexcept : m_Name(name) {}

	SDiskShaderCache* AcquireDiskCache(cacheSource src)
	{
		auto &cache = m_pDiskShaderCache[static_cast<i32>(src)];
		if (cache)
			return cache.get();

		return (cache = stl::make_unique<SDiskShaderCache>(m_Name.c_str(), src)).get();
	}

	const string& GetName() const { return m_Name; }
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;
	void Reset()
	{
		m_shaders.clear();
		m_pDiskShaderCache[0] = nullptr;
		m_pDiskShaderCache[1] = nullptr;
	}
};

class CHWShader : public CBaseResource
{
public:
	EHWShaderClass            m_eSHClass;
	//EHWSProfile m_eHWProfile;

	static class CHWShader* s_pCurHWVS;
	static char *s_GS_MultiRes_NV;

	string                    m_Name;
	string                    m_NameSourceFX;
	string                    m_EntryFunc;
	uint64                    m_nMaskAnd_RT;
	uint64                    m_nMaskOr_RT;
	uint64                    m_nMaskGenShader; // Masked/Optimised m_nMaskGenFX for this specific HW shader
	uint64                    m_nMaskGenFX;     // FX Shader should be parsed with this flags
	uint64                    m_nMaskSetFX;     // AffectMask GL for parser tree

	u32                    m_nPreprocessFlags;
	i32                       m_nFrame;
	i32                       m_nFrameLoad;
	u32                    m_Flags;
	u32                    m_CRC32;
	u32                    m_dwShaderType;

protected:
	STxt               m_CachedTokens;
	SHWShaderCache*           m_pCache = nullptr;

public:
	CHWShader()
	{
		m_nFrame = 0;
		m_nFrameLoad = 0;
		m_Flags = 0;
		m_nMaskGenShader = 0;
		m_nMaskAnd_RT = -1;
		m_nMaskOr_RT = 0;
		m_CRC32 = 0;
		m_nMaskGenFX = 0;
		m_nMaskSetFX = 0;
		m_eSHClass = eHWSC_Vertex;
	}
	~CHWShader() noexcept;

	void InvalidateCache(cacheSource src) { m_pCache->m_pDiskShaderCache[static_cast<i32>(src)] = nullptr; }
	void InvalidateCaches()
	{
		m_pCache->Reset();
	}
	SDiskShaderCache* QueryDiskCache(cacheSource src) const
	{
		if (m_pCache->m_pDiskShaderCache[static_cast<i32>(src)] && m_pCache->m_pDiskShaderCache[static_cast<i32>(src)]->m_pRes)
			return m_pCache->m_pDiskShaderCache[static_cast<i32>(src)].get();
		return nullptr;
	}
	SDiskShaderCache* AcquireDiskCache(cacheSource src) { return m_pCache->AcquireDiskCache(src); }
	SHWShaderCache::shaderCache& GetDevCache() { return m_pCache->m_shaders; }
	const SHWShaderCache::shaderCache& GetDevCache() const { return m_pCache->m_shaders; }

	//EHWSProfile mfGetHWProfile(u32 nFlags);

	static CDrxNameTSCRC mfGetClassName(EHWShaderClass eClass);
	static CDrxNameTSCRC mfGetCacheClassName(EHWShaderClass eClass);

	static CHWShader*  mfForName(tukk name, tukk nameSource, u32 CRC32, tukk szEntryFunc, EHWShaderClass eClass, const TArray<u32>& SHData, const FXShaderToken& Table, u32 dwType, CShader* pFX, uint64 nMaskGen = 0, uint64 nMaskGenFX = 0);

	static void        mfReloadScript(tukk szPath, tukk szName, i32 nFlags, uint64 nMaskGen);
	static void        mfFlushPendedShadersWait(i32 nMaxAllowed);
	inline tukk GetName()
	{
		return m_Name.c_str();
	}
	virtual i32         Size() = 0;
	virtual void        GetMemoryUsage(IDrxSizer* Sizer) const = 0;
	virtual void        mfReset() = 0;

	virtual bool        mfAddEmptyCombination(uint64 nRT, uint64 nGL, u32 nLT, const SCacheCombination& cmbSaved) = 0;
	virtual bool        mfStoreEmptyCombination(SEmptyCombination& Comb) = 0;
	virtual tukk mfGetCurScript() { return NULL; }
	virtual tukk mfGetEntryName() = 0;
	virtual void        mfUpdatePreprocessFlags(SShaderTechnique* pTech) = 0;
	virtual bool        mfFlushCacheFile() = 0;
	bool                mfWriteoutTokensToCache();

	// Used to precache shader combination during shader cache generation.
	virtual bool        PrecacheShader(CShader* pSH, const SShaderCombIdent &cacheIdent,u32 nFlags) = 0;

	virtual bool        Export(CShader *pSH, SShaderSerializeContext& SC) = 0;
	static CHWShader*   Import(SShaderSerializeContext& SC, i32 nOffs, u32 CRC32, CShader* pSH);

	// Vertex shader specific functions
	virtual InputLayoutHandle mfVertexFormat(bool& bUseTangents, bool& bUseLM, bool& bUseHWSkin) = 0;

	virtual tukk   mfGetActivatedCombinations(bool bForLevel) = 0;

	static tukk    mfProfileString(EHWShaderClass eClass);
	static tukk    mfClassString(EHWShaderClass eClass);
	static EHWShaderClass mfStringProfile(tukk profile);
	static EHWShaderClass mfStringClass(tukk szClass);
	static void           mfGenName(uint64 GLMask, uint64 RTMask, u32 LightMask, u32 MDMask, u32 MDVMask, uint64 PSS, EHWShaderClass eClass, tuk dstname, i32 nSize, byte bType);
	static void           mfGenMasksFromName(tuk srcName, uint64& GLMask, uint64 RTMask, u32 LightMask, u32 MDMask, u32 MDVMask, uint64 PSS, EHWShaderClass eClass, i32 nSize, byte bType);

	static void           mfLazyUnload();
	static void           mfCleanupCache();

	static tukk      GetCurrentShaderCombinations(bool bForLevel) threadsafe;

	static byte*            mfIgnoreRemapsFromCache(i32 nRemaps, byte* pP);
	static byte*            mfIgnoreBindsFromCache(i32 nParams, byte* pP);

	static void             mfValidateTokenData(CResFile* pRF);
	static void             mfValidateDirEntries(CResFile* pRF);

	// Import/Export
	static bool ImportSamplers(SShaderSerializeContext& SC, struct SCHWShader* pSHW, byte*& pData, std::vector<STexSamplerRT>& Samplers);
	static bool ImportParams(SShaderSerializeContext& SC, SCHWShader* pSHW, byte*& pData, std::vector<SFXParam>& Params);
};

inline void SortLightTypes(i32 Types[4], i32 nCount)
{
	switch (nCount)
	{
	case 2:
		if (Types[0] > Types[1])
			Exchange(Types[0], Types[1]);
		break;
	case 3:
		if (Types[0] > Types[1])
			Exchange(Types[0], Types[1]);
		if (Types[0] > Types[2])
			Exchange(Types[0], Types[2]);
		if (Types[1] > Types[2])
			Exchange(Types[1], Types[2]);
		break;
	case 4:
		{
			for (i32 i = 0; i < 4; i++)
			{
				for (i32 j = i; j < 4; j++)
				{
					if (Types[i] > Types[j])
						Exchange(Types[i], Types[j]);
				}
			}
		}
		break;
	}
}

//=========================================================================
// Dynamic lights evaluating via shaders

enum ELightStyle
{
	eLS_Intensity,
	eLS_RGB,
};

enum ELightMoveType
{
	eLMT_Wave,
	eLMT_Patch,
};

struct SLightMove
{
	ELightMoveType m_eLMType;
	SWaveForm      m_Wave;
	Vec3           m_Dir;
	float          m_fSpeed;

	i32            Size()
	{
		i32 nSize = sizeof(SLightMove);
		return nSize;
	}
};

struct SLightStyleKeyFrame
{
	ColorF cColor;     // xyz: color, w: spec mult
	Vec3   vPosOffset; // position offset

	SLightStyleKeyFrame()
	{
		cColor = ColorF(Col_Black);
		vPosOffset = Vec3(ZERO);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

class CLightStyle
{
public:

	CLightStyle() : m_Color(Col_White),
		m_vPosOffset(ZERO),
		m_LastTime(0.0f),
		m_TimeIncr(60.0f),
		m_bRandColor(0),
		m_bRandIntensity(0),
		m_bRandPosOffset(0),
		m_bRandSpecMult(0)
	{
	}

	static TArray<CLightStyle*> s_LStyles;
	TArray<SLightStyleKeyFrame> m_Map;

	ColorF                      m_Color;      // xyz: color, w: spec mult
	Vec3                        m_vPosOffset; // position offset

	float                       m_TimeIncr;
	float                       m_LastTime;

	u8                       m_bRandColor     : 1;
	u8                       m_bRandIntensity : 1;
	u8                       m_bRandPosOffset : 1;
	u8                       m_bRandSpecMult  : 1;

	i32 Size()
	{
		i32 nSize = sizeof(CLightStyle);
		nSize += m_Map.GetMemoryUsage();
		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->Add(*this);
		pSizer->AddObject(m_Map);
	}

	static inline CLightStyle* mfGetStyle(u32 nStyle, float fTime)
	{
		if (nStyle >= s_LStyles.Num() || !s_LStyles[nStyle])
			return NULL;
		s_LStyles[nStyle]->mfUpdate(fTime);
		return s_LStyles[nStyle];
	}

	void mfUpdate(float fTime);
};

//=========================================================================
// HW Shader Layer

#define SHPF_AMBIENT             0x100
#define SHPF_HASLM               0x200
#define SHPF_SHADOW              0x400
#define SHPF_RADIOSITY           0x800
#define SHPF_ALLOW_SPECANTIALIAS 0x1000
#define SHPF_BUMP                0x2000
#define SHPF_NOMATSTATE          0x4000
#define SHPF_FORCEZFUNC          0x8000

// Shader pass definition for HW shaders
struct SShaderPass
{
	u32      m_RenderState;     // Render state flags
	signed char m_eCull;
	u8       m_AlphaRef;

	u16      m_PassFlags;         // Different usefull Pass flags (SHPF_)

	CHWShader*  m_VShader;        // Pointer to the vertex shader for the current pass
	CHWShader*  m_PShader;        // Pointer to fragment shader
	CHWShader*  m_GShader;        // Pointer to the geometry shader for the current pass
	CHWShader*  m_DShader;        // Pointer to the domain shader for the current pass
	CHWShader*  m_HShader;        // Pointer to the hull shader for the current pass
	CHWShader*  m_CShader;        // Pointer to the compute shader for the current pass
	SShaderPass();

	i32 Size()
	{
		i32 nSize = sizeof(SShaderPass);
		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_VShader);
		pSizer->AddObject(m_PShader);
		pSizer->AddObject(m_GShader);
		pSizer->AddObject(m_HShader);
		pSizer->AddObject(m_DShader);
		pSizer->AddObject(m_CShader);
	}
	void mfFree()
	{
		SAFE_RELEASE(m_VShader);
		SAFE_RELEASE(m_PShader);
		SAFE_RELEASE(m_GShader);
		SAFE_RELEASE(m_HShader);
		SAFE_RELEASE(m_DShader);
		SAFE_RELEASE(m_CShader);
	}

	void AddRefsToShaders()
	{
		if (m_VShader)
			m_VShader->AddRef();
		if (m_PShader)
			m_PShader->AddRef();
		if (m_GShader)
			m_GShader->AddRef();
		if (m_DShader)
			m_DShader->AddRef();
		if (m_HShader)
			m_HShader->AddRef();
		if (m_CShader)
			m_CShader->AddRef();
	}

private:
	SShaderPass& operator=(const SShaderPass& sl);
};

//===================================================================================
// Hardware Stage for HW only Shaders

#define FHF_FIRSTLIGHT          8
#define FHF_FORANIM             0x10
#define FHF_TERRAIN             0x20
#define FHF_NOMERGE             0x40
#define FHF_DETAILPASS          0x80
#define FHF_LIGHTPASS           0x100
#define FHF_FOGPASS             0x200
#define FHF_PUBLIC              0x400
#define FHF_NOLIGHTS            0x800
#define FHF_POSITION_INVARIANT  0x1000
#define FHF_TRANSPARENT         0x40000
#define FHF_WASZWRITE           0x80000
#define FHF_USE_GEOMETRY_SHADER 0x100000
#define FHF_USE_HULL_SHADER     0x200000
#define FHF_USE_DOMAIN_SHADER   0x400000
#define FHF_RE_LENSOPTICS       0x1000000

struct SShaderTechnique
{
	CShader*                  m_shader;    // Shader owner of this technique.
	CDrxNameR                 m_NameStr;
	CDrxNameTSCRC             m_NameCRC;
	TArray<SShaderPass>       m_Passes;    // General passes
	i32                       m_Flags;     // Different flags (FHF_)
	u32                    m_nPreprocessFlags;
	int8                      m_nTechnique[TTYPE_MAX]; // Next technique in sequence
	TArray<CRenderElement*> m_REs;                   // List of all render elements registered in the shader
	TArray<SHRenderTarget*>   m_RTargets;
	float                     m_fProfileTime;

	i32                       Size()
	{
		u32 i;
		i32 nSize = sizeof(SShaderTechnique);
		for (i = 0; i < m_Passes.Num(); i++)
		{
			nSize += m_Passes[i].Size();
		}
		nSize += m_RTargets.GetMemoryUsage();
		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->Add(*this);
		pSizer->AddObject(m_Passes);
		pSizer->AddObject(m_REs);
		pSizer->AddObject(m_RTargets);
	}

	SShaderTechnique(CShader* shader)
	{
		m_shader = shader;
		u32 i;
		for (i = 0; i < TTYPE_MAX; i++)
		{
			m_nTechnique[i] = -1;
		}
		for (i = 0; i < m_REs.Num(); i++)
		{
			SAFE_DELETE(m_REs[i]);
		}
		m_REs.Free();

		m_Flags = 0;
		m_nPreprocessFlags = 0;
		m_fProfileTime = 0;
	}
	SShaderTechnique& operator=(const SShaderTechnique& sl)
	{
		memcpy(this, &sl, sizeof(SShaderTechnique));
		if (sl.m_Passes.Num())
		{
			m_Passes.Copy(sl.m_Passes);
			for (u32 i = 0; i < sl.m_Passes.Num(); i++)
			{
				SShaderPass* d = &m_Passes[i];
				d->AddRefsToShaders();
			}
		}
		if (sl.m_REs.Num())
		{
			m_REs.Create(sl.m_REs.Num());
			for (u32 i = 0; i < sl.m_REs.Num(); i++)
			{
				if (sl.m_REs[i])
					m_REs[i] = sl.m_REs[i]->mfCopyConstruct();
			}
		}

		return *this;
	}

	~SShaderTechnique()
	{
		for (u32 i = 0; i < m_Passes.Num(); i++)
		{
			SShaderPass* sl = &m_Passes[i];

			sl->mfFree();
		}
		for (u32 i = 0; i < m_REs.Num(); i++)
		{
			CRenderElement* pRE = m_REs[i];
			pRE->Release(false);
		}
		m_REs.Free();
		m_Passes.Free();
	}
	void  UpdatePreprocessFlags(CShader* pSH);

	uk operator new(size_t Size)                                { uk ptr = malloc(Size); memset(ptr, 0, Size); return ptr; }
	uk operator new(size_t Size, const std::nothrow_t& nothrow) { uk ptr = malloc(Size); if (ptr) memset(ptr, 0, Size); return ptr; }
	void  operator delete(uk Ptr)                               { free(Ptr); }
};

//===============================================================================

enum EShaderDrawType
{
	eSHDT_General,
	eSHDT_Light,
	eSHDT_Shadow,
	eSHDT_Terrain,
	eSHDT_Overlay,
	eSHDT_OceanShore,
	eSHDT_Fur,
	eSHDT_NoDraw,
	eSHDT_CustomDraw,
	eSHDT_Sky,
	eSHDT_DebugHelper,
};

// General Shader structure
class CShader : public IShader, public CBaseResource
{
	static CDrxNameTSCRC      s_sClassName;
public:
	string                    m_NameFile; // } FIXME: This fields order is very important
	string                    m_NameShader;
	EShaderDrawType           m_eSHDType; // } Check CShader::operator = in ShaderCore.cpp for more info

	u32                    m_Flags;  // Different flags EF_  (see IShader.h)
	u32                    m_Flags2; // Different flags EF2_ (see IShader.h)
	u32                    m_nMDV;   // Vertex modificator flags
	u32                    m_NameShaderICRC;

	InputLayoutHandle         m_eVertexFormat; // Base vertex format for the shader (see VertexFormats.h)
	ECull                     m_eCull;         // Global culling type

	TArray<SShaderTechnique*> m_HWTechniques;    // Hardware techniques
	i32                       m_nMaskCB;

	EShaderType               m_eShaderType;

	uint64                    m_nMaskGenFX;
	SShaderGen*               m_ShaderGenParams;           // BitMask params used in automatic script generation
	SShaderTexSlots*          m_ShaderTexSlots[TTYPE_MAX]; // filled out with data of the used texture slots for a given technique
	                                                       // (might be NULL if this data isn't gathered)
	std::vector<CShader*>*    m_DerivedShaders;
	CShader*                  m_pGenShader;

	i32                       m_nRefreshFrame; // Current frame for shader reloading (to avoid multiple reloading)
	u32                    m_SourceCRC32;
	u32                    m_CRC32;

	//! Minimal known distance to the object using this shader
	float                     m_fMinVisibleDistance;

	//============================================================================

	inline i32 mfGetID() { return CBaseResource::GetID(); }

	void       mfFree();
	CShader()
		: m_eSHDType(eSHDT_General)
		, m_Flags(0)
		, m_Flags2(0)
		, m_nMDV(0)
		, m_NameShaderICRC(0)
		, m_eVertexFormat(EDefaultInputLayouts::P3F_C4B_T2F)
		, m_eCull((ECull) - 1)
		, m_nMaskCB(0)
		, m_eShaderType(eST_General)
		, m_nMaskGenFX(0)
		, m_ShaderGenParams(nullptr)
		, m_DerivedShaders(nullptr)
		, m_pGenShader(nullptr)
		, m_nRefreshFrame(0)
		, m_SourceCRC32(0)
		, m_CRC32(0)
		, m_fMinVisibleDistance(FLT_MAX)
	{
		memset(m_ShaderTexSlots, 0, sizeof(m_ShaderTexSlots));
	}
	virtual ~CShader();

	//===================================================================================

	// IShader interface
	virtual i32 AddRef() { return CBaseResource::AddRef(); }
	virtual i32 Release()
	{
		if (m_Flags & EF_SYSTEM)
			return -1;
		return CBaseResource::Release();
	}
	virtual i32 ReleaseForce()
	{
		m_Flags &= ~EF_SYSTEM;
		i32 nRef = 0;
		while (true)
		{
			nRef = Release();
#if !defined(_RELEASE) && defined(_DEBUG)
			IF (nRef < 0, 0)
				__debugbreak();
#endif
			if (nRef == 0)
				break;
		}
		return nRef;
	}

	virtual i32         GetID()               { return CBaseResource::GetID(); }
	virtual i32         GetRefCounter() const { return CBaseResource::GetRefCounter(); }
	virtual tukk GetName()             { return m_NameShader.c_str(); }
	virtual tukk GetName() const       { return m_NameShader.c_str(); }

	virtual i32  GetFlags() const       { return m_Flags; }
	virtual i32  GetFlags2() const      { return m_Flags2; }
	virtual void SetFlags2(i32 Flags)   { m_Flags2 |= Flags; }
	virtual void ClearFlags2(i32 Flags) { m_Flags2 &= ~Flags; }

	virtual bool Reload(i32 nFlags, tukk szShaderName);
#if DRX_PLATFORM_DESKTOP
	virtual void mfFlushCache();
#endif

	void        mfFlushPendedShaders();

	virtual i32 GetTechniqueID(i32 nTechnique, i32 nRegisteredTechnique)
	{
		if (nTechnique < 0)
			nTechnique = 0;
		if ((i32)m_HWTechniques.Num() <= nTechnique)
			return -1;
		SShaderTechnique* pTech = m_HWTechniques[nTechnique];
		return pTech->m_nTechnique[nRegisteredTechnique];
	}
	virtual TArray<CRenderElement*>* GetREs(i32 nTech)
	{
		if (nTech < 0)
			nTech = 0;
		if (nTech < (i32)m_HWTechniques.Num())
		{
			SShaderTechnique* pTech = m_HWTechniques[nTech];
			return &pTech->m_REs;
		}
		return NULL;
	}
	virtual i32           GetTexId();
	virtual u32  GetUsedTextureTypes(void);
	virtual InputLayoutHandle GetVertexFormat(void) { return m_eVertexFormat; }
	virtual uint64        GetGenerationMask()   { return m_nMaskGenFX; }
	virtual ECull         GetCull(void)
	{
		if (m_HWTechniques.Num())
		{
			SShaderTechnique* pTech = m_HWTechniques[0];
			if (pTech->m_Passes.Num())
				return (ECull)pTech->m_Passes[0].m_eCull;
		}
		return eCULL_None;
	}
	virtual SShaderGen* GetGenerationParams()
	{
		if (m_ShaderGenParams)
			return m_ShaderGenParams;
		if (m_pGenShader)
			return m_pGenShader->m_ShaderGenParams;
		return NULL;
	}
	virtual SShaderTexSlots*           GetUsedTextureSlots(i32 nTechnique);

	virtual DynArrayRef<SShaderParam>& GetPublicParams();
	virtual void                       CopyPublicParamsTo(SInputShaderResources& copyToResource);
	virtual EShaderType                GetShaderType()        { return m_eShaderType; }
	virtual u32                     GetVertexModificator() { return m_nMDV; }

	SShaderTechnique* mfFindTechnique(const CDrxNameTSCRC& name)
	{
		u32 i;
		for (i = 0; i < m_HWTechniques.Num(); i++)
		{
			SShaderTechnique* pTech = m_HWTechniques[i];
			if (pTech->m_NameCRC == name)
				return pTech;
		}
		return NULL;
	}

	SShaderTechnique* GetTechnique(i32 nStartTechnique, i32 nRequestedTechnique, bool bSilent = false);

	virtual ITexture* GetBaseTexture(i32* nPass, i32* nTU);

	CShader&          operator=(const CShader& src);
	CTexture*         mfFindBaseTexture(TArray<SShaderPass>& Passes, i32* nPass, i32* nTU);

	i32               mfSize();

	// All loaded shaders resources list
	static TArray<CShaderResources*> s_ShaderResources_known;

	virtual i32 Size(i32 Flags)
	{
		return mfSize();
	}

	virtual void         GetMemoryUsage(IDrxSizer* Sizer) const;
	uk                operator new(size_t Size)                                { uk ptr = malloc(Size); memset(ptr, 0, Size); return ptr; }
	uk                operator new(size_t Size, const std::nothrow_t& nothrow) { uk ptr = malloc(Size); if (ptr) memset(ptr, 0, Size); return ptr; }
	void                 operator delete(uk Ptr)                               { free(Ptr); }

	static CDrxNameTSCRC mfGetClassName()
	{
		return s_sClassName;
	}

	void UpdateMinVisibleDistance(float fMinDistance)
	{
		if (fMinDistance < m_fMinVisibleDistance)
		{
			m_fMinVisibleDistance = fMinDistance;
		}
	}
};

inline SShaderTechnique* SShaderItem::GetTechnique() const
{
	SShaderTechnique* pTech = NULL;
	i32 nTech = m_nTechnique;
	if (nTech < 0)
		nTech = 0;
	CShader* pSH = (CShader*)m_pShader;

	if (pSH && !pSH->m_HWTechniques.empty())
	{
		DrxPrefetch(&pSH->m_HWTechniques[0]);

		assert(m_nTechnique < 0 || pSH->m_HWTechniques.Num() == 0 || nTech < (i32)pSH->m_HWTechniques.Num());
		if (nTech < (i32)pSH->m_HWTechniques.Num())
			return pSH->m_HWTechniques[nTech];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

#endif
