// Разработка 2018-2023 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 30:8:2005   12:33 : Created by M�rcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/Game.h>

//------------------------------------------------------------------------
void CItem::ReadProperties(IScriptTable *pProperties)
{
	if (pProperties)
	{
		GetEntityProperty("HitPoints", m_properties.hitpoints);
		GetEntityProperty("bPickable", m_properties.pickable);
		GetEntityProperty("bMounted", m_properties.mounted);
		GetEntityProperty("bUsable", m_properties.usable);

		GetEntityProperty("Respawn", "bRespawn", m_respawnprops.respawn);
		GetEntityProperty("Respawn", "nTimer", m_respawnprops.timer);
		GetEntityProperty("Respawn", "bUnique", m_respawnprops.unique);

		i32 physicsTemp;
		GetEntityProperty("eiPhysicsType", physicsTemp);
		m_properties.physics = (ePhysicalization)physicsTemp;

		if(!gEnv->bMultiplayer)
		{
			GetEntityProperty("bSpecialSelect", m_properties.specialSelect);

			ReadMountedProperties(pProperties);
		}
	}
}

//------------------------------------------------------------------------
void CItem::ReadMountedProperties(IScriptTable* pScriptTable)
{
	float minPitch = 0.f;
	float maxPitch = 0.f;

	GetEntityProperty("MountedLimits", "pitchMin", minPitch);
	GetEntityProperty("MountedLimits", "pitchMax", maxPitch);
	GetEntityProperty("MountedLimits", "yaw", m_properties.mounted_yaw_range);

	m_properties.mounted_min_pitch = min(minPitch,maxPitch);
	m_properties.mounted_max_pitch = max(minPitch,maxPitch);

	GetEntityProperty("Respawn", "bRespawn", m_respawnprops.respawn);
	GetEntityProperty("Respawn", "nTimer", m_respawnprops.timer);
	GetEntityProperty("Respawn", "bUnique", m_respawnprops.unique);
}

//------------------------------------------------------------------------
void CItem::InitItemFromParams()
{
	InitGeometry();
	InitAccessories();
	InitDamageLevels();
}

//------------------------------------------------------------------------
void CItem::InitGeometry()
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);

	//skip loading the first person geometry for now, it may never be used
	m_sharedparams->LoadGeometryForItem(this, eIGS_FirstPerson);
}

//-----------------------------------------------------------------------
void CItem::InitAccessories()
{
	m_initialSetup = m_sharedparams->initialSetup;
}

//-----------------------------------------------------------------------
void CItem::InitDamageLevels()
{
	i32 numLevels = m_sharedparams->damageLevels.size();

	m_damageLevelEffects.resize(numLevels);

	for(i32 i = 0; i < numLevels; i++)
	{
		m_damageLevelEffects[i] = -1;
	}
}