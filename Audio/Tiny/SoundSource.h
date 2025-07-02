#ifndef D3_SOUND_SOURCE_H
#define D3_SOUND_SOURCE_H

#include <drxtypes.h>
#include "Sound_C_Api.h"

class SoundSource
{
	struct SoundSourceInternalData* m_data;

public:
	SoundSource();
	virtual ~SoundSource();

	virtual bool computeSamples(double* sampleBuffer, i32 numSamples, double sampleRate);

	i32 getNumOscillators() const;
	void setOscillatorType(i32 oscillatorIndex, i32 type);
	void setOscillatorFrequency(i32 oscillatorIndex, double frequency);
	void setOscillatorAmplitude(i32 oscillatorIndex, double amplitude);
	void setOscillatorPhase(i32 oscillatorIndex, double phase);
	void setADSR(double attackRate, double decayRate, double sustainLevel, double releaseRate);

	bool setWavFile(i32 oscillatorIndex, class b3ReadWavFile* wavFilePtr, i32 sampleRate);

	void startSound(bool autoKeyOff);
	void stopSound();

	bool isAvailable() const;
};

#endif  //D3_SOUND_SOURCE_H
