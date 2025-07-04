// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/FullscreenPass.h>
#include <drx3D/Render/UtilityPasses.h>

class CWaterRipplesStage : public CGraphicsPipelineStage
{
public:
	CWaterRipplesStage();
	~CWaterRipplesStage();

	void Init() final;
	void Update() final;

	void Execute();

	Vec4 GetWaterRippleLookupParam() const
	{
		return m_lookupParam;
	}

	CTexture* GetWaterRippleTex() const;

	bool IsVisible() const;

private:
	bool RefreshParameters();
	void ExecuteWaterRipples(CTexture* pTargetTex, const D3DViewPort& viewport);
	void UpdateAndDrawDebugInfo();

private:
	static i32k                         sVertexCount = 4;
	static i32k                         sTotalVertexCount = sVertexCount * SWaterRippleInfo::MaxWaterRipplesInScene;
	static const EDefaultInputLayouts::PreDefs sVertexFormat = EDefaultInputLayouts::P3F_C4B_T2F;
	static const size_t                        sVertexStride = sizeof(SVF_P3F_C4B_T2F);

	typedef SVF_P3F_C4B_T2F SVertex;

	struct SWaterRippleRecord
	{
		SWaterRippleInfo info;
		float            lifetime;

		SWaterRippleRecord(const SWaterRippleInfo& srcInfo, float srcLifetime)
			: info(srcInfo)
			, lifetime(srcLifetime)
		{}
	};

	struct SWaterRippleConstants
	{
		Vec4 params;
	};

private:
	_smart_ptr<CTexture>          m_pTexWaterRipplesDDN; // xy: wave propagation normals, z: frame t-2, w: frame t-1
	_smart_ptr<CTexture>          m_pTempTexture;

	CTypedConstantBuffer<SWaterRippleConstants, 256> m_constants;

	CFullscreenPass               m_passSnapToCenter;
	CStretchRectPass              m_passCopy;
	CFullscreenPass               m_passWaterWavePropagation;
	CPrimitiveRenderPass          m_passAddWaterRipples;
	CMipmapGenPass                m_passMipmapGen;

	CRenderPrimitive              m_ripplePrimitive[SWaterRippleInfo::MaxWaterRipplesInScene];
	buffer_handle_t               m_vertexBuffer; // stored all ripples' vertices.

	SResourceRegionMapping        m_TempCopyParams;

	ICVar*                        m_pCVarWaterRipplesDebug;

	CDrxNameTSCRC                 m_ripplesGenTechName;
	CDrxNameTSCRC                 m_ripplesHitTechName;

	i32                         m_frameID;

	float                         m_lastSpawnTime;
	float                         m_lastUpdateTime;

	float                         m_simGridSnapRange;
	Vec2                          m_simOrigin;

	i32                         m_updateMask;
	Vec4                          m_shaderParam;
	Vec4                          m_lookupParam;

	bool                          m_bInitializeSim;
	bool                          m_bSnapToCenter;

	std::vector<SWaterRippleInfo> m_waterRipples;
	std::vector<SWaterRippleInfo> m_waterRipplesMGPU;

#if !defined(_RELEASE)
	std::vector<SWaterRippleRecord> m_debugRippleInfos;
#endif

	static constexpr i32 nGridSize = 256;
};
