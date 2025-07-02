// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 
	Header for CLocalizedStringUpr class
	Shared by G02 and G04


-------------------------------------------------------------------------
История:
- 23:09:2007: Created by David Mondelore

*************************************************************************/
#ifndef __LocalizedStringUpr_h__
#define __LocalizedStringUpr_h__

//-----------------------------------------------------------------------------------------------------

#include <map>

//-----------------------------------------------------------------------------------------------------

class CLocalizedStringUpr
{

public:

	static i32 s_maxAge;

	CLocalizedStringUpr();

	// Adds a generated localized string and it's generation parameters to the cache managed by this class.
	// Only pointers are copied, not whole string content.
	// Returns a pointer to the internally allocated string.
	const wchar_t* add(const wchar_t* finalStr, tukk label, bool bAdjustActions, bool bPreferXI, tukk param1, tukk param2, tukk param3, tukk param4);
	
	// Finds and returns an already generated localized string given it's generation parameters.
	// Returns NULL if no match was found.
	// Uses strcmp for internal comparison (which does initial pointer comparison, afaik).
	const wchar_t* find(tukk label, bool bAdjustActions, bool bPreferXI, tukk param1, tukk param2, tukk param3, tukk param4);

	// Increases the age of cached strings and remove/delete strings older than s_maxAge.
	// Should be called once per frame.
	// NOTE: Current implementation does NOT remove old/unreferenced strings (due to problem with wstring refcount).
	// Cache is invalidated when language changes.
	void tick();

	// Explicitly reset the cache. Should be called on change of map, gamerules, serialization, etc.
	void clear();

private:

	struct SLocalizedString
	{
		i32 m_refTick;
		bool m_bAdjustActions;
		bool m_bPreferXI;
		wstring m_finalStr;
		tukk m_label;
		tukk m_param1;
		tukk m_param2;
		tukk m_param3;
		tukk m_param4;
	};

	typedef u32 Key;
	typedef std::map<Key, SLocalizedString> Map;
	typedef std::pair<Key, SLocalizedString> Pair;

	Map m_cache;

	i32 m_curTick;

	tukk m_language;

	Key generateKey(tukk label, bool bAdjustActions, bool bPreferXI,
									tukk param1, tukk param2, 
									tukk param3, tukk param4);

};

//-----------------------------------------------------------------------------------------------------

#endif // __LocalizedStringUpr_h__

//-----------------------------------------------------------------------------------------------------
