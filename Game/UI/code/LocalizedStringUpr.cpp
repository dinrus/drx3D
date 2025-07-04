// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 
	Implementation for CLocalizedStringUpr class
	Shared by G02 and G04


-------------------------------------------------------------------------
История:
- 23:09:2007: Created by David Mondelore

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/LocalizedStringUpr.h>
#include <drx3D/Sys/ILocalizationUpr.h>
#include <drx3D/CoreX/DrxCrc32.h>

//-----------------------------------------------------------------------------------------------------

i32 CLocalizedStringUpr::s_maxAge = -1;

//-----------------------------------------------------------------------------------------------------
CLocalizedStringUpr::CLocalizedStringUpr()
{
	m_curTick = 0;
	m_language = NULL;
}

//-----------------------------------------------------------------------------------------------------
// Adds a generated localized string and it's generation parameters to the cache managed by this class.
const wchar_t* CLocalizedStringUpr::add(const wchar_t* finalStr, tukk label, 
																						bool bAdjustActions, bool bPreferXI, 
																						tukk param1, tukk param2, 
																						tukk param3, tukk param4)
{
	if (gEnv->bMultiplayer)
		return finalStr;

	SLocalizedString entry;
	entry.m_refTick = m_curTick;
	entry.m_finalStr = finalStr;
	entry.m_label = label;
	entry.m_bPreferXI = bPreferXI;
	entry.m_bAdjustActions = bAdjustActions;
	entry.m_param1 = param1;
	entry.m_param2 = param2;
	entry.m_param3 = param3;
	entry.m_param4 = param4;

	Key key = generateKey(label, bAdjustActions, bPreferXI, param1, param2, param3, param4);
	m_cache[key] = entry; // Will overwrite old instance of key (insert would fail if the key already exists), in case of key overlap.

	return entry.m_finalStr;
}

//-----------------------------------------------------------------------------------------------------
// Finds and returns an already generated localized string given it's generation parameters.
// Returns NULL if no match was found.
const wchar_t* CLocalizedStringUpr::find(tukk label, 
																						 bool bAdjustActions, bool bPreferXI,
																						 tukk param1, tukk param2, 
																						 tukk param3, tukk param4)
{
	if (gEnv->bMultiplayer)
		return NULL;

	Key key = generateKey(label, bAdjustActions, bPreferXI, param1, param2, param3, param4);

	Map::iterator found = m_cache.find(key);
	if (found == m_cache.end())
		return NULL;

	SLocalizedString& entry = (*found).second;
	entry.m_refTick = m_curTick;
	return entry.m_finalStr;
}

//-----------------------------------------------------------------------------------------------------
// Generate a unique key for the given label and parameters.
CLocalizedStringUpr::Key CLocalizedStringUpr::generateKey(
	tukk label, 
	bool bAdjustActions, bool bPreferXI,
	tukk param1, tukk param2, 
	tukk param3, tukk param4)
{
	Key actionsKey = bAdjustActions ? -1 : 0;
	Key controllerKey = bPreferXI ? -2 : 0;
	Key labelKey  = (label  != NULL) ? CCrc32::Compute(label)  : (Key)-4;
	Key param1Key = (param1 != NULL) ? CCrc32::Compute(param1) : (Key)-8;
	Key param2Key = (param2 != NULL) ? CCrc32::Compute(param2) : (Key)-16;
	Key param3Key = (param3 != NULL) ? CCrc32::Compute(param3) : (Key)-32;
	Key param4Key = (param4 != NULL) ? CCrc32::Compute(param4) : (Key)-64;
	Key key = actionsKey ^ controllerKey ^ labelKey ^ param1Key ^ param2Key ^ param3Key ^ param4Key;
	return key;
}

//-----------------------------------------------------------------------------------------------------
// Increases the age of cached strings and removes strings older than maxAge (not referenced for that many ticks).
// Should be called once per frame.
void CLocalizedStringUpr::tick()
{
	m_curTick++;

	if (m_curTick > 60*60*1)
	{
		m_curTick = 0;
		m_cache.clear();
	}

	tukk language = gEnv->pSystem->GetLocalizationUpr()->GetLanguage();
	if ((m_language == NULL) || stricmp(m_language, language) != 0)
	{
		m_cache.clear();
		m_language = language;
		return;
	}

/*
	if (s_maxAge < 0)
		return;

	for (Map::iterator i = m_cache.begin(); i != m_cache.end(); )
	{
		SLocalizedString& entry = (*i).second;
		i32 age = m_curTick - entry.m_refTick;
		if (age > s_maxAge)
			m_cache.erase(i); // This will generate an assert on the refcount of the string being zero, which is incorrect.
		else
			++i;
	}
*/
}

//-----------------------------------------------------------------------------------------------------
// Explicitly reset the cache. Should be called on change of map, gamerules, serialization, etc.
void CLocalizedStringUpr::clear()
{
	m_cache.clear();
}

//-----------------------------------------------------------------------------------------------------