// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CSHADER_H__
#define __CSHADER_H__

#include <map>
#include <drx3D/Render/CShaderBin.h>
#include <drx3D/Render/ShaderSerialize.h>
#include <drx3D/Render/ShaderCache.h>
#include <drx3D/Render/ShaderComponents.h> // ECGParam
#include <drx3D/Render/ResFileLookupDataMan.h>

struct SRenderBuf;
class CRenderElement;
struct SEmitter;
struct SParticleInfo;
struct SPartMoveStage;
struct SSunFlare;

//===================================================================

struct SMacroFX
{
	string m_szMacro;
	u32 m_nMask;
};

//typedef std::unordered_map<string, SMacroFX, stl::hash_strcmp<string>, stl::hash_strcmp<string>> FXMacro;
//typedef std::map<string,SMacroFX> FXMacro;
typedef std::unordered_map<string, SMacroFX, stl::hash_strcmp<tukk >, stl::hash_strcmp<tukk >> FXMacro;

typedef FXMacro::iterator                                                                                  FXMacroItor;

//////////////////////////////////////////////////////////////////////////
// Helper class for shader parser, holds temporary strings vector etc...
//////////////////////////////////////////////////////////////////////////
struct CShaderParserHelper
{
	CShaderParserHelper()
	{
	}
	~CShaderParserHelper()
	{
	}

	tuk GetTempStringArray(i32 nIndex, i32 nLen)
	{
		m_tempString.reserve(nLen + 1);
		return (tuk)&(m_tempStringArray[nIndex])[0];
	}

	std::vector<char> m_tempStringArray[32];
	std::vector<char> m_tempString;
};
extern CShaderParserHelper* g_pShaderParserHelper;

enum EShaderFlagType
{
	eSFT_Global = 0,
	eSFT_Runtime,
	eSFT_MDV,
	eSFT_LT,
};

enum EShaderFilterOperation
{
	eSFO_Expand = 0,  // expand all permutations of the mask
	eSFO_And,         // and against the mask
	eSFO_Eq,          // set the mask
};

// includes or excludes
struct CShaderListFilter
{
	CShaderListFilter() :
		m_bInclude(true)
	{}

	bool   m_bInclude;
	string m_ShaderName;

	struct Predicate
	{
		Predicate() :
			m_Negated(false),
			m_Flags(eSFT_Global),
			m_Op(eSFO_And),
			m_Mask(0)
		{}

		bool                   m_Negated;
		EShaderFlagType        m_Flags;
		EShaderFilterOperation m_Op;
		uint64                 m_Mask;
	};
	std::vector<Predicate> m_Predicates;
};

//==================================================================================

#define PD_INDEXED 1
#define PD_MERGED  4

struct SParamDB
{
	tukk szName;
	tukk szAliasName;
	ECGParam    eParamType;
	u32      nFlags;
	void        (* ParserFunc)(tukk szScr, tukk szAnnotations, SCGParam* vpp, i32 nComp, CShader* ef);
	SParamDB()
		: szName(nullptr)
		, szAliasName(nullptr)
		, eParamType(ECGP_Unknown)
		, nFlags(0)
		, ParserFunc(nullptr)
	{
	}
	SParamDB(tukk inName, ECGParam ePrmType, u32 inFlags)
	{
		szName = inName;
		szAliasName = NULL;
		nFlags = inFlags;
		ParserFunc = NULL;
		eParamType = ePrmType;
	}
	SParamDB(tukk inName, ECGParam ePrmType, u32 inFlags, void(*InParserFunc)(tukk szScr, tukk szAnnotations, SCGParam * vpp, i32 nComp, CShader * ef))
	{
		szName = inName;
		szAliasName = NULL;
		nFlags = inFlags;
		ParserFunc = InParserFunc;
		eParamType = ePrmType;
	}
};

struct SSamplerDB
{
	tukk szName;
	ECGSampler  eSamplerType;
	u32      nFlags;
	void        (* ParserFunc)(tukk szScr, tukk szAnnotations, std::vector<SFXSampler>* pSamplers, SCGSampler* vpp, CShader* ef);
	SSamplerDB()
	{
		szName = NULL;
		nFlags = 0;
		ParserFunc = NULL;
		eSamplerType = ECGS_Unknown;
	}
	SSamplerDB(tukk inName, ECGSampler ePrmType, u32 inFlags)
	{
		szName = inName;
		nFlags = inFlags;
		ParserFunc = NULL;
		eSamplerType = ePrmType;
	}
	SSamplerDB(tukk inName, ECGSampler ePrmType, u32 inFlags, void(*InParserFunc)(tukk szScr, tukk szAnnotations, std::vector<SFXSampler>* pSamplers, SCGSampler * vpp, CShader * ef))
	{
		szName = inName;
		nFlags = inFlags;
		ParserFunc = InParserFunc;
		eSamplerType = ePrmType;
	}
};

struct STextureDB
{
	tukk szName;
	ECGTexture  eTextureType;
	u32      nFlags;
	void        (* ParserFunc)(tukk szScr, tukk szAnnotations, std::vector<SFXTexture>* pSamplers, SCGTexture* vpp, CShader* ef);
	STextureDB()
	{
		szName = NULL;
		nFlags = 0;
		ParserFunc = NULL;
		eTextureType = ECGT_Unknown;
	}
	STextureDB(tukk inName, ECGTexture ePrmType, u32 inFlags)
	{
		szName = inName;
		nFlags = inFlags;
		ParserFunc = NULL;
		eTextureType = ePrmType;
	}
	STextureDB(tukk inName, ECGTexture ePrmType, u32 inFlags, void(*InParserFunc)(tukk szScr, tukk szAnnotations, std::vector<SFXTexture>* pSamplers, SCGTexture * vpp, CShader * ef))
	{
		szName = inName;
		nFlags = inFlags;
		ParserFunc = InParserFunc;
		eTextureType = ePrmType;
	}
};

enum EShaderCacheMode
{
	eSC_Normal          = 0,
	eSC_BuildGlobal     = 2,
	eSC_BuildGlobalList = 3,
	eSC_Preactivate     = 4,
};

//////////////////////////////////////////////////////////////////////////
class CShaderMan :
	public ISystemEventListener
#if defined(SHADERS_SERIALIZING)
	, public CShaderSerialize
#endif
{
	friend class CShader;
	friend class CParserBin;

	//////////////////////////////////////////////////////////////////////////
	// ISystemEventListener interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	//////////////////////////////////////////////////////////////////////////

private:
	STexAnim*      mfReadTexSequence(tukk name, i32 Flags, bool bFindOnly);
	i32            mfReadTexSequence(STexSamplerRT* smp, tukk name, i32 Flags, bool bFindOnly);

	CShader*       mfNewShader(tukk szName);

	bool           mfCompileShaderGen(SShaderGen* shg, tuk scr);
	SShaderGenBit* mfCompileShaderGenProperty(tuk scr);

	void           mfSetResourceTexState(SEfResTexture* Tex);
	CTexture*      mfTryToLoadTexture(tukk nameTex, STexSamplerRT* smp, i32 Flags, bool bFindOnly);
	void           mfRefreshResourceTextureConstants(EEfResTextures Id, SInputShaderResources& RS);
	void           mfRefreshResourceTextureConstants(EEfResTextures Id, CShaderResources& RS);
	CTexture*      mfFindResourceTexture(tukk nameTex, tukk path, i32 Flags, SEfResTexture* Tex);
	CTexture*      mfLoadResourceTexture(tukk nameTex, tukk path, i32 Flags, SEfResTexture* Tex);
	bool           mfLoadResourceTexture(EEfResTextures Id, SInputShaderResources& RS, u32 CustomFlags, bool bReplaceMeOnFail = false);
	bool           mfLoadResourceTexture(EEfResTextures Id, CShaderResources& RS, u32 CustomFlags, bool bReplaceMeOnFail = false);
	void           mfLoadDefaultTexture(EEfResTextures Id, CShaderResources& RS, EEfResTextures Alias);
	void           mfCheckShaderResTextures(TArray<SShaderPass>& Dst, CShader* ef, CShaderResources* Res);
	void           mfCheckShaderResTexturesHW(TArray<SShaderPass>& Dst, CShader* ef, CShaderResources* Res);
	CTexture*      mfCheckTemplateTexName(tukk mapname, ETEX_Type eTT);

	CShader*       mfCompile(CShader* ef, tuk scr);

	void           mfRefreshResources(CShaderResources* Res, const IRenderer::SLoadShaderItemArgs* pArgs = 0);

	bool           mfReloadShaderFile(tukk szName, i32 nFlags);
#if DRX_PLATFORM_DESKTOP
	bool           CheckAllFilesAreWritable(tukk szDir) const;
#endif

	static void    FilterShaderCacheGenListForOrbis(FXShaderCacheCombinations& combinations);

public:
	tuk                 m_pCurScript;
	CShaderManBin         m_Bin;
	CResFileLookupDataMan m_ResLookupDataMan[2];  // cacheSource::readonly, cacheSource::user

	tukk     mfTemplateTexIdToName(i32 Id);
	SShaderGenComb* mfGetShaderGenInfo(tukk nmFX);

	bool            mfReloadShaderIncludes(tukk szPath, i32 nFlags);
	bool            mfReloadAllShaders(i32 nFlags, u32 nFlagsHW, i32 currentFrameID);
	bool            mfReloadFile(tukk szPath, tukk szName, i32 nFlags);

	void            ParseShaderProfiles();
	void            ParseShaderProfile(tuk scr, SShaderProfile* pr);

	EEfResTextures  mfCheckTextureSlotName(tukk mapname);
	SParamDB*       mfGetShaderParamDB(tukk szSemantic);
	tukk     mfGetShaderParamName(ECGParam ePR);
	bool            mfParseParamComp(i32 comp, SCGParam* pCurParam, tukk szSemantic, tuk params, tukk szAnnotations, SShaderFXParams& FXParams, CShader* ef, u32 nParamFlags, EHWShaderClass eSHClass, bool bExpressionOperand);
	bool            mfParseCGParam(tuk scr, tukk szAnnotations, SShaderFXParams& FXParams, CShader* ef, std::vector<SCGParam>* pParams, i32 nComps, u32 nParamFlags, EHWShaderClass eSHClass, bool bExpressionOperand);
	bool            mfParseFXParameter(SShaderFXParams& FXParams, SFXParam* pr, tukk ParamName, CShader* ef, bool bInstParam, i32 nParams, std::vector<SCGParam>* pParams, EHWShaderClass eSHClass, bool bExpressionOperand);

	bool            mfParseFXTexture(SShaderFXParams& FXParams, SFXTexture* pr, tukk ParamName, CShader* ef, i32 nParams, std::vector<SCGTexture>* pParams, EHWShaderClass eSHClass);
	bool            mfParseFXSampler(SShaderFXParams& FXParams, SFXSampler* pr, tukk ParamName, CShader* ef, i32 nParams, std::vector<SCGSampler>* pParams, EHWShaderClass eSHClass);

	void            mfCheckObjectDependParams(std::vector<SCGParam>& PNoObj, std::vector<SCGParam>& PObj, EHWShaderClass eSH, CShader* pFXShader);

	void            mfBeginFrame();

	void            mfGetShaderListPath(stack_string& nameOut, i32 nType);

public:
	bool                       m_bInitialized;
	bool                       m_bLoadedSystem;

	string                     m_ShadersGamePath;
	string                     m_ShadersGameExtPath;
	tukk                m_ShadersPath;
	tukk                m_ShadersExtPath;
	tukk                m_ShadersCache;
	tukk                m_ShadersFilter;
	tukk                m_ShadersMergeCachePath;
	string                     m_szUserPath;

	i32                        m_nFrameForceReload;

	char                       m_HWPath[128];

	CShader*                   m_pCurShader;
	static SResourceContainer* s_pContainer;  // List/Map of objects for shaders resource class

	std::vector<string>        m_ShaderNames;

	static CDrxNameTSCRC       s_cNameHEAD;

	static CShader*            s_DefaultShader;
	static CShader*            s_shPostEffects;      // engine specific post process effects
	static CShader*            s_shPostDepthOfField; // depth of field
	static CShader*            s_shPostMotionBlur;
	static CShader*            s_shPostSunShafts;
	static CShader*            s_sh3DHUD;

	// Deferred rendering passes
	static CShader*              s_shDeferredShading;
	static CShader*              s_ShaderDeferredCaustics;
	static CShader*              s_ShaderDeferredRain;
	static CShader*              s_ShaderDeferredSnow;

	static CShader*              s_ShaderFPEmu;
	static CShader*              s_ShaderFallback;
	static CShader*              s_ShaderScaleForm;
	static CShader*              s_ShaderStars;
	static CShader*              s_ShaderShadowBlur;
	static CShader*              s_ShaderShadowMaskGen;
#if defined(FEATURE_SVO_GI)
	static CShader*              s_ShaderSVOGI;
#endif
	static CShader*              s_shHDRPostProcess;
	static CShader*              s_shPostEffectsGame; // game specific post process effects
	static CShader*              s_shPostEffectsRenderModes;
	static CShader*              s_shPostAA;
	static CShader*              s_ShaderDebug;
	static CShader*              s_ShaderLensOptics;
	static CShader*              s_ShaderSoftOcclusionQuery;
	static CShader*              s_ShaderLightStyles;
	static CShader*              s_ShaderCommon;
	static CShader*              s_ShaderOcclTest;
	static CShader*              s_ShaderDXTCompress;
	static CShader*              s_ShaderStereo;
	static CShader*              s_ShaderClouds;
	static CShader*              s_ShaderMobileComposition;
	static CShader*              s_ShaderGpuParticles;

	const SInputShaderResources* m_pCurInputResources;
	SShaderGen*                  m_pGlobalExt;

	std::vector<SShaderGenComb>  m_SGC;

	i32                          m_nCombinationsProcess;
	i32                          m_nCombinationsProcessOverall;
	i32                          m_nCombinationsCompiled;
	i32                          m_nCombinationsEmpty;

	EShaderCacheMode             m_eCacheMode;

	tuk                        m_szShaderPrecache;

	FXShaderCacheCombinations    m_ShaderCacheCombinations[2];
	FXShaderCacheCombinations    m_ShaderCacheExportCombinations;
	FILE*                        m_FPCacheCombinations[2];

	typedef std::vector<CDrxNameTSCRC, stl::STLGlobalAllocator<CDrxNameTSCRC>> ShaderCacheMissesVec;
	ShaderCacheMissesVec    m_ShaderCacheMisses;
	string                  m_ShaderCacheMissPath;
	ShaderCacheMissCallback m_ShaderCacheMissCallback;

	SShaderCacheStatistics  m_ShaderCacheStats;

	u32                  m_nFrameLastSubmitted;
	u32                  m_nFrameSubmit;
	SShaderProfile          m_ShaderProfiles[eST_Max];
	SShaderProfile          m_ShaderFixedProfiles[eSQ_Max];

	i32                     m_bActivated;

	CShaderParserHelper     m_shaderParserHelper;

	bool                    m_bReload;

	// Shared common global flags data

	// Map used for storing automatically-generated flags and mapping old shader names masks to generated ones
	//  map< shader flag names, mask >
	typedef std::map<string, uint64> MapNameFlags;
	typedef MapNameFlags::iterator   MapNameFlagsItor;
	MapNameFlags m_pShaderCommonGlobalFlag;

	MapNameFlags m_pSCGFlagLegacyFix;
	uint64       m_nSGFlagsFix;

	// Map stored for convenience mapping betweens old flags and new ones
	//  map < shader name , map< shader flag names, mask > >
	typedef std::map<string, MapNameFlags*> ShaderMapNameFlags;
	typedef ShaderMapNameFlags::iterator    ShaderMapNameFlagsItor;
	ShaderMapNameFlags m_pShadersGlobalFlags;

	typedef std::map<CDrxNameTSCRC, SShaderGen*> ShaderExt;
	typedef ShaderExt::iterator                  ShaderExtItor;
	ShaderExt   m_ShaderExts;

	// Concatenated list of shader names using automatic masks generation
	string m_pShadersRemapList;

	// Helper functors for cleaning up

	struct SShaderMapNameFlagsContainerDelete
	{
		void operator()(ShaderMapNameFlags::value_type& pObj)
		{
			SAFE_DELETE(pObj.second);
		}
	};

public:
	CShaderMan();

	void              ShutDown();
	void              mfReleaseShaders();

	SShaderGen*       mfCreateShaderGenInfo(tukk szName, bool bRuntime);
	void              mfRemapShaderGenInfoBits(tukk szName, SShaderGen* pShGen);

	uint64            mfGetRemapedShaderMaskGen(tukk szName, uint64 nMaskGen = 0, bool bFixup = 0);
	string            mfGetShaderBitNamesFromMaskGen(tukk szName, uint64 nMaskGen);

	bool              mfUsesGlobalFlags(tukk szShaderName);
	tukk       mfGetShaderBitNamesFromGlobalMaskGen(uint64 nMaskGen);
	uint64            mfGetShaderGlobalMaskGenFromString(tukk szShaderGen);

	void              mfInitGlobal(void);
	void              mfInitLookups(void);

	void              mfPreloadShaderExts(void);
	void              mfInitCommonGlobalFlags(void);
	void              mfInitCommonGlobalFlagsLegacyFix(void);
	bool              mfRemapCommonGlobalFlagsWithLegacy(void);
	void              mfCreateCommonGlobalFlags(tukk szName);
	void              mfSaveCommonGlobalFlagsToDisk(tukk szName, u32 nMaskCount);

	void              mfInit(void);
	void              mfPostInit(void);
	void              mfSortResources();
	CShaderResources* mfCreateShaderResources(const SInputShaderResources* Res, bool bShare);
	bool              mfRefreshResourceConstants(CShaderResources* Res);
	inline bool       mfRefreshResourceConstants(SShaderItem& SI) { return mfRefreshResourceConstants((CShaderResources*)SI.m_pShaderResources); }
	bool              mfUpdateTechnik(SShaderItem& SI, CDrxNameTSCRC& Name);
	SShaderItem       mfShaderItemForName(tukk szName, bool bShare, i32 flags, SInputShaderResources* Res = NULL, uint64 nMaskGen = 0, const IRenderer::SLoadShaderItemArgs* pArgs = 0);
	CShader*          mfForName(tukk name, i32 flags, const CShaderResources* Res = NULL, uint64 nMaskGen = 0);

	bool              mfRefreshSystemShader(tukk szName, CShader*& pSysShader)
	{
		if (!pSysShader)
		{
			DrxComment("Load System Shader '%s'...", szName);

			if (pSysShader = mfForName(szName, EF_SYSTEM))
				return true;
		}

		return false;
	}

	void              RT_ParseShader(CShader* pSH, uint64 nMaskGen, u32 flags, CShaderResources* Res);
	void              RT_SetShaderQuality(EShaderType eST, EShaderQuality eSQ);

	void              CreateShaderMaskGenString(const CShader* pSH, stack_string& flagString);
	void              CreateShaderExportRequestLine(const CShader* pSH, stack_string& exportString);

	SFXParam*         mfGetFXParameter(std::vector<SFXParam>& Params, tukk param);
	SFXSampler*       mfGetFXSampler(std::vector<SFXSampler>& Params, tukk param);
	SFXTexture*       mfGetFXTexture(std::vector<SFXTexture>& Params, tukk param);
	tukk       mfParseFX_Parameter(const string& buf, EParamType eType, tukk szName);
	void              mfParseFX_Annotations_Script(tuk buf, CShader* ef, std::vector<SFXStruct>& Structs, bool* bPublic, CDrxNameR techStart[2]);
	void              mfParseFX_Annotations(tuk buf, CShader* ef, std::vector<SFXStruct>& Structs, bool* bPublic, CDrxNameR techStart[2]);
	void              mfParseFXTechnique_Annotations_Script(tuk buf, CShader* ef, std::vector<SFXStruct>& Structs, SShaderTechnique* pShTech, bool* bPublic, std::vector<SShaderTechParseParams>& techParams);
	void              mfParseFXTechnique_Annotations(tuk buf, CShader* ef, std::vector<SFXStruct>& Structs, SShaderTechnique* pShTech, bool* bPublic, std::vector<SShaderTechParseParams>& techParams);
	void              mfParseFX_Global(SFXParam& pr, CShader* ef, std::vector<SFXStruct>& Structs, CDrxNameR techStart[2]);
	bool              mfParseDummyFX_Global(std::vector<SFXStruct>& Structs, tuk annot, CDrxNameR techStart[2]);
	const string&     mfParseFXTechnique_GenerateShaderScript(std::vector<SFXStruct>& Structs, FXMacro& Macros, std::vector<SFXParam>& Params, std::vector<SFXParam>& AffectedParams, tukk szEntryFunc, CShader* ef, EHWShaderClass eSHClass, tukk szShaderName, u32& nAffectMask, tukk szType);
	bool              mfParseFXTechnique_MergeParameters(std::vector<SFXStruct>& Structs, std::vector<SFXParam>& Params, std::vector<i32>& AffectedFunc, SFXStruct* pMainFunc, CShader* ef, EHWShaderClass eSHClass, tukk szShaderName, std::vector<SFXParam>& NewParams);
	CTexture*         mfParseFXTechnique_LoadShaderTexture(STexSamplerRT* smp, tukk szName, SShaderPass* pShPass, CShader* ef, i32 nIndex, byte ColorOp, byte AlphaOp, byte ColorArg, byte AlphaArg);
	bool              mfParseFXTechnique_CustomRE(tuk buf, tukk name, SShaderTechnique* pShTech, CShader* ef);
	bool              mfParseLightStyle(CLightStyle* ls, tuk buf);
	bool              mfParseFXLightStyle(tuk buf, i32 nID, CShader* ef, std::vector<SFXStruct>& Structs);
	CShader*          mfParseFX(tuk buf, CShader* ef, CShader* efGen, uint64 nMaskGen);
	void              mfPostLoadFX(CShader* efT, std::vector<SShaderTechParseParams>& techParams, CDrxNameR techStart[2]);
	bool              mfParseDummyFX(tuk buf, std::vector<string>& ShaderNames, tukk szName);
	bool              mfAddFXShaderNames(tukk szName, std::vector<string>* ShaderNames, bool bUpdateCRC);
	bool              mfInitShadersDummy(bool bUpdateCRC);

	uint64            mfGetRTForName(tuk buf);
	u32            mfGetGLForName(tuk buf, CShader* ef);

	void              mfFillGenMacroses(SShaderGen* shG, TArray<char>& buf, uint64 nMaskGen);
	bool              mfModifyGenFlags(CShader* efGen, const CShaderResources* pRes, uint64& nMaskGen, uint64& nMaskGenHW);

	bool              mfGatherShadersList(tukk szPath, bool bCheckIncludes, bool bUpdateCRC, std::vector<string>* Names);
	void              mfGatherFilesList(tukk szPath, std::vector<CDrxNameR>& Names, i32 nLevel, bool bUseFilter, bool bMaterial = false);
	i32               mfInitShadersList(std::vector<string>* ShaderNames);
	void              mfSetDefaults(void);
	void              mfReleaseSystemShaders();
	void              mfLoadBasicSystemShaders();
	void              mfLoadDefaultSystemShaders();
	void              mfCloseShadersCache(i32 nID);
	void              mfInitShadersCacheMissLog();

	void              mfInitShadersCache(byte bForLevel, FXShaderCacheCombinations* Combinations = NULL, tukk pCombinations = NULL, i32 nType = 0);
	void              mfMergeShadersCombinations(FXShaderCacheCombinations* Combinations, i32 nType);
	void              mfInsertNewCombination(SShaderCombIdent& Ident, EHWShaderClass eCL, tukk name, i32 nID, string* Str = NULL, byte bStore = 1);
	string            mfGetShaderCompileFlags(EHWShaderClass eClass, UPipelineState pipelineState) const;

	bool              mfPreloadBinaryShaders();

	bool              LoadShaderStartupCache();
	void              UnloadShaderStartupCache();

#if DRX_PLATFORM_DESKTOP
	void AddCombination(SCacheCombination& cmb, FXShaderCacheCombinations& CmbsMap, CHWShader* pHWS);
	void AddGLCombinations(CShader* pSH, std::vector<SCacheCombination>& CmbsGL);
	void AddLTCombinations(SCacheCombination& cmb, FXShaderCacheCombinations& CmbsMap, CHWShader* pHWS);
	void AddRTCombinations(FXShaderCacheCombinations& CmbsMap, CHWShader* pHWS, CShader* pSH, FXShaderCacheCombinations* Combinations);
	void AddGLCombination(FXShaderCacheCombinations& CmbsMap, SCacheCombination& cc);
	void FilterShaderCombinations(std::vector<SCacheCombination>& Cmbs, const std::vector<CShaderListFilter>& Filters);
	void mfPrecacheShaders(bool bStatsOnly);
	void _PrecacheShaderList(bool bStatsOnly);

	void mfAddRTCombinations(FXShaderCacheCombinations& CmbsMapSrc, FXShaderCacheCombinations& CmbsMapDst, CHWShader* pSH, bool bListOnly);
	void mfAddRTCombination_r(i32 nComb, FXShaderCacheCombinations& CmbsMapDst, SCacheCombination* cmb, CHWShader* pSH, bool bAutoPrecache);
	void mfAddLTCombinations(SCacheCombination* cmb, FXShaderCacheCombinations& CmbsMapDst);
	void mfAddLTCombination(SCacheCombination* cmb, FXShaderCacheCombinations& CmbsMapDst, DWORD dwL);
#endif

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#pragma warning( push )           //AMD Port
	#pragma warning( disable : 4267 ) // conversion from 'size_t' to 'XXX', possible loss of data
#endif

	i32 Size()
	{
		i32 nSize = sizeof(*this);

		nSize += m_SGC.capacity();
		nSize += m_Bin.Size();

		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_Bin);
		pSizer->AddObject(m_SGC);
		pSizer->AddObject(m_ShaderNames);
		pSizer->AddObject(m_ShaderCacheCombinations[0]);
		pSizer->AddObject(m_ShaderCacheCombinations[1]);
	}

	static float EvalWaveForm(SWaveForm* wf);
	static float EvalWaveForm(SWaveForm2* wf);
	static float EvalWaveForm2(SWaveForm* wf, float frac);
};

#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	#pragma warning( pop ) //AMD Port
#endif

//=====================================================================

#endif                   // __CSHADER_H__
