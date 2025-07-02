// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CRESKY_H__
#define __CRESKY_H__

#include <drx3D/CoreX/Renderer/VertexFormats.h>

struct SSkyLightRenderParams;

class CRESky : public CRenderElement
{
	friend class CRender3D;

public:

	float m_fTerrainWaterLevel;
	float m_fSkyBoxStretching;
	float m_fAlpha;
	i32   m_nSphereListId;

public:
	CRESky()
	{
		mfSetType(eDATA_Sky);
		mfUpdateFlags(FCEF_TRANSFORM);
		m_fTerrainWaterLevel = 0;
		m_fAlpha = 1;
		m_nSphereListId = 0;
		m_fSkyBoxStretching = 1.f;
	}

	virtual ~CRESky();

	virtual InputLayoutHandle GetVertexFormat() const override;
	virtual bool          GetGeometryInfo(SGeometryInfo& streams, bool bSupportTessellation = false) override;

	virtual bool          Compile(CRenderObject* pObj, CRenderView *pRenderView, bool updateInstanceDataOnly) override;
	virtual void          DrawToCommandList(CRenderObject* pObj, const struct SGraphicsPipelinePassContext& ctx, CDeviceCommandList* commandList) override;

	virtual void          GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

class CREHDRSky : public CRenderElement
{
	friend class CSceneForwardStage;

public:
	CREHDRSky()
		: m_pRenderParams(0)
		, m_skyDomeTextureLastTimeStamp(-1)
		, m_frameReset(0)
		, m_pSkyDomeTextureMie(0)
		, m_pStars(0)
		, m_pSkyDomeTextureRayleigh(0)
	{
		mfSetType(eDATA_HDRSky);
		mfUpdateFlags(FCEF_TRANSFORM);
		Init();
	}

	virtual ~CREHDRSky();

	virtual InputLayoutHandle GetVertexFormat() const override;
	virtual bool          GetGeometryInfo(SGeometryInfo& streams, bool bSupportTessellation = false) override;

	virtual bool          Compile(CRenderObject* pObj, CRenderView *pRenderView, bool updateInstanceDataOnly) override;
	virtual void          DrawToCommandList(CRenderObject* pObj, const struct SGraphicsPipelinePassContext& ctx, CDeviceCommandList* commandList) override;

	virtual void          GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	void GenerateSkyDomeTextures(i32 width, i32 height);

public:
	const SSkyLightRenderParams* m_pRenderParams;
	i32                          m_moonTexId;
	class CTexture*              m_pSkyDomeTextureMie;
	class CTexture*              m_pSkyDomeTextureRayleigh;

	//static void SetCommonMoonParams(CShader* ef, bool bUseMoon = false);

private:
	void Init();

private:
	i32           m_skyDomeTextureLastTimeStamp;
	i32           m_frameReset;
	class CStars* m_pStars;
};

#endif  // __CRESKY_H__
