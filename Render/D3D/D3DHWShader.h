// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   D3DCGPShader.h : Direct3D9 CG pixel shaders interface declaration.

   Revision история:
* Created by Honitch Andrey

   =============================================================================*/

#ifndef __D3DHWSHADER_H__
#define __D3DHWSHADER_H__

#include <drx3D/Render/Shaders/ShaderComponents.h>                 // SCGBind, SCGParam, etc.
#include <drx3D/Render/D3D/DeviceUpr/DeviceSubmissionQueue_D3D11.h> // CSubmissionQueue_DX11

#if DRX_PLATFORM_ORBIS && defined(USE_SCUE)
//#define USE_PER_FRAME_CONSTANT_BUFFER_UPDATES // TODO: Restore this
#endif

#define MAX_CONSTANTS_PS  512
#define MAX_CONSTANTS_VS  512
#define MAX_CONSTANTS_GS  512
#define MAX_CONSTANTS_DS  512
#define MAX_CONSTANTS_HS  512
#define MAX_CONSTANTS_CS  512

#define INST_PARAM_SIZE   sizeof(Vec4)

union DRX_ALIGN (16) UFloat4
{
	float       f[4];
	uint        ui[4];
#ifdef DRX_TYPE_SIMD4
	f32v4       v;

	ILINE void Load(const float* src)
	{
		v = *(f32v4*)src;
	}
#else
	ILINE void Load(const float* src)
	{
		f[0] = src[0];
		f[1] = src[1];
		f[2] = src[2];
		f[3] = src[3];
	}
#endif
};

class CConstantBuffer;

//==============================================================================

i32 D3DXGetSHParamHandle(uk pSH, SCGBind* pParam);

struct SParamsGroup
{
	std::vector<SCGParam> Params[2];
	std::vector<SCGParam> Params_Inst;
};

enum ED3DShError
{
	ED3DShError_NotCompiled,
	ED3DShError_CompilingError,
	ED3DShError_Fake,
	ED3DShError_Ok,
	ED3DShError_Compiling,
};

//====================================================================================

struct SCGParamsGroup
{
	u16    nParams;
	u16    nSamplers;
	u16    nTextures;
	bool      bGeneral; // Indicates that we should use fast parameters handling
	SCGParam* pParams;
	i32       nPool;
	i32       nRefCounter;
	SCGParamsGroup()
	{
		nParams = 0;
		bGeneral = false;
		nPool = 0;
		pParams = NULL;
		nRefCounter = 1;
	}
	unsigned Size()                                  { return sizeof(*this); }
	void     GetMemoryUsage(IDrxSizer* pSizer) const {}
};

#define PARAMS_POOL_SIZE 256

struct SCGParamPool
{
protected:
	PodArray<alloc_info_struct> m_alloc_info;
	Array<SCGParam>             m_Params;

public:

	SCGParamPool(i32 nEntries = 0);
	~SCGParamPool();
	SCGParamsGroup Alloc(i32 nEntries);
	bool           Free(SCGParamsGroup& Group);

	size_t         Size()
	{
		return sizeof(*this) + sizeOfV(m_alloc_info) + m_Params.size_mem();
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_alloc_info);
		pSizer->Add(m_Params.begin(), m_Params.size());
	}
};

template<>
inline bool raw_movable(const SCGParamPool&)
{
	return true;
}

class CGParamUpr
{
	friend class CHWShader_D3D;
	//friend struct CHWShader_D3D::SHWSInstance;

	static std::vector<u32, stl::STLGlobalAllocator<u32>> s_FreeGroups;

public:
	static void          Init();
	static void          Shutdown();

	static SCGParamPool* NewPool(i32 nEntries);
	static i32           GetParametersGroup(SParamsGroup& Gr, i32 nId);
	static bool          FreeParametersGroup(i32 nID);

	static std::vector<SCGParamsGroup> s_Groups;
	static DynArray<SCGParamPool>      s_Pools;
};

//=========================================================================================

#ifdef DRX_PLATFORM_DURANGO
using d3dShaderHandleType = IGraphicsUnknown;
#else
using d3dShaderHandleType = IUnknown;
#endif

class SD3DShader
{
	friend struct SD3DShaderHandle;

	d3dShaderHandleType* m_pHandle = nullptr;
	EHWShaderClass m_eSHClass;
	std::size_t    m_nSize;

	std::size_t    m_refCount = 0;

	SD3DShader(d3dShaderHandleType *handle, EHWShaderClass eSHClass, std::size_t size) : m_pHandle(handle), m_eSHClass(eSHClass), m_nSize(size) {}

public:
	~SD3DShader() noexcept;
	SD3DShader(const SD3DShader&) = delete;
	SD3DShader &operator=(const SD3DShader&) = delete;
	
	uk GetHandle() const { return m_pHandle; }
	void GetMemoryUsage(IDrxSizer* pSizer) const { pSizer->AddObject(this, sizeof(*this)); }

	void AddRef() { ++m_refCount; }
	std::size_t Release()
	{
		const auto referencesLeft = --m_refCount;
		if (referencesLeft == 0)
			delete this;

		return referencesLeft;
	}

	bool m_bDisabled = false;
};

struct SD3DShaderHandle
{
	_smart_ptr<SD3DShader> m_pShader;

	byte*       m_pData = nullptr;
	i32         m_nData = 0;
	byte        m_bStatus = 0;

	SD3DShaderHandle() = default;
	SD3DShaderHandle(_smart_ptr<SD3DShader> &&handle) noexcept : m_pShader(std::move(handle)) {}
	SD3DShaderHandle(d3dShaderHandleType *handle, EHWShaderClass eSHClass, std::size_t size) noexcept : m_pShader(new SD3DShader(handle, eSHClass, size)) {}
	SD3DShaderHandle(const SD3DShaderHandle&) = default;
	SD3DShaderHandle(SD3DShaderHandle&&) noexcept = default;
	SD3DShaderHandle& operator=(const SD3DShaderHandle&) = default;
	SD3DShaderHandle& operator=(SD3DShaderHandle&&) noexcept = default;

	void SetFake()
	{
		m_bStatus = 2;
	}
	void SetNonCompilable()
	{
		m_bStatus = 1;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_pShader);
	}
};

struct SShaderAsyncInfo
{
	SShaderAsyncInfo* m_Next;           //!<
	SShaderAsyncInfo* m_Prev;           //!<
	inline void       Unlink()
	{
		if (!m_Next || !m_Prev)
			return;
		m_Next->m_Prev = m_Prev;
		m_Prev->m_Next = m_Next;
		m_Next = m_Prev = NULL;
	}
	inline void Link(SShaderAsyncInfo* Before)
	{
		if (m_Next || m_Prev)
			return;
		m_Next = Before->m_Next;
		Before->m_Next->m_Prev = this;
		Before->m_Next = this;
		m_Prev = Before;
	}
	static void FlushPendingShaders();

#if DRX_PLATFORM_DURANGO //|| DRX_RENDERER_OPENGL
	#define LPD3DXBUFFER    D3DBlob *
	#define ID3DXBuffer     D3DBlob
#endif

	i32                  m_nHashInstance;
	uint64               m_RTMask;
	u32               m_LightMask;
	u32               m_MDMask;
	u32               m_MDVMask;
	EHWShaderClass       m_eClass;
	UPipelineState       m_pipelineState;
	class CHWShader_D3D* m_pShader;
	CShader*             m_pFXShader;

	D3DBlob*             m_pDevShader;
	D3DBlob*             m_pErrors;
	uk                m_pConstants;
	string               m_Name;
	string               m_Text;
	string               m_Errors;
	string               m_Profile;
	string               m_RequestLine;
	string               m_shaderList;
	//CShaderThread *m_pThread;
	std::vector<SCGBind> m_InstBindVars;
	byte                 m_bPending;
	bool                 m_bPendedFlush;
	bool                 m_bPendedSamplers;
	bool                 m_bPendedEnv;
	bool                 m_bDeleteAfterRequest;
	float                m_fMinDistance;
	i32                  m_nFrame;
	i32                  m_nThread;
	i32                  m_nCombination;

	SShaderAsyncInfo()
		: m_Next(nullptr)
		, m_Prev(nullptr)
		, m_nHashInstance(-1)
		, m_RTMask(0)
		, m_LightMask(0)
		, m_MDMask(0)
		, m_MDVMask(0)
		, m_eClass(eHWSC_Num)
		, m_pShader(nullptr)
		, m_pFXShader(nullptr)
		, m_pDevShader(nullptr)
		, m_pErrors(nullptr)
		, m_pConstants(nullptr)
		, m_bPending(true) // this flag is now used as an atomic indication that if the async shader has been compiled
		, m_bPendedFlush(false)
		, m_bPendedSamplers(false)
		, m_bPendedEnv(false)
		, m_bDeleteAfterRequest(false)
		, m_fMinDistance(0.0f)
		, m_nFrame(0)
		, m_nThread(0)
		, m_nCombination(-1)
	{
	}

	~SShaderAsyncInfo();
	static  i32      s_nPendingAsyncShaders;
	static i32               s_nPendingAsyncShadersFXC;
	static SShaderAsyncInfo& PendingList();
	static SShaderAsyncInfo& PendingListT();
	static DrxEvent          s_RequestEv;
};

#ifdef SHADER_ASYNC_COMPILATION

	#include <drx3D/CoreX/Thread/IThreadUpr.h>
	#define SHADER_THREAD_NAME "ShaderCompile"

class CAsyncShaderTask
{
	friend class CD3D9Renderer; // so it can instantiate us

public:
	CAsyncShaderTask();

	static void InsertPendingShader(SShaderAsyncInfo* pAsync);
	i32         GetThread()
	{
		return m_nThread;
	}
	i32 GetThreadFXC()
	{
		return m_nThreadFXC;
	}
	void SetThread(i32 nThread)
	{
		m_nThread = nThread;
		m_nThreadFXC = -1;
	}
	void SetThreadFXC(i32 nThread)
	{
		m_nThreadFXC = nThread;
	}

private:
	void                     FlushPendingShaders();

	static SShaderAsyncInfo& BuildList();
	SShaderAsyncInfo         m_flush_list;
	i32                      m_nThread;
	i32                      m_nThreadFXC;

	class CShaderThread : public IThread
	{
	public:
		CShaderThread(CAsyncShaderTask* task)
			: m_task(task)
			, m_quit(false)
		{
			CAsyncShaderTask::BuildList().m_Next = &CAsyncShaderTask::BuildList();
			CAsyncShaderTask::BuildList().m_Prev = &CAsyncShaderTask::BuildList();

			task->m_flush_list.m_Next = &task->m_flush_list;
			task->m_flush_list.m_Prev = &task->m_flush_list;

			if (!gEnv->pThreadUpr->SpawnThread(this, SHADER_THREAD_NAME))
			{
				DrxFatalError("Error spawning \"%s\" thread.", SHADER_THREAD_NAME);
			}
		}

		~CShaderThread()
		{
			m_quit = true;
			gEnv->pThreadUpr->JoinThread(this, eJM_Join);
		}

	private:
		// Start accepting work on thread
		virtual void ThreadEntry();

		CAsyncShaderTask* m_task;
		 bool     m_quit;
	};

	CShaderThread m_thread;

	bool CompileAsyncShader(SShaderAsyncInfo* pAsync);
	void SubmitAsyncRequestLine(SShaderAsyncInfo* pAsync);
	bool PostCompile(SShaderAsyncInfo* pAsync);
};
#endif

tukk GetShaderlistName(u32 nPlatform);
tukk CurrentPlatformShaderListFile();

class CHWShader_D3D : public CHWShader
{
	// TODO: remove all of these friends (via public access)
	friend class CCompiledRenderObject;
	friend class CD3D9Renderer;
	friend class CAsyncShaderTask;
	friend class CGParamUpr;
	friend struct SShaderAsyncInfo;
	friend class CHWShader;
	friend class CShaderMan;
	friend struct InstContainerByHash;
	friend class CREGeomCache;
	friend struct SRenderStatePassD3D;
	friend struct SDeviceObjectHelpers;
	friend class CDeviceGraphicsCommandList;
	friend class CDeviceGraphicsPSO;
	friend class CDeviceGraphicsPSO_DX11;
	friend class CDeviceGraphicsPSO_DX12;
	friend class CGnmGraphicsPipelineState;
	friend class CDeviceObjectFactory;
	friend struct SInputLayout;
	friend class CDeviceGraphicsPSO_Vulkan;

#if DRX_PLATFORM_DESKTOP
	SPreprocessTree* m_pTree;
	CParserBin*      m_pParser;
#endif

	struct SHWSInstance
	{
		friend struct SShaderAsyncInfo;

		SShaderBlob                m_Shader;

		SShaderCombIdent           m_Ident;

		SD3DShaderHandle           m_Handle;
		EHWShaderClass             m_eClass;

		std::vector<const SFXTexture*> m_pFXTextures;
		
		i32                        m_nParams[2]; // 0: Instance independent; 1: Instance depended
		std::vector<STexSamplerRT> m_pSamplers;
		std::vector<SCGSampler>    m_Samplers;
		std::vector<SCGTexture>    m_Textures;
		std::vector<SCGBind>       m_pBindVars;
		i32                        m_nParams_Inst;
		float                      m_fLastAccess;
		i32                        m_nUsed;
		i32                        m_nUsedFrame;
		i32                        m_nFrameSubmit;
		i32                        m_nMaxVecs[CB_NUM];
		short                      m_nInstMatrixID;
		short                      m_nInstIndex;
		short                      m_nInstructions;
		short                      m_nTempRegs;
		u16                     m_VStreamMask_Stream;
		u16                     m_VStreamMask_Decl;
		short                      m_nParent;
		byte                       m_bDeleted         : 1;
		byte                       m_bHasPMParams     : 1;
		byte                       m_bFallback        : 1;
		byte                       m_bAsyncActivating : 1;
		byte                       m_bHasSendRequest  : 1;
		InputLayoutHandle          m_nVertexFormat;
		byte                       m_nNumInstAttributes;

		i32                        m_DeviceObjectID;
		SShaderAsyncInfo*  m_pAsync;

#if DRX_RENDERER_VULKAN
		std::vector<SVertexInputStream>  m_VSInputStreams;
#endif

		SHWSInstance()
			: m_Ident()
			, m_Handle()
			, m_eClass(eHWSC_Num)
			, m_nParams_Inst(-1)
			, m_fLastAccess(0.0f)
			, m_nUsed(0)
			, m_nUsedFrame(0)
			, m_nFrameSubmit(0)
			, m_nInstMatrixID(1)
			, m_nInstIndex(-1)
			, m_nInstructions(0)
			, m_nTempRegs(0)
			, m_VStreamMask_Stream(0)
			, m_VStreamMask_Decl(0)
			, m_nParent(-1)
			, m_bDeleted(false)
			, m_bHasPMParams(false)
			, m_bFallback(false)
			, m_bAsyncActivating(false)
			, m_bHasSendRequest(false)
			, m_nVertexFormat(EDefaultInputLayouts::P3F_C4B_T2F)
			, m_nNumInstAttributes(0)
			, m_DeviceObjectID(-1)
			, m_pAsync(nullptr)
		{
			m_nParams[0] = -1;
			m_nParams[1] = -1;
			m_nMaxVecs[0] = 0;
			m_nMaxVecs[1] = 0;

			m_Shader.m_nDataSize = 0;
			m_Shader.m_pShaderData = nullptr;
		}

		void Release();
		void GetInstancingAttribInfo(u8 Attributes[32], i32 & nUsedAttr, i32& nInstAttrMask);

		i32  Size()
		{
			i32 nSize = sizeof(*this);
			nSize += sizeOfV(m_pSamplers);
			nSize += sizeOfV(m_pBindVars);

			return nSize;
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(m_Handle);
			pSizer->AddObject(m_pSamplers);
			pSizer->AddObject(m_pBindVars);
			pSizer->AddObject(m_Shader.m_pShaderData, m_Shader.m_nDataSize);
		}

		bool IsAsyncCompiling()
		{
			if (m_pAsync || m_bAsyncActivating)
				return true;

			return false;
		}
	};

	typedef std::vector<SHWSInstance*> InstContainer;
	typedef InstContainer::iterator    InstContainerIt;

public:

	SHWSInstance* m_pCurInst;
	InstContainer m_Insts;

	static i32    m_FrameObj;

	// FX support
	i32 m_nCurInstFrame;

	// Bin FX support
	FXShaderToken  m_TokenTable;
	TArray<u32> m_TokenData;

	virtual i32 Size() override
	{
		i32 nSize = sizeof(*this);
		nSize += sizeOfVP(m_Insts);
		//nSize += sizeOfMapP(m_LookupMap);
		nSize += sizeofVector(m_TokenData);
		nSize += sizeOfV(m_TokenTable);

		return nSize;
	}

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
		//pSizer->AddObject(m_pCurInst); // crahes.., looks like somewhere this ptr is not set back to NULL
		pSizer->AddObject(m_Insts);
		//pSizer->AddObject( m_LookupMap );
		pSizer->AddObject(m_TokenData);
		pSizer->AddObject(m_TokenTable);

		pSizer->AddObject(m_CachedTokens.data(), m_CachedTokens.size());
		pSizer->AddObject(m_pCache);
	}
	CHWShader_D3D()
	{
#if DRX_PLATFORM_DESKTOP
		m_pTree = NULL;
		m_pParser = NULL;
#endif
		mfConstruct();
	}

	static void mfInit();
	void        mfConstruct();

	void mfFree()
	{
#if DRX_PLATFORM_DESKTOP
		SAFE_DELETE(m_pTree);
		SAFE_DELETE(m_pParser);
#endif

		m_Flags = 0;
		mfReset();
	}

	//============================================================================
	// Binary cache support
	SDeviceShaderEntry mfGetCacheItem(CShader* pFX, tukk name, SDiskShaderCache *cache, u32& nFlags);
	bool         mfAddCacheItem(SDiskShaderCache* pCache, SShaderCacheHeaderItem* pItem, const byte* pData, i32 nLen, bool bFlush, CDrxNameTSCRC Name);

	static byte* mfBindsToCache(SHWSInstance* pInst, std::vector<SCGBind>* Binds, i32 nParams, byte* pP);
	const  byte* mfBindsFromCache(std::vector<SCGBind>& Binds, i32 nParams, const byte* pP);

	bool         mfActivateCacheItem(CShader* pSH, const SDeviceShaderEntry* cacheEntry, u32 nFlags);
	bool         mfCreateCacheItem(SHWSInstance* pInst, CShader *ef, std::vector<SCGBind>& InstBinds, byte* pData, i32 nLen, bool bShaderThread);

	//============================================================================

	i32 mfGetParams(i32 Type)
	{
		assert(m_pCurInst);
		return m_pCurInst->m_nParams[Type];
	}

	bool mfSetHWStartProfile(u32 nFlags);
	//bool mfNextProfile(u32 nFlags);

	void        mfSaveCGFile(tukk scr, tukk path);
	void        mfOutputCompilerError(string& strErr, tukk szSrc);
	static bool mfCreateShaderEnv(i32 nThread, SHWSInstance* pInst, D3DBlob* pShader, uk & pConstantTable, D3DBlob*& pErrorMsgs, std::vector<SCGBind>& InstBindVars, CHWShader_D3D* pSH, bool bShaderThread, CShader* pFXShader, i32 nCombination, tukk src = NULL);
	void        mfPrintCompileInfo(SHWSInstance* pInst);
	bool        mfCompileHLSL_Int(CShader* pSH, tuk prog_text, D3DBlob** ppShader, uk * ppConstantTable, D3DBlob** ppErrorMsgs, string& strErr, std::vector<SCGBind>& InstBindVars);

	i32         mfAsyncCompileReady(SHWSInstance* pInst);
	bool        mfRequestAsync(CShader* pSH, SHWSInstance* pInst, std::vector<SCGBind>& InstBindVars, tukk prog_text, tukk szProfile, tukk szEntry);

	void        mfSubmitRequestLine(SHWSInstance* pInst, string* pRequestLine = NULL);
	D3DBlob*    mfCompileHLSL(CShader* pSH, tuk prog_text, uk * ppConstantTable, D3DBlob** ppErrorMsgs, u32 nFlags, std::vector<SCGBind>& InstBindVars);
	bool        mfUploadHW(SHWSInstance* pInst, const byte* pBuf, u32 nSize, CShader* pSH, u32 nFlags);
	bool        mfUploadHW(D3DBlob* pShader, SHWSInstance* pInst, CShader* pSH, u32 nFlags);

	ED3DShError mfIsValid_Int(SHWSInstance*& pInst, bool bFinalise);

	//ILINE most common outcome (avoid LHS on link register 360)
	ILINE ED3DShError mfIsValid(SHWSInstance*& pInst, bool bFinalise)
	{
		if (pInst->m_Handle.m_pShader)
			return ED3DShError_Ok;
		if (pInst->m_bAsyncActivating)
			return ED3DShError_NotCompiled;

		return mfIsValid_Int(pInst, bFinalise);
	}
	
	ED3DShError mfFallBack(SHWSInstance*& pInst, i32 nStatus);
	void        mfCommitCombinations(i32 nFrame, i32 nFrameDiff);
	void        mfCommitCombination(SHWSInstance* pInst, i32 nFrame, i32 nFrameDiff);
	void        mfLogShaderCacheMiss(SHWSInstance* pInst);
	void        mfLogShaderRequest(SHWSInstance* pInst);

	
	static void   mfSetParameters(SCGParam* pParams, i32k nParams, EHWShaderClass eSH, i32 nMaxRegs, Vec4* pOutBuffer, u32 outBufferSize, const D3DViewPort* pVP);   // handles all the parameter except PI and SI ones

	//============================================================================

	void mfLostDevice(SHWSInstance* pInst, byte* pBuf, i32 nSize)
	{
		pInst->m_Handle.SetFake();
		pInst->m_Handle.m_pData = new byte[nSize];
		memcpy(pInst->m_Handle.m_pData, pBuf, nSize);
		pInst->m_Handle.m_nData = nSize;
	}

	bool PrecacheShader(CShader* pSH, const SShaderCombIdent &cache,u32 nFlags) override;

	i32 CheckActivation(CShader* pSH, SHWSInstance*& pInst, u32 nFlags);

	SHWSInstance* mfGetInstance(SShaderCombIdent& Ident, u32 nFlags);
	SHWSInstance* mfGetInstance(i32 nHashInstance, SShaderCombIdent& Ident);
	SHWSInstance* mfGetHashInst(InstContainer *pInstCont, u32 identHash, SShaderCombIdent& Ident, InstContainerIt& it);
	void          mfPrepareShaderDebugInfo(SHWSInstance* pInst, tukk szAsm, std::vector<SCGBind>& InstBindVars, uk pBuffer);
	void          mfGetSrcFileName(tuk srcName, i32 nSize);
	void          mfGetDstFileName(SHWSInstance* pInst, tuk dstname, i32 nSize, byte bType);
	static void   mfGenName(SHWSInstance* pInst, tuk dstname, i32 nSize, byte bType);
	void          CorrectScriptEnums(CParserBin& Parser, SHWSInstance* pInst, std::vector<SCGBind>& InstBindVars, const FXShaderToken& Table);
	bool          ConvertBinScriptToASCII(CParserBin& Parser, SHWSInstance* pInst, std::vector<SCGBind>& InstBindVars, const FXShaderToken& Table, TArray<char>& Scr);
	bool          AddResourceLayoutToScriptHeader(SHWSInstance* pInst, tukk szProfile, tukk pFunCDrxName, TArray<char>& Scr);
	void          RemoveUnaffectedParameters_D3D10(CParserBin& Parser, SHWSInstance* pInst, std::vector<SCGBind>& InstBindVars);
	void          AddResourceLayoutToBinScript(CParserBin& Parser, SHWSInstance* pInst, FXShaderToken* Table);
	bool          mfStoreCacheTokenMap(const FXShaderToken& Table, const TArray<u32>& pSHData);
	void          mfSetDefaultRT(uint64& nAndMask, uint64& nOrMask);
	bool          AutoGenMultiresGS(TArray<char>& sNewScr, CShader *pSH);

public:
	bool        mfGetCacheTokenMap(FXShaderToken& Table, TArray<u32>& pSHData);
	bool        mfGenerateScript(CShader* pSH, SHWSInstance* pInst, std::vector<SCGBind>& InstBindVars, u32 nFlags, TArray<char>& sNewScr);
	bool        mfActivate(CShader* pSH, u32 nFlags);

	void        SetTokenFlags(u32 nToken);
	uint64      CheckToken(u32 nToken);
	uint64      CheckIfExpr_r(u32k* pTokens, u32& nCur, u32 nSize);
	void        mfConstructFX_Mask_RT(const TArray<u32>& pSHData);
	void        mfConstructFX(const FXShaderToken& Table, const TArray<u32>& pSHData);

	static bool mfAddFXSampler(SHWSInstance* pInst, SShaderFXParams& FXParams, SFXSampler* pr, tukk ParamName, SCGBind* pBind, CShader* ef, EHWShaderClass eSHClass);
	static bool mfAddFXTexture(SHWSInstance* pInst, SShaderFXParams& FXParams, SFXTexture* pr, tukk ParamName, SCGBind* pBind, CShader* ef, EHWShaderClass eSHClass);

	static void mfAddFXParameter(SHWSInstance* pInst, SParamsGroup& OutParams, SShaderFXParams& FXParams, SFXParam* pr, tukk ParamName, SCGBind* pBind, CShader* ef, bool bInstParam, EHWShaderClass eSHClass);
	static bool mfAddFXParameter(SHWSInstance* pInst, SParamsGroup& OutParams, SShaderFXParams& FXParams, tukk param, SCGBind* bn, bool bInstParam, EHWShaderClass eSHClass, CShader* pFXShader);
	static void mfGatherFXParameters(SHWSInstance* pInst, std::vector<SCGBind>& BindVars, CHWShader_D3D* pSH, i32 nFlags, CShader* pFXShader);

	static void mfCreateBinds(std::vector<SCGBind> &binds, ukk pConstantTable, std::size_t nSize);
	static void mfPostVertexFormat(SHWSInstance * pInst, CHWShader_D3D * pHWSH, bool bCol, byte bNormal, bool bTC0, bool bTC1[2], bool bPSize, bool bTangent[2], bool bBitangent[2], bool bHWSkin, bool bSH[2], bool bMorphTarget, bool bMorph);
	void        mfUpdateFXVertexFormat(SHWSInstance* pInst, CShader* pSH);

	void        ModifyLTMask(u32& nMask);

public:
	virtual ~CHWShader_D3D();

	bool                mfAddEmptyCombination(uint64 nRT, uint64 nGL, u32 nLT, const SCacheCombination& cmbSaved) override;
	bool                mfStoreEmptyCombination(SEmptyCombination& Comb) override;

	virtual void        mfReset() override;
	virtual tukk mfGetEntryName() override { return m_EntryFunc.c_str(); }
	virtual bool        mfFlushCacheFile() override;
	virtual bool        Export(CShader *pSH, SShaderSerializeContext& SC) override;

	enum class cacheValidationResult { ok, no_lookup, version_mismatch, checksum_mismatch };
	cacheValidationResult mfValidateCache(const SDiskShaderCache &cache);

	bool               mfWarmupCache(CShader* pFX);
	void               mfPrecacheAllCombinations(CShader* pFX, CResFileOpenScope &rfOpenGuard, SDiskShaderCache &cache);
	SDeviceShaderEntry mfShaderEntryFromCache(CShader* pFX, const CDirEntry& de, CResFileOpenScope &rfOpenGuard, SDiskShaderCache &cache);

	// Vertex shader specific functions
	virtual InputLayoutHandle mfVertexFormat(bool& bUseTangents, bool& bUseLM, bool& bUseHWSkin) override;
	static InputLayoutHandle  mfVertexFormat(SHWSInstance* pInst, CHWShader_D3D* pSH, D3DBlob* pBuffer, uk pConstantTable);
	virtual void          mfUpdatePreprocessFlags(SShaderTechnique* pTech) override;

	virtual tukk   mfGetActivatedCombinations(bool bForLevel) override;

	static u16         GetDeclaredVertexStreamMask(uk pHwInstance)
	{
		DRX_ASSERT(pHwInstance && reinterpret_cast<SHWSInstance*>(pHwInstance)->m_eClass == eHWSC_Vertex);
		return reinterpret_cast<SHWSInstance*>(pHwInstance)->m_VStreamMask_Decl;
	}

	static void ShutDown();

	static Vec4 GetVolumetricFogParams(const CCamera& camera);
	static Vec4 GetVolumetricFogRampParams();
	static Vec4 GetVolumetricFogSunDir(const Vec3& sunDir);
	static void GetFogColorGradientConstants(Vec4& fogColGradColBase, Vec4& fogColGradColDelta);
	static Vec4 GetFogColorGradientRadial(const CCamera& camera);

	// Import/Export
	bool ExportSamplers(SCHWShader& SHW, SShaderSerializeContext& SC);
	bool ExportParams(SCHWShader& SHW, SShaderSerializeContext& SC);

	static i32                          s_nActivationFailMask;

	static bool                         s_bInitShaders;

	static i32                          s_nResetDeviceFrame;
	static i32                          s_nInstFrame;

	static i32                          s_nDevicePSDataSize;
	static i32                          s_nDeviceVSDataSize;
};

#endif  // __D3DHWSHADER_H__
