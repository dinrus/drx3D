// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id:$
$DateTime$
Описание:
-------------------------------------------------------------------------
История:
- 07:04:2010: Created by Filipe Amim

*************************************************************************/

#pragma once

#ifndef _HEAVY_WEAPON_H_
#define _HEAVY_WEAPON_H_

#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/GameRulesModules/IGameRulesClientConnectionListener.h>

class CActor;

class CHeavyWeapon : public CWeapon, public IGameRulesClientConnectionListener
{
private:
	struct StopUseAction;
	struct DetachItemAction;
	typedef CWeapon BaseClass;

public:
	CHeavyWeapon();
	virtual ~CHeavyWeapon();

	virtual bool CanPickUp(EntityId userId) const;
	virtual bool CanUse(EntityId userId) const;
	virtual void Use(EntityId userId);
	virtual void StartUse(EntityId userId);
	virtual void StopUse(EntityId  userId);
	virtual bool IsPickable() const;
	virtual bool IsHeavyWeapon() const;
	virtual bool IsRippedOff() const;
	virtual void GetAngleLimits(EStance stance, float& minAngle, float& maxAngle);
	virtual bool CanSelect() const;
	virtual bool AllowInteraction(EntityId interactionEntity, EInteractionType interactionType);

	virtual void PostSerialize();
	virtual void InitClient(i32 channelId);

	using CWeapon::StartFire;
	virtual void StartFire(const SProjectileLaunchParams& launchParams);

	// IGameRulesClientConnectionListener
	virtual void OnClientConnect(i32 channelId, bool isReset, EntityId playerId) {};
	virtual void OnClientDisconnect(i32 channelId, EntityId playerId) {};
	virtual void OnClientEnteredGame(i32 channelId, bool isReset, EntityId playerId) {}
	virtual void OnOwnClientEnteredGame();
	// ~IGameRulesClientConnectionListener

	struct SHeavyWeaponUserParams
	{
		SHeavyWeaponUserParams(): ownerId(0) {};
		SHeavyWeaponUserParams(EntityId owner): ownerId(owner) {};
		void SerializeWith(TSerialize ser)
		{
			ser.Value("ownerId", ownerId, 'eid');
		}
		EntityId ownerId;
	};

	struct SNoParams
	{
		SNoParams() {}
		void SerializeWith(TSerialize ser) {}
	};

	DECLARE_CLIENT_RMI_NOATTACH(ClDeselectAndDrop, SHeavyWeaponUserParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH(ClHeavyWeaponUsed, SHeavyWeaponUserParams, eNRT_ReliableUnordered);
	DECLARE_CLIENT_RMI_NOATTACH(ClHeavyWeaponHighlighted, SNoParams, eNRT_ReliableUnordered);

protected:
	void HandleHeavyWeaponPro(CActor& rOwner);

	FragmentID GetSelectAction() const;
	virtual void DeselectAndDrop(EntityId userId);
};


#endif
