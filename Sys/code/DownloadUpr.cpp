// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/DownloadUpr.h>
#include <drx3D/Sys/HTTPDownloader.h>

#if DRX_PLATFORM_WINDOWS

//-------------------------------------------------------------------------------------------------
CDownloadUpr::CDownloadUpr()
	: m_pSystem(0)
{
}

//-------------------------------------------------------------------------------------------------
CDownloadUpr::~CDownloadUpr()
{
}

//-------------------------------------------------------------------------------------------------
void CDownloadUpr::Create(ISystem* pSystem)
{
	m_pSystem = pSystem;
}

//-------------------------------------------------------------------------------------------------
CHTTPDownloader* CDownloadUpr::CreateDownload()
{
	CHTTPDownloader* pDL = new CHTTPDownloader;

	m_lDownloadList.push_back(pDL);

	pDL->Create(m_pSystem, this);

	return pDL;
}

//-------------------------------------------------------------------------------------------------
void CDownloadUpr::RemoveDownload(CHTTPDownloader* pDownload)
{
	std::list<CHTTPDownloader*>::iterator it = std::find(m_lDownloadList.begin(), m_lDownloadList.end(), pDownload);

	if (it != m_lDownloadList.end())
	{
		m_lDownloadList.erase(it);
	}
}

//-------------------------------------------------------------------------------------------------
void CDownloadUpr::Update()
{
	std::list<CHTTPDownloader*>::iterator it = m_lDownloadList.begin();

	while (it != m_lDownloadList.end())
	{
		CHTTPDownloader* pDL = *it;

		switch (pDL->GetState())
		{
		case HTTP_STATE_NONE:
		case HTTP_STATE_WORKING:
			++it;
			continue;
		case HTTP_STATE_COMPLETE:
			pDL->OnComplete();
			break;
		case HTTP_STATE_ERROR:
			pDL->OnError();
			break;
		case HTTP_STATE_CANCELED:
			pDL->OnCancel();
			break;
		}

		it = m_lDownloadList.erase(it);

		pDL->Release();
	}
}

//-------------------------------------------------------------------------------------------------
void CDownloadUpr::Release()
{
	for (std::list<CHTTPDownloader*>::iterator it = m_lDownloadList.begin(); it != m_lDownloadList.end(); )
	{
		CHTTPDownloader* pDL = *it;

		it = m_lDownloadList.erase(it);

		pDL->Release();
	}

	delete this;
}

#endif
