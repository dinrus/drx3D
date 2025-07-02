// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include  <drx3D/Network/IVoiceEncoder.h>

CVoiceEncodingSession::CVoiceEncodingSession(IVoiceEncoder* pEnc) : m_pEnc(pEnc)
{
	m_buildFragment.resize(pEnc->GetFrameSize());
	m_buildPos = 0;
	m_seq = 0;
	++g_objcnt.voiceEncodingSession;
}

void CVoiceEncodingSession::AddVoiceFragment(i32 numSamples, i16k* pSamples, IVoicePacketSink* pSink)
{
	while (numSamples)
	{
		size_t copySize = min(m_buildFragment.size() - m_buildPos, size_t(numSamples));
		memcpy(&m_buildFragment[m_buildPos], pSamples, sizeof(i16) * copySize);
		m_buildPos += copySize;
		numSamples -= copySize;
		pSamples += copySize;
		if (m_buildPos == m_buildFragment.size())
		{
			TVoicePacketPtr pPacket = CVoicePacket::Allocate();
			pPacket->SetSeq(m_seq);
			m_pEnc->EncodeFrame(m_buildFragment.size(), &m_buildFragment[0], pPacket);
			m_buildPos = 0;
			m_seq++;
			pSink->AddPacket(pPacket);
		}
	}
}

void CVoiceEncodingSession::Reset(IVoicePacketSink* pSink)
{
	//fill up with silence to compress current data
	if (m_buildPos)
	{
		memset(&m_buildFragment[m_buildPos], 0, sizeof(i16) * (m_buildFragment.size() - m_buildPos));
		TVoicePacketPtr pPacket = CVoicePacket::Allocate();
		pPacket->SetSeq(m_seq);
		m_pEnc->EncodeFrame(m_buildFragment.size(), &m_buildFragment[0], pPacket);
		m_buildPos = 0;
		m_seq++;
		pSink->AddPacket(pPacket);
	}
	m_pEnc->Reset();
}

CVoiceEncodingSession::~CVoiceEncodingSession()
{
	SAFE_RELEASE(m_pEnc);
	--g_objcnt.voiceEncodingSession;
}

#endif
