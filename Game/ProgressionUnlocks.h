// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 08:04:2010		Created by Ben Parbury
*************************************************************************/

#ifndef __PROGRESSIONUNLOCKS_H__
#define __PROGRESSIONUNLOCKS_H__

#include <drx3D/Game/Utility/StringUtils.h>
#include <drx3D/CoreX/Containers/DrxFixedArray.h>
#include <drx3D/CoreX/String/StringUtils.h>

// These are in order sorted for display. Highest displayed first
// This enum matches Flash Actionscript code, so must be kept in sync
enum EUnlockType
{
	eUT_Invalid = -1,
	eUT_TokenMax,		//Used for progressionTokenSystem (Tokens should run from 0 to TokenMax)
	eUT_CreateCustomClass = eUT_TokenMax,
	eUT_Loadout,
	eUT_Weapon,
	eUT_Playlist,
	eUT_Attachment,
	eUT_Max,
};

// These are in order sorted for display. Highest displayed first
enum EUnlockReason
{
	eUR_Invalid = -1,
	eUR_None = 0,
	eUR_SuitRank = 1,
	eUR_Rank = 2,
	eUR_Token = 3,
	eUR_Assessment = 4,
	eUR_Max,
};

struct SPPHaveUnlockedQuery
{
	SPPHaveUnlockedQuery() { Clear(); }

	void Clear() { unlockString.clear(); reason = eUR_None; exists = false; unlocked = false; available = false; getstring = true; }

	DrxFixedStringT<128> unlockString;
	EUnlockReason reason;
	bool exists;
	bool unlocked;
	bool available;	// if you're at the right rank but it may still be locked
	bool getstring; // set to false to avoid obtaining and localizing the weapon description. Saves 0.1ms on 360
};

struct SUnlock
{
	SUnlock(XmlNodeRef node, i32 rank);

	void Unlocked(bool isNew);

	const static i32 k_maxNameLength = 32;
	char m_name[k_maxNameLength];

	EUnlockType m_type;
	int8 m_rank;
	int8 m_reincarnation;
	bool m_unlocked;

	static EUnlockType GetUnlockTypeFromName(tukk name);
	static tukk  GetUnlockTypeName(EUnlockType type);

	static EUnlockReason GetUnlockReasonFromName(tukk name);
	static tukk  GetUnlockReasonName(EUnlockReason reason);

	static tukk  GetUnlockTypeDescriptionString(EUnlockType type);
	static tukk  GetUnlockReasonDescriptionString(EUnlockReason reason, i32 data=0);

	static bool GetUnlockDisplayString( EUnlockType type, tukk name, DrxFixedStringT<32>& outStr );

	bool operator ==(const SUnlock &rhs) const;
};

#endif // __PROGRESSIONUNLOCKS_H__
