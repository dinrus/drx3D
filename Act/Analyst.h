// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ANALYST_H__
#define __ANALYST_H__

#pragma once

#include "IGameplayRecorder.h"

class CGameplayAnalyst : public IGameplayListener
{
public:
	CGameplayAnalyst();
	virtual ~CGameplayAnalyst();

	// IGameplayListener
	virtual void OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event);
	//~IGameplayListener

	void ProcessPlayerEvent(EntityId id, const GameplayEvent& event);

	// Feel free to add
	void GetMemoryStatistics(IDrxSizer* s) {}

	typedef struct WeaponAnalysis
	{
		WeaponAnalysis() : usage(0), timeUsed(0.0f), zoomUsage(0), timeZoomed(0.0f), firemodes(0), melee(0), shots(0), hits(0), reloads(0), kills(0), deaths(0), damage(0) {}

		i32        usage;    // number of times this weapon was selected
		CTimeValue timeUsed; // total time this weapon was used

		i32        zoomUsage;  // number of zoom ins
		CTimeValue timeZoomed; // total times zoomed with this weapon

		i32        firemodes; // total number of firemode changes

		i32        melee; // total number of melee attacks with this weapon
		i32        shots; // total number of shots fired with this weapon
		i32        hits;  // total number of hits with this weapon
		i32        reloads;// total number of reloads with this weapon
		i32        kills;  // kills with this weapon
		i32        deaths; // deaths while carrying this weapon
		i32        damage; // total damage dealt with this weapon
	}WeaponAnalysis;

	typedef struct PurchaseDetail
	{
		PurchaseDetail() : totalAmount(0), totalSpent(0) {}

		i32 totalAmount;    // total amount of times purchased
		i32 totalSpent;     // total amount of currency spent
	}PurchaseDetail;

	typedef struct CurrencyAnalysis
	{
		CurrencyAnalysis() : totalEarned(0), totalSpent(0), nMin(0), nMax(0) {}

		i32                              totalEarned; // total number of currency earned
		i32                              totalSpent;  // total number of currency spent
		i32                              nMin;        // lowest ever currency value
		i32                              nMax;        // max ever currency value

		std::map<string, PurchaseDetail> spentDetail; // total currency spent on each item
	} CurrencyAnalysis;

	typedef struct SuitAnalysis
	{
		SuitAnalysis()
			: mode(3)
		{
			for (i32 i = 0; i < 4; i++)
			{
				timeUsed[i] = 0.0f;
				usage[i] = 0;
				kills[i] = 0;
				deaths[i] = 0;
			}
		}

		CTimeValue usageStart;
		CTimeValue timeUsed[4];   // total time using each suit mode
		i32        usage[4];      // number of times this suit mode was selected

		i32        kills[4];      // number of kills using each suit mode
		i32        deaths[4];     // number of deaths using each suit mode
		i32        mode;
	} SuitAnalysis;

	typedef std::map<string, WeaponAnalysis> Weapons;

	typedef struct PlayerAnalysis
	{
		PlayerAnalysis()
			: promotions(0),
			demotions(0),
			rank(0),
			rankStart(0.0f),
			maxRank(0),
			zoomUsage(0),
			itemUsage(0),
			firemodes(0),
			melee(0),
			shots(0),
			hits(0),
			reloads(0),
			damage(0),
			kills(0),
			deaths(0),
			deathStart(0.0f),
			timeDead(0.0f),
			timeAlive(0.0f),
			timeStart(0.0f),
			alive(false)
		{
			memset(rankTime, 0, sizeof(rankTime));
		};

		string           name; // player name

		i32              promotions;
		i32              demotions;
		i32              rank;
		CTimeValue       rankStart;

		i32              maxRank; // max rank achieved
		CTimeValue       rankTime[10];// total time spent in each rank

		i32              zoomUsage; // total number of times zoomed
		i32              itemUsage; // total number of item selections
		i32              firemodes; // total number of firemode changes

		i32              melee;   // total number of melee attacks
		i32              shots;   // total number of shots
		i32              hits;    // total number of hits
		i32              reloads; // total number of reloads
		i32              damage;  // total damage dealt
		i32              kills;   // total number of kills
		i32              deaths;  // total number of deaths
		bool             alive;

		CTimeValue       deathStart;
		CTimeValue       timeDead;  // time dead / waiting to respawn
		CTimeValue       timeAlive; // time alive
		CTimeValue       timeStart; // total time played

		Weapons          weapons;   // weapon analysis
		SuitAnalysis     suit;
		CurrencyAnalysis currency;  // currency analysis
	} PlayerAnalysis;

	typedef std::map<EntityId, PlayerAnalysis> Players;

	typedef struct
	{
		Players players;
	} GameAnalysis;

	void Release() { delete this; };

	void Reset();
	void DumpToTXT();

	void DumpWeapon(EntityId playerId, string& lines);
	void DumpRank(string& lines);
	void DumpSuit(string& lines);
	void DumpCurrency(string& lines);

private:
	void            NewPlayer(IEntity* pEntity);
	bool            IsPlayer(EntityId entityId) const;

	PlayerAnalysis& GetPlayer(EntityId playerId);

	WeaponAnalysis& GetWeapon(EntityId playerId, EntityId weaponId);
	WeaponAnalysis& GetWeapon(EntityId playerId, tukk weapon);
	WeaponAnalysis& GetCurrentWeapon(EntityId playerId);

	GameAnalysis    m_gameanalysis;
};
#endif
