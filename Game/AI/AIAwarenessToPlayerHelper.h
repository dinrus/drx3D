// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: Helper to calculate a value that represents AI awareness toward the player.
*************************************************************************/

#pragma once

#ifndef __AIAWARENESSTOPLAYERHELPER_H__
#define __AIAWARENESSTOPLAYERHELPER_H__

struct IAIActor;
struct IAIObject;
class CActor;

class CAIAwarenessToPlayerHelper
{
	public:
		struct IListener
		{
			virtual	~IListener(){}
			virtual void AwarenessChanged( i32 value ) = 0; // listeners are notified only when the integer value changes, not every time the internal float values change.
		};

		// entities that need to be added to the awareness calculation, but that are not AIActors (turrets and towers atm)
		struct IAwarenessEntity
		{
			virtual	~IAwarenessEntity(){}
			virtual i32 GetAwarenessToActor( IAIObject* pAIObject, CActor* pActor ) const = 0;
		};

		enum
		{
			AI_Awarness_To_Player_Aware = 50,
			AI_Awarness_To_Player_Alerted = 100
		};

		enum VisorIconColor
		{
			Green,
			Orange,
			Red
		};
	
	public:
		CAIAwarenessToPlayerHelper() { Reset(); }
		void Update( float frameTime );

		void Serialize( TSerialize ser );
		float GetFloatAwareness() const { return m_actualAwareness; };  // 0..100 value.
		i32 GetIntAwareness() const { return m_intCurrentAwareness; };  // 0..100 value.
		void Reset();
		void AddListener( IListener *pListener );
		void RemoveListener( IListener *pListener );
		VisorIconColor GetMarkerColorForAgent(const EntityId entityId) const;
		void RegisterAwarenessEntity( IAwarenessEntity* pAwarenessEntity );
		void UnregisterAwarenessEntity( IAwarenessEntity* pAwarenessEntity );

		void AddTurret(const EntityId turretId);
		void RemoveTurret(const EntityId turretId);


	private:
		void NotifyListeners();
		i32 GetAlertnessAffectedByVisibility(const IAIActor& aiActor, const IAIObject& playerAiObject, const bool playerIsCloaked) const;
		void RecalculateAwareness();

		static const float kAIAwarenessToPlayerAware;
		static const float kAIAwarenessToPlayerAlerted;

		i32 m_intCurrentAwareness;
		float m_actualAwareness; // 0..100 value
		float m_animatedAwarenessThatStrivesTowardsActualAwareness; // 0..100 value
		float m_timeToRecalculateAwareness;

		typedef std::vector<IListener *> TListenersVector;
		TListenersVector m_listeners;

		// entities that need to be added to the awareness calculation, but that are not actual AI objects (turrets and towers atm)
		typedef std::vector<IAwarenessEntity*> SAwarenessEntitiesVector;
		SAwarenessEntitiesVector m_awarenessEntities;
};

#endif //__AIAWARENESSTOPLAYERHELPER_H__
