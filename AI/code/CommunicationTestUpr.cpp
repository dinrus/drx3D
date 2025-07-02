// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/CommunicationTestUpr.h>
#include <drx3D/AI/CommunicationUpr.h>

CommunicationTestUpr::CommunicationTestUpr()
	: m_playGenID(0)
{
}

void CommunicationTestUpr::Reset()
{
	while (!m_playingActors.empty())
		Stop(m_playingActors.begin()->first);
}

void CommunicationTestUpr::StartFor(EntityId actorID, tukk commName, i32 variationNumber)
{
	IEntity* entity = gEnv->pEntitySystem->GetEntity(actorID);

	if (IAIObject* aiObject = entity->GetAI())
	{
		if (IAIActorProxy* proxy = aiObject->GetProxy())
		{
			DrxLogAlways("Playing communications test for '%s'...", entity ? entity->GetName() : "<null>");

			std::pair<PlayingActors::iterator, bool> iresult = m_playingActors.insert(
			  PlayingActors::value_type(actorID, PlayingActor()));

			if (!iresult.second)
			{
				// already playing
				return;
			}

			PlayingActor& playingActor = iresult.first->second;
			playingActor.configID = gAIEnv.pCommunicationUpr->GetConfigID(proxy->GetCommunicationConfigName());
			playingActor.onlyOne = commName && *commName;

			if (playingActor.onlyOne)
			{
				playingActor.commID = gAIEnv.pCommunicationUpr->GetCommunicationID(commName);
				playingActor.variation = variationNumber;
			}

			PlayNext(actorID);
		}
	}
}

void CommunicationTestUpr::Stop(EntityId actorID)
{
	PlayingActors::iterator it = m_playingActors.find(actorID);

	if (it != m_playingActors.end())
	{
		PlayingActor& playingActor = it->second;
		CommPlayID playID = playingActor.playID;
		m_playing.erase(playingActor.playID);
		m_playingActors.erase(it);

		m_player.Stop(playID);

		IEntity* entity = gEnv->pEntitySystem->GetEntity(actorID);
		DrxLogAlways("Cancelled communications test for '%s'...", entity ? entity->GetName() : "<null>");
	}
}

void CommunicationTestUpr::Update(float updateTime)
{
	m_player.Update(updateTime);
}

void CommunicationTestUpr::OnCommunicationFinished(const CommPlayID& playID, u8 stateFlags)
{
	PlayingCommunications::iterator it = m_playing.find(playID);
	if (it != m_playing.end())
	{
		EntityId actorID = it->second;
		m_playing.erase(playID);

		PlayingActors::iterator pit = m_playingActors.find(actorID);
		if (pit != m_playingActors.end())
		{
			PlayingActor& playingActor = pit->second;

			if (playingActor.onlyOne)
			{
				const CCommunicationUpr::CommunicationsConfig& config =
				  gAIEnv.pCommunicationUpr->GetConfig(playingActor.configID);

				Report(actorID, playingActor, config.name.c_str());
				m_playingActors.erase(pit);
			}
			else
				PlayNext(actorID);
		}
	}
}

void CommunicationTestUpr::PlayNext(EntityId actorID)
{
	PlayingActors::iterator pit = m_playingActors.find(actorID);
	if (pit != m_playingActors.end())
	{
		PlayingActor& playingActor = pit->second;

		const CCommunicationUpr::CommunicationsConfig& config =
		  gAIEnv.pCommunicationUpr->GetConfig(playingActor.configID);

		while (true)
		{
			if (!playingActor.commID && !config.comms.empty())
				playingActor.commID = config.comms.begin()->first;

			if (!playingActor.commID)
			{
				Report(actorID, playingActor, config.name.c_str());
				m_playingActors.erase(pit);

				return;
			}

			CCommunicationUpr::Communications::const_iterator commIt = config.comms.find(playingActor.commID);
			if (commIt == config.comms.end())
			{
				++playingActor.failedCount;
				Report(actorID, playingActor, config.name.c_str());
				m_playingActors.erase(pit);

				return;
			}

			const SCommunication& prevComm = commIt->second;
			u32 prevVarCount = prevComm.variations.size();

			if (playingActor.variation >= prevVarCount)
			{
				++commIt;

				if (commIt == config.comms.end())
				{
					Report(actorID, playingActor, config.name.c_str());
					m_playingActors.erase(pit);

					return;
				}

				playingActor.variation = 0;
				playingActor.commID = commIt->first;
			}

			SCommunicationRequest request;
			request.actorID = actorID;
			request.commID = playingActor.commID;
			request.configID = playingActor.configID;

			const SCommunication& comm = commIt->second;
			++playingActor.totalCount;

			SCommunication testComm(comm);
			SCommunicationVariation& variation = testComm.variations[playingActor.variation];
			variation.flags |= (SCommunication::FinishVoice | SCommunication::FinishSound);

			++m_playGenID.id;
			if (m_player.Play(m_playGenID, request, testComm, playingActor.variation++, this, 0))
			{
				DrxLogAlways("Now playing communication test '%s' variation %u from '%s'...",
				             comm.name.c_str(), playingActor.variation - 1, config.name.c_str());
				m_playing.insert(PlayingCommunications::value_type(m_playGenID, actorID));
				playingActor.playID = m_playGenID;

				break;
			}
			else
			{
				DrxLogAlways("Failed to play communication test '%s' variation '%u' from '%s'...",
				             comm.name.c_str(), playingActor.variation - 1, config.name.c_str());

				++playingActor.failedCount;

				if (playingActor.onlyOne)
				{
					Report(actorID, playingActor, config.name.c_str());
					m_playingActors.erase(pit);

					return;
				}
			}
		}
	}
}

void CommunicationTestUpr::Report(EntityId actorID, const PlayingActor& playingActor, tukk configName)
{
	IEntity* entity = gEnv->pEntitySystem->GetEntity(actorID);

	DrxLogAlways("Finished communication test for '%s' using '%s'...", entity ? entity->GetName() : "<null>", configName);
	DrxLogAlways("Attempted: %u", playingActor.totalCount);
	DrxLogAlways("Failed: %u", playingActor.failedCount);
}
