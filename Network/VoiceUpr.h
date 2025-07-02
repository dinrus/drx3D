// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  управляет голосовыми кодеками.
   -------------------------------------------------------------------------
   История:
   - 28/11/2005   : Created by Craig Tiller
*************************************************************************/
#ifndef __VOICEMANAGER_H__
#define __VOICEMANAGER_H__

#pragma once

#if !defined(OLD_VOICE_SYSTEM_DEPRECATED)

struct IVoiceEncoder;
struct IVoiceDecoder;

typedef IVoiceEncoder*(* VoiceEncoderFactory)();
typedef IVoiceDecoder*(* VoiceDecoderFactory)();

class CVoiceUpr
{
public:
	static IVoiceEncoder* CreateEncoder(tukk name);
	static IVoiceDecoder* CreateDecoder(tukk name);
};

class CAutoRegCodec
{
public:
	CAutoRegCodec(tukk name, VoiceEncoderFactory, VoiceDecoderFactory);

	IVoiceEncoder* CreateEncoder() const
	{
		return m_encoderFactory();
	}
	IVoiceDecoder* CreateDecoder() const
	{
		return m_decoderFactory();
	}

	static const CAutoRegCodec* FindCodec(tukk name);

private:
	static CAutoRegCodec* m_root;
	CAutoRegCodec*        m_next;
	tukk           m_name;
	VoiceEncoderFactory   m_encoderFactory;
	VoiceDecoderFactory   m_decoderFactory;
};

template<class T_Enc, class T_Dec>
class CAutoRegCodecT : public CAutoRegCodec
{
public:
	CAutoRegCodecT(tukk name) : CAutoRegCodec(name, CreateEncoder, CreateDecoder)
	{
	}

private:
	static IVoiceEncoder* CreateEncoder()
	{
		return new T_Enc();
	}
	static IVoiceDecoder* CreateDecoder()
	{
		return new T_Dec();
	}
};

	#define REGISTER_CODEC(name, encoder, decoder) CAutoRegCodecT<encoder, decoder> codec_ ## name( # name)

#endif
#endif
