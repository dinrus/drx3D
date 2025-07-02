// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 09:03:2012		Created by Colin Gulliver
*************************************************************************/

#pragma once

#include <drx3D/CoreX/String/DrxFixedString.h>

struct IEntityClass;

class CStatsEntityIdRegistry
{
public:
	CStatsEntityIdRegistry();

	u16 GetGameModeId(tukk pModeName) const;
	u16 GetGameModeId(const IEntityClass *pClass) const;

	u16 GetMapId(tukk pMapName) const;
	
	u16 GetWeaponId(tukk pWeaponName) const;
	u16 GetWeaponId(const IEntityClass *pClass) const;

	u16 GetHitTypeId(tukk pHitTypeName) const; 
	u16 GetDefaultWeapon() const { return m_defaultWeapon; }

	tukk GetGameMode(u16 id) const;
	tukk GetMap(u16 id) const;
	tukk GetWeapon(u16 id) const;
	tukk GetHitType(u16 id) const; 
	
private:
	typedef DrxFixedStringT<32> TFixedString;

	struct SClassId
	{
		const IEntityClass *m_pClass;
		u16 m_id;
	};

	struct SStringId
	{
		TFixedString m_name;
		u16 m_id;
	};

	typedef std::vector<SClassId> TClassIdVec;
	typedef std::vector<SStringId> TStringIdVec;

	static void ReadClassIds(XmlNodeRef xmlNode, u16 &defaultId, TClassIdVec &vec );
	static void ReadStringIds(XmlNodeRef xmlNode, u16 &defaultId, TStringIdVec &vec );
	
	TClassIdVec m_gameModes;
	TClassIdVec m_weapons;
	TStringIdVec m_weaponExtensions;
	TStringIdVec m_maps;
	TStringIdVec	m_hitTypes;
	
	u16 m_defaultGameMode;
	u16 m_defaultWeapon;
	u16 m_defaultMap;
	u16 m_defaultHitType;
};
