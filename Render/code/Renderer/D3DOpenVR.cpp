// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>

#if defined(INCLUDE_VR_RENDERING)

	#include <drx3D/Render/D3DOpenVR.h>
	#include <drx3D/Render/DriverD3D.h>
	#include <drx3D/Render/D3DPostProcess.h>
	#include <drx3D/Render/DeviceInfo.h>

	#include <drx3D/Render/RenderDisplayContext.h>

	#include <DinrusXSys/VR/IHMDUpr.h>
	#include <DinrusXSys/VR/IHMDDevice.h>
	#ifdef ENABLE_BENCHMARK_SENSOR
		#include <IBenchmarkFramework.h>
		#include <IBenchmarkRendererSensorUpr.h>
	#endif

CD3DOpenVRRenderer::CD3DOpenVRRenderer(DrxVR::OpenVR::IOpenVRDevice* openVRDevice, CD3D9Renderer* renderer, CD3DStereoRenderer* stereoRenderer)
	: m_pOpenVRDevice(openVRDevice)
	, m_pRenderer(renderer)
	, m_pStereoRenderer(stereoRenderer)
	, m_eyeWidth(~0L)
	, m_eyeHeight(~0L)
	, m_numFrames(0)
	, m_currentFrame(0)
{
	ZeroArray(m_scene3DRenderData);
	ZeroArray(m_quadLayerRenderData);
	ZeroArray(m_mirrorTextures);

	for (u32 i = RenderLayer::eQuadLayers_0; i < RenderLayer::eQuadLayers_Headlocked_0; ++i)
	{
		m_quadLayerProperties[i].SetType(RenderLayer::eLayer_Quad);
		m_quadLayerProperties[i].SetPose(QuatTS(Quat(IDENTITY), Vec3(0.f, 0.f, -0.8f), 1.f));
		m_quadLayerProperties[i].SetId(i);
	}
	for (u32 i = RenderLayer::eQuadLayers_Headlocked_0; i < RenderLayer::eQuadLayers_Total; ++i)
	{
		m_quadLayerProperties[i].SetType(RenderLayer::eLayer_Quad_HeadLocked);
		m_quadLayerProperties[i].SetPose(QuatTS(Quat(IDENTITY), Vec3(0.f, 0.f, -0.8f), 1.f));
		m_quadLayerProperties[i].SetId(i);
	}
}

bool CD3DOpenVRRenderer::Initialize(i32 initialWidth, i32 initialeight)
{
	D3DDevice* d3d11Device = m_pRenderer->GetDevice_Unsynchronized().GetRealDevice();

#if DRX_RENDERER_DIRECT3D >= 120
	ID3D12CommandQueue* pD3d12Queue = GetDeviceObjectFactory().GetNativeCoreCommandQueue();
	const auto type = DrxVR::OpenVR::ERenderAPI::eRenderAPI_DirectX12;
#elif DRX_RENDERER_DIRECT3D >= 110
	const auto type = DrxVR::OpenVR::ERenderAPI::eRenderAPI_DirectX;
#else
	DRX_ASSERT(false, "Unsuported");
#endif

	m_eyeWidth  = initialWidth;
	m_eyeHeight = initialeight;

	DrxVR::OpenVR::TextureDesc eyeTextureDesc;
	eyeTextureDesc.width = m_eyeWidth;
	eyeTextureDesc.height = m_eyeHeight;
	eyeTextureDesc.format = (u32)DXGI_FORMAT_R8G8B8A8_UNORM;

	DrxVR::OpenVR::TextureDesc quadTextureDesc;
	quadTextureDesc.width = m_eyeWidth;
	quadTextureDesc.height = m_eyeHeight;
	quadTextureDesc.format = (u32)DXGI_FORMAT_R8G8B8A8_UNORM;

	DrxVR::OpenVR::TextureDesc mirrorTextureDesc;
	mirrorTextureDesc.width = m_eyeWidth;
	mirrorTextureDesc.height = m_eyeHeight;
	mirrorTextureDesc.format = (u32)DXGI_FORMAT_R8G8B8A8_UNORM;

	if (!InitializeEyeTarget(d3d11Device, EEyeType::eEyeType_LeftEye, eyeTextureDesc, "$LeftEye") ||
	    !InitializeEyeTarget(d3d11Device, EEyeType::eEyeType_RightEye, eyeTextureDesc, "$RightEye") ||
	    !InitializeQuadLayer(d3d11Device, RenderLayer::eQuadLayers_0, quadTextureDesc, "$QuadLayer0") ||
		!InitializeQuadLayer(d3d11Device, RenderLayer::eQuadLayers_Headlocked_0, quadTextureDesc, "$QuadLayerHeadLocked0_") ||
		!InitializeQuadLayer(d3d11Device, RenderLayer::eQuadLayers_Headlocked_1, quadTextureDesc, "$QuadLayerHeadLocked1_") ||
	    !InitializeMirrorTexture(d3d11Device, EEyeType::eEyeType_LeftEye, mirrorTextureDesc, "$LeftMirror") ||
	    !InitializeMirrorTexture(d3d11Device, EEyeType::eEyeType_RightEye, mirrorTextureDesc, "$RightMirror"))
	{
		DrxLogAlways("[HMD][OpenVR] Texture creation failed");
		Shutdown();
		return false;
	}

	// Create display contexts for eyes
	for (u32 eye = 0; eye < eEyeType_NumEyes; ++eye)
	{
		std::vector<_smart_ptr<CTexture>> swapChain = { m_scene3DRenderData[eye].texture };
		m_pStereoRenderer->CreateEyeDisplayContext(CCamera::EEye(eye), std::move(swapChain));
	}
	// Create display contexts for quad layers
	for (u32 quad = RenderLayer::eQuadLayers_0; quad < RenderLayer::eQuadLayers_Total; ++quad)
	{
		std::vector<_smart_ptr<CTexture>> swapChain = { m_quadLayerRenderData[quad].texture };
		m_pStereoRenderer->CreateVrQuadLayerDisplayContext(RenderLayer::EQuadLayers(quad), std::move(swapChain));
	}

#if DRX_RENDERER_DIRECT3D >= 120
	// Transition
	{
		// Transition resources
		NDrxDX12::CCommandList* pCL = ((CDrxDX12Device*)d3d11Device)->GetDeviceContext()->GetCoreGraphicsCommandList();

		CDrxDX12RenderTargetView* lRV = (CDrxDX12RenderTargetView*)m_scene3DRenderData[EEyeType::eEyeType_LeftEye].texture->GetSurface(-1, 0);
		CDrxDX12RenderTargetView* rRV = (CDrxDX12RenderTargetView*)m_scene3DRenderData[EEyeType::eEyeType_RightEye].texture->GetSurface(-1, 0);

		NDrxDX12::CView& lV = lRV->GetDX12View();
		NDrxDX12::CView& rV = rRV->GetDX12View();

		lV.GetDX12Resource().TransitionBarrier(pCL, lV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		rV.GetDX12Resource().TransitionBarrier(pCL, rV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		((CDrxDX12Device*)d3d11Device)->GetDeviceContext()->Finish();
	}

	uk leftTexture = m_pOpenVRDevice->GetD3D12EyeTextureData(EEyeType::eEyeType_LeftEye,
		m_scene3DRenderData[EEyeType::eEyeType_LeftEye].texture->GetDevTexture()->Get2DTexture()->GetDX12Resource().GetD3D12Resource(),
		pD3d12Queue);
	uk rightTexture = m_pOpenVRDevice->GetD3D12EyeTextureData(EEyeType::eEyeType_RightEye,
		m_scene3DRenderData[EEyeType::eEyeType_RightEye].texture->GetDevTexture()->Get2DTexture()->GetDX12Resource().GetD3D12Resource(),
		pD3d12Queue);
#elif DRX_RENDERER_DIRECT3D >= 110
	uk leftTexture = m_scene3DRenderData[EEyeType::eEyeType_LeftEye].texture->GetDevTexture()->Get2DTexture();
	uk rightTexture = m_scene3DRenderData[EEyeType::eEyeType_RightEye].texture->GetDevTexture()->Get2DTexture();
#endif

	// Scene3D layers
	m_pOpenVRDevice->OnSetupEyeTargets(
		type,
		DrxVR::OpenVR::ERenderColorSpace::eRenderColorSpace_Auto,
		leftTexture,
		rightTexture
	);

	// Quad layers
	for (i32 i = RenderLayer::eQuadLayers_0; i < RenderLayer::eQuadLayers_Total; ++i)
	{
#if DRX_RENDERER_DIRECT3D >= 120
		// Transition
		{
			// Transition resources
			NDrxDX12::CCommandList* pCL = ((CDrxDX12Device*)d3d11Device)->GetDeviceContext()->GetCoreGraphicsCommandList();

			CDrxDX12RenderTargetView* s = (CDrxDX12RenderTargetView*)m_quadLayerRenderData[i].texture->GetSurface(-1, 0);
			NDrxDX12::CView& v = s->GetDX12View();
			v.GetDX12Resource().TransitionBarrier(pCL, v, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			((CDrxDX12Device*)d3d11Device)->GetDeviceContext()->Finish();
		}

		const auto texture = m_pOpenVRDevice->GetD3D12QuadTextureData(i, 
			m_quadLayerRenderData[i].texture->GetDevTexture()->Get2DTexture()->GetDX12Resource().GetD3D12Resource(),
			pD3d12Queue);
#elif DRX_RENDERER_DIRECT3D >= 110
		const auto texture = m_quadLayerRenderData[i].texture->GetDevTexture()->Get2DTexture();
#endif

		const bool headLocked = i >= RenderLayer::eQuadLayers_Headlocked_0;
		const bool higestQuality = i == RenderLayer::eQuadLayers_Headlocked_1;
		m_pOpenVRDevice->OnSetupOverlay(i,
			type,
			DrxVR::OpenVR::ERenderColorSpace::eRenderColorSpace_Auto,
			texture,
			GetQuadLayerProperties(RenderLayer::EQuadLayers(i))->GetPose(),
			!headLocked,
			higestQuality
		);
	}

	// Mirror texture
	uk srv = nullptr;
	for (u32 eye = 0; eye < 2; ++eye)
	{
#if DRX_RENDERER_DIRECT3D >= 120
		// No mirror textures in D3D12
		// Request the resource-view from openVR
#elif DRX_RENDERER_DIRECT3D >= 110
		m_pOpenVRDevice->GetMirrorImageView(static_cast<EEyeType>(eye), m_mirrorTextures[eye]->GetDevTexture()->Get2DTexture(), &srv);
		m_mirrorTextures[eye]->SetDefaultShaderResourceView(static_cast<D3DShaderResource*>(srv), false);
#endif
	}

	return true;
}

bool CD3DOpenVRRenderer::InitializeEyeTarget(D3DDevice* d3d11Device, EEyeType eye, DrxVR::OpenVR::TextureDesc desc, tukk name)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = desc.width;
	textureDesc.Height = desc.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = (DXGI_FORMAT)desc.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* texture;
	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);

	m_scene3DRenderData[eye].texture = m_pStereoRenderer->WrapD3DRenderTarget(static_cast<D3DTexture*>(texture), desc.width, desc.height, (DXGI_FORMAT)desc.format, name, true);

	return true;
}

bool CD3DOpenVRRenderer::InitializeQuadLayer(D3DDevice* d3d11Device, RenderLayer::EQuadLayers quadLayer, DrxVR::OpenVR::TextureDesc desc, tukk name)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = desc.width;
	textureDesc.Height = desc.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = (DXGI_FORMAT)desc.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	ID3D11Texture2D* texture;
	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);

	char textureName[16];
	drx_sprintf(textureName, name, quadLayer);

	m_quadLayerRenderData[quadLayer].texture = m_pStereoRenderer->WrapD3DRenderTarget(static_cast<D3DTexture*>(texture), desc.width, desc.height, (DXGI_FORMAT)desc.format, textureName, true);

	return true;
}

bool CD3DOpenVRRenderer::InitializeMirrorTexture(D3DDevice* d3d11Device, EEyeType eye, DrxVR::OpenVR::TextureDesc desc, tukk name)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = desc.width;
	textureDesc.Height = desc.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = (DXGI_FORMAT)desc.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* texture;
	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);

	m_mirrorTextures[eye] = m_pStereoRenderer->WrapD3DRenderTarget(static_cast<D3DTexture*>(texture), desc.width, desc.height, (DXGI_FORMAT)desc.format, name, false);
	
	return true;
}

void CD3DOpenVRRenderer::Shutdown()
{
// 	m_pStereoRenderer->SetEyeTextures(nullptr, nullptr);

	// Scene3D layers
	for (u32 eye = 0; eye < 2; ++eye)
	{
		m_scene3DRenderData[eye].texture = nullptr;
	}

	// Quad layers
	for (u32 i = 0; i < RenderLayer::eQuadLayers_Total; ++i)
	{
		m_pOpenVRDevice->OnDeleteOverlay(i);
		m_quadLayerRenderData[i].texture = nullptr;
	}

	// Mirror texture
	for (u32 eye = 0; eye < 2; ++eye)
	{
		if (m_mirrorTextures[eye])
			m_mirrorTextures[eye] = nullptr;
	}
	
	ReleaseBuffers();
}

void CD3DOpenVRRenderer::OnResolutionChanged(i32 newWidth, i32 newHeight)
{
	if (m_eyeWidth  != newWidth ||
	    m_eyeHeight != newHeight)
	{
		Shutdown();
		Initialize(newWidth, newHeight);
	}
}

void CD3DOpenVRRenderer::PrepareFrame(uint64_t frameId)
{
	m_pOpenVRDevice->OnPrepare();
}

void CD3DOpenVRRenderer::SubmitFrame()
{
#ifdef ENABLE_BENCHMARK_SENSOR
	gcpRendD3D->m_benchmarkRendererSensor->PreStereoFrameSubmit(m_scene3DRenderData[0].texture, m_scene3DRenderData[1].texture);
#endif

#if (DRX_RENDERER_DIRECT3D >= 120)
	{
		// Transition resources
		ID3D11Device* pD3d11Device = m_pRenderer->GetDevice_Unsynchronized().GetRealDevice();
		NDrxDX12::CCommandList* pCL = ((CDrxDX12Device*)pD3d11Device)->GetDeviceContext()->GetCoreGraphicsCommandList();

		CDrxDX12RenderTargetView* lRV = (CDrxDX12RenderTargetView*)m_scene3DRenderData[EEyeType::eEyeType_LeftEye].texture->GetSurface(-1, 0);
		CDrxDX12RenderTargetView* rRV = (CDrxDX12RenderTargetView*)m_scene3DRenderData[EEyeType::eEyeType_RightEye].texture->GetSurface(-1, 0);

		NDrxDX12::CView& lV = lRV->GetDX12View();
		NDrxDX12::CView& rV = rRV->GetDX12View();
		lV.GetDX12Resource().TransitionBarrier(pCL, lV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		rV.GetDX12Resource().TransitionBarrier(pCL, rV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		// Quad layers
		for (u32 i = 0; i < RenderLayer::eQuadLayers_Total; ++i)
		{
			const RenderLayer::CProperties* pQuadProperties = GetQuadLayerProperties(static_cast<RenderLayer::EQuadLayers>(i));
			if (!pQuadProperties->IsActive())
				continue;

			if (m_quadLayerRenderData[i].texture)
			{
				CDrxDX12RenderTargetView* qRV = (CDrxDX12RenderTargetView*)m_quadLayerRenderData[i].texture->GetSurface(-1, 0);
				if (qRV)
				{
					NDrxDX12::CView& qV = qRV->GetDX12View();
					qV.GetDX12Resource().TransitionBarrier(pCL, qV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}
		}

		((CDrxDX12Device*)pD3d11Device)->GetDeviceContext()->Finish();
	}
#endif

	// Quad layers
	for (u32 i = 0; i < RenderLayer::eQuadLayers_Total; ++i)
	{
		RenderLayer::CProperties* pQuadProperties = GetQuadLayerProperties(static_cast<RenderLayer::EQuadLayers>(i));
		if (pQuadProperties->IsActive())
		{
			m_pOpenVRDevice->SubmitOverlay(i, pQuadProperties);
			pQuadProperties->SetActive(false);
		}
	}

	// Pass the final images and layer configuration to the OpenVR device
	m_pOpenVRDevice->SubmitFrame();

#ifdef ENABLE_BENCHMARK_SENSOR
	gcpRendD3D->m_benchmarkRendererSensor->AfterStereoFrameSubmit();
#endif
}

void CD3DOpenVRRenderer::OnPostPresent()
{
	m_pOpenVRDevice->OnPostPresent();
}

RenderLayer::CProperties* CD3DOpenVRRenderer::GetQuadLayerProperties(RenderLayer::EQuadLayers id)
{
	if (id < RenderLayer::eQuadLayers_Total)
		return &(m_quadLayerProperties[id]);
	return nullptr;
}

std::pair<CTexture*, Vec4> CD3DOpenVRRenderer::GetMirrorTexture(EEyeType eye) const
{
	return std::make_pair(m_mirrorTextures[eye], Vec4(0,0,1,1));
}

#endif //defined(INCLUDE_VR_RENDERING)
