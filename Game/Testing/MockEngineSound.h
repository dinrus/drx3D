// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 01:06:2009   Created by Federico Rebora
*************************************************************************/

#pragma once

#ifndef MOCK_ENGINE_SOUND_H_INCLUDED
#define MOCK_ENGINE_SOUND_H_INCLUDED

#include <drx3D/Game/EngineFacade/EngineSound.h>
#include <drx3D/Game/EngineFacade/AudioSignalPlayer.h>
#include <drx3D/Game/MockSoundSystemWrapper.h>


namespace EngineFacade
{
	struct ISoundSystemWrapper;
}

namespace GameTesting
{
	class CMockEngineSound : public EngineFacade::CDummyEngineSound
	{
	public:
		CMockEngineSound();

		virtual ~CMockEngineSound();

		virtual const EngineFacade::CAudioSignal* GetAudioSignal( const string& name );
		virtual void Reset();
		virtual i32 GetAmountAudioSignalsPlayed () const;
		virtual bool HasAnyAudioSignalBeenPlayed () const;
		virtual bool HasAudioSignalBeenPlayed( const string& audioSignalName ) const;
		virtual i32 GetAmountAudioSignalsStopped () const;
		virtual bool HasAnyAudioSignalBeenStopped () const;
		virtual bool HasAudioSignalBeenStopped( const string& audioSignalName ) const;
		virtual string GetLastPlayedSignalName() const;
		virtual const ComponentEntityID& GetLastPlayedSignalEntityID() const;
		virtual EngineFacade::ISoundSystemWrapper* GetWrapper();
		
	private:
	
		void NotifySoundPlayed( const string& soundName, const ComponentEntityID& entityID );
		void NotifySoundStopped( const string& soundName );
		
	private:
	
		class CLocalMockSoundSystemWrapper : public CMockSoundSystemWrapper
		{
		public:
			void SetMockEngineSound( CMockEngineSound* mockEngineSound );
			tSoundID Play(const string& soundName, const ESoundSemantic semantic, const ComponentEntityID& entityID);
			void Stop(const tSoundID soundID, const ComponentEntityID& entityID);
		
		private:
			CMockEngineSound* m_mockEngineSound;
			std::vector<string> m_playedSounds;
		};
		
		friend class CLocalMockSoundSystemWrapper;

	private:
		u32 m_DummyIDCounter;

		EntityId          m_CreateAndPlay_EntityId_Param;
		string            m_CreateAndPlay_SoundName_Param;
		ESoundSemantic    m_CreateAndPlay_Semantic_Param;
		tSoundID          m_Stop_SoundID_Param;
		EntityId          m_Stop_EntityId_Param;
		bool              m_CreateAndPlay_WasCalled;
		bool              m_Stop_WasCalled;
		string            m_UpdateSoundMood_MoodName_Param;
		float             m_UpdateSoundMood_FadeTarget_Param;
		
		struct SPlayedSignal
		{
			i32 m_IndSignalInObtainedVector;
			ComponentEntityID m_entityID;
		};
		
		std::vector<EngineFacade::CAudioSignal*> m_obtainedSignals;
		std::vector<SPlayedSignal> m_playedSignals;
		std::vector<i32> m_stoppedSignals;  // index into m_obtainedSignals
		CLocalMockSoundSystemWrapper m_mockSoundSystemWrapper;
	};
}

#endif
