// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxDXGL.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Contains the definitions of the global functions defined in
//               DrxDXGL
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/Interfaces/CDrxDXGLGIAdapter.hpp>
#include <drx3D/Render/Interfaces/CDrxDXGLBlob.hpp>
#include <drx3D/Render/Interfaces/CDrxDXGLDevice.hpp>
#include <drx3D/Render/Interfaces/CDrxDXGLShaderReflection.hpp>
#include <drx3D/Render/Interfaces/CDrxDXGLDeviceContext.hpp>
#include <drx3D/Render/Interfaces/CDrxDXGLGIFactory.hpp>
#include <drx3D/Render/Implementation/GLShader.hpp>
#include <drx3D/Render/Implementation/GLDevice.hpp>

template<typename Factory>
HRESULT CreateDXGIFactoryInternal(REFIID riid, uk * ppFactory)
{
	if (riid == __uuidof(Factory))
	{
		CDrxDXGLGIFactory* pFactory(new CDrxDXGLGIFactory());
		if (!pFactory->Initialize())
		{
			pFactory->Release();
			*ppFactory = NULL;
			return E_FAIL;
		}
		CDrxDXGLGIFactory::ToInterface(reinterpret_cast<IDXGIFactory**>(ppFactory), pFactory);
		return S_OK;
	}

	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////
//  Required global functions declared in D3D11.h and included headers
////////////////////////////////////////////////////////////////////////////

DXGL_API HRESULT WINAPI D3D11CreateDeviceAndSwapChain(
  IDXGIAdapter* pAdapter,
  D3D_DRIVER_TYPE DriverType,
  HMODULE Software,
  UINT Flags,
  CONST D3D_FEATURE_LEVEL* pFeatureLevels,
  UINT FeatureLevels,
  UINT SDKVersion,
  CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
  IDXGISwapChain** ppSwapChain,
  ID3D11Device** ppDevice,
  D3D_FEATURE_LEVEL* pFeatureLevel,
  ID3D11DeviceContext** ppImmediateContext)
{
	if (pAdapter == NULL)
	{
		// Get the first adapter enumerated by the factory according to the specification
		uk pvFactory(NULL);
		HRESULT iResult = CreateDXGIFactoryInternal<IDXGIFactory1>(__uuidof(IDXGIFactory1), &pvFactory);
		if (FAILED(iResult))
			return iResult;
		CDrxDXGLGIFactory* pFactory(CDrxDXGLGIFactory::FromInterface(static_cast<IDXGIFactory1*>(pvFactory)));
		iResult = pFactory->EnumAdapters(0, &pAdapter);
		pFactory->Release();
		if (FAILED(iResult))
			return iResult;
	}

	assert(pAdapter != NULL);
	CDrxDXGLGIAdapter* pDXGLAdapter(CDrxDXGLGIAdapter::FromInterface(pAdapter));

	D3D_FEATURE_LEVEL eDevFeatureLevel;
	if (pDXGLAdapter != NULL)
		eDevFeatureLevel = pDXGLAdapter->GetSupportedFeatureLevel();
	else
	{
		DXGL_TODO("Get the supported feature level even if no adapter is specified")
		eDevFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	}

	if (pFeatureLevels != NULL && FeatureLevels > 0)
	{
		D3D_FEATURE_LEVEL uMaxAllowedFL(*pFeatureLevels);
		while (FeatureLevels > 1)
		{
			++pFeatureLevels;
			uMaxAllowedFL = max(uMaxAllowedFL, *pFeatureLevels);
			--FeatureLevels;
		}
		eDevFeatureLevel = min(eDevFeatureLevel, uMaxAllowedFL);
	}

	if (pFeatureLevel != NULL)
		*pFeatureLevel = eDevFeatureLevel;

	if (ppDevice != NULL)
	{
		_smart_ptr<CDrxDXGLDevice> spDevice(new CDrxDXGLDevice(pDXGLAdapter, eDevFeatureLevel));
		if (!spDevice->Initialize(pSwapChainDesc, ppSwapChain))
			return E_FAIL;

		CDrxDXGLDevice::ToInterface(ppDevice, spDevice);

		if (ppImmediateContext != NULL)
			(*ppDevice)->GetImmediateContext(ppImmediateContext);
	}

	return S_OK;
}

DXGL_API HRESULT WINAPI D3DCreateBlob(SIZE_T NumBytes, LPD3D10BLOB* ppBuffer)
{
	CDrxDXGLBlob::ToInterface(ppBuffer, new CDrxDXGLBlob(NumBytes));
	return (*ppBuffer)->GetBufferPointer() != NULL;
}

////////////////////////////////////////////////////////////////////////////
//  Required global functions declared in D3DCompiler.h and included headers
////////////////////////////////////////////////////////////////////////////

DXGL_API HRESULT WINAPI D3DReflect(
  LPCVOID pSrcData,
  SIZE_T SrcDataSize,
  REFIID pInterface,
  uk * ppReflector)
{
	if (pInterface == IID_ID3D11ShaderReflection)
	{
		CDrxDXGLShaderReflection* pReflection(new CDrxDXGLShaderReflection());
		if (pReflection->Initialize(pSrcData))
		{
			CDrxDXGLShaderReflection::ToInterface(reinterpret_cast<ID3D11ShaderReflection**>(ppReflector), pReflection);
			return S_OK;
		}
		pReflection->Release();
	}
	return E_FAIL;
}

DXGL_API HRESULT WINAPI D3DDisassemble(
  LPCVOID pSrcData,
  SIZE_T SrcDataSize,
  UINT Flags,
  LPCSTR szComments,
  ID3DBlob** ppDisassembly)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////
//  Required global functions declared in D3DX11.h and included headers
////////////////////////////////////////////////////////////////////////////

DXGL_API HRESULT WINAPI D3DX11CreateTextureFromMemory(
  ID3D11Device* pDevice,
  ukk pSrcData,
  size_t SrcDataSize,
  D3DX11_IMAGE_LOAD_INFO* pLoadInfo,
  ID3DX11ThreadPump* pPump,
  ID3D11Resource** ppTexture,
  long* pResult)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

DXGL_API HRESULT WINAPI D3DX11SaveTextureToFile(ID3D11DeviceContext* pDevice,
                                                ID3D11Resource* pSrcResource,
                                                D3DX11_IMAGE_FILE_FORMAT fmt,
                                                tukk pDestFile)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

DXGL_API HRESULT WINAPI D3DX11CompileFromMemory(
  LPCSTR pSrcData,
  SIZE_T SrcDataLen,
  LPCSTR pFileName,
  CONST D3D10_SHADER_MACRO* pDefines,
  LPD3D10INCLUDE pInclude,
  LPCSTR pFunctionName,
  LPCSTR pProfile,
  UINT Flags1,
  UINT Flags2,
  ID3DX11ThreadPump* pPump,
  ID3D10Blob** ppShader,
  ID3D10Blob** ppErrorMsgs,
  HRESULT* pHResult)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////
//  Required global functions declared in dxgi.h and included headers
////////////////////////////////////////////////////////////////////////////

DXGL_API HRESULT WINAPI CreateDXGIFactory(REFIID riid, uk * ppFactory)
{
	return CreateDXGIFactoryInternal<IDXGIFactory>(riid, ppFactory);
}

DXGL_API HRESULT WINAPI CreateDXGIFactory1(REFIID riid, uk * ppFactory)
{
	return CreateDXGIFactoryInternal<IDXGIFactory1>(riid, ppFactory);
}

////////////////////////////////////////////////////////////////////////////
//  Frame debugging functions
////////////////////////////////////////////////////////////////////////////

#define DXGL_PROFILE_USE_GREMEDY_STRING_MARKER 0
#if defined(ENABLE_FRAME_PROFILER_LABELS)
	#define DXGL_PROFILE_USE_KHR_DEBUG           1
#else
	#define DXGL_PROFILE_USE_KHR_DEBUG           0
#endif

#if DXGL_PROFILE_USE_GREMEDY_STRING_MARKER

template<size_t uSuffixSize>
struct SDebugStringBuffer
{
	enum {MAX_TEXT_LENGTH = 1024};
	char s_acBuffer[MAX_TEXT_LENGTH + uSuffixSize];

	SDebugStringBuffer(tukk szSuffix)
	{
		NDrxOpenGL::Memcpy(s_acBuffer + MAX_TEXT_LENGTH, szSuffix, uSuffixSize);
	}

	tukk Write(tukk szText)
	{
		size_t uTextLength(min((size_t)MAX_TEXT_LENGTH, strlen(szText)));
		tuk szDest(s_acBuffer + MAX_TEXT_LENGTH - uTextLength);
		NDrxOpenGL::Memcpy(szDest, szText, uTextLength);
		return szDest;
	}
};

	#define DXGL_DEBUG_STRING_BUFFER(_Suffix, _Name) SDebugStringBuffer<DXGL_ARRAY_SIZE(_Suffix)> _Name(_Suffix)

DXGL_DEBUG_STRING_BUFFER(": enter", g_kEnterDebugBuffer);
DXGL_DEBUG_STRING_BUFFER(": leave", g_kLeaveDebugBuffer);

#endif //DXGL_PROFILE_USE_GREMEDY_STRING_MARKER

DXGL_API void DXGLProfileLabel(tukk szName)
{
#if DXGL_PROFILE_USE_GREMEDY_STRING_MARKER && DXGL_EXTENSION_LOADER
	if (DXGL_GL_EXTENSION_SUPPORTED(GREMEDY_string_marker))
		glStringMarkerGREMEDY(0, szName);
#endif //DXGL_PROFILE_USE_GREMEDY_STRING_MARKER && DXGL_EXTENSION_LOADER
#if DXGL_PROFILE_USE_KHR_DEBUG && DXGL_SUPPORT_DEBUG_OUTPUT
	if (NDrxOpenGL::SupportDebugOutput())
		glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, strlen(szName), szName);
#endif //DXGL_PROFILE_USE_KHR_DEBUG && DXGL_SUPPORT_DEBUG_OUTPUT
}

DXGL_API void DXGLProfileLabelPush(tukk szName)
{
#if DXGL_PROFILE_USE_GREMEDY_STRING_MARKER && DXGL_EXTENSION_LOADER
	if (DXGL_GL_EXTENSION_SUPPORTED(GREMEDY_string_marker))
		glStringMarkerGREMEDY(0, g_kEnterDebugBuffer.Write(szName));
#endif //DXGL_PROFILE_USE_GREMEDY_STRING_MARKER && DXGL_EXTENSION_LOADER
#if DXGL_PROFILE_USE_KHR_DEBUG && DXGL_SUPPORT_DEBUG_OUTPUT
	if (NDrxOpenGL::SupportDebugOutput())
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, strlen(szName), szName);
#endif //DXGL_PROFILE_USE_KHR_DEBUG && DXGL_SUPPORT_DEBUG_OUTPUT
}

DXGL_API void DXGLProfileLabelPop(tukk szName)
{
#if DXGL_PROFILE_USE_GREMEDY_STRING_MARKER && DXGL_EXTENSION_LOADER
	if (DXGL_GL_EXTENSION_SUPPORTED(GREMEDY_string_marker))
		glStringMarkerGREMEDY(0, g_kLeaveDebugBuffer.Write(szName));
#endif //DXGL_PROFILE_USE_GREMEDY_STRING_MARKER && DXGL_EXTENSION_LOADER
#if DXGL_PROFILE_USE_KHR_DEBUG && DXGL_SUPPORT_DEBUG_OUTPUT
	if (NDrxOpenGL::SupportDebugOutput())
		glPopDebugGroup();
#endif //DXGL_PROFILE_USE_KHR_DEBUG && DXGL_SUPPORT_DEBUG_OUTPUT
}

inline CDrxDXGLDevice* GetDXGLDevice(ID3D11Device* pDevice)
{
	return CDrxDXGLDevice::FromInterface(pDevice);
}

inline CDrxDXGLDeviceContext* GetDXGLDeviceContext(ID3D11DeviceContext* pDeviceContext)
{
	return CDrxDXGLDeviceContext::FromInterface(pDeviceContext);
}

////////////////////////////////////////////////////////////////////////////
//  DXGL Extensions
////////////////////////////////////////////////////////////////////////////

#if !DXGL_FULL_EMULATION

void DXGLInitialize(u32 uNumSharedContexts)
{
	NDrxOpenGL::SGlobalConfig::RegisterVariables();
	NDrxOpenGL::CDevice::Configure(uNumSharedContexts);
}

	#if OGL_SINGLE_CONTEXT

void DXGLBindDeviceContext(ID3D11DeviceContext* pDeviceContext)
{
	NDrxOpenGL::CContext* pGLContext(GetDXGLDeviceContext(pDeviceContext)->GetGLContext());
	pGLContext->GetDevice()->BindContext(pGLContext);
}

void DXGLUnbindDeviceContext(ID3D11DeviceContext* pDeviceContext)
{
	NDrxOpenGL::CContext* pGLContext(GetDXGLDeviceContext(pDeviceContext)->GetGLContext());
	pGLContext->GetDevice()->UnbindContext(pGLContext);
}

	#else

void DXGLReserveContext(ID3D11Device* pDevice)
{
	GetDXGLDevice(pDevice)->GetGLDevice()->ReserveContext();
}

void DXGLReleaseContext(ID3D11Device* pDevice)
{
	GetDXGLDevice(pDevice)->GetGLDevice()->ReleaseContext();
}

void DXGLBindDeviceContext(ID3D11DeviceContext* pDeviceContext, bool bReserved)
{
	NDrxOpenGL::CContext* pGLContext(GetDXGLDeviceContext(pDeviceContext)->GetGLContext());
	pGLContext->GetDevice()->BindContext(pGLContext);
	if (bReserved)
	{
		pGLContext->SetReservedContext(pGLContext);
		pGLContext->GetDevice()->ReserveContext();
	}
}

void DXGLUnbindDeviceContext(ID3D11DeviceContext* pDeviceContext, bool bReserved)
{
	NDrxOpenGL::CContext* pGLContext(GetDXGLDeviceContext(pDeviceContext)->GetGLContext());
	if (bReserved)
		pGLContext->GetDevice()->ReleaseContext();
	pGLContext->GetDevice()->UnbindContext(pGLContext);
}

	#endif

HRESULT DXGLMapBufferRange(ID3D11DeviceContext* pDeviceContext, ID3D11Buffer* pBuffer, size_t uOffset, size_t uSize, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource)
{
	NDrxOpenGL::CContext* pGLContext(static_cast<CDrxDXGLDeviceContext*>(pDeviceContext)->GetGLContext());
	NDrxOpenGL::SBuffer* pGLBuffer(pBuffer->GetGLBuffer());
	return (*pGLBuffer->m_pfMapBufferRange)(pGLBuffer, uOffset, uSize ? uSize : pGLBuffer->m_uSize - uOffset, MapType, MapFlags, pMappedResource, pGLContext) ? S_OK : E_FAIL;
}

void DXGLSetDepthBoundsTest(bool bEnabled, float fMin, float fMax)
{
	#if defined(GL_EXT_depth_bounds_test)
	if (bEnabled)
		glEnable(GL_DEPTH_BOUNDS_TEST_EXT);
	else
		glDisable(GL_DEPTH_BOUNDS_TEST_EXT);
	glDepthBoundsEXT(fMin, fMax);
	#else
	DXGL_WARNING("Depths Bounds Test extension not available on this platform");
	#endif
}

void DXGLTogglePixelTracing(ID3D11DeviceContext* pDeviceContext, bool bEnable, u32 uShaderHash, u32 uPixelX, u32 uPixelY)
{
	#if DXGL_ENABLE_SHADER_TRACING
	GetDXGLDeviceContext(pDeviceContext)->GetGLContext()->TogglePixelTracing(bEnable, uShaderHash, uPixelX, uPixelY);
	#endif
}

void DXGLToggleVertexTracing(ID3D11DeviceContext* pDeviceContext, bool bEnable, u32 uShaderHash, u32 uVertexID)
{
	#if DXGL_ENABLE_SHADER_TRACING
	GetDXGLDeviceContext(pDeviceContext)->GetGLContext()->ToggleVertexTracing(bEnable, uShaderHash, uVertexID);
	#endif
}

void DXGLToggleComputeTracing(
  ID3D11DeviceContext* pDeviceContext, bool bEnable, u32 uShaderHash,
  u32 uWorkGroupIDX, u32 uWorkGroupIDY, u32 uWorkGroupIDZ,
  u32 uLocalInvocationIDX, u32 uLocalInvocationIDY, u32 uLocalInvocationIDZ)
{
	#if DXGL_ENABLE_SHADER_TRACING
	GetDXGLDeviceContext(pDeviceContext)->GetGLContext()->ToggleComputeTracing(
	  bEnable, uShaderHash,
	  uWorkGroupIDX, uWorkGroupIDY, uWorkGroupIDZ,
	  uLocalInvocationIDX, uLocalInvocationIDY, uLocalInvocationIDZ);
	#endif
}

void DXGLCSSetConstantBuffers(ID3D11DeviceContext* pDeviceContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	GetDXGLDeviceContext(pDeviceContext)->CSSetConstantBuffers1(StartSlot, NumBuffers, pConstantBuffers, pFirstConstant, pNumConstants);
}

void DXGLPSSetConstantBuffers(ID3D11DeviceContext* pDeviceContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	GetDXGLDeviceContext(pDeviceContext)->PSSetConstantBuffers1(StartSlot, NumBuffers, pConstantBuffers, pFirstConstant, pNumConstants);
}

void DXGLVSSetConstantBuffers(ID3D11DeviceContext* pDeviceContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	GetDXGLDeviceContext(pDeviceContext)->VSSetConstantBuffers1(StartSlot, NumBuffers, pConstantBuffers, pFirstConstant, pNumConstants);
}

void DXGLGSSetConstantBuffers(ID3D11DeviceContext* pDeviceContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	GetDXGLDeviceContext(pDeviceContext)->GSSetConstantBuffers1(StartSlot, NumBuffers, pConstantBuffers, pFirstConstant, pNumConstants);
}

void DXGLHSSetConstantBuffers(ID3D11DeviceContext* pDeviceContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	GetDXGLDeviceContext(pDeviceContext)->HSSetConstantBuffers1(StartSlot, NumBuffers, pConstantBuffers, pFirstConstant, pNumConstants);
}

void DXGLDSSetConstantBuffers(ID3D11DeviceContext* pDeviceContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* pConstantBuffers, const UINT* pFirstConstant, const UINT* pNumConstants)
{
	GetDXGLDeviceContext(pDeviceContext)->DSSetConstantBuffers1(StartSlot, NumBuffers, pConstantBuffers, pFirstConstant, pNumConstants);
}

void DXGLIssueFrameFences(ID3D11Device* pDevice)
{
	GetDXGLDevice(pDevice)->GetGLDevice()->IssueFrameFences();
}

#endif //!DXGL_FULL_EMULATION

#if defined(USE_SDL2_VIDEO)

DXGL_API bool DXGLCreateSDLWindow(
  tukk szTitle,
  u32 uWidth,
  u32 uHeight,
  bool bFullScreen,
  HWND* pHandle)
{
	return NDrxOpenGL::CDevice::CreateSDLWindow(szTitle, uWidth, uHeight, bFullScreen, pHandle);
}

DXGL_API void DXGLDestroySDLWindow(HWND kHandle)
{
	NDrxOpenGL::CDevice::DestroySDLWindow(kHandle);
}

#endif //defined(USE_SDL2_VIDEO)

////////////////////////////////////////////////////////////////////////////
//  DxErr Logging and error functions
////////////////////////////////////////////////////////////////////////////

DXGL_API tukk WINAPI DXGetErrorStringA(HRESULT hr)
{
	DXGL_NOT_IMPLEMENTED
	return "";
}

DXGL_API const WCHAR* WINAPI DXGetErrorStringW(HRESULT hr)
{
	DXGL_NOT_IMPLEMENTED
	return L"";
}

DXGL_API tukk WINAPI DXGetErrorDescriptionA(HRESULT hr)
{
	DXGL_NOT_IMPLEMENTED
	return "";
}

DXGL_API const WCHAR* WINAPI DXGetErrorDescriptionW(HRESULT hr)
{
	DXGL_NOT_IMPLEMENTED
	return L"";
}

DXGL_API HRESULT WINAPI DXTraceA(tukk strFile, DWORD dwLine, HRESULT hr, tukk strMsg, BOOL bPopMsgBox)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

DXGL_API HRESULT WINAPI DXTraceW(tukk strFile, DWORD dwLine, HRESULT hr, const WCHAR* strMsg, BOOL bPopMsgBox)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

#if !DXGL_FULL_EMULATION

	#if OGL_SINGLE_CONTEXT

SDXGLDeviceContextThreadLocalHandle::SDXGLDeviceContextThreadLocalHandle()
{
	m_pTLSHandle = NDrxOpenGL::CreateTLS();
}

SDXGLDeviceContextThreadLocalHandle::~SDXGLDeviceContextThreadLocalHandle()
{
	NDrxOpenGL::DestroyTLS(m_pTLSHandle);
}

void SDXGLDeviceContextThreadLocalHandle::Set(ID3D11DeviceContext* pDeviceContext)
{
	ID3D11DeviceContext* pPrevDeviceContext(static_cast<ID3D11DeviceContext*>(NDrxOpenGL::GetTLSValue(m_pTLSHandle)));
	if (pPrevDeviceContext != pDeviceContext)
	{
		if (pPrevDeviceContext != NULL)
			DXGLUnbindDeviceContext(pPrevDeviceContext);
		NDrxOpenGL::SetTLSValue(m_pTLSHandle, pDeviceContext);
		if (pDeviceContext != NULL)
			DXGLBindDeviceContext(pDeviceContext);
	}
}

	#else

SDXGLContextThreadLocalHandle::SDXGLContextThreadLocalHandle()
{
	m_pTLSHandle = NDrxOpenGL::CreateTLS();
}

SDXGLContextThreadLocalHandle::~SDXGLContextThreadLocalHandle()
{
	NDrxOpenGL::DestroyTLS(m_pTLSHandle);
}

void SDXGLContextThreadLocalHandle::Set(ID3D11Device* pDevice)
{
	ID3D11Device* pPrevDevice(static_cast<ID3D11Device*>(NDrxOpenGL::GetTLSValue(m_pTLSHandle)));
	if (pPrevDevice != pDevice)
	{
		if (pPrevDevice != NULL)
			DXGLReleaseContext(pPrevDevice);
		NDrxOpenGL::SetTLSValue(m_pTLSHandle, pDevice);
		if (pDevice != NULL)
			DXGLReserveContext(pDevice);
	}
}

SDXGLDeviceContextThreadLocalHandle::SDXGLDeviceContextThreadLocalHandle()
{
	m_pTLSHandle = NDrxOpenGL::CreateTLS();
}

SDXGLDeviceContextThreadLocalHandle::~SDXGLDeviceContextThreadLocalHandle()
{
	NDrxOpenGL::DestroyTLS(m_pTLSHandle);
}

void SDXGLDeviceContextThreadLocalHandle::Set(ID3D11DeviceContext* pDeviceContext, bool bReserved)
{
	ID3D11DeviceContext* pPrevDeviceContext(static_cast<ID3D11DeviceContext*>(NDrxOpenGL::GetTLSValue(m_pTLSHandle)));
	if (pPrevDeviceContext != pDeviceContext)
	{
		if (pPrevDeviceContext != NULL)
			DXGLUnbindDeviceContext(pPrevDeviceContext, bReserved);
		NDrxOpenGL::SetTLSValue(m_pTLSHandle, pDeviceContext);
		if (pDeviceContext != NULL)
			DXGLBindDeviceContext(pDeviceContext, bReserved);
	}
}

	#endif

#endif //!DXGL_FULL_EMULATION
