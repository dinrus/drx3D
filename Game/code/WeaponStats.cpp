// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 23:09:2009   : Created by Jan Neugebauer

*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/WeaponStats.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/GameXmlParamReader.h>

#define MAX_STAT_VALUE 10

//////////////////////////////////////////////////////////////////////////


SWeaponStatsData::SWeaponStatsData()
{
	i32k numStats = eWeaponStat_NumStats;
	for(i32 i=0; i<numStats; ++i)
	{
		stats.push_back(0);
	}
}

void SWeaponStatsData::ReadStats(const XmlNodeRef& paramsNode, bool defaultInit /*= false*/)
{	
	if (defaultInit)
	{
		i32k numStats = eWeaponStat_NumStats;
		for(i32 i=0; i<numStats; ++i)
		{
			stats[i] = 0;
		}
	}

	if (paramsNode == NULL)
		return;

	CGameXmlParamReader reader(paramsNode);

	for (i32 i  = 0; i < eWeaponStat_NumStats; i++)
	{
		reader.ReadParamValue<i32>(s_weaponStatNames[i], stats[i]);
	}

}

void SWeaponStatsData::ReadStatsByAttribute(const XmlNodeRef& paramsNode, bool defaultInit /*= false*/)
{	
	if (defaultInit)
	{
		i32k numStats = eWeaponStat_NumStats;
		for(i32 i=0; i<numStats; ++i)
		{
			stats[i] = 0;
		}
	}

	if (paramsNode == NULL)
		return;

	i32 iReadValue; // Can't use i32 directly in the vector, somehow will get translated to an u32
	for (i32 i  = 0; i < eWeaponStat_NumStats; i++)
	{
		if (paramsNode->getAttr(s_weaponStatNames[i], iReadValue))
		{
			stats[i] = iReadValue;
		}		
	}
}

IMPLEMENT_OPERATORS_WITH_VECTORS(WEAPON_STATSDATA_PARAMS, WEAPON_STATSDATA_PARAM_VECTORS, SWeaponStatsData)

//////////////////////////////////////////////////////////////////////////


void SWeaponStatStoredData::UpdateTotalValue()
{
	totalValue = baseValue + accessoryModifier + firemodeModifier;
}


//////////////////////////////////////////////////////////////////////////


CWeaponStats::CWeaponStats()
{
	i32k numStats = eWeaponStat_NumStats;
	for(i32 i=0; i<numStats; ++i)
	{
		SWeaponStatStoredData weaponStoredData;
		DRX_TODO(25,08,2010, "Dean - Remove setting weapon stat default value to 5 when all designer data is in");
		weaponStoredData.baseValue = MAX_STAT_VALUE / 2;
		weaponStoredData.totalValue = weaponStoredData.baseValue;
		m_weaponStats.push_back(weaponStoredData);
	}
}



CWeaponStats::~CWeaponStats()
{

}



void CWeaponStats::SetBaseStats(const CItemSharedParams* params)
{
	DRX_ASSERT(m_weaponStats.size() == params->params.weaponStats.stats.size() && m_weaponStats.size() == eWeaponStat_NumStats);
	i32k numStats = eWeaponStat_NumStats;
	for (i32 i = 0; i < numStats; i++)
	{
		m_weaponStats[i].baseValue = params->params.weaponStats.stats[i];
	}	

	UpdateTotalValue();
}



void CWeaponStats::SetStatsFromAccessory(const SAccessoryParams *params, const bool attach)
{

	DRX_ASSERT(m_weaponStats.size() == params->weaponStats.stats.size() && m_weaponStats.size() == eWeaponStat_NumStats);

	// Safeguard against this being called more than once on the same accessory which happens for some weapons
	const IEntityClass* pAccessoryClass = params->pAccessoryClass;
	if (pAccessoryClass)
	{
		i32k numStats = eWeaponStat_NumStats;
		if(attach)
		{
			if (m_attachments.find(pAccessoryClass) != m_attachments.end()) // Don't add stats if already added
			{
				return;
			}
			m_attachments.insert(pAccessoryClass);

			for (i32 i = 0; i < numStats; i++)
			{
				m_weaponStats[i].accessoryModifier += params->weaponStats.stats[i];
			}
		}
		else
		{
			const TWeaponStatsAttachments::iterator iter = m_attachments.find(pAccessoryClass);
			if (iter == m_attachments.end()) // Don't remove stats if already removed
			{
				return;
			}
			m_attachments.erase(iter);

			for (i32 i = 0; i < numStats; i++)
			{
				m_weaponStats[i].accessoryModifier -= params->weaponStats.stats[i];
			}
		}

		UpdateTotalValue();
	}
}



void CWeaponStats::SetStatsFromFiremode(const SWeaponStatsData *pStats)
{
	i32k numStats = eWeaponStat_NumStats;

	if (!pStats) // Valid, notifies clear
	{
		for (i32 i = 0; i < numStats; i++)
		{
			m_weaponStats[i].firemodeModifier = 0;
		}
	}
	else
	{
		DRX_ASSERT(m_weaponStats.size() == pStats->stats.size() && m_weaponStats.size() == eWeaponStat_NumStats);
		for (i32 i = 0; i < numStats; i++)
		{
			m_weaponStats[i].firemodeModifier = pStats->stats[i];
		}
	}

	UpdateTotalValue();
}



i32k CWeaponStats::GetStat(const EWeaponStat stat) const
{
	DRX_ASSERT(stat >= 0 && stat < eWeaponStat_NumStats);
	return m_weaponStats[stat].totalValue;
}


const SWeaponStatStoredData& CWeaponStats::GetStatData(const EWeaponStat stat) const
{
	DRX_ASSERT(stat >= 0 && stat < eWeaponStat_NumStats);
	return m_weaponStats[stat];
}


void CWeaponStats::SetStat(const EWeaponStat stat, const SWeaponStatStoredData& statData)
{
	DRX_ASSERT(stat >= 0 && stat < eWeaponStat_NumStats);
	m_weaponStats[stat] = statData;
}


void CWeaponStats::UpdateTotalValue()
{
	i32k numStats = eWeaponStat_NumStats;
	for (i32 i = 0; i < numStats; i++)
	{
		SWeaponStatStoredData& weaponStat = m_weaponStats[i];
		weaponStat.UpdateTotalValue();
	}	

}


//////////////////////////////////////////////////////////////////////////