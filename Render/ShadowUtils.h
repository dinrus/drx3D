// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   ShadowUtils.h :

   Revision история:
* Created by Nick Kasyan
   =============================================================================*/

#pragma once

#define DEG2RAD_R(a) (f64(a) * (g_PI / 180.0))
#define RAD2DEG_R(a) float((f64)(a) * (180.0 / g_PI))

static const float g_fOmniShadowFov = 95.0f;
static const float g_fOmniLightFov = 90.5f;

class CPoissonDiskGen
{
	std::vector<Vec2>                   m_vSamples;

	static std::vector<CPoissonDiskGen> s_kernelSizeGens; // the size of the kernel for each entry is the index in this vector

private:
	static void RandomPoint(Vec2& p);
	void        InitSamples();

public:
	Vec2&                   GetSample(i32 ind);

	static CPoissonDiskGen& GetGenForKernelSize(i32 num);
	static void             FreeMemory();
};

enum EFrustum_Type
{
	FTYP_SHADOWOMNIPROJECTION,
	FTYP_SHADOWPROJECTION,
	FTYP_OMNILIGHTVOLUME,
	FTYP_LIGHTVOLUME,
	FTYP_MAX,
	FTYP_UNKNOWN
};

struct SRenderTileInfo;
struct SRenderViewport;

class CRenderView;

class CShadowUtils
{
public:
	static i32k MaxCascadesNum = 4;

	// Bit flags for forward shadows.
	enum eForwardShadowFlags
	{
		eForwardShadowFlags_Cascade0           = BIT(0),
		eForwardShadowFlags_Cascade1           = BIT(1),
		eForwardShadowFlags_Cascade2           = BIT(2),
		eForwardShadowFlags_Cascade3           = BIT(3),
		eForwardShadowFlags_Cascade0_SingleTap = BIT(4),
		eForwardShadowFlags_CloudsShadows      = BIT(5),
	};

	// forward shadow textures.
	struct SShadowCascades
	{
		CTexture* pShadowMap[MaxCascadesNum];
		CTexture* pCloudShadowMap;
	};

	// forward shadow sampling parameters.
	struct SShadowCascadesSamplingInfo
	{
		Matrix44 shadowTexGen[MaxCascadesNum];
		Vec4     invShadowMapSize;
		Vec4     depthTestBias;
		Vec4     oneDivFarDist;
		Vec4     kernelRadius;
		Vec4     cloudShadowParams;
		Vec4     cloudShadowAnimParams;
		Vec4     irregKernel2d[8];
	};

	struct SShadowSamplingInfo
	{
		Matrix44 shadowTexGen;
		Matrix44 screenToShadowBasis; // normalized basis vectors in rows 0-2, basis vector scales in row 3
		Matrix44 noiseProjection;
		Matrix44 blendTexGen;
		Vec4     camPosShadowSpace;
		Vec4     blendInfo;
		Vec4     blendTcNormalize;
		float    oneDivFarDist;
		float    oneDivFarDistBlend;
		float    depthTestBias;
		float    kernelRadius;
		float    invShadowMapSize;
		float    shadowFadingDist;
	};

	struct SShadowsSetupInfo
	{
		Matrix44 ShadowMat;
		float    RecpFarDist;
	};

public:
	static void              CalcDifferentials(const CCamera& cam, float fViewWidth, float fViewHeight, float& fFragSizeX);
	static void              ProjectScreenToWorldExpansionBasis(const Matrix44r& mShadowTexGen, const CCamera& cam, const Vec2& vJitter, float fViewWidth, float fViewHeight, Vec4r& vWBasisX, Vec4r& vWBasisY, Vec4r& vWBasisZ, Vec4r& vCamPos, bool bWPos);
	static void              CalcScreenToWorldExpansionBasis(const CCamera& cam, float fViewWidth, float fViewHeight, Vec3& vWBasisX, Vec3& vWBasisY, Vec3& vWBasisZ, bool bWPos);

	static void              GetProjectiveTexGen(const SRenderLight* pLight, i32 nFace, Matrix44A* mTexGen);
	static void              GetCubemapFrustumForLight(const SRenderLight* pLight, i32 nS, float fFov, Matrix44A* pmProj, Matrix44A* pmView, bool bProjLight);

	static void              GetShadowMatrixForObject(Matrix44A& mLightProj, Matrix44A& mLightView, Vec4& vFrustumInfo, Vec3 vLightSrcRelPos, const AABB& aabb);
	static AABB              GetShadowMatrixForCasterBox(Matrix44A& mLightProj, Matrix44A& mLightView, ShadowMapFrustum* lof, float fFarPlaneOffset = 0);
	static void              GetCubemapFrustum(EFrustum_Type eFrustumType, const ShadowMapFrustum* pFrust, i32 nS, Matrix44A* pmProj, Matrix44A* pmView, Matrix33* pmLightRot = NULL);

	static Matrix34          GetAreaLightMatrix(const SRenderLight* pLight, Vec3 vScale);

	static void              mathMatrixLookAtSnap(Matrix44A* pMatr, const Vec3& Eye, const Vec3& At, ShadowMapFrustum* pFrust);
	static void              GetShadowMatrixOrtho(Matrix44A& mLightProj, Matrix44A& mLightView, const Matrix44A& mViewMatrix, ShadowMapFrustum* lof, bool bViewDependent);

	static void              GetIrregKernel(float sData[][4], i32 nSamplesNum);

	static bool              GetSubfrustumMatrix(Matrix44A& result, const ShadowMapFrustum* pFullFrustum, const ShadowMapFrustum* pSubFrustum);

	static Matrix44          GetClipToTexSpaceMatrix(const ShadowMapFrustum* pFrustum, i32 nSide);

	// setup shader constants, return the validity of the textures for forward sun shadow, and return the textures and the shader runtime flags.
	static bool SetupShadowsForFog(SShadowCascades& shadowCascades, const CRenderView* pRenderView);

	// set the sampler-states and textures for forward sun shadow to RenderPass.
	template<class RenderPassType>
	static void SetShadowSamplingContextToRenderPass(
	  RenderPassType& pass,
	  i32 linearClampComparisonSamplerSlot,
	  i32 pointWrapSamplerSlot,
	  i32 pointClampSamplerSlot,
	  i32 bilinearWrapSamplerSlot,
	  i32 shadowNoiseTextureSlot);

	// set the textures for forward sun shadow to RenderPass.
	template<class RenderPassType>
	static void SetShadowCascadesToRenderPass(
	  RenderPassType& pass,
	  i32 startShadowMapsTexSlot,
	  i32 cloudShadowTexSlot,
	  const SShadowCascades& shadowCascades);

	// return the validity of all cascade textures for forward sun shadow, and return the textures.
	static bool GetShadowCascades(SShadowCascades& shadowCascades, const CRenderView* pRenderView);

	// return the validity of all sun shadow cascades, and shadow sampling info for forward rendering.
	static bool                GetShadowCascadesSamplingInfo(SShadowCascadesSamplingInfo& samplingInfo, const CRenderView* pRenderView);

	static SShadowSamplingInfo GetDeferredShadowSamplingInfo(ShadowMapFrustum* pFr, i32 nSide, const CCamera& cam, const SRenderViewport& viewport, const Vec2& subpixelOffset);

	CShadowUtils();
	~CShadowUtils();

private:
	static void GetForwardShadowSamplingInfo(SShadowSamplingInfo& samplingInfo, Matrix44& lightViewProj, const ShadowMapFrustum* pFr, i32 nSide);

private:
	// Currently forced to use always ID 0 for sun (if sun present)
	static i32k nSunLightID = 0;
};
