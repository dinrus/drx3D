// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/Renderer.h>

#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_ORBIS
	#define SUPPORTS_INPLACE_TEXTURE_STREAMING
#endif

struct ICVar;
struct IConsole;
struct IConsoleCmdArgs;

class CRendererCVars
{
public:
	void InitCVars();

protected:

	// Helper methods.
	static i32  GetTexturesStreamPoolSize();
	static void Cmd_ShowRenderTarget(IConsoleCmdArgs* pArgs);
	static void OnChange_CachedShadows(ICVar* pCVar);
	static void OnChange_GeomInstancingThreshold(ICVar* pVar);
	void        InitExternalCVars();
	void        CacheCaptureCVars();

public:
	//////////////////////////////////////////////////////////////////////
	// console variables
	//////////////////////////////////////////////////////////////////////

	//------------------i32 cvars-------------------------------
	static ICVar* CV_r_ShowDynTexturesFilter;
	static ICVar* CV_r_ShaderCompilerServer;
	static ICVar* CV_r_ShaderCompilerFolderName;
	static ICVar* CV_r_ShaderCompilerFolderSuffix;
	static ICVar* CV_r_ShaderEmailTags;
	static ICVar* CV_r_ShaderEmailCCs;
	static ICVar* CV_r_excludeshader;
	static ICVar* CV_r_excludemesh;
	static ICVar* CV_r_ShowTexture;
	static ICVar* CV_r_TexturesStreamingDebugfilter;

	//declare cvars differing on platforms
	static i32   CV_r_vsync;
#if defined(SUPPORT_DEVICE_INFO_USER_DISPLAY_OVERRIDES)
	static float CV_r_overrideRefreshRate;
	static i32   CV_r_overrideScanlineOrder;
#endif
#if DRX_PLATFORM_WINDOWS
	static i32 CV_r_FullscreenPreemption;
#endif
	DeclareStaticConstIntCVar(CV_r_SyncToFrameFence, 1);

	static i32   CV_r_GraphicsPipeline;
	static i32   CV_r_GraphicsPipelineMobile;
	static i32   CV_r_GraphicsPipelinePassScheduler;

	static i32   CV_r_DeferredShadingTiled;
	static i32   CV_r_DeferredShadingTiledDebug;
	static i32   CV_r_DeferredShadingTiledHairQuality;
	static i32   CV_r_DeferredShadingSSS;
	static i32   CV_r_DeferredShadingFilterGBuffer;

	static i32 CV_r_MotionVectors;
	static i32   CV_r_MotionBlur;
	static i32   CV_r_MotionBlurQuality;
	static i32   CV_r_MotionBlurGBufferVelocity;
	static float CV_r_MotionBlurThreshold;
	static i32   CV_r_UseMergedPosts;
	static i32   CV_r_MaxFrameLatency;
	static i32   CV_r_texatlassize;
	static i32   CV_r_DeferredShadingSortLights;
	static i32   CV_r_batchtype;
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	//HACK: make sure we can only use it for dx11
	static i32 CV_r_SilhouettePOM;
#else
	enum { CV_r_SilhouettePOM = 0 };
#endif
#ifdef WATER_TESSELLATION_RENDERER
	static i32 CV_r_WaterTessellationHW;
#else
	enum { CV_r_WaterTessellationHW = 0 };
#endif

	static i32   CV_r_tessellationdebug;
	static float CV_r_tessellationtrianglesize;
	static float CV_r_displacementfactor;
	static i32   CV_r_geominstancingthreshold;
	static i32   CV_r_ShadowsPCFiltering;
	static i32   CV_r_rc_autoinvoke;
	static i32   CV_r_Refraction;
	static i32   CV_r_PostProcessReset;
	static i32   CV_r_colorRangeCompression;
	static i32   CV_r_colorgrading_selectivecolor;
	static i32   CV_r_colorgrading_charts;
	static i32   CV_r_ColorgradingChartsCache;
	static i32   CV_r_ShaderCompilerPort;
	static i32   CV_r_ShowDynTexturesMaxCount;
	static i32   CV_r_ShaderCompilerDontCache;
	static i32   CV_r_dyntexmaxsize;
	static i32   CV_r_dyntexatlascloudsmaxsize;
	static i32   CV_r_dyntexatlasspritesmaxsize;
	static i32   CV_r_dyntexatlasvoxterrainsize;
	static i32   CV_r_dyntexatlasdyntexsrcsize;
	static i32   CV_r_texminanisotropy;
	static i32   CV_r_texmaxanisotropy;
	static i32   CV_r_rendertargetpoolsize;
	static i32   CV_r_watercausticsdeferred;
	static i32   CV_r_WaterUpdateThread;
	static i32   CV_r_ConditionalRendering;
	static i32   CV_r_watercaustics;
	static i32   CV_r_watervolumecaustics;
	static i32   CV_r_watervolumecausticsdensity;
	static i32   CV_r_watervolumecausticsresolution;
#if DRX_PLATFORM_DESKTOP
	static ICVar*       CV_r_ShaderTarget;
	static i32          ShaderTargetFlag;
#endif

	static ICVar*       CV_r_VkShaderCompiler;

	//  static i32 CV_r_envcmwrite;
	static i32 CV_r_shadersremotecompiler;
	static i32 CV_r_shadersasynccompiling;
	static i32 CV_r_shadersasyncactivation;
	static i32 CV_r_shadersasyncmaxthreads;
	static i32 CV_r_shaderscachedeterministic;
	static i32 CV_r_ShadersCachePrecacheAll;
	static i32 CV_r_shaderssubmitrequestline;
	static i32 CV_r_shaderslogcachemisses;
	static i32 CV_r_shadersImport;
	static i32 CV_r_shadersExport;
	static i32 CV_r_meshpoolsize;
	static i32 CV_r_meshinstancepoolsize;
	static i32 CV_r_multigpu;

	static i32 CV_r_nodrawnear;
	static i32 CV_r_DrawNearShadows;
	static i32 CV_r_scissor;
	static i32 CV_r_usezpass;
	static i32 CV_r_VegetationSpritesTexRes;
	static i32 CV_r_ShowVideoMemoryStats;
	static i32 CV_r_TexturesStreamingDebugMinSize;
	static i32 CV_r_TexturesStreamingDebugMinMip;
	static i32 CV_r_enableAltTab;
	static i32 CV_r_StereoFlipEyes;
	static i32 CV_r_StereoEnableMgpu;
	static i32 CV_r_DynTexSourceSharedRTWidth;
	static i32 CV_r_DynTexSourceSharedRTHeight;
	static i32 CV_r_DynTexSourceUseSharedRT;
	static i32 CV_r_GetScreenShot;

	static i32 CV_r_cloakFadeByDist;
	static i32 CV_r_cloakRefractionFadeByDist;

	static i32 CV_r_BreakOnError;

	static i32 CV_r_TexturesStreamPoolSize; //plz do not access directly, always by GetTexturesStreamPoolSize()
	static i32 CV_r_TexturesStreamPoolSecondarySize;
	static i32 CV_r_texturesstreampooldefragmentation;
	static i32 CV_r_texturesstreampooldefragmentationmaxmoves;
	static i32 CV_r_texturesstreampooldefragmentationmaxamount;

	static i32 CV_r_ReprojectOnlyStaticObjects;
	static i32 CV_r_ReadZBufferDirectlyFromVMEM;
	static i32 CV_r_durango_async_dips;
	static i32 CV_r_durango_async_dips_sync;
	static i32 CV_r_D3D12SubmissionThread;
	static i32 CV_r_D3D12WaitableSwapChain;
	static i32 CV_r_D3D12BatchResourceBarriers;
	static i32 CV_r_D3D12EarlyResourceBarriers;
	static i32 CV_r_D3D12AsynchronousCompute;
	static i32 CV_r_D3D12HardwareComputeQueue;
	static i32 CV_r_D3D12HardwareCopyQueue;
	static i32 CV_r_VkSubmissionThread;
	static i32 CV_r_VkBatchResourceBarriers;
	static i32 CV_r_VkHardwareComputeQueue;
	static i32 CV_r_VkHardwareCopyQueue;
	static i32 CV_r_FlushToGPU;
	static i32 CV_r_EnableDebugLayer;
	static i32 CV_r_NoDraw;
	static i32 CV_r_UpdateInstances;

	// compute skinning related cvars
	DeclareConstIntCVar(r_ComputeSkinning, 1);
	DeclareConstIntCVar(r_ComputeSkinningMorphs, 1);
	DeclareConstIntCVar(r_ComputeSkinningTangents, 1);
	DeclareConstIntCVar(r_ComputeSkinningDebugDraw, 0);

	//declare in release mode constant cvars
	DeclareStaticConstIntCVar(CV_r_stats, 0);
	DeclareStaticConstIntCVar(CV_r_statsMinDrawcalls, 0);
	DeclareStaticConstIntCVar(CV_r_profiler, 0);

	static i32 CV_r_HDRDithering;

	static float CV_r_profilerTargetFPS;
	static float CV_r_profilerSmoothingWeight;
	DeclareStaticConstIntCVar(CV_r_ShadowPoolMaxFrames, 30);
	DeclareStaticConstIntCVar(CV_r_log, 0);
	DeclareStaticConstIntCVar(CV_r_logTexStreaming, 0);
	DeclareStaticConstIntCVar(CV_r_logShaders, 0);
	static i32 CV_r_logVBuffers;
	DeclareStaticConstIntCVar(CV_r_logVidMem, 0);
	DeclareStaticConstIntCVar(CV_r_useESRAM, 1);
	DeclareStaticConstIntCVar(CV_r_multithreaded, MULTITHREADED_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_multithreadedDrawing, -1);
	DeclareStaticConstIntCVar(CV_r_multithreadedDrawingMinJobSize, 100);
	DeclareStaticConstIntCVar(CV_r_deferredshadingLightVolumes, 1);
	DeclareStaticConstIntCVar(CV_r_deferredDecals, 1);
	DeclareStaticConstIntCVar(CV_r_deferredDecalsDebug, 0);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingLBuffersFmt, 1);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingScissor, 1);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingDebugGBuffer, 0);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingEnvProbes, 1);
	static i32 CV_r_DeferredShadingAmbient;
	DeclareStaticConstIntCVar(CV_r_DeferredShadingAmbientLights, 1);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingLights, 1);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingAreaLights, 0);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingStencilPrepass, 1);
	static i32 CV_r_HDRSwapChain;
	DeclareStaticConstIntCVar(CV_r_HDRDebug, 0);
	static i32 CV_r_HDRBloom;
	static i32 CV_r_HDRBloomQuality;
	static i32 CV_r_HDRVignetting;
	DeclareStaticConstIntCVar(CV_r_HDRTexFormat, 1);
	DeclareStaticConstIntCVar(CV_r_HDRRangeAdapt, HDR_RANGE_ADAPT_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_GrainEnableExposureThreshold, 0);

	static i32 CV_r_HDREyeAdaptationMode;
	DeclareStaticConstIntCVar(CV_r_geominstancing, GEOM_INSTANCING_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_geominstancingdebug, 0);
	DeclareStaticConstIntCVar(CV_r_materialsbatching, 1);
	DeclareStaticConstIntCVar(CV_r_DebugLightVolumes, 0);
	DeclareStaticConstIntCVar(CV_r_shadowtexformat, 0);
	DeclareStaticConstIntCVar(CV_r_ShadowsMask, 1);
	DeclareStaticConstIntCVar(CV_r_ShadowsMaskResolution, 0);
	DeclareStaticConstIntCVar(CV_r_CBufferUseNativeDepth, CBUFFER_NATIVE_DEPTH_DEAFULT_VAL);
	DeclareStaticConstIntCVar(CV_r_ShadowMaskStencilPrepass, 0);
	DeclareStaticConstIntCVar(CV_r_ShadowsGridAligned, 1);
	DeclareStaticConstIntCVar(CV_r_ShadowMapsUpdate, 1);
	DeclareStaticConstIntCVar(CV_r_ShadowGenDepthClip, 1);
	static i32   CV_r_ShadowsCache;
	static i32   CV_r_ShadowsCacheFormat;
	static i32   CV_r_ShadowsNearestMapResolution;
	static i32   CV_r_ShadowsScreenSpace;
	static float CV_r_ShadowsScreenSpaceLength;
	DeclareStaticConstIntCVar(CV_r_debuglights, 0);
	DeclareStaticConstIntCVar(CV_r_DeferredShadingDepthBoundsTest, DEF_SHAD_DBT_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_deferredshadingDBTstencil, DEF_SHAD_DBT_STENCIL_DEFAULT_VAL);
	static i32 CV_r_sunshafts;
	DeclareStaticConstIntCVar(CV_r_SonarVision, 1);
	DeclareStaticConstIntCVar(CV_r_ThermalVision, 1);
	DeclareStaticConstIntCVar(CV_r_ThermalVisionViewCloakFrequencyPrimary, 1);
	DeclareStaticConstIntCVar(CV_r_ThermalVisionViewCloakFrequencySecondary, 1);
	DeclareStaticConstIntCVar(CV_r_NightVision, 2);
	DeclareStaticConstIntCVar(CV_r_MergeShadowDrawcalls, 1);
	static i32 CV_r_PostProcess_CB;
	static i32 CV_r_PostProcess;
	static i32 CV_r_PostProcessFilters;
	DeclareStaticConstIntCVar(CV_r_PostProcessGameFx, 1);
	DeclareStaticConstIntCVar(CV_r_PostProcessParamsBlending, 1);
	DeclareStaticConstIntCVar(CV_r_PostProcessHUD3D, 1);
	DeclareStaticConstIntCVar(CV_r_PostProcessHUD3DDebugView, 0);
	DeclareStaticConstIntCVar(CV_r_PostProcessHUD3DStencilClear, 1);
	static i32 CV_r_PostProcessHUD3DCache;
	DeclareStaticConstIntCVar(CV_r_PostProcessNanoGlassDebugView, 0);
	static i32 CV_r_colorgrading;
	DeclareStaticConstIntCVar(CV_r_colorgrading_levels, 1);
	DeclareStaticConstIntCVar(CV_r_colorgrading_filters, 1);
	DeclareStaticConstIntCVar(CV_r_showdyntextures, 0);
	DeclareStaticConstIntCVar(CV_r_shownormals, 0);
	DeclareStaticConstIntCVar(CV_r_showlines, 0);
	DeclareStaticConstIntCVar(CV_r_showtangents, 0);
	DeclareStaticConstIntCVar(CV_r_showtimegraph, 0);
	DeclareStaticConstIntCVar(CV_r_DebugFontRendering, 0);
	DeclareStaticConstIntCVar(CV_profileStreaming, 0);
	DeclareStaticConstIntCVar(CV_r_graphstyle, 0);
	DeclareStaticConstIntCVar(CV_r_showbufferusage, 0);
	DeclareStaticConstIntCVar(CV_r_profileshaders, 0);
	DeclareStaticConstIntCVar(CV_r_ProfileShadersSmooth, 4);
	DeclareStaticConstIntCVar(CV_r_ProfileShadersGroupByName, 1);
	DeclareStaticConstIntCVar(CV_r_texpostponeloading, 1);
	DeclareStaticConstIntCVar(CV_r_texpreallocateatlases, TEXPREALLOCATLAS_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_texlog, 0);
	DeclareStaticConstIntCVar(CV_r_texnoload, 0);
	DeclareStaticConstIntCVar(CV_r_texturecompiling, 1);
	DeclareStaticConstIntCVar(CV_r_texturecompilingIndicator, 0);
	DeclareStaticConstIntCVar(CV_r_texturesstreaming, TEXSTREAMING_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_TexturesStreamingDebug, 0);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingnoupload, 0);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingonlyvideo, 0);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingmipfading, 8);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingUpdateType, TEXSTREAMING_UPDATETYPE_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingPrecacheRounds, 1);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingSuppress, 0);
	DeclareStaticConstIntCVar(CV_r_TexturesStreamingLowestPrefetchBias, 0);
	DeclareStaticConstIntCVar(CV_r_TexturesStreamingMaxUpdateRate, 1);
	static i32 CV_r_texturesstreamingSkipMips;
	static i32 CV_r_texturesstreamingMinUsableMips;
	static i32 CV_r_texturesstreamingJobUpdate;
#if defined(TEXSTRM_DEFERRED_UPLOAD)
	static i32 CV_r_texturesstreamingDeferred;
#endif
	DeclareStaticConstIntCVar(CV_r_texturesstreamingPostponeMips, 0);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingPostponeThresholdKB, 1024);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingPostponeThresholdMip, 1);
	DeclareStaticConstIntCVar(CV_r_texturesstreamingMinReadSizeKB, 64);
#if defined(SUPPORTS_INPLACE_TEXTURE_STREAMING)
	static i32 CV_r_texturesstreamingInPlace;
#endif
	DeclareStaticConstIntCVar(CV_r_lightssinglepass, 1);
	static i32 CV_r_envcmresolution;
	static i32 CV_r_envtexresolution;
	DeclareStaticConstIntCVar(CV_r_waterreflections_mgpu, 0);
	DeclareStaticConstIntCVar(CV_r_waterreflections_use_min_offset, 1);
	static i32 CV_r_waterreflections;
	DeclareStaticConstIntCVar(CV_r_waterreflections_quality, WATERREFLQUAL_DEFAULT_VAL);
	static i32 CV_r_water_godrays;
	static i32 CV_r_reflections;
	static i32 CV_r_reflections_quality;
	static i32 CV_r_dof;
	static i32 CV_r_texNoAnisoAlphaTest;
	DeclareStaticConstIntCVar(CV_r_reloadshaders, 0);
	DeclareStaticConstIntCVar(CV_r_detailtextures, 1);
	DeclareStaticConstIntCVar(CV_r_texbindmode, 0);
	DeclareStaticConstIntCVar(CV_r_shadersdebug, 0);
	DeclareStaticConstIntCVar(CV_r_shadersignoreincludeschanging, 0);
	DeclareStaticConstIntCVar(CV_r_shaderslazyunload, 0);
	DeclareStaticConstIntCVar(CV_r_shadersCompileStrict, 0);
	DeclareStaticConstIntCVar(CV_r_shadersCompileCompatible, 1);
	static i32 CV_r_shadersAllowCompilation;
	DeclareStaticConstIntCVar(CV_r_shaderscompileautoactivate, 0);
	DeclareStaticConstIntCVar(CV_r_shadersediting, 0);
	DeclareStaticConstIntCVar(CV_r_shadersprecachealllights, 1);
	DeclareStaticConstIntCVar(CV_r_ReflectTextureSlots, 1);
	DeclareStaticConstIntCVar(CV_r_debugrendermode, 0);
	DeclareStaticConstIntCVar(CV_r_debugrefraction, 0);
	DeclareStaticConstIntCVar(CV_r_meshprecache, 1);
	DeclareStaticConstIntCVar(CV_r_validateDraw, 0);
	static i32 CV_r_flares;
	static i32 CV_r_flareHqShafts;
	DeclareStaticConstIntCVar(CV_r_ZPassDepthSorting, ZPASS_DEPTH_SORT_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_TransparentPasses, 1);
	DeclareStaticConstIntCVar(CV_r_SkipAlphaTested, 0);
	static i32 CV_r_TranspDepthFixup;
	DeclareStaticConstIntCVar(CV_r_usehwskinning, 1);
	DeclareStaticConstIntCVar(CV_r_usemateriallayers, 2);
	static i32 CV_r_ParticlesSoftIsec;
	static i32 CV_r_ParticlesRefraction;
	static i32   CV_r_ParticlesTessellation;
	static i32   CV_r_ParticlesTessellationTriSize;
	static float CV_r_ParticlesAmountGI;
	static i32   CV_r_ParticlesHalfRes;
	DeclareStaticConstIntCVar(CV_r_ParticlesHalfResAmount, 0);
	DeclareStaticConstIntCVar(CV_r_ParticlesHalfResBlendMode, 0);
	DeclareStaticConstIntCVar(CV_r_ParticlesInstanceVertices, 1);
	DeclareStaticConstIntCVar(CV_r_AntialiasingModeEditor, 1);
	DeclareStaticConstIntCVar(CV_r_AntialiasingModeDebug, 0);
	static i32 CV_r_rain;
	static i32 CV_r_rain_ignore_nearest;
	static i32 CV_r_snow;
	DeclareStaticConstIntCVar(CV_r_snow_halfres, 0);
	DeclareStaticConstIntCVar(CV_r_snow_displacement, 0);
	DeclareStaticConstIntCVar(CV_r_snowFlakeClusters, 100);
	DeclareStaticConstIntCVar(CV_r_customvisions, CUSTOMVISIONS_DEFAULT_VAL);
	DeclareStaticConstIntCVar(CV_r_DebugLayerEffect, 0);
	DeclareStaticConstIntCVar(CV_r_VrProjectionType, 0);
	DeclareStaticConstIntCVar(CV_r_VrProjectionPreset, 0);
	DeclareStaticConstIntCVar(CV_r_stereoMirrorProjection, 1);
	static i32 CV_r_DofMode;
	static i32 CV_r_DofBokehQuality;
	DeclareStaticConstIntCVar(CV_r_nohwgamma, 2);
	DeclareStaticConstIntCVar(CV_r_wireframe, 0);
	DeclareStaticConstIntCVar(CV_r_printmemoryleaks, 0);
	DeclareStaticConstIntCVar(CV_r_releaseallresourcesonexit, 1);
	DeclareStaticConstIntCVar(CV_r_character_nodeform, 0);
	DeclareStaticConstIntCVar(CV_r_VegetationSpritesGenDebug, 0);
	DeclareStaticConstIntCVar(CV_r_VegetationSpritesMaxLightingUpdates, 8);
	DeclareStaticConstIntCVar(CV_r_ZPassOnly, 0);
	DeclareStaticConstIntCVar(CV_r_VegetationSpritesNoGen, 0);
	DeclareStaticConstIntCVar(CV_r_VegetationSpritesGenAlways, 0);
	//DeclareStaticConstIntCVar(CV_r_measureoverdraw, 0);
	enum { CV_r_measureoverdraw = 0 };
	DeclareStaticConstIntCVar(CV_r_ShowLightBounds, 0);
	DeclareStaticConstIntCVar(CV_r_TextureCompressor, 1);
	DeclareStaticConstIntCVar(CV_r_TexturesStreamingDebugDumpIntoLog, 0);
	DeclareStaticConstIntCVar(CV_e_DebugTexelDensity, 0);
	DeclareStaticConstIntCVar(CV_e_DebugDraw, 0);
	static i32 CV_r_RainDropsEffect;
	DeclareStaticConstIntCVar(CV_r_RefractionPartialResolveMode, 2);
	DeclareStaticConstIntCVar(CV_r_RefractionPartialResolveMinimalResolveArea, 0);
	DeclareStaticConstIntCVar(CV_r_RefractionPartialResolveMaxResolveCount, 0);
	DeclareStaticConstIntCVar(CV_r_RefractionPartialResolvesDebug, 0);
	DeclareStaticConstIntCVar(CV_r_Batching, 1);
	DeclareStaticConstIntCVar(CV_r_Unlit, 0);
	DeclareStaticConstIntCVar(CV_r_HideSunInCubemaps, 1);
	DeclareStaticConstIntCVar(CV_r_ParticlesDebug, 0);
	DeclareStaticConstIntCVar(CV_r_CubemapGenerationTimeout, 100);

	//--------------float cvars----------------------

	static float CV_r_ZPrepassMaxDist;
	static float CV_r_FlaresChromaShift;
	static i32   CV_r_FlaresIrisShaftMaxPolyNum;
	static i32   CV_r_FlaresEnableColorGrading;
	static float CV_r_FlaresTessellationRatio;

	static float CV_r_drawnearfov;
	static float CV_r_measureoverdrawscale;
	static float CV_r_DeferredShadingLightLodRatio;
	static float CV_r_DeferredShadingLightStencilRatio;
	static float CV_r_rainDistMultiplier;
	static float CV_r_rainOccluderSizeTreshold;

	static float CV_r_HDRRangeAdaptMaxRange;
	static float CV_r_HDRRangeAdaptMax;
	static float CV_r_HDRRangeAdaptLBufferMaxRange;
	static float CV_r_HDRRangeAdaptLBufferMax;
	static float CV_r_HDRRangeAdaptationSpeed;

	static float CV_r_HDREyeAdaptationSpeed;

	static float CV_r_HDRGrainAmount;

	static float CV_r_Sharpening;
	static float CV_r_ChromaticAberration;

	static float CV_r_NightVisionFinalMul;
	static float CV_r_NightVisionBrightLevel;
	static float CV_r_NightVisionSonarRadius;
	static float CV_r_NightVisionSonarLifetime;
	static float CV_r_NightVisionSonarMultiplier;
	static float CV_r_NightVisionCamMovNoiseAmount;
	static float CV_r_NightVisionCamMovNoiseBlendSpeed;

	static float CV_r_dofMinZ;
	static float CV_r_dofMinZScale;
	static float CV_r_dofMinZBlendMult;
	static float CV_r_dofDilation;
	static float CV_r_ShadowsBias;
	static float CV_r_ShadowsAdaptionRangeClamp;
	static float CV_r_ShadowsAdaptionSize;
	static float CV_r_ShadowsAdaptionMin;
	static float CV_r_ShadowsParticleKernelSize;
	static float CV_r_ShadowsParticleJitterAmount;
	static float CV_r_ShadowsParticleAnimJitterAmount;
	static float CV_r_ShadowsParticleNormalEffect;
	static float CV_r_shadow_jittering; // dont use this directly for rendering. use m_shadowJittering or GetShadowJittering() instead;

	static i32   CV_r_ShadowPoolMaxTimeslicedUpdatesPerFrame;
	static i32   CV_r_ShadowCastingLightsMaxCount;
	static i32   CV_r_HeightMapAO;
	static float CV_r_HeightMapAOAmount;
	static float CV_r_HeightMapAOResolution;
	static float CV_r_HeightMapAORange;
	static float CV_r_RenderMeshHashGridUnitSize;
	static float CV_r_ThermalVisionViewDistance;
	static float CV_r_ThermalVisionViewCloakFlickerMinIntensity;
	static float CV_r_ThermalVisionViewCloakFlickerMaxIntensity;
	static float CV_r_PostprocessParamsBlendingTimeScale;
	static float CV_r_PostProcessHUD3DShadowAmount;
	static float CV_r_PostProcessHUD3DGlowAmount;
	static float CV_r_normalslength;
	static float CV_r_TexelsPerMeter;
	static float CV_r_TexturesStreamingMaxRequestedMB;
	static i32   CV_r_TexturesStreamingMaxRequestedJobs;
	static float CV_r_TexturesStreamingMipBias;
	static i32   CV_r_TexturesStreamingMipClampDVD;
	static i32   CV_r_TexturesStreamingDisableNoStreamDuringLoad;
	static float CV_r_envtexupdateinterval;
	static float CV_r_TextureLodDistanceRatio;
	static float CV_r_water_godrays_distortion;
	static float CV_r_waterupdateFactor;
	static float CV_r_waterupdateDistance;
	static float CV_r_waterreflections_min_visible_pixels_update;
	static float CV_r_waterreflections_minvis_updatefactormul;
	static float CV_r_waterreflections_minvis_updatedistancemul;
	static float CV_r_waterreflections_offset;
	static float CV_r_watercausticsdistance;
	static float CV_r_watervolumecausticssnapfactor;
	static float CV_r_watervolumecausticsmaxdistance;
	static float CV_r_detaildistance;
	static float CV_r_DrawNearZRange;
	static float CV_r_DrawNearFarPlane;
	static float CV_r_rainamount;
	static float CV_r_MotionBlurShutterSpeed;
	static float CV_r_MotionBlurCameraMotionScale;
	static float CV_r_MotionBlurMaxViewDist;
	static float CV_r_cloakLightScale;
	static float CV_r_cloakTransitionLightScale;
	static float CV_r_cloakFadeLightScale;
	static float CV_r_cloakFadeStartDistSq;
	static float CV_r_cloakFadeEndDistSq;
	static float CV_r_cloakFadeMinValue;
	static float CV_r_cloakRefractionFadeStartDistSq;
	static float CV_r_cloakRefractionFadeEndDistSq;
	static float CV_r_cloakRefractionFadeMinValue;
	static float CV_r_cloakMinLightValue;
	static float CV_r_cloakHeatScale;
	static i32   CV_r_cloakRenderInThermalVision;
	static float CV_r_cloakMinAmbientOutdoors;
	static float CV_r_cloakMinAmbientIndoors;
	static float CV_r_cloakSparksAlpha;
	static float CV_r_cloakInterferenceSparksAlpha;
	static float CV_r_cloakHighlightStrength;
	static float CV_r_armourPulseSpeedMultiplier;
	static float CV_r_maxSuitPulseSpeedMultiplier;
	static float CV_r_gamma;
	static float CV_r_contrast;
	static float CV_r_brightness;

	static float CV_r_ZFightingDepthScale;
	static float CV_r_ZFightingExtrude;
	static float CV_r_FlashMatTexResQuality;
	static float CV_r_stereoScaleCoefficient;
	static float CV_r_StereoStrength;
	static float CV_r_StereoEyeDist;
	static float CV_r_StereoScreenDist;
	static float CV_r_StereoNearGeoScale;
	static float CV_r_StereoHudScreenDist;
	static float CV_r_StereoGammaAdjustment;
	static i32   CV_r_ConsoleBackbufferWidth;
	static i32   CV_r_ConsoleBackbufferHeight;

	static i32   CV_r_AntialiasingMode_CB;
	static i32   CV_r_AntialiasingMode;
	static i32   CV_r_AntialiasingModeSCull;
	static i32   CV_r_AntialiasingTAAPattern;
	static float CV_r_AntialiasingTAAFalloffHiFreq;
	static float CV_r_AntialiasingTAAFalloffLowFreq;
	static float CV_r_AntialiasingTAASharpening;
	static float CV_r_AntialiasingTSAAMipBias;
	static float CV_r_AntialiasingTSAASubpixelDetection;
	static float CV_r_AntialiasingTSAASmoothness;

	static float CV_r_FogDepthTest;
#if defined(VOLUMETRIC_FOG_SHADOWS)
	static i32   CV_r_FogShadows;
	static i32   CV_r_FogShadowsMode;
#endif
	static i32   CV_r_FogShadowsWater;

	static float CV_r_rain_maxviewdist;
	static float CV_r_rain_maxviewdist_deferred;

	static i32   CV_r_SSReflections;
	static i32   CV_r_SSReflHalfRes;
	static float CV_r_SSReflDistance;
	static i32   CV_r_SSReflSamples;
	static i32   CV_r_ssdo;
	static i32   CV_r_ssdoHalfRes;
	static i32   CV_r_ssdoColorBleeding;
	static float CV_r_ssdoRadius;
	static float CV_r_ssdoRadiusMin;
	static float CV_r_ssdoRadiusMax;
	static float CV_r_ssdoAmountDirect;
	static float CV_r_ssdoAmountAmbient;
	static float CV_r_ssdoAmountReflection;

	static i32   CV_r_CustomResMaxSize;
	static i32   CV_r_CustomResWidth;
	static i32   CV_r_CustomResHeight;
	static i32   CV_r_CustomResPreview;
	static i32   CV_r_Supersampling;
	static i32   CV_r_SupersamplingFilter;

#if defined(ENABLE_RENDER_AUX_GEOM)
	static i32 CV_r_enableauxgeom;
#endif

	static i32 CV_r_buffer_banksize;
	static i32 CV_r_constantbuffer_banksize;
	static i32 CV_r_constantbuffer_watermark;
	static i32 CV_r_transient_pool_size;
	static i32 CV_r_buffer_sli_workaround;
	DeclareStaticConstIntCVar(CV_r_buffer_enable_lockless_updates, 1);
	DeclareStaticConstIntCVar(CV_r_enable_full_gpu_sync, 0);
	static i32    CV_r_buffer_pool_max_allocs;
	static i32    CV_r_buffer_pool_defrag_static;
	static i32    CV_r_buffer_pool_defrag_dynamic;
	static i32    CV_r_buffer_pool_defrag_max_moves;

	static i32    CV_r_ParticleVerticePoolSize;
	static i32    CV_r_ParticleMaxVerticePoolSize;

	static i32    CV_r_GeomCacheInstanceThreshold;

	static i32    CV_r_VisAreaClipLightsPerPixel;

	static i32    CV_r_VolumetricFogTexScale;
	static i32    CV_r_VolumetricFogTexDepth;
	static float  CV_r_VolumetricFogReprojectionBlendFactor;
	static i32    CV_r_VolumetricFogSample;
	static i32    CV_r_VolumetricFogShadow;
	static i32    CV_r_VolumetricFogDownscaledSunShadow;
	static i32    CV_r_VolumetricFogDownscaledSunShadowRatio;
	static i32    CV_r_VolumetricFogReprojectionMode;
	static float  CV_r_VolumetricFogMinimumLightBulbSize;
	static i32    CV_r_VolumetricFogSunLightCorrection;

	static i32    CV_r_UsePersistentRTForModelHUD;

	static i32    CV_d3d11_CBUpdateStats;
	static ICVar* CV_d3d11_forcedFeatureLevel;

#if defined(DX11_ALLOW_D3D_DEBUG_RUNTIME)
	static ICVar* CV_d3d11_debugMuteSeverity;
	static ICVar* CV_d3d11_debugMuteMsgID;
	static ICVar* CV_d3d11_debugBreakOnMsgID;
	static i32    CV_d3d11_debugBreakOnce;
#endif

#if defined(DRX_PLATFORM_WINDOWS)
	static i32 CV_d3d11_preventDriverThreading;
	ICVar*     CV_r_FullscreenNativeRes;
#endif

	static i32 CV_r_VolumetricClouds;
	static i32 CV_r_VolumetricCloudsRaymarchStepNum;
	static i32 CV_r_VolumetricCloudsPipeline;
	static i32 CV_r_VolumetricCloudsStereoReprojection;
	static i32 CV_r_VolumetricCloudsTemporalReprojection;
	static i32 CV_r_VolumetricCloudsShadowResolution;
	static i32 CV_r_GpuParticles;
	static i32 CV_r_GpuParticlesConstantRadiusBoundingBoxes;
	static i32 CV_r_GpuPhysicsFluidDynamicsDebug;

	ICVar*     CV_capture_frames;
	ICVar*     CV_capture_folder;
	ICVar*     CV_capture_file_format;
	ICVar*     CV_capture_frame_once;
	ICVar*     CV_capture_file_name;
	ICVar*     CV_capture_file_prefix;
	//--------------end cvars------------------------

	//////////////////////////////////////////////////////////////////////////
	ICVar* m_CVWidth;
	ICVar* m_CVHeight;
	ICVar* m_CVWindowType;
	ICVar* m_CVColorBits;
	ICVar* m_CVDisplayInfo;
	//////////////////////////////////////////////////////////////////////////

	static Vec2 s_overscanBorders;
};

class CCVarUpdateRecorder : public IConsoleVarSink
{
public:

	struct SUpdateRecord
	{
		union
		{
			i32   intValue;
			float floatValue;
			char  stringValue[64];
		};

		tukk name;
		i32         type;

		SUpdateRecord(ICVar* pCVar);
		bool operator==(const SUpdateRecord& rhs)
		{
			return type == rhs.type && (strcmp(name, rhs.name) == 0);
		}
	};

	typedef std::vector<SUpdateRecord> CVarList;

public:
	CCVarUpdateRecorder(IConsole* pConsole);
	~CCVarUpdateRecorder();

	// IConsoleVarSink
	virtual bool         OnBeforeVarChange(ICVar* pVar, tukk sNewValue) { return true; }
	virtual void         OnAfterVarChange(ICVar* pVar);
	virtual void         OnVarUnregister(ICVar* pVar);

	void                 Reset();
	const CVarList&      GetCVars() const;
	const SUpdateRecord* GetCVar(tukk cvarName) const;

public:
	CVarList  m_updatedCVars[RT_COMMAND_BUF_COUNT];
	IConsole* m_pConsole;

};
