// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Render
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>

#define DRX_RENDERER_VULKAN 1
#define DRX_RENDERER_OPENGL 1

/* определённые внешне переключатели реализации отобразителя:
 *  DRX_RENDERER_DIRECT3D       110, 111, 120, 121, 122
 *  DRX_RENDERER_OPENGL         430, 440, 450
 *  DRX_RENDERER_OPENGLES       310
 *  DRX_RENDERER_VULKAN         10
 *
 * Эмуляторы комбинаторных триггеров:
 *  DRX_RENDERER_DIRECT3D + DRX_RENDERER_OPENGL  -> реализация DirectX-API + Opengl
 *
 * Combinations select special code paths:
 *  DRX_RENDERER_DIRECT3D + DRX_PLATFORM_DURANGO -> Durango-only Direct3D extensions
 *  DRX_RENDERER_DIRECT3D + USE_NV_API           -> Nvidia-only Direct3D extensions
 */

#if ( defined(DRX_USE_DX12) || defined(OPENGL) || defined(VULKAN)) || \
   !(defined(DRX_RENDERER_DIRECT3D) || defined(DRX_RENDERER_OPENGL) || \
   defined(DRX_RENDERER_OPENGLES) || defined(DRX_RENDERER_VULKAN))
     #error "Конфигурация Renderer-Type не изменена на унифицированную схему именования!"
#endif

#if (DRX_RENDERER_DIRECT3D >= 110) && !((DRX_RENDERER_DIRECT3D >= 120) || DRX_RENDERER_VULKAN)
	#define DX11_WRAPPABLE_INTERFACE	1

	#if !defined(_RELEASE) // DO_RENDERLOG cannot be compiled in release configs for consoles
		#define DO_RENDERLOG 1
	#endif
#endif

// BUFFER_ENABLE_DIRECT_ACCESS
// stores pointers to actual backing storage of vertex buffers. Can only be used on architectures
// that have a unified memory architecture and further guarantee that buffer storage does not change
// on repeated accesses.
// NOTE: DX12 doesn't support non-direct buffer access, because the otherwise used MAP_DISCARD
// implementation doesn't track if to-be-discarded resources have open views, this is a design decision
#if (DRX_PLATFORM_CONSOLE || (DRX_RENDERER_DIRECT3D >= 120) || DRX_RENDERER_VULKAN)
	#define BUFFER_ENABLE_DIRECT_ACCESS 1
#endif

// BUFFER_USE_STAGED_UPDATES
// On platforms that support staging buffers, special buffers are allocated that act as a staging area
// for updating buffer contents on the fly.
// On platforms with UMA it's not useful to stage copies as all resources are in CPU accessible memory
#if !(DRX_PLATFORM_CONSOLE) && !(DRX_PLATFORM_MOBILE)
	#define BUFFER_USE_STAGED_UPDATES 1
#else
	#define BUFFER_USE_STAGED_UPDATES 0
#endif

// BUFFER_SUPPORT_TRANSIENT_POOLS
// On d3d11 we want to separate the fire-and-forget allocations from the longer lived dynamic ones
#if DRX_PLATFORM_DESKTOP
	#define BUFFER_SUPPORT_TRANSIENT_POOLS 1
#else
	#define BUFFER_SUPPORT_TRANSIENT_POOLS 0
#endif

#ifndef BUFFER_ENABLE_DIRECT_ACCESS
	#define BUFFER_ENABLE_DIRECT_ACCESS 0
#endif

// CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS
// Enable if we can directly write to subranges of constant buffers while they are in use by the GPU
// and the device manager should manage constant buffers
#if BUFFER_ENABLE_DIRECT_ACCESS == 1
	#if DRX_PLATFORM_DURANGO || (DRX_RENDERER_DIRECT3D >= 111) || DRX_RENDERER_VULKAN
		#define CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS 1
	#else
		#define CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS 0
	#endif
#elif (DRX_RENDERER_DIRECT3D >= 111) && (DRX_RENDERER_DIRECT3D < 120)
	#define CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS 1
#else
	#define CONSTANT_BUFFER_ENABLE_DIRECT_ACCESS 0
#endif

#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	#define DEVICE_SUPPORTS_PERFORMANCE_DEVICE
#endif

#if DRX_PLATFORM_DURANGO
	#define DURANGO_USE_ESRAM 1
#endif

//#define DEFINE_MODULE_NAME "DrxRender???"

#ifdef _DEBUG
	#define CRTDBG_MAP_ALLOC
#endif //_DEBUG

#undef USE_STATIC_NAME_TABLE
#define USE_STATIC_NAME_TABLE

#if !defined(_RELEASE)
	#define ENABLE_FRAME_PROFILER
#endif

#if !defined(_RELEASE) || defined(PERFORMANCE_BUILD)
	#define ENABLE_SIMPLE_GPU_TIMERS
	#define ENABLE_FRAME_PROFILER_LABELS
#endif

#ifdef ENABLE_FRAME_PROFILER
	#define PROFILE 1
#endif

#define FUNCTION_PROFILER_RENDERER() DRX_PROFILE_FUNCTION(PROFILE_RENDERER)

#define SCOPED_RENDERER_ALLOCATION_NAME_HINT(str)

#define SHADER_ASYNC_COMPILATION

#if !defined(_RELEASE)
//# define DETAILED_PROFILING_MARKERS
#endif
#if defined(DETAILED_PROFILING_MARKERS)
	#define DETAILED_PROFILE_MARKER(x) DETAILED_PROFILE_MARKER((x))
#else
	#define DETAILED_PROFILE_MARKER(x) (void)0
#endif

#define RAIN_OCC_MAP_SIZE 256

#if DRX_PLATFORM_ORBIS
	#define USE_SCUE
	#ifdef USE_SCUE
			#define CUSTOM_FETCH_SHADERS // InputLayouts generate fetch shader code instead of being generated when vertex shader created

//#  define ENABLE_SCUE_VALIDATION	        // Checks for NULL bindings + incorrect bindings
//#  define GPU_MEMORY_MAPPING_VALIDATION   // Checks that the objects being bound are mapped in GPU visible memory (Slow)
	#else
	#endif
	#define CUE_SUPPORTS_GEOMETRY_SHADERS // Define if you want to use geometry shaders

#endif

#ifdef ENABLE_SCUE_VALIDATION
enum EVerifyType
{
	eVerify_Normal,
	eVerify_ConstantBuffer,
	eVerify_VertexBuffer,
	eVerify_SRVTexture,
	eVerify_SRVBuffer,
	eVerify_UAVTexture,
	eVerify_UAVBuffer,
};
#endif

#if DRX_PLATFORM_SSE2 && DRX_COMPILER_MSVC
	#include <fvec.h>
	#include <drx3D/CoreX/Assert/DrxAssert.h> // to restore assert macro which was changed by <fvec.h>
	#define CONST_INT32_PS(N, V3, V2, V1, V0)                                   \
	  const _MM_ALIGN16 i32 ## N[] = { V0, V1, V2, V3 };   /*little endian!*/ \
	  const F32vec4 N = _mm_load_ps((float*)## N);
#endif

// enable support for baked meshes and decals on PC
#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE || DRX_PLATFORM_DURANGO
	#define TEXTURE_GET_SYSTEM_COPY_SUPPORT
#endif

#if BUFFER_ENABLE_DIRECT_ACCESS && DRX_PLATFORM_WINDOWS && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	#error BUFFER_ENABLE_DIRECT_ACCESS is not supported on windows pre DX12 platforms
#endif

#define MAX_REND_RECURSION_LEVELS 2

#if DRX_RENDERER_OPENGL
	#define OGL_ADAPT_CLIP_SPACE   1
	#define OGL_FLIP_Y             1
	#define OGL_MODIFY_PROJECTIONS !OGL_ADAPT_CLIP_SPACE
	#define OGL_SINGLE_CONTEXT     1
#endif

#ifdef STRIP_RENDER_THREAD
	#define m_nCurThreadFill    0
	#define m_nCurThreadProcess 0
#endif

#ifdef STRIP_RENDER_THREAD
	#define ASSERT_IS_RENDER_THREAD(rt)
	#define ASSERT_IS_MAIN_THREAD(rt)
#else
	#define ASSERT_IS_RENDER_THREAD(rt)         assert((rt)->IsRenderThread());
	#define ASSERT_IS_MAIN_THREAD(rt)           assert((rt)->IsMainThread());
	#define ASSERT_IS_MAIN_OR_RENDER_THREAD(rt) assert((rt)->IsMainThread() || (rt)->IsRenderThread());
#endif

#define threadsafe
#define threadsafe_const const

typedef void (*RenderFunc)(void);

//#define ASSERT_IN_SHADER( expr ) assert( expr );
#define ASSERT_IN_SHADER(expr)

// TODO: linux clang doesn't behave like orbis clang and linux gcc doesn't behave like darwin gcc, all linux instanced can't manage squish-template overloading
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_ORBIS || DRX_PLATFORM_APPLE
	#define EXCLUDE_SQUISH_SDK
#endif

#if USE_SDL2 && (DRX_PLATFORM_ANDROID || DRX_PLATFORM_IOS || DRX_PLATFORM_LINUX)
	#define USE_SDL2_VIDEO	1
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_syswm.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#if !defined(__GNUC__)

#if DRX_PLATFORM_WINDOWS && DRX_RENDERER_DIRECT3D && !DRX_RENDERER_OPENGL && !DRX_RENDERER_OPENGLES && !DRX_RENDERER_VULKAN
// nv API
	#if !defined(EXCLUDE_NV_API)
		#define USE_NV_API 1
		#define NV_API_HEADER "NVIDIA/NVAPI_r386/nvapi.h>

		#if DRX_PLATFORM_64BIT
			#define NV_API_LIB "SDKs/NVIDIA/NVAPI_r386/amd64/nvapi64.lib"
		#else
			#define NV_API_LIB "SDKs/NVIDIA/NVAPI_r386/x86/nvapi.lib"
		#endif
	#endif

	// AMD EXT (DX11 only)
	#if !defined(EXCLUDE_AMD_API) && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
		#define USE_AMD_API 1
		#define AMD_API_HEADER "AMD/AGS Lib/inc/amd_ags.h>

		#if DRX_PLATFORM_64BIT
			#define AMD_API_LIB "SDKs/AMD/AGS Lib/lib/amd_ags_x64.lib"
		#else
			#define AMD_API_LIB "SDKs/AMD/AGS Lib/lib/amd_ags_x86.lib"
		#endif
	#endif
#endif
#endif

// SF implementation enabled
#define RENDERER_SUPPORT_SCALEFORM 1

// windows desktop API available for usage
#if DRX_PLATFORM_WINDOWS
	#define WINDOWS_DESKTOP_API
#endif

#if (DRX_RENDERER_DIRECT3D >= 120) || DRX_PLATFORM_DURANGO /*|| DRX_PLATFORM_ANDROID*/ || (DRX_RENDERER_VULKAN >= 10)
	#define DX11_COM_INTERFACES 0
#else
	#define DX11_COM_INTERFACES 1
#endif

//////////////////////////////////////////////////////////////////////////
#if DRX_RENDERER_OPENGL || DRX_RENDERER_OPENGLES
	#include <drx3D/CoreX/Platform/DrxLibrary.h>
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <drx3D/Render/D3D/DXGL/DrxDXGL.hpp>
	#if DRX_PLATFORM_WINDOWS
		typedef uintptr_t SOCKET;
	#endif
#elif ((DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120))
	#include <drx3D/CoreX/Platform/DrxLibrary.h>
	#include <drx3D/CoreX/Platform/DrxWindows.h>

	#if DRX_PLATFORM_DURANGO
		#include <d3d11_x.h>
		#include <d3d11shader_x.h>
		#include <d3dcompiler_x.h>
	#else
		#include <d3d11.h> // includes <windows.h>
		#if (DRX_RENDERER_DIRECT3D >= 111)
			#include <d3d11_1.h> // includes <windows.h>
		#endif
		#if (DRX_RENDERER_DIRECT3D >= 112)
			#include <d3d11_2.h> // includes <windows.h>
		#endif
		#if (DRX_RENDERER_DIRECT3D >= 113)
			#include <d3d11_3.h> // includes <windows.h>
		#endif

		#include <d3d11sdklayers.h>
		#include <d3d11shader.h>
		#include <d3dcompiler.h>

		#define VIRTUALGFX virtual
		#define FINALGFX final
		#define IID_GFX_ARGS IID_PPV_ARGS
	#endif

	#if DRX_PLATFORM_WINDOWS && !DRX_RENDERER_OPENGL
		#include <d3d9.h>
	#endif

	#if BUFFER_USE_STAGED_UPDATES == 0
		namespace detail
		{
			template<typename T> inline void safe_release(T*& ptr) { SAFE_RELEASE(ptr); }
			template<> inline void safe_release<ID3D11Buffer>(ID3D11Buffer*& ptr);
		}

		// Call custom release-code for ID3D11Buffer on Durango by replacing SAFE_RELEASE()
		#undef SAFE_RELEASE
		#define SAFE_RELEASE(x) do { detail::safe_release((x)); } while (false)
	#endif
#elif (DRX_RENDERER_DIRECT3D >= 120)
	#include <drx3D/CoreX/Platform/DrxLibrary.h>
	#include <drx3D/CoreX/Platform/DrxWindows.h>

	#if DRX_PLATFORM_DURANGO
		#include <d3d12_x.h>
		#include <d3d11shader_x.h>
		#include <d3dcompiler_x.h>
	#else
		#include <d3d12.h>       // includes <windows.h>
		#include <dxgi1_5.h>     // includes <windows.h>

		#include <d3d12sdklayers.h>
		#include <d3d11shader.h>
		#include <d3dcompiler.h>
	#endif

	#include <drx3D/Render/D3D/DX12/d3dx12.h>
#elif (DRX_RENDERER_VULKAN >= 10)
	#include <drx3D/CoreX/Platform/DrxLibrary.h>

	#ifdef DRX_PLATFORM_WINDOWS
		#define VK_USE_PLATFORM_WIN32_KHR   1
		#define VK_USE_DXGI                 1
	#elif defined(DRX_PLATFORM_ANDROID)
		#define VK_USE_PLATFORM_ANDROID_KHR 1
	#elif defined(DRX_PLATFORM_LINUX)
		#define VK_USE_PLATFORM_XCB_KHR     1 //not sure what platform to initialize on linux, figure it out later
	#endif

	#include <vulkan/vulkan.h>
#endif

// Internal numbers:  10|0   (three decimal digits)
// Direct3D numbers: 0xa|0?? (four hexadecimal digits)
#if DRX_RENDERER_DIRECT3D
	#define DRX_RENDERER_DIRECT3D_FL (D3D_FEATURE_LEVEL)(((DRX_RENDERER_DIRECT3D / 10) << 12) + ((DRX_RENDERER_DIRECT3D % 10) << 8))
#else
	#define DRX_RENDERER_DIRECT3D_FL (D3D_FEATURE_LEVEL)0
#endif

#if !DX11_COM_INTERFACES //including system headers before redefining D3D interfaces
	#if DRX_PLATFORM_WINAPI
		#include <Unknwn.h>
	#else
		#include <drx3D/Render/D3D/DX12/Unknwn_empty.h>
	#endif

	#if (DRX_RENDERER_DIRECT3D >= 120)
		#include <drx3D/Render/D3D/DX12/DrxDX12Guid.hpp>
	#elif (DRX_RENDERER_VULKAN >= 10)
		#include <drx3D/Render/D3D/Vulkan/CDrxVKGuid.hpp>
		#include <drx3D/Render/D3D/Vulkan/CDrxVKMisc.hpp>

		#if VK_USE_DXGI
			#define __dxgiformat_h__ // we define DXGI_FORMAT ourselves as we add more formats from vulkan
			#include <dxgi1_2.h>     // includes <windows.h>
		#endif
	#endif

	#if !DRX_PLATFORM_DURANGO
		#include <drx3D/Render/D3D/DX12/d3d11_structs.h>
	#endif
	#include <drx3D/Render/D3D/DX12/d3d11_empty.h>
#endif

#if DRX_PLATFORM_DURANGO
	#include <pix.h>

	#if !defined(RELEASE) || defined(ENABLE_PROFILING_CODE)
		#define USE_PIX_DURANGO 1
	#endif
#elif DRX_PLATFORM_WINDOWS
	#include <pix_win.h>
#endif

//////////////////////////////////////////////////////////////////////////
//#define Direct3D IDXGIAdapter

#if (DRX_RENDERER_DIRECT3D >= 120)
    #if DRX_PLATFORM_DURANGO
        #define DXGIFactory               IDXGIFactory1
        #define DXGIDevice                IDXGIDevice2
        #define DXGIAdapter               IDXGIAdapter2
        #define DXGIOutput                IDXGIOutput
        #define DXGISwapChain             IDXGISwapChain1
        typedef char DXGI_MATRIX_3X2_F;
    #else
        #define DXGIFactory               IDXGIFactory4
        #define DXGIDevice                IDXGIDevice3
        #define DXGIAdapter               IDXGIAdapter3
        #define DXGIOutput                IDXGIOutput4
        #define DXGISwapChain             IDXGISwapChain3
        #define IID_GRAPHICS_PPV_ARGS IID_PPV_ARGS
    #endif

    #if (DRX_RENDERER_DIRECT3D >= 111)
        #define D3DDeviceContext          ID3D11DeviceContext1
        #define D3DDevice                 ID3D11Device1
    #else
        #define D3DDeviceContext          ID3D11DeviceContext
        #define D3DDevice                 ID3D11Device
    #endif

#elif (DRX_RENDERER_DIRECT3D >= 111) && !DRX_PLATFORM_ORBIS
    #define     DXGIFactory               IDXGIFactory2
    #define     DXGIDevice                IDXGIDevice1
    #define     DXGIAdapter               IDXGIAdapter1
    #define     DXGIOutput                IDXGIOutput1
    #define     DXGISwapChain             IDXGISwapChain1

    #define     D3DDeviceContext          ID3D11DeviceContext1
    #define     D3DDevice                 ID3D11Device1

	typedef     IDXGIAdapter              IDXGIAdapterToCall;
	typedef     IDXGIFactory2             IDXGIFactory2ToCall;
	typedef     IDXGISwapChain1           IDXGISwapChain1ToCall;
#elif (DRX_RENDERER_VULKAN >= 10)

	#if VK_USE_DXGI
		typedef IDXGIFactory2             IDXGIFactoryToCall;
		typedef IDXGIAdapter1             IDXGIAdapterToCall;
		typedef IDXGIOutput               IDXGIOutputToCall;
	#endif

	#define     DXGIFactory               CDrxVKGIFactory
	#define     DXGIDevice                NDrxVulkan::CDevice
	#define     DXGIAdapter               CDrxVKGIAdapter
	#define     DXGIOutput                CDrxVKGIOutput
	#define     DXGISwapChain             CDrxVKSwapChain
    #define     D3DDeviceContext          NDrxVulkan::CRefCounted /* not used */
    #define     D3DDevice                 NDrxVulkan::CDevice
#else
    #define     DXGIFactory               IDXGIFactory1
    #define     DXGIDevice                IDXGIDevice1
    #define     DXGIAdapter               IDXGIAdapter1
    #define     DXGIOutput                IDXGIOutput
    #define     DXGISwapChain             IDXGISwapChain

    #define     D3DDeviceContext          ID3D11DeviceContext
    #define     D3DDevice                 ID3D11Device
#endif

#define         D3DVertexDeclaration      ID3D11InputLayout
#define         D3DVertexShader           ID3D11VertexShader
#define         D3DPixelShader            ID3D11PixelShader
#define         D3DResource               ID3D11Resource
#define         D3DBaseBuffer             ID3D11Resource
#define         D3DBaseTexture            ID3D11BaseTexture
#define         D3DLookupTexture          ID3D11Texture1D
#define         D3DTexture                ID3D11Texture2D
#define         D3DVolumeTexture          ID3D11Texture3D
#define         D3DCubeTexture            ID3D11Texture2D
#define         D3DVertexBuffer           ID3D11Buffer
#define         D3DShaderResource         ID3D11ShaderResourceView
#define         D3DUAV                    ID3D11UnorderedAccessView
#define         D3DIndexBuffer            ID3D11Buffer
#define         D3DBuffer                 ID3D11Buffer
#define         D3DSurface                ID3D11RenderTargetView
#define         D3DDepthSurface           ID3D11DepthStencilView
#define         D3DBaseView               ID3D11View

#if DRX_RENDERER_DIRECT3D
	#define     D3DOcclusionQuery         ID3D11Query
#elif DRX_RENDERER_VULKAN
	#define     D3DOcclusionQuery         NDrxVulkan::COcclusionQuery
#endif

#define         D3DViewPort               D3D11_VIEWPORT
#define         D3DRectangle              D3D11_RECT
#define         D3DFormat                 DXGI_FORMAT
#define         D3DPrimitiveType          D3D11_PRIMITIVE_TOPOLOGY
#define         D3DBlob                   ID3DBlob
#define         D3DSamplerState           ID3D11SamplerState
#define         D3DInputLayout            ID3D11InputLayout

#if (DRX_RENDERER_DIRECT3D >= 120) && (!DX11_COM_INTERFACES)
	typedef     IDXGIOutput               IDXGIOutputToCall;
	typedef     IDXGIAdapter              IDXGIAdapterToCall;
	typedef     IDXGIAdapter1             IDXGIAdapter1ToCall;
	typedef     IDXGIAdapter2             IDXGIAdapter2ToCall;
	typedef     IDXGIFactory              IDXGIFactoryToCall;
	typedef     IDXGIFactory1             IDXGIFactory1ToCall;
	typedef     IDXGIFactory2             IDXGIFactory2ToCall;
	typedef     IDXGISwapChain            IDXGISwapChainToCall;
	typedef     IDXGISwapChain1           IDXGISwapChain1ToCall;

	#if DRX_PLATFORM_DURANGO
	typedef     IDXGIOutput               IDXGIOutput4ToCall;
	typedef     IDXGIOutput               IDXGIOutput5ToCall;
	typedef     IDXGIAdapter2             IDXGIAdapter3ToCall;
	typedef     IDXGIFactory2             IDXGIFactory3ToCall;
	typedef     IDXGIFactory2             IDXGIFactory4ToCall;
	typedef     IDXGIFactory2             IDXGIFactory5ToCall;
	typedef     IDXGISwapChain1           IDXGISwapChain2ToCall;
	typedef     IDXGISwapChain1           IDXGISwapChain3ToCall;
	#else
	typedef     IDXGIOutput4              IDXGIOutput4ToCall;
	typedef     IDXGIOutput5              IDXGIOutput5ToCall;
	typedef     IDXGIAdapter2             IDXGIAdapter2ToCall;
	typedef     IDXGIAdapter3             IDXGIAdapter3ToCall;
	typedef     IDXGIFactory3             IDXGIFactory3ToCall;
	typedef     IDXGIFactory4             IDXGIFactory4ToCall;
	typedef     IDXGIFactory5             IDXGIFactory5ToCall;
	typedef     IDXGISwapChain2           IDXGISwapChain2ToCall;
	typedef     IDXGISwapChain3           IDXGISwapChain3ToCall;
	#endif

	#define     IDXGIOutput               CDrxDX12GIOutput
	#define     IDXGIOutput4              CDrxDX12GIOutput
	#define     IDXGIOutput5              CDrxDX12GIOutput
	#define     IDXGIAdapter              CDrxDX12GIAdapter
	#define     IDXGIAdapter1             CDrxDX12GIAdapter
	#define     IDXGIAdapter2             CDrxDX12GIAdapter
	#define     IDXGIAdapter3             CDrxDX12GIAdapter
	#define     IDXGIFactory              CDrxDX12GIFactory
	#define     IDXGIFactory1             CDrxDX12GIFactory
	#define     IDXGIFactory2             CDrxDX12GIFactory
	#define     IDXGIFactory3             CDrxDX12GIFactory
	#define     IDXGIFactory4             CDrxDX12GIFactory
	#define     IDXGIFactory5             CDrxDX12GIFactory
	#define     IDXGISwapChain            CDrxDX12SwapChain
	#define     IDXGISwapChain1           CDrxDX12SwapChain
	#define     IDXGISwapChain2           CDrxDX12SwapChain
	#define     IDXGISwapChain3           CDrxDX12SwapChain

	#define     ID3D11DeviceContext       CDrxDX12DeviceContext
	#define     ID3D11DeviceContext1      CDrxDX12DeviceContext
	#define     ID3D11Device              CDrxDX12Device
	#define     ID3D11Device1             CDrxDX12Device
	#define     ID3D11DeviceChild         CDrxDX12DeviceChild<IEmptyDeviceChild>
	#define     ID3D11BlendState          CDrxDX12BlendState
	#define     ID3D11DepthStencilState   CDrxDX12DepthStencilState
	#define     ID3D11RasterizerState     CDrxDX12RasterizerState
	#define     ID3D11SamplerState        CDrxDX12SamplerState
	#define     ID3D11InputLayout         CDrxDX12InputLayout
	#define     ID3D11View                CDrxDX12View<IEmptyView>
	#define     ID3D11DepthStencilView    CDrxDX12DepthStencilView
	#define     ID3D11RenderTargetView    CDrxDX12RenderTargetView
	#define     ID3D11ShaderResourceView  CDrxDX12ShaderResourceView
	#define     ID3D11UnorderedAccessView CDrxDX12UnorderedAccessView
	#define     ID3D11Resource            CDrxDX12Resource<IEmptyResource>
	#define     ID3D11Buffer              CDrxDX12Buffer
	#define     ID3D11BaseTexture         CDrxDX12Resource<IEmptyResource>
	#define     ID3D11Texture1D           CDrxDX12Texture1D
	#define     ID3D11Texture2D           CDrxDX12Texture2D
	#define     ID3D11Texture3D           CDrxDX12Texture3D
	#define     ID3D11Asynchronous        CDrxDX12Asynchronous<IEmptyAsynchronous>
	#define     ID3D11Query               CDrxDX12Query
	#define     ID3D11InputLayout         CDrxDX12InputLayout
	#define     ID3D11PixelShader         CDrxDX12Shader
	#define     ID3D11VertexShader        CDrxDX12Shader
	#define     ID3D11ComputeShader       CDrxDX12Shader
	#define     ID3D11HullShader          CDrxDX12Shader
	#define     ID3D11DomainShader        CDrxDX12Shader
	#define     ID3D11GeometryShader      CDrxDX12Shader

	struct ID3D11ClassInstance;
	struct ID3D11ClassLinkage;
	struct ID3D11Predicate;
	struct ID3D11Counter;
	struct ID3D11CommandList;
	struct ID3D11BlendState1;
	struct ID3D11RasterizerState1;
	struct ID3D11InfoQueue;
	struct ID3D11ShaderReflection;
	struct ID3DDeviceContextState;

	typedef     IEmptyOutput4             IDXGIOutput4ToImplement;
	typedef     IEmptyAdapter3            IDXGIAdapter3ToImplement;
	typedef     IEmptyFactory4            IDXGIFactory4ToImplement;
	typedef     IEmptySwapChain1          IDXGISwapChain1ToImplement;
	typedef     IEmptySwapChain3          IDXGISwapChain3ToImplement;

	typedef     IEmptyDeviceContext1      ID3D11DeviceContext1ToImplement;
	typedef     IEmptyDevice1             ID3D11Device1ToImplement;
	typedef     IEmptyDeviceContext1      ID3D11DeviceContextToImplement;
	typedef     IEmptyDevice1             ID3D11DeviceToImplement;
	typedef     IEmptyDeviceChild         ID3D11DeviceChildToImplement;
	typedef     IEmptyState               ID3D11BlendStateToImplement;
	typedef     IEmptyState               ID3D11DepthStencilStateToImplement;
	typedef     IEmptyState               ID3D11RasterizerStateToImplement;
	typedef     IEmptyState               ID3D11SamplerStateToImplement;
	typedef     IEmptyState               ID3D11InputLayoutToImplement;
	typedef     IEmptyView                ID3D11ViewToImplement;
	typedef     IEmptyView                ID3D11DepthStencilViewToImplement;
	typedef     IEmptyView                ID3D11RenderTargetViewToImplement;
	typedef     IEmptyView                ID3D11ShaderResourceViewToImplement;
	typedef     IEmptyView                ID3D11UnorderedAccessViewToImplement;
	typedef     IEmptyResource            ID3D11ResourceToImplement;
	typedef     IEmptyResource            ID3D11BufferToImplement;
	typedef     IEmptyResource            ID3D11Texture1DToImplement;
	typedef     IEmptyResource            ID3D11Texture2DToImplement;
	typedef     IEmptyResource            ID3D11Texture3DToImplement;
	typedef     IEmptyAsynchronous        ID3D11AsynchronousToImplement;
	typedef     IEmptyAsynchronous        ID3D11QueryToImplement;

	#include <drx3D/Render/D3D/DX12/DrxDX12.hpp>
	typedef uintptr_t SOCKET;

#elif (DRX_RENDERER_VULKAN >= 10)
	#define     IDXGIAdapter1                        CDrxVKGIAdapter
	#define     IDXGIAdapter                         CDrxVKGIAdapter
	#define     IDXGISwapChain                       CDrxVKSwapChain
	#define     IDXGIOutput                          CDrxVKGIOutput
	#define     IDXGIFactory1                        CDrxVKGIFactory

	#define     ID3D11Resource                       NDrxVulkan::CMemoryResource
	#define     ID3D11Device                         NDrxVulkan::CDevice
	#define     ID3D11DeviceContext                  NDrxVulkan::CRefCounted /* unused */

	#define     ID3D11DeviceChild                    NDrxVulkan::CDeviceObject
	#define     ID3D11View                           NDrxVulkan::CResourceView
	#define     ID3D11BaseTexture                    NDrxVulkan::CImageResource
	#define     ID3D11Texture1D                      NDrxVulkan::CImageResource
	#define     ID3D11Texture2D                      NDrxVulkan::CImageResource
	#define     ID3D11Texture3D                      NDrxVulkan::CImageResource
	#define     ID3D11CommandList                    NDrxVulkan::CRefCounted /* unused */
	#define     ID3D11GeometryShader                 NDrxVulkan::CShader
	#define     ID3D11PixelShader                    NDrxVulkan::CShader
	#define     ID3D11VertexShader                   NDrxVulkan::CShader
	#define     ID3D11HullShader                     NDrxVulkan::CShader
	#define     ID3D11DomainShader                   NDrxVulkan::CShader
	#define     ID3D11ComputeShader                  NDrxVulkan::CShader
	#define     ID3D11UnorderedAccessView            NDrxVulkan::CResourceView
	#define     ID3D11DepthStencilView               NDrxVulkan::CImageView
	#define     ID3D11RenderTargetView               NDrxVulkan::CImageView
	#define     ID3D11ShaderResourceView             NDrxVulkan::CResourceView
	#define     ID3D11InputLayout                    NDrxVulkan::CRefCounted /* unused */
	#define     ID3D11BlendState                     NDrxVulkan::CRefCounted /* unused */
	#define     ID3D11DepthStencilState              NDrxVulkan::CRefCounted /* unused */
	#define     ID3D11RasterizerState                NDrxVulkan::CRefCounted /* unused */
	#define     ID3D11SamplerState                   NDrxVulkan::CSampler
	#define     ID3D11Buffer                         NDrxVulkan::CBufferResource
	#define     ID3D11Query                          NDrxVulkan::CRefCounted /* unused */
	#define     ID3D11Asynchronous                   NDrxVulkan::CRefCounted /* unused */

	#define     ID3DBlob                             CDrxVKBlob
	#define     ID3D11ShaderReflection               CDrxVKShaderReflection
	#define     ID3D11ShaderReflectionConstantBuffer CDrxVKShaderReflectionConstantBuffer
	#define     ID3D11ShaderReflectionType           CDrxVKShaderReflectionType
	#define     ID3D11ShaderReflectionVariable       CDrxVKShaderReflectionVariable

	#include <drx3D/Render/D3D/Vulkan/DrxVulkan.hpp>
	typedef uintptr_t SOCKET;

#elif (DRX_RENDERER_DIRECT3D >= 120)

	#if (DRX_RENDERER_DIRECT3D >= 111)
	    typedef ID3D11DeviceContext1      ID3D11DeviceContext1ToImplement;
	    typedef ID3D11Device1             ID3D11Device1ToImplement;
	#endif

	typedef     ID3D11DeviceContext       ID3D11DeviceContextToImplement;
	typedef     ID3D11Device              ID3D11DeviceToImplement;
	typedef     ID3D11DeviceChild         ID3D11DeviceChildToImplement;
	typedef     ID3D11BlendState          ID3D11BlendStateToImplement;
	typedef     ID3D11DepthStencilState   ID3D11DepthStencilStateToImplement;
	typedef     ID3D11RasterizerState     ID3D11RasterizerStateToImplement;
	typedef     ID3D11SamplerState        ID3D11SamplerStateToImplement;
	typedef     ID3D11InputLayout         ID3D11InputLayoutToImplement;
	typedef     ID3D11DepthStencilView    ID3D11DepthStencilViewToImplement;
	typedef     ID3D11RenderTargetView    ID3D11RenderTargetViewToImplement;
	typedef     ID3D11ShaderResourceView  ID3D11ShaderResourceViewToImplement;
	typedef     ID3D11UnorderedAccessView ID3D11UnorderedAccessViewToImplement;
	typedef     ID3D11Resource            ID3D11ResourceToImplement;
	typedef     ID3D11Buffer              ID3D11BufferToImplement;
    typedef     ID3D11BaseTexture         ID3D11ResourceToImplement;
	typedef     ID3D11Texture1D           ID3D11Texture1DToImplement;
	typedef     ID3D11Texture2D           ID3D11Texture2DToImplement;
	typedef     ID3D11Texture3D           ID3D11Texture3DToImplement;
	typedef     ID3D11Asynchronous        ID3D11AsynchronousToImplement;
	typedef     ID3D11Query               ID3D11QueryToImplement;

	#include <drx3D/Render/D3D/DX12/DrxDX12.hpp>
	typedef uintptr_t SOCKET;

#elif (DRX_RENDERER_DIRECT3D < 120)
//#include <d3d11.h>
	typedef  ID3D11Resource            ID3D11BaseTexture;
	typedef  ID3D11Query               ID3D11Fence;

	#include <drx3D/Render/D3D/DX11/DrxDX11.hpp>
#endif

typedef D3DSamplerState CDeviceSamplerState;
typedef D3DInputLayout  CDeviceInputLayout;
typedef D3DBaseView     CDeviceResourceView;

//////////////////////////////////////////////////////////////////////////
#define MAX_FRAME_LATENCY    3                          // At most 16 - 1 (DXGI limitation)
#define MAX_FRAMES_IN_FLIGHT (MAX_FRAME_LATENCY + 1)    // Current frame and frames buffered by driver/GPU

#if DRX_PLATFORM_DURANGO
	#include <xg.h>
#endif

#if !defined(_RELEASE) || defined(ENABLE_STATOSCOPE_RELEASE)
	#define ENABLE_TEXTURE_STREAM_LISTENER
#endif

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_ORBIS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#define FEATURE_SILHOUETTE_POM
#endif

#if !defined(_RELEASE) && (DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO) && !DRX_RENDERER_OPENGL && \
 (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	#define DX11_ALLOW_D3D_DEBUG_RUNTIME
#endif

#if !defined(_RELEASE) && (DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO) && !DRX_RENDERER_OPENGL &&\
 (DRX_RENDERER_DIRECT3D >= 120)
	#define DX12_ALLOW_D3D_DEBUG_RUNTIME
#endif

#if !DRX_PLATFORM_DURANGO && !DRX_PLATFORM_ORBIS
	#define SUPPORT_DEVICE_INFO
	#if DRX_PLATFORM_WINDOWS
		#define SUPPORT_DEVICE_INFO_MSG_PROCESSING
		#define SUPPORT_DEVICE_INFO_USER_DISPLAY_OVERRIDES
	#endif
#endif

#include <drx3D/Eng3D/I3DEngine.h>


#if (DRX_RENDERER_DIRECT3D >= 120) || DRX_RENDERER_VULKAN
// ConstantBuffer/ShaderResource/UnorderedAccess need markers,
// VertexBuffer/IndexBuffer are fine with discard
	#define D3D11_MAP_WRITE_DISCARD_VB      D3D11_MAP(D3D11_MAP_WRITE_DISCARD)
	#define D3D11_MAP_WRITE_DISCARD_IB      D3D11_MAP(D3D11_MAP_WRITE_DISCARD)
	#define D3D11_MAP_WRITE_DISCARD_CB      D3D11_MAP(D3D11_MAP_WRITE_DISCARD + DX12_MAP_DISCARD_MARKER)
	#define D3D11_MAP_WRITE_DISCARD_SR      D3D11_MAP(D3D11_MAP_WRITE_DISCARD + DX12_MAP_DISCARD_MARKER)
	#define D3D11_MAP_WRITE_DISCARD_UA      D3D11_MAP(D3D11_MAP_WRITE_DISCARD + DX12_MAP_DISCARD_MARKER)

	#define D3D11_MAP_WRITE_NO_OVERWRITE_VB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_IB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_CB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_SR (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_UA (D3D11_MAP_WRITE_NO_OVERWRITE)

	#define D3D11_COPY_NO_OVERWRITE_REVERT  D3D11_COPY_FLAGS(D3D11_COPY_NO_OVERWRITE + DX12_COPY_REVERTSTATE_MARKER)
	#define D3D11_COPY_NO_OVERWRITE_PXLSRV  D3D11_COPY_FLAGS(D3D11_COPY_NO_OVERWRITE + DX12_COPY_PIXELSTATE_MARKER)
	#define D3D11_COPY_NO_OVERWRITE_CONC    D3D11_COPY_FLAGS(D3D11_COPY_NO_OVERWRITE + DX12_COPY_CONCURRENT_MARKER)
	#define D3D11_RESOURCE_MISC_UAV_OVERLAP D3D11_RESOURCE_MISC_FLAG(DX12_RESOURCE_FLAG_OVERLAP)
	#define D3D11_RESOURCE_MISC_HIFREQ_HEAP D3D11_RESOURCE_MISC_FLAG(DX12_RESOURCE_FLAG_HIFREQ_HEAP)
#else
	#define D3D11_MAP_WRITE_DISCARD_VB      (D3D11_MAP_WRITE_DISCARD)
	#define D3D11_MAP_WRITE_DISCARD_IB      (D3D11_MAP_WRITE_DISCARD)
	#define D3D11_MAP_WRITE_DISCARD_CB      (D3D11_MAP_WRITE_DISCARD)
	#define D3D11_MAP_WRITE_DISCARD_SR      (D3D11_MAP_WRITE_DISCARD)
	#define D3D11_MAP_WRITE_DISCARD_UA      (D3D11_MAP_WRITE_DISCARD)

#if DRX_PLATFORM_DURANGO
	#define D3D11_MAP_WRITE_NO_OVERWRITE_VB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_IB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_CB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_SR (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_UA (D3D11_MAP_WRITE_NO_OVERWRITE)
#else
	// NO_OVERWRITE on CBs/SRs-UAs could actually work when we require 11.1
	// and check the feature in D3D11_FEATURE_DATA_D3D11_OPTIONS, but because
	// we would keep using ID3D11DeviceContext::Map (11.0 context) it's
	// possible to use these features even though no 11.1 context is present.
	extern D3D11_MAP D3D11_MAP_WRITE_NO_OVERWRITE_OPTIONAL[3];

	#define D3D11_MAP_WRITE_NO_OVERWRITE_VB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_IB (D3D11_MAP_WRITE_NO_OVERWRITE)
	#define D3D11_MAP_WRITE_NO_OVERWRITE_CB (D3D11_MAP_WRITE_NO_OVERWRITE_OPTIONAL[0])
	#define D3D11_MAP_WRITE_NO_OVERWRITE_SR (D3D11_MAP_WRITE_NO_OVERWRITE_OPTIONAL[1])
	#define D3D11_MAP_WRITE_NO_OVERWRITE_UA (D3D11_MAP_WRITE_NO_OVERWRITE_OPTIONAL[2])
#endif

#if (DRX_RENDERER_DIRECT3D >= 111)
	#define D3D11_COPY_NO_OVERWRITE_REVERT  D3D11_COPY_NO_OVERWRITE
	#define D3D11_COPY_NO_OVERWRITE_PXLSRV  D3D11_COPY_NO_OVERWRITE
	#define D3D11_COPY_NO_OVERWRITE_CONC    D3D11_COPY_NO_OVERWRITE
#else
	#define D3D11_COPY_NO_OVERWRITE_REVERT  (0)
	#define D3D11_COPY_NO_OVERWRITE_PXLSRV  (0)
	#define D3D11_COPY_NO_OVERWRITE_CONC    (0)
#endif

	#define D3D11_RESOURCE_MISC_UAV_OVERLAP D3D11_RESOURCE_MISC_FLAG(0)
	#define D3D11_RESOURCE_MISC_HIFREQ_HEAP D3D11_RESOURCE_MISC_FLAG(DX11_RESOURCE_FLAG_HIFREQ_HEAP)
#endif

#if !defined(USE_D3DX)

// this should be moved into seperate D3DX defintion file which should still be used
// for console builds, until everything in the engine has been cleaned up which still
// references this

typedef struct ID3DXConstTable  ID3DXConstTable;
typedef struct ID3DXConstTable* LPD3DXCONSTANTTABLE;

	#ifndef MAKEFOURCC
		#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                              \
		  ((u32)(u8)(ch0) | ((u32)(u8)(ch1) << 8) | \
		   ((u32)(u8)(ch2) << 16) | ((u32)(u8)(ch3) << 24))
	#endif // defined(MAKEFOURCC)

#endif

i32k g_nD3D10MaxSupportedSubres = (6 * 8 * 64);
//////////////////////////////////////////////////////////////////////////

#define USAGE_WRITEONLY 8

//////////////////////////////////////////////////////////////////////////
// Linux specific defines for Renderer.
//////////////////////////////////////////////////////////////////////////

#if defined(_AMD64_) && !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID
	#include <io.h>
#endif

#if defined(RENDERER_ENABLE_LEGACY_PIPELINE) || defined(DRX_RENDERER_DIRECT3D)
	#include <drx3D/Render/D3D/DeviceUpr/DeviceWrapper_D3D11.h>
	#include <drx3D/Render/D3D/DeviceUpr/DeviceWrapper_D3D11_MemReplay.h>
#else

	// These are dummy objects to replace device wrapper
	struct IDrxDeviceWrapperHook;

	class CDrxDeviceWrapper
	{
	public:
		void AssignDevice(D3DDevice* pDevice) { m_pDevice = pDevice; }
		void ReleaseDevice() { SAFE_RELEASE(m_pDevice); }
		D3DDevice* GetRealDevice() const { assert(false && "Don't use device wrapper without legacy define set!"); return m_pDevice; }
		UINT GetNodeCount() const { return 1; }
		void SwitchNodeVisibility(UINT) {}
		bool IsValid() const { return m_pDevice != nullptr; }
		void RegisterHook(IDrxDeviceWrapperHook*) { assert(false); }
		void UnregisterHook(tukk) { assert(false); }
		HRESULT GetDeviceRemovedReason() const { return DXGI_ERROR_DRIVER_INTERNAL_ERROR; }
	private:
		D3DDevice* m_pDevice = nullptr;
	};

	class CDrxDeviceContextWrapper
	{
	public:
		void AssignDeviceContext(D3DDeviceContext* pDevice) { m_pDevice = pDevice; }
		void ReleaseDeviceContext() { SAFE_RELEASE(m_pDevice); }
		D3DDeviceContext* GetRealDeviceContext() const { assert(false && "Don't use device context wrapper without legacy define set!"); return m_pDevice; }
		i32 GetNodeCount() const { return 1; }
		bool IsValid() const { return m_pDevice != nullptr; }
		void RegisterHook(IDrxDeviceWrapperHook*) { assert(false); }
		void UnregisterHook(tukk) { assert(false); }
		void CopyResourceOvercross(ID3D11Resource*, ID3D11Resource*) { assert(false); }
		void ResetCachedState() {}
	private:
		D3DDeviceContext* m_pDevice = nullptr;
	};
#endif

/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <memory>

//=======================================================================

#ifdef DEBUGALLOC

	#include <crtdbg.h>
	#define DEBUG_CLIENTBLOCK new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new               DEBUG_CLIENTBLOCK

// memman
	#define   calloc(s, t)  _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   malloc(s)     _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   realloc(p, s) _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif

#include <drx3D/CoreX/String/DrxName.h>
#include <drx3D/Render/DrxNameR.h>

#if defined(DRX_PLATFORM_ORBIS)
#define MAX_TMU   32
#else
#define MAX_TMU   64
#endif

//! Include main interfaces.
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/IProcess.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

#include <drx3D/CoreX/Containers/DrxArray.h>

enum ERenderPrimitiveType : i16
{
	eptUnknown                = -1,
	eptTriangleList           = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	eptTriangleStrip          = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	eptLineList               = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	eptLineStrip              = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
	eptPointList              = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
	ept1ControlPointPatchList = D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,
	ept2ControlPointPatchList = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST,
	ept3ControlPointPatchList = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
	ept4ControlPointPatchList = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,

	// non-real primitives, used for logical batching
	eptHWSkinGroups = 0x3f
};

inline ERenderPrimitiveType GetInternalPrimitiveType(PublicRenderPrimitiveType t)
{
	switch (t)
	{
	case prtTriangleList:
	default:
		return eptTriangleList;
	case prtTriangleStrip:
		return eptTriangleStrip;
	case prtLineList:
		return eptLineList;
	case prtLineStrip:
		return eptLineStrip;
	}
}

#define SUPPORT_FLEXIBLE_INDEXBUFFER // supports 16 as well as 32 bit indices AND index buffer bind offset

enum RenderIndexType
{
#if defined(SUPPORT_FLEXIBLE_INDEXBUFFER)
	Index16 = DXGI_FORMAT_R16_UINT,
	Index32 = DXGI_FORMAT_R32_UINT
#else
	Index16,
	Index32
#endif
};

// Interfaces from the Game
extern ILog* iLog;
extern IConsole* iConsole;
extern ITimer* iTimer;
extern ISystem* iSystem;

template<class Container>
unsigned sizeOfVP(Container& arr)
{
	i32 i;
	unsigned size = 0;
	for (i = 0; i < (i32)arr.size(); i++)
	{
		typename Container::value_type& T = arr[i];
		size += T->Size();

	}
	size += (arr.capacity() - arr.size()) * sizeof(typename Container::value_type);
	return size;
}

template<class Container>
unsigned sizeOfV(Container& arr)
{
	i32 i;
	unsigned size = 0;
	for (i = 0; i < (i32)arr.size(); i++)
	{
		typename Container::value_type& T = arr[i];
		size += T.Size();

	}
	size += (arr.capacity() - arr.size()) * sizeof(typename Container::value_type);
	return size;
}
template<class Container>
unsigned sizeOfA(Container& arr)
{
	i32 i;
	unsigned size = 0;
	for (i = 0; i < arr.size(); i++)
	{
		typename Container::value_type& T = arr[i];
		size += T.Size();

	}
	return size;
}
template<class Map>
unsigned sizeOfMap(Map& map)
{
	unsigned size = 0;
	for (typename Map::iterator it = map.begin(); it != map.end(); ++it)
	{
		typename Map::mapped_type& T = it->second;
		size += T.Size();
	}
	size += map.size() * sizeof(stl::MapLikeStruct);
	return size;
}
template<class Map>
unsigned sizeOfMapStr(Map& map)
{
	unsigned size = 0;
	for (typename Map::iterator it = map.begin(); it != map.end(); ++it)
	{
		typename Map::mapped_type& T = it->second;
		size += T.capacity();
	}
	size += map.size() * sizeof(stl::MapLikeStruct);
	return size;
}
template<class Map>
unsigned sizeOfMapP(Map& map)
{
	unsigned size = 0;
	for (typename Map::iterator it = map.begin(); it != map.end(); ++it)
	{
		typename Map::mapped_type& T = it->second;
		size += T->Size();
	}
	size += map.size() * sizeof(stl::MapLikeStruct);
	return size;
}
template<class Map>
unsigned sizeOfMapS(Map& map)
{
	unsigned size = 0;
	for (typename Map::iterator it = map.begin(); it != map.end(); ++it)
	{
		typename Map::mapped_type& T = it->second;
		size += sizeof(T);
	}
	size += map.size() * sizeof(stl::MapLikeStruct);
	return size;
}

#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#define VOLUMETRIC_FOG_SHADOWS
#endif

#if DRX_PLATFORM_WINDOWS && !DRX_RENDERER_OPENGL && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120) && defined(RELEASE)
	#define ENABLE_NULL_D3D11DEVICE
#endif

#if DRX_PLATFORM_DURANGO || DRX_PLATFORM_ORBIS
	#define TEXSTRM_BYTECENTRIC_MEMORY
#endif

// Enable to eliminate DevTextureDataSize calls during stream updates - costs 4 bytes per mip header
#define TEXSTRM_STORE_DEVSIZES

#ifndef TEXSTRM_BYTECENTRIC_MEMORY
	#define TEXSTRM_TEXTURECENTRIC_MEMORY
#endif

#if DRX_PLATFORM_DESKTOP && !DRX_RENDERER_OPENGL && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	#define TEXSTRM_DEFERRED_UPLOAD
#endif

#if DRX_PLATFORM_DESKTOP
	#define TEXSTRM_COMMIT_COOLDOWN
#endif

#if 0 /*|| (DRX_RENDERER_DIRECT3D >= 120)*/
	#define TEXSTRM_ASYNC_UPLOAD
#endif

// Multi-threaded texture uploading, requires UpdateSubresourceRegion() being thread-safe
#if DRX_PLATFORM_DURANGO /*|| (DRX_RENDERER_DIRECT3D >= 120)*/
	#define TEXSTRM_ASYNC_TEXCOPY
#endif

// The below submits the device context state changes and draw commands
// asynchronously via a high priority packet queue.
// Note: please continuously monitor ASYNC_DIP_SYNC profile marker for stalls
#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	#define DURANGO_ENABLE_ASYNC_DIPS 1
#endif

#if DRX_PLATFORM_DURANGO
	#define TEXSTRM_CUBE_DMA_BROKEN
#endif

//#define TEXSTRM_SUPPORT_REACTIVE

#if defined(_RELEASE)
	#define EXCLUDE_RARELY_USED_R_STATS
#endif

#if DRX_PLATFORM_DURANGO && (DRX_RENDERER_DIRECT3D >= 110) && (DRX_RENDERER_DIRECT3D < 120)
	#define DEVRES_USE_PINNING 1
#endif

#define DEVRES_USE_STAGING_POOL 1
#define DEVRES_TRACK_LATENCY 0

#ifdef WIN32
	#define ASSERT_LEGACY_PIPELINE DRX_ASSERT_MESSAGE(0,__func__);
#else
	#define ASSERT_LEGACY_PIPELINE assert(0);
#endif


#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
//#include <drx3D/Render/_Malloc.h>
//#include <drx3D/Render/math.h>
#include <drx3D/CoreX/StlUtils.h>
#include <drx3D/Render/DevBuffer.h>
#include <drx3D/Render/D3D/DeviceUpr/DeviceResources.h>
#include <drx3D/Render/D3D/DeviceUpr/D3D11/DeviceSubmissionQueue_D3D11.h>
#include <drx3D/Render/D3D/DeviceUpr/DeviceObjects.h>

#include <drx3D/CoreX/Renderer/VertexFormats.h>

#include <drx3D/Render/CommonRender.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/Render/Shaders/ShaderComponents.h>
#include <drx3D/Render/Shaders/Shader.h>
//#include <drx3D/Render/XFile/File.h>
//#include <drx3D/Render/Image.h>
#include <drx3D/Render/Shaders/CShader.h>
#include <drx3D/Render/RenderMesh.h>
#include <drx3D/Render/RenderPipeline.h>
#include <drx3D/Render/RenderThread.h>
#include <drx3D/Render/Renderer.h>
#include <drx3D/Render/Textures/Texture.h>
#include <drx3D/Render/Shaders/Parser.h>
#include <drx3D/Render/RenderFrameProfiler.h>
#include <drx3D/Render/Shadow_Renderer.h>
#include <drx3D/Render/DeferredRenderUtils.h>
#include <drx3D/Render/ShadowUtils.h>
#include <drx3D/Render/WaterUtils.h>

#include <drx3D/Render/OcclQuery.h>

// All handled render elements (except common ones included in "RendElement.h>)
#include <drx3D/Render/RendElements/CRELensOptics.h>
#include <drx3D/Render/RendElements/CREMeshImpl.h>

#include <drx3D/Render/PostProcess/PostProcess.h>

#include <drx3D/Render/D3D/GraphicsPipeline/StandardGraphicsPipeline.h>

/*-----------------------------------------------------------------------------
   Vector transformations.
   -----------------------------------------------------------------------------*/

inline void TransformVector(Vec3& out, const Vec3& in, const Matrix44A& m)
{
	out.x = in.x * m(0, 0) + in.y * m(1, 0) + in.z * m(2, 0);
	out.y = in.x * m(0, 1) + in.y * m(1, 1) + in.z * m(2, 1);
	out.z = in.x * m(0, 2) + in.y * m(1, 2) + in.z * m(2, 2);
}

inline void TransformPosition(Vec3& out, const Vec3& in, const Matrix44A& m)
{
	TransformVector(out, in, m);
	out += m.GetRow(3);
}

inline Plane TransformPlaneByUsingAdjointT(const Matrix44A& M, const Matrix44A& TA, const Plane plSrc)
{
	Vec3 newNorm;
	TransformVector(newNorm, plSrc.n, TA);
	newNorm.Normalize();

	if (M.Determinant() < 0.f)
		newNorm *= -1;

	Plane plane;
	Vec3 p;
	TransformPosition(p, plSrc.n * plSrc.d, M);
	plane.Set(newNorm, p | newNorm);

	return plane;
}

inline Matrix44 TransposeAdjoint(const Matrix44A& M)
{
	Matrix44 ta;

	ta(0, 0) = M(1, 1) * M(2, 2) - M(2, 1) * M(1, 2);
	ta(1, 0) = M(2, 1) * M(0, 2) - M(0, 1) * M(2, 2);
	ta(2, 0) = M(0, 1) * M(1, 2) - M(1, 1) * M(0, 2);

	ta(0, 1) = M(1, 2) * M(2, 0) - M(2, 2) * M(1, 0);
	ta(1, 1) = M(2, 2) * M(0, 0) - M(0, 2) * M(2, 0);
	ta(2, 1) = M(0, 2) * M(1, 0) - M(1, 2) * M(0, 0);

	ta(0, 2) = M(1, 0) * M(2, 1) - M(2, 0) * M(1, 1);
	ta(1, 2) = M(2, 0) * M(0, 1) - M(0, 0) * M(2, 1);
	ta(2, 2) = M(0, 0) * M(1, 1) - M(1, 0) * M(0, 1);

	ta(0, 3) = 0.f;
	ta(1, 3) = 0.f;
	ta(2, 3) = 0.f;

	return ta;
}

inline Plane TransformPlane(const Matrix44A& M, const Plane& plSrc)
{
	Matrix44 tmpTA = TransposeAdjoint(M);
	return TransformPlaneByUsingAdjointT(M, tmpTA, plSrc);
}

// Homogeneous plane transform.
inline Plane TransformPlane2(const Matrix34A& m, const Plane& src)
{
	Plane plDst;

	float v0 = src.n.x, v1 = src.n.y, v2 = src.n.z, v3 = src.d;
	plDst.n.x = v0 * m(0, 0) + v1 * m(1, 0) + v2 * m(2, 0);
	plDst.n.y = v0 * m(0, 1) + v1 * m(1, 1) + v2 * m(2, 1);
	plDst.n.z = v0 * m(0, 2) + v1 * m(1, 2) + v2 * m(2, 2);

	plDst.d = v0 * m(0, 3) + v1 * m(1, 3) + v2 * m(2, 3) + v3;

	return plDst;
}

// Homogeneous plane transform.
inline Plane TransformPlane2(const Matrix44A& m, const Plane& src)
{
	Plane plDst;

	float v0 = src.n.x, v1 = src.n.y, v2 = src.n.z, v3 = src.d;
	plDst.n.x = v0 * m(0, 0) + v1 * m(0, 1) + v2 * m(0, 2) + v3 * m(0, 3);
	plDst.n.y = v0 * m(1, 0) + v1 * m(1, 1) + v2 * m(1, 2) + v3 * m(1, 3);
	plDst.n.z = v0 * m(2, 0) + v1 * m(2, 1) + v2 * m(2, 2) + v3 * m(2, 3);

	plDst.d = v0 * m(3, 0) + v1 * m(3, 1) + v2 * m(3, 2) + v3 * m(3, 3);

	return plDst;
}
inline Plane TransformPlane2_NoTrans(const Matrix44A& m, const Plane& src)
{
	Plane plDst;
	TransformVector(plDst.n, src.n, m);
	plDst.d = src.d;

	return plDst;
}

inline Plane TransformPlane2Transposed(const Matrix44A& m, const Plane& src)
{
	Plane plDst;

	float v0 = src.n.x, v1 = src.n.y, v2 = src.n.z, v3 = src.d;
	plDst.n.x = v0 * m(0, 0) + v1 * m(1, 0) + v2 * m(2, 0) + v3 * m(3, 0);
	plDst.n.y = v0 * m(0, 1) + v1 * m(1, 1) + v2 * m(2, 1) + v3 * m(3, 1);
	plDst.n.z = v0 * m(0, 2) + v1 * m(2, 1) + v2 * m(2, 2) + v3 * m(3, 2);

	plDst.d = v0 * m(0, 3) + v1 * m(1, 3) + v2 * m(2, 3) + v3 * m(3, 3);

	return plDst;
}

//===============================================================================================

#define MAX_PATH_LENGTH 512

//////////////////////////////////////////////////////////////////////////
// Report warning to validator.
//////////////////////////////////////////////////////////////////////////
inline void Warning(tukk format, ...) PRINTF_PARAMS(1, 2);
inline void Warning(tukk format, ...)
{
	va_list args;
	va_start(args, format);
	if (iSystem)
		iSystem->WarningV(VALIDATOR_MODULE_RENDERER, VALIDATOR_WARNING, 0, NULL, format, args);
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
// Report warning to validator.
//////////////////////////////////////////////////////////////////////////
inline void LogWarning(tukk format, ...) PRINTF_PARAMS(1, 2);
inline void LogWarning(tukk format, ...)
{
	va_list args;
	va_start(args, format);
	if (iSystem)
		iSystem->WarningV(VALIDATOR_MODULE_RENDERER, VALIDATOR_WARNING, 0, NULL, format, args);
	va_end(args);
}

inline void LogWarningEngineOnly(tukk format, ...) PRINTF_PARAMS(1, 2);
inline void LogWarningEngineOnly(tukk format, ...)
{
	va_list args;
	va_start(args, format);
	if (iSystem)
		iSystem->WarningV(VALIDATOR_MODULE_RENDERER, VALIDATOR_WARNING, VALIDATOR_FLAG_IGNORE_IN_EDITOR, NULL, format, args);
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
// Report warning to validator.
//////////////////////////////////////////////////////////////////////////
inline void FileWarning(tukk filename, tukk format, ...) PRINTF_PARAMS(2, 3);
inline void FileWarning(tukk filename, tukk format, ...)
{
	va_list args;
	va_start(args, format);
	if (iSystem)
		iSystem->WarningV(VALIDATOR_MODULE_RENDERER, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE, filename, format, args);
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
// Report warning to validator.
//////////////////////////////////////////////////////////////////////////
inline void TextureWarning(tukk filename, tukk format, ...) PRINTF_PARAMS(2, 3);
inline void TextureWarning(tukk filename, tukk format, ...)
{
	va_list args;
	va_start(args, format);
	if (iSystem)
		iSystem->WarningV(VALIDATOR_MODULE_RENDERER, VALIDATOR_WARNING, (VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE), filename, format, args);
	va_end(args);
}

inline void TextureError(tukk filename, tukk format, ...)
{
	va_list args;
	va_start(args, format);
	if (iSystem)
		iSystem->WarningV(VALIDATOR_MODULE_RENDERER, VALIDATOR_ERROR, (VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE), filename, format, args);
	va_end(args);
}

inline void _SetVar(tukk szVarName, i32 nVal)
{
	ICVar* var = iConsole->GetCVar(szVarName);
	if (var)
		var->Set(nVal);
	else
	{
		assert(0);
	}
}

inline D3DViewPort RenderViewportToD3D11Viewport(const SRenderViewport &vp)
{
	D3DViewPort viewport = {
		float(vp.x),
		float(vp.y),
		float(vp.width),
		float(vp.height),
		vp.zmin,
		vp.zmax
	};
	return viewport;
}

//=========================================================================================
//
// Normal timing.
//
#define ticks(Timer)   { Timer -= DrxGetTicks(); }
#define unticks(Timer) { Timer += DrxGetTicks() + 34; }

//=============================================================================

#include <drx3D/Render/Defs.h>

#include <drx3D/Render/D3D/DeviceUpr/DeviceCommandList.inl>
#include <drx3D/Render/D3D/DeviceUpr/D3D11/DeviceSubmissionQueue_D3D11.h>
#include <drx3D/Sys/FrameProfiler_JobSystem.h>  // to be removed
