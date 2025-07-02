// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   HTTPDownloader.h
//  Version:     v1.00
//  Created:     24/9/2004.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#if DRX_PLATFORM_WINDOWS
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <wininet.h> // requires <windows.h>
	#include <drx3D/Script/IScriptSystem.h>
	#include <dbghelp.h>

	#define HTTP_BUFFER_SIZE (16384)

enum
{
	HTTP_STATE_WORKING = 0,
	HTTP_STATE_COMPLETE,
	HTTP_STATE_CANCELED,
	HTTP_STATE_ERROR,
	HTTP_STATE_NONE,
};

class CDownloadUpr;

class CHTTPDownloader // : public _ScriptableEx<CHTTPDownloader>
{
public:
	CHTTPDownloader();
	virtual ~CHTTPDownloader();

	i32           Create(ISystem* pISystem, CDownloadUpr* pParent);
	i32           Download(tukk szURL, tukk szDestination);
	void          Cancel();
	i32           GetState()             { return m_iState; };
	i32           GetFileSize() const    { return m_iFileSize; };
	const string& GetURL() const         { return m_szURL; };
	const string& GetDstFileName() const { return m_szDstFile; };
	void          Release();

	i32           Download(IFunctionHandler* pH);
	i32           Cancel(IFunctionHandler* pH);
	i32           Release(IFunctionHandler* pH);

	i32           GetURL(IFunctionHandler* pH);
	i32           GetFileSize(IFunctionHandler* pH);
	i32           GetFileName(IFunctionHandler* pH);

	void          OnError();
	void          OnComplete();
	void          OnCancel();

private:

	static DWORD  DownloadProc(CHTTPDownloader* _this);
	void          CreateDownloadThread();
	DWORD         DoDownload();
	void          PrepareBuffer();
	IScriptTable* GetScriptObject() { return 0; };

	string            m_szURL;
	string            m_szDstFile;
	HANDLE            m_hThread;
	HINTERNET         m_hINET;
	HINTERNET         m_hUrl;

	u8*    m_pBuffer;
	i32               m_iFileSize;

	 i32      m_iState;
	 bool     m_bContinue;

	ISystem*          m_pSystem;
	CDownloadUpr* m_pParent;

	IScriptSystem*    m_pScriptSystem;
};

#endif
