// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if DRX_PLATFORM_WINDOWS

class CHTTPDownloader;

class CDownloadUpr
{
public:
	CDownloadUpr();
	virtual ~CDownloadUpr();

	void             Create(ISystem* pSystem);
	CHTTPDownloader* CreateDownload();
	void             RemoveDownload(CHTTPDownloader* pDownload);
	void             Update();
	void             Release();

private:

	ISystem*                    m_pSystem;
	std::list<CHTTPDownloader*> m_lDownloadList;
};

#endif
