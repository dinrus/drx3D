// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/HTTPDownloader.h>
#include <drx3D/Sys/DownloadUpr.h>

#include <drx3D/Sys/ILog.h>
#include <drx3D/Sys/ISystem.h>

#if DRX_PLATFORM_WINDOWS

//-------------------------------------------------------------------------------------------------
CHTTPDownloader::CHTTPDownloader()
	: m_hThread(0),
	m_hINET(0),
	m_hUrl(0),
	m_iFileSize(0),
	m_pBuffer(0),
	m_iState(HTTP_STATE_NONE),
	m_bContinue(false),
	m_pSystem(0),
	m_pParent(0)
{
	m_pScriptSystem = NULL;
}

//-------------------------------------------------------------------------------------------------
CHTTPDownloader::~CHTTPDownloader()
{
}

/*
   //-------------------------------------------------------------------------------------------------
   void CHTTPDownloader::InitializeTemplate(IScriptSystem *pSS)
   {
   _ScriptableEx<CHTTPDownloader>::InitializeTemplate(pSS);

   REG_FUNC(CHTTPDownloader, Download);
   REG_FUNC(CHTTPDownloader, Cancel);
   REG_FUNC(CHTTPDownloader, Release);
   REG_FUNC(CHTTPDownloader, GetURL);
   REG_FUNC(CHTTPDownloader, GetFileName);
   REG_FUNC(CHTTPDownloader, GetFileSize);
   }
 */

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::Create(ISystem* pISystem, CDownloadUpr* pParent)
{
	m_pSystem = pISystem;
	m_pParent = pParent;
	//Init(m_pSystem->GetIScriptSystem(), this);

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::Download(tukk szURL, tukk szDestination)
{
	m_szURL = szURL;
	m_szDstFile = szDestination;
	m_bContinue = 1;

	CreateDownloadThread();

	return 1;
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::Cancel()
{
	m_bContinue = 0;
}

//-------------------------------------------------------------------------------------------------
DWORD CHTTPDownloader::DownloadProc(CHTTPDownloader* _this)
{
	_this->DoDownload();

	return 0;
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::CreateDownloadThread()
{
	DWORD dwThreadId = 0;

	m_hThread = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)DownloadProc, this, 0, &dwThreadId);
}

//-------------------------------------------------------------------------------------------------
DWORD CHTTPDownloader::DoDownload()
{
	m_iState = HTTP_STATE_WORKING;

	m_hINET = InternetOpen("", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);

	if (!m_hINET)
	{
		m_iState = HTTP_STATE_ERROR;

		return 1;
	}

	if (!m_bContinue)
	{
		m_iState = HTTP_STATE_CANCELED;

		return 1;
	}

	DWORD dwFlags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD;
	m_hUrl = InternetOpenUrl(m_hINET, m_szURL.c_str(), 0, 0, dwFlags, 0);

	if (!m_hUrl)
	{
		m_iState = HTTP_STATE_ERROR;

		return 1;
	}

	if (!m_bContinue)
	{
		m_iState = HTTP_STATE_CANCELED;

		return 1;
	}

	char szBuffer[64] = { 0 };
	DWORD dwSize = 64;
	i32 bQuery = HttpQueryInfo(m_hUrl, HTTP_QUERY_CONTENT_LENGTH, szBuffer, &dwSize, 0);

	if (bQuery)
	{
		m_iFileSize = atoi(szBuffer);
	}
	else
	{
		m_iFileSize = -1;
	}

	if (!m_bContinue)
	{
		m_iState = HTTP_STATE_CANCELED;

		return 1;
	}

	PrepareBuffer();

	FILE* hFile = fopen(m_szDstFile.c_str(), "wb");

	if (!hFile)
	{
		m_iState = HTTP_STATE_ERROR;

		return 1;
	}

	DWORD dwRead = 0;

	while (InternetReadFile(m_hUrl, m_pBuffer, HTTP_BUFFER_SIZE, &dwRead))
	{
		if (dwRead)
		{
			fwrite(m_pBuffer, 1, dwRead, hFile);
		}
		else
		{
			fclose(hFile);

			m_iState = HTTP_STATE_COMPLETE;

			return 1;
		}

		if (!m_bContinue)
		{
			fclose(hFile);

			m_iState = HTTP_STATE_CANCELED;

			return 1;
		}

		DrxSleep(5);
	}

	fclose(hFile);

	m_iState = HTTP_STATE_ERROR;

	return 1;
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::PrepareBuffer()
{
	if (!m_pBuffer)
	{
		m_pBuffer = new u8[HTTP_BUFFER_SIZE];
	}
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::Release()
{
	m_bContinue = 0;

	if (m_hUrl)
	{
		InternetCloseHandle(m_hUrl);
	}

	if (m_hINET)
	{
		InternetSetStatusCallback(m_hINET, 0);
		InternetCloseHandle(m_hINET);
	}

	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = 0;
	}

	m_hINET = 0;
	m_hUrl = 0;
	m_iFileSize = 0;
	m_szURL.clear();
	m_szDstFile.clear();

	WaitForSingleObject(m_hThread, 5); // wait five milliseconds for the thread to finish
	CloseHandle(m_hThread);
	m_hThread = 0;

	m_pParent->RemoveDownload(this);

	delete this;
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::OnError()
{
	m_pSystem->GetILog()->LogError("DOWNLOAD ERROR: %s", CHTTPDownloader::GetURL().c_str());

	IScriptTable* pScriptObject = GetScriptObject();

	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject->GetValue("OnError", pScriptFunction))
	{
		return;
	}

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(pScriptObject);
	m_pScriptSystem->EndCall();
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::OnComplete()
{
	m_pSystem->GetILog()->Log("DOWNLOAD COMPLETE: %s", CHTTPDownloader::GetURL().c_str());

	IScriptTable* pScriptObject = GetScriptObject();

	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject->GetValue("OnComplete", pScriptFunction))
	{
		return;
	}

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(pScriptObject);
	m_pScriptSystem->EndCall();
}

//-------------------------------------------------------------------------------------------------
void CHTTPDownloader::OnCancel()
{
	m_pSystem->GetILog()->Log("DOWNLOAD CANCELED: %s", CHTTPDownloader::GetURL().c_str());

	IScriptTable* pScriptObject = GetScriptObject();

	HSCRIPTFUNCTION pScriptFunction = 0;

	if (!pScriptObject->GetValue("OnCancel", pScriptFunction))
	{
		return;
	}

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(pScriptObject);
	m_pScriptSystem->EndCall();
}

//-------------------------------------------------------------------------------------------------
// Script Functions
//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::Download(IFunctionHandler* pH)
{
	SCRIPT_CHECK_PARAMETERS(2);

	tuk szURL = 0;
	tuk szFileName = 0;

	pH->GetParam(1, szURL);
	pH->GetParam(2, szFileName);

	if (szURL && szFileName)
	{
		CHTTPDownloader::Download(szURL, szFileName);
	}

	return pH->EndFunction();
}

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::Cancel(IFunctionHandler* pH)
{
	CHTTPDownloader::Cancel();

	return pH->EndFunction();
}

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::Release(IFunctionHandler* pH)
{
	CHTTPDownloader::Release();

	return pH->EndFunction();
}

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::GetURL(IFunctionHandler* pH)
{
	return pH->EndFunction(CHTTPDownloader::GetURL().c_str());
}

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::GetFileSize(IFunctionHandler* pH)
{
	return pH->EndFunction(CHTTPDownloader::GetFileSize());
}

//-------------------------------------------------------------------------------------------------
i32 CHTTPDownloader::GetFileName(IFunctionHandler* pH)
{
	return pH->EndFunction(CHTTPDownloader::GetDstFileName().c_str());
}

#endif
