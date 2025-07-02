// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IVOICEENCODER_H__
#define __IVOICEENCODER_H__

#pragma once

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include <drx3D/Network/VoicePacket.h>

struct IVoiceEncoder
{
	IVoiceEncoder()
	{
		++g_objcnt.voiceEncoder;
	}
	virtual ~IVoiceEncoder()
	{
		--g_objcnt.voiceEncoder;
	}
	virtual i32  GetFrameSize() = 0;
	virtual void EncodeFrame(i32 numSamples, i16k* pSamples, TVoicePacketPtr vp) = 0;
	virtual void Reset() = 0;
	virtual void Release() = 0;
	virtual void GetMemoryStatistics(IDrxSizer* pSizer) = 0;
};

struct IVoicePacketSink
{
	virtual ~IVoicePacketSink(){}
	virtual void AddPacket(const TVoicePacketPtr& pkt) = 0;
};

class CVoiceEncodingSession
{
public:
	CVoiceEncodingSession(IVoiceEncoder* pEnc);
	~CVoiceEncodingSession();
	void AddVoiceFragment(i32 numSamples, i16k* pSamples, IVoicePacketSink* pSink);
	void Reset(IVoicePacketSink* pSink);
	void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CVoiceEncodingSession");

		if (!pSizer->Add(*this))
			return;
		pSizer->AddContainer(m_buildFragment);
	}

private:
	IVoiceEncoder*     m_pEnc;
	std::vector<i16> m_buildFragment;
	size_t             m_buildPos;
	u8              m_seq;
};

#endif
#endif
