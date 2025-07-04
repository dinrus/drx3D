// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 09:03:2012		Created by Colin Gulliver
*************************************************************************/

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/StatsEntityIdRegistry.h>

#include <drx3D/Entity/IEntitySystem.h>

//-------------------------------------------------------------------------
CStatsEntityIdRegistry::CStatsEntityIdRegistry()
	: m_defaultGameMode(0)
	, m_defaultMap(0)
	, m_defaultWeapon(0)
	, m_defaultHitType(0)
{
	XmlNodeRef xmlRoot = gEnv->pSystem->LoadXmlFromFile("Scripts/Progression/StatsEntityIds.xml");
	if (xmlRoot)
	{
		i32k numChildren = xmlRoot->getChildCount();
		for (i32 i = 0; i < numChildren; ++ i)
		{
			XmlNodeRef xmlChild = xmlRoot->getChild(i);
			tukk pTag = xmlChild->getTag();
			if (!stricmp(pTag, "GameModes"))
			{
				ReadClassIds(xmlChild, m_defaultGameMode, m_gameModes);
			}
			else if (!stricmp(pTag, "Weapons"))
			{
				ReadClassIds(xmlChild, m_defaultWeapon, m_weapons);
			}
			else if (!stricmp(pTag, "WeaponExtensions"))
			{
				ReadStringIds(xmlChild, m_defaultWeapon, m_weaponExtensions);
			}
			else if (!stricmp(pTag, "Maps"))
			{
				ReadStringIds(xmlChild, m_defaultMap, m_maps);
			}
			else if (!stricmp(pTag, "DamageTypes"))
			{
				ReadStringIds(xmlChild, m_defaultHitType, m_hitTypes);
			}
		}
	}
}

//-------------------------------------------------------------------------
/* static */ void CStatsEntityIdRegistry::ReadClassIds( XmlNodeRef xmlNode, u16 &defaultId, TClassIdVec &vec )
{
	tukk pDefault;
	if (xmlNode->getAttr("default", &pDefault))
	{
		defaultId = atoi(pDefault);
	}

	const IEntityClassRegistry *pClassRegistry = gEnv->pEntitySystem->GetClassRegistry();
	i32k numEntities = xmlNode->getChildCount();
	vec.reserve(numEntities);
	for (i32 i = 0; i < numEntities; ++ i)
	{
		XmlNodeRef xmlEntity = xmlNode->getChild(i);

		tukk pName;
		tukk pValue;
		if (xmlEntity->getAttr("name", &pName) && xmlEntity->getAttr("value", &pValue))
		{
			SClassId entity;
			entity.m_pClass = pClassRegistry->FindClass(pName);
			DRX_ASSERT_TRACE(entity.m_pClass, ("Failed to find class '%s' referenced in StatsEntityIds.xml", pName));
			entity.m_id = atoi(pValue);
			vec.push_back(entity);
		}
	}
}

//-------------------------------------------------------------------------
/* static */ void CStatsEntityIdRegistry::ReadStringIds( XmlNodeRef xmlNode, u16 &defaultId, TStringIdVec &vec )
{
	tukk pDefault;
	if (xmlNode->getAttr("default", &pDefault))
	{
		defaultId = atoi(pDefault);
	}

	i32k numEntities = xmlNode->getChildCount();
	vec.reserve(numEntities);
	for (i32 i = 0; i < numEntities; ++ i)
	{
		XmlNodeRef xmlEntity = xmlNode->getChild(i);

		tukk pName;
		tukk pValue;
		if (xmlEntity->getAttr("name", &pName) && xmlEntity->getAttr("value", &pValue))
		{
			SStringId entity;
			entity.m_name = pName;
			entity.m_id = atoi(pValue);
			vec.push_back(entity);
		}
	}
}

//-------------------------------------------------------------------------
u16 CStatsEntityIdRegistry::GetGameModeId( tukk pModeName ) const
{
	const IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pModeName);
	DRX_ASSERT_TRACE(pClass, ("Failed to find class '%s'", pModeName));
	return GetGameModeId(pClass);
}

//-------------------------------------------------------------------------
u16 CStatsEntityIdRegistry::GetGameModeId( const IEntityClass *pClass ) const
{
	i32 numModes = m_gameModes.size();
	for (i32 i = 0; i < numModes; ++ i)
	{
		if (m_gameModes[i].m_pClass == pClass)
		{
			return m_gameModes[i].m_id;
		}
	}
	return m_defaultGameMode;
}

//-------------------------------------------------------------------------
u16 CStatsEntityIdRegistry::GetMapId( tukk pMapName ) const
{
	i32 numMaps = m_maps.size();
	for (i32 i = 0; i < numMaps; ++ i)
	{
		if (!stricmp(m_maps[i].m_name.c_str(), pMapName))
		{
			return m_maps[i].m_id;
		}
	}
	return m_defaultMap;
}

//-------------------------------------------------------------------------
u16 CStatsEntityIdRegistry::GetWeaponId( tukk pWeaponName ) const
{
	if( const IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pWeaponName) )
	{
		return GetWeaponId( pClass );
	}
	else
	{
		i32 numWeaponExtensions = m_weaponExtensions.size();
		for( i32 i = 0; i < numWeaponExtensions; ++i )
		{
			if( !stricmp( m_weaponExtensions[i].m_name.c_str(), pWeaponName ) )
			{
				return m_weaponExtensions[i].m_id;
			}
		}
		return m_defaultWeapon;
	}
	
}

//-------------------------------------------------------------------------
u16 CStatsEntityIdRegistry::GetWeaponId( const IEntityClass *pClass ) const
{
	i32 numWeapons = m_weapons.size();
	for (i32 i = 0; i < numWeapons; ++ i)
	{
		if (m_weapons[i].m_pClass == pClass)
		{
			return m_weapons[i].m_id;
		}
	}
	return m_defaultWeapon;
}

//-------------------------------------------------------------------------
u16 CStatsEntityIdRegistry::GetHitTypeId( tukk pHitTypeName ) const
{
	i32 numHitTypes = m_hitTypes.size();
	for (i32 i = 0; i < numHitTypes; ++ i)
	{
		if (!stricmp(m_hitTypes[i].m_name.c_str(), pHitTypeName))
		{
			return m_hitTypes[i].m_id;
		}
	}
	return m_defaultHitType;
}

//-------------------------------------------------------------------------
tukk  CStatsEntityIdRegistry::GetGameMode( u16 id ) const
{
	i32 numModes = m_gameModes.size();
	for (i32 i = 0; i < numModes; ++ i)
	{
		if (m_gameModes[i].m_id == id)
		{
			return m_gameModes[i].m_pClass->GetName();
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------
tukk  CStatsEntityIdRegistry::GetMap( u16 id ) const
{
	i32 numMaps = m_maps.size();
	for (i32 i = 0; i < numMaps; ++ i)
	{
		if (m_maps[i].m_id == id)
		{
			return m_maps[i].m_name.c_str();
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------
tukk  CStatsEntityIdRegistry::GetWeapon( u16 id ) const
{
	i32 numWeapons = m_weapons.size();
	for( i32 i = 0; i < numWeapons; ++i )
	{
		if( m_weapons[i].m_id == id )
		{
			return m_weapons[i].m_pClass->GetName();
		}
	}

	//not in main weapons, try extensions
	numWeapons = m_weaponExtensions.size();
	for( i32 i = 0; i < numWeapons; ++i )
	{
		if( m_weaponExtensions[i].m_id == id )
		{
			return m_weaponExtensions[i].m_name.c_str();
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------
tukk CStatsEntityIdRegistry::GetHitType( u16 id ) const
{
	i32 numHitTypes = m_hitTypes.size();
	for (i32 i = 0; i < numHitTypes; ++ i)
	{
		if (m_hitTypes[i].m_id == id)
		{
			return m_hitTypes[i].m_name;
		}
	}
	return NULL;
}
