// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PLAYER_ENTITY_INTERACTION_H__
#define __PLAYER_ENTITY_INTERACTION_H__

#if _MSC_VER > 1000
# pragma once
#endif



class CPlayerEntityInteraction
{
	struct SLastInteraction
	{
		SLastInteraction()
		{
			Reset();
		}

		void Reset()
		{
			m_frameId = 0;
		}
		 
		bool CanInteractThisFrame( i32k frameId ) const
		{
			return (m_frameId != frameId);
		}

		void Update( i32k frameId )
		{
			m_frameId = frameId;
		}

	private:
		i32      m_frameId;
	};

public:
	CPlayerEntityInteraction();

	void ItemPickUpMechanic(CPlayer* pPlayer, const ActionId& actionId, i32 activationMode);
	void UseEntityUnderPlayer(CPlayer* pPlayer);

	void JustInteracted( );

	void Update(CPlayer* pPlayer, float frameTime);

private:
	void ReleaseHeavyWeapon(CPlayer* pPlayer);

	SLastInteraction m_lastUsedEntity;

	float m_autoPickupDeactivatedTime;

	bool m_useHoldFiredAlready;
	bool m_usePressFiredForUse;
	bool m_usePressFiredForPickup;
	bool m_useButtonPressed;
};



#endif
