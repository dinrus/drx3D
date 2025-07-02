// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifdef INCLUDE_VR_RENDERING

	#include<drx3D/Sys/IHMDDevice.h>
	#include <drx3D/DrxPlugins/VR/DrxOSVR/Interface/IHmdOSVRDevice.h>
	#include <drx3D/CoreX/Renderer/IStereoRenderer.h>

class CD3D9Renderer;
class CTexture;

class CD3DOsvrRenderer : public IHmdRenderer
{
public:

	CD3DOsvrRenderer(DrxVR::Osvr::IOsvrDevice* pDevice, CD3D9Renderer* pRenderer, CD3DStereoRenderer* pStereoRenderer);
	virtual ~CD3DOsvrRenderer();

	// IHDMRenderer implementation
	virtual bool                      Initialize(i32 initialWidth, i32 initialHeight) final;
	virtual void                      Shutdown() final;
	virtual void                      OnResolutionChanged(i32 newWidth, i32 newHeight) final;
	virtual void                      ReleaseBuffers() final {}
	virtual void                      PrepareFrame(uint64_t frameId) final;
	virtual void                      SubmitFrame() final;
	// TODO
	virtual RenderLayer::CProperties*  GetQuadLayerProperties(RenderLayer::EQuadLayers id) final   { return nullptr; }
	virtual RenderLayer::CProperties*  GetSceneLayerProperties(RenderLayer::ESceneLayers id) final { return nullptr; }
	virtual std::pair<CTexture*, Vec4> GetMirrorTexture(EEyeType eye) const final { return { nullptr, Vec4{} }; }
private:

	static u32k EyeCount = 2;

	struct EyeTextures
	{
		TArray<CTexture*> textures;
	};

	DrxVR::Osvr::IOsvrDevice* m_pOsvrDevice;
	CD3D9Renderer*            m_pRenderer;
	CD3DStereoRenderer*       m_pStereoRenderer;

	u32                    m_eyeWidth;
	u32                    m_eyeHeight;

	EyeTextures               m_scene3DRenderData[EyeCount];

	i32                       m_swapSetCount;
	i32                       m_currentFrame;

	void CreateTextureSwapSets(u32 width, u32 height, u32 swapSetCount);
	void ReleaseTextureSwapSets();

};

#endif
