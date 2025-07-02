#include "../SoundSource.h"

#define MY2PI (2. * 3.14159265)
#include <math.h>
#include <drx3D/Common/b3FileUtils.h>
#include "../ReadWavFile.h"
#include "../ADSR.h"
#include "../Sound_C_Api.h"

struct SoundOscillator
{
	i32 m_type;
	double m_frequency;
	double m_amplitude;
	double m_phase;

	b3WavTicker m_wavTicker;

	double sampleSineWaveForm(double sampleRate)
	{
		while (m_phase >= MY2PI)
			m_phase -= MY2PI;

		double z = sinf(m_phase);
		double sample = m_amplitude * z;

		m_phase += MY2PI * (1. / sampleRate) * m_frequency;
		return sample;
	}

	double sampleSawWaveForm(double sampleRate)
	{
		while (m_phase >= MY2PI)
			m_phase -= MY2PI;

		double z = 2. * (m_phase) / MY2PI - 1.;
		double sample = m_amplitude * z;

		m_phase += MY2PI * (1. / sampleRate) * m_frequency;
		return sample;
	}

	void reset()
	{
		m_phase = 0;
	}

	SoundOscillator()
		: m_type(0),
		  m_frequency(442.),
		  m_amplitude(1),
		  m_phase(0)
	{
	}
};
#define MAX_OSCILLATORS 2

struct SoundSourceInternalData
{
	SoundOscillator m_oscillators[MAX_OSCILLATORS];
	b3ADSR m_envelope;
	b3ReadWavFile* m_wavFilePtr;
	SoundSourceInternalData()
		: m_wavFilePtr(0)
	{
	}
};

SoundSource::SoundSource()
{
	m_data = new SoundSourceInternalData();
}

SoundSource::~SoundSource()
{
	delete m_data;
}

void SoundSource::setADSR(double attack, double decay, double sustain, double release)
{
	m_data->m_envelope.setValues(attack, decay, sustain, release);
}

bool SoundSource::computeSamples(double* sampleBuffer, i32 numSamples, double sampleRate)
{
	double* outputSamples = sampleBuffer;
	i32 numActive = 0;

	for (i32 i = 0; i < numSamples; i++)
	{
		double samples[MAX_OSCILLATORS] = {0};

		double env = m_data->m_envelope.tick();
		if (env)
		{
			for (i32 osc = 0; osc < MAX_OSCILLATORS; osc++)
			{
				if (m_data->m_oscillators[osc].m_type == 0)
				{
					samples[osc] += env * m_data->m_oscillators[osc].sampleSineWaveForm(sampleRate);
					numActive++;
				}

				if (m_data->m_oscillators[osc].m_type == 1)
				{
					samples[osc] += env * m_data->m_oscillators[osc].sampleSawWaveForm(sampleRate);
					numActive++;
				}

				if (m_data->m_oscillators[osc].m_type == 128)
				{
					i32 frame = 0;
					double data = env * m_data->m_oscillators[osc].m_amplitude * m_data->m_wavFilePtr->tick(frame, &m_data->m_oscillators[osc].m_wavTicker);
					samples[osc] += data;
					numActive++;
				}
			}
		}
		else
		{
			for (i32 osc = 0; osc < MAX_OSCILLATORS; osc++)
			{
				if (m_data->m_oscillators[osc].m_type == 128)
				{
					m_data->m_oscillators[osc].m_wavTicker.finished_ = true;
				}
			}
		}
		//sample *= 1./double(MAX_OSCILLATORS);

		double sampleLeft = samples[0];
		double sampleRight = samples[1];
		if (sampleLeft != sampleRight)
		{
		}

		*outputSamples++ = sampleRight;
		*outputSamples++ = sampleLeft;
	}

	/*	if (m_data->m_flags & looping)
	{
		for (i32 osc=0;osc<MAX_OSCILLATORS;osc++)
		{
			if (m_data->m_oscillators[osc].m_waveIn.isFinished())
				m_data->m_oscillators[osc].m_waveIn.reset();
		}
	}
	*/
	return numActive > 0;
	//	return false;
}

i32 SoundSource::getNumOscillators() const
{
	return MAX_OSCILLATORS;
}
void SoundSource::setOscillatorType(i32 oscillatorIndex, i32 type)
{
	m_data->m_oscillators[oscillatorIndex].m_type = type;
}
void SoundSource::setOscillatorFrequency(i32 oscillatorIndex, double frequency)
{
	m_data->m_oscillators[oscillatorIndex].m_frequency = frequency;
}
void SoundSource::setOscillatorAmplitude(i32 oscillatorIndex, double amplitude)
{
	m_data->m_oscillators[oscillatorIndex].m_amplitude = amplitude;
}
void SoundSource::setOscillatorPhase(i32 oscillatorIndex, double phase)
{
	m_data->m_oscillators[oscillatorIndex].m_phase = phase;
}

bool SoundSource::isAvailable() const
{
	//available if ADSR is idle and wavticker is finished
	return m_data->m_envelope.isIdle();
}

void SoundSource::startSound(bool autoKeyOff)
{
	if (m_data->m_envelope.isIdle())
	{
		for (i32 osc = 0; osc < MAX_OSCILLATORS; osc++)
		{
			m_data->m_oscillators[osc].reset();

			if (m_data->m_oscillators[osc].m_type == D3_SOUND_SOURCE_WAV_FILE)  //				.m_wavTicker.finished_)
			{
				//test reverse playback of wav
				//m_data->m_oscillators[osc].m_wavTicker.rate_ *= -1;
				if (m_data->m_oscillators[osc].m_wavTicker.rate_ < 0)
				{
					m_data->m_oscillators[osc].m_wavTicker.time_ = m_data->m_wavFilePtr->getNumFrames() - 1.;
				}
				else
				{
					m_data->m_oscillators[osc].m_wavTicker.time_ = 0.f;
				}

				m_data->m_oscillators[osc].m_wavTicker.finished_ = false;
			}
		}
	}
	m_data->m_envelope.keyOn(autoKeyOff);
}

void SoundSource::stopSound()
{
	m_data->m_envelope.keyOff();
}

bool SoundSource::setWavFile(i32 oscillatorIndex, b3ReadWavFile* wavFilePtr, i32 sampleRate)
{
	{
		m_data->m_wavFilePtr = wavFilePtr;
		m_data->m_oscillators[oscillatorIndex].m_wavTicker = m_data->m_wavFilePtr->createWavTicker(sampleRate);

		//		waveIn.openFile(resourcePath);
		double rate = 1.0;
		//	rate = waveIn.getFileRate() / stkSampleRate;
		//	waveIn.setRate( rate );
		//	waveIn.ignoreSampleRateChange();
		// Find out how many channels we have.
		//	i32 channels = waveIn.channelsOut();
		//	m_data->m_oscillators[oscillatorIndex].m_frames.resize( 1, channels );
		m_data->m_oscillators[oscillatorIndex].m_type = 128;
		return true;
	}
	return false;
}