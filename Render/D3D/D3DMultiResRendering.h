// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _MULTIRESRENDERING_H_
#define _MULTIRESRENDERING_H_

#ifdef USE_NV_API
	#include <NVIDIA/multiprojection_dx_2.0/nv_vr.h>
#endif

#include <drx3D/Render/D3D/GraphicsPipeline/Common/FullscreenPass.h>

class CD3D9Renderer;
class CDeviceGraphicsCommandInterface;
struct D3D11_VIEWPORT;

class CVrProjectionUpr
{
public:
	enum EVrProjection
	{
		eVrProjection_Planar,
		eVrProjection_MultiRes,
		eVrProjection_LensMatched
	};

public:
	static void Init(CD3D9Renderer* pRenderer);
	static void Reset();

	static CVrProjectionUpr* Instance() { return m_pInstance; }
	static bool IsMultiResEnabledStatic();

	bool IsMultiResEnabled() const;
	bool IsProjectionConfigured() const;
	void Configure(const SRenderViewport& originalViewport, bool bMirrored);
	
	// creates a cached configuration and prepares the constant buffer for use
	void PrepareProjectionParameters(CDeviceCommandListRef RESTRICT_REFERENCE commandList, const D3D11_VIEWPORT& viewport);
	// returns false if the caller needs to do SetViewports on its own
	bool SetRenderingState(CDeviceCommandListRef RESTRICT_REFERENCE commandList, const D3D11_VIEWPORT& viewport, bool bSetViewports, bool bBindConstantBuffer);
	// removes the ModifiedW state in case of LensMatched projection, otherwise does nothing
	void RestoreState(CDeviceCommandListRef RESTRICT_REFERENCE commandList);

	void MapScreenPosToMultiRes(float& x, float& y) const;

	uint64    GetRTFlags() const;
	CTexture* GetZTargetFlattened() const { return m_ptexZTargetFlattened; }

	EVrProjection GetProjectionType() const { return m_projection; }
	void GetProjectionSize(i32 flattenedWidth, i32 flattenedHeight, i32& projectionWidth, i32& projectionHeight);
	CConstantBufferPtr GetProjectionConstantBuffer(i32 flattenedWidth, i32 flattenedHeight);

	void ExecuteFlattenDepth(CTexture* pSrcRT, CTexture* pDestRT);
	void ExecuteLensMatchedOctagon(CTexture* pDestRT);

private:
	CVrProjectionUpr(CD3D9Renderer* const pRenderer);

	CD3D9Renderer*                      m_pRenderer;
	bool                                m_isConfigured;
	bool                                m_multiResSupported;
	bool                                m_lensMatchedSupported;
	bool                                m_currentConfigMirrored;
	i32                                 m_currentPreset;

	EVrProjection                       m_projection;

	CFullscreenPass                     m_passDepthFlattening;
	CRenderPrimitive                    m_primitiveLensMatchedOctagon;
	CPrimitiveRenderPass                m_passLensMatchedOctagon;
	_smart_ptr<CTexture>                m_ptexZTargetFlattened;

	static CVrProjectionUpr*        m_pInstance;
	

#if defined(USE_NV_API)
	struct VrProjectionInfo
	{
		bool bMirrored;
		Nv::VR::Data data;
		CConstantBufferPtr pConstantBuffer;
	};

	void              GetDerivedData(float width, float height, Nv::VR::Data* pOutData, bool bMirrored = false) const;
	VrProjectionInfo& GetProjectionForViewport(const D3D11_VIEWPORT& viewport);

	Nv::VR::Planar::Configuration       m_planarConfig;
	Nv::VR::MultiRes::Configuration     m_multiResConfig;
	Nv::VR::LensMatched::Configuration  m_lensMatchedConfig;
	Nv::VR::Data                        m_data;
	Nv::VR::Data                        m_mirroredData;
	Nv::VR::Data                        m_planarData;
	Nv::VR::Viewport                    m_originalViewport;

	std::vector<VrProjectionInfo>       m_projectionCache;
#endif

};

#endif // _MULTIRESRENDERING_H_