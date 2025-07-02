// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/DebugOutput.h>

void CDebugOutput::Put(u8 val)
{
	DrxAutoLock<CDebugOutput> lk(*this);
	if (m_buffers.empty() || !m_buffers.back().ready)
		m_buffers.push_back(SBuf());
	NET_ASSERT(m_buffers.back().ready);
	SBuf& buf = m_buffers.back();
	buf.data[buf.sz++] = val;
	if (buf.sz == DATA_SIZE)
		buf.ready = false;
}

void CDebugOutput::Run(DRXSOCKET sock)
{
	{
		DrxAutoLock<CDebugOutput> lk(*this);
		m_buffers.resize(0);
		Write('i');
		Write(m_sessionID.GetHumanReadable());
	}

	while (true)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		timeval timeout;
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;
		// not portable to *nix
		i32 n = select(DRXSOCKET(0), NULL, &fds, NULL, &timeout);

		switch (n)
		{
		case 0:
		case DRX_SOCKET_ERROR:
			return;
		}

		bool sleep = false;
		{
			DrxAutoLock<CDebugOutput> lk(*this);
			if (m_buffers.empty() || m_buffers.front().ready)
				sleep = true;
			else
			{
				SBuf& buf = m_buffers.front();
				i32 w = DrxSock::send(sock, buf.data + buf.pos, buf.sz - buf.pos, 0);
				DrxSock::eDrxSockError sockErr = DrxSock::TranslateSocketError(w);
				if (sockErr != DrxSock::eCSE_NO_ERROR)
				{
					if (sockErr == DrxSock::eCSE_EWOULDBLOCK)
						sleep = true;
					else
						return;
				}
				else
				{
					buf.pos += w;
					if (buf.pos == buf.sz)
						m_buffers.pop_front();
				}
			}
		}
		if (sleep)
		{
			DrxSleep(1000);
		}
	}
}
