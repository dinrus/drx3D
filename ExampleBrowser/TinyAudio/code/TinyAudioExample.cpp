#include "../TinyAudioExample.h"
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3HashMap.h>

#include <drx3D/Audio/Tiny/SoundEngine.h>
#include <drx3D/Audio/Tiny/SoundSource.h>
#include <string>

///very basic hashable string implementation, compatible with b3HashMap
struct MyHashString
{
	STxt m_string;
	u32 m_hash;

	D3_FORCE_INLINE u32 getHash() const
	{
		return m_hash;
	}

	MyHashString(tukk name)
		: m_string(name)
	{
		/* magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/ */
		static u32k InitialFNV = 2166136261u;
		static u32k FNVMultiple = 16777619u;

		/* Fowler / Noll / Vo (FNV) Hash */
		u32 hash = InitialFNV;

		for (i32 i = 0; m_string[i]; i++)
		{
			hash = hash ^ (m_string[i]); /* xor  the low 8 bits */
			hash = hash * FNVMultiple;   /* multiply by the magic number */
		}
		m_hash = hash;
	}

	bool equals(const MyHashString& other) const
	{
		return (m_string == other.m_string);
	}
};

double base_frequency = 440.0;
double base_pitch = 69.0;

double MidiPitch2Frequency(double incoming_note)
{
	return base_frequency * pow(2.0, (incoming_note - base_pitch) / 12.0);
}

double FrequencytoMidiPitch(double incoming_frequency)
{
	return base_pitch + (12.0 * log(incoming_frequency / base_frequency) / log(2));
}

class TinyAudioExample : public CommonExampleInterface
{
	GUIHelperInterface* m_guiHelper;

	SoundEngine m_soundEngine;
	i32 m_wavId;

	b3HashMap<MyHashString, i32> m_keyToSoundSource;

public:
	TinyAudioExample(struct GUIHelperInterface* helper)
		: m_guiHelper(helper)
	{
	}

	virtual ~TinyAudioExample()
	{
	}

	virtual void initPhysics()
	{
		i32 numSoundSources = 32;
		bool useRealTimeDac = true;

		m_soundEngine.init(numSoundSources, useRealTimeDac);

		m_wavId = m_soundEngine.loadWavFile("wav/xylophone.rosewood.ff.C5B5_1.wav");
		i32 sampleRate = m_soundEngine.getSampleRate();
	}

	virtual void exitPhysics()
	{
		m_soundEngine.exit();
	}

	virtual void renderScene()
	{
	}

	virtual void stepSimulation(float deltaTime)
	{
	}

	virtual void physicsDebugDraw(i32 debugFlags)
	{
	}
	virtual bool mouseMoveCallback(float x, float y)
	{
		return false;
	}
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return false;
	}

	virtual bool keyboardCallback(i32 key, i32 state)
	{
		if (key >= 'a' && key <= 'z')
		{
			char keyStr[2];
			keyStr[0] = (char)key;
			keyStr[1] = 0;
			MyHashString hs(keyStr);

			if (state)
			{
				i32 soundSourceIndex = m_soundEngine.getAvailableSoundSource();
				if (soundSourceIndex >= 0)
				{
					i32 note = key - (97 - 58);
					double freq = MidiPitch2Frequency(note);

					SoundMessage msg;
					msg.m_type = D3_SOUND_SOURCE_SINE_OSCILLATOR;
					msg.m_frequency = freq;
					msg.m_amplitude = 1;

					msg.m_type = D3_SOUND_SOURCE_WAV_FILE;
					msg.m_wavId = m_wavId;
					msg.m_attackRate = 1;
					msg.m_sustainLevel = 1;
					msg.m_releaseRate = 0.001;

					m_soundEngine.startSound(soundSourceIndex, msg);
					m_keyToSoundSource.insert(hs, soundSourceIndex);
					//printf("soundSourceIndex:%d\n", soundSourceIndex);

#if 0
					SoundSource* soundSource = this->m_soundSourcesPool[soundSourceIndex];

					soundSource->setOscillatorFrequency(0, freq );
					soundSource->setOscillatorFrequency(1, freq );
					soundSource->startSound();
					
					{
						i32* soundSourceIndexPtr = m_keyToSoundSource[hs];
						if (soundSourceIndexPtr)
						{
							i32 newIndex = *soundSourceIndexPtr;
							printf("just inserted: %d\n", newIndex);
						}
					}
#endif
				}
			}
			else
			{
				i32* soundSourceIndexPtr = m_keyToSoundSource[hs];
				if (soundSourceIndexPtr)
				{
					i32 soundSourceIndex = *soundSourceIndexPtr;
					//printf("releaseSound: %d\n", soundSourceIndex);
					m_soundEngine.releaseSound(soundSourceIndex);
				}
#if 0
					if (soundSourceIndex>=0)
					{
						printf("releasing %d\n", soundSourceIndex);
						SoundSource* soundSource = this->m_soundSourcesPool[soundSourceIndex];
						soundSource->stopSound();
					}
				}
#endif
			}
		}

		return false;
	}

	void resetCamera()
	{
		float dist = 4;
		float pitch = 52;
		float yaw = 35;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, pitch, yaw, targetPos[0], targetPos[1], targetPos[2]);
	}
};

CommonExampleInterface* TinyAudioExampleCreateFunc(CommonExampleOptions& options)
{
	return new TinyAudioExample(options.m_guiHelper);
}

D3_STANDALONE_EXAMPLE(TinyAudioExampleCreateFunc)
