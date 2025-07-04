// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DrxWindows.h"

//////////////////////////////////////////////////////////////////////////
bool DrxCreateDirectory(const char* lpPathName)
{
	// Convert from UTF-8 to UNICODE
	wstring widePath;
	Unicode::Convert(widePath, lpPathName);

	const DWORD dwAttr = ::GetFileAttributesW(widePath.c_str());
	if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return true;
	}

	return ::CreateDirectoryW(widePath.c_str(), 0) != 0;
}

//////////////////////////////////////////////////////////////////////////
bool DrxDirectoryExists(const char* szPath)
{
	// Convert from UTF-8 to UNICODE
	wstring widePath;
	Unicode::Convert(widePath, szPath);

	const DWORD dwAttr = ::GetFileAttributesW(widePath.c_str());
	if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void DrxGetCurrentDirectory(unsigned int nBufferLength, char* lpBuffer)
{
	if (nBufferLength <= 0 || !lpBuffer)
	{
		return;
	}

	*lpBuffer = 0;

	// Get directory in UTF-16
	std::vector<wchar_t> buffer;
	{
		const size_t requiredLength = ::GetCurrentDirectoryW(0, 0);

		if (requiredLength <= 0)
		{
			return;
		}

		buffer.resize(requiredLength, 0);

		if (::GetCurrentDirectoryW(requiredLength, &buffer[0]) != requiredLength - 1)
		{
			return;
		}
	}

	// Convert to UTF-8
	if (Unicode::Convert<Unicode::eEncoding_UTF8, Unicode::eEncoding_UTF16>(lpBuffer, nBufferLength, &buffer[0]) > nBufferLength)
	{
		*lpBuffer = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void DrxSetCurrentWorkingDirectory(const char* szWorkingDirectory)
{
	SetCurrentDirectoryW(Unicode::Convert<wstring>(szWorkingDirectory).c_str());
}

//////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/String/Path.h>
void DrxGetExecutableFolder(unsigned int pathSize, char* szPath)
{
	WCHAR filePath[512];
	size_t nLen = GetModuleFileNameW(DrxGetCurrentModule(), filePath, DRX_ARRAY_COUNT(filePath));

	if (nLen >= DRX_ARRAY_COUNT(filePath))
	{
		DrxFatalError("The path to the current executable exceeds the expected length. TruncatedPath:%s", filePath);
	}

	if (nLen <= 0)
	{
		DrxFatalError("Unexpected error encountered trying to get executable path. GetModuleFileNameW failed.");
	}

	if (wchar_t* pathEnd = wcsrchr(filePath, L'\\'))
	{
		pathEnd[1] = L'\0';
	}

	size_t requiredLength = Unicode::Convert(szPath, pathSize, filePath);
	if (requiredLength > pathSize)
	{
		DrxFatalError("Executable path is to long. MaxPathSize:%u, PathSize:%u, Path:%s", pathSize, (uint)requiredLength, filePath);
	}
}

//////////////////////////////////////////////////////////////////////////
int DrxGetWritableDirectory(unsigned int nBufferLength, char* lpBuffer)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
short DrxGetAsyncKeyState(int vKey)
{
#if DRX_PLATFORM_WINDOWS
	return GetAsyncKeyState(vKey);
#else
	return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
void* DrxCreateCriticalSection()
{
	CRITICAL_SECTION* pCS = new CRITICAL_SECTION;
	InitializeCriticalSection(pCS);
	return pCS;
}

void DrxCreateCriticalSectionInplace(void* pCS)
{
	InitializeCriticalSection((CRITICAL_SECTION*)pCS);
}
//////////////////////////////////////////////////////////////////////////
void DrxDeleteCriticalSection(void* cs)
{
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*)cs;
	if (pCS->LockCount >= 0)
		DrxFatalError("Critical Section hanging lock");
	DeleteCriticalSection(pCS);
	delete pCS;
}

//////////////////////////////////////////////////////////////////////////
void DrxDeleteCriticalSectionInplace(void* cs)
{
	CRITICAL_SECTION* pCS = (CRITICAL_SECTION*)cs;
	if (pCS->LockCount >= 0)
		DrxFatalError("Critical Section hanging lock");
	DeleteCriticalSection(pCS);
}

//////////////////////////////////////////////////////////////////////////
void DrxEnterCriticalSection(void* cs)
{
	EnterCriticalSection((CRITICAL_SECTION*)cs);
}

//////////////////////////////////////////////////////////////////////////
bool DrxTryCriticalSection(void* cs)
{
	return TryEnterCriticalSection((CRITICAL_SECTION*)cs) != 0;
}

//////////////////////////////////////////////////////////////////////////
void DrxLeaveCriticalSection(void* cs)
{
	LeaveCriticalSection((CRITICAL_SECTION*)cs);
}

//////////////////////////////////////////////////////////////////////////
u32 DrxGetFileAttributes(const char* lpFileName)
{
	// Normal GetFileAttributes not available anymore in non desktop applications (eg Durango)
	WIN32_FILE_ATTRIBUTE_DATA data;
	BOOL res;
#if DRX_PLATFORM_DURANGO
	res = GetFileAttributesExA(lpFileName, GetFileExInfoStandard, &data);
#else
	wstring widePath;
	Unicode::Convert(widePath, lpFileName);
	res = GetFileAttributesExW(widePath.c_str(), GetFileExInfoStandard, &data);
#endif
	return res != 0 ? data.dwFileAttributes : -1;
}

//////////////////////////////////////////////////////////////////////////
bool DrxSetFileAttributes(const char* lpFileName, u32 dwFileAttributes)
{
#if DRX_PLATFORM_DURANGO
	return SetFileAttributesA(lpFileName, dwFileAttributes) != 0;
#else
	wstring widePath;
	Unicode::Convert(widePath, lpFileName);
	return SetFileAttributesW(widePath.c_str(), dwFileAttributes) != 0;
#endif
}

namespace
{
	unsigned int GetMessageBoxType(EMessageBox type)
	{
		switch (type)
		{
		case eMB_YesCancel:
			return MB_OKCANCEL;
		case eMB_YesNoCancel:
			return MB_YESNOCANCEL;
			break;
		case eMB_Error:
			return MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL;
			break;
		case eMB_AbortRetryIgnore:
			return MB_ABORTRETRYIGNORE | MB_ICONWARNING | MB_DEFBUTTON2 | MB_SYSTEMMODAL;

		default:
		case eMB_Info:
			return MB_OK;
		}
	}

	EQuestionResult GetMessageBoxResult(int returnValue)
	{
		switch (returnValue)
		{
		case IDABORT:
			return eQR_Abort;
		case IDCANCEL:
			return eQR_Cancel;
		case IDIGNORE:
			return eQR_Ignore;
		case IDNO:
			return eQR_No;
		case IDYES:
		case IDOK:
			return eQR_Yes;
		case IDRETRY:
			return eQR_Retry;
		default:
			return eQR_None;
		}
	}
}

EQuestionResult DrxMessageBoxImpl(const char* szText, const char* szCaption, EMessageBox type)
{
	const char* szCommandLine = ::GetCommandLineA();
	if (DrxStringUtils::stristr(szCommandLine, "-noprompt") != 0)
	{
		return eQR_None;
	}

#if DRX_PLATFORM_WINDOWS
	return GetMessageBoxResult(::MessageBoxA(nullptr, szText, szCaption, GetMessageBoxType(type)));
#else
	wstring text;
	Unicode::Convert(text, szText);

	wstring caption;
	Unicode::Convert(caption, szCaption);

	Windows::UI::Popups::MessageDialog^ messageDialog = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(text.c_str()), ref new Platform::String(caption.c_str()));
	Windows::Foundation::IAsyncOperation<Windows::UI::Popups::IUICommand^ >^ asyncOperation = messageDialog->ShowAsync();
	
	// Block waiting for user input
	while (!asyncOperation->Completed) {}

	// TODO: Populate options and return value, currently we only support the Info type on Xbox.
	//IUICommand* command = asyncOperation->GetResults();
	return eQR_No;
#endif
}

EQuestionResult DrxMessageBoxImpl(const wchar_t* szText, const wchar_t* szCaption, EMessageBox type)
{
	const char* szCommandLine = ::GetCommandLineA();
	if (DrxStringUtils::stristr(szCommandLine, "-noprompt") != 0)
	{
		return eQR_None;
	}

#if DRX_PLATFORM_WINDOWS
	return GetMessageBoxResult(::MessageBoxW(nullptr, szText, szCaption, GetMessageBoxType(type)));
#else
	Windows::UI::Popups::MessageDialog^ messageDialog = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(szText), ref new Platform::String(szCaption));
	Windows::Foundation::IAsyncOperation<Windows::UI::Popups::IUICommand^ >^ asyncOperation = messageDialog->ShowAsync();
	
	// Block waiting for user input
	while (!asyncOperation->Completed) {}

	// TODO: Populate options and return value, currently we only support the Info type on Xbox.
	//IUICommand* command = asyncOperation->GetResults();
	return eQR_No;
#endif
}