// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/LocalDatagramSocket.h>
#include  <drx3D/Network/Network.h>

CLocalDatagramSocket::CUpr* CLocalDatagramSocket::m_pUpr = NULL;

CLocalDatagramSocket::CLocalDatagramSocket() : m_addr(0), m_isDead(false)
{
}

bool CLocalDatagramSocket::Init(TLocalNetAddress addr)
{
	NET_ASSERT(0 == m_addr);
	m_addr = addr;

	if (!m_pUpr)
		m_pUpr = new CUpr();
	return m_pUpr->Register(this);
}

CLocalDatagramSocket::~CLocalDatagramSocket()
{
	if (m_pUpr && m_pUpr->Unregister(this))
		SAFE_DELETE(m_pUpr);
}

void CLocalDatagramSocket::GetSocketAddresses(TNetAddressVec& addrs)
{
	addrs.push_back(TNetAddress(m_addr));
}

ESocketError CLocalDatagramSocket::Send(u8k* pBuffer, size_t nLength, const TNetAddress& to)
{
	if (m_isDead)
		return eSE_MiscFatalError;

	const TLocalNetAddress* addr = stl::get_if<TLocalNetAddress>(&to);
	if (!addr)
		return eSE_Ok; // unreachable address will clobber udp sending

	CLocalDatagramSocket* pTarget = m_pUpr->GetSocket(*addr);
	if (!pTarget)
	{
		NET_TO_NET(&CLocalDatagramSocket::OnRead, this);
		m_sendErrorAddresses.insert(*addr);
		return eSE_Ok;
	}

	if (nLength > MAX_PACKET_SIZE)
	{
		NET_TO_NET(&CLocalDatagramSocket::OnRead, this);
		m_fragmentationAddresses.insert(*addr);
		return eSE_Ok;
	}

	SPacket* pPacket = m_pUpr->AllocPacket();
	memcpy(pPacket->vData, pBuffer, nLength);
	pPacket->addr = m_addr;
	pPacket->nBytes = nLength;
	pTarget->m_packets.push(pPacket);
	NET_TO_NET(&CLocalDatagramSocket::OnRead, pTarget);

	return eSE_Ok;
}

ESocketError CLocalDatagramSocket::SendVoice(u8k* pBuffer, size_t nLength, const TNetAddress& to)
{
	return Send(pBuffer, nLength, to);
}

void CLocalDatagramSocket::OnRead()
{
	if (!m_fragmentationAddresses.empty())
	{
		TNetAddress addr(*m_fragmentationAddresses.begin());
		m_fragmentationAddresses.erase(m_fragmentationAddresses.begin());
		OnError(addr, eSE_FragmentationOccured);
	}
	else if (!m_sendErrorAddresses.empty())
	{
		TNetAddress addr(*m_sendErrorAddresses.begin());
		m_sendErrorAddresses.erase(m_sendErrorAddresses.begin());
		OnError(addr, eSE_UnreachableAddress);
	}
	else if (!m_packets.empty())
	{
		SPacket* pPacket = m_packets.front();
		m_packets.pop();
		SPacketCleanup cleanupPacket(pPacket);

		OnPacket(TNetAddress(pPacket->addr), pPacket->vData, pPacket->nBytes);
	}
}

/* CUpr impl */

CLocalDatagramSocket::CUpr::CUpr()
{
	m_freeAddresses.reserve(MAX_FREE_ADDRESSES);
	for (TLocalNetAddress i = 1; i <= MAX_FREE_ADDRESSES; i++)
		m_freeAddresses.push_back(i);
	NET_ASSERT(m_freeAddresses.size() == MAX_FREE_ADDRESSES);
}

CLocalDatagramSocket::CUpr::~CUpr()
{
	while (!m_freePackets.empty())
	{
		delete m_freePackets.back();
		m_freePackets.pop_back();
	}
}

bool CLocalDatagramSocket::CUpr::Register(CLocalDatagramSocket* pSock)
{
	// address allocation
	while (0 == pSock->m_addr)
	{
		if (m_freeAddresses.empty())
		{
			NetWarning("No free local net addresses");
			return false;
		}
		if (m_sockets.find(m_freeAddresses.back()) == m_sockets.end())
			pSock->m_addr = m_freeAddresses.back();
		m_freeAddresses.pop_back();
	}

	if (m_sockets.find(pSock->m_addr) != m_sockets.end())
		return false;

	m_sockets[pSock->m_addr] = pSock;
	return true;
}

bool CLocalDatagramSocket::CUpr::Unregister(CLocalDatagramSocket* pSock)
{
	if (pSock->m_addr && pSock->m_addr <= MAX_FREE_ADDRESSES)
		m_freeAddresses.push_back(pSock->m_addr);
	m_sockets.erase(pSock->m_addr);
	return m_sockets.empty();
}

CLocalDatagramSocket* CLocalDatagramSocket::CUpr::GetSocket(TLocalNetAddress addr)
{
	return stl::find_in_map(m_sockets, addr, NULL);
}

CLocalDatagramSocket::SPacket* CLocalDatagramSocket::CUpr::AllocPacket()
{
	SPacket* pPacket;
	if (m_freePackets.empty())
		pPacket = new SPacket;
	else
	{
		pPacket = m_freePackets.back();
		m_freePackets.pop_back();
	}
	return pPacket;
}

void CLocalDatagramSocket::CUpr::ReleasePacket(SPacket* pPacket)
{
	m_freePackets.push_back(pPacket);
}
