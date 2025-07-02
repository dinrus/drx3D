#ifndef D3_AUDIO_LISTENER_H
#define D3_AUDIO_LISTENER_H

#include <drxtypes.h>

class SoundSource;

class AudioListener
{
	struct AudioListenerInternalData* m_data;

public:
	AudioListener();
	virtual ~AudioListener();

	static i32 tick(uk outputBuffer, uk inputBuffer1, u32 nBufferFrames,
					double streamTime, u32 status, uk dataPointer);

	i32 addSoundSource(SoundSource* source);
	void removeSoundSource(SoundSource* source);

	AudioListenerInternalData* getTickData();
	const AudioListenerInternalData* getTickData() const;

	double getSampleRate() const;
	void setSampleRate(double sampleRate);
};

#endif  //D3_AUDIO_LISTENER_H