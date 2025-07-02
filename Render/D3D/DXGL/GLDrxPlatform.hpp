// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GLDrxPlatform.hpp
//  Version:     v1.00
//  Created:     27/03/2014 by Valerio Guagliumi.
//  Описание: Platform specific DXGL requirements implementation relying
//               on DrxCommon and DinrusSystem.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GLDRXPLATFORM__
#define __GLDRXPLATFORM__

#include <drx3D/Render/DriverD3D.h>

namespace NDrxOpenGL
{

inline uk Malloc(size_t size)
{
	return DrxModuleMalloc(size);
}

inline uk Calloc(size_t num, size_t size)
{
	return DrxModuleCalloc(num, size);
}

inline uk Realloc(uk memblock, size_t size)
{
	return DrxModuleRealloc(memblock, size);
}

inline void Free(uk memblock)
{
	DrxModuleFree(memblock);
}

inline uk Memalign(size_t size, size_t alignment)
{
	return DrxModuleMemalign(size, alignment);
}

inline void MemalignFree(uk memblock)
{
	return DrxModuleMemalignFree(memblock);
}

void        LogMessage(ELogSeverity eSeverity, tukk szFormat, ...) PRINTF_PARAMS(2, 3);

inline void LogMessage(ELogSeverity eSeverity, tukk szFormat, ...)
{
	va_list kArgs;
	va_start(kArgs, szFormat);
	if (eSeverity == eLS_Fatal)
	{
		char szBuffer[4096];
		drx_vsprintf(szBuffer, szFormat, kArgs);
		GetISystem()->FatalError(szBuffer);
	}
	else
	{
		EValidatorSeverity eValidatorSeverity(VALIDATOR_COMMENT);
		switch (eSeverity)
		{
		case eLS_Error:
			eValidatorSeverity = VALIDATOR_ERROR;
			break;
		case eLS_Warning:
			eValidatorSeverity = VALIDATOR_WARNING;
			break;
		case eLS_Info:
			eValidatorSeverity = VALIDATOR_COMMENT;
			break;
		}
		GetISystem()->WarningV(VALIDATOR_MODULE_RENDERER, eValidatorSeverity, 0, 0, szFormat, kArgs);
	}
	va_end(kArgs);
}

inline void BreakUnique(tukk szFile, u32 uLine)
{
	LogMessage(eLS_Warning, "Break at %s(%d)", szFile, uLine);
	DrxDebugBreak();
}

inline u32 GetCRC32(ukk pData, size_t uSize)
{
	return CCrc32::Compute(pData, uSize);
}

inline LONG Exchange(LONG * piDestination, LONG iExchange)
{
#if DRX_PLATFORM_WINDOWS
	return InterlockedExchange(piDestination, iExchange);
#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	LONG iOldValue(__sync_lock_test_and_set(piDestination, iExchange));
	__sync_synchronize();
	return iOldValue;
#else
	#error "Not implemented on this platform"
#endif
}

inline LONG CompareExchange(LONG * piDestination, LONG iExchange, LONG iComparand)
{
	return DrxInterlockedCompareExchange(piDestination, iExchange, iComparand);
}

inline LONG AtomicIncrement(LONG * piDestination)
{
	return DrxInterlockedIncrement(reinterpret_cast<i32 *>(piDestination));
}

inline LONG AtomicDecrement(LONG * piDestination)
{
	return DrxInterlockedDecrement(reinterpret_cast<i32 *>(piDestination));
}

typedef DrxCriticalSection TCriticalSection;

inline void LockCriticalSection(TCriticalSection* pCriticalSection)
{
	pCriticalSection->Lock();
}

inline void UnlockCriticalSection(TCriticalSection* pCriticalSection)
{
	pCriticalSection->Unlock();
}

struct STraceFile
{
	STraceFile()
		: m_pFile(NULL)
	{
	}

	~STraceFile()
	{
		if (m_pFile != NULL)
			gEnv->pDrxPak->FClose(m_pFile);
	}

	bool Open(tukk szFileName, bool bBinary)
	{
		static tukk s_szTraceDirectory = "DXGLTrace";

		if (m_pFile != NULL)
			return false;

		char acFullPath[MAX_PATH];
		drx_sprintf(acFullPath, "%%USER%%/%s/%s", s_szTraceDirectory, szFileName);
		tukk szMode(bBinary ? "wb" : "w");
		m_pFile = gEnv->pDrxPak->FOpen(acFullPath, szMode, IDrxPak::FLAGS_FOR_WRITING);
		if (m_pFile != NULL)
			return true;

		gEnv->pDrxPak->MakeDir(s_szTraceDirectory, 0);
		m_pFile = gEnv->pDrxPak->FOpen(acFullPath, szMode);
		return m_pFile != NULL;
	}

	void Write(ukk pvData, u32 uSize)
	{
		gEnv->pDrxPak->FWrite(pvData, (size_t)uSize, 1, m_pFile);
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

inline void RegisterConfigVariable(tukk szName, i32* piVariable, i32 iDefaultValue)
{
	REGISTER_CVAR2(szName, piVariable, iDefaultValue, 0, "");
}

inline void Memcpy(uk pDst, ukk pSrc, size_t uLength)
{
	drxMemcpy(pDst, pSrc, uLength);
}

inline void PushProfileLabel(tukk szName)
{
	PROFILE_LABEL_PUSH(szName);
}

inline void PopProfileLabel(tukk szName)
{
	PROFILE_LABEL_POP(szName);
}

}

#endif //__GLDRXPLATFORM__
