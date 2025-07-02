// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CSHADERBIN_H__
#define __CSHADERBIN_H__

#include <map>
#include <drx3D/Render/ParserBin.h>

#ifndef FOURCC
typedef DWORD FOURCC;
#endif

struct SShaderTechParseParams
{
	CDrxNameR techName[TTYPE_MAX];
};

class CShaderMan;

struct SShaderBinHeader
{
	FOURCC m_Magic;
	u32 m_CRC32;
	u16 m_VersionLow;
	u16 m_VersionHigh;
	u32 m_nOffsetStringTable;
	u32 m_nOffsetParamsLocal;
	u32 m_nTokens;
	u32 m_nSourceCRC32;

	AUTO_STRUCT_INFO;
};

struct SShaderBinParamsHeader
{
	uint64 nMask;
	u32 nName;
	i32  nParams;
	i32  nSamplers;
	i32  nTextures;
	i32  nFuncs;

	AUTO_STRUCT_INFO;
};

struct SParamCacheInfo
{
	typedef std::vector<i32, STLShaderAllocator<i32>> AffectedFuncsVec;
	typedef std::vector<i32, STLShaderAllocator<i32>> AffectedParamsVec;

	u32            m_dwName;
	uint64            m_nMaskGenFX;
	AffectedFuncsVec  m_AffectedFuncs;
	AffectedParamsVec m_AffectedParams;
	AffectedParamsVec m_AffectedSamplers;
	AffectedParamsVec m_AffectedTextures;

	SParamCacheInfo() : m_dwName(0), m_nMaskGenFX(0) {};

	i32 Size()
	{
		return sizeof(SParamCacheInfo) + sizeofVector(m_AffectedFuncs) + sizeofVector(m_AffectedParams) + sizeofVector(m_AffectedSamplers) + sizeofVector(m_AffectedTextures);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_AffectedFuncs);
		pSizer->AddObject(m_AffectedParams);
		pSizer->AddObject(m_AffectedSamplers);
		pSizer->AddObject(m_AffectedTextures);
	}
};

#define MAX_FXBIN_CACHE 200

struct SShaderBin
{
	typedef std::vector<SParamCacheInfo, STLShaderAllocator<SParamCacheInfo>> ParamsCacheVec;
	static SShaderBin s_Root;
	static u32     s_nCache;
	static u32     s_nMaxFXBinCache;

	SShaderBin*       m_Next;
	SShaderBin*       m_Prev;

	u32            m_CRC32;
	u32            m_dwName;
	tuk             m_szName;
	u32            m_SourceCRC32;
	bool              m_bLocked;
	bool              m_bReadOnly;
	bool              m_bInclude;
	FXShaderToken     m_TokenTable;
	ShaderTokensVec   m_Tokens;

	// Local shader info (after parsing)
	u32         m_nOffsetLocalInfo;
	u32         m_nCurCacheParamsID;
	u32         m_nCurParamsID;
	ParamsCacheVec m_ParamsCache;

	SShaderBin()
		: m_Next(nullptr)
		, m_Prev(nullptr)
		, m_CRC32(0)
		, m_dwName(0)
		, m_szName("")
		, m_SourceCRC32(0)
		, m_bLocked(false)
		, m_bReadOnly(true)
		, m_bInclude(false)
		, m_nOffsetLocalInfo(0)
		, m_nCurCacheParamsID(-1)
		, m_nCurParamsID(-1)
	{
		if (!s_Root.m_Next)
		{
			s_Root.m_Next = &s_Root;
			s_Root.m_Prev = &s_Root;
		}
	}

	~SShaderBin()
	{
		if (m_szName[0])
			g_shaderBucketAllocator.deallocate((uk ) m_szName);
	}

	void SetName(tukk name)
	{
		if (m_szName[0])
		{
			g_shaderBucketAllocator.deallocate((uk ) m_szName);
			m_szName = "";
		}

		if (name[0])
		{
			m_szName = (tuk) g_shaderBucketAllocator.allocate(strlen(name) + 1);
			strcpy(m_szName, name);
		}
	}

	inline void Unlink()
	{
		if (!m_Next || !m_Prev)
			return;
		m_Next->m_Prev = m_Prev;
		m_Prev->m_Next = m_Next;
		m_Next = m_Prev = NULL;
	}
	inline void Link(SShaderBin* Before)
	{
		if (m_Next || m_Prev)
			return;
		m_Next = Before->m_Next;
		Before->m_Next->m_Prev = this;
		Before->m_Next = this;
		m_Prev = Before;
	}
	inline bool IsReadOnly() { return m_bReadOnly; }
	inline void Lock()       { m_bLocked = true; }
	inline void Unlock()     { m_bLocked = false; }

	u32      ComputeCRC();
	void        SetCRC(u32 nCRC) { m_CRC32 = nCRC; }

	void        CryptData();

	i32         Size()
	{
		i32 nSize = sizeof(SShaderBin);
		nSize += sizeOfV(m_TokenTable);
		nSize += sizeofVector(m_Tokens);
		nSize += sizeOfV(m_ParamsCache);

		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_TokenTable);
		pSizer->AddObject(m_Tokens);
		pSizer->AddObject(m_ParamsCache);
	}

	uk operator new(size_t sz)
	{
		return g_shaderBucketAllocator.allocate(sz);
	}

	void operator delete(uk p)
	{
		g_shaderBucketAllocator.deallocate(p);
	}

private:
	SShaderBin(const SShaderBin&);
	SShaderBin& operator=(const SShaderBin&);
};

#define FXP_PARAMS_DIRTY   1
#define FXP_SAMPLERS_DIRTY 2
#define FXP_TEXTURES_DIRTY 4

typedef std::vector<SFXParam>::iterator      FXParamsIt;
typedef std::vector<SFXSampler>::iterator    FXSamplersIt;
typedef std::vector<SFXTexture>::iterator    FXTexturesIt;
struct SShaderFXParams
{
	u32                     m_nFlags; // FXP_DIRTY

	std::vector<SFXParam>      m_FXParams;
	std::vector<SFXSampler>    m_FXSamplers;
	std::vector<SFXTexture>    m_FXTextures;

	DynArray<SShaderParam>     m_PublicParams;

	SShaderFXParams()
	{
		m_nFlags = 0;
	}
	i32 Size()
	{
		i32 nSize = sizeOfV(m_FXParams);
		nSize += sizeOfV(m_FXSamplers);
		nSize += sizeOfV(m_FXSamplers);

		nSize += sizeOfV(m_PublicParams);

		return nSize;
	}
};

typedef std::map<u32, bool>          FXShaderBinValidCRC;
typedef FXShaderBinValidCRC::iterator   FXShaderBinValidCRCItor;

typedef std::map<CDrxNameTSCRC, string> FXShaderBinPath;
typedef FXShaderBinPath::iterator       FXShaderBinPathItor;

class CShaderManBin
{
	friend class CShaderMan;

	SShaderBin*       LoadBinShader(FILE* fpBin, tukk szName, tukk szNameBin, bool bReadParams);
	SShaderBin*       SaveBinShader(u32 nSourceCRC32, tukk szName, bool bInclude, FILE* fpSrc);
	bool              SaveBinShaderLocalInfo(SShaderBin* pBin, u32 dwName, uint64 nMaskGenFX, TArray<i32>& Funcs, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures);
	SParamCacheInfo*  GetParamInfo(SShaderBin* pBin, u32 dwName, uint64 nMaskGenFX);

	bool              ParseBinFX_Global_Annotations(CParserBin& Parser, SParserFrame& Frame, bool* bPublic, CDrxNameR techStart[2]);
	bool              ParseBinFX_Global(CParserBin& Parser, SParserFrame& Frame, bool* bPublic, CDrxNameR techStart[2]);
	bool              ParseBinFX_Texture_Annotations_Script(CParserBin& Parser, SParserFrame& Frame, SFXTexture* pTexture);
	bool              ParseBinFX_Texture_Annotations(CParserBin& Parser, SParserFrame& Frame, SFXTexture* pTexture);
	bool              ParseBinFX_Texture(CParserBin& Parser, SParserFrame& Data, SFXTexture& Sampl, SParserFrame Annotations);

	void              InitShaderDependenciesList(CParserBin& Parser, SCodeFragment* pFunc, TArray<byte>& bChecked, TArray<i32>& AffectedFuncs);
	void              CheckFragmentsDependencies(CParserBin& Parser, TArray<byte>& bChecked, TArray<i32>& AffectedFuncs);
	void              CheckStructuresDependencies(CParserBin& Parser, SCodeFragment* pFrag, TArray<byte>& bChecked, TArray<i32>& AffectedFunc);

	void              AddParameterToScript(CParserBin& Parser, SFXParam* pr, PodArray<u32>& SHData, EHWShaderClass eSHClass, i32 nCB);
	void              AddSamplerToScript(CParserBin& Parser, SFXSampler* pr, PodArray<u32>& SHData, EHWShaderClass eSHClass);
	void              AddTextureToScript(CParserBin& Parser, SFXTexture* pr, PodArray<u32>& SHData, EHWShaderClass eSHClass);
	void              AddAffectedParameter(CParserBin& Parser, std::vector<SFXParam>& AffectedParams, TArray<i32>& AffectedFunc, SFXParam* pParam, EHWShaderClass eSHClass, u32 dwType, SShaderTechnique* pShTech);
	void              AddAffectedSampler(CParserBin& Parser, std::vector<SFXSampler>& AffectedSamplers, TArray<i32>& AffectedFunc, SFXSampler* pParam, EHWShaderClass eSHClass, u32 dwType, SShaderTechnique* pShTech);
	void              AddAffectedTexture(CParserBin& Parser, std::vector<SFXTexture>& AffectedTextures, TArray<i32>& AffectedFunc, SFXTexture* pParam, EHWShaderClass eSHClass, u32 dwType, SShaderTechnique* pShTech);
	bool              ParseBinFX_Technique_Pass_PackParameters(CParserBin& Parser, std::vector<SFXParam>& AffectedParams, TArray<i32>& AffectedFunc, SCodeFragment* pFunc, EHWShaderClass eSHClass, u32 dwSHName, std::vector<SFXParam>& PackedParams, TArray<SCodeFragment>& Replaces, TArray<u32>& NewTokens, TArray<byte>& bMerged);
	bool              ParseBinFX_Technique_Pass_GenerateShaderData(CParserBin& Parser, FXMacroBin& Macros, SShaderFXParams& FXParams, u32 dwSHName, EHWShaderClass eSHClass, uint64& nGenMask, u32 dwSHType, PodArray<u32>& SHData, SShaderTechnique* pShTech);
	bool              ParseBinFX_Technique_Pass_LoadShader(CParserBin& Parser, FXMacroBin& Macros, SParserFrame& SHFrame, SShaderTechnique* pShTech, SShaderPass* pPass, EHWShaderClass eSHClass, SShaderFXParams& FXParams);
	bool              ParseBinFX_Technique_Pass(CParserBin& Parser, SParserFrame& Frame, SShaderTechnique* pTech);
	bool              ParseBinFX_Technique_Annotations_String(CParserBin& Parser, SParserFrame& Frame, SShaderTechnique* pSHTech, std::vector<SShaderTechParseParams>& techParams, bool* bPublic);
	bool              ParseBinFX_Technique_Annotations(CParserBin& Parser, SParserFrame& Frame, SShaderTechnique* pSHTech, std::vector<SShaderTechParseParams>& techParams, bool* bPublic);
	bool              ParseBinFX_Technique_CustomRE(CParserBin& Parser, SParserFrame& Frame, SParserFrame& Name, SShaderTechnique* pShTech);
	SShaderTechnique* ParseBinFX_Technique(CParserBin& Parser, SParserFrame& Data, SParserFrame Annotations, std::vector<SShaderTechParseParams>& techParams, bool* bPublic);
	bool              ParseBinFX_LightStyle_Val(CParserBin& Parser, SParserFrame& Frame, CLightStyle* ls);
	bool              ParseBinFX_LightStyle(CParserBin& Parser, SParserFrame& Frame, i32 nStyle);

	void              MergeTextureSlots(SShaderTexSlots* master, SShaderTexSlots* overlay);
	SShaderTexSlots*  GetTextureSlots(CParserBin& Parser, SShaderBin* pBin, CShader* ef, i32 nTech = 0, i32 nPass = 0);

	SShaderBin*       SearchInCache(tukk szName, bool bInclude);
	bool              AddToCache(SShaderBin* pSB, bool bInclude);
	bool              DeleteFromCache(SShaderBin* pSB);

	SFXParam*         mfAddFXParam(SShaderFXParams& FXP, const SFXParam* pParam);
	SFXParam*         mfAddFXParam(CShader* pSH, const SFXParam* pParam);

	void              mfAddFXSampler(CShader* pSH, const SFXSampler* pParam);
	void              mfAddFXTexture(CShader* pSH, const SFXTexture* pParam);

	void              mfGeneratePublicFXParams(CShader* pSH, CParserBin& Parser);

public:
	CShaderManBin();
	SShaderBin*      GetBinShader(tukk szName, bool bInclude, u32 nRefCRC32, bool* pbChanged = NULL);
	bool             ParseBinFX(SShaderBin* pBin, CShader* ef, uint64 nMaskGen);
	bool             ParseBinFX_Dummy(SShaderBin* pBin, std::vector<string>& ShaderNames, tukk szName);

	SShaderFXParams& mfGetFXParams(CShader* pSH);
	void             mfRemoveFXParams(CShader* pSH);
	i32              mfSizeFXParams(u32& nCount);
	void             mfReleaseFXParams();
	void             AddGenMacros(SShaderGen* shG, CParserBin& Parser, uint64 nMaskGen);

	void             InvalidateCache(bool bIncludesOnly = false);

	CShaderMan*         m_pCEF;
	FXShaderBinPath     m_BinPaths;
	FXShaderBinValidCRC m_BinValidCRCs;

	bool                m_bBinaryShadersLoaded;

	typedef std::map<CDrxNameTSCRC, SShaderFXParams> ShaderFXParams;
	typedef ShaderFXParams::iterator                 ShaderFXParamsItor;
	ShaderFXParams m_ShaderFXParams;

	i32  Size();
	void GetMemoryUsage(IDrxSizer* pSizer) const;
};

//=====================================================================

#endif  // __CSHADERBIN_H__
