// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

	#include  <drx3D/Network/VoiceUpr.h>
	#include  <drx3D/Network/IVoiceEncoder.h>
	#include  <drx3D/Network/IVoiceDecoder.h>
	#include <drx3D/CoreX/Renderer/Tarray.h>

	#include <speex.h>
	#include <speex/speex_preprocess.h>

//TODO: move to syncronized CVAR
i32k SAMPLE_RATE = 8000;

static std::vector<float> tempBuffer;

class CSpeexEncoder : public IVoiceEncoder
{
public:
	CSpeexEncoder()
	{
		speex_bits_init(&m_bits);
		m_pEncState = speex_encoder_init(&speex_nb_mode);

		m_pPreprocessor = speex_preprocess_state_init(GetFrameSize(), SAMPLE_RATE);

		i32 tmp = 12000;
		speex_encoder_ctl(m_pEncState, SPEEX_SET_ABR, &tmp);
		/*i32 tmp=9;
		   speex_encoder_ctl( m_pEncState, SPEEX_SET_QUALITY, &tmp);*/
		tmp = 1;
		speex_encoder_ctl(m_pEncState, SPEEX_SET_VAD, &tmp);
		tmp = 1;
		speex_encoder_ctl(m_pEncState, SPEEX_SET_DTX, &tmp);
		tmp = SAMPLE_RATE;
		speex_encoder_ctl(m_pEncState, SPEEX_SET_SAMPLING_RATE, &tmp);

		tmp = 1;
		speex_preprocess_ctl(m_pPreprocessor, SPEEX_PREPROCESS_SET_VAD, &tmp);
		tmp = 1;
		speex_preprocess_ctl(m_pPreprocessor, SPEEX_PREPROCESS_SET_DENOISE, &tmp);
		tmp = 1;
		speex_preprocess_ctl(m_pPreprocessor, SPEEX_PREPROCESS_SET_AGC, &tmp);
	}

	~CSpeexEncoder()
	{
		speex_bits_destroy(&m_bits);
		speex_encoder_destroy(m_pEncState);
		speex_preprocess_state_destroy(m_pPreprocessor);
	}

	virtual i32 GetFrameSize()
	{
		i32 frameSize;
		speex_encoder_ctl(m_pEncState, SPEEX_GET_FRAME_SIZE, &frameSize);
		return frameSize;
	}

	virtual void EncodeFrame(i32 numSamples, i16k* pSamples, TVoicePacketPtr pkt)
	{
		if (!speex_preprocess(m_pPreprocessor, const_cast<i16*>(pSamples), NULL))
		{
			pkt->SetLength(0);
			return;
		}

		if (tempBuffer.size() < (u32)numSamples)
			tempBuffer.resize((u32)numSamples);
		for (i32 i = 0; i < numSamples; i++)
			tempBuffer[i] = pSamples[i];

		speex_bits_reset(&m_bits);
		bool transmit = (0 != speex_encode(m_pEncState, &tempBuffer[0], &m_bits));
		if (transmit)
		{
			i32 len = speex_bits_write(&m_bits, (tuk)pkt->GetData(), CVoicePacket::MAX_LENGTH);
			pkt->SetLength(len);
		}
		else
			pkt->SetLength(0);
	}

	virtual void Reset()
	{
		//speex_encoder_ctl( m_pEncState, SPEEX_RESET_STATE, 0);
	}

	virtual void Release()
	{
		delete this;
	}

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CSpeexEncoder");

		pSizer->Add(*this);
		pSizer->AddObject(m_pPreprocessor, sizeof(SpeexPreprocessState));
	}

private:
	SpeexBits             m_bits;
	uk                 m_pEncState;
	SpeexPreprocessState* m_pPreprocessor;
};

class CSpeexDecoder : public IVoiceDecoder
{
public:
	CSpeexDecoder()
	{
		speex_bits_init(&m_bits);
		m_pDecState = speex_decoder_init(&speex_nb_mode);
		i32 tmp = SAMPLE_RATE;
		speex_decoder_ctl(m_pDecState, SPEEX_SET_SAMPLING_RATE, &tmp);
	}

	~CSpeexDecoder()
	{
		speex_bits_destroy(&m_bits);
		speex_decoder_destroy(m_pDecState);
	}

	virtual i32 GetFrameSize()
	{
		i32 frameSize;
		speex_decoder_ctl(m_pDecState, SPEEX_GET_FRAME_SIZE, &frameSize);
		return frameSize;
	}

	virtual void DecodeFrame(const CVoicePacket& pkt, i32 numSamples, i16* samples)
	{
		if (tempBuffer.size() < (u32)numSamples)
			tempBuffer.resize(numSamples);

		speex_bits_read_from(&m_bits, (tuk)pkt.GetData(), pkt.GetLength());
		speex_decode(m_pDecState, &m_bits, &tempBuffer[0]);

		for (i32 i = 0; i < numSamples; i++)
			samples[i] = (short) CLAMP(tempBuffer[i], -32768.0f, 32767.0f);
	}

	virtual void DecodeSkippedFrame(i32 numSamples, i16* samples)
	{
		if (tempBuffer.size() < (u32)numSamples)
			tempBuffer.resize(numSamples);

		//speex_bits_read_from( &m_bits, NULL, 0 );
		i32 ret = speex_decode(m_pDecState, NULL, &tempBuffer[0]);

		NET_ASSERT(!ret && "error decoding stream");

		//NET_ASSERT(speex_bits_remaining(&m_bits)<0 && "error decoding stream - stream is corrupted");

		for (i32 i = 0; i < numSamples; i++)
			samples[i] = (short) CLAMP(tempBuffer[i], -32768.0f, 32767.0f);
	}

	virtual void Release()
	{
		delete this;
	}

	virtual void Reset()
	{
		//speex_decoder_ctl( m_pDecState, SPEEX_RESET_STATE, 0);
	}

	virtual void GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CSpeexDecoder");

		pSizer->Add(*this);
	}

private:
	SpeexBits m_bits;
	uk     m_pDecState;
};

REGISTER_CODEC(speex, CSpeexEncoder, CSpeexDecoder);

#endif
