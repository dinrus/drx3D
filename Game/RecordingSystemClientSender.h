// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RECORDINGSYSTEMCLIENTSENDER_H__
#define __RECORDINGSYSTEMCLIENTSENDER_H__

#include <drx3D/Game/RecordingSystemCircularBuffer.h>

#define KILLCAM_SEND_BUFFER_SIZE	(8*1024)

class CClientKillCamSender
{
public:
	enum
	{
		KCP_NEWFIRSTPERSON,
		KCP_NEWTHIRDPERSON,
		KCP_FORWARD,
		KCP_MAX
	};

	CClientKillCamSender();
	void Reset();
	void AddKill(IActor *pShooter, EntityId victim, bool bulletTimeKill, bool bToEveryone);
	void Update();

	void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddContainer(m_kills);
		pSizer->AddContainer(m_sentPackets);
	}

private:
	void SendData();
	void AddKillDataToSendQueue(IActor *pShooter, EntityId victim, float from, float to, bool bulletTimeKill, i32 &fpPacketOffset, i32 &tpPacketOffset, bool bFinal, bool bToEveryone);
	i32 AddFirstPersonDataToSendQueue(IActor *pShooter, EntityId victim, float from, float to, i32 packetOffset, bool bFinal, bool bToEveryone, float& timeoffset);
	size_t AddVictimDataToSendQueue(IActor *pShooter, EntityId victim, float from, float to, bool bulletTimeKill, i32 packetOffset, bool bFinal, bool bToEveryone, float timeoffset);
	void AddDataToSendQueue(IActor *pShooter, u8 packetType, i32 packetId, EntityId victim, uk data, size_t datasize, i32 packetOffset, bool bFinal, bool bToEveryone);

private:
	struct SSentKillCamPacket
	{
		float from;
		float to;
		i32 id;
		u8 numPackets;
	};

	struct SSendingState
	{
		IActor *pShooter;
		EntityId victim;
		u16 dataOffset;
		u16 dataSize;
		u8 packetType;
		u8 packetID;
		u8 packetOffset;
		u8 bFinalPacket:1;
		u8 bToEveryone:1;
	};

	struct KillQueue 
	{
		KillQueue(IActor *pShooter, EntityId victim, float deathTime, bool bKillHit, i32 fpPacketOffset, i32 tpPacketOffset, bool bToEveryone, float timeLeft)
		{
			m_pShooter=pShooter;
			m_victim=victim;
			m_startSendTime=deathTime;
			m_bSendKillHit=bKillHit;
			m_fpPacketOffset=fpPacketOffset;
			m_tpPacketOffset=tpPacketOffset;
			m_bToEveryone=bToEveryone;
			m_timeLeft=timeLeft;
		}
		IActor*	 m_pShooter;
		EntityId m_victim;
		float    m_startSendTime;
		float    m_timeLeft;
		i32			 m_fpPacketOffset;
		i32			 m_tpPacketOffset;
		bool		 m_bSendKillHit;
		bool		 m_bToEveryone;
	};

	typedef std::deque<SSentKillCamPacket> SentPacketQueue;

	std::deque<KillQueue> m_kills;
	SSendingState m_inflightPacket;
	SentPacketQueue m_sentPackets;
	CCircularBuffer<KILLCAM_SEND_BUFFER_SIZE> m_buffer;
	i32 m_packetId;
};

#endif // __RECORDINGSYSTEMCLIENTSENDER_H__