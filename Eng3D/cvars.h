// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   cvars.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio.NET
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _3DENGINE_CVARS_H_
#define _3DENGINE_CVARS_H_

#if defined(CONSOLE_CONST_CVAR_MODE)
	#define GetFloatCVar(name) name ## Default
#else
	#define GetFloatCVar(name) (DinrusX3dEngBase::GetCVars())->name
#endif

// console variables
struct CVars : public DinrusX3dEngBase
{
	CVars()
	{ Init(); }

	void Init();

#if defined(FEATURE_SVO_GI)
	void RegisterTICVars();
#endif

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	//default values used for const cvars
#ifdef _RELEASE
	enum { e_StatObjValidateDefault = 0 };  // Validate meshes in all but release builds.
#else
	enum { e_StatObjValidateDefault = 1 };  // Validate meshes in all but release builds.
#endif
#ifdef CONSOLE_CONST_CVAR_MODE
	enum { e_DisplayMemoryUsageIconDefault = 0 };
#else
	enum { e_DisplayMemoryUsageIconDefault = 1 };
#endif
#define e_PhysOceanCellDefault (0.f)
#if DRX_PLATFORM_DURANGO
	enum { e_ParticlesSimpleWaterCollisionDefault = 1 };
	enum { e_DeformableObjectsDefault = 0 }; // temporarily disabled until RC issue found
	enum { e_OcclusionVolumesDefault = 1 };
	enum { e_WaterOceanFastpathDefault = 0 };
	enum { e_WaterVolumesDefault = 1 };
	enum { e_WaterOceanDefault = 1 };
	enum { e_LightVolumesDefault = 1 };
	enum { e_ShadowsBlendDefault = 1 };
#else
	enum { e_DeformableObjectsDefault = 1 };
	enum { e_OcclusionVolumesDefault = 1 };
	enum { e_WaterOceanDefault = 1 };
	enum { e_WaterVolumesDefault = 1 };
	enum { e_LightVolumesDefault = 1 };
#endif

#define e_DecalsDeferredDynamicMinSizeDefault              (0.35f)
#define e_DecalsPlacementTestAreaSizeDefault               (0.08f)
#define e_DecalsPlacementTestMinDepthDefault               (0.05f)
#define e_StreamPredictionDistanceFarDefault               (16.f)
#define e_StreamPredictionDistanceNearDefault              (0.f)
#define e_StreamCgfVisObjPriorityDefault                   (0.5f)
#define e_WindBendingDistRatioDefault                      (0.5f)
#define e_MaxViewDistFullDistCamHeightDefault              (1000.f)
#define e_CoverageBufferOccludersLodRatioDefault           (0.25f)
#define e_LodCompMaxSizeDefault                            (6.f)
#define e_MaxViewDistanceDefault                           (-1.f)
#define e_ViewDistCompMaxSizeDefault                       (64.f)
#define e_ViewDistRatioPortalsDefault                      (60.f)
#define e_ParticlesLightsViewDistRatioDefault              (256.f)
#define e_TerrainOcclusionCullingPrecisionDefault          (0.25f)
#define e_TerrainOcclusionCullingPrecisionDistRatioDefault (3.f)
#define e_TerrainOcclusionCullingStepSizeDeltaDefault      (1.05f)
#define e_WindDefault                                      (0.1f)
#define e_ShadowsCastViewDistRatioLightsDefault            (1.f)
#define e_DecalsRangeDefault                               (20.f)
#define e_MinMassDistanceCheckRenderMeshCollisionDefault   (0.5f)
#define e_GsmRangeStepExtendedDefault                      (8.f)
#define e_TerrainDetailMaterialsViewDistXYDefault          (2048.f)
#define e_SunAngleSnapSecDefault                           (0.1f)
#define e_SunAngleSnapDotDefault                           (0.999999f)
#define e_OcclusionVolumesViewDistRatioDefault             (0.05f)
#define e_FoliageStiffnessDefault                          (3.2f)
#define e_FoliageBranchesStiffnessDefault                  (100.f)
#define e_FoliageBranchesDampingDefault                    (10.f)
#define e_FoliageBrokenBranchesDampingDefault              (15.f)
#define e_TerrainTextureLodRatioDefault                    (1.f)
#define e_JointStrengthScaleDefault                        (1.f)
#define e_VolObjShadowStrengthDefault                      (.4f)
#define e_CameraRotationSpeedDefault                       (0.f)
#define e_DecalsDeferredDynamicDepthScaleDefault           (4.0f)
#define e_TerrainDetailMaterialsViewDistZDefault           (128.f)
#define e_StreamCgfFastUpdateMaxDistanceDefault            (16.f)
#define e_StreamPredictionMinFarZoneDistanceDefault        (16.f)
#define e_StreamPredictionMinReportDistanceDefault         (0.75f)
#define e_StreamCgfGridUpdateDistanceDefault               (0.f)
#define e_StreamPredictionAheadDefault                     (0.5f)
#define e_StreamPredictionAheadDebugDefault                (0.f)
#define e_RenderMeshCollisionToleranceDefault              (0.3f)
#define e_VegetationSpritesScaleFactorDefault              (1.0f)
#ifdef DEDICATED_SERVER
	#define e_RenderDefault                                  (0)
#else
	#ifdef _RELEASE
		#define e_RenderDefault (1)
	#else
		#define e_RenderDefault (gEnv->IsDedicated() ? 0 : 1)
	#endif
#endif

	i32    e_PermanentRenderObjects;
	i32    e_TerrainTextureStreamingPoolItemsNum;
	i32    e_ParticlesPoolSize;
	i32    e_ParticlesVertexPoolSize;
	i32    e_ParticlesIndexPoolSize;
	i32    e_ParticlesProfile;
	i32    e_ParticlesProfiler;
	ICVar* e_ParticlesProfilerOutputFolder;
	ICVar* e_ParticlesProfilerOutputName;
	i32    e_ParticlesProfilerCountBudget;
	i32    e_ParticlesProfilerTimingBudget;
	i32    e_ParticlesForceSeed;
	float  e_VegetationSpritesDistanceRatio;
	i32    e_Decals;
	i32    e_DecalsAllowGameDecals;
	DeclareConstFloatCVar(e_FoliageBrokenBranchesDamping);
	float  e_ShadowsCastViewDistRatio;
	i32    e_WaterTessellationAmountY;
	float  e_OnDemandMaxSize;
	float  e_MaxViewDistSpecLerp;
	float  e_StreamAutoMipFactorSpeedThreshold;
	DeclareConstFloatCVar(e_DecalsDeferredDynamicMinSize);
	DeclareConstIntCVar(e_Objects, 1);
	float e_ViewDistRatioCustom;
	float e_StreamPredictionUpdateTimeSlice;
	DeclareConstIntCVar(e_DisplayMemoryUsageIcon, e_DisplayMemoryUsageIconDefault);
	i32   e_ScreenShotWidth;
	i32   e_ScreenShotDebug;
#if DRX_PLATFORM_WINDOWS
	i32   e_ShadowsLodBiasFixed;
#else
	DeclareConstIntCVar(e_ShadowsLodBiasFixed, 0);
#endif
	DeclareConstIntCVar(e_FogVolumes, 1);
	i32    e_VolumetricFog;
	DeclareConstIntCVar(e_Render, e_RenderDefault);
	i32    e_Tessellation;
	float  e_TessellationMaxDistance;
	i32    e_ShadowsTessellateCascades;
	DeclareConstIntCVar(e_ShadowsTessellateDLights, 0);
	i32    e_CoverageBufferReproj;
	i32    e_CoverageBufferRastPolyLimit;
	i32    e_CoverageBufferShowOccluder;
	float  e_ViewDistRatioPortals;
	i32    e_ParticlesLights;
	DeclareConstIntCVar(e_ObjFastRegister, 1);
	float  e_ViewDistRatioLights;
	float  e_LightIlluminanceThreshold;
	DeclareConstIntCVar(e_DebugDraw, 0);
	ICVar* e_DebugDrawFilter;
	DeclareConstIntCVar(e_DebugDrawListSize, 16);
	DeclareConstIntCVar(e_DebugDrawListBBoxIndex, 0);
	float e_DebugDrawMaxDistance;
#if !defined(_RELEASE)
	tukk e_pStatObjRenderFilterStr;
	i32         e_statObjRenderFilterMode;
#endif
	DeclareConstIntCVar(e_AutoPrecacheTexturesAndShaders, 0);
	i32   e_StreamPredictionMaxVisAreaRecursion;
	float e_StreamPredictionBoxRadius;
	i32   e_Clouds;
	i32   e_VegetationBillboards;
	i32   e_VegetationUseTerrainColor;
	float e_VegetationUseTerrainColorDistance;
	i32   e_BrushUseTerrainColor;
	i32   e_DecalsMaxTrisInObject;
	DeclareConstFloatCVar(e_OcclusionVolumesViewDistRatio);
	DeclareConstFloatCVar(e_SunAngleSnapDot);
	i32   e_PreloadDecals;
	float e_DecalsLifeTimeScale;
	i32   e_DecalsForceDeferred;
	DeclareConstIntCVar(e_CoverageBufferDebugFreeze, 0);
	DeclareConstIntCVar(e_TerrainOcclusionCulling, 1);
	i32    e_PhysProxyTriLimit;
	float  e_FoliageWindActivationDist;
	ICVar* e_SQTestTextureName;
	i32    e_ShadowsClouds;
	i32    e_levelStartupFrameDelay;
	float  e_SkyUpdateRate;
	float  e_RecursionViewDistRatio;
	DeclareConstIntCVar(e_StreamCgfDebugMinObjSize, 100);
	i32    e_CullVegActivation;
	i32    e_StreamPredictionTexelDensity;
	i32    e_StreamPredictionAlwaysIncludeOutside;
	DeclareConstIntCVar(e_DynamicLights, 1);
	i32    e_DynamicLightsFrameIdVisTest;
	DeclareConstIntCVar(e_WaterWavesTessellationAmount, 5);
	i32    e_ShadowsLodBiasInvis;
	float  e_CoverageBufferBias;
	i32    e_DynamicLightsMaxEntityLights;
	i32    e_SQTestMoveSpeed;
	float  e_StreamAutoMipFactorMax;
	i32    e_ObjQuality;
	i32    e_StaticInstancing;
	i32    e_StaticInstancingMinInstNum;
	i32    e_RNTmpDataPoolMaxFrames;
	i32    e_DynamicLightsMaxCount;
	DeclareConstIntCVar(e_DebugLights, 0);
	i32    e_StreamCgfPoolSize;
	DeclareConstIntCVar(e_StatObjPreload, 1);
	DeclareConstIntCVar(e_ShadowsDebug, 0);
	DeclareConstIntCVar(e_ShadowsCascadesCentered, 0);
	DeclareConstIntCVar(e_ShadowsCascadesDebug, 0);
	DeclareConstFloatCVar(e_StreamPredictionDistanceNear);
	DeclareConstIntCVar(e_TerrainDrawThisSectorOnly, 0);
	DeclareConstFloatCVar(e_VegetationSpritesScaleFactor);
	float e_ParticlesMaxDrawScreen;
	i32   e_ParticlesAnimBlend;
	DeclareConstIntCVar(e_GsmStats, 0);
	DeclareConstIntCVar(e_DynamicLightsForceDeferred, 1);
	DeclareConstIntCVar(e_Fog, 1);
	float e_TimeOfDay;
	i32   e_Terrain;
	i32   e_TerrainAutoGenerateBaseTexture;
	float e_TerrainAutoGenerateBaseTextureTiling;
	i32   e_TerrainIntegrateObjectsMaxVertices;
	i32   e_TerrainIntegrateObjectsMaxHeight;
	DeclareConstIntCVar(e_SkyBox, 1);
	float e_CoverageBufferAABBExpand;
	i32   e_CoverageBufferEarlyOut;
	float e_CoverageBufferEarlyOutDelay;
	float e_CoverageBufferTerrainExpand;
	DeclareConstIntCVar(e_WaterWaves, 0);
	i32   e_GsmCastFromTerrain;
	float e_TerrainLodDistanceRatio;
	float e_TerrainLodErrorRatio;
	i32   e_StatObjBufferRenderTasks;
	DeclareConstIntCVar(e_StreamCgfUpdatePerNodeDistance, 1);
	DeclareConstFloatCVar(e_DecalsDeferredDynamicDepthScale);
	DeclareConstIntCVar(e_TerrainBBoxes, 0);
	i32 e_LightVolumes;
	DeclareConstIntCVar(e_LightVolumesDebug, 0);
	DeclareConstIntCVar(e_Portals, 1);
	DeclareConstIntCVar(e_PortalsBlend, 1);
	i32   e_PortalsMaxRecursion;
	float e_StreamAutoMipFactorMaxDVD;
	DeclareConstIntCVar(e_CameraFreeze, 0);
	float e_ParticlesMinDrawPixels;
	DeclareConstFloatCVar(e_StreamPredictionAhead);
	DeclareConstFloatCVar(e_FoliageBranchesStiffness);
	DeclareConstFloatCVar(e_StreamPredictionMinFarZoneDistance);
	i32   e_StreamCgf;
	i32   e_StreamInstances;
	i32   e_StreamInstancesMaxTasks;
	float e_StreamInstancesDistRatio;
	i32   e_CheckOcclusion;
	i32   e_CheckOcclusionQueueSize;
	i32   e_CheckOcclusionOutputQueueSize;
	DeclareConstIntCVar(e_WaterVolumes, e_WaterVolumesDefault);
	DeclareConstFloatCVar(e_TerrainOcclusionCullingPrecisionDistRatio);
	float e_ScreenShotMapCamHeight;
	i32   e_DeformableObjects;
	DeclareConstFloatCVar(e_StreamCgfFastUpdateMaxDistance);
	DeclareConstIntCVar(e_DecalsClip, 1);
	ICVar* e_ScreenShotFileFormat;
	i32    e_CharLodMin;
	float  e_PhysOceanCell;
	i32    e_WindAreas;
	DeclareConstFloatCVar(e_WindBendingDistRatio);
	float  e_WindBendingStrength;
	float  e_WindBendingAreaStrength;
	float  e_SQTestDelay;
	i32    e_PhysMinCellSize;
	i32    e_StreamCgfMaxTasksInProgress;
	i32    e_StreamCgfMaxNewTasksPerUpdate;
	DeclareConstFloatCVar(e_DecalsPlacementTestAreaSize);
	DeclareConstFloatCVar(e_DecalsPlacementTestMinDepth);
	DeclareConstFloatCVar(e_CameraRotationSpeed);
	float  e_ScreenShotMapSizeY;
	i32    e_GI;
	DeclareConstIntCVar(e_PortalsBigEntitiesFix, 1);
	i32    e_SQTestBegin;
	i32    e_VegetationSprites;
	ICVar* e_CameraGoto;
	DeclareConstIntCVar(e_ParticlesCullAgainstViewFrustum, 1);
	DeclareConstIntCVar(e_ParticlesCullAgainstOcclusionBuffer, 1);
	DeclareConstFloatCVar(e_StreamPredictionMinReportDistance);
	i32   e_WaterTessellationSwathWidth;
	DeclareConstIntCVar(e_StreamSaveStartupResultsIntoXML, 0);
	i32   e_PhysFoliage;
	DeclareConstIntCVar(e_RenderMeshUpdateAsync, 1);
	i32   e_ParticlesPreload;
	i32   e_ParticlesAllowRuntimeLoad;
	i32   e_ParticlesConvertPfx1;
	DeclareConstIntCVar(e_ParticlesSerializeNamedFields, 1);
	float e_CoverageBufferOccludersViewDistRatio; // TODO: make use of this cvar
	i32   e_DecalsDeferredDynamic;
	DeclareConstIntCVar(e_DefaultMaterial, 0);
	i32   e_LodMin;
	i32   e_RenderMeshCollisionLod;
	DeclareConstIntCVar(e_PreloadMaterials, 1);
	DeclareConstIntCVar(e_ObjStats, 0);
	DeclareConstIntCVar(e_TerrainDeformations, 0);
	i32 e_TerrainDetailMaterials;
	DeclareConstIntCVar(e_ShadowsFrustums, 0);
	DeclareConstIntCVar(e_OcclusionVolumes, e_OcclusionVolumesDefault);
	i32   e_TerrainEditPostponeTexturesUpdate;
	i32   e_DecalsDeferredStatic;
	DeclareConstIntCVar(e_Roads, 1);
	float e_TerrainDetailMaterialsViewDistXY;
	i32   e_ParticlesQuality;
	DeclareConstIntCVar(e_DebugDrawShowOnlyCompound, 0);
	DeclareConstFloatCVar(e_SunAngleSnapSec);
	float e_GsmRangeStep;
	i32   e_ParticlesGI;
	i32   e_ParticlesSoftIntersect;
	i32   e_ParticlesMotionBlur;
	i32   e_ParticlesShadows;
	i32   e_ParticlesAudio;
	i32   e_ParticleShadowsNumGSMs;
	float e_FoliageBranchesTimeout;
	DeclareConstFloatCVar(e_TerrainOcclusionCullingStepSizeDelta);
	float e_LodRatio;
	float e_LodTransitionTime;
	float e_LodFaceAreaTargetSize;
	float e_ObjectsTreeNodeMinSize;
	float e_ObjectsTreeNodeSizeRatio;
	i32   e_ExecuteRenderAsJobMask;
	float e_ObjectsTreeLevelsDebug;
	DeclareConstIntCVar(e_CoverageBufferDrawOccluders, 0);
	DeclareConstIntCVar(e_ObjectsTreeBBoxes, 0);
	DeclareConstIntCVar(e_PrepareDeformableObjectsAtLoadTime, 0);
	DeclareConstIntCVar(e_3dEngineTempPoolSize, 1024);
	DeclareConstFloatCVar(e_MaxViewDistFullDistCamHeight);
	i32   e_VegetationBending;
	DeclareConstFloatCVar(e_StreamPredictionAheadDebug);
	float e_ShadowsSlopeBias;
	float e_ShadowsAutoBias;
	DeclareConstIntCVar(e_GsmDepthBoundsDebug, 0);
	DeclareConstIntCVar(e_TimeOfDayDebug, 0);
	i32   e_WaterTessellationAmount;
	i32   e_Entities;
	i32   e_CoverageBuffer;
	i32   e_ScreenShotQuality;
	DeclareConstFloatCVar(e_FoliageBranchesDamping);
	i32   e_levelStartupFrameNum;
	DeclareConstIntCVar(e_DecalsPreCreate, 1);
	float e_ParticlesLightsViewDistRatio;
	i32   e_Brushes;
	i32   e_SQTestCount;
	float e_GsmRange;
	i32   e_ScreenShotMapOrientation;
	i32   e_ScreenShotHeight;
	DeclareConstIntCVar(e_VegetationBoneInfo, 0);
	i32   e_WaterOceanFFT;
	DeclareConstFloatCVar(e_MaxViewDistance);
	DeclareConstIntCVar(e_AutoPrecacheCameraJumpDist, 16);
	DeclareConstIntCVar(e_LodsForceUse, 1);
	i32 e_Particles;
	DeclareConstIntCVar(e_ParticlesDumpMemoryAfterMapLoad, 0);
	DeclareConstIntCVar(e_ForceDetailLevelForScreenRes, 0);
	DeclareConstIntCVar(e_TerrainTextureStreamingDebug, 0);
	DeclareConstIntCVar(e_3dEngineLogAlways, 0);
	float e_VegetationMinSize;
	DeclareConstIntCVar(e_DecalsHitCache, 1);
	DeclareConstIntCVar(e_TerrainOcclusionCullingDebug, 0);
	float e_ParticlesLod;
	DeclareConstIntCVar(e_BBoxes, 0);
	i32   e_Vegetation;
	float e_TimeOfDaySpeed;
	i32   e_LodMax;
	DeclareConstFloatCVar(e_ViewDistCompMaxSize);
	DeclareConstFloatCVar(e_TerrainTextureLodRatio);
	float e_ShadowsAdaptScale;
	float e_ScreenShotMapSizeX;
	i32	e_ScreenShotMapResolution;
	float e_OcclusionCullingViewDistRatio;
	i32   e_WaterOceanBottom;
	DeclareConstIntCVar(e_WaterRipplesDebug, 0);
	i32   e_OnDemandPhysics;
	float e_ShadowsResScale;
	i32   e_Recursion;
	DeclareConstIntCVar(e_StatObjValidate, e_StatObjValidateDefault);
	DeclareConstIntCVar(e_DecalsMaxValidFrames, 600);
	DeclareConstIntCVar(e_DecalsMerge, 0);
	DeclareConstFloatCVar(e_FoliageStiffness);
	i32   e_SQTestDistance;
	float e_ViewDistMin;
	float e_StreamAutoMipFactorMin;
	DeclareConstIntCVar(e_LodMinTtris, 300);
	i32   e_SkyQuality;
	DeclareConstIntCVar(e_ScissorDebug, 0);
	DeclareConstIntCVar(e_StatObjMergeMaxTrisPerDrawCall, 500);
	DeclareConstIntCVar(e_DynamicLightsConsistentSortOrder, 1);
	DeclareConstIntCVar(e_StreamCgfDebug, 0);
	float e_TerrainOcclusionCullingMaxDist;
	i32   e_TerrainMeshInstancingMinLod;
	float e_TerrainMeshInstancingShadowLodRatio;
	float e_TerrainMeshInstancingShadowBias;
	i32   e_StatObjTessellationMode;
	DeclareConstIntCVar(e_OcclusionLazyHideFrames, 0);
	i32   e_CoverageBufferCullIndividualBrushesMaxNodeSize;
	DeclareConstFloatCVar(e_TerrainOcclusionCullingPrecision);
	float e_RenderMeshCollisionTolerance;
	i32   e_ShadowsCacheUpdate;
	i32   e_ShadowsCacheExtendLastCascade;
	i32   e_ShadowsCacheMaxNodesPerFrame;
	i32   e_ShadowsCacheObjectLod;
	i32   e_ShadowsCacheRenderCharacters;
	i32   e_ShadowsPerObject;
	i32   e_DynamicDistanceShadows;
	float e_ShadowsPerObjectResolutionScale;
	i32   e_ObjShadowCastSpec;
	DeclareConstFloatCVar(e_JointStrengthScale);
	float e_DecalsNeighborMaxLifeTime;
	DeclareConstFloatCVar(e_StreamCgfVisObjPriority);
	i32   e_ObjectLayersActivation;
	float e_VegetationSpritesDistanceCustomRatioMin;
	float e_LodTransitionSpriteDistRatio;
	float e_LodTransitionSpriteMinDist;
	i32   e_WaterTessellationAmountX;
	i32   e_ScreenShotMinSlices;
	i32   e_DecalsMaxUpdatesPerFrame;
	i32   e_SkyType;
	i32   e_GsmLodsNum;
	DeclareConstIntCVar(e_AutoPrecacheCgf, 1);
	DeclareConstIntCVar(e_HwOcclusionCullingWater, 1);
	DeclareConstIntCVar(e_DeferredPhysicsEvents, 1);
	float e_ParticlesMinDrawAlpha;
	float e_ShadowsCastViewDistRatioLights;
	i32   e_ShadowsUpdateViewDistRatio;
	i32   e_Lods;
	DeclareConstIntCVar(e_LodFaceArea, 1);
	float e_ShadowsConstBias;
	i32   e_ParticlesCollisions;
	i32   e_ParticlesObjectCollisions;
	i32   e_ParticlesMinPhysicsDynamicBounds;
	i32   e_ParticlesSortQuality;
	DeclareConstIntCVar(e_Ropes, 1);
	i32   e_ShadowsPoolSize;
	i32   e_ShadowsMaxTexRes;
	i32   e_Sun;
	DeclareConstFloatCVar(e_MinMassDistanceCheckRenderMeshCollision);
	float e_DecalsRange;
	float e_ScreenShotMapCenterY;
	i32   e_CacheNearestCubePicking;
	DeclareConstIntCVar(e_CoverCgfDebug, 0);
	DeclareConstFloatCVar(e_StreamCgfGridUpdateDistance);
	DeclareConstFloatCVar(e_LodCompMaxSize);
	float e_ViewDistRatioDetail;
	DeclareConstIntCVar(e_TerrainDetailMaterialsDebug, 0);
	DeclareConstIntCVar(e_Sleep, 0);
	DeclareConstIntCVar(e_TerrainOcclusionCullingStepSize, 4);
	i32   e_Wind;
	i32   e_SQTestMip;
	i32   e_Shadows;
	i32   e_ShadowsBlendCascades;
	float e_ShadowsBlendCascadesVal;
	float e_ParticlesMaxScreenFill;
	DeclareConstIntCVar(e_DebugDrawShowOnlyLod, -1);
	DeclareConstIntCVar(e_ScreenShot, 0);
	DeclareConstIntCVar(e_PrecacheLevel, 0);
	float e_ScreenShotMapCenterX;
	DeclareConstIntCVar(e_CoverageBufferDebug, 0);
	DeclareConstIntCVar(e_StatObjMerge, 1);
	DeclareConstIntCVar(e_StatObjStoreMesh, 0);
	ICVar* e_StreamCgfDebugFilter;
	float  e_VegetationSpritesMinDistance;
	float  e_TerrainDetailMaterialsViewDistZ;
	DeclareConstFloatCVar(e_VolObjShadowStrength);
	DeclareConstIntCVar(e_ParticlesDebug, 0);
	DeclareConstIntCVar(e_WaterOcean, e_WaterOceanDefault);
	float e_ViewDistRatio;
	float e_ViewDistRatioVegetation;
	ICVar* e_AutoViewDistRatio;
	float e_ViewDistRatioModifierGameDecals;
	DeclareConstIntCVar(e_ObjectLayersActivationPhysics, 1);
	DeclareConstIntCVar(e_StreamCgfDebugHeatMap, 0);
	DeclareConstFloatCVar(e_StreamPredictionDistanceFar);
	DeclareConstIntCVar(e_VegetationSpritesBatching, 1);
	DeclareConstIntCVar(e_CoverageBufferTerrain, 0);
	i32   e_ParticlesThread;
	i32   e_SQTestExitOnFinish;
	DeclareConstIntCVar(e_TerrainOcclusionCullingMaxSteps, 50);
	i32   e_ParticlesUseLevelSpecificLibs;
	i32   e_DecalsOverlapping;
	float e_DecalsSpawnDistRatio;
	i32   e_CGFMaxFileSize;
	i32   e_MaxDrawCalls;
	i32   e_ClipVolumes;
#if !defined(_RELEASE)
	i32   e_MergedMeshesClusterVisualization;
	i32   e_MergedMeshesClusterVisualizationDimension;
#endif

	// ProcVegetation cvars
	i32    e_ProcVegetation;
	i32    e_ProcVegetationMaxSectorsInCache;
	i32    e_ProcVegetationMaxChunksInCache;
	i32    e_ProcVegetationMaxCacheLevels;
	i32    e_ProcVegetationMaxViewDistance;
	i32    e_ProcVegetationMaxObjectsInChunk;
	i32    e_AutoPrecacheTerrainAndProcVeget;

	i32    e_DebugGeomPrep;
	i32    e_MergedMeshes;
	i32    e_MergedMeshesDebug;
	i32    e_MergedMeshesPool;
	i32    e_MergedMeshesPoolSpines;
	i32    e_MergedMeshesTesselationSupport;
	float  e_MergedMeshesViewDistRatio;
	float  e_MergedMeshesLodRatio;
	float  e_MergedMeshesInstanceDist;
	float  e_MergedMeshesActiveDist;
	float  e_MergedMeshesDeformViewDistMod;
	i32    e_MergedMeshesUseSpines;
	float  e_MergedMeshesBulletSpeedFactor;
	float  e_MergedMeshesBulletScale;
	float  e_MergedMeshesBulletLifetime;
	i32    e_MergedMeshesOutdoorOnly;
	i32    e_MergedMeshesMaxTriangles;
	i32    e_CheckOctreeObjectsBoxSize;
	DeclareConstIntCVar(e_GeomCaches, 1);
	i32    e_GeomCacheBufferSize;
	i32    e_GeomCacheMaxPlaybackFromMemorySize;
	i32    e_GeomCachePreferredDiskRequestSize;
	float  e_GeomCacheMinBufferAheadTime;
	float  e_GeomCacheMaxBufferAheadTime;
	float  e_GeomCacheDecodeAheadTime;
	DeclareConstIntCVar(e_GeomCacheDebug, 0);
	ICVar* e_GeomCacheDebugFilter;
	DeclareConstIntCVar(e_GeomCacheDebugDrawMode, 0);
	DeclareConstIntCVar(e_GeomCacheLerpBetweenFrames, 1);

#if defined(FEATURE_SVO_GI)
	#include "SceneTreeCVars.inl" // include SVO related variables
#endif
};

#endif // _3DENGINE_CVARS_H_
