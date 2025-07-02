// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if defined(INCLUDE_VR_RENDERING)

	#include<drx3D/Sys/IHMDDevice.h>
	#include <drx3D/DrxPlugins/VR/DrxOpenVR/Interface/IHmdOpenVRDevice.h>
	#include <drx3D/CoreX/Renderer/IStereoRenderer.h>

class CD3D9Renderer;

class CD3DOpenVRRenderer : public IHmdRenderer
{
public:
	CD3DOpenVRRenderer(DrxVR::OpenVR::IOpenVRDevice* openVRDevice, CD3D9Renderer* renderer, CD3DStereoRenderer* stereoRenderer);
	virtual ~CD3DOpenVRRenderer() = default;

	// IHDMRenderer
	virtual bool                      Initialize(i32 initialWidth, i32 initialeight) final;
	virtual void                      Shutdown() final;
	virtual void                      OnResolutionChanged(i32 newWidth, i32 newHeight) final;
	virtual void                      ReleaseBuffers() final {}
	virtual void                      PrepareFrame(uint64_t frameId) final;
	virtual void                      SubmitFrame() final;
	virtual void                      OnPostPresent() final;

	virtual RenderLayer::CProperties*  GetQuadLayerProperties(RenderLayer::EQuadLayers id) final;
	virtual RenderLayer::CProperties*  GetSceneLayerProperties(RenderLayer::ESceneLayers id) final { return nullptr; }
	virtual std::pair<CTexture*, Vec4> GetMirrorTexture(EEyeType eye) const final;
	// ~IHDMRenderer

protected:
	struct SSocialScreenRenderAutoRestore;

	struct Eye
	{
		_smart_ptr<CTexture> texture;
	};

	struct QuadLayer
	{
		_smart_ptr<CTexture> texture;
	};

	bool             InitializeEyeTarget(D3DDevice* d3dDevice, EEyeType eye, DrxVR::OpenVR::TextureDesc desc, tukk name);
	bool             InitializeQuadLayer(D3DDevice* d3dDevice, RenderLayer::EQuadLayers quadLayer, DrxVR::OpenVR::TextureDesc desc, tukk name);
	bool             InitializeMirrorTexture(D3DDevice* d3dDevice, EEyeType eye, DrxVR::OpenVR::TextureDesc desc, tukk name);

protected:
	_smart_ptr<CTexture>          m_mirrorTextures[EEyeType::eEyeType_NumEyes];
	Eye                           m_scene3DRenderData[EEyeType::eEyeType_NumEyes];
	QuadLayer                     m_quadLayerRenderData[RenderLayer::eQuadLayers_Total];
	RenderLayer::CProperties      m_quadLayerProperties[RenderLayer::eQuadLayers_Total];

	u32                        m_numFrames;
	u32                        m_currentFrame;

	u32                        m_eyeWidth;
	u32                        m_eyeHeight;

	DrxVR::OpenVR::IOpenVRDevice* m_pOpenVRDevice;
	CD3D9Renderer*                m_pRenderer;
	CD3DStereoRenderer*           m_pStereoRenderer;
};

#endif //defined(INCLUDE_VR_RENDERING)
