// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SHARED_STATES_H_
#define _SHARED_STATES_H_

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK

	#pragma warning(push)
	#pragma warning(disable : 6326)// Potential comparison of a constant with another constant
	#pragma warning(disable : 6011)// Dereferencing NULL pointer
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <drx3D/Sys/IWindowMessageHandler.h>
	#include <GFxLoader.h>
	#include <GFxPlayer.h> // includes <windows.h>
	#include <GFxLog.h>
	#include <GFxImageResource.h>
	#pragma warning(pop)

struct ILocalizationUpr;
struct ILog;
struct ICVar;

	#ifndef GFC_NO_THREADSUPPORT
//////////////////////////////////////////////////////////////////////////
// Convert custom DinrusX thread priority to a GFx acceptable thread priority value

GThread::ThreadPriority ConvertToGFxThreadPriority(i32 nDrxPriority);
	#endif

//////////////////////////////////////////////////////////////////////////
// DrxGFxFileOpener

class DrxGFxFileOpener : public GFxFileOpener
{
public:
	// GFxFileOpener interface
	virtual GFile* OpenFile(tukk pUrl, SInt flags = GFileConstants::Open_Read | GFileConstants::Open_Buffered, SInt mode = GFileConstants::Mode_ReadWrite);
	virtual SInt64 GetFileModifyTime(tukk pUrl);
	// no overwrite for OpenFileEx needed yet as it calls out OpenFile impl
	//virtual GFile* OpenFileEx(tukk pUrl, class GFxLog *pLog, SInt flags = GFileConstants::Open_Read|GFileConstants::Open_Buffered, SInt mode = GFileConstants::Mode_ReadWrite);

public:
	static DrxGFxFileOpener& GetAccess();
	~DrxGFxFileOpener();

private:
	DrxGFxFileOpener();
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxURLBuilder

class DrxGFxURLBuilder : public GFxURLBuilder
{
	// GFxURLBuilder interface
	virtual void BuildURL(GString* pPath, const LocationInfo& loc);

public:
	static DrxGFxURLBuilder& GetAccess();
	~DrxGFxURLBuilder();

private:
	DrxGFxURLBuilder();
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxTextClipboard

class DrxGFxTextClipboard : public GFxTextClipboard, IWindowMessageHandler
{
public:
	void OnTextStore(const wchar_t* szText, UPInt length) override;
	static DrxGFxTextClipboard& GetAccess();
	~DrxGFxTextClipboard();

#if DRX_PLATFORM_WINDOWS
	bool HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
#endif // DRX_PLATFORM_WINDOWS

private:
	DrxGFxTextClipboard();

#if DRX_PLATFORM_WINDOWS
	bool m_bSyncingClipboardFromWindows;
#endif // DRX_PLATFORM_WINDOWS
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxTranslator

class DrxGFxTranslator : public GFxTranslator
{
public:
	// GFxTranslator interface
	virtual UInt GetCaps() const;
	virtual void Translate(TranslateInfo* pTranslateInfo);
	virtual bool IsDirty() const      { return m_bDirty; }
	void         SetDirty(bool dirty) { m_bDirty = dirty; }

public:
	static DrxGFxTranslator& GetAccess();
	~DrxGFxTranslator();

	void SetWordWrappingMode(tukk pLanguage);

private:
	DrxGFxTranslator();

private:
	ILocalizationUpr* m_pILocMan;
	bool                  m_bDirty;
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxLog

	#if DRX_PLATFORM_ORBIS || DRX_PLATFORM_APPLE || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
		#define DRXGFXLOG_USE_IMPLICT_TLS
	#endif

class DrxGFxLog : public GFxLog
{
public:
	// GFxLog interface
	virtual void LogMessageVarg(LogMessageType messageType, tukk pFmt, va_list argList);

public:
	static DrxGFxLog& GetAccess();
	~DrxGFxLog();

	void        SetContext(tukk pFlashContext);
	tukk GetContext() const;

private:
	DrxGFxLog();

private:
	static ICVar* CV_sys_flash_debuglog;
	static i32    ms_sys_flash_debuglog;

private:
	ILog* m_pLog;
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxFSCommandHandler

class DrxGFxFSCommandHandler : public GFxFSCommandHandler
{
public:
	// GFxFSCommandHandler interface
	virtual void Callback(GFxMovieView* pMovieView, tukk pCommand, tukk pArgs);

public:
	static DrxGFxFSCommandHandler& GetAccess();
	~DrxGFxFSCommandHandler();

private:
	DrxGFxFSCommandHandler();
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxExternalInterface

class DrxGFxExternalInterface : public GFxExternalInterface
{
public:
	// GFxExternalInterface interface
	virtual void Callback(GFxMovieView* pMovieView, tukk pMethodName, const GFxValue* pArgs, UInt numArgs);

public:
	static DrxGFxExternalInterface& GetAccess();
	~DrxGFxExternalInterface();

private:
	DrxGFxExternalInterface();
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxUserEventHandler

class DrxGFxUserEventHandler : public GFxUserEventHandler
{
public:
	// GFxUserEventHandler interface
	virtual void HandleEvent(class GFxMovieView* pMovieView, const class GFxEvent& event);

public:
	static DrxGFxUserEventHandler& GetAccess();
	~DrxGFxUserEventHandler();

private:
	DrxGFxUserEventHandler();
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxImageCreator

class DrxGFxImageCreator : public GFxImageCreator
{
public:
	// GFxImageCreator interface
	virtual GImageInfoBase* CreateImage(const GFxImageCreateInfo& info);

public:
	static DrxGFxImageCreator& GetAccess();
	~DrxGFxImageCreator();

private:
	DrxGFxImageCreator();
};

//////////////////////////////////////////////////////////////////////////
// DrxGFxImageLoader

class DrxGFxImageLoader : public GFxImageLoader
{
public:
	// GFxImageLoader interface
	virtual GImageInfoBase* LoadImage(tukk pUrl);

public:
	static DrxGFxImageLoader& GetAccess();
	~DrxGFxImageLoader();

private:
	DrxGFxImageLoader();
};

#endif // #ifdef INCLUDE_SCALEFORM_SDK

#endif // #ifndef _SHARED_STATES_H_
