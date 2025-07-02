// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __AIDEMOINPUT_H__
#define __AIDEMOINPUT_H__

#pragma once

#if (USE_DEDICATED_INPUT)

//#include <drx3D/Game/IEntitySystem.h>
#include <drx3D/Game/IPlayerInput.h>
//#include <drx3D/Game/GameRulesModules/IGameRulesSpawningModule.h>
//#include <drx3D/Game/GameCVars.h>

class CPlayer;

enum EDefaultableBool
{
	eDB_Default = -1,
	eDB_False = 0,
	eDB_True = 1,
};

// special input for dedicated client (simple, generating enough client side traffic for server)
class CDedicatedInput : public IPlayerInput
{
public:
	CDedicatedInput(CPlayer* pPlayer);
	virtual void PreUpdate();
	virtual void Update();
	virtual void PostUpdate();
	virtual void OnAction( const ActionId& action, i32 activationMode, float value );
	virtual void SetState( const SSerializedPlayerInput& input );
	virtual void GetState( SSerializedPlayerInput& input );
	virtual void Reset();
	virtual void DisableXI(bool disabled);
	virtual void ClearXIMovement() {};
	virtual EInputType GetType() const;
	virtual void GetMemoryUsage(IDrxSizer * pSizer) const;

	ILINE virtual u32 GetMoveButtonsState() const { return 0; }
	ILINE virtual u32 GetActions() const { return 0; }

	virtual float GetLastRegisteredInputTime() const { return 0.0f; }
	virtual void SerializeSaveGame( TSerialize ser ) {}

	void GiveItems();

	EDefaultableBool GetFire() { return m_fire; }
	void SetFire(EDefaultableBool value) { m_fire = value; }
	EDefaultableBool GetMove() { return m_move; }
	void SetMove(EDefaultableBool value) { m_move = value; }

private:
	void SelectEquipmentPackage();
	void HandleDeathAndSuicide(CGameRules* const pGameRules);

	CPlayer* m_pPlayer;
	SSerializedPlayerInput m_serializedInput;

	Vec3 m_deltaMovement, m_targetDeltaMovement;
	Ang3 m_deltaRotation, m_targetDeltaRotation;

	CMTRand_int32 m_rand;

	float m_timeBeforeChangeMode;
	float m_interpolateToNewMoveLookInputs;
	float m_grenadeStateTime;
	float m_interpolationSpeed;
	float m_timeUntilDie;
	float m_timeUntilRequestRespawn;
	float m_timeUntilChangeWeapon;

	i32 m_explosiveItemsIdx;
	i32 m_itemIdx;

	u8 m_stance;
	u8 m_jump;
	
	Vec3 m_spawnPosition;
	Vec3 m_targetPosition;
	float m_timeToReachTarget;
	EDefaultableBool m_fire;
	EDefaultableBool m_move;

	enum
	{
		GRENADE_PREPARE,
		GRENADE_PREPARE_WAIT,
		GRENADE_THROW_WAIT
	};
	u8 m_grenadeState;

	enum
	{
		E_SMALLMEDIUM,
		E_EXPLOSIVE,
		E_MELEE,
		E_ZOOM_IN_OUT,
		NUM_MODES
	};
	u8 m_mode;
};

#endif //#if USE_DEDICATED_INPUT

#endif

