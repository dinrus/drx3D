// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include  <drx3D/Network/IVoiceDecoder.h>
	#include  <drx3D/Network/Network.h>

//static const float DEBUG_FADEOUT = 1;

CVoiceDecodingSession::CVoiceDecodingSession(IVoiceDecoder* d) : m_pDecoder(d)
{
	m_counter = 10;
	m_endCounter = 0;
	m_zeros = false;
	m_currentFramePos = 0;
	m_currentFrameLength = 0;
	m_minPendingPackets = 999999;
	m_samplesSinceCorrection = 0;
	m_skippedPackets = 10000;
	m_mute = false;
	m_paused = false;
}

	#undef min
	#undef max

void CVoiceDecodingSession::AddPacket(const TVoicePacketPtr pkt)
{
	DrxAutoCriticalSection lk(m_lock);

	if (m_mute || m_paused)
		return;

	if (!m_packets.empty() && m_packets.begin()->first < m_counter)
	{
		NetWarning("Resetting voice decoding counter");
		m_counter = m_packets.begin()->first - std::min(m_packets.begin()->first, u32(CVARS.VoiceLeadPackets));
	}

	if (m_packets.empty() && m_counter > m_endCounter + CVARS.VoiceTrailPackets)
	{
		m_counter = 0;
		m_endCounter = CVARS.VoiceLeadPackets;
		m_endSeq = pkt->GetSeq();
		m_packets.insert(std::make_pair(CVARS.VoiceLeadPackets, pkt));
	}
	else
	{
		u32 pos = u8(pkt->GetSeq() - m_endSeq) + m_endCounter;
		if (pos <= m_endCounter)
			pos = m_endCounter + 1;
		m_endCounter = pos;
		m_endSeq = pkt->GetSeq();
		m_packets.insert(std::make_pair(pos, pkt));
	}

	//in case client does not request sound data at all
	while (m_packets.size() > 100)
	{
		m_packets.erase(m_packets.begin());
		m_counter++;
	}
}

//called from sound thread
void CVoiceDecodingSession::GetSamples(i32 inNumSamples, i16* pSamples)
{
	DrxAutoCriticalSection lk(m_lock);

	enum DECODE_MODE
	{
		DECODE_SKIPPED,
		DECODE_NORMAL,
		DECODE_ZEROS,
	};

	i32 numSamples = inNumSamples;

	while (numSamples)
	{
		if (m_currentFramePos == m_currentFrameLength)
		{
			DECODE_MODE mode = DECODE_SKIPPED;

			if (!m_packets.empty())
			{
				if (m_packets.begin()->first == m_counter)
					mode = DECODE_NORMAL;
			}
			else if (m_counter > m_endCounter + CVARS.VoiceTrailPackets)
			{
				mode = DECODE_ZEROS;
			}

			if (m_skippedPackets > CVARS.VoiceTrailPackets && mode == DECODE_SKIPPED)
			{
				mode = DECODE_ZEROS;
			}

			// TODO: craig: figure out why this code needs to be here
			i32 frameSize = m_pDecoder->GetFrameSize();
			if (frameSize > BUFFER_SIZE)
			{
				NET_ASSERT(false);
				if (mode != DECODE_ZEROS)
				{
					m_packets.erase(m_packets.begin());
					mode = DECODE_ZEROS;
				}
				frameSize = BUFFER_SIZE;
			}

			if (m_mute || m_paused)
			{
				mode = DECODE_ZEROS;
			}

			switch (mode)
			{
			case DECODE_NORMAL:
				m_pDecoder->DecodeFrame(*(m_packets.begin()->second), frameSize, m_currentFrame);
				m_skippedPackets = 0;
				m_packets.erase(m_packets.begin());
				break;
			case DECODE_SKIPPED:
				m_pDecoder->DecodeSkippedFrame(frameSize, m_currentFrame);
				m_skippedPackets++;
				m_stats.SkippedSamples += frameSize;
				break;
			case DECODE_ZEROS:
				memset(m_currentFrame, 0, frameSize * sizeof(i16));
				m_stats.ZeroSamples += frameSize;
				break;
			}

			m_currentFrameLength = frameSize;
			m_currentFramePos = 0;
			m_counter++;
		}

		i32 copySize = min(numSamples, m_currentFrameLength - m_currentFramePos);
		memcpy(pSamples, m_currentFrame + m_currentFramePos, copySize * sizeof(i16));
		pSamples += copySize;
		m_currentFramePos += copySize;
		numSamples -= copySize;
	}

	if (m_packets.size() < (u32)m_minPendingPackets)
		m_minPendingPackets = m_packets.size();

	m_samplesSinceCorrection += inNumSamples;
	if (m_samplesSinceCorrection >= 8000)
	{
		m_samplesSinceCorrection = 0;

		for (i32 i = 0; i < std::max(0, m_minPendingPackets - 2) && !m_packets.empty(); ++i)
			m_packets.erase(m_packets.begin());

		m_minPendingPackets = 999999;
	}

	u32 pendingPackets = GetPendingPackets();
	m_stats.MaxPendingPackets = std::max(pendingPackets, m_stats.MaxPendingPackets);
	m_stats.MinPendingPackets = std::min(pendingPackets, m_stats.MinPendingPackets);
	if (!m_packets.empty())
		m_stats.IDFirst = m_packets.begin()->first;
	m_stats.IDCounter = m_counter;
}

u32 CVoiceDecodingSession::GetPendingPackets()
{
	DrxAutoCriticalSection lk(m_lock);

	return m_packets.size();
}

void CVoiceDecodingSession::GetStats(SDecodingStats& s)
{
	DrxAutoCriticalSection lk(m_lock);

	GetSamples(0, 0);
	s = m_stats;
	m_stats = SDecodingStats();
}

void CVoiceDecodingSession::GetPackets(std::vector<TVoicePacketPtr>& p)
{
	DrxAutoCriticalSection lk(m_lock);

	//HACK
	size_t sz = m_packets.size();
	p.resize(sz);
	for (size_t i = 0; i < sz; i++)
	{
		p[i] = m_packets.begin()->second;
		m_packets.erase(m_packets.begin());
	}
}

void CVoiceDecodingSession::Mute(bool mute)
{
	DrxAutoCriticalSection lk(m_lock);

	m_mute = mute;
	if (m_mute)
	{
		m_packets.clear();
	}
}

void CVoiceDecodingSession::Pause(bool pause)
{
	DrxAutoCriticalSection lk(m_lock);

	m_paused = pause;
	if (m_paused)
	{
		m_packets.clear();
	}
}

#endif
