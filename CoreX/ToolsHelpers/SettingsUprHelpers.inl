// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(DRX_PLATFORM)
	#error DRX_PLATFORM is not defined, probably #include "stdafx.h" is missing.
#endif

#if defined(DRX_ENABLE_RC_HELPER)

	#include "SettingsUprHelpers.h"
	#include "EngineSettingsUpr.h"
	#include <DrxSystem/DrxUtils.h>

	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <shellapi.h> //ShellExecuteW()

bool SettingsUprHelpers::Utf16ContainsAsciiOnly(const wchar_t* wstr)
{
	while (*wstr)
	{
		if (*wstr > 127 || *wstr < 0)
		{
			return false;
		}
		++wstr;
	}
	return true;
}

void SettingsUprHelpers::ConvertUtf16ToUtf8(const wchar_t* src, CCharBuffer dst)
{
	if (dst.getSizeInElements() <= 0)
	{
		return;
	}

	if (src[0] == 0)
	{
		dst[0] = 0;
	}
	else
	{
		const int srclen = int(wcslen(src));
		const int dstsize = int(dst.getSizeInBytes());
		const int byteCount = WideCharToMultiByte(
		  CP_UTF8,
		  0,
		  src,
		  srclen,
		  dst.getPtr(), // output buffer
		  dstsize - 1,  // size of the output buffer in bytes
		  NULL,
		  NULL);
		if (byteCount <= 0 || byteCount >= dstsize)
		{
			dst[0] = 0;
		}
		else
		{
			dst[byteCount] = 0;
		}
	}
}

void SettingsUprHelpers::ConvertUtf8ToUtf16(const char* src, CWCharBuffer dst)
{
	if (dst.getSizeInElements() <= 0)
	{
		return;
	}

	if (src[0] == 0)
	{
		dst[0] = 0;
	}
	else
	{
		const int srclen = int(strlen(src));
		const int dstsize = int(dst.getSizeInElements());
		const int charCount = MultiByteToWideChar(
		  CP_UTF8,
		  0,
		  src,
		  srclen,
		  dst.getPtr(), // output buffer
		  dstsize - 1); // size of the output buffer in characters
		if (charCount <= 0 || charCount >= dstsize)
		{
			dst[0] = 0;
		}
		else
		{
			dst[charCount] = 0;
		}
	}
}

void SettingsUprHelpers::GetAsciiFilename(const wchar_t* wfilename, CCharBuffer buffer)
{
	if (buffer.getSizeInElements() <= 0)
	{
		return;
	}

	if (wfilename[0] == 0)
	{
		buffer[0] = 0;
		return;
	}

	if (Utf16ContainsAsciiOnly(wfilename))
	{
		ConvertUtf16ToUtf8(wfilename, buffer);
		return;
	}

	// The path is non-ASCII unicode, so let's resort to short filenames (they are always ASCII-only, I hope)
	wchar_t shortW[MAX_PATH];
	const int bufferCharCount = DRX_ARRAY_COUNT(shortW);
	const int charCount = GetShortPathNameW(wfilename, shortW, bufferCharCount);
	if (charCount <= 0 || charCount >= bufferCharCount)
	{
		buffer[0] = 0;
		return;
	}

	shortW[charCount] = 0;
	if (!Utf16ContainsAsciiOnly(shortW))
	{
		buffer[0] = 0;
		return;
	}

	ConvertUtf16ToUtf8(shortW, buffer);
}

//////////////////////////////////////////////////////////////////////////
CSettingsUprTools::CSettingsUprTools(const wchar_t* szModuleName)
{
	m_pSettingsUpr = new CEngineSettingsUpr(szModuleName);
}

//////////////////////////////////////////////////////////////////////////
CSettingsUprTools::~CSettingsUprTools()
{
	delete m_pSettingsUpr;
}

//////////////////////////////////////////////////////////////////////////
void CSettingsUprTools::GetRootPathUtf16(bool pullFromRegistry, SettingsUprHelpers::CWCharBuffer wbuffer)
{
	if (pullFromRegistry)
	{
		m_pSettingsUpr->GetRootPathUtf16(wbuffer);
	}
	else
	{
		m_pSettingsUpr->GetValueByRef("ENG_RootPath", wbuffer);
	}
}

void CSettingsUprTools::GetRootPathAscii(bool pullFromRegistry, SettingsUprHelpers::CCharBuffer buffer)
{
	wchar_t wbuffer[MAX_PATH];

	GetRootPathUtf16(pullFromRegistry, SettingsUprHelpers::CWCharBuffer(wbuffer, sizeof(wbuffer)));

	SettingsUprHelpers::GetAsciiFilename(wbuffer, buffer);
}

//////////////////////////////////////////////////////////////////////////
bool CSettingsUprTools::GetInstalledBuildPathUtf16(const int index, SettingsUprHelpers::CWCharBuffer name, SettingsUprHelpers::CWCharBuffer path)
{
	return m_pSettingsUpr->GetInstalledBuildRootPathUtf16(index, name, path);
}

bool CSettingsUprTools::GetInstalledBuildPathAscii(const int index, SettingsUprHelpers::CCharBuffer name, SettingsUprHelpers::CCharBuffer path)
{
	wchar_t wName[MAX_PATH];
	wchar_t wPath[MAX_PATH];
	if (GetInstalledBuildPathUtf16(index, SettingsUprHelpers::CWCharBuffer(wName, sizeof(wName)), SettingsUprHelpers::CWCharBuffer(wPath, sizeof(wPath))))
	{
		SettingsUprHelpers::GetAsciiFilename(wName, name);
		SettingsUprHelpers::GetAsciiFilename(wPath, path);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CSettingsUprTools::CallSettingsUprExe(void* hParent)
{
	m_pSettingsUpr->CallSettingsDialog(hParent);
}

#endif // #if defined(DRX_ENABLE_RC_HELPER)

// eof
