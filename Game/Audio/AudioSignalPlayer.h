// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef AUDIO_SIGNAL_PLAYER
#define AUDIO_SIGNAL_PLAYER

#include <drx3D/Game/Audio/GameAudio.h>


class CAudioSignalPlayer
{
public:
	CAudioSignalPlayer()
	: m_audioSignalId( INVALID_AUDIOSIGNAL_ID )
	{}

	friend class CGameAudioUtils;
	
	// TODO: only Play should need the entityId param.

	void SetSignal( tukk pName );
	void SetSignalSafe( tukk pName );
	void SetSignal( TAudioSignalID signalID );
	//void Play( EntityId entityID = 0, tukk pParam = NULL, float param = 0, ESoundSemantic semanticOverride = eSoundSemantic_None, i32* const pSpecificRandomSoundIndex = NULL );
	void SetPaused( EntityId entityID, const bool paused);
	//void Stop( EntityId entityID = 0, const ESoundStopMode stopMode = ESoundStopMode_EventFade );
	bool IsPlaying( EntityId entityID = 0 ) const;
	void SetVolume( EntityId entityID, float vol );
	void SetParam( EntityId entityID, tukk paramName, float paramValue );
	bool HasValidSignal() const { return m_audioSignalId!=INVALID_AUDIOSIGNAL_ID; }
	void InvalidateSignal() { SetSignal( INVALID_AUDIOSIGNAL_ID ); }
	void SetOffsetPos( EntityId entityID, const Vec3& pos );
	
	static void JustPlay( TAudioSignalID signalID, EntityId entityID = 0, tukk pParam = NULL, float param = 0);
	static void JustPlay( tukk signal, EntityId entityID = 0, tukk pParam = NULL, float param = 0);
	static void JustPlay( TAudioSignalID signalID, const Vec3& pos );
	static void JustPlay( tukk signal, const Vec3& pos );

	void SetCurrentSamplePos( EntityId entityID, float relativePosition );
	float GetCurrentSamplePos( EntityId entityID );

	static float GetSignalLength(TAudioSignalID signalID);
	#if !defined(_RELEASE) || defined(RELEASE_LOGGING)
	static tukk GetSignalName(TAudioSignalID signalId); // for debug purposes
	tukk GetSignalName();  // for debug purposes
	#endif

	TAudioSignalID GetSignalID() const { return m_audioSignalId; };

	void Reset();

	void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		//pSizer->AddContainer(m_playingSoundIDs);
	}
private:	
	//static tSoundID PlaySound( const string& name, const ESoundSemantic semantic, EntityId entityID, tukk pParam, float param, u32 flags = FLAG_SOUND_EVENT );
	//static tSoundID PlaySound( const string& name, const ESoundSemantic semantic, const Vec3& pos , u32 flags = FLAG_SOUND_EVENT );
	//void StopSound( const tSoundID soundID, EntityId entityID, const ESoundStopMode stopMode = ESoundStopMode_EventFade );
	//bool IsSoundLooped( const tSoundID soundID, EntityId entityID );
	//ISound* GetSoundInterface( IEntityAudioProxy* pProxy, tSoundID soundID ) const;	
	static void ExecuteCommands( EntityId entityID, const CGameAudio::CAudioSignal* pAudioSignal );
	
private:
	TAudioSignalID m_audioSignalId;
	//std::vector<tSoundID> m_playingSoundIDs;
};

#endif
