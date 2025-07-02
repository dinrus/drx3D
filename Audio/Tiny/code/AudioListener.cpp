#include "../AudioListener.h"
#include "../SoundSource.h"
#include <drx3D/Common/b3Logging.h>
#include "../WriteWavFile.h"
#include <math.h>

template <class T>
inline const T& MyMin(const T& a, const T& b)
{
	return a < b ? a : b;
}
#define MAX_SOUND_SOURCES 128
#define D3_SAMPLE_RATE 48000

struct AudioListenerInternalData
{
	i32 m_numControlTicks;
	double m_sampleRate;

	SoundSource* m_soundSources[MAX_SOUND_SOURCES];

	WriteWavFile m_wavOut2;
	bool m_writeWavOut;

	AudioListenerInternalData()
		: m_numControlTicks(64),
		  m_sampleRate(D3_SAMPLE_RATE),
		  m_writeWavOut(false)
	{
		for (i32 i = 0; i < MAX_SOUND_SOURCES; i++)
		{
			m_soundSources[i] = 0;
		}
	}
};

AudioListener::AudioListener()
{
	m_data = new AudioListenerInternalData();
	if (m_data->m_writeWavOut)
	{
		m_data->m_wavOut2.setWavFile("drx3DAudio2.wav", D3_SAMPLE_RATE, 2, false);
	}
}

AudioListener::~AudioListener()
{
	if (m_data->m_writeWavOut)
	{
		m_data->m_wavOut2.closeWavFile();
	}

	delete m_data;
}

i32 AudioListener::addSoundSource(SoundSource* source)
{
	i32 soundIndex = -1;

	for (i32 i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		if (m_data->m_soundSources[i] == 0)
		{
			m_data->m_soundSources[i] = source;
			soundIndex = i;
			break;
		}
	}
	return soundIndex;
}

void AudioListener::removeSoundSource(SoundSource* source)
{
	for (i32 i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		if (m_data->m_soundSources[i] == source)
		{
			m_data->m_soundSources[i] = 0;
		}
	}
}

AudioListenerInternalData* AudioListener::getTickData()
{
	return m_data;
}

const AudioListenerInternalData* AudioListener::getTickData() const
{
	return m_data;
}

double AudioListener::getSampleRate() const
{
	return m_data->m_sampleRate;
}

void AudioListener::setSampleRate(double sampleRate)
{
	m_data->m_sampleRate = sampleRate;
}

i32 AudioListener::tick(uk outputBuffer, uk inputBuffer1, u32 nBufferFrames,
						  double streamTime, u32 status, uk dataPointer)
{
	D3_PROFILE("AudioListener::tick");

	AudioListenerInternalData* data = (AudioListenerInternalData*)dataPointer;
	double outs[2], *samples = (double*)outputBuffer;
	double tempOuts[2];
	i32 counter, nTicks = (i32)nBufferFrames;
	bool done = false;

	i32 numSamples = 0;

	while (nTicks > 0 && !done)
	{
		counter = MyMin(nTicks, data->m_numControlTicks);
		bool newsynth = true;
		if (newsynth)
		{
			for (i32 i = 0; i < counter; i++)
			{
				outs[0] = 0.;
				outs[1] = 0.;
				//make_sound_double(outs,1);
				float numActiveSources = 0;

				for (i32 i = 0; i < MAX_SOUND_SOURCES; i++)
				{
					if (data->m_soundSources[i])
					{
						tempOuts[0] = 0;
						tempOuts[1] = 0;

						if (data->m_soundSources[i]->computeSamples(tempOuts, 1, data->m_sampleRate))
						{
							numActiveSources++;
							//simple mixer
							outs[0] += tempOuts[0];
							outs[1] += tempOuts[1];
						}
					}
				}

				//soft-clipping of sounds
				outs[0] = tanh(outs[0]);
				outs[1] = tanh(outs[1]);

				*samples++ = outs[0];
				*samples++ = outs[1];
				numSamples++;
			}
			nTicks -= counter;
		}
		if (nTicks == 0)
			break;
	}

	//logging to wav file
	if (data->m_writeWavOut && numSamples)
	{
		data->m_wavOut2.tick((double*)outputBuffer, numSamples);
	}
	return 0;
}
