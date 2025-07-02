// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxDXGLMisc.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Internal declarations of types and macros required by the
//               DXGL library
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLMISC__
#define __DRXDXGLMISC__

#if DRX_PLATFORM_WINDOWS
	#include <objbase.h>
	#include <mmsyscom.h>
#endif

#if defined(DO_RENDERLOG) || !defined(_RELEASE)
	#define DXGL_VIRTUAL_DEVICE_AND_CONTEXT 1
#else
	#define DXGL_VIRTUAL_DEVICE_AND_CONTEXT 0
#endif

#include <drx3D/Render/D3D/DXGL/DrxDXGLGuid.hpp>

#if !defined(_MSC_VER)
	#define __in
	#define __in_opt
	#define __out
	#define __out_opt
	#define __inout
	#define __inout_opt
	#define __out_ecount_opt(X)
	#define __stdcall
	#define __noop ((void)0)
	#define STDMETHODCALLTYPE
	#define __in_range(Start, Count)
	#define __in_ecount(Count)
	#define __out_ecount(Count)
	#define __in_ecount_opt(Count)
	#define __in_bcount_opt(Count)
	#define __out_bcount(Count)
	#define __out_bcount_opt(Count)
	#define __in_xcount_opt(Count)
	#define __RPC__deref_out
	#define _In_
	#define _In_opt_
	#define _Out_
	#define _Out_opt_
	#define _Inout_
	#define _Inout_opt_
#endif  // !defined(_MSC_VER)

#if !DRX_PLATFORM_WINDOWS

	#ifdef NO_ILINE
		#define FORCEINLINE inline
	#else
		#define FORCEINLINE __attribute__((always_inline)) inline
	#endif

	#define SEVERITY_SUCCESS 0
	#define SEVERITY_ERROR   1

	#undef SUCCEEDED
	#define SUCCEEDED(x) ((x) >= 0)
	#define FAILED(x)    (!(SUCCEEDED(x)))

	#define MAKE_HRESULT(sev, fac, code) \
	  ((HRESULT) (((u64)(sev) << 31) | ((u64)(fac) << 16) | ((u64)(code))))

	#define S_OK          0
	#define S_FALSE       1

	#define CCHDEVICENAME 32

	#define CONST         const
	#define VOID          void
	#define THIS          void
	#define THIS_
	#if !defined(TRUE) || !defined(FALSE)
		#undef TRUE
		#undef FALSE
		#define TRUE  true
		#define FALSE false
	#endif //!defined(TRUE) || !defined(FALSE)

	#define interface struct
	#define STDMETHOD(method)         virtual HRESULT STDMETHODCALLTYPE method
	#define STDMETHOD_(type, method)  virtual type STDMETHODCALLTYPE method
	#define STDMETHODV(method)        virtual HRESULT STDMETHODVCALLTYPE method
	#define STDMETHODV_(type, method) virtual type STDMETHODVCALLTYPE method

	#if defined(UNICODE)
		#define TEXT(_STRING) L ## _STRING
	#else
		#define TEXT(_STRING) _STRING
	#endif

typedef i32         BOOL;
typedef char          CHAR;
typedef i32         INT;
typedef u8 UCHAR;
typedef u8         UINT8;
typedef u32        UINT32;
typedef u32        UINT;
typedef ukk   LPCVOID;

typedef u32        HMONITOR;
typedef uk         HINSTANCE;

typedef struct _LUID
{
	u32 LowPart;
	long   HighPart;
} LUID, * PLUID;

typedef RECT*       LPRECT;
typedef const RECT* LPCRECT;

enum
{
	E_OUTOFMEMORY = 0x8007000E,
	E_FAIL        = 0x80004005,
	E_ABORT       = 0x80004004,
	E_INVALIDARG  = 0x80070057,
	E_NOINTERFACE = 0x80004002,
	E_NOTIMPL     = 0x80004001,
	E_UNEXPECTED  = 0x8000FFFF
};

#else

// Required as long as Windows builds allow compilation through a separate DLL (DrxD3DCompilerStub) to distinguish between ID3D11Blob and CDrxDXGLBlob
	#define DXGL_BLOB_INTEROPERABILITY

typedef RECT D3D11_RECT;

#endif  // !DRX_PLATFORM_WINDOWS

#if defined(_MSC_VER)
	#define DXGL_EXPORT __declspec(dllexport)
	#define DXGL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
	#define DXGL_EXPORT __attribute__ ((visibility("default")))
	#define DXGL_IMPORT __attribute__ ((visibility("default")))
#else
	#define DXGL_EXPORT
	#define DXGL_IMPORT
#endif

////////////////////////////////////////////////////////////////////////////
//  Forward declaration of typedef interfaces
////////////////////////////////////////////////////////////////////////////

#if !DXGL_FULL_EMULATION

typedef class CDrxDXGLResource                    ID3D11Resource;
typedef class CDrxDXGLBuffer                      ID3D11Buffer;
typedef class CDrxDXGLTexture1D                   ID3D11Texture1D;
typedef class CDrxDXGLTexture2D                   ID3D11Texture2D;
typedef class CDrxDXGLTexture3D                   ID3D11Texture3D;
typedef class CDrxDXGLView                        ID3D11View;
typedef class CDrxDXGLRenderTargetView            ID3D11RenderTargetView;
typedef class CDrxDXGLDepthStencilView            ID3D11DepthStencilView;
typedef class CDrxDXGLShaderResourceView          ID3D11ShaderResourceView;
typedef class CDrxDXGLUnorderedAccessView         ID3D11UnorderedAccessView;
typedef class CDrxDXGLInputLayout                 ID3D11InputLayout;
typedef class CDrxDXGLVertexShader                ID3D11VertexShader;
typedef class CDrxDXGLHullShader                  ID3D11HullShader;
typedef class CDrxDXGLDomainShader                ID3D11DomainShader;
typedef class CDrxDXGLGeometryShader              ID3D11GeometryShader;
typedef class CDrxDXGLPixelShader                 ID3D11PixelShader;
typedef class CDrxDXGLComputeShader               ID3D11ComputeShader;
typedef class CDrxDXGLSamplerState                ID3D11SamplerState;
typedef class CDrxDXGLQuery                       ID3D11Asynchronous;
typedef class CDrxDXGLQuery                       ID3D11Predicate;
typedef class CDrxDXGLQuery                       ID3D11Query;
typedef class CDrxDXGLGIOutput                    IDXGIOutput;
typedef class CDrxDXGLSwapChain                   IDXGISwapChain;
typedef class CDrxDXGLGIFactory                   IDXGIFactory;
typedef class CDrxDXGLGIFactory                   IDXGIFactory1;
typedef class CDrxDXGLGIObject                    IDXGIObject;
typedef class CDrxDXGLGIAdapter                   IDXGIAdapter;
typedef class CDrxDXGLGIAdapter                   IDXGIAdapter1;
typedef class CDrxDXGLDeviceChild                 ID3D11DeviceChild;
typedef class CDrxDXGLSwitchToRef                 ID3D11SwitchToRef;
typedef class CDrxDXGLShaderReflection            ID3D11ShaderReflection;
typedef class CDrxDXGLShaderReflectionVariable    ID3D11ShaderReflectionVariable;
typedef class CDrxDXGLShaderReflectionVariable    ID3D11ShaderReflectionType;
typedef class CDrxDXGLShaderReflectionConstBuffer ID3D11ShaderReflectionConstantBuffer;
typedef class CDrxDXGLShaderReflection            ID3D11ShaderReflection;
typedef class CDrxDXGLBlendState                  ID3D11BlendState;
typedef class CDrxDXGLDepthStencilState           ID3D11DepthStencilState;
typedef class CDrxDXGLRasterizerState             ID3D11RasterizerState;
	#if DXGL_VIRTUAL_DEVICE_AND_CONTEXT
struct ID3D11Device;
struct ID3D11DeviceContext;
	#else
typedef class CDrxDXGLDevice        ID3D11Device;
typedef class CDrxDXGLDevice        IDXGIDevice;
typedef class CDrxDXGLDeviceContext ID3D11DeviceContext;
	#endif

#endif //!DXGL_FULL_EMULATION

#if defined(DXGL_BLOB_INTEROPERABILITY) || DXGL_FULL_EMULATION
typedef struct ID3D10Blob  ID3DBlob;
#else
typedef class CDrxDXGLBlob ID3D10Blob;
typedef class CDrxDXGLBlob ID3DBlob;
#endif

#include <drx3D/Render/D3D/DXGL/IDrxDXGLUnknown.hpp>
#include <drx3D/Render/D3D/DXGL/DXGL_D3D11.h>
#include <drx3D/Render/D3D/DXGL/DXGL_D3DX11.h>
#include <drx3D/Render/D3D/DXGL/DXGL_D3DCompiler.h>

////////////////////////////////////////////////////////////////////////////
//  Helper functions
////////////////////////////////////////////////////////////////////////////

inline UINT D3D11CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT MipLevels)
{
	return MipSlice + ArraySlice * MipLevels;
}

#include <drx3D/Render/D3D/DXGL/DrxDXGLLegacy.hpp>

#endif //__DRXDXGLMISC__
