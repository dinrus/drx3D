// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   3dengine.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio.NET
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef C3DENGINE_H
#define C3DENGINE_H

	#pragma once

#include <drx3D/CoreX/Thread/DrxThreadSafeRendererContainer.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/Eng3D/VisibleRenderNodeUpr.h>
#include <drx3D/Eng3D/LightVolumeUpr.h>

#ifdef DrawText
	#undef DrawText
#endif //DrawText

// forward declaration
struct SNodeInfo;
class CStitchedImage;
class CWaterRippleUpr;

struct SEntInFoliage
{
	i32   id;
	float timeIdle;

	void  GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
};

class CMemoryBlock : public IMemoryBlock
{
public:
	virtual uk GetData() { return m_pData; }
	virtual i32   GetSize() { return m_nSize; }
	virtual ~CMemoryBlock() { delete[] m_pData; }

	CMemoryBlock() { m_pData = 0; m_nSize = 0; }
	CMemoryBlock(ukk pData, i32 nSize)
	{
		m_pData = 0;
		m_nSize = 0;
		SetData(pData, nSize);
	}
	void SetData(ukk pData, i32 nSize)
	{
		delete[] m_pData;
		m_pData = new u8[nSize];
		memcpy(m_pData, pData, nSize);
		m_nSize = nSize;
	}
	void Free()
	{
		delete[] m_pData;
		m_pData = NULL;
		m_nSize = 0;
	}
	void Allocate(i32 nSize)
	{
		delete[] m_pData;
		m_pData = new u8[nSize];
		memset(m_pData, 0, nSize);
		m_nSize = nSize;
	}

	static CMemoryBlock* CompressToMemBlock(uk pData, i32 nSize, ISystem* pSystem)
	{
		CMemoryBlock* pMemBlock = NULL;
		u8* pTmp = new u8[nSize + 4];
		size_t nComprSize = nSize;
		*(u32*)pTmp = nSize;
		if (pSystem->CompressDataBlock(pData, nSize, pTmp + 4, nComprSize))
		{
			pMemBlock = new CMemoryBlock(pTmp, nComprSize + 4);
		}

		delete[] pTmp;
		return pMemBlock;
	}

	static CMemoryBlock* DecompressFromMemBlock(CMemoryBlock* pMemBlock, ISystem* pSystem)
	{
		size_t nUncompSize = *(u32*)pMemBlock->GetData();
		SwapEndian(nUncompSize);
		CMemoryBlock* pResult = new CMemoryBlock;
		pResult->Allocate(nUncompSize);
		if (!pSystem->DecompressDataBlock((byte*)pMemBlock->GetData() + 4, pMemBlock->GetSize() - 4, pResult->GetData(), nUncompSize))
		{
			assert(!"CMemoryBlock::DecompressFromMemBlock failed");
			delete pResult;
			pResult = NULL;
		}

		return pResult;
	}

	u8* m_pData;
	i32    m_nSize;
};

struct SOptimizedOutdoorWindArea
{
	i32              x0, x1, y0, y1; // 2d rectangle extents for the wind area
	Vec2             point[5];       // Extent points of the wind area (index 4 is center)
	Vec3             windSpeed[5];   // Wind speed at every corner and center.
	float            z;
	IPhysicalEntity* pArea;// Physical area
};

struct DLightAmount
{
	SRenderLight* pLight;
	float         fAmount;
};

struct SImageSubInfo
{
	SImageSubInfo() { memset(this, 0, sizeof(*this)); fTiling = fTilingIn = 1.f; }

	static i32k nMipsNum = 4;

	union
	{
		byte* pImgMips[nMipsNum];
		i32   pImgMipsSizeKeeper[8];
	};

	float fAmount;
	i32   nReady;
	i32   nDummy[4];

	union
	{
		IMaterial* pMat;
		i32        pMatSizeKeeper[2];
	};

	i32   nDim;
	float fTilingIn;
	float fTiling;
	float fSpecularAmount;
	i32   nSortOrder;
	i32   nAlignFix;

	AUTO_STRUCT_INFO;
};

struct SImageInfo : public DinrusX3dEngBase
{
	SImageInfo()
	{
		szDetMatName[0] = szBaseTexName[0] = nPhysSurfaceType = 0;
		nLayerId = 0;
		fUseRemeshing = 0;
		fBr = 1.f;
		layerFilterColor = Col_White;
		nDetailSurfTypeId = 0;
		ZeroStruct(arrTextureId);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }

	SImageSubInfo baseInfo;
	SImageSubInfo detailInfo;

	char          szDetMatName[128 - 20];

	i32           arrTextureId[4];
	i32           nPhysSurfaceType;

	char          szBaseTexName[128];

	float         fUseRemeshing;
	ColorF        layerFilterColor;
	i32           nLayerId;
	float         fBr;
	i32           nDetailSurfTypeId;

	i32 GetMemoryUsage();

	AUTO_STRUCT_INFO;
};

struct SSceneFrustum
{
	u32*      pRgbImage;
	u32       nRgbWidth, nRgbHeight;

	float*       pDepthImage;
	u32       nDepthWidth, nDepthHeight;

	CCamera      camera;

	IRenderMesh* pRM;
	IMaterial*   pMaterial;

	float        fDistance;
	i32          nId;

	static i32   Compare(ukk v1, ukk v2)
	{
		SSceneFrustum* p[2] = { (SSceneFrustum*)v1, (SSceneFrustum*)v2 };

		if (p[0]->fDistance > p[1]->fDistance)
			return 1;
		if (p[0]->fDistance < p[1]->fDistance)
			return -1;

		if (p[0]->nId > p[1]->nId)
			return 1;
		if (p[0]->nId < p[1]->nId)
			return -1;

		return 0;
	}
};

struct SPerObjectShadow
{
	IShadowCaster* pCaster;
	float          fConstBias;
	float          fSlopeBias;
	float          fJitter;
	Vec3           vBBoxScale;
	uint           nTexSize;
};
//////////////////////////////////////////////////////////////////////

// onscreen infodebug for e_debugDraw >= 100
#ifndef _RELEASE
class CDebugDrawListMgr
{
	typedef DrxFixedStringT<64>  TMyStandardString;
	typedef DrxFixedStringT<128> TFilenameString;

public:

	CDebugDrawListMgr();
	void        Update();
	void        AddObject(I3DEngine::SObjectInfoToAddToDebugDrawList& objInfo);
	void        DumpLog();
	bool        IsEnabled() const { return DinrusX3dEngBase::GetCVars()->e_DebugDraw >= LM_BASENUMBER; }
	static void ConsoleCommand(IConsoleCmdArgs* args);

private:

	enum { UNDEFINED_ASSET_ID = 0xffffffff };

	struct TAssetInfo
	{
		TMyStandardString                   name;
		TFilenameString                     fileName;
		u32                              numTris;
		u32                              numVerts;
		u32                              texMemory;
		u32                              meshMemory;
		u32                              drawCalls;
		u32                              numInstances;
		I3DEngine::EDebugDrawListAssetTypes type;
		u32                              ID; // to identify which drawBoxes belong to this asset

		TAssetInfo(const I3DEngine::SObjectInfoToAddToDebugDrawList& objInfo);
		bool operator<(const TAssetInfo& other) const;
	};

	struct TObjectDrawBoxInfo
	{
		Matrix34 mat;
		AABB     bbox;
		u32   assetID;

		TObjectDrawBoxInfo(const I3DEngine::SObjectInfoToAddToDebugDrawList& objInfo);
	};

	void        FindNewLeastValueAsset();
	void        ClearFrameData();
	void        ClearConsoleCommandRequestVars();
	static bool SortComparison(const TAssetInfo& A, const TAssetInfo& B) { return B < A; }
	tukk GetStrCurrMode();
	void        GetStrCurrFilter(TMyStandardString& strOut);
	bool        ShouldFilterOutObject(const I3DEngine::SObjectInfoToAddToDebugDrawList& newObject);
	void        MemToString(u32 memVal, TMyStandardString& outStr);
	static void PrintText(float x, float y, const ColorF& fColor, tukk label_text, ...);
	tukk GetAssetTypeName(I3DEngine::EDebugDrawListAssetTypes type);
	TAssetInfo* FindDuplicate(const TAssetInfo& object);
	void        CheckFilterCVar();

	// to avoid any heap allocation
	static void                   MyStandardString_Concatenate(TMyStandardString& outStr, tukk str);
	static void                   MyFileNameString_Assign(TFilenameString& outStr, tukk pStr);

	template<class T> static void MyString_Assign(T& outStr, tukk pStr)
	{
		if (pStr)
			outStr._Assign(pStr, min(strlen(pStr), outStr.capacity()));
		else
			outStr = "";
	}

	enum EListModes
	{
		LM_BASENUMBER = 100,
		LM_TRI_COUNT  = LM_BASENUMBER,
		LM_VERT_COUNT,
		LM_DRAWCALLS,
		LM_TEXTMEM,
		LM_MESHMEM
	};

	bool                            m_isFrozen;
	u32                          m_counter;
	u32                          m_assetCounter;
	u32                          m_indexLeastValueAsset;
	std::vector<TAssetInfo>         m_assets;
	std::vector<TObjectDrawBoxInfo> m_drawBoxes;
	DrxCriticalSection              m_lock;

	static bool                     m_dumpLogRequested;
	static bool                     m_freezeRequested;
	static bool                     m_unfreezeRequested;
	static u32                   m_filter;
};
#endif //_RELEASE

//////////////////////////////////////////////////////////////////////
class C3DEngine : public I3DEngine, public DinrusX3dEngBase
{
	// IProcess Implementation
	void SetFlags(i32 flags) { m_nFlags = flags; }
	i32  GetFlags(void)      { return m_nFlags; }
	i32 m_nFlags;

public:

	// I3DEngine interface implementation
	virtual bool      Init();
	virtual void      OnFrameStart();
	virtual void      Update();
	virtual void      RenderWorld(i32k nRenderFlags, const SRenderingPassInfo& passInfo, tukk szDebugName);
	virtual void      PreWorldStreamUpdate(const CCamera& cam);
	virtual void      WorldStreamUpdate();
	virtual void      ShutDown();
	virtual void      Release() { DrxAlignedDelete(this); };
	virtual void      SetLevelPath(tukk szFolderName);
	virtual bool      LoadLevel(tukk szFolderName, tukk szMissionName);
	virtual void      UnloadLevel();
	virtual void      PostLoadLevel();
	virtual bool      InitLevelForEditor(tukk szFolderName, tukk szMissionName);
	virtual void      DisplayInfo(float& fTextPosX, float& fTextPosY, float& fTextStepY, const bool bEnhanced);
	virtual IStatObj* LoadStatObj(tukk szFileName, tukk szGeomName = NULL, /*[Out]*/ IStatObj::SSubObject** ppSubObject = NULL, bool bUseStreaming = true, u64 nLoadingFlags = 0);
	virtual IStatObj* FindStatObjectByFilename(tukk filename);
	virtual void      RegisterEntity(IRenderNode* pEnt);
	virtual void      SelectEntity(IRenderNode* pEnt);

#ifndef _RELEASE
	virtual void AddObjToDebugDrawList(SObjectInfoToAddToDebugDrawList& objInfo);
	virtual bool IsDebugDrawListEnabled() const { return m_DebugDrawListMgr.IsEnabled(); }
#endif

	virtual void   UnRegisterEntityDirect(IRenderNode* pEnt);
	virtual void   UnRegisterEntityAsJob(IRenderNode* pEnt);

	virtual void   AddWaterRipple(const Vec3& vPos, float scale, float strength);

	virtual bool   IsUnderWater(const Vec3& vPos) const;
	virtual void   SetOceanRenderFlags(u8 nFlags);
	virtual u8  GetOceanRenderFlags() const { return m_nOceanRenderFlags; }
	virtual u32 GetOceanVisiblePixelsCount() const;
	virtual float  GetBottomLevel(const Vec3& referencePos, float maxRelevantDepth, i32 objtypes);
	virtual float  GetBottomLevel(const Vec3& referencePos, float maxRelevantDepth /* = 10.0f*/);
	virtual float  GetBottomLevel(const Vec3& referencePos, i32 objflags);

#if defined(USE_GEOM_CACHES)
	virtual IGeomCache* LoadGeomCache(tukk szFileName);
	virtual IGeomCache* FindGeomCacheByFilename(tukk szFileName);
#endif

	virtual IStatObj* LoadDesignerObject(i32 nVersion, tukk szBinaryStream, i32 size);

	void              AsyncOctreeUpdate(IRenderNode* pEnt, u32 nFrameID, bool bUnRegisterOnly);
	bool              UnRegisterEntityImpl(IRenderNode* pEnt);
	virtual void      UpdateObjectsLayerAABB(IRenderNode* pEnt);

	// Fast option - use if just ocean height required
	virtual float                    GetWaterLevel();
	// This will return ocean height or water volume height, optional for accurate water height query
	virtual float                    GetWaterLevel(const Vec3* pvPos, IPhysicalEntity* pent = NULL, bool bAccurate = false);
	// Only use for Accurate query - this will return exact ocean height
	virtual float                    GetAccurateOceanHeight(const Vec3& pCurrPos) const;

	virtual Vec4                     GetCausticsParams() const;
	virtual Vec4                     GetOceanAnimationCausticsParams() const;
	virtual void                     GetOceanAnimationParams(Vec4& pParams0, Vec4& pParams1) const;
	virtual void                     GetHDRSetupParams(Vec4 pParams[5]) const;
	virtual void                     CreateDecal(const DinrusXDecalInfo& Decal);
	virtual void                     DrawFarTrees(const SRenderingPassInfo& passInfo);
	virtual void                     GenerateFarTrees(const SRenderingPassInfo& passInfo);
	virtual float                    GetTerrainElevation(float x, float y);
	virtual float                    GetTerrainElevation3D(Vec3 vPos);
	virtual float                    GetTerrainZ(float x, float y);
	virtual bool                     GetTerrainHole(float x, float y);
	virtual float                    GetHeightMapUnitSize();
	virtual i32                      GetTerrainSize();
	virtual void                     SetSunDir(const Vec3& newSunOffset);
	virtual Vec3                     GetSunDir() const;
	virtual Vec3                     GetSunDirNormalized() const;
	virtual Vec3                     GetRealtimeSunDirNormalized() const;
	virtual void                     SetSkyColor(Vec3 vColor);
	virtual void                     SetSunColor(Vec3 vColor);
	virtual void                     SetSkyBrightness(float fMul);
	virtual void                     SetSSAOAmount(float fMul);
	virtual void                     SetSSAOContrast(float fMul);
	virtual void                     SetGIAmount(float fMul);
	virtual float                    GetSunRel() const;
	virtual void                     SetRainParams(const SRainParams& rainParams);
	virtual bool                     GetRainParams(SRainParams& rainParams);
	virtual void                     SetSnowSurfaceParams(const Vec3& vCenter, float fRadius, float fSnowAmount, float fFrostAmount, float fSurfaceFreezing);
	virtual bool                     GetSnowSurfaceParams(Vec3& vCenter, float& fRadius, float& fSnowAmount, float& fFrostAmount, float& fSurfaceFreezing);
	virtual void                     SetSnowFallParams(i32 nSnowFlakeCount, float fSnowFlakeSize, float fSnowFallBrightness, float fSnowFallGravityScale, float fSnowFallWindScale, float fSnowFallTurbulence, float fSnowFallTurbulenceFreq);
	virtual bool                     GetSnowFallParams(i32& nSnowFlakeCount, float& fSnowFlakeSize, float& fSnowFallBrightness, float& fSnowFallGravityScale, float& fSnowFallWindScale, float& fSnowFallTurbulence, float& fSnowFallTurbulenceFreq);
	virtual void                     OnExplosion(Vec3 vPos, float fRadius, bool bDeformTerrain = true);
	//! For editor
	virtual void                     RemoveAllStaticObjects();
	virtual void                     SetTerrainSurfaceType(i32 x, i32 y, i32 nType);
	virtual void                     SetTerrainSectorTexture(i32k nTexSectorX, i32k nTexSectorY, u32 textureId);
	virtual void                     SetPhysMaterialEnumerator(IPhysMaterialEnumerator* pPhysMaterialEnumerator);
	virtual IPhysMaterialEnumerator* GetPhysMaterialEnumerator();
	virtual void                     LoadMissionDataFromXMLNode(tukk szMissionName);

	void                             AddDynamicLightSource(const SRenderLight& LSource, ILightSource* pEnt, i32 nEntityLightId, float fFadeout, const SRenderingPassInfo& passInfo);

	inline void                      AddLightToRenderer(const SRenderLight& light, float fMult, const SRenderingPassInfo& passInfo)
	{
		u32k nLightID = passInfo.GetIRenderView()->GetLightsCount(eDLT_DeferredLight);
		//passInfo.GetIRenderView()->AddLight(eDLT_DeferredLight,light);
		GetRenderer()->EF_AddDeferredLight(light, fMult, passInfo);
		Get3DEngine()->m_LightVolumesMgr.RegisterLight(light, nLightID, passInfo);
		m_nDeferredLightsNum++;
	}

	virtual void                 ApplyForceToEnvironment(Vec3 vPos, float fRadius, float fAmountOfForce);
	virtual void                 SetMaxViewDistanceScale(float fScale) { m_fMaxViewDistScale = fScale; }
	virtual float                GetMaxViewDistance(bool bScaled = true);
	virtual const SFrameLodInfo& GetFrameLodInfo() const               { return m_frameLodInfo; }
	virtual void                 SetFrameLodInfo(const SFrameLodInfo& frameLodInfo);
	virtual void                 SetFogColor(const Vec3& vFogColor);
	virtual Vec3                 GetFogColor();
	virtual float                GetDistanceToSectorWithWater();

	virtual void                 GetSkyLightParameters(Vec3& sunDir, Vec3& sunIntensity, float& Km, float& Kr, float& g, Vec3& rgbWaveLengths);
	virtual void                 SetSkyLightParameters(const Vec3& sunDir, const Vec3& sunIntensity, float Km, float Kr, float g, const Vec3& rgbWaveLengths, bool forceImmediateUpdate = false);

	void                         SetLightsHDRDynamicPowerFactor(const float value);
	virtual float                GetLightsHDRDynamicPowerFactor() const;

	// Return true if tessellation is allowed (by cvars) into currently set shadow map LOD
	bool                                   IsTessellationAllowedForShadowMap(const SRenderingPassInfo& passInfo) const;
	// Return true if tessellation is allowed for given render object
	virtual bool                           IsTessellationAllowed(const CRenderObject* pObj, const SRenderingPassInfo& passInfo, bool bIgnoreShadowPass = false) const;

	virtual bool                           IsStatObjBufferRenderTasksAllowed() const;

	virtual void                           SetRenderNodeMaterialAtPosition(EERType eNodeType, const Vec3& vPos, IMaterial* pMat);
	virtual void                           OverrideCameraPrecachePoint(const Vec3& vPos);
	virtual i32                            AddPrecachePoint(const Vec3& vPos, const Vec3& vDir, float fTimeOut = 3.f, float fImportanceFactor = 1.0f);
	virtual void                           ClearPrecachePoint(i32 id);
	virtual void                           ClearAllPrecachePoints();
	virtual void                           GetPrecacheRoundIds(i32 pRoundIds[MAX_STREAM_PREDICTION_ZONES]);

	virtual void                           TraceFogVolumes(const Vec3& worldPos, ColorF& fogVolumeContrib, const SRenderingPassInfo& passInfo);

	virtual Vec3                           GetSkyColor() const;
	virtual Vec3                           GetSunColor() const;
	virtual float                          GetSkyBrightness() const;
	virtual float                          GetSSAOAmount() const;
	virtual float                          GetSSAOContrast() const;
	virtual float                          GetGIAmount() const;
	virtual float                          GetTerrainTextureMultiplier() const;

	virtual Vec3                           GetAmbientColorFromPosition(const Vec3& vPos, float fRadius = 1.f);
	virtual void                           FreeRenderNodeState(IRenderNode* pEnt);
	virtual tukk                    GetLevelFilePath(tukk szFileName);
	virtual void                           SetTerrainBurnedOut(i32 x, i32 y, bool bBurnedOut);
	virtual bool                           IsTerrainBurnedOut(i32 x, i32 y);
	virtual i32                            GetTerrainSectorSize();
	virtual void                           LoadTerrainSurfacesFromXML(XmlNodeRef pDoc, bool bUpdateTerrain);
	virtual bool                           SetStatInstGroup(i32 nGroupId, const IStatInstGroup& siGroup);
	virtual bool                           GetStatInstGroup(i32 nGroupId, IStatInstGroup& siGroup);
	virtual void                           ActivatePortal(const Vec3& vPos, bool bActivate, tukk szEntityName);
	virtual void                           ActivateOcclusionAreas(IVisAreaTestCallback* pTest, bool bActivate);
	virtual void                           GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void                           GetResourceMemoryUsage(IDrxSizer* pSizer, const AABB& cstAABB);
	virtual IVisArea*                      CreateVisArea(uint64 visGUID);
	virtual void                           DeleteVisArea(IVisArea* pVisArea);
	virtual void                           UpdateVisArea(IVisArea* pArea, const Vec3* pPoints, i32 nCount, tukk szName,
	                                                     const SVisAreaInfo& info, bool bReregisterObjects);
	virtual IClipVolume*                   CreateClipVolume();
	virtual void                           DeleteClipVolume(IClipVolume* pClipVolume);
	virtual void                           UpdateClipVolume(IClipVolume* pClipVolume, _smart_ptr<IRenderMesh> pRenderMesh, IBSPTree3D* pBspTree, const Matrix34& worldTM, u8 viewDistRatio, bool bActive, u32 flags, tukk szName);
	virtual void                           ResetParticlesAndDecals();
	virtual IRenderNode*                   CreateRenderNode(EERType type);
	virtual void                           DeleteRenderNode(IRenderNode* pRenderNode);
	void                                   TickDelayedRenderNodeDeletion();
	virtual void                           SetWind(const Vec3& vWind);
	virtual Vec3                           GetWind(const AABB& box, bool bIndoors) const;
	virtual void                           AddForcedWindArea(const Vec3& vPos, float fAmountOfForce, float fRadius);

	void                                   StartWindGridJob(const Vec3& vPos);
	void                                   FinishWindGridJob();
	void                                   UpdateWindGridJobEntry(Vec3 vPos);
	void                                   UpdateWindGridArea(SWindGrid& rWindGrid, const SOptimizedOutdoorWindArea& windArea, const AABB& windBox);
	void                                   RasterWindAreas(std::vector<SOptimizedOutdoorWindArea>* pWindAreas, const Vec3& vGlobalWind);

	virtual Vec3                           GetGlobalWind(bool bIndoors) const;
	virtual bool                           SampleWind(Vec3* pSamples, i32 nSamples, const AABB& volume, bool bIndoors) const;
	virtual IVisArea*                      GetVisAreaFromPos(const Vec3& vPos);
	virtual bool                           IntersectsVisAreas(const AABB& box, uk * pNodeCache = 0);
	virtual bool                           ClipToVisAreas(IVisArea* pInside, Sphere& sphere, Vec3 const& vNormal, uk pNodeCache = 0);
	virtual bool                           IsVisAreasConnected(IVisArea* pArea1, IVisArea* pArea2, i32 nMaxReqursion, bool bSkipDisabledPortals);
	void                                   EnableOceanRendering(bool bOcean); // todo: remove

	virtual struct ILightSource*           CreateLightSource();
	virtual void                           DeleteLightSource(ILightSource* pLightSource);
	virtual const PodArray<SRenderLight*>* GetStaticLightSources();
	virtual bool                           IsTerrainHightMapModifiedByGame();
	virtual bool                           RestoreTerrainFromDisk();
	virtual void                           CheckMemoryHeap();
	virtual void                           CloseTerrainTextureFile();
	virtual i32                            GetLoadedObjectCount();
	virtual void                           GetLoadedStatObjArray(IStatObj** pObjectsArray, i32& nCount);
	virtual void                           GetObjectsStreamingStatus(SObjectsStreamingStatus& outStatus);
	virtual void                           GetStreamingSubsystemData(i32 subsystem, SStremaingBandwidthData& outData);
	virtual void                           DeleteEntityDecals(IRenderNode* pEntity);
	virtual void                           DeleteDecalsInRange(AABB* pAreaBox, IRenderNode* pEntity);
	virtual void                           CompleteObjectsGeometry();
	virtual void                           LockCGFResources();
	virtual void                           UnlockCGFResources();

	virtual void                           SerializeState(TSerialize ser);
	virtual void                           PostSerialize(bool bReading);

	virtual void                           SetHeightMapMaxHeight(float fMaxHeight);

	virtual void                           SetStreamableListener(IStreamedObjectListener* pListener);

	//////////////////////////////////////////////////////////////////////////
	// Materials access.
	virtual IMaterialHelpers& GetMaterialHelpers();
	virtual IMaterialUpr* GetMaterialUpr();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// CGF Loader.
	//////////////////////////////////////////////////////////////////////////
	virtual CContentCGF* CreateChunkfileContent(tukk filename);
	virtual void         ReleaseChunkfileContent(CContentCGF*);
	virtual bool         LoadChunkFileContent(CContentCGF* pCGF, tukk filename, bool bNoWarningMode = false, bool bCopyChunkFile = true);
	virtual bool         LoadChunkFileContentFromMem(CContentCGF* pCGF, ukk pData, size_t nDataLen, u32 nLoadingFlags, bool bNoWarningMode = false, bool bCopyChunkFile = true);
	//////////////////////////////////////////////////////////////////////////
	virtual IChunkFile*  CreateChunkFile(bool bReadOnly = false);

	//////////////////////////////////////////////////////////////////////////
	// Chunk file writer.
	//////////////////////////////////////////////////////////////////////////
	virtual ChunkFile::IChunkFileWriter* CreateChunkFileWriter(EChunkFileFormat eFormat, IDrxPak* pPak, tukk filename) const;
	virtual void                         ReleaseChunkFileWriter(ChunkFile::IChunkFileWriter* p) const;

	//////////////////////////////////////////////////////////////////////////
	// Post processing effects interfaces

	virtual void   SetPostEffectParam(tukk pParam, float fValue, bool bForceValue = false) const;
	virtual void   SetPostEffectParamVec4(tukk pParam, const Vec4& pValue, bool bForceValue = false) const;
	virtual void   SetPostEffectParamString(tukk pParam, tukk pszArg) const;

	virtual void   GetPostEffectParam(tukk pParam, float& fValue) const;
	virtual void   GetPostEffectParamVec4(tukk pParam, Vec4& pValue) const;
	virtual void   GetPostEffectParamString(tukk pParam, tukk & pszArg) const;

	virtual i32  GetPostEffectID(tukk pPostEffectName);

	virtual void   ResetPostEffects(bool bOnSpecChange = false) const;

	virtual void   SetShadowsGSMCache(bool bCache);
	virtual void   SetCachedShadowBounds(const AABB& shadowBounds, float fAdditionalCascadesScale);
	virtual void   SetRecomputeCachedShadows(uint nUpdateStrategy = 0);
	virtual void   InvalidateShadowCacheData();
	void           SetShadowsCascadesBias(const float* pCascadeConstBias, const float* pCascadeSlopeBias);
	const float*   GetShadowsCascadesConstBias() const { return m_pShadowCascadeConstBias; }
	const float*   GetShadowsCascadesSlopeBias() const { return m_pShadowCascadeSlopeBias; }
	i32            GetShadowsCascadeCount(const SRenderLight* pLight) const;

	virtual u32 GetObjectsByType(EERType objType, IRenderNode** pObjects);
	virtual u32 GetObjectsByTypeInBox(EERType objType, const AABB& bbox, IRenderNode** pObjects, uint64 dwFlags = ~0);
	virtual u32 GetObjectsInBox(const AABB& bbox, IRenderNode** pObjects = 0);
	virtual u32 GetObjectsByFlags(uint dwFlags, IRenderNode** pObjects = 0);
	virtual void   OnObjectModified(IRenderNode* pRenderNode, IRenderNode::RenderFlagsType dwFlags);

	virtual void   ActivateObjectsLayer(u16 nLayerId, bool bActivate, bool bPhys, bool bObjects, bool bStaticLights, tukk pLayerName, IGeneralMemoryHeap* pHeap = NULL, bool bCheckLayerActivation = true);
	bool           IsObjectsLayerHidden(u16 nLayerId, const AABB& objBox);
	virtual void   GetLayerMemoryUsage(u16 nLayerId, IDrxSizer* pSizer, i32* pNumBrushes, i32* pNumDecals) const;
	virtual void   SkipLayerLoading(u16 nLayerId, bool bClearList);
	bool           IsLayerSkipped(u16 nLayerId);

	//////////////////////////////////////////////////////////////////////////

	virtual i32  GetTerrainTextureNodeSizeMeters();
	virtual i32  GetTerrainTextureNodeSizePixels(i32 nLayer);

	virtual void SetTerrainLayerBaseTextureData(i32 nLayerId, byte* pImage, i32 nDim, tukk nImgFileName, IMaterial* pMat, float fBr, float fTiling, i32 nDetailSurfTypeId, float fTilingDetail, float fSpecularAmount, float fSortOrder, ColorF layerFilterColor, float fUseRemeshing, bool bShowSelection);
	SImageInfo*  GetBaseTextureData(i32 nLayerId);
	SImageInfo*  GetBaseTextureDataFromSurfType(i32 nSurfTypeId);

	tukk  GetLevelFolder() { return m_szLevelFolder; }

	bool         SaveCGF(std::vector<IStatObj*>& pObjs);

	virtual bool IsAreaActivationInUse() { return m_bAreaActivationInUse && GetCVars()->e_ObjectLayersActivation; }

	i32          GetCurrentLightSpec()
	{
		return CONFIG_VERYHIGH_SPEC; // very high spec.
	}

	void         UpdateRenderingCamera(tukk szCallerName, const SRenderingPassInfo& passInfo);
	virtual void PrepareOcclusion(const CCamera& rCamera);
	virtual void EndOcclusion();
#ifndef _RELEASE
	void         ProcessStreamingLatencyTest(const CCamera& camIn, CCamera& camOut, const SRenderingPassInfo& passInfo);
#endif

	void ScreenShotHighRes(CStitchedImage* pStitchedImage, i32k nRenderFlags, const SRenderingPassInfo& passInfo, u32 SliceCount, f32 fTransitionSize);

	// cylindrical mapping made by multiple slices rendered and distorted
	// Returns:
	//   true=mode is active, stop normal rendering, false=mode is not active
	bool         ScreenShotPanorama(CStitchedImage* pStitchedImage, i32k nRenderFlags, const SRenderingPassInfo& passInfo, u32 SliceCount, f32 fTransitionSize);
	// Render simple top-down screenshot for map overviews
	bool         ScreenShotMap(CStitchedImage* pStitchedImage, i32k nRenderFlags, const SRenderingPassInfo& passInfo, u32 SliceCount, f32 fTransitionSize);

	void         ScreenshotDispatcher(i32k nRenderFlags, const SRenderingPassInfo& passInfo);

	virtual void FillDebugFPSInfo(SDebugFPSInfo& info);

	void         ClearDebugFPSInfo(bool bUnload = false)
	{
		m_fAverageFPS = 0.0f;
		m_fMinFPS = m_fMinFPSDecay = 999.f;
		m_fMaxFPS = m_fMaxFPSDecay = 0.0f;
		if (bUnload)
			stl::free_container(arrFPSforSaveLevelStats);
		else
			arrFPSforSaveLevelStats.clear();
	}

	void ClearPrecacheInfo()
	{
		m_nFramesSinceLevelStart = 0;
		m_nStreamingFramesSinceLevelStart = 0;
		m_bPreCacheEndEventSent = false;
		m_fTimeStateStarted = 0.0f;
	}

	virtual const CCamera& GetRenderingCamera() const { return m_RenderingCamera; }
	virtual float          GetZoomFactor() const      { return m_fZoomFactor; }
	virtual float          IsZoomInProgress()  const  { return m_bZoomInProgress; }
	virtual void           Tick();

	virtual void           UpdateShaderItems();
	void                   GetCollisionClass(SCollisionClass& collclass, i32 tableIndex);

	void                   SetRecomputeCachedShadows(IRenderNode* pNode, uint updateStrategy);

public:
	C3DEngine(ISystem* pSystem);
	~C3DEngine();

	virtual void RenderScene(i32k nRenderFlags, const SRenderingPassInfo& passInfo);
	virtual void DebugDraw_UpdateDebugNode();

	void         DebugDraw_Draw();
	bool         IsOutdoorVisible();
	void         RenderSkyBox(IMaterial* pMat, const SRenderingPassInfo& passInfo);
	i32          GetStreamingFramesSinceLevelStart() { return m_nStreamingFramesSinceLevelStart; }
	i32          GetRenderFramesSinceLevelStart()    { return m_nFramesSinceLevelStart; }

	bool CreateDecalInstance(const DinrusXDecalInfo &DecalInfo, class CDecal * pCallerManagedDecal);
	//void CreateDecalOnCharacterComponents(ICharacterInstance * pChar, const struct DinrusXDecalInfo & decal);
	Vec3 GetTerrainSurfaceNormal(Vec3 vPos);
	void LoadEnvironmentSettingsFromXML(XmlNodeRef pInputNode);
#if defined(FEATURE_SVO_GI)
	void LoadTISettings(XmlNodeRef pInputNode);
#endif
	void LoadDefaultAssets();

	// access to components
	ILINE static CVars*            GetCVars()              { return m_pCVars; }
	ILINE CVisAreaUpr*         GetVisAreaUpr()     { return m_pVisAreaUpr; }
	ILINE CClipVolumeUpr*      GetClipVolumeUpr()  { return m_pClipVolumeUpr; }
	ILINE PodArray<ILightSource*>* GetLightEntities()      { return &m_lstStaticLights; }

	ILINE IGeneralMemoryHeap*      GetBreakableBrushHeap() { return m_pBreakableBrushHeap; }

	virtual void                   OnCameraTeleport();

	// this data is stored in memory for instances streaming
	std::vector<struct IStatObj*>* m_pLevelStatObjTable;
	std::vector<IMaterial*>*       m_pLevelMaterialsTable;
	bool                           m_bLevelFilesEndian;
	struct SLayerActivityInfo
	{
		SLayerActivityInfo() { objectsBox.Reset(); bActive = false; }
		AABB objectsBox;
		bool bActive;
	};
	PodArray<SLayerActivityInfo> m_arrObjectLayersActivity;
	uint                         m_objectLayersModificationId;
	bool                         m_bAreaActivationInUse;

	// Level info
	float m_fSkyBoxAngle,
	      m_fSkyBoxStretching;

	float                 m_fMaxViewDistScale;
	float                 m_fMaxViewDistHighSpec;
	float                 m_fMaxViewDistLowSpec;
	float                 m_fTerrainDetailMaterialsViewDistRatio;

	float                 m_volFogGlobalDensity;
	float                 m_volFogGlobalDensityMultiplierLDR;
	float                 m_volFogFinalDensityClamp;

	i32                   m_nCurWind;         // Current wind-field buffer Id
	SWindGrid             m_WindGrid[2];      // Wind field double-buffered
	Vec2*                 m_pWindField;       // Old wind speed values for interpolation
	i32*                  m_pWindAreaFrames;  // Area frames for rest updates
	Vec3                  m_vWindFieldCamera; // Wind field camera for interpolation
	JobUpr::SJobState m_WindJobState;
	bool                  m_bWindJobRun;

	float                 m_fCloudShadingSunLightMultiplier;
	float                 m_fCloudShadingSkyLightMultiplier;
	Vec3                  m_vCloudShadingCustomSunColor;
	Vec3                  m_vCloudShadingCustomSkyColor;

	Vec3                  m_vVolCloudAtmosphericScattering;
	Vec3                  m_vVolCloudGenParams;
	Vec3                  m_vVolCloudScatteringLow;
	Vec3                  m_vVolCloudScatteringHigh;
	Vec3                  m_vVolCloudGroundColor;
	Vec3                  m_vVolCloudScatteringMulti;
	Vec3                  m_vVolCloudWindAtmospheric;
	Vec3                  m_vVolCloudTurbulence;
	Vec3                  m_vVolCloudEnvParams;
	Vec3                  m_vVolCloudGlobalNoiseScale;
	Vec3                  m_vVolCloudRenderParams;
	Vec3                  m_vVolCloudTurbulenceNoiseScale;
	Vec3                  m_vVolCloudTurbulenceNoiseParams;
	Vec3                  m_vVolCloudDensityParams;
	Vec3                  m_vVolCloudMiscParams;
	Vec3                  m_vVolCloudTilingSize;
	Vec3                  m_vVolCloudTilingOffset;

	Vec3                  m_vFogColor;
	Vec3                  m_vDefFogColor;
	Vec3                  m_vSunDir;
	Vec3                  m_vSunDirNormalized;
	float                 m_fSunDirUpdateTime;
	Vec3                  m_vSunDirRealtime;
	Vec3                  m_vWindSpeed;

	Vec3                  m_volFogRamp;
	Vec3                  m_volFogShadowRange;
	Vec3                  m_volFogShadowDarkening;
	Vec3                  m_volFogShadowEnable;

	Vec3                  m_volFog2CtrlParams;
	Vec3                  m_volFog2ScatteringParams;
	Vec3                  m_volFog2Ramp;
	Vec3                  m_volFog2Color;
	Vec3                  m_volFog2GlobalDensity;
	Vec3                  m_volFog2HeightDensity;
	Vec3                  m_volFog2HeightDensity2;
	Vec3                  m_volFog2Color1;
	Vec3                  m_volFog2Color2;

	Vec3                  m_nightSkyHorizonCol;
	Vec3                  m_nightSkyZenithCol;
	float                 m_nightSkyZenithColShift;
	float                 m_nightSkyStarIntensity;
	Vec3                  m_nightMoonCol;
	float                 m_nightMoonSize;
	Vec3                  m_nightMoonInnerCoronaCol;
	float                 m_nightMoonInnerCoronaScale;
	Vec3                  m_nightMoonOuterCoronaCol;
	float                 m_nightMoonOuterCoronaScale;

	float                 m_moonRotationLatitude;
	float                 m_moonRotationLongitude;
	Vec3                  m_moonDirection;
	i32                   m_nWaterBottomTexId;
	i32                   m_nNightMoonTexId;
	bool                  m_bShowTerrainSurface;
	float                 m_fSunClipPlaneRange;
	float                 m_fSunClipPlaneRangeShift;
	bool                  m_bSunShadows;
	bool                  m_bSunShadowsFromTerrain;

	i32                   m_nCloudShadowTexId;

	float                 m_fGsmRange;
	float                 m_fGsmRangeStep;
	float                 m_fShadowsConstBias;
	float                 m_fShadowsSlopeBias;

	i32                   m_nSunAdditionalCascades;
	i32                   m_nGsmCache;
	Vec3                  m_oceanFogColor;
	Vec3                  m_oceanFogColorShallow;
	float                 m_oceanFogDensity;
	float                 m_skyboxMultiplier;
	float                 m_dayNightIndicator;
	bool                  m_bHeightMapAoEnabled;
	bool                  m_bIntegrateObjectsIntoTerrain;

	Vec3                  m_fogColor2;
	Vec3                  m_fogColorRadial;
	Vec3                  m_volFogHeightDensity;
	Vec3                  m_volFogHeightDensity2;
	Vec3                  m_volFogGradientCtrl;

	Vec3                  m_fogColorSkylightRayleighInScatter;

	float                 m_oceanCausticsDistanceAtten;
	float                 m_oceanCausticsMultiplier;
	float                 m_oceanCausticsDarkeningMultiplier;
	float                 m_oceanCausticsTilling;
	float                 m_oceanCausticHeight;
	float                 m_oceanCausticDepth;
	float                 m_oceanCausticIntensity;

	string                m_skyMatName;
	string                m_skyLowSpecMatName;

	float                 m_oceanWindDirection;
	float                 m_oceanWindSpeed;
	float                 m_oceanWavesAmount;
	float                 m_oceanWavesSize;

	float                 m_dawnStart;
	float                 m_dawnEnd;
	float                 m_duskStart;
	float                 m_duskEnd;

	// film characteristic curve tweakables
	Vec4  m_vHDRFilmCurveParams;
	Vec3  m_vHDREyeAdaptation;
	Vec3  m_vHDREyeAdaptationLegacy;
	float m_fHDRBloomAmount;

	// hdr color grading
	Vec3  m_vColorBalance;
	float m_fHDRSaturation;

#ifndef _RELEASE
	CDebugDrawListMgr m_DebugDrawListMgr;
#endif

#define MAX_SHADOW_CASCADES_NUM 20
	float m_pShadowCascadeConstBias[MAX_SHADOW_CASCADES_NUM];
	float m_pShadowCascadeSlopeBias[MAX_SHADOW_CASCADES_NUM];

	AABB  m_CachedShadowsBounds;
	uint  m_nCachedShadowsUpdateStrategy;
	float m_fCachedShadowsCascadeScale;

	// special case for combat mode adjustments
	float m_fSaturation;
	Vec4  m_pPhotoFilterColor;
	float m_fPhotoFilterColorDensity;
	float m_fGrainAmount;
	float m_fSunSpecMult;

	// Level shaders
	_smart_ptr<IMaterial> m_pTerrainWaterMat;
	_smart_ptr<IMaterial> m_pSkyMat;
	_smart_ptr<IMaterial> m_pSkyLowSpecMat;
	_smart_ptr<IMaterial> m_pSunMat;

	// Fog Materials
	_smart_ptr<IMaterial> m_pMatFogVolEllipsoid;
	_smart_ptr<IMaterial> m_pMatFogVolBox;

	void CleanLevelShaders()
	{
		m_pTerrainWaterMat = 0;
		m_pSkyMat = 0;
		m_pSkyLowSpecMat = 0;
		m_pSunMat = 0;

		m_pMatFogVolEllipsoid = 0;
		m_pMatFogVolBox = 0;
	}

	// Render elements
	CRESky*    m_pRESky;
	CREHDRSky* m_pREHDRSky;

	i32        m_nDeferredLightsNum;

private:
	// not sorted

	void  LoadTimeOfDaySettingsFromXML(XmlNodeRef node);
	tuk GetXMLAttribText(XmlNodeRef pInputNode, tukk szLevel1, tukk szLevel2, tukk szDefaultValue);
	tuk GetXMLAttribText(XmlNodeRef pInputNode, tukk szLevel1, tukk szLevel2, tukk szLevel3, tukk szDefaultValue);
	bool  GetXMLAttribBool(XmlNodeRef pInputNode, tukk szLevel1, tukk szLevel2, bool bDefaultValue);

	// without calling high level functions like panorama screenshot
	void RenderInternal(i32k nRenderFlags, const SRenderingPassInfo& passInfo, tukk szDebugName);

	void RegisterLightSourceInSectors(SRenderLight* pDynLight, const SRenderingPassInfo& passInfo);

	bool IsCameraAnd3DEngineInvalid(const SRenderingPassInfo& passInfo, tukk szCaller);

	void DebugDrawStreaming(const SRenderingPassInfo& passInfo);
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ResetCasterCombinationsCache();

	void FindPotentialLightSources(const SRenderingPassInfo& passInfo);
	void LoadParticleEffects(tukk szFolderName);

	void UpdateSunLightSource(const SRenderingPassInfo& passInfo);

	// Query physics for physical air affecting area and create an optimized query structure for 3d engine
	void UpdateWindAreas();

	void UpdateMoonDirection();

	// Copy objects from tree
	void CopyObjectsByType(EERType objType, const AABB* pBox, PodArray<IRenderNode*>* plistObjects, uint64 dwFlags = ~0);
	void CopyObjects(const AABB* pBox, PodArray<IRenderNode*>* plistObjects);

	void CleanUpOldDecals();
public:
	// functions SRenderingPass
	virtual CCamera* GetRenderingPassCamera(const CCamera& rCamera);
	virtual i32      GetZoomMode() const;
	virtual float    GetPrevZoomFactor();
	virtual void     SetZoomMode(i32 nZoomMode);
	virtual void     SetPrevZoomFactor(float fZoomFactor);

#if defined(FEATURE_SVO_GI)
	virtual bool GetSvoStaticTextures(I3DEngine::SSvoStaticTexInfo& svoInfo, PodArray<I3DEngine::SLightTI>* pLightsTI_S, PodArray<I3DEngine::SLightTI>* pLightsTI_D);
	virtual void GetSvoBricksForUpdate(PodArray<SSvoNodeInfo>& arrNodeInfo, float fNodeSize, PodArray<SVF_P3F_C4B_T2F>* pVertsOut);
	virtual bool IsSvoReady(bool testPostponed) const;
	virtual i32  GetSvoCompiledData(IDrxArchive* pArchive);
#endif
	// LiveCreate
	virtual void SaveInternalState(struct IDataWriteStream& writer, const AABB& filterArea, const bool bTerrain, u32k objectMask);
	virtual void LoadInternalState(struct IDataReadStream& reader, u8k* pVisibleLayersMask, u16k* pLayerIdTranslation);

	void         SetupLightScissors(SRenderLight* pLight, const SRenderingPassInfo& passInfo);
	bool         IsTerrainTextureStreamingInProgress() { return m_bTerrainTextureStreamingInProgress; }

	bool         IsTerrainSyncLoad()                   { return m_bContentPrecacheRequested && GetCVars()->e_AutoPrecacheTerrainAndProcVeget; }
	bool         IsShadersSyncLoad()                   { return m_bContentPrecacheRequested && GetCVars()->e_AutoPrecacheTexturesAndShaders; }
	bool         IsStatObjSyncLoad()                   { return m_bContentPrecacheRequested && GetCVars()->e_AutoPrecacheCgf; }
	float        GetAverageCameraSpeed()               { return m_fAverageCameraSpeed; }
	Vec3         GetAverageCameraMoveDir()             { return m_vAverageCameraMoveDir; }

	typedef std::map<uint64, i32> ShadowFrustumListsCacheUsers;
	ShadowFrustumListsCacheUsers m_FrustumsCacheUsers[2];

	class CStatObjFoliage*       m_pFirstFoliage, * m_pLastFoliage;
	PodArray<SEntInFoliage>      m_arrEntsInFoliage;
	void RemoveEntInFoliage(i32 i, IPhysicalEntity* pent = 0);

	PodArray<class CRoadRenderNode*> m_lstRoadRenderNodesForUpdate;

	struct ILightSource*            GetSunEntity();
	PodArray<struct ILightSource*>* GetAffectingLights(const AABB& bbox, bool bAllowSun, const SRenderingPassInfo& passInfo);
	void                            UregisterLightFromAccessabilityCache(ILightSource* pLight);
	void                            OnCasterDeleted(IShadowCaster* pCaster);

	virtual void                    ResetCoverageBufferSignalVariables();

	void                            UpdateScene(const SRenderingPassInfo& passInfo);
	void                            UpdateLightSources(const SRenderingPassInfo& passInfo);
	void                            PrepareLightSourcesForRendering_0(const SRenderingPassInfo& passInfo);
	void                            PrepareLightSourcesForRendering_1(const SRenderingPassInfo& passInfo);
	void                            InitShadowFrustums(const SRenderingPassInfo& passInfo);
	void                            PrepareShadowPasses(const SRenderingPassInfo& passInfo, u32& nTimeSlicedShadowsUpdatedThisFrame, std::vector<std::pair<ShadowMapFrustum*, const class CLightEntity*>>& shadowFrustums, std::vector<SRenderingPassInfo>& shadowPassInfo);

	///////////////////////////////////////////////////////////////////////////////

	virtual void            GetLightVolumes(threadID nThreadID, SLightVolume*& pLightVols, u32& nNumVols);
	const CLightVolumesMgr& GetLightVolumeUpr() const { return m_LightVolumesMgr; }
	CLightVolumesMgr&       GetLightVolumeUpr()       { return m_LightVolumesMgr; }

	///////////////////////////////////////////////////////////////////////////////

	void                             FreeLightSourceComponents(SRenderLight* pLight, bool bDeleteLight = true);
	void                             RemoveEntityLightSources(IRenderNode* pEntity);

	void                             CheckPhysicalized(const Vec3& vBoxMin, const Vec3& vBoxMax);

	virtual PodArray<SRenderLight*>* GetDynamicLightSources() { return &m_lstDynLights; }

	i32                              GetRealLightsNum()       { return m_nRealLightsNum; }
	void                             SetupClearColor(const SRenderingPassInfo& passInfo);
	void                             CheckAddLight(SRenderLight* pLight, const SRenderingPassInfo& passInfo);

	void                             DrawTextRightAligned(const float x, const float y, tukk format, ...) PRINTF_PARAMS(4, 5);
	void                             DrawTextRightAligned(const float x, const float y, const float scale, const ColorF& color, tukk format, ...) PRINTF_PARAMS(6, 7);
	void                             DrawTextLeftAligned(const float x, const float y, const float scale, const ColorF& color, tukk format, ...) PRINTF_PARAMS(6, 7);
	void                             DrawTextAligned(i32 flags, const float x, const float y, const float scale, const ColorF& color, tukk format, ...) PRINTF_PARAMS(7, 8);

	float                            GetLightAmount(SRenderLight* pLight, const AABB& objBox);

	IStatObj*                        CreateStatObj();
	virtual IStatObj*                CreateStatObjOptionalIndexedMesh(bool createIndexedMesh);

	IStatObj*                        UpdateDeformableStatObj(IGeometry* pPhysGeom, bop_meshupdate* pLastUpdate = 0, IFoliage* pSrcFoliage = 0);

	// Creates a new indexed mesh.
	IIndexedMesh*                 CreateIndexedMesh();

	void                          InitMaterialDefautMappingAxis(IMaterial* pMat);

	virtual ITerrain*             GetITerrain()             { return (ITerrain*)m_pTerrain; }
	virtual IVisAreaUpr*      GetIVisAreaUpr()      { return (IVisAreaUpr*)m_pVisAreaUpr; }
	virtual IMergedMeshesUpr* GetIMergedMeshesUpr() { return (IMergedMeshesUpr*)m_pMergedMeshesUpr; }

	virtual ITerrain*             CreateTerrain(const STerrainInfo& TerrainInfo);
	void                          DeleteTerrain();
	bool                          LoadTerrain(XmlNodeRef pDoc, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable);
	bool                          LoadVisAreas(std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable);
	bool                          LoadUsedShadersList();
	bool                          PrecreateDecals();
	void                          LoadPhysicsData();
	void                          UnloadPhysicsData();
	void                          LoadFlaresData();
	void                          FreeFoliages();

	void                          LoadCollisionClasses(XmlNodeRef node);

	virtual float                 GetLightAmountInRange(const Vec3& pPos, float fRange, bool bAccurate = 0);

	virtual void                  PrecacheLevel(bool bPrecacheAllVisAreas, Vec3* pPrecachePoints, i32 nPrecachePointsNum);
	virtual void                  ProposeContentPrecache()     { m_bContentPrecacheRequested = true; }
	bool                          IsContentPrecacheRequested() { return m_bContentPrecacheRequested; }

	virtual ITimeOfDay*           GetTimeOfDay();
	//! [GDC09]: Return SkyBox material
	virtual IMaterial*            GetSkyMaterial();
	virtual void                  SetSkyMaterial(IMaterial* pSkyMat);
	bool                          IsHDRSkyMaterial(IMaterial* pMat) const;

	using I3DEngine::SetGlobalParameter;
	virtual void                     SetGlobalParameter(E3DEngineParameter param, const Vec3& v);
	using I3DEngine::GetGlobalParameter;
	virtual void                     GetGlobalParameter(E3DEngineParameter param, Vec3& v);
	virtual void                     SetShadowMode(EShadowMode shadowMode) { m_eShadowMode = shadowMode; }
	virtual EShadowMode              GetShadowMode() const                 { return m_eShadowMode; }
	virtual void                     AddPerObjectShadow(IShadowCaster* pCaster, float fConstBias, float fSlopeBias, float fJitter, const Vec3& vBBoxScale, uint nTexSize);
	virtual void                     RemovePerObjectShadow(IShadowCaster* pCaster);
	virtual struct SPerObjectShadow* GetPerObjectShadow(IShadowCaster* pCaster);
	virtual void                     GetCustomShadowMapFrustums(ShadowMapFrustum**& arrFrustums, i32& nFrustumCount);
	virtual i32                      SaveStatObj(IStatObj* pStatObj, TSerialize ser);
	virtual IStatObj*                LoadStatObj(TSerialize ser);

	virtual void                     OnRenderMeshDeleted(IRenderMesh* pRenderMesh);
	virtual bool                     RenderMeshRayIntersection(IRenderMesh* pRenderMesh, SRayHitInfo& hitInfo, IMaterial* pCustomMtl = 0);
	virtual void                     OnEntityDeleted(struct IEntity* pEntity);
	virtual tukk              GetVoxelEditOperationName(EVoxelEditOperation eOperation);

	virtual void                     SetEditorHeightmapCallback(IEditorHeightmap* pCallBack) { m_pEditorHeightmap = pCallBack; }
	static IEditorHeightmap* m_pEditorHeightmap;

	virtual IParticleUpr* GetParticleUpr() { return m_pPartUpr; }
	virtual IOpticsUpr*   GetOpticsUpr()   { return m_pOpticsUpr; }

	virtual void              RegisterForStreaming(IStreamable* pObj);
	virtual void              UnregisterForStreaming(IStreamable* pObj);

	virtual void              PrecacheCharacter(IRenderNode* pObj, const float fImportance, ICharacterInstance* pCharacter, IMaterial* pSlotMat, const Matrix34& matParent, const float fEntDistance, const float fScale, i32 nMaxDepth, bool bForceStreamingSystemUpdate, const SRenderingPassInfo& passInfo);
	virtual void              PrecacheRenderNode(IRenderNode* pObj, float fEntDistanceReal);

	void                      MarkRNTmpDataPoolForReset() { m_bResetRNTmpDataPool = true; }

	bool                      IsObjectsTreeValid()        { return m_pObjectsTree != nullptr; }
	class COctreeNode*   GetObjectsTree()            { return m_pObjectsTree; }

	static void          GetObjectsByTypeGlobal(PodArray<IRenderNode*>& lstObjects, EERType objType, const AABB* pBBox, bool* pInstStreamReady = NULL, uint64 dwFlags = ~0);
	static void          MoveObjectsIntoListGlobal(PodArray<SRNInfo>* plstResultEntities, const AABB* pAreaBox, bool bRemoveObjects = false, bool bSkipDecals = false, bool bSkip_ERF_NO_DECALNODE_DECALS = false, bool bSkipDynamicObjects = false, EERType eRNType = eERType_TypesNum);

	SRenderNodeTempData* CreateRenderNodeTempData(IRenderNode* pRNode, const SRenderingPassInfo& passInfo);
	SRenderNodeTempData* CheckAndCreateRenderNodeTempData(IRenderNode* pRNode, const SRenderingPassInfo& passInfo);

	void                 UpdateRNTmpDataPool(bool bFreeAll);

	void                 UpdateStatInstGroups();
	void                 UpdateRenderTypeEnableLookup();
	void                 ProcessOcean(const SRenderingPassInfo& passInfo);
	void                 ReRegisterKilledVegetationInstances();
	Vec3                 GetEntityRegisterPoint(IRenderNode* pEnt);

	void                 RenderRenderNode_ShadowPass(IShadowCaster* pRNode, const SRenderingPassInfo& passInfo);
	void                 ProcessCVarsChange();
	ILINE i32            GetGeomDetailScreenRes()
	{
		if (GetCVars()->e_ForceDetailLevelForScreenRes)
		{
			return GetCVars()->e_ForceDetailLevelForScreenRes;
		}
		else if (GetRenderer())
		{
			return std::max(GetRenderer()->GetOverlayWidth(), GetRenderer()->GetOverlayHeight());
		}
		return 1;
	}

	i32                                   GetBlackTexID()   { return m_nBlackTexID; }
	i32                                   GetBlackCMTexID() { return m_nBlackCMTexID; }
	i32                                   GetWhiteTexID()   { return m_nWhiteTexID; }

	virtual void                          SyncProcessStreamingUpdate();

	virtual void                          SetScreenshotCallback(IScreenshotCallback* pCallback);

	virtual void                          RegisterRenderNodeStatusListener(IRenderNodeStatusListener* pListener, EERType renderNodeType);
	virtual void                          UnregisterRenderNodeStatusListener(IRenderNodeStatusListener* pListener, EERType renderNodeType);

	virtual IDeferredPhysicsEventUpr* GetDeferredPhysicsEventUpr() { return m_pDeferredPhysicsEventUpr; }

	void                                  PrintDebugInfo(const SRenderingPassInfo& passInfo);

public:
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC DATA
	//////////////////////////////////////////////////////////////////////////
	class COctreeNode*             m_pObjectsTree = nullptr;

	i32                            m_idMatLeaves; // for shooting foliages
	bool                           m_bResetRNTmpDataPool;

	float                          m_fRefreshSceneDataCVarsSumm;
	i32                            m_nRenderTypeEnableCVarSum;

	PodArray<IRenderNode*>         m_lstAlwaysVisible;
	PodArray<IRenderNode*>         m_lstKilledVegetations;
	PodArray<SPerObjectShadow>     m_lstPerObjectShadows;
	std::vector<ShadowMapFrustum*> m_lstCustomShadowFrustums;
	i32                            m_nCustomShadowFrustumCount;
	u32                         m_onePassShadowFrustumsCount = 0;

	PodArray<SImageInfo>           m_arrBaseTextureData;

	bool                           m_bInShutDown;
	bool                           m_bInUnload;
	bool                           m_bInLoad;

private:
	//////////////////////////////////////////////////////////////////////////
	// PRIVATE DATA
	//////////////////////////////////////////////////////////////////////////
	class CLightEntity* m_pSun;

	std::vector<byte>   arrFPSforSaveLevelStats;
	PodArray<float>     m_arrProcessStreamingLatencyTestResults;
	PodArray<i32>       m_arrProcessStreamingLatencyTexNum;

	// fields which are used by SRenderingPass to store over frame information
	CThreadSafeRendererContainer<CCamera> m_RenderingPassCameras[2];                 // camera storage for SRenderingPass, the cameras cannot be stored on stack to allow job execution

	float m_fZoomFactor;                                // zoom factor of m_RenderingCamera
	float m_fPrevZoomFactor;                            // zoom factor of m_RenderingCamera from last frame
	bool  m_bZoomInProgress;                            // indicates if the RenderingCamera is currently zooming
	i32   m_nZoomMode;                                  // the zoom level of the camera (0-4) 0: no zoom, 4: full zoom

	// cameras used by 3DEngine
	CCamera                m_RenderingCamera;           // Camera used for Rendering on 3DEngine Side, normaly equal to the viewcamera, except if frozen with e_camerafreeze

	PodArray<IRenderNode*> m_deferredRenderProxyStreamingPriorityUpdates;     // deferred streaming priority updates for newly seen CRenderProxies

	float                  m_fLightsHDRDynamicPowerFactor; // lights hdr exponent/exposure

	i32                    m_nBlackTexID;
	i32                    m_nBlackCMTexID;
	i32                    m_nWhiteTexID;

	char                   m_sGetLevelFilePathTmpBuff[MAX_PATH_LENGTH];
	char                   m_szLevelFolder[_MAX_PATH];

	bool                   m_bOcean; // todo: remove

	Vec3                   m_vSkyHightlightPos;
	Vec3                   m_vSkyHightlightCol;
	float                  m_fSkyHighlightSize;
	Vec3                   m_vAmbGroundCol;
	float                  m_fAmbMaxHeight;
	float                  m_fAmbMinHeight;
	IPhysicalEntity*       m_pGlobalWind;
	u8                  m_nOceanRenderFlags;
	Vec3                   m_vPrevMainFrameCamPos;
	float                  m_fAverageCameraSpeed;
	Vec3                   m_vAverageCameraMoveDir;
	EShadowMode            m_eShadowMode;
	bool                   m_bLayersActivated;
	bool                   m_bContentPrecacheRequested;
	bool                   m_bTerrainTextureStreamingInProgress;

	// interfaces
	IPhysMaterialEnumerator* m_pPhysMaterialEnumerator;

	// data containers
	PodArray<SRenderLight*>                   m_lstDynLights;
	PodArray<SRenderLight*>                   m_lstDynLightsNoLight;
	i32                                       m_nRealLightsNum;

	PodArray<ILightSource*>                   m_lstStaticLights;
	PodArray<PodArray<struct ILightSource*>*> m_lstAffectingLightsCombinations;
	PodArray<SRenderLight*>                   m_tmpLstLights;
	PodArray<struct ILightSource*>            m_tmpLstAffectingLights;

	PodArray<SCollisionClass>                 m_collisionClasses;

#define MAX_LIGHTS_NUM 32
	PodArray<CCamera> m_arrLightProjFrustums;

	class CTimeOfDay* m_pTimeOfDay;

	ICVar*            m_pLightQuality;

	// FPS for savelevelstats

	float                          m_fAverageFPS;
	float                          m_fMinFPS, m_fMinFPSDecay;
	float                          m_fMaxFPS, m_fMaxFPSDecay;
	i32                            m_nFramesSinceLevelStart;
	i32                            m_nStreamingFramesSinceLevelStart;
	bool                           m_bPreCacheEndEventSent;
	float                          m_fTimeStateStarted;
	u32                         m_nRenderWorldUSecs;
	SFrameLodInfo                  m_frameLodInfo;

	ITexture*                      m_ptexIconLowMemoryUsage;
	ITexture*                      m_ptexIconAverageMemoryUsage;
	ITexture*                      m_ptexIconHighMemoryUsage;
	ITexture*                      m_ptexIconEditorConnectedToConsole;

	std::vector<IDecalRenderNode*> m_decalRenderNodes; // list of registered decal render nodes, used to clean up longer not drawn decals
	std::vector<IRenderNode*>      m_renderNodesToDelete[2];    // delay deletion of rendernodes by 1 frame to make sure
	u32                         m_renderNodesToDeleteID = 0; // they can be safely used on renderthread

	SImageSubInfo* RegisterImageInfo(byte** pMips, i32 nDim, tukk pName);
	SImageSubInfo* GetImageInfo(tukk pName);
	std::map<string, SImageSubInfo*> m_imageInfos;
	byte**         AllocateMips(byte* pImage, i32 nDim, byte** pImageMips);
	IScreenshotCallback*             m_pScreenshotCallback;

	typedef CListenerSet<IRenderNodeStatusListener*> TRenderNodeStatusListeners;
	typedef std::vector<TRenderNodeStatusListeners>  TRenderNodeStatusListenersArray;
	TRenderNodeStatusListenersArray        m_renderNodeStatusListenersArray;

	OcclusionTestClient                    m_OceanOcclTestVar;

	IDeferredPhysicsEventUpr*          m_pDeferredPhysicsEventUpr;

	std::set<u16>                       m_skipedLayers;

	IGeneralMemoryHeap*                    m_pBreakableBrushHeap;

	CVisibleRenderNodesUpr             m_visibleNodesUpr;

	i32                                    m_nCurrentWindAreaList;
	std::vector<SOptimizedOutdoorWindArea> m_outdoorWindAreas[2];
	std::vector<SOptimizedOutdoorWindArea> m_indoorWindAreas[2];
	std::vector<SOptimizedOutdoorWindArea> m_forcedWindAreas;

	CLightVolumesMgr                       m_LightVolumesMgr;

	std::unique_ptr<CWaterRippleUpr>   m_pWaterRippleUpr;

	friend struct SRenderNodeTempData;
};

#endif // C3DENGINE_H
