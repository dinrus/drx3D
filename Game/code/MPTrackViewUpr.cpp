// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <StdAfx.h>
#include <drx3D/Game/MPTrackViewUpr.h>
#include <drx3D/Game/Utility/DrxHash.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/GameRules.h>

static const float s_TrackViewMinTimeDifferenceForSynch = 0.25f;

CMPTrackViewUpr::CMPTrackViewUpr() : m_FinishedTrackViewCount(0), m_movieListener(false)
{
	std::memset(m_FinishedTrackViews, 0, sizeof (m_FinishedTrackViews));
	std::memset(m_FinishedTrackViewTimes, 0, sizeof(m_FinishedTrackViewTimes));
}

CMPTrackViewUpr::~CMPTrackViewUpr()
{
	if (m_movieListener == true)
	{
		IMovieSystem *pMovieSystem = gEnv->pMovieSystem;
		if(pMovieSystem)
		{
				pMovieSystem->RemoveMovieListener(NULL, this);
		}

		m_FinishedTrackViewCount = 0;
	}
}

void CMPTrackViewUpr::Init()
{
	m_movieListener = false;
	if(gEnv->bServer)
	{
		IMovieSystem *pMovieSystem = gEnv->pMovieSystem;
		if(pMovieSystem)
		{
			pMovieSystem->AddMovieListener(NULL, this);
			m_movieListener = true;
		}
	}
}

void CMPTrackViewUpr::Update()
{
#ifndef _RELEASE
	if (g_pGameCVars->g_mptrackview_debug)
	{
		IMovieSystem *pMovieSystem = gEnv->pMovieSystem;

		i32 numSequences=pMovieSystem->GetNumSequences();

		DrxWatch("num finished trackviews=%d", m_FinishedTrackViewCount);
		for (i32 i=0; i<m_FinishedTrackViewCount; i++)
		{
			tukk foundName="NOT FOUND";

			// SLOW
			IAnimSequence *foundSequence = FindTrackviewSequence(m_FinishedTrackViews[i]);
			if (foundSequence)
			{
				foundName = foundSequence->GetName();
			}

			DrxWatch("finished[%d] hash=%x; time=%f; foundName=%s", i, m_FinishedTrackViews[i], m_FinishedTrackViewTimes[i], foundName);
		}

		i32 numPlaying=pMovieSystem->GetNumPlayingSequences();
		for(i32 i = 0; i < numPlaying; ++i)
		{
			IAnimSequence* pSequence = pMovieSystem->GetPlayingSequence(i);

			if( pSequence )
			{
				tukk name=pSequence->GetName();
				if (!name)
				{
					name = "[NULL]";
				}
				DrxHashStringId hash_id(pSequence->GetName());
				float timeValue = pMovieSystem->GetPlayingTime(pSequence).ToFloat();

				DrxWatch("Seq[%d]: name=%s; time=%f; hash=%x", i, name, timeValue, hash_id.id);
			}
		}
	}
#endif
}

void CMPTrackViewUpr::Server_SynchAnimationTimes(CGameRules::STrackViewParameters& params)
{
	IMovieSystem *pMovieSystem = gEnv->pMovieSystem;
	
	i32 count = 0;

	IAnimSequence* pSequence = NULL; 

	DrxLog("CMPTrackViewUpr::Server_SynchAnimationTimes()");

	for(i32 i = 0; i < CGameRules::STrackViewParameters::sMaxTrackViews; ++i)
	{
		if(i < pMovieSystem->GetNumPlayingSequences())
			pSequence = pMovieSystem->GetPlayingSequence(i);
		else
			pSequence = NULL;

		if( pSequence )
		{
			if (pSequence->GetFlags() & IAnimSequence::eSeqFlags_NoMPSyncingNeeded)
			{
				DrxLog("CMPTrackViewUpr::Server_SynchAnimationTimes() skipping syncing of playing anim sequence %s as it has NO_MP_SYNCING_NEEDED flag set", pSequence->GetName());
				continue;
			}

			DrxHashStringId id(pSequence->GetName());
			params.m_Ids[i] = id.id;

			float timeValue = pMovieSystem->GetPlayingTime(pSequence).ToFloat();
			params.m_Times[i] = timeValue;

			DrxLog("CMPTrackViewUpr::Server_SynchAnimationTimes() adding playing sequence %d %s at time %f", i, pSequence->GetName(), timeValue);

			++count;
		}
		else
		{
#ifndef _RELEASE
			if(count + m_FinishedTrackViewCount > CGameRules::STrackViewParameters::sMaxTrackViews)
			{
				DrxWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, "Trying to synch %i animations but system set to %i in GameRules.h>,
					count + m_FinishedTrackViewCount, CGameRules::STrackViewParameters::sMaxTrackViews);
			}
#endif

			params.m_NumberOfFinishedTrackViews = m_FinishedTrackViewCount;
			for(i32 j = 0; j < m_FinishedTrackViewCount && i < CGameRules::STrackViewParameters::sMaxTrackViews; ++j, ++i)
			{
				params.m_Ids[i] = m_FinishedTrackViews[j];
				params.m_Times[i] = m_FinishedTrackViewTimes[j];
			
#ifndef _RELEASE
				IAnimSequence *pTrackviewSequence = FindTrackviewSequence(m_FinishedTrackViews[j]);
				DrxLog("CMPTrackViewUpr::Server_SynchAnimationTimes() adding finished sequence %d %s at time %f", i, pTrackviewSequence ? pTrackviewSequence->GetName() : "NULL", m_FinishedTrackViewTimes[j]);
#endif
			}

			break;
		}
	}

	params.m_NumberOfTrackViews = count;
	params.m_bInitialData = true;
}

void CMPTrackViewUpr::Client_SynchAnimationTimes(const CGameRules::STrackViewParameters& params)
{
	IMovieSystem *pMovieSystem = gEnv->pMovieSystem;

	i32 numTrackViews = params.m_NumberOfTrackViews + params.m_NumberOfFinishedTrackViews;
	DrxLog("CMPTrackViewUpr::Client_SynchAnimationTimes() numTrackViews=%d; numFinishedTrackViews=%d", params.m_NumberOfTrackViews, params.m_NumberOfFinishedTrackViews);

	for(i32 i = 0; i < numTrackViews; ++i)
	{
		i32 trackviewHashId = params.m_Ids[i];

		if(trackviewHashId == 0)
			continue;

		for(i32 k = 0; k < pMovieSystem->GetNumSequences(); ++k)
		{
			IAnimSequence* pSequence = pMovieSystem->GetSequence(k); 
			DrxHashStringId id(pSequence->GetName());
			if(id == trackviewHashId)
			{
				float timeValue = params.m_Times[i]; // even finished sequences include their time now, to handle aborted sequences
				float currentTimeValue = pMovieSystem->GetPlayingTime(pSequence).ToFloat();
				
				DrxLog("CMPTrackViewUpr::Client_SynchAnimationTimes() Sequence %d %s at time %f", i, pSequence->GetName(), timeValue);
				if(pMovieSystem->IsPlaying(pSequence))
				{
					// always update finished anims
					if( i>=params.m_NumberOfTrackViews || fabs(currentTimeValue - timeValue) >  s_TrackViewMinTimeDifferenceForSynch) //If we are close enough don't update - avoid unnecessary jerking
					{
						pMovieSystem->SetPlayingTime(pSequence, SAnimTime(timeValue));
					}
				}
				else
				{
					pMovieSystem->PlaySequence(pSequence, NULL, true, false, SAnimTime(timeValue));

					if(params.m_bInitialData && i>=params.m_NumberOfTrackViews && m_FinishedTrackViewCount < CGameRules::STrackViewParameters::sMaxTrackViews)
					{
						m_FinishedTrackViews[m_FinishedTrackViewCount++] = id.id;
					}
				}

				if (i>=params.m_NumberOfTrackViews)
				{
					// this is a finished trackview, so having setup the correct playtime above, lets actually stop it like it should be
					DrxLog("CMPTrackViewUpr::Client_SynchAnimationTimes() Sequence %d %s is a finished trackview, stopping it", i, pSequence->GetName());
					pMovieSystem->StopSequence(pSequence);
				}
			}
		}
	}
}

void CMPTrackViewUpr::AnimationRequested(const CGameRules::STrackViewRequestParameters& params)
{
	i32 trackviewID = params.m_TrackViewID;
	if(trackviewID)
	{
		IAnimSequence* pSequence = FindTrackviewSequence(trackviewID);
		if(pSequence)
		{
			IMovieSystem *pMovieSystem = gEnv->pMovieSystem;
			//Ignore requests for an animation that is already playing
			if(!pMovieSystem->IsPlaying(pSequence))
			{
				pMovieSystem->PlaySequence(pSequence, NULL, true, false);
			}
		}
	}
}

IAnimSequence* CMPTrackViewUpr::FindTrackviewSequence(i32 trackviewId)
{
	IMovieSystem *pMovieSystem = gEnv->pMovieSystem;

	for(i32 i = 0; i < pMovieSystem->GetNumSequences(); ++i)
	{
		IAnimSequence* pSequence = pMovieSystem->GetSequence(i); 
		DrxHashStringId id(pSequence->GetName());
		if(id == trackviewId)
		{
			return pSequence;
		}
	}

	return NULL;
}

void CMPTrackViewUpr::OnMovieEvent(IMovieListener::EMovieEvent movieEvent, IAnimSequence* pAnimSequence)
{
	switch(movieEvent)
	{
	case IMovieListener::eMovieEvent_Started:
		{
			if (pAnimSequence->GetFlags() & IAnimSequence::eSeqFlags_NoMPSyncingNeeded)
			{
				DrxLog("CMPTrackViewUpr::OnMovieEvent MOVIE_EVENT_START() Skipping twelling clients about anim that does not need to be synced");
				return;
			}

			IMovieSystem *pMovieSystem = gEnv->pMovieSystem;
			
			DrxHashStringId sequenceId(pAnimSequence->GetName());

			CGameRules::STrackViewParameters params;
			params.m_NumberOfTrackViews = 1;
			params.m_Ids[0] = sequenceId.id;
			params.m_Times[0] = pMovieSystem->GetPlayingTime(pAnimSequence).ToFloat();
			g_pGame->GetGameRules()->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClTrackViewSynchAnimations(), params, eRMI_ToRemoteClients, 0);

			//Need to remove from finished list if present
			for(i32 i = 0; i < m_FinishedTrackViewCount; ++i)
			{
				if(sequenceId == m_FinishedTrackViews[i])
				{
					if(i != m_FinishedTrackViewCount-1) //If not at end swap element with end element
					{
						m_FinishedTrackViews[i] = m_FinishedTrackViews[m_FinishedTrackViewCount-1];
						m_FinishedTrackViewTimes[i] = m_FinishedTrackViewTimes[m_FinishedTrackViewCount-1];
					}
					--m_FinishedTrackViewCount;
				}
			}
		}
		break;

	case IMovieListener::eMovieEvent_Stopped:
	case IMovieListener::eMovieEvent_Aborted:
		{
			if (pAnimSequence->GetFlags() & IAnimSequence::eSeqFlags_NoMPSyncingNeeded)
			{
				DrxLog("CMPTrackViewUpr::Server_SynchAnimationTimes() skipping remembering of a completed playing anim sequence %s in the finishedTrackviews as it has NO_MP_SYNCING_NEEDED flag set", pAnimSequence->GetName());
				return;
			}

			//If a movie finishes we should store this fact so we can tell clients that join later that the animation has already been played
			IMovieSystem *pMovieSystem = gEnv->pMovieSystem;
			DrxHashStringId sequenceId(pAnimSequence->GetName());
			float timeValue = pMovieSystem->GetPlayingTime(pAnimSequence).ToFloat();	// aborted movies will not be at the end, send the stopped movies endtime to save the client from playing to figure out its length
			if(m_FinishedTrackViewCount < CGameRules::STrackViewParameters::sMaxTrackViews)
			{
				m_FinishedTrackViews[m_FinishedTrackViewCount] = sequenceId.id;
				m_FinishedTrackViewTimes[m_FinishedTrackViewCount] = timeValue;
				m_FinishedTrackViewCount++;
			}
#ifndef _RELEASE
			else
			{
				DrxWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_WARNING, "Trying to store that animation %s is finished, but the array is full so ignoring",
					pAnimSequence->GetName());
			}
#endif
			CGameRules::STrackViewParameters params;
			params.m_NumberOfTrackViews = 0;
			params.m_NumberOfFinishedTrackViews = 1;
			params.m_Ids[0] = sequenceId.id;
			params.m_Times[0] = timeValue;
			g_pGame->GetGameRules()->GetGameObject()->InvokeRMIWithDependentObject(CGameRules::ClTrackViewSynchAnimations(), params, eRMI_ToRemoteClients, 0);
		}
		break;
	}
}

bool CMPTrackViewUpr::HasTrackviewFinished( const DrxHashStringId& id ) const
{
	for(i32 i = 0; i < m_FinishedTrackViewCount; ++i)
	{
		if(id == m_FinishedTrackViews[i])
		{
			return true;
		}
	}

	return false;
}
