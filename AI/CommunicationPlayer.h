// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CommunicationPlayer_h__
#define __CommunicationPlayer_h__

#pragma once

#include <drx3D/AI/Communication.h>

class CommunicationPlayer : public IAICommunicationHandler::IEventListener
{
public:
	struct ICommunicationFinishedListener
	{
		/// playID is the id of the communication to stop.
		/// StateFlags contains  whether the components of the communication are finished or not (animation, sound, etc).
		virtual void OnCommunicationFinished(const CommPlayID& playID, u8 stateFlags) = 0;
		virtual ~ICommunicationFinishedListener(){}
	};

public:
	void         Clear();
	void         Reset();

	bool         Play(const CommPlayID& playID, const SCommunicationRequest& request,
	                  const SCommunication& comm, u32 variationIdx, ICommunicationFinishedListener* listener, uk param);
	void         Stop(const CommPlayID& playID);
	void         Update(float updateTime);

	bool         IsPlaying(const CommPlayID& playID, float* remainingTime = 0) const;

	virtual void OnCommunicationHandlerEvent(
	  IAICommunicationHandler::ECommunicationHandlerEvent type, CommPlayID playID, EntityId actorID);

	CommID GetCommunicationID(const CommPlayID& playID) const;

public:
	struct PlayState
	{
		enum EFinishedFlags
		{
			FinishedAnimation = 1 << 0,
			FinishedSound     = 1 << 1,
			FinishedVoice     = 1 << 2,
			FinishedTimeout   = 1 << 3,
			FinishedAll       = FinishedAnimation | FinishedSound | FinishedVoice | FinishedTimeout,
		};

		PlayState(const SCommunicationRequest& request, const SCommunicationVariation& variation,
		          ICommunicationFinishedListener* _listener = 0, uk _param = 0)
			: actorID(request.actorID)
			, commID(request.commID)
			, timeout(variation.timeout)
			, target(request.target)
			, targetID(request.targetID)
			, flags(variation.flags)
			, finishedFlags(0)
			, listener(_listener)
			, param(_param)
		{
		}
		~PlayState(){}

		void OnCommunicationEvent(
		  IAICommunicationHandler::ECommunicationHandlerEvent type, EntityId actorID);

		Vec3                            target;

		string                          animationName;

		ICommunicationFinishedListener* listener;
		uk                           param;

		EntityId                        targetID;
		EntityId                        actorID;

		SCommunicationSound             soundInfo;
		SCommunicationSound             voiceInfo;

		CommID                          commID;

		float                           timeout;
		u32                          flags;

		//Tracks whether this playstate has finished.
		u8 finishedFlags;
	};

private:

	void CleanUpPlayState(const CommPlayID& playID, PlayState& playState);

	bool UpdatePlaying(const CommPlayID& playId, PlayState& playing, float updateTime);
	void BlockingSettings(IAIObject* aiObject, PlayState& playing, bool set);

	typedef std::map<CommPlayID, PlayState> PlayingCommunications;
	PlayingCommunications m_playing;
};

#endif
