// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MOCK_SOUND_SYSTEM_WRAPPER_H
#define MOCK_SOUND_SYSTEM_WRAPPER_H

#include <drx3D/Game/EngineFacade/DrxEngineWrappers/SoundSystemWrapper.h>
#include <drx3D/Game/Actor2\\ComponentEntityID.h>

namespace GameTesting
{
	class CMockSoundSystemWrapper : public EngineFacade::ISoundSystemWrapper
	{
	public:
		CMockSoundSystemWrapper();

		virtual tSoundID Play(const string& soundName, const ESoundSemantic semantic, const ComponentEntityID& entityID);
		virtual void Stop(const tSoundID soundID, const ComponentEntityID& entityID);

		virtual void AddSoundMood(const string& name, u32k fadeInTimeMSecs);
		virtual void RemoveSoundMood(const string& name, u32k fadeOutTimeMSecs);

		void AddReturnedSoundIDAfterPlay(const tSoundID soundID);

		bool WasAddSoundMoodCalledOnceWith(const string& name, u32k fadeInTimeMSecs) const;

		bool WasRemoveSoundMoodCalledOnceWith(const string& name, u32k fadeOutTimeMSecs) const;

		bool WasAddSoundMoodCalled() const;

		bool WasRemoveSoundMoodCalled() const;

		bool WasSoundPlayedOnceWith(const string& soundName) const;
		
		bool WasSoundStoppedOnceWith(const tSoundID soundID) const;
		bool WasSoundStoppedOnceWith(const tSoundID soundID, const ComponentEntityID& entityID) const;

		bool WasSoundPlayedOnceWithSpecificSemantic( const string& soundName, const ESoundSemantic semantic ) const;
		bool WasSoundPlayedOnceWithSpecificEntityID( const string& soundName, const ComponentEntityID& entityID ) const;

	private:
		struct SPlayParameters
		{
			string soundName;
			ESoundSemantic semantic;
			ComponentEntityID entityID;
		};

		struct SStopParameters
		{
			tSoundID soundID;
			ComponentEntityID entityID;
		};

		struct SAddRemoveMoodParameters
		{
			SAddRemoveMoodParameters( const string& _moodName, u32k _fadeTime )
				: moodName( _moodName )
				, fadeTime( _fadeTime )
			{}

			bool operator == ( const SAddRemoveMoodParameters& other ) const { return moodName==other.moodName && fadeTime==other.fadeTime; }

			string moodName;
			u32 fadeTime;
		};

		std::vector<tSoundID> m_soundIDsToReturnOnNextPlay;

		std::vector<SPlayParameters> m_playCalls;
		std::vector<SStopParameters> m_stopCalls;

		std::vector<SAddRemoveMoodParameters> m_addSoundMoodCalls;
		std::vector<SAddRemoveMoodParameters> m_removeSoundMoodCalls;
	};
}

#endif



