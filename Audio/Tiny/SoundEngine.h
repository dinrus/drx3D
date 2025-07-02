#ifndef D3_SOUND_ENGINE_H
#define D3_SOUND_ENGINE_H

#include <drx3D/Common/b3Scalar.h>
#include "Sound_C_Api.h"

struct SoundMessage
{
	i32 m_type;  //D3_SOUND_SOURCE_TYPE
	double m_amplitude;

	double m_frequency;
	i32 m_wavId;

	double m_attackRate;
	double m_decayRate;
	double m_sustainLevel;
	double m_releaseRate;
	bool m_autoKeyOff;

	SoundMessage()
		: m_type(D3_SOUND_SOURCE_SINE_OSCILLATOR),
		  m_amplitude(0.5),
		  m_frequency(440),
		  m_wavId(-1),
		  m_attackRate(0.001),
		  m_decayRate(0.00001),
		  m_sustainLevel(0.5),
		  m_releaseRate(0.0005),
		  m_autoKeyOff(false)
	{
	}
};

class SoundEngine
{
	struct b3SoundEngineInternalData* m_data;

public:
	SoundEngine();
	virtual ~SoundEngine();

	void init(i32 maxNumSoundSources, bool useRealTimeDac);
	void exit();

	i32 getAvailableSoundSource();
	void startSound(i32 soundSourceIndex, SoundMessage msg);
	void releaseSound(i32 soundSourceIndex);

	i32 loadWavFile(tukk fileName);

	double getSampleRate() const;
};

#endif  //D3_SOUND_ENGINE_H
