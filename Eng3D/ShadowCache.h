// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef STATIC_SHADOWS_H
#define STATIC_SHADOWS_H

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Render/Shadow_Renderer.h>

class ShadowCacheGenerator : public DinrusX3dEngBase
{
public:
	ShadowCacheGenerator(CLightEntity* pLightEntity, ShadowMapFrustum::ShadowCacheData::eUpdateStrategy nUpdateStrategy)
		: m_pLightEntity(pLightEntity)
		, m_nUpdateStrategy(nUpdateStrategy)
	{}

	static void  ResetGenerationID() { m_cacheGenerationId = 0; }

	void InitShadowFrustum(ShadowMapFrustumPtr& pFr, i32 nLod, i32 nFirstStaticLod, float fDistFromViewDynamicLod, float fRadiusDynamicLod, const SRenderingPassInfo& passInfo);
	void InitHeightMapAOFrustum(ShadowMapFrustumPtr& pFr, i32 nLod, i32 nFirstStaticLod, const SRenderingPassInfo& passInfo);

private:
	static i32k    MAX_RENDERNODES_PER_FRAME = 50;
	static const float  AO_FRUSTUM_SLOPE_BIAS;

	static i32   m_cacheGenerationId;

	void         InitCachedFrustum(ShadowMapFrustumPtr& pFr, ShadowMapFrustum::ShadowCacheData::eUpdateStrategy nUpdateStrategy, i32 nLod, i32 cacheLod, i32 nTexSize, const Vec3& vLightPos, const AABB& projectionBoundsLS, const SRenderingPassInfo& passInfo);
	void         AddTerrainCastersToFrustum(ShadowMapFrustum* pFr, const SRenderingPassInfo& passInfo);

	void         GetCasterBox(AABB& BBoxWS, AABB& BBoxLS, float fRadius, const Matrix34& matView, const SRenderingPassInfo& passInfo);
	Matrix44     GetViewMatrix(const SRenderingPassInfo& passInfo);

	u8        GetNextGenerationID() const;

	CLightEntity* m_pLightEntity;
	ShadowMapFrustum::ShadowCacheData::eUpdateStrategy m_nUpdateStrategy;
};
#endif
