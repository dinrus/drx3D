// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __DISTRIBUTEDLOGGER_H__
#define __DISTRIBUTEDLOGGER_H__

#pragma once

#include <drx3D/Network/Config.h>
#if ENABLE_DISTRIBUTED_LOGGER

	#include <drx3D/Network/IDatagramSocket.h>
	#include <drx3D/Network/NetResolver.h>

class CDistributedLogger : public ILogCallback, public IDatagramListener
{
public:
	CDistributedLogger();
	~CDistributedLogger();

	// ILogCallback
	virtual void OnWriteToConsole(tukk sText, bool bNewLine);
	virtual void OnWriteToFile(tukk sText, bool bNewLine);
	// ~ILogCallback

	virtual void OnPacket(const TNetAddress& addr, TMemHdl hdl);
	virtual void OnError(const TNetAddress& addr, ESocketError error) {}

	void         Update(CTimeValue now);

private:
	static const size_t MAX_PKT_SIZE = 1300;
	char                m_buffer[MAX_PKT_SIZE];
	size_t              m_bufferPos;
	CTimeValue          m_lastPoll;
	bool                m_inPoll;
	bool                m_inFlush;

	void Update_BetweenPoll(CTimeValue now);
	void Update_InPoll(CTimeValue now);
	void InitBuffer();
	void Flush();

	IDatagramSocketPtr          m_pSocket;

	std::map<TNetAddress, bool> m_listeners;
};
#endif

#endif
