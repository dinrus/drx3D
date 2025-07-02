// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RECORDINGSYSTEMSERVERFORWARDER_H__
#define __RECORDINGSYSTEMSERVERFORWARDER_H__

#include <drx3D/Game/RecordingSystemCircularBuffer.h>

#define KILLCAM_FORWARDING_BUFFER_SIZE	(16*1024)

class CServerKillCamForwarder
{
	typedef CCircularBuffer<KILLCAM_FORWARDING_BUFFER_SIZE> BufferType;
	struct SForwardingPacket;

public:
	void Reset();
	void ReceivePacket(IActor *pActor, const CActor::KillCamFPData &packet);
	void Update();
	void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddContainer(m_forwarding);
	}

private:
	void ForwardPacket(SForwardingPacket &forward, CActor::KillCamFPData *pPacket);
	void DropOldestPacket();
	void AddDataAndErase(ukk data, size_t size);
	void GetFirstPacket(BufferType::iterator &first, IActor *pActor, i32 packetId);
	CActor::KillCamFPData* FindPacket(BufferType::iterator &it, IActor *pActor, i32 packetId);
	
	struct SForwardingPacket
	{
		SForwardingPacket()
			: pActor(nullptr)
			, m_sent(0)
			, packetID(0)
			, m_numPackets(0)
		{}

		EntityId victim;
		IActor *pActor;
		i32 packetID;
		i32 m_sent;
		i32 m_numPackets;
		CTimeValue m_lastPacketTime;
		BufferType::iterator iterator;
	};

	std::deque<SForwardingPacket> m_forwarding;
	BufferType m_history;
};

#endif // __RECORDINGSYSTEMSERVERFORWARDER_H__