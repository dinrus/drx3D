// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/EnvironmentDisturbanceUpr.h>
#include <drx3D/Game/Agent.h>
#include <drx3D/AI/IAISystem.h>
#include <drx3D/AI/VisionMapTypes.h>
#include <drx3D/AI/IAIObjectUpr.h>

namespace GameAI
{
	void ObservableEvent::Initialize( const Vec3& position, float duration, u8 faction, tukk signal )
	{
		IVisionMap* visionMap = gEnv->pAISystem->GetVisionMap();
		assert(visionMap);
		if(!visionMap)
			return;

		static u32 eventCounter = 0;
		string visionName;
		visionName.Format( "EnvironmentDisturbanceVisualEvent%d", eventCounter++ );
		m_visionId = visionMap->CreateVisionID( visionName );

		ObservableParams observableParams;
		observableParams.observablePositionsCount = 1;
		observableParams.observablePositions[0] = position;
		observableParams.typeMask = General;
		observableParams.faction = faction;
		visionMap->RegisterObservable( m_visionId, observableParams );

		m_expirationTime = gEnv->pTimer->GetFrameStartTime() + duration;
		m_position = position;
		m_signal = signal;
	}

	void ObservableEvent::Release()
	{
		if ( m_visionId )
		{
			gEnv->pAISystem->GetVisionMap()->UnregisterObservable( m_visionId );
		}
	}

	bool ObservableEvent::IsExpired( CTimeValue currentTime )
	{
		return currentTime > m_expirationTime;
	}

	void ObservableEvent::SetObservedBy( EntityId entityId )
	{
		stl::push_back_unique(m_obeservedBy, entityId);
	}

	bool ObservableEvent::HasBeenObservedBy( EntityId entityId )
	{
		return stl::find( m_obeservedBy, entityId );
	}


	EnvironmentDisturbanceUpr::~EnvironmentDisturbanceUpr()
	{
		Reset();
	}

	void EnvironmentDisturbanceUpr::Reset()
	{
		ObservableEvents::iterator observableEventIterator = m_observableEvents.begin();
		ObservableEvents::iterator observableEventIteratorEnd = m_observableEvents.end();
		while ( observableEventIterator != observableEventIteratorEnd )
		{
			(*observableEventIterator).Release();
			++observableEventIterator;
		}
		stl::free_container(m_observableEvents);
	}

	void EnvironmentDisturbanceUpr::Update()
	{
		if( m_observableEvents.empty() )
			return;

		RemoveExpiredEvents();

		AutoAIObjectIter aiActorsIterator( gEnv->pAISystem->GetAIObjectUpr()->GetFirstAIObject( OBJFILTER_TYPE, AIOBJECT_ACTOR ) );
		while ( IAIObject* aiObject = aiActorsIterator->GetObject() )
		{
			Agent agent( aiObject );

			ObservableEvents::iterator observableEventIterator = m_observableEvents.begin();
			while ( observableEventIterator != m_observableEvents.end() )
			{
				ObservableEvent& observableEvent = *observableEventIterator;
				if ( !observableEvent.HasBeenObservedBy( agent.GetEntityID() ) && agent.CanSee( observableEvent.GetVisionId() ) )
				{
					observableEvent.SetObservedBy( agent.GetEntityID() );

					IAISignalExtraData* data = gEnv->pAISystem->CreateSignalExtraData();
					data->point = observableEvent.GetPosition();
					agent.SetSignal( SIGNALFILTER_SENDER, observableEvent.GetSignal(), data );
				}

				++observableEventIterator;
			}

			aiActorsIterator->Next();
		}
	}

	void EnvironmentDisturbanceUpr::AddObservableEvent( const Vec3& position, float duration, tukk signal, EntityId sourceEntityId /*= 0*/ )
	{
		Agent sourceAgent( sourceEntityId );
		u8 faction = sourceAgent.IsValid() ? sourceAgent.GetFactionID() : 31;
		ObservableEvent observableEvent;
		observableEvent.Initialize( position, duration, faction, signal );
		m_observableEvents.push_back( observableEvent );
	}

	void EnvironmentDisturbanceUpr::RemoveExpiredEvents()
	{
		CTimeValue currentTime = gEnv->pTimer->GetFrameStartTime();

		ObservableEvents::iterator observableEventIterator = m_observableEvents.begin();
		while ( observableEventIterator != m_observableEvents.end() )
		{
			ObservableEvent& observableEvent = *observableEventIterator;
			if ( observableEvent.IsExpired( currentTime ) )
			{
				observableEvent.Release();
				observableEventIterator = m_observableEvents.erase( observableEventIterator );
			}
			else
			{
				++observableEventIterator;
			}
		}
	}
}