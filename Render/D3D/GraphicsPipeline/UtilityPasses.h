// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/FullscreenPass.h>

struct IUtilityRenderPass
{
	enum class EPassId : u32
	{
		StretchRectPass = 0,
		StretchRegionPass,
		SharpeningUpsamplePass,
		NearestDepthUpsamplePass,
		DownsamplePass,
		StableDownsamplePass,
		DepthDownsamplePass,
		GaussianBlurPass,
		MipmapGenPass,
		ClearSurfacePass,
		ClearRegionPass,
		AnisotropicVerticalBlurPass,

		MaxPassCount,
	};

	virtual ~IUtilityRenderPass() {};
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStretchRectPass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pSrcRT, CTexture* pDestRT);

	static CStretchRectPass &GetPass();
	static void Shutdown();

	static EPassId GetPassId() { return EPassId::StretchRectPass; }

protected:
	CFullscreenPass m_pass;

private:
	static CStretchRectPass *s_pPass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStretchRegionPass : public IUtilityRenderPass
{
public:
	CStretchRegionPass() {};
	~CStretchRegionPass() {};

	void Execute(CTexture* pSrcRT, CTexture* pDestRT, const RECT *pSrcRect = NULL, const RECT *pDstRect = NULL, bool bBigDownsample = false, const ColorF& color = ColorF(1,1,1,1), i32k renderStateFlags = 0);
	bool PreparePrimitive(CRenderPrimitive& prim, CPrimitiveRenderPass& targetPass, const RECT& rcS, i32 renderState, const D3DViewPort& targetViewport, bool bResample, bool bBigDownsample, CTexture *pSrcRT, CTexture *pDstRT, const ColorF& color);

	static CStretchRegionPass &GetPass();
	static void Shutdown();

	static EPassId GetPassId() { return EPassId::StretchRegionPass; }

protected:
	CPrimitiveRenderPass m_pass;

	CRenderPrimitive m_Primitive;

private:
	static CStretchRegionPass *s_pPass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSharpeningUpsamplePass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pSrcRT, CTexture* pDestRT);

	static EPassId GetPassId() { return EPassId::SharpeningUpsamplePass; }

private:
	CFullscreenPass m_pass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CNearestDepthUpsamplePass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pOrgDS, CTexture* pSrcRT, CTexture* pSrcDS, CTexture* pDestRT, bool bAlphaBased = false);

	static EPassId GetPassId() { return EPassId::NearestDepthUpsamplePass; }

private:
	CFullscreenPass m_pass[2];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDownsamplePass : public IUtilityRenderPass
{
public:
	enum EFilterType
	{
		FilterType_Box,
		FilterType_Tent,
		FilterType_Gauss,
		FilterType_Lanczos,
	};

	void Execute(CTexture* pSrcRT, CTexture* pDestRT, i32 nSrcW, i32 nSrcH, i32 nDstW, i32 nDstH, EFilterType eFilter);

	static EPassId GetPassId() { return EPassId::DownsamplePass; }

private:
	CFullscreenPass m_pass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStableDownsamplePass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pSrcRT, CTexture* pDestRT, bool bKillFireflies);

	static EPassId GetPassId() { return EPassId::StableDownsamplePass; }

private:
	CFullscreenPass m_pass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDepthDownsamplePass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pSrcRT, CTexture* pDestRT, bool bLinearizeSrcDepth, bool bFromSingleChannel);

	static EPassId GetPassId() { return EPassId::DepthDownsamplePass; }

private:
	CFullscreenPass m_pass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CGaussianBlurPass : public IUtilityRenderPass
{
public:
	CGaussianBlurPass()
		: m_scale(-FLT_MIN)
		, m_distribution(-FLT_MIN)
	{
	}
	void Execute(CTexture* pScrDestRT, CTexture* pTempRT, float scale, float distribution, bool bAlphaOnly = false);

	static EPassId GetPassId() { return EPassId::GaussianBlurPass; }

protected:
	float GaussianDistribution1D(float x, float rho);
	void  ComputeParams(i32 texWidth, i32 texHeight, i32 numSamples, float scale, float distribution);

protected:
	float           m_scale;
	float           m_distribution;
	Vec4            m_paramsH[16];
	Vec4            m_paramsV[16];
	Vec4            m_weights[16];

	CFullscreenPass m_passH;
	CFullscreenPass m_passV;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMipmapGenPass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pScrDestRT, i32 mipCount = 0);

	static EPassId GetPassId() { return EPassId::MipmapGenPass; }

protected:
	std::array<CFullscreenPass, 13> m_downsamplePasses;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CClearSurfacePass : public IUtilityRenderPass
{
public:
	static void Execute(const CTexture* pDepthTex, i32k nFlags, const float cDepth, u8k cStencil);
	static void Execute(const CTexture* pColorTex, const ColorF& cClear);
	static void Execute(const CGpuBuffer* pBuf, const ColorF& cClear);
	static void Execute(const CGpuBuffer* pBuf, const ColorI& cClear);

	static EPassId GetPassId() { return EPassId::ClearSurfacePass; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CClearRegionPass : public CClearSurfacePass
{
public:
	CClearRegionPass();
	virtual ~CClearRegionPass();

	void Execute(CTexture* pDepthTex, i32k nFlags, const float cDepth, u8k cStencil, const uint numRects, const RECT* pRects);
	void Execute(CTexture* pColorTex, const ColorF& cClear, const uint numRects, const RECT* pRects);
	void Execute(CGpuBuffer* pBuf, const ColorF& cClear, const uint numRanges, const RECT* pRanges);
	void Execute(CGpuBuffer* pBuf, const ColorI& cClear, const uint numRanges, const RECT* pRanges);

	static EPassId GetPassId() { return EPassId::ClearRegionPass; }

protected:
	bool PreparePrimitive(CRenderPrimitive& prim, i32 renderState, i32 stencilState, const ColorF& cClear, float cDepth, i32 stencilRef, const RECT& rect, const D3DViewPort& targetViewport);

	CPrimitiveRenderPass          m_clearPass;
	std::vector<CRenderPrimitive> m_clearPrimitives;
	buffer_handle_t               m_quadVertices;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAnisotropicVerticalBlurPass : public IUtilityRenderPass
{
public:
	void Execute(CTexture* pTex, i32 nAmount, float fScale, float fDistribution, bool bAlphaOnly);

	static EPassId GetPassId() { return EPassId::AnisotropicVerticalBlurPass; }

private:
	CFullscreenPass m_passBlurAnisotropicVertical[2];
};
