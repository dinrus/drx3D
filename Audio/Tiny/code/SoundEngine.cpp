#include "../SoundEngine.h"
#include "../RtAudio.h"
#include "../AudioListener.h"
#include "../SoundSource.h"
#include <drx3D/Common/b3AlignedObjectArray.h>
#include "../ReadWavFile.h"
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3HashMap.h>

// The default real-time audio input and output buffer size.  If
// clicks are occuring in the input and/or output sound stream, a
// larger buffer size may help.  Larger buffer sizes, however, produce
// more latency.
//u32k RT_BUFFER_SIZE = 1024;
u32k RT_BUFFER_SIZE = 256;

struct b3SoundEngineInternalData
{
	AudioListener m_listener;
	RtAudio m_dac;
	bool m_useRealTimeDac;

	b3AlignedObjectArray<SoundSource*> m_soundSources;
	b3HashMap<b3HashInt, b3ReadWavFile*> m_wavFiles;
	b3HashMap<b3HashString, i32> m_name2wav;

	i32 m_wavFileUidGenerator;

	b3SoundEngineInternalData()
		: m_useRealTimeDac(false),
		  m_wavFileUidGenerator(123)
	{
	}
};

SoundEngine::SoundEngine()
{
	m_data = new b3SoundEngineInternalData();
}

SoundEngine::~SoundEngine()
{
	exit();
	delete m_data;
}

void SoundEngine::init(i32 maxNumSoundSources, bool useRealTimeDac)
{
	for (i32 i = 0; i < maxNumSoundSources; i++)
	{
		SoundSource* source = new SoundSource();
		m_data->m_soundSources.push_back(source);
		m_data->m_listener.addSoundSource(source);
	}

	this->m_data->m_useRealTimeDac = useRealTimeDac;

	if (useRealTimeDac)
	{
		RtAudioFormat format = (sizeof(double) == 8) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
		RtAudio::StreamParameters parameters;
		parameters.deviceId = m_data->m_dac.getDefaultOutputDevice();
		parameters.nChannels = 2;

		u32 bufferFrames = RT_BUFFER_SIZE;
		double sampleRate = m_data->m_listener.getSampleRate();

		m_data->m_dac.openStream(&parameters, NULL, format, (u32)sampleRate, &bufferFrames, &AudioListener::tick,
								 (uk )m_data->m_listener.getTickData());

		m_data->m_dac.startStream();
	}
}

void SoundEngine::exit()
{
	m_data->m_dac.closeStream();
	m_data->m_useRealTimeDac = false;

	for (i32 i = 0; i < m_data->m_soundSources.size(); i++)
	{
		m_data->m_listener.removeSoundSource(m_data->m_soundSources[i]);
		m_data->m_soundSources[i]->stopSound();
		delete m_data->m_soundSources[i];
	}
	m_data->m_soundSources.clear();

	for (i32 i = 0; i < m_data->m_wavFiles.size(); i++)
	{
		b3ReadWavFile** wavPtr = m_data->m_wavFiles.getAtIndex(i);
		if (wavPtr && *wavPtr)
		{
			b3ReadWavFile* wav = *wavPtr;
			delete wav;
		}
	}
	m_data->m_wavFiles.clear();
	m_data->m_name2wav.clear();
}

i32 SoundEngine::getAvailableSoundSource()
{
	for (i32 i = 0; i < m_data->m_soundSources.size(); i++)
	{
		if (m_data->m_soundSources[i]->isAvailable())
		{
			return i;
		}
	}
	return -1;
}

void SoundEngine::startSound(i32 soundSourceIndex, SoundMessage msg)
{
	SoundSource* soundSource = m_data->m_soundSources[soundSourceIndex];
	soundSource->setOscillatorAmplitude(0, msg.m_amplitude);
	soundSource->setOscillatorAmplitude(1, msg.m_amplitude);

	soundSource->setADSR(msg.m_attackRate, msg.m_decayRate, msg.m_sustainLevel, msg.m_releaseRate);

	switch (msg.m_type)
	{
		case D3_SOUND_SOURCE_SINE_OSCILLATOR:
		{
			soundSource->setOscillatorFrequency(0, msg.m_frequency);
			soundSource->setOscillatorFrequency(1, msg.m_frequency);

			soundSource->startSound(msg.m_autoKeyOff);
			break;
		}
		case D3_SOUND_SOURCE_WAV_FILE:
		{
			b3ReadWavFile** wavFilePtr = m_data->m_wavFiles[msg.m_wavId];
			if (wavFilePtr)
			{
				b3ReadWavFile* wavFile = *wavFilePtr;
				soundSource->setWavFile(0, wavFile, getSampleRate());
				soundSource->setWavFile(1, wavFile, getSampleRate());
				soundSource->startSound(msg.m_autoKeyOff);
			}
			break;
		}
		default:
		{
		}
	}
}

void SoundEngine::releaseSound(i32 soundSourceIndex)
{
	SoundSource* soundSource = m_data->m_soundSources[soundSourceIndex];
	soundSource->stopSound();
}

i32 SoundEngine::loadWavFile(tukk fileName)
{
	i32* wavUidPtr = m_data->m_name2wav[fileName];
	if (wavUidPtr)
	{
		return *wavUidPtr;
	}
	char resourcePath[1024];

	if (ResourcePath::findResourcePath(fileName, resourcePath, 1024, 0))
	{
		b3ReadWavFile* wavFile = new b3ReadWavFile();
		wavFile->getWavInfo(resourcePath);
		wavFile->resize();
		wavFile->read(0, true);
		wavFile->normalize(1);
		i32 wavUID = m_data->m_wavFileUidGenerator++;
		m_data->m_wavFiles.insert(wavUID, wavFile);
		m_data->m_name2wav.insert(fileName, wavUID);
		return wavUID;
	}
	return 0;
}

double SoundEngine::getSampleRate() const
{
	return m_data->m_listener.getSampleRate();
}
