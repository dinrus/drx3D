// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if DRX_PLATFORM_WINAPI
	#define DX12_STRINGIFY(_STRING) # _STRING

	#define DX12_GUID_STRING(_D0, _W0, _W1, _B0, _B1, _B2, _B3, _B4, _B5, _B6, _B7) \
		  DX12_STRINGIFY(_D0 ## - ## _W0 ## - ## _W1 ## - ## _B0 ## _B1 ## - ## _B2 ## _B3 ## _B4 ## _B5 ## _B6 ## _B7)

	#define DX12_DEFINE_TYPE_GUID(_CLASS, _TYPE, _D0, _W0, _W1, _B0, _B1, _B2, _B3, _B4, _B5, _B6, _B7) \
		  _CLASS __declspec(uuid(DX12_GUID_STRING(_D0, _W0, _W1, _B0, _B1, _B2, _B3, _B4, _B5, _B6, _B7))) _TYPE;
#endif

#if !DX11_COM_INTERFACES
	class CDrxDX12Device;
	class CDrxDX12DeviceContext;
	class CDrxDX12BlendState;
	class CDrxDX12DepthStencilState;
	class CDrxDX12RasterizerState;
	class CDrxDX12SamplerState;
	template<typename T> class CDrxDX12Resource;
	class CDrxDX12Buffer;
	class CDrxDX12Texture1D;
	class CDrxDX12Texture2D;
	class CDrxDX12Texture3D;
	template<typename T> class CDrxDX12View;
	class CDrxDX12DepthStencilView;
	class CDrxDX12RenderTargetView;
	class CDrxDX12ShaderResourceView;
	class CDrxDX12UnorderedAccessView;
	template<typename T> class CDrxDX12Asynchronous;
	class CDrxDX12Query;
	class CDrxDX12InputLayout;

	class CDrxDX12GIOutput;
	class CDrxDX12GIAdapter;
	class CDrxDX12GIFactory;
	class CDrxDX12SwapChain;

	#if DRX_PLATFORM_WINAPI
		// *INDENT-OFF*
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12Device,              DB6F6DDB, AC77, 4E88, 82, 53, 81, 9D, F9, BB, F1, 40)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12DeviceContext,       C0BFA96C, E089, 44FB, 8E, AF, 26, F8, 79, 61, 90, DA)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12BlendState,          75B68FAA, 347D, 4159, 8F, 45, A0, 64, 0F, 01, CD, 9A)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12DepthStencilState,   03823EFB, 8D8F, 4E1C, 9A, A2, F6, 4B, B2, CB, FD, F1)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12RasterizerState,     9BB4AB81, AB1A, 4D8F, B5, 06, FC, 04, 20, 0B, 6E, E7)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12SamplerState,        DA6FEA51, 564C, 4487, 98, 10, F0, D0, F9, B4, E3, A5)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12Buffer,              48570B85, D1EE, 4FCD, A2, 50, EB, 35, 07, 22, B0, 37)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12Texture1D,           F8FB5C27, C6B3, 4F75, A4, C8, 43, 9A, F2, EF, 56, 4C)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12Texture2D,           6F15AAF2, D208, 4E89, 9A, B4, 48, 95, 35, D3, 4F, 9C)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12Texture3D,           037E866E, F56D, 4357, A8, AF, 9D, AB, BE, 6E, 25, 0E)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12DepthStencilView,    9FDAC92A, 1876, 48C3, AF, AD, 25, B9, 4F, 84, A9, B6)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12RenderTargetView,    DFDBA067, 0B8D, 4865, 87, 5B, D7, B4, 51, 6C, C1, 64)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12ShaderResourceView,  B0E06FE0, 8192, 4E1A, B1, CA, 36, D7, 41, 47, 10, B2)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12UnorderedAccessView, 28ACF509, 7F5C, 48F6, 86, 11, F3, 16, 01, 0A, 63, 80)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12Query,               D6C00747, 87B7, 425E, B8, 4D, 44, D1, 08, 56, 0A, FD)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12InputLayout,         E4819DDC, 4CF0, 4025, BD, 26, 5D, E8, 2A, 3E, 07, B7)

		DX12_DEFINE_TYPE_GUID(class, CDrxDX12GIOutput,            DC7DCA35, 2196, 414D, 9F, 53, 61, 78, 84, 03, 2A, 60)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12GIAdapter,           645967A4, 1392, 4310, A7, 98, 80, 53, CE, 3E, 93, FD)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12GIFactory,           1BC6EA02, EF36, 464F, BF, 0C, 21, CA, 39, E5, 16, 8A)
		DX12_DEFINE_TYPE_GUID(class, CDrxDX12SwapChain,           94D99BDB, F1F8, 4AB0, B2, 36, 7D, A0, 17, 0E, DA, B1)
		// *INDENT-ON*
	#endif
#endif
