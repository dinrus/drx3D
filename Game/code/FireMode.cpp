// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Basic Fire Mode Implementation

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/FireMode.h>

#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/WeaponSystem.h>
#include <drx3D/Game/FireModePlugin.h>
#include <drx3D/Game/Actor.h>

#include <drx3D/Game/Player.h>

DRX_IMPLEMENT_GTI_BASE(CFireMode);

CFireMode::CFireMode() 
: m_pWeapon(NULL)
, m_fireParams(NULL)
, m_parentFireParams(NULL)
, m_enabled(true)
, m_accessoryEnabled(false)
, m_listeners(5)
{

}

CFireMode::~CFireMode()
{
	if(g_pGame)
	{
		CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem();

		i32k numPlugins = m_plugins.size();

		for (i32 i = 0; i < numPlugins; i++)
		{
			m_plugins[i]->Activate(false);
			pWeaponSystem->DestroyFireModePlugin(m_plugins[i]);
		}

		m_plugins.clear();

		for (TFireModeListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
		{
			notifier->OnFireModeDeleted();
		}
		m_listeners.Clear();
	}
}

void CFireMode::InitFireMode( IWeapon* pWeapon, const SParentFireModeParams* pParams)
{
	DRX_ASSERT_MESSAGE(pParams, "Fire Mode Params NULL! Have you set up the weapon xml correctly?");
	DRX_ASSERT_MESSAGE(pParams->pBaseFireMode, "Fire Mode Base Params NULL!");

	m_pWeapon = static_cast<CWeapon *>(pWeapon);
	m_fireParams = pParams->pBaseFireMode;
	m_parentFireParams = pParams;

	ResetParams();
}

void CFireMode::Release()
{
	if (g_pGame)
	{
		g_pGame->GetWeaponSystem()->DestroyFireMode(this);
	}
}

void CFireMode::ResetParams()
{
	CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem();
	i32k numPluginParams = m_fireParams->pluginParams.size();

	TFireModePluginVector currentPlugins = m_plugins;

	m_plugins.clear();
	m_plugins.reserve(numPluginParams);

	for(i32 i = 0; i < numPluginParams; i++)
	{
		if(IFireModePluginParams* pPluginParams = m_fireParams->pluginParams[i])
		{
			IFireModePlugin* pPlugin = NULL;
			const CGameTypeInfo* pluginType = pPluginParams->GetPluginType();

			i32k currentNumPlugins = currentPlugins.size();

			for(i32 a = 0; a < currentNumPlugins; a++)
			{
				if(currentPlugins[a]->GetRunTimeType() == pluginType)
				{
					pPlugin = currentPlugins[a];

					currentPlugins.erase(currentPlugins.begin() + a);
					break;
				}
			}

			if(!pPlugin)
			{
				pPlugin = pWeaponSystem->CreateFireModePlugin(pluginType);
			}

			if(pPlugin && pPlugin->Init(this, pPluginParams))
			{
				m_plugins.push_back(pPlugin);
			}
			else
			{
				GameWarning("Firemode Plugin creation and initialisation failed. This weapon may be missing functionality");
				
				if(pPlugin)
				{
					pWeaponSystem->DestroyFireModePlugin(pPlugin);
				}
			}
		}
	}

	i32k numToDelete = currentPlugins.size();

	for(i32 i = 0; i < numToDelete; i++)
	{
		currentPlugins[i]->Activate(false);
		pWeaponSystem->DestroyFireModePlugin(currentPlugins[i]);
	}
}

void CFireMode::Update(float frameTime, u32 frameId)
{
	bool keepUpdating = false;

	i32k numPlugins = m_plugins.size();

	for (i32 i = 0; i < numPlugins; i++)
	{
		keepUpdating |= m_plugins[i]->Update(frameTime, frameId);
	}

	if (keepUpdating)
	{
		m_pWeapon->RequireUpdate(eIUS_FireMode);
	}
}

void CFireMode::Activate(bool activate)
{
	i32k numPlugins = m_plugins.size();

	for (i32 i = 0; i < numPlugins; i++)
	{
		m_plugins[i]->Activate(activate);
	}

	for (TFireModeListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnFireModeActivated(activate);
	}

	if (GetWeapon()->GetOwnerId())
	{
		if (m_pWeapon->IsSelected())
		{
			UpdateMannequinTags(activate);
		}
	}
	else
	{
		UpdateMannequinTags(activate);
	}
}

void CFireMode::UpdateMannequinTags(bool enable)
{
	IActionController* pActionController = m_pWeapon->GetActionController();
	if (pActionController)
	{
		u32k firemodeCrC = CCrc32::ComputeLowercase(m_fireParams->fireparams.tag.c_str());
		if (firemodeCrC)
		{
			SAnimationContext& animationContext = pActionController->GetContext();
			animationContext.state.SetByCRC(firemodeCrC, enable);
		}
	}
}

void CFireMode::PatchSpreadMod( const SSpreadModParams &sSMP, float modMultiplier )
{

}

void CFireMode::ResetSpreadMod()
{

}

void CFireMode::PatchRecoilMod( const SRecoilModParams &sRMP, float modMultiplier )
{

}

void CFireMode::ResetRecoilMod()
{

}

bool CFireMode::CanZoom() const
{
	return true;
}

float CFireMode::GetRange() const
{
	return 0.f;
}

bool CFireMode::FillAmmo(bool fromInventory)
{
	return false;
}

i32 CFireMode::GetClipSize() const
{
	return 0;
}

i32 CFireMode::GetChamberSize() const
{
	return 0;
}

void CFireMode::OnEnterFirstPerson()
{

}

void CFireMode::GetMemoryUsage( IDrxSizer * pSizer ) const
{
	pSizer->AddObject(this, sizeof(*this));
	GetInternalMemoryUsage(pSizer);
}

void CFireMode::GetInternalMemoryUsage( IDrxSizer * pSizer ) const
{
	pSizer->AddObject(m_pWeapon);
	pSizer->AddObject(m_fireParams);
	pSizer->AddObject(m_parentFireParams);
	pSizer->AddObject(m_name);
}

void CFireMode::SetProjectileSpeedScale( float fSpeedScale )
{
}

bool CFireMode::PluginsAllowFire() const
{
	i32k numPlugins = m_plugins.size();

	for (i32 i = 0; i < numPlugins; i++)
	{
		if(!m_plugins[i]->AllowFire())
		{
			return false;
		}
	}

	return true;
}

void CFireMode::OnShoot(EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel)
{
	m_pWeapon->OnShoot(shooterId, ammoId, pAmmoType, pos, dir, vel);

	i32k numPlugins = m_plugins.size();

	for (i32 i = 0; i < numPlugins; i++)
	{
		m_plugins[i]->OnShoot();
	}
}

void CFireMode::OnReplayShoot()
{
	i32k numPlugins = m_plugins.size();

	for (i32 i = 0; i < numPlugins; i++)
	{
		m_plugins[i]->OnReplayShoot();
	}
}


void CFireMode::AlterFiringDirection(const Vec3& firingPos, Vec3& rFiringDir) const
{
	i32k numPlugins = m_plugins.size();
	for (i32 i = 0; i < numPlugins; i++)
	{
		m_plugins[i]->AlterFiringDirection(firingPos, rFiringDir);
	}
}

void CFireMode::OnSpawnProjectile(CProjectile* pAmmo)
{
}

const IFireModePlugin* CFireMode::FindPluginOfType(const CGameTypeInfo* pluginType) const
{
	i32k numPlugins = m_plugins.size();

	for (i32 i = 0; i < numPlugins; i++)
	{
		if(m_plugins[i]->GetRunTimeType() == pluginType)
		{
			return m_plugins[i];
		}
	}

	return NULL;
}

bool CFireMode::CanOverheat() const
{
	return (FindPluginOfType(CFireModePlugin_Overheat::GetStaticType()) != NULL);
}

float CFireMode::GetHeat() const
{
	const CFireModePlugin_Overheat* pHeatPlugin = crygti_cast<const CFireModePlugin_Overheat*>(FindPluginOfType(CFireModePlugin_Overheat::GetStaticType()));

	if(pHeatPlugin)
	{
		return pHeatPlugin->GetHeat();
	}

	return 0.f;
}

float CFireMode::GetTimeBetweenShotsMultiplier(const CPlayer* pPlayer) const
{
	if(gEnv->bMultiplayer)
	{
		return pPlayer->GetModifiableValues().GetValue(kPMV_WeaponTimeBetweenShotsMultiplier);
	}

	return 1.f;
}
