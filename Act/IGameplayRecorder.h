// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IGAMEPLAYRECORDER_H__
#define __IGAMEPLAYRECORDER_H__

#pragma once

#include "IGameStateRecorder.h"
typedef enum
{
	eGE_DiscreetSample = 0,
	eGE_GameReset,
	eGE_GameStarted,
	eGE_SuddenDeath,
	eGE_RoundEnd,
	eGE_GameEnd,
	eGE_Connected,
	eGE_Disconnected,
	eGE_Renamed,
	eGE_ChangedTeam,
	eGE_Died,
	eGE_Scored,
	eGE_Currency,
	eGE_Rank,
	eGE_Spectator,
	eGE_ScoreReset,

	eGE_AttachedAccessory,

	eGE_ZoomedIn,
	eGE_ZoomedOut,

	eGE_Kill,
	eGE_Death,
	eGE_Revive,

	eGE_SuitModeChanged,

	eGE_Hit,
	eGE_Damage,

	eGE_WeaponHit,
	eGE_WeaponReload,
	eGE_WeaponShot,
	eGE_WeaponMelee,
	eGE_WeaponFireModeChanged,
	eGE_AmmoCount,

	eGE_ItemSelected,
	eGE_ItemPickedUp,
	eGE_ItemDropped,
	eGE_ItemBought,
	eGE_ItemExchanged,

	eGE_EnteredVehicle,
	eGE_LeftVehicle,
	eGE_HealthChanged,
	eGE_EntityGrabbed,

	eGE_Last
} EGameplayEvent;

struct GameplayEvent
{
	GameplayEvent() : event(0), description(0), value(0), extra(0), strData(NULL), ivalue(0) {};
	GameplayEvent(u8 evt, tukk desc = 0, float val = 0.0f, uk xtra = 0, tukk str_data = 0, i32 int_val = 0)
		: event(evt), description(desc), value(val), extra(xtra), strData(str_data), ivalue(int_val){};

	u8       event;
	tukk description;
	float       value;
	tukk strData;
	i32         ivalue;
	uk       extra;
};

struct IGameplayListener
{
	virtual ~IGameplayListener(){}
	virtual void OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event) = 0;
};

struct IMetadata;

struct IGameplayRecorder
{
	virtual ~IGameplayRecorder(){}
	virtual void                RegisterListener(IGameplayListener* pGameplayListener) = 0;
	virtual void                UnregisterListener(IGameplayListener* pGameplayListener) = 0;

	virtual IGameStateRecorder* EnableGameStateRecorder(bool bEnable, IGameplayListener* pGameplayListener, bool bRecording) = 0;
	virtual IGameStateRecorder* GetIGameStateRecorder() = 0;

	virtual CTimeValue          GetSampleInterval() const = 0;

	virtual void                Event(IEntity* pEntity, const GameplayEvent& event) = 0;

	virtual void                OnGameData(const IMetadata* pGameData) = 0;
};

#endif //__IGAMEPLAYRECORDER_H__
