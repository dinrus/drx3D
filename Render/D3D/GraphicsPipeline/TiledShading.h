// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/ComputeRenderPass.h>
#include <drx3D/Render/FullscreenPass.h>

class CTiledShadingStage : public CGraphicsPipelineStage
{
public:
	CTiledShadingStage();
	~CTiledShadingStage();

	void Init();
	void Execute();

private:
	enum EVolumeTypes
	{
		eVolumeType_Box,
		eVolumeType_Sphere,
		eVolumeType_Cone,

		eVolumeType_Count
	};
	
	struct SVolumeGeometry
	{
		CGpuBuffer       vertexDataBuf;
		buffer_handle_t  vertexBuffer;
		buffer_handle_t  indexBuffer;
		u32           numVertices;
		u32           numIndices;
	};

private:
	CGpuBuffer            m_lightVolumeInfoBuf;

	_smart_ptr<CTexture>  m_pTexGiDiff;
	_smart_ptr<CTexture>  m_pTexGiSpec;

	SVolumeGeometry       m_volumeMeshes[eVolumeType_Count];

	CConstantBufferPtr    m_pPerViewConstantBuffer = nullptr;
	CComputeRenderPass    m_passCullingShading;
	
	CFullscreenPass       m_passCopyDepth;
	CPrimitiveRenderPass  m_passLightVolumes;
	CRenderPrimitive      m_volumePasses[eVolumeType_Count * 2];  // Inside and outside of volume for each type
	u32                m_numVolumesPerPass[eVolumeType_Count * 2];
};
