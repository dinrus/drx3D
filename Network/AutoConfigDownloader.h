// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __AUTOCONFIGDOWNLOADER_H__
#define __AUTOCONFIGDOWNLOADER_H__

#pragma once

#include <drx3D/Sys/IConsole.h>
#include <drx3D/Network/INetworkService.h>

class CAutoConfigDownloader : private IDownloadStream
{
public:
	CAutoConfigDownloader();

	void TriggerRefresh();
	void Update();

private:
	void GotData(u8k* pData, u32 length);
	void Complete(bool success);

	void Reconfigure(const string& loc);
	string                        m_url;
	string                        m_buffer;
	string                        m_toExec;
	bool                          m_executing;

	static CAutoConfigDownloader* m_pThis;

	static void OnCVarChanged(ICVar* pVar);
};

#endif
