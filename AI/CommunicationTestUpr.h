// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CommunicationTestUpr_h__
#define __CommunicationTestUpr_h__

#pragma once

#include <drx3D/AI/CommunicationPlayer.h>

class CommunicationTestUpr
	: public CommunicationPlayer::ICommunicationFinishedListener
{
public:
	CommunicationTestUpr();

	void Reset();

	void StartFor(EntityId actorID, tukk commName, i32 variationNumber = 0);
	void Stop(EntityId actorID);
	void Update(float updateTime);

private:
	// ICommunicationFinishedListener
	/// playID is the id of the communication to stop.
	/// StateFlags contains  whether the components of the communication are finished or not (animation, sound, etc).
	virtual void OnCommunicationFinished(const CommPlayID& playID, u8 stateFlags);
	//~ICommunicationFinishedListener

	struct PlayingActor
	{
		PlayingActor()
			: configID(0)
			, commID(0)
			, playID(0)
			, variation(0)
			, failedCount(0)
			, totalCount(0)
			, onlyOne(false)
		{
		}

		CommConfigID configID;

		CommID       commID;
		CommPlayID   playID;
		u32       variation;

		u32       failedCount;
		u32       totalCount;
		bool         onlyOne;
	};

	void PlayNext(EntityId actorID);
	void Report(EntityId actorID, const PlayingActor& playingActor, tukk configName);

	CommunicationPlayer m_player;

	typedef std::map<tAIObjectID, PlayingActor> PlayingActors;
	PlayingActors m_playingActors;

	typedef std::map<CommPlayID, tAIObjectID> PlayingCommunications;
	PlayingCommunications m_playing;

	CommPlayID            m_playGenID;
};

#endif //__CommunicationTestUpr_h__
