// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 23:09:2009   : Created by Jan Neugebauer

*************************************************************************/

#pragma once

#ifndef WEAPONSTATS_H
#define WEAPONSTATS_H

#include <drx3D/Game/ItemParamsRegistration.h>

class CItemSharedParams;
struct SAccessoryParams;

enum EWeaponStat
{
	eWeaponStat_Accuracy = 0,
	eWeaponStat_RateOfFire,
	eWeaponStat_Mobility,
	eWeaponStat_Damage,
	eWeaponStat_Range,
	eWeaponStat_Recoil,
	eWeaponStat_NumStats
};

static tukk s_weaponStatNames[eWeaponStat_NumStats] = { "stat_accuracy", "stat_rate_of_fire", "stat_mobility", "stat_damage", "stat_range", "stat_recoil" };

struct SWeaponStatsData
{
	SWeaponStatsData();

	void ReadStats(const XmlNodeRef& paramsNode, bool defaultInit=false);
	void ReadStatsByAttribute(const XmlNodeRef& paramsNode, bool defaultInit = false);

	void GetMemoryUsage(IDrxSizer* s) const {}

	typedef DrxFixedArray<i32, eWeaponStat_NumStats> StatsArray;

#define WEAPON_STATSDATA_PARAMS(f)
#define WEAPON_STATSDATA_PARAM_VECTORS(f) \
	f(StatsArray, stats)

	REGISTER_STRUCT(WEAPON_STATSDATA_PARAM_VECTORS, SWeaponStatsData)
};

struct SWeaponStatStoredData
{
	SWeaponStatStoredData()
	: totalValue(0)
	, baseValue(0)
	, accessoryModifier(0)
	, firemodeModifier(0)
	{
	}

	void UpdateTotalValue();

	i32 totalValue;
	i32 baseValue;
	i32 accessoryModifier;
	i32 firemodeModifier;

};

class CWeaponStats
{

public:

	CWeaponStats();
	~CWeaponStats();

	void SetBaseStats(const CItemSharedParams* params);
	void SetStatsFromAccessory(const SAccessoryParams *params, const bool attach);
	void SetStatsFromFiremode(const SWeaponStatsData *pStats);
	i32k GetStat(const EWeaponStat stat) const;
	const SWeaponStatStoredData& GetStatData(const EWeaponStat stat) const;
	void SetStat(const EWeaponStat stat, const SWeaponStatStoredData& statData);
	void UpdateTotalValue();
private:

	DrxFixedArray<SWeaponStatStoredData, eWeaponStat_NumStats> m_weaponStats;
	
	typedef std::set<const IEntityClass*> TWeaponStatsAttachments;
	TWeaponStatsAttachments m_attachments;
};

#endif //WEAPONSTATS_H
