// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __LOCALDATAGRAMSOCKET_H__
#define __LOCALDATAGRAMSOCKET_H__

#pragma once

#include <drx3D/Network/Network.h>
#include <drx3D/Network/IDatagramSocket.h>
#include <queue>

class CLocalDatagramSocket : public CDatagramSocket
{
public:
	CLocalDatagramSocket();
	~CLocalDatagramSocket();

	bool Init(TLocalNetAddress addr);

	// IDatagramSocket
	virtual void         GetSocketAddresses(TNetAddressVec& addrs) override;
	virtual ESocketError Send(u8k* pBuffer, size_t nLength, const TNetAddress& to) override;
	virtual ESocketError SendVoice(u8k* pBuffer, size_t nLength, const TNetAddress& to) override;
	virtual void         Die() override                                      { m_isDead = true; }
	virtual bool         IsDead() override                                   { return m_isDead; }
	virtual void         RegisterBackoffAddress(const TNetAddress& addr) override   {}
	virtual void         UnregisterBackoffAddress(const TNetAddress& addr) override {}
	virtual void         GetMemoryStatistics(IDrxSizer* pSizer) override
	{
		SIZER_COMPONENT_NAME(pSizer, "CLocalDatagramSocket");

		pSizer->Add(*this);
		pSizer->AddContainer(m_fragmentationAddresses);
		pSizer->AddContainer(m_sendErrorAddresses);

		for (size_t i = 0; i < m_packets.size(); ++i)
		{
			SPacket* packet = m_packets.front();
			pSizer->AddObject(packet, sizeof(SPacket));
			m_packets.pop();
			m_packets.push(packet);
		}

		if (m_pUpr)
			m_pUpr->GetMemoryStatistics(pSizer);
	}
	// ~IDatagramSocket

private:
	void OnRead();

	TLocalNetAddress    m_addr;

	static const size_t MAX_PACKET_SIZE = MAX_UDP_PACKET_SIZE;
	struct SPacket
	{
		size_t           nBytes;
		TLocalNetAddress addr;
		u8            vData[MAX_PACKET_SIZE];
	};
	std::queue<SPacket*>       m_packets;
	std::set<TLocalNetAddress> m_fragmentationAddresses;
	std::set<TLocalNetAddress> m_sendErrorAddresses;

	class CUpr
	{
	public:
		CUpr();
		~CUpr();

		bool Register(CLocalDatagramSocket* pSock);
		bool Unregister(CLocalDatagramSocket* pSock);
		CLocalDatagramSocket* GetSocket(TLocalNetAddress);

		SPacket* AllocPacket();
		void     ReleasePacket(SPacket*);

		void     GetMemoryStatistics(IDrxSizer* pSizer)
		{
			SIZER_COMPONENT_NAME(pSizer, "CLocalDatagramSocket::CUpr");

			if (!pSizer->Add(*this))
				return;
			pSizer->AddContainer(m_freeAddresses);
			pSizer->AddContainer(m_sockets);
			pSizer->AddContainer(m_freePackets);

			for (size_t i = 0; i < m_freePackets.size(); ++i)
			{
				pSizer->AddObject(m_freePackets[i], sizeof(SPacket));
				//MMM_PACKETDATA.AddHdlToSizer(m_freePackets[i]->hdl, pSizer);
			}

		}

	private:
		static const TLocalNetAddress                     MAX_FREE_ADDRESSES = 16;
		std::vector<TLocalNetAddress>                     m_freeAddresses;
		std::map<TLocalNetAddress, CLocalDatagramSocket*> m_sockets;
		std::vector<SPacket*>                             m_freePackets;
	};
	static CUpr* m_pUpr;

	struct SPacketCleanup
	{
		SPacketCleanup(SPacket* pPacket) : m_pPacket(pPacket) {}
		~SPacketCleanup() { m_pUpr->ReleasePacket(m_pPacket); }
		SPacket* m_pPacket;
	};

	bool m_isDead;
};

#endif
