// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLDevice.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrapper for ID3D11Device
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLDEVICE__
#define __DRXDXGLDEVICE__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLGIObject.hpp>

namespace NDrxOpenGL
{
class CDevice;
struct SDummyContext;
}

class CDrxDXGLGIAdapter;
class CDrxDXGLDeviceContext;

class CDrxDXGLDevice
#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT && !DXGL_FULL_EMULATION
	: public ID3D11Device
#else
	: public CDrxDXGLGIObject
#endif
{
public:
#if DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLDevice, DXGIDevice)
#endif //DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLDevice, D3D11Device)

	CDrxDXGLDevice(CDrxDXGLGIAdapter* pAdapter, D3D_FEATURE_LEVEL eFeatureLevel);
	virtual ~CDrxDXGLDevice();

	bool                 Initialize(const DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
	NDrxOpenGL::CDevice* GetGLDevice();

	// IDXGIObject overrides
	HRESULT GetParent(REFIID riid, uk * ppParent);

	// IDXGIDevice implementation
	HRESULT STDMETHODCALLTYPE GetAdapter(IDXGIAdapter** pAdapter);
	HRESULT STDMETHODCALLTYPE CreateSurface(const DXGI_SURFACE_DESC* pDesc, UINT NumSurfaces, DXGI_USAGE Usage, const DXGI_SHARED_RESOURCE* pSharedResource, IDXGISurface** ppSurface);
	HRESULT STDMETHODCALLTYPE QueryResourceResidency(IUnknown* const* ppResources, DXGI_RESIDENCY* pResidencyStatus, UINT NumResources);
	HRESULT STDMETHODCALLTYPE SetGPUThreadPriority(INT Priority);
	HRESULT STDMETHODCALLTYPE GetGPUThreadPriority(INT* pPriority);

	// ID3D11Device implementation
	HRESULT STDMETHODCALLTYPE           CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer);
	HRESULT STDMETHODCALLTYPE           CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D);
	HRESULT STDMETHODCALLTYPE           CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D);
	HRESULT STDMETHODCALLTYPE           CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D);
	HRESULT STDMETHODCALLTYPE           CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView);
	HRESULT STDMETHODCALLTYPE           CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView);
	HRESULT STDMETHODCALLTYPE           CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView);
	HRESULT STDMETHODCALLTYPE           CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView);
	HRESULT STDMETHODCALLTYPE           CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, ukk pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout);
	HRESULT STDMETHODCALLTYPE           CreateVertexShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader);
	HRESULT STDMETHODCALLTYPE           CreateGeometryShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader);
	HRESULT STDMETHODCALLTYPE           CreateGeometryShaderWithStreamOutput(ukk pShaderBytecode, SIZE_T BytecodeLength, const D3D11_SO_DECLARATION_ENTRY* pSODeclaration, UINT NumEntries, const UINT* pBufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader);
	HRESULT STDMETHODCALLTYPE           CreatePixelShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader);
	HRESULT STDMETHODCALLTYPE           CreateHullShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader);
	HRESULT STDMETHODCALLTYPE           CreateDomainShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader);
	HRESULT STDMETHODCALLTYPE           CreateComputeShader(ukk pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader);
	HRESULT STDMETHODCALLTYPE           CreateClassLinkage(ID3D11ClassLinkage** ppLinkage);
	HRESULT STDMETHODCALLTYPE           CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState);
	HRESULT STDMETHODCALLTYPE           CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState);
	HRESULT STDMETHODCALLTYPE           CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState);
	HRESULT STDMETHODCALLTYPE           CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);
	HRESULT STDMETHODCALLTYPE           CreateQuery(const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery);
	HRESULT STDMETHODCALLTYPE           CreatePredicate(const D3D11_QUERY_DESC* pPredicateDesc, ID3D11Predicate** ppPredicate);
	HRESULT STDMETHODCALLTYPE           CreateCounter(const D3D11_COUNTER_DESC* pCounterDesc, ID3D11Counter** ppCounter);
	HRESULT STDMETHODCALLTYPE           CreateDeferredContext(UINT ContextFlags, ID3D11DeviceContext** ppDeferredContext);
	HRESULT STDMETHODCALLTYPE           OpenSharedResource(HANDLE hResource, REFIID ReturnedInterface, uk * ppResource);
	HRESULT STDMETHODCALLTYPE           CheckFormatSupport(DXGI_FORMAT Format, UINT* pFormatSupport);
	HRESULT STDMETHODCALLTYPE           CheckMultisampleQualityLevels(DXGI_FORMAT Format, UINT SampleCount, UINT* pNumQualityLevels);
	void STDMETHODCALLTYPE              CheckCounterInfo(D3D11_COUNTER_INFO* pCounterInfo);
	HRESULT STDMETHODCALLTYPE           CheckCounter(const D3D11_COUNTER_DESC* pDesc, D3D11_COUNTER_TYPE* pType, UINT* pActiveCounters, LPSTR szName, UINT* pNameLength, LPSTR szUnits, UINT* pUnitsLength, LPSTR szDescription, UINT* pDescriptionLength);
	HRESULT STDMETHODCALLTYPE           CheckFeatureSupport(D3D11_FEATURE Feature, uk pFeatureSupportData, UINT FeatureSupportDataSize);
#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT && !DXGL_FULL_EMULATION
	HRESULT STDMETHODCALLTYPE           GetPrivateData(REFGUID guid, UINT* pDataSize, uk pData);
	HRESULT STDMETHODCALLTYPE           SetPrivateData(REFGUID guid, UINT DataSize, ukk pData);
	HRESULT STDMETHODCALLTYPE           SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);
#endif
	D3D_FEATURE_LEVEL STDMETHODCALLTYPE GetFeatureLevel(void);
	UINT STDMETHODCALLTYPE              GetCreationFlags(void);
	HRESULT STDMETHODCALLTYPE           GetDeviceRemovedReason(void);
	void STDMETHODCALLTYPE              GetImmediateContext(ID3D11DeviceContext** ppImmediateContext);
	HRESULT STDMETHODCALLTYPE           SetExceptionMode(UINT RaiseFlags);
	UINT STDMETHODCALLTYPE              GetExceptionMode(void);

#if !DXGL_FULL_EMULATION
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, uk * ppvObject);
#endif //!DXGL_FULL_EMULATION
protected:
#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT && !DXGL_FULL_EMULATION
	CDrxDXGLPrivateDataContainer m_kPrivateDataContainer;
#endif
	_smart_ptr<CDrxDXGLGIAdapter> m_spAdapter;
	_smart_ptr<NDrxOpenGL::CDevice>   m_spGLDevice;
	_smart_ptr<CDrxDXGLDeviceContext> m_spImmediateContext;
	D3D_FEATURE_LEVEL                 m_eFeatureLevel;
#if DXGL_FULL_EMULATION
	NDrxOpenGL::SDummyContext*        m_pDummyContext;
#endif //DXGL_FULL_EMULATION
};

#endif //__DRXDXGLDEVICE__
