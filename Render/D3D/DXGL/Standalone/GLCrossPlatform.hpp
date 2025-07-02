// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GLCrossPlatform.hpp
//  Version:     v1.00
//  Created:     27/03/2014 by Valerio Guagliumi.
//  Описание: Cross platform DXGL helper types
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GLCROSSPLATFORM__
#define __GLCROSSPLATFORM__

#include <algorithm>
#include <cfloat>

using namespace std;

#if defined(_MSC_VER)
	#define ILINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
	#define ILINE __attribute__((always_inline))
#else
	#define ILINE inline
#endif

#define IF_LIKELY   if
#define IF_UNLIKELY if

#if !defined(_MSC_VER)

inline i32 sprintf_s(tuk buffer, size_t size, tukk format, ...)
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	va_list args;
	va_start(args, format);
	i32 err = vsnprintf(buffer, size, format, args);
	va_end(args);
	return err;
	#else
		#error "На этой платформе не реализовано"
	#endif
}

template<size_t size>
i32 sprintf_s(char (&buffer)[size], tukk format, ...)
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	va_list args;
	va_start(args, format);
	i32 err = vsnprintf(buffer, size, format, args);
	va_end(args);
	return err;
	#else
		#error "На этой платформе не реализовано"
	#endif
}

inline i32 strcpy_s(tuk dst, size_t size, tukk src)
{
	#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	strncpy(dst, src, size);
	dst[size - 1] = 0;
	return 0;
	#else
		#error "На этой платформе не реализовано"
	#endif
}

template<size_t SIZE>
i32 strcpy_s(char (&dst)[SIZE], tukk src)
{
	return strcpy_s(dst, SIZE, src);
}

inline void ZeroMemory(uk pPtr, i32 nSize)
{
	memset(pPtr, 0, nSize);
}

#endif //!defined(_MSC_VER)

namespace NDrxOpenGL
{

inline uk Malloc(size_t size)
{
	return malloc(size)
}

inline uk Calloc(size_t num, size_t size)
{
	return calloc(num, size)
}

inline uk Realloc(uk memblock, size_t size)
{
	return realloc(memblock, size)
}

inline void Free(uk memblock)
{
	free(memblock)
}

uk CreateTLS();
void  SetTLSValue(uk pTLSHandle, uk pValue);
uk GetTLSValue(uk pTLSHandle);
void  DestroyTLS(uk pTLSHandle);
bool  MakeDir(tukk szDirName);

namespace NCrossPlatformImpl
{

struct SAutoLog
{
	FILE* m_pFile;

	SAutoLog(tukk szFileName)
	{
		m_pFile = fopen("DXGL.log", "w");
	}

	~SAutoLog()
	{
		if (m_pFile != NULL)
			fclose(m_pFile);
	}
};

struct SAutoTLSSlot
{
	uk m_pTLSHandle;

	SAutoTLSSlot()
	{
		m_pTLSHandle = CreateTLS();
	}

	~SAutoTLSSlot()
	{
		DestroyTLS(m_pTLSHandle);
	}
};

inline u32 CRC32Reflect(u32 ref, char ch)
{
	u32 value = 0;

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for (i32 i = 1; i < (ch + 1); i++)
	{
		if (ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

extern SAutoLog g_kLog;
extern SAutoTLSSlot g_kCRCTable;

}

#define PRINTF_PARAMS(a, b)

void        LogMessage(ELogSeverity eSeverity, tukk szFormat, ...) PRINTF_PARAMS(2, 3);
inline void LogMessage(ELogSeverity eSeverity, tukk szFormat, ...)
{
	if (NCrossPlatformImpl::g_kLog.m_pFile != NULL)
	{
		va_list kArgs;
		va_start(kArgs, szFormat);
		vfprintf(NCrossPlatformImpl::g_kLog.m_pFile, szFormat, kArgs);
		va_end(kArgs);
		fputc('\n', NCrossPlatformImpl::g_kLog.m_pFile);
		fflush(NCrossPlatformImpl::g_kLog.m_pFile);
	}

	if (eSeverity == eLS_Fatal)
		std::exit();
}

inline u32 GetCRC32(tukk pData, size_t uSize, u32 uCRC)
{
	u32* pCRCTable(static_cast<u32*>(GetTLSValue(NCrossPlatformImpl::g_kCRCTable.m_pTLSHandle)));
	if (pCRCTable == NULL)
	{
		pCRCTable = new u32[256];
		SetTLSValue(NCrossPlatformImpl::g_kCRCTable.m_pTLSHandle, pCRCTable);

		// This is the official polynomial used by CRC-32
		// in PKZip, WinZip and Ethernet.
		u32 ulPolynomial = 0x04c11db7;

		// 256 values representing ASCII character codes.
		for (i32 i = 0; i <= 0xFF; i++)
		{
			pCRCTable[i] = NCrossPlatformImpl::CRC32Reflect(i, 8) << 24;
			for (i32 j = 0; j < 8; j++)
				pCRCTable[i] = (pCRCTable[i] << 1) ^ (pCRCTable[i] & (1U << 31) ? ulPolynomial : 0);
			pCRCTable[i] = NCrossPlatformImpl::CRC32Reflect(pCRCTable[i], 32);
		}
	}

	i32 len;
	u8* buffer;

	// Get the length.
	len = uSize;

	// Save the text in the buffer.
	buffer = (u8*)pData;
	// Perform the algorithm on each character in the string, using the lookup table values.

	while (len--)
		uCRC = (uCRC >> 8) ^ pCRCTable[(uCRC & 0xFF) ^ *buffer++];
	// Exclusive OR the result with the beginning value.
	return uCRC ^ 0xffffffff;
}

using STxt;

struct STraceFile
{
	STraceFile()
		: m_pFile(NULL)
	{
	}

	~STraceFile()
	{
		if (m_pFile != NULL)
			fclose(m_pFile);
	}

	bool Open(tukk szFileName, bool bBinary)
	{
		if (m_pFile != NULL)
			return false;

		tukk szDirName = "DXGLTrace";
		do
		{
			char acFullPath[256];
			sprintf_s(acFullPath, "%s/%s", szDirName, szFileName);
			tukk szMode(bBinary ? "wb" : "w");
			m_pFile = fopen(acFullPath, szMode);
			if (m_pFile != NULL)
				return true;
		}
		while (MakeDir(szDirName));

		return false;
	}

	void Write(ukk pvData, u32 uSize)
	{
		fwrite(pvData, (size_t)uSize, 1, m_pFile);
	}

	void Printf(tukk szFormat, ...)
	{
		va_list kArgs;
		va_start(kArgs, szFormat);
		vfprintf(m_pFile, szFormat, kArgs);
		va_end(kArgs);
	}

	FILE* m_pFile;
};

inline void RegisterConfigVariable(tukk , i32* piVariable, i32 iValue)
{
	*piVariable = iValue;
}

#define IL2VAL(mask, shift)       \
  c |= ((x & mask) != 0) * shift; \
  x >>= ((x & mask) != 0) * shift

inline u32 IntegerLog2(u32 x)
{
	u32 c = 0;
	IL2VAL(0xffff0000u, 16);
	IL2VAL(0xff00, 8);
	IL2VAL(0xf0, 4);
	IL2VAL(0xc, 2);
	IL2VAL(0x2, 1);
	return c;
}

#undef IL2VAL

template<typename T>
inline size_t countTrailingZeroes(T v)
{
	size_t n = 0;

	v = ~v & (v - 1);
	while (v)
	{
		++n;
		v >>= 1;
	}

	return n;
}

template<typename DestinationType, typename SourceType>
inline DestinationType alias_cast(SourceType pPtr)
{
	union
	{
		SourceType      pSrc;
		DestinationType pDst;
	} conv_union;
	conv_union.pSrc = pPtr;
	return conv_union.pDst;
}

inline void Memcpy(uk pDst, ukk pSrc, size_t uLength)
{
	memcpy(pDst, pSrc, uLength);
}

inline void PushProfileLabel(tukk szName)
{
	DXGLProfileLabelPush(szName);
}

inline void PopProfileLabel(tukk szName)
{
	DXGLProfileLabelPop(szName);
}

}

#endif //__GLCROSSPLATFORM__
