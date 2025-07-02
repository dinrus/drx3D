// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Network
#define DRXNETWORK_EXPORTS
#include <drx3D/CoreX/Platform/platform.h>

#if !defined(_DEBUG)
	#define DRXNETWORK_RELEASEBUILD 1
#else
	#define DRXNETWORK_RELEASEBUILD 0
#endif // #if !defined(_DEBUG)

#define NOT_USE_UBICOM_SDK

#include <stdio.h>
#include <stdarg.h>
#include <map>
#include <algorithm>

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/CoreX/StlUtils.h>

#include <drx3D/CoreX/Renderer/IRenderer.h>

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#include <unistd.h>
	#include <fcntl.h>
#endif

#include <memory>
#include <vector>

#if DRX_PLATFORM_WINAPI
	#include <process.h>
	#define S_ADDR_IP4(ADDR) ((ADDR).sin_addr.S_un.S_addr)
#else
	#define S_ADDR_IP4(ADDR) ((ADDR).sin_addr.s_addr)
#endif

//#if defined(_DEBUG) && DRX_PLATFORM_WINDOWS
//#include <crtdbg.h>
//#endif

#define NET_ASSERT_LOGGING 1

#if DRXNETWORK_RELEASEBUILD
	#undef NET_ASSERT_LOGGING
	#define NET_ASSERT_LOGGING 0
#endif
void NET_ASSERT_FAIL(tukk check, tukk file, i32 line);
#undef NET_ASSERT
#if NET_ASSERT_LOGGING
	#define NET_ASSERT(x) if (true) { bool net_ok = true; if (!(x)) net_ok = false; DRX_ASSERT_MESSAGE(net_ok, # x); if (!net_ok) NET_ASSERT_FAIL( # x, __FILE__, __LINE__); } else
#else
	#define NET_ASSERT assert
#endif

#ifndef NET____TRACE
	#define NET____TRACE

// src and trg can be the same pointer (in place encryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit:  i32 key[4] = {n1,n2,n3,n4};
// void encipher(u32 *const v,u32 *const w,u32k *const k )
	#define TEA_ENCODE(src, trg, len, key) {                                                                                      \
	  u32* v = (src), * w = (trg), * k = (key), nlen = (len) >> 3;                                                       \
	  u32 delta = 0x9E3779B9, a = k[0], b = k[1], c = k[2], d = k[3];                                                    \
	  while (nlen--) {                                                                                                            \
	    u32 y = v[0], z = v[1], n = 32, sum = 0;                                                                         \
	    while (n-- > 0) { sum += delta; y += (z << 4) + a ^ z + sum ^ (z >> 5) + b; z += (y << 4) + c ^ y + sum ^ (y >> 5) + d; } \
	    w[0] = y; w[1] = z; v += 2, w += 2; }                                                                                     \
}

// src and trg can be the same pointer (in place decryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit: i32 key[4] = {n1,n2,n3,n4};
// void decipher(u32 *const v,u32 *const w,u32k *const k)
	#define TEA_DECODE(src, trg, len, key) {                                                                                      \
	  u32* v = (src), * w = (trg), * k = (key), nlen = (len) >> 3;                                                       \
	  u32 delta = 0x9E3779B9, a = k[0], b = k[1], c = k[2], d = k[3];                                                    \
	  while (nlen--) {                                                                                                            \
	    u32 y = v[0], z = v[1], sum = 0xC6EF3720, n = 32;                                                                \
	    while (n-- > 0) { z -= (y << 4) + c ^ y + sum ^ (y >> 5) + d; y -= (z << 4) + a ^ z + sum ^ (z >> 5) + b; sum -= delta; } \
	    w[0] = y; w[1] = z; v += 2, w += 2; }                                                                                     \
}

// encode size ignore last 3 bits of size in bytes. (encode by 8bytes min)
	#define TEA_GETSIZE(len) ((len) & (~7))

inline void __cdecl __NET_TRACE(tukk sFormat, ...) PRINTF_PARAMS(1, 2);

inline void __cdecl __NET_TRACE(tukk sFormat, ...)
{
	/*
	   va_list args;
	   static char sTraceString[500];

	   va_start(args, sFormat);
	   drx_vsprintf(sTraceString, sFormat, args);
	   va_end(args);

	   NET_ASSERT(strlen(sTraceString) < 500)

	   NetLog(sTraceString);

	   ::OutputDebugString(sTraceString);*/
}

	#if 1

		#define NET_TRACE __NET_TRACE

	#else

		#define NET_TRACE 1 ? (void)0 : __NET_TRACE;

	#endif //NET____TRACE

#endif //_DEBUG

struct IStreamAllocator
{
	virtual ~IStreamAllocator(){}
	virtual uk Alloc(size_t size, uk callerOverride = 0) = 0;
	virtual uk Realloc(uk old, size_t size) = 0;
	virtual void  Free(uk old) = 0;
};

class CDefaultStreamAllocator : public IStreamAllocator
{
	uk Alloc(size_t size, uk )       { return malloc(size); }
	uk Realloc(uk old, size_t size) { return realloc(old, size); }
	void  Free(uk old)                 { free(old); }
};

#include <drx3D/Sys/IValidator.h>   // MAX_WARNING_LENGTH
#include <drx3D/Sys/ISystem.h>      // NetLog
#include <drx3D/Network/NetLog.h>

#if _MSC_VER > 1000
	#pragma intrinsic(memcpy)
#endif

#include <drx3D/Network/INetwork.h>

#ifdef __WITH_PB__
	#include <drx3D/Network/PunkBuster/pbcommon.h>
#endif

#include <drx3D/Network/objcnt.h>

#if defined(_MSC_VER)
extern "C" uk _ReturnAddress(void);
	#pragma intrinsic(_ReturnAddress)
	#define UP_STACK_PTR _ReturnAddress()
#else
	#define UP_STACK_PTR 0
#endif

#include <drx3D/CoreX/Platform/DrxWindows.h>
// All headers below include <windows.h> not via DrxWindows.h
// This should be fixed
#include <drx3D/Network/ArithAlphabet.h>
#include <drx3D/Network/ClientContextView.h>
#include <drx3D/Network/ContextView.h>
#include <drx3D/Network/NetChannel.h>
