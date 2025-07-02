// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IVOICEDECODER_H__
#define __IVOICEDECODER_H__

#pragma once

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include <drx3D/Network/VoicePacket.h>
	#include <queue>
	#include <drx3D/CoreX/Memory/STLPoolAllocator.h>

struct IVoiceDecoder
{
	IVoiceDecoder()
	{
		++g_objcnt.voiceDecoder;
	}
	virtual ~IVoiceDecoder()
	{
		--g_objcnt.voiceDecoder;
	}
	virtual i32  GetFrameSize() = 0;
	virtual void DecodeFrame(const CVoicePacket& pkt, i32 frameSize, i16* samples) = 0;
	virtual void DecodeSkippedFrame(i32 frameSize, i16* samples) = 0;
	virtual void Reset() = 0;
	virtual void Release() = 0;
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) = 0;
};

struct SDecodingStats
{
	SDecodingStats() : MaxPendingPackets(0), MinPendingPackets(9999), ZeroSamples(0), SkippedSamples(0)
	{

	}
	u32 MaxPendingPackets;
	u32 MinPendingPackets;
	u32 ZeroSamples;
	u32 SkippedSamples;
	u32 IDFirst, IDCounter;
};

class CVoiceDecodingSession
{
public:
	CVoiceDecodingSession(IVoiceDecoder*);
	void   AddPacket(TVoicePacketPtr pkt);
	void   GetSamples(i32 numSamples, i16* pSamples);
	u32 GetPendingPackets();
	void   GetStats(SDecodingStats& s);
	void   GetPackets(std::vector<TVoicePacketPtr>&);
	void   Mute(bool mute);
	void   Pause(bool pause);

	void   GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CVoiceDecodingSession");

		if (!pSizer->Add(*this))
			return;
		pSizer->AddContainer(m_packets);
		pSizer->AddObject(&m_packets, m_packets.size() * sizeof(CVoicePacket));
	}
private:
	static i32k   BUFFER_SIZE = 8192;

	DrxCriticalSection m_lock;

	IVoiceDecoder*     m_pDecoder;
	#if USE_SYSTEM_ALLOCATOR
	typedef std::map<u32, TVoicePacketPtr, std::less<u32>>                                                            PacketMap;
	#else
	typedef std::map<u32, TVoicePacketPtr, std::less<u32>, stl::STLPoolAllocator<std::pair<u32k, TVoicePacketPtr>>> PacketMap;
	#endif
	PacketMap      m_packets;
	u32         m_counter;
	u32         m_endCounter;
	u8          m_endSeq;
	bool           m_zeros;
	i32            m_minPendingPackets;
	i32            m_samplesSinceCorrection;
	i32            m_skippedPackets;
	bool           m_mute;
	bool           m_paused;

	i16          m_currentFramePos;
	i16          m_currentFrameLength;
	i16          m_currentFrame[BUFFER_SIZE];

	SDecodingStats m_stats;
};

#endif
#endif
