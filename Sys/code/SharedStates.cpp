// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#ifdef INCLUDE_SCALEFORM_SDK

#include <drx3D/Sys/ConfigScaleform.h>
	#include <drx3D/Sys/SharedStates.h>

	#include "../PakVars.h"
	#include <drx3D/Sys/FlashPlayerInstance.h>
	#include "GFileDrxPak.h"
	#include "GImage.h"
	#include <drx3D/Sys/GFxVideoWrapper.h>
	#include <drx3D/Sys/GImageInfo_Impl.h>
	#include <drx3D/Sys/GTexture_Impl.h>

	#include <drx3D/Sys/IConsole.h>
	#include <drx3D/CoreX/String/StringUtils.h>
	#include <drx3D/CoreX/String/Path.h>
	#include <drx3D/Sys/ILocalizationUpr.h>
	#include <drx3D/CoreX/Renderer/ITexture.h>
	#include <drx3D/Sys/IStreamEngine.h>
	#include <drx3D/CoreX/String/UnicodeFunctions.h>

	#ifndef GFC_NO_THREADSUPPORT
//////////////////////////////////////////////////////////////////////////
// Convert custom DinrusX thread priority to a GFx acceptable thread priority value

GThread::ThreadPriority ConvertToGFxThreadPriority(i32 nPriority)
{
	if (nPriority > THREAD_PRIORITY_HIGHEST)
		return GThread::CriticalPriority;
	else if (nPriority > THREAD_PRIORITY_ABOVE_NORMAL)
		return GThread::HighestPriority;
	else if (nPriority > THREAD_PRIORITY_NORMAL)
		return GThread::AboveNormalPriority;
	else if (nPriority > THREAD_PRIORITY_BELOW_NORMAL)
		return GThread::NormalPriority;
	else if (nPriority > THREAD_PRIORITY_LOWEST)
		return GThread::BelowNormalPriority;
	else
		return GThread::LowestPriority;
}
	#endif

//////////////////////////////////////////////////////////////////////////
// DrxGFxFileOpener

DrxGFxFileOpener::DrxGFxFileOpener()
{
}

DrxGFxFileOpener& DrxGFxFileOpener::GetAccess()
{
	static auto s_pInst = new DrxGFxFileOpener(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxFileOpener::~DrxGFxFileOpener()
{
}

GFile* DrxGFxFileOpener::OpenFile(tukk pUrl, SInt flags, SInt /*mode*/)
{
	assert(pUrl);
	if (flags & ~(GFileConstants::Open_Read | GFileConstants::Open_Buffered))
		return 0;
	tukk pExt = PathUtil::GetExt(pUrl);
	#if defined(USE_GFX_VIDEO)
	if (!strcmp(pUrl, internal_video_player))
		return new GMemoryFile(pUrl, fxvideoplayer_swf, sizeof(fxvideoplayer_swf));
	if (!stricmp(pExt, "usm"))
		return new GFileDrxStream(pUrl, eStreamTaskTypeVideo);
	#endif
	// delegate all file I/O to IDrxPak by returning a GFileDrxPak object
	return new GFileDrxPak(pUrl);
}

SInt64 DrxGFxFileOpener::GetFileModifyTime(tukk pUrl)
{
	SInt64 fileModTime = 0;
	if (CFlashPlayer::CheckFileModTimeEnabled())
	{
		IDrxPak* pPak = gEnv->pDrxPak;
		if (pPak)
		{
			FILE* f = pPak->FOpen(pUrl, "rb");
			if (f)
			{
				fileModTime = (SInt64) pPak->GetModificationTime(f);
				pPak->FClose(f);
			}
		}
	}
	return fileModTime;
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxURLBuilder

DrxGFxURLBuilder::DrxGFxURLBuilder()
{
}

DrxGFxURLBuilder& DrxGFxURLBuilder::GetAccess()
{
	static auto s_pInst = new DrxGFxURLBuilder(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxURLBuilder::~DrxGFxURLBuilder()
{
}

void DrxGFxURLBuilder::BuildURL(GString* pPath, const LocationInfo& loc)
{
	assert(pPath); // GFx always passes a valid dst ptr

	tukk pOrigFilePath = loc.FileName.ToCStr();
	if (pOrigFilePath && *pOrigFilePath)
	{
		// if pOrigFilePath ends with "_locfont.[swf|gfx]", we search for the font in the localization folder
		const char locFontPostFixSwf[] = "_locfont.swf";
		const char locFontPostFixGfx[] = "_locfont.gfx";
		assert(sizeof(locFontPostFixSwf) == sizeof(locFontPostFixGfx)); // both postfixes must be of same length
		const size_t locFontPostFixLen = sizeof(locFontPostFixSwf) - 1;

		size_t filePathLength = strlen(pOrigFilePath);
		if (filePathLength > locFontPostFixLen)
		{
			size_t offset = filePathLength - locFontPostFixLen;
			if (!strnicmp(locFontPostFixGfx, pOrigFilePath + offset, locFontPostFixLen) || !strnicmp(locFontPostFixSwf, pOrigFilePath + offset, locFontPostFixLen))
			{
				char const* const szLanguage = gEnv->pSystem->GetLocalizationUpr()->GetLanguage();
				*pPath = PathUtil::GetLocalizationFolder() + DRX_NATIVE_PATH_SEPSTR + szLanguage + "_xml" + DRX_NATIVE_PATH_SEPSTR;
				*pPath += PathUtil::GetFile(pOrigFilePath);
				return;
			}
		}

		if (*pOrigFilePath != '/')
			*pPath = loc.ParentPath + loc.FileName;
		else
			*pPath = GString(&pOrigFilePath[1], filePathLength - 1);
	}
	else
	{
		assert(0 && "DrxGFxURLBuilder::BuildURL() - no filename passed!");
		*pPath = "";
	}
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxTextClipboard

DrxGFxTextClipboard::DrxGFxTextClipboard()
#if DRX_PLATFORM_WINDOWS
	: m_bSyncingClipboardFromWindows(false)
#endif // DRX_PLATFORM_WINDOWS
{
	if (auto pSystem = gEnv->pSystem)
	{
#if DRX_PLATFORM_WINDOWS
		const HWND hWnd = reinterpret_cast<HWND>(pSystem->GetHWND());
		AddClipboardFormatListener(hWnd);
		HandleMessage(hWnd, WM_CLIPBOARDUPDATE, 0, 0, nullptr); // Sync current clipboard content with Scaleform
#endif // DRX_PLATFORM_WINDOWS
		pSystem->RegisterWindowMessageHandler(this);
	}
}

DrxGFxTextClipboard::~DrxGFxTextClipboard()
{
	if (gEnv && gEnv->pSystem)
	{
		gEnv->pSystem->UnregisterWindowMessageHandler(this);
#if DRX_PLATFORM_WINDOWS
		RemoveClipboardFormatListener(reinterpret_cast<HWND>(gEnv->pSystem->GetHWND()));
#endif // DRX_PLATFORM_WINDOWS
	}
}

DrxGFxTextClipboard& DrxGFxTextClipboard::GetAccess()
{
	static auto s_pInst = new DrxGFxTextClipboard(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

void DrxGFxTextClipboard::OnTextStore(const wchar_t* szText, UPInt length)
{
#if DRX_PLATFORM_WINDOWS
	// Copy to windows clipboard
	if (!m_bSyncingClipboardFromWindows && OpenClipboard(nullptr) != 0)
	{
		const HWND hWnd = reinterpret_cast<HWND>(gEnv->pSystem->GetHWND());

		// Avoid endless notification loop
		RemoveClipboardFormatListener(hWnd);

		static_assert(sizeof(wchar_t) == 2, "sizeof(wchar_t) needs to be 2 to be compatible with Scaleform.");
		const HGLOBAL clipboardData = GlobalAlloc(GMEM_DDESHARE, sizeof(wchar_t) * (length + 1));

		wchar_t* const pData = reinterpret_cast<wchar_t*>(GlobalLock(clipboardData));
		G_wcscpy(pData, length + 1, szText);
		GlobalUnlock(clipboardData);

		SetClipboardData(CF_UNICODETEXT, clipboardData);

		CloseClipboard();

		AddClipboardFormatListener(hWnd);
	}
#endif // DRX_PLATFORM_WINDOWS
}

#if DRX_PLATFORM_WINDOWS
bool DrxGFxTextClipboard::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (uMsg == WM_CLIPBOARDUPDATE)
	{
		if (OpenClipboard(nullptr) != 0)
		{
			wstring data;
			const HANDLE wideData = GetClipboardData(CF_UNICODETEXT);
			if (wideData)
			{
				const LPCWSTR pWideData = reinterpret_cast<LPCWSTR>(GlobalLock(wideData));
				if (pWideData)
				{
					// Note: This conversion is just to make sure we discard malicious or malformed data
					Unicode::ConvertSafe<Unicode::eErrorRecovery_Discard>(data, pWideData);
					GlobalUnlock(wideData);
				}
			}
			CloseClipboard();

			m_bSyncingClipboardFromWindows = true;
			SetText(data.c_str(), data.size());
			m_bSyncingClipboardFromWindows = false;
		}
	}

	return false;
}
#endif // DRX_PLATFORM_WINDOWS

//////////////////////////////////////////////////////////////////////////
// DrxGFxTranslator

DrxGFxTranslator::DrxGFxTranslator()
	: m_pILocMan(0)
{
}

DrxGFxTranslator& DrxGFxTranslator::GetAccess()
{
	static auto s_pInst = new DrxGFxTranslator(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

void DrxGFxTranslator::SetWordWrappingMode(tukk pLanguage)
{
	if (!stricmp(pLanguage, "japanese"))
	{
		WWMode = WWT_Prohibition;
	}
	else if (!stricmp(pLanguage, "chineset"))
	{
		WWMode = WWT_Asian;
	}
	else
	{
		WWMode = WWT_Default;
	}
}

DrxGFxTranslator::~DrxGFxTranslator()
{
}

UInt DrxGFxTranslator::GetCaps() const
{
	return Cap_ReceiveHtml | Cap_StripTrailingNewLines;
}

void DrxGFxTranslator::Translate(TranslateInfo* pTranslateInfo)
{
	IF(m_pILocMan == 0, 0)
	m_pILocMan = gEnv->pSystem->GetLocalizationUpr();

	if (!m_pILocMan || !pTranslateInfo)
		return;

	const wchar_t* pKey = pTranslateInfo->GetKey();

	if (!pKey) // No string -> not localizable
		return;

	string utf8Key, localizedString;
	Unicode::Convert(utf8Key, pKey);

	if (m_pILocMan->LocalizeString(utf8Key, localizedString))
	{
		if (pTranslateInfo->IsKeyHtml())
			pTranslateInfo->SetResultHtml(localizedString.c_str());
		else
			pTranslateInfo->SetResult(localizedString.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxLog

ICVar* DrxGFxLog::CV_sys_flash_debuglog(0);
i32 DrxGFxLog::ms_sys_flash_debuglog(0);

	#if defined(DRXGFXLOG_USE_IMPLICT_TLS)
		#if DRX_PLATFORM_WINDOWS || DRX_PLATFORM_DURANGO
static __declspec(thread) tukk g_pCurFlashContext = 0;
		#elif DRX_PLATFORM_ORBIS
static __thread tukk g_pCurFlashContext = 0;
		#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
static __thread tukk g_pCurFlashContext = 0;
		#else
			#error No TLS storage defined for DrxGFxLog on this platform config
		#endif
	#else
static u32 g_curFlashContextTlsIdx = TLS_OUT_OF_INDEXES;
	#endif

DrxGFxLog::DrxGFxLog()
	: m_pLog(0)
{
	m_pLog = gEnv->pLog;

	if (!CV_sys_flash_debuglog)
		CV_sys_flash_debuglog = REGISTER_CVAR2("sys_flash_debuglog", &ms_sys_flash_debuglog, 0, VF_NULL, "");

	#if !defined(DRXGFXLOG_USE_IMPLICT_TLS)
	g_curFlashContextTlsIdx = TlsAlloc();
	assert(g_curFlashContextTlsIdx != TLS_OUT_OF_INDEXES);
	#endif
}

DrxGFxLog& DrxGFxLog::GetAccess()
{
	static auto s_pInst = new DrxGFxLog(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxLog::~DrxGFxLog()
{
	#if !defined(DRXGFXLOG_USE_IMPLICT_TLS)
	if (g_curFlashContextTlsIdx != TLS_OUT_OF_INDEXES)
		TlsFree(g_curFlashContextTlsIdx);
	#endif
}

void DrxGFxLog::LogMessageVarg(LogMessageType messageType, tukk pFmt, va_list argList)
{
	char logBuf[1024];
	{
		const char prefix[] = "<Flash> ";

		static_assert(sizeof(prefix) + 128 <= sizeof(logBuf), "Invalid array size!");

		// prefix
		{
			drx_strcpy(logBuf, prefix);
		}

		// msg
		size_t len = sizeof(prefix) - 1;
		{
			drx_vsprintf(&logBuf[len], sizeof(logBuf) - len, pFmt, argList);
			len = strlen(logBuf);
			if (logBuf[len - 1] == '\n')
			{
				logBuf[--len] = 0;
			}
		}

		// context
		{
	#if defined(DRXGFXLOG_USE_IMPLICT_TLS)
			tukk pCurFlashContext = g_pCurFlashContext;
	#else
			tukk pCurFlashContext = g_curFlashContextTlsIdx != TLS_OUT_OF_INDEXES ? (tukk) TlsGetValue(g_curFlashContextTlsIdx) : 0;
	#endif
			drx_sprintf(&logBuf[len], sizeof(logBuf) - len, " [%s]", pCurFlashContext ? pCurFlashContext : "#!NO_CONTEXT!#");
		}
	}

	if (ms_sys_flash_debuglog)
	{
		PREFAST_SUPPRESS_WARNING(6053);
		OutputDebugString(logBuf);
	}

	switch (messageType & (~Log_Channel_Mask))
	{
	case Log_MessageType_Error:
		{
			m_pLog->LogError("%s", logBuf);
			break;
		}
	case Log_MessageType_Warning:
		{
			m_pLog->LogWarning("%s", logBuf);
			break;
		}
	case Log_MessageType_Message:
	default:
		{
			m_pLog->Log("%s", logBuf);
			break;
		}
	}
}

void DrxGFxLog::SetContext(tukk pFlashContext)
{
	#if defined(DRXGFXLOG_USE_IMPLICT_TLS)
	g_pCurFlashContext = pFlashContext;
	#else
	if (g_curFlashContextTlsIdx != TLS_OUT_OF_INDEXES)
		TlsSetValue(g_curFlashContextTlsIdx, (uk ) pFlashContext);
	#endif
}

tukk DrxGFxLog::GetContext() const
{
	#if defined(DRXGFXLOG_USE_IMPLICT_TLS)
	return g_pCurFlashContext;
	#else
	tukk pCurFlashContext = g_curFlashContextTlsIdx != TLS_OUT_OF_INDEXES ? (tukk) TlsGetValue(g_curFlashContextTlsIdx) : 0;
	return pCurFlashContext;
	#endif
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxFSCommandHandler

DrxGFxFSCommandHandler::DrxGFxFSCommandHandler()
{
}

DrxGFxFSCommandHandler& DrxGFxFSCommandHandler::GetAccess()
{
	static auto s_pInst = new DrxGFxFSCommandHandler(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxFSCommandHandler::~DrxGFxFSCommandHandler()
{
}

void DrxGFxFSCommandHandler::Callback(GFxMovieView* pMovieView, tukk pCommand, tukk pArgs)
{
	// get flash player instance this action script command belongs to and delegate it to client
	CFlashPlayer* pFlashPlayer(static_cast<CFlashPlayer*>(pMovieView->GetUserData()));
	if (pFlashPlayer)
		pFlashPlayer->DelegateFSCommandCallback(pCommand, pArgs);
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxExternalInterface

DrxGFxExternalInterface::DrxGFxExternalInterface()
{
}

DrxGFxExternalInterface& DrxGFxExternalInterface::GetAccess()
{
	static auto s_pInst = new DrxGFxExternalInterface(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxExternalInterface::~DrxGFxExternalInterface()
{
}

void DrxGFxExternalInterface::Callback(GFxMovieView* pMovieView, tukk pMethodName, const GFxValue* pArgs, UInt numArgs)
{
	CFlashPlayer* pFlashPlayer(static_cast<CFlashPlayer*>(pMovieView->GetUserData()));
	if (pFlashPlayer)
		pFlashPlayer->DelegateExternalInterfaceCallback(pMethodName, pArgs, numArgs);
	else
		pMovieView->SetExternalInterfaceRetVal(GFxValue(GFxValue::VT_Undefined));
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxUserEventHandler

DrxGFxUserEventHandler::DrxGFxUserEventHandler()
{
}

DrxGFxUserEventHandler& DrxGFxUserEventHandler::GetAccess()
{
	static auto s_pInst = new DrxGFxUserEventHandler(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxUserEventHandler::~DrxGFxUserEventHandler()
{
}

void DrxGFxUserEventHandler::HandleEvent(class GFxMovieView* /*pMovieView*/, const class GFxEvent& /*event*/)
{
	// not implemented since it's not needed yet... would need to translate GFx event to independent representation
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxImageCreator

DrxGFxImageCreator::DrxGFxImageCreator()
{
}

DrxGFxImageCreator& DrxGFxImageCreator::GetAccess()
{
	static auto s_pInst = new DrxGFxImageCreator(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxImageCreator::~DrxGFxImageCreator()
{
}

GImageInfoBase* DrxGFxImageCreator::CreateImage(const GFxImageCreateInfo& info)
{
	GImageInfoBase* pImageInfo(0);
	switch (info.Type)
	{
	case GFxImageCreateInfo::Input_Image:
	case GFxImageCreateInfo::Input_File:
		{
			switch (info.Type)
			{
			case GFxImageCreateInfo::Input_Image:
				{
					// create image info and texture for internal image
					pImageInfo = new GImageInfoXRender(info.pImage);
					break;
				}
			case GFxImageCreateInfo::Input_File:
				{
					// create image info and texture for external image
					pImageInfo = new GImageInfoFileXRender(info.pFileInfo->FileName.ToCStr(), info.pFileInfo->TargetWidth, info.pFileInfo->TargetHeight);
					break;
				}
			}
			break;
		}
	default:
		{
			assert(0);
			break;
		}
	}
	return pImageInfo;
}

//////////////////////////////////////////////////////////////////////////
// DrxGFxImageLoader

DrxGFxImageLoader::DrxGFxImageLoader()
{
}

DrxGFxImageLoader& DrxGFxImageLoader::GetAccess()
{
	static auto s_pInst = new DrxGFxImageLoader(); // This pointer is released in ~GFxLoader2()
	return *s_pInst;
}

DrxGFxImageLoader::~DrxGFxImageLoader()
{
}

static bool LookupDDS(tukk pFilePath, u32& width, u32& height)
{
	ITexture* pTex = gEnv->pRenderer->EF_GetTextureByName(pFilePath);
	if (pTex)
	{
		height = pTex->GetHeight();
		width = pTex->GetWidth();

		return true;
	}
	else
	{
		DrxStackStringT<char, 256> splitFilePath(pFilePath);
		splitFilePath += ".0";

		IDrxPak* pPak(gEnv->pDrxPak);

		FILE* f = pPak->FOpen(splitFilePath.c_str(), "rb");
		if (!f)
			f = pPak->FOpen(pFilePath, "rb");

		if (f)
		{
			pPak->FSeek(f, 12, SEEK_SET);
			pPak->FRead(&height, 1, f);
			pPak->FRead(&width, 1, f);
			pPak->FClose(f);
		}
		return f != 0;
	}
}

GImageInfoBase* DrxGFxImageLoader::LoadImage(tukk pUrl)
{
	// callback for loadMovie API
	tukk pFilePath(strstr(pUrl, "://"));
	tukk pTexId(strstr(pUrl, "://TEXID:"));
	if (!pFilePath && !pTexId)
		return 0;
	if (pFilePath)
		pFilePath += 3;
	if (pTexId)
		pTexId += 9;

	GImageInfoBase* pImageInfo(0);
	if (CFlashPlayer::GetFlashLoadMovieHandler())
	{
		IFlashLoadMovieImage* pImage(CFlashPlayer::GetFlashLoadMovieHandler()->LoadMovie(pFilePath));
		if (pImage && pImage->IsValid())
			pImageInfo = new GImageInfoILMISrcXRender(pImage);
	}

	if (!pImageInfo)
	{
		if (pTexId)
		{
			i32 texId(atoi(pTexId));
			ITexture* pTexture(gEnv->pRenderer->EF_GetTextureByID(texId));
			if (pTexture)
			{
				pImageInfo = new GImageInfoTextureXRender(pTexture);
			}
		}
		else if (stricmp(pFilePath, "undefined") != 0)
		{
			tukk pExt = PathUtil::GetExt(pFilePath);
			if (!stricmp(pExt, "dds"))
			{
				// Note: We need to know the dimensions in advance as otherwise the image won't show up!
				// The seemingly more elegant approach to pass zero as w/h and explicitly call GetTexture() on the image object to
				// query w/h won't work reliably with MT rendering (potential deadlock between Advance() and Render()). This approach
				// works but requires that we keep in sync with any changes to the streaming system on consoles (see LookupDDS()).
				u32 width = 0, height = 0;
				if (LookupDDS(pFilePath, width, height))
					pImageInfo = new GImageInfoFileXRender(pFilePath, width, height);
			}
	#if defined(USE_GFX_JPG)
			else if (!stricmp(pExt, "jpg"))
			{
				GFxLoader2* pLoader = CSharedFlashPlayerResources::GetAccess().GetLoader(true);
				assert(pLoader);
				GPtr<GFxJpegSupportBase> pJPGLoader = pLoader->GetJpegSupport();
				if (pJPGLoader)
				{
					GFileDrxPak file(pFilePath, !strnicmp(pFilePath, "%user%", 6) ? IDrxPak::FOPEN_ONDISK : 0);
					GPtr<GImage> pImage = *pJPGLoader->ReadJpeg(&file, GMemory::GetGlobalHeap());
					if (pImage)
						pImageInfo = new GImageInfoXRender(pImage);
				}
			}
	#endif
	#if defined(USE_GFX_PNG)
			else if (!stricmp(pExt, "png"))
			{
				GFxLoader2* pLoader = CSharedFlashPlayerResources::GetAccess().GetLoader(true);
				assert(pLoader);
				GPtr<GFxPNGSupportBase> pPNGLoader = pLoader->GetPNGSupport();
				if (pPNGLoader)
				{
					GFileDrxPak file(pFilePath, !strnicmp(pFilePath, "%user%", 6) ? IDrxPak::FOPEN_ONDISK : 0);
					GPtr<GImage> pImage = *pPNGLoader->CreateImage(&file, GMemory::GetGlobalHeap());
					if (pImage)
						pImageInfo = new GImageInfoXRender(pImage);
				}
			}
	#endif
			else if ((pExt[0] == '\0') && (pFilePath[0] == '$'))
			{
				u32 width = 0, height = 0;
				if (LookupDDS(pFilePath, width, height))
				{
					pImageInfo = new GImageInfoFileXRender(pFilePath, width, height);
				}
			}
			else
				DrxGFxLog::GetAccess().LogWarning("\"%s\" cannot be loaded by loadMovie API! Invalid file or format passed.", pFilePath);
		}
	}

	return pImageInfo;
}

#endif // #ifdef INCLUDE_SCALEFORM_SDK
