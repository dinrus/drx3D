// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/ParamLoader.h>
#include <drx3D/Animation/AttachmentVCloth.h>
#include <drx3D/Animation/CharacterInstanceProcessing.h>

class CSkin;              //default skinning
class CAttachmentSKIN;    //skin-instance
class CAttachmentVCLOTH;  //cloth-instance
class CDefaultSkeleton;   //default skeleton
class CCharInstance;      //skel-instance
class CAttachmentUpr; //skel-instance
class CClothUpr;
class CAttachmentMerger;
class CFacialAnimation;
struct IAnimationSet;
class CPoseModifierSetup;

DECLARE_SHARED_POINTERS(CPoseModifierSetup);

extern float g_YLine;

struct DebugInstances
{
	_smart_ptr<ICharacterInstance> m_pInst;
	QuatTS                         m_GridLocation;
	f32                            m_fExtrapolation;
	ColorF                         m_AmbientColor;
};

struct CDefaultSkeletonReferences
{
	CDefaultSkeleton*        m_pDefaultSkeleton;
	DynArray<CCharInstance*> m_RefByInstances;
	CDefaultSkeletonReferences()
	{
		m_pDefaultSkeleton = 0;
		m_RefByInstances.reserve(0x10);
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};
struct CDefaultSkinningReferences
{
	CSkin*                       m_pDefaultSkinning;
	DynArray<CAttachmentSKIN*>   m_RefByInstances;
	DynArray<CAttachmentVCLOTH*> m_RefByInstancesVCloth;
	CDefaultSkinningReferences()
	{
		m_pDefaultSkinning = 0;
		m_RefByInstances.reserve(0x10);
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct CharacterAttachment
{
	CharacterAttachment()
		: m_relRotation(false)
		, m_relPosition(false)
		, m_RelativeDefault(IDENTITY)
		, m_AbsoluteDefault(IDENTITY)
		, m_Type(0xDeadBeef)
		, m_AttFlags(0)
		, m_ProxyParams(0, 0, 0, 0)
		, m_ProxyPurpose(0)
	{
		memset(&m_AttPhysInfo, 0, sizeof(m_AttPhysInfo));
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_strAttachmentName);
		pSizer->AddObject(m_strJointName);
		pSizer->AddObject(m_strBindingPath);
		pSizer->AddObject(m_pMaterial);
		pSizer->AddObject(m_parrMaterial);
		pSizer->AddObject(m_pStaticObject);
	}

	string                m_strAttachmentName;
	u32                m_Type;
	u32                m_AttFlags;
	string                m_strJointName;
	bool                  m_relRotation;
	bool                  m_relPosition;
	QuatT                 m_RelativeDefault;
	QuatT                 m_AbsoluteDefault;
	Vec4                  m_ProxyParams;
	u32                m_ProxyPurpose;

	SimulationParams      ap;

	string                m_strRowJointName;
	RowSimulationParams   rowap;

	SVClothParams         clothParams;

	string                m_strBindingPath;
	string                m_strSimBindingPath;
	_smart_ptr<IMaterial> m_pMaterial;                         //this is the shared material for all LODs
	_smart_ptr<IMaterial> m_parrMaterial[g_nMaxGeomLodLevels]; //some LODs can have an individual material
	_smart_ptr<IStatObj>  m_pStaticObject;

	DrxBonePhysics        m_AttPhysInfo[2];
};

struct CharacterDefinition
{
	virtual ~CharacterDefinition(){}
	CharacterDefinition()
	{
		m_nRefCounter = 0;
		m_nKeepInMemory = 0;
		m_nKeepModelsInMemory = -1;
	}

	virtual void AddRef()
	{
		++m_nRefCounter;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_strFilePath);
		pSizer->AddObject(m_strBaseModelFilePath);
		pSizer->AddObject(m_pBaseModelMaterial);
		pSizer->AddObject(m_arrAttachments);
	}

	string                        m_strFilePath;
	string                        m_strBaseModelFilePath;
	_smart_ptr<IMaterial>         m_pBaseModelMaterial;
	i32                           m_nKeepModelsInMemory; // Reference count.
	i32                           m_nRefCounter;         // Reference count.
	i32                           m_nKeepInMemory;
	DynArray<CharacterAttachment> m_arrAttachments;
	CPoseModifierSetupPtr         m_pPoseModifierSetup;
};

//////////////////////////////////////////////////////////////////////
// This class contains a list of character bodies and list of character instances.
// On attempt to create same character second time only new instance will be created.
// Only this class can create actual characters.
class CharacterUpr : public ICharacterUpr
{
public:
	static bool   s_bPaused;
	static u32 s_renderFrameIdLocal;
	static u32 GetRendererThreadId();
	static u32 GetRendererMainThreadId();

	friend class CAnimationUpr;
	friend class CAttachmentUpr;
	friend class CAttachmentMerger;
	friend class CAttachmentSKIN;
	friend class CAttachmentVCLOTH;
	friend class CClothPiece;
	friend class CCharInstance;
	friend class CDefaultSkeleton;
	friend class CSkin;
	friend class CParamLoader;
#if BLENDSPACE_VISUALIZATION
	friend class CSkeletonAnim;
	friend struct SParametricSamplerInternal;
#endif

	CharacterUpr();
	~CharacterUpr();
	virtual void Release(); // Deletes itself

	virtual void PostInit();

	virtual void SyncAllAnimations();

	void         ClearPoseModifiersFromSynchQueue();
	void         StopAnimationsOnAllInstances();

	/**
	 * Stop animations on all instances using the specified animation set.
	 */
	void                                        StopAnimationsOnAllInstances(const IAnimationSet& animationSet);

	CharacterInstanceProcessing::CContextQueue& GetContextSyncQueue() { return m_ContextSyncQueue; };

	void                                        UpdateRendererFrame();

	// should be called every frame
	void           Update(bool bPause);
	void           UpdateStreaming(i32 nFullUpdateRoundId, i32 nFastUpdateRoundId);
	void           DatabaseUnloading();

	CAnimationSet* GetAnimationSetUsedInCharEdit();

	void           DummyUpdate(); // can be called instead of Update() for UI purposes (such as in preview viewports, etc).

	//////////////////////////////////////////////////////////////////////////
	IFacialAnimation*        GetIFacialAnimation();
	const IFacialAnimation*  GetIFacialAnimation() const;

	IAnimEvents*             GetIAnimEvents()            { return &g_AnimationUpr; };
	const IAnimEvents*       GetIAnimEvents() const      { return &g_AnimationUpr; };

	CAnimationUpr&       GetAnimationUpr()       { return m_AnimationUpr; };
	const CAnimationUpr& GetAnimationUpr() const { return m_AnimationUpr; };

	CParamLoader&            GetParamLoader()            { return m_ParamLoader; };

	CFacialAnimation*        GetFacialAnimation()        { return m_pFacialAnimation; }
	const CFacialAnimation*  GetFacialAnimation() const  { return m_pFacialAnimation; }

	const IAttachmentMerger& GetIAttachmentMerger() const;

	//a list with model-names that use "ForceSkeletonUpdates"
	std::vector<string>          m_arrSkeletonUpdates;
	std::vector<u32>          m_arrAnimPlaying;
	std::vector<u32>          m_arrForceSkeletonUpdates;
	std::vector<u32>          m_arrVisible;
	u32                       m_nUpdateCounter;
	u32                       m_AllowStartOfAnimation;
	u32                       g_SkeletonUpdates;
	u32                       g_AnimationUpdates;

	IAnimationStreamingListener* m_pStreamingListener;

	//methods to handle animation assets
	void SetAnimMemoryTracker(const SAnimMemoryTracker& amt)
	{
		g_AnimationUpr.m_AnimMemoryTracker.m_nMemTracker = amt.m_nMemTracker;
		g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsCurrent = amt.m_nAnimsCurrent;
		g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsMax = amt.m_nAnimsMax;
		g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsAdd = amt.m_nAnimsAdd;
		g_AnimationUpr.m_AnimMemoryTracker.m_nAnimsCounter = amt.m_nAnimsCounter;
	}
	SAnimMemoryTracker GetAnimMemoryTracker() const
	{
		return g_AnimationUpr.m_AnimMemoryTracker;
	}

	bool             DBA_PreLoad(tukk filepath, ICharacterUpr::EStreamingDBAPriority priority);
	bool             DBA_LockStatus(tukk filepath, u32 status, ICharacterUpr::EStreamingDBAPriority priority);
	bool             DBA_Unload(tukk filepath);
	bool             DBA_Unload_All();

	virtual bool     CAF_AddRef(u32 filePathCRC);
	virtual bool     CAF_IsLoaded(u32 filePathCRC) const;
	virtual bool     CAF_Release(u32 filePathCRC);
	virtual bool     CAF_LoadSynchronously(u32 filePathCRC);
	virtual bool     LMG_LoadSynchronously(u32 filePathCRC, const IAnimationSet* pAnimationSet);

	virtual bool     CAF_AddRefByGlobalId(i32 globalID);
	virtual bool     CAF_ReleaseByGlobalId(i32 globalID);

	EReloadCAFResult ReloadCAF(tukk szFilePathCAF)
	{
		return g_AnimationUpr.ReloadCAF(szFilePathCAF);
	};
	i32 ReloadLMG(tukk szFilePathLMG)
	{
		return g_AnimationUpr.ReloadLMG(szFilePathLMG);
	};

	CDefaultSkeleton* GetModelByAimPoseID(u32 nGlobalIDAimPose);
	tukk       GetDBAFilePathByGlobalID(i32 globalID) const;
	virtual void      SetStreamingListener(IAnimationStreamingListener* pListener) { m_pStreamingListener = pListener; };

	// light profiler functions
	virtual void                AddFrameTicks(uint64 nTicks)     { m_nFrameTicks += nTicks; }
	virtual void                AddFrameSyncTicks(uint64 nTicks) { m_nFrameSyncTicks += nTicks; }
	virtual void                ResetFrameTicks()                { m_nFrameTicks = 0; m_nFrameSyncTicks = 0; }
	virtual uint64              NumFrameTicks() const            { return m_nFrameTicks; }
	virtual uint64              NumFrameSyncTicks() const        { return m_nFrameSyncTicks; }
	virtual u32              NumCharacters() const            { return m_nActiveCharactersLastFrame; }

	void                        UpdateDatabaseUnloadTimeStamp();
	u32                      GetDatabaseUnloadTimeDelta() const;

	ICharacterInstance*         LoadCharacterDefinition(const string pathname, u32 nLoadingFlags = 0);
	i32                       LoadCDF(tukk pathname);
	i32                       LoadCDFFromXML(XmlNodeRef root, tukk pathname);
	void                        ReleaseCDF(tukk pathname);
	u32                      GetCDFId(const string& pathname);
	u32                      GetOrLoadCDFId(const string& pathname);
	bool                        StreamKeepCDFResident(tukk szFilePath, i32 nLod, i32 nRefAdj, bool bUrgent);
	CDefaultSkinningReferences* GetDefaultSkinningReferences(CSkin* pDefaultSkinning);

private:
	void UpdateInstances(bool bPause);

	u32 m_StartGAH_Iterator;
	void   LoadAnimationImageFile(tukk filenameCAF, tukk filenameAIM);
	bool   LoadAnimationImageFileCAF(tukk filenameCAF);
	bool   LoadAnimationImageFileAIM(tukk filenameAIM);
	u32 IsInitializedByIMG() { return m_InitializedByIMG;  };
	u32 m_InitializedByIMG;

	void DumpAssetStatistics();
	f32  GetAverageFrameTime(f32 sec, f32 FrameTime, f32 TimeScale, f32 LastAverageFrameTime);

	//methods to manage instances
	virtual ICharacterInstance* CreateInstance(tukk szFilePath, u32 nLoadingFlags = 0);
	ICharacterInstance*         CreateCGAInstance(tukk szFilePath, u32 nLoadingFlags = 0);
	ICharacterInstance*         CreateSKELInstance(tukk szFilePath, u32 nLoadingFlags);
	void                        RegisterInstanceSkel(CDefaultSkeleton* pDefaultSkeleton, CCharInstance* pInstance);
	void                        UnregisterInstanceSkel(CDefaultSkeleton* pDefaultSkeleton, CCharInstance* pInstance);
	void                        RegisterInstanceSkin(CSkin* pDefaultSkinning, CAttachmentSKIN* pInstance);
	void                        UnregisterInstanceSkin(CSkin* pDefaultSkinning, CAttachmentSKIN* pInstance);
	void                        RegisterInstanceVCloth(CSkin* pDefaultSkinning, CAttachmentVCLOTH* pInstance);
	void                        UnregisterInstanceVCloth(CSkin* pDefaultSkinning, CAttachmentVCLOTH* pInstance);
	virtual u32              GetNumInstancesPerModel(const IDefaultSkeleton& rIDefaultSkeleton) const;
	virtual ICharacterInstance* GetICharInstanceFromModel(const IDefaultSkeleton& rIDefaultSkeleton, u32 num) const;
	void GetCharacterInstancesSize(class IDrxSizer* pSizer) const;

	//methods to manage skels and skins
	virtual IDefaultSkeleton* LoadModelSKEL(tukk szFilePath, u32 nLoadingFlags);
	virtual ISkin*            LoadModelSKIN(tukk szFilePath, u32 nLoadingFlags);
	CDefaultSkeleton*         FetchModelSKEL(tukk szFilePath, u32 nLoadingFlags);
	CDefaultSkeleton*         FetchModelSKELForGCA(tukk szFilePath, u32 nLoadingFlags);
	CSkin*                    FetchModelSKIN(tukk szFilePath, u32 nLoadingFlags);
	void                      PreloadModelsCDF();
	void                      PreloadModelsCHR();
	void                      PreloadModelsCGA();
	void                      RegisterModelSKEL(CDefaultSkeleton* pModelSKEL, u32 nLoadingFlags);
	void                      RegisterModelSKIN(CSkin* pModelSKIN, u32 nLoadingFlags);
	void                      UnregisterModelSKEL(CDefaultSkeleton* pModelSKEL);
	void                      UnregisterModelSKIN(CSkin* pModelSKIN);
	void                      SkelExtension(CCharInstance* pCharInstance, tukk pFilepathSKEL, u32k cdfId, u32k nLoadingFlags);
	u32                    CompatibilityTest(CDefaultSkeleton* pDefaultSkeleton, CSkin* pCSkin);
	CDefaultSkeleton*         CheckIfModelSKELLoaded(const string& strFileName, u32 nLoadingFlags);
	CDefaultSkeleton*         CreateExtendedSkel(CCharInstance* pCharInstance, CDefaultSkeleton* pDefaultSkeleton, uint64 nExtendedCRC64, const std::vector<tukk >& mismatchingSkins, u32k nLoadingFlags);
#ifdef EDITOR_PCDEBUGCODE
	virtual void              ClearAllKeepInMemFlags();
#endif

	CSkin*            CheckIfModelSKINLoaded(const string& strFileName, u32 nLoadingFlags);
	CDefaultSkeleton* CheckIfModelExtSKELCreated(const uint64 nCRC64, u32 nLoadingFlags);
	void              UpdateStreaming_SKEL(std::vector<CDefaultSkeletonReferences>& skels, u32 nRenderFrameId, u32k* nRoundIds);
	void              UpdateStreaming_SKIN(std::vector<CDefaultSkinningReferences>& skins, u32 nRenderFrameId, u32k* nRoundIds);

	virtual void      ExtendDefaultSkeletonWithSkinAttachments(ICharacterInstance* pCharInstance, tukk szFilepathSKEL, tukk* szSkinAttachments, u32k skinsCount, u32k nLoadingFlags);
	virtual bool      LoadAndLockResources(tukk szFilePath, u32 nLoadingFlags);
	virtual void      StreamKeepCharacterResourcesResident(tukk szFilePath, i32 nLod, bool bKeep, bool bUrgent = false);
	virtual bool      StreamHasCharacterResources(tukk szFilePath, i32 nLod);
	virtual void      GetLoadedModels(IDefaultSkeleton** pIDefaultSkeleton, u32& nCount) const;
	virtual void      ReloadAllModels();
	virtual void      ReloadAllCHRPARAMS();
	virtual void      PreloadLevelModels();

	void              TryLoadModelSkin(tukk szFilePath, u32 nLoadingFlags, bool bKeep);
	void              GetModelCacheSize() const;
	void              DebugModelCache(u32 printtxt, std::vector<CDefaultSkeletonReferences>& parrModelCache, std::vector<CDefaultSkinningReferences>& parrModelCacheSKIN);
	virtual void      ClearResources(bool bForceCleanup);      //! Cleans up all resources - currently deletes all bodies and characters (even if there are references on them)
	void              CleanupModelCache(bool bForceCleanup);   // deletes all registered bodies; the character instances got deleted even if they're still referenced
	void              GetStatistics(Statistics& rStats) const; // returns statistics about this instance of character animation manager don't call this too frequently
	void              GetMemoryUsage(IDrxSizer* pSizer) const; //puts the size of the whole subsystem into this sizer object, classified, according to the flags set in the sizer
	void              TrackMemoryOfModels();
	std::vector<CDefaultSkeletonReferences> m_arrModelCacheSKEL;
	std::vector<CDefaultSkinningReferences> m_arrModelCacheSKIN;

#if BLENDSPACE_VISUALIZATION
	void CreateDebugInstances(tukk szCharacterFileName);
	void DeleteDebugInstances();
	void RenderDebugInstances(const SRenderingPassInfo& passInfo);
	void RenderBlendSpace(const SRenderingPassInfo& passInfo, ICharacterInstance* pCharacterInstance, float fCharacterScale, u32 flags);
	bool HasAnyDebugInstancesCreated() const;
	bool HasDebugInstancesCreated(tukk szCharacterFileName) const;
	DynArray<DebugInstances> m_arrCharacterBase;
#endif
	virtual void GetMotionParameterDetails(SMotionParameterDetails& outDetails, EMotionParamID paramId) const;
#ifdef EDITOR_PCDEBUGCODE
	virtual bool InjectCDF(tukk pathname, tukk content, size_t contentLength);
	virtual void ClearCDFCache() { m_arrCacheForCDF.clear(); }  //deactivate the cache in Editor-Mode, or we can't load the same CDF after we changed & saved it
	virtual void InjectCHRPARAMS(tukk pathname, tukk content, size_t contentLength);
	virtual void ClearCHRPARAMSCache();
	virtual void InjectBSPACE(tukk pathname, tukk content, size_t contentLength);
	virtual void ClearBSPACECache();
	std::vector<CDefaultSkeletonReferences> m_arrModelCacheSKEL_CharEdit;
	std::vector<CDefaultSkinningReferences> m_arrModelCacheSKIN_CharEdit;
#endif

	/**
	 * Stop animations on all instances using the specified animation set.
	 * Stop animations on all instances if nullptr is passed.
	 */
	void InternalStopAnimationsOnAllInstances(const IAnimationSet* pAnimationSet);

	CAnimationUpr                          m_AnimationUpr;

	CFacialAnimation*                          m_pFacialAnimation;

	CParamLoader                               m_ParamLoader;

	std::vector<f32>                           m_arrFrameTimes;

	std::vector<CharacterDefinition>           m_arrCacheForCDF;

	CharacterInstanceProcessing::CContextQueue m_ContextSyncQueue;  // queue to remember all animations which were started and need a sync

	uint64 m_nFrameTicks;                 // number of ticks spend in animations function during the last frame
	uint64 m_nFrameSyncTicks;             // number of ticks spend in animations sync function during the last frame
	u32 m_nActiveCharactersLastFrame;  // number of characters for which ForwardKinematics was called

	u32 m_nStreamUpdateRoundId[MAX_STREAM_PREDICTION_ZONES];

	u32 m_lastDatabaseUnloadTimeStamp;

	// geometry data cache for VCloth
	SClothGeometry* LoadVClothGeometry(const CAttachmentVCLOTH& pRendAtt, _smart_ptr<IRenderMesh> pRenderMeshes[]);
	typedef std::map<uint64, SClothGeometry> TClothGeomCache;
	TClothGeomCache m_clothGeometries;
};
