// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if defined(INCLUDE_SCALEFORM_SDK)

	#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
// include debug libraries for scaleform
		#if defined(USE_GFX_JPG)
LINK_SYSTEM_LIBRARY("libjpeg.lib")
		#endif
		#if defined(USE_GFX_VIDEO)
LINK_SYSTEM_LIBRARY("libgfx_video.lib")
		#endif
		#if defined(USE_GFX_PNG)
LINK_SYSTEM_LIBRARY("libpng.lib")
		#endif
		#if defined(USE_GFX_IME)
LINK_SYSTEM_LIBRARY("libgfx_ime.lib")
		#endif
	#elif DRX_PLATFORM_ORBIS || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	#else
		#error Unknown ScaleForm configuration selected
	#endif

// IME dependencies
	#if DRX_PLATFORM_WINDOWS && defined(USE_GFX_IME)
LINK_SYSTEM_LIBRARY("imm32.lib")
LINK_SYSTEM_LIBRARY("oleaut32.lib")
	#endif

#endif // #ifdef INCLUDE_SCALEFORM_SDK
