// Разработка 2018-2023 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 5:9:2005   14:55 : Created by M�rcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/ItemSharedParams.h>
#include <drx3D/Game/EntityUtility/EntityEffects.h>


//------------------------------------------------------------------------
void CItem::OnStartUsing()
{
	// Corpses have weapons with 'no save' flag set, restore it after start using
	GetEntity()->SetFlags( GetEntity()->GetFlags() & ~ENTITY_FLAG_NO_SAVE );
}

//------------------------------------------------------------------------
void CItem::OnStopUsing()
{
}

//------------------------------------------------------------------------
void CItem::OnSelect(bool select)
{
}

//------------------------------------------------------------------------
void CItem::OnSelected(bool selected)
{
	i32k numAccessories = m_accessories.size();

	//Let accessories know about it
	for (i32 i = 0; i < numAccessories; i++)
	{
		if (IItem* pAccessory = m_pItemSystem->GetItem(m_accessories[i].accessoryId))
		{
			pAccessory->OnParentSelect(selected);
		}
	}

	bool renderAlways = (IsOwnerFP() && !IsMounted() && selected);
	RegisterFPWeaponForRenderingAlways(renderAlways);
}

//------------------------------------------------------------------------
void CItem::OnReloaded()
{
	i32k numAccessories = m_accessories.size();
	for (i32 i = 0; i < numAccessories; i++)
	{
		CItem* pAccessory = static_cast<CItem*>(m_pItemSystem->GetItem(m_accessories[i].accessoryId));
		if (pAccessory)
			pAccessory->OnParentReloaded();
	}
}

//------------------------------------------------------------------------
void CItem::OnEnterFirstPerson()
{
	//Prevent FP model to show up when activating AI/Physics in editor
	if(gEnv->IsEditor() && gEnv->IsEditing())
		return;

	EnableUpdate(true, eIUS_General);
	SetViewMode(eIVM_FirstPerson);

	i32k numAccessories = m_accessories.size();

	//Inform accessories as well
	for (i32 i = 0; i < numAccessories; i++)
	{
		if (CItem* pAccessory = static_cast<CItem*>(m_pItemSystem->GetItem(m_accessories[i].accessoryId)))
		{
			pAccessory->OnEnterFirstPerson();
		}
	}
}

//------------------------------------------------------------------------
void CItem::OnEnterThirdPerson()
{
	SetViewMode(eIVM_ThirdPerson);

	i32k numAccessories = m_accessories.size();

	//Inform accessories as well
	for (i32 i = 0; i < numAccessories; i++)
	{
		if (CItem* pAccessory = static_cast<CItem*>(m_pItemSystem->GetItem(m_accessories[i].accessoryId)))
		{
			pAccessory->OnEnterThirdPerson();
		}
	}
}

//------------------------------------------------------------------------
void CItem::OnReset()
{
	//Hidden entities must have physics disabled
	if(!GetEntity()->IsHidden())
		GetEntity()->EnablePhysics(true);

	DestroyedGeometry(false);  
	m_stats.health = (float)m_properties.hitpoints;

	UpdateDamageLevel();

	if(m_sharedparams->params.scopeAttachment)
		DrawSlot(eIGS_Aux1,false); //Hide secondary FP scope

	if (m_properties.mounted && m_sharedparams->params.mountable)
		MountAt(GetEntity()->GetWorldPos());
	else
		SetViewMode(eIVM_ThirdPerson);

	if (m_properties.pickable)
	{
		const bool hasOwner = (GetOwnerId() != 0);
		DeferPhysicalize(hasOwner ? false : (m_properties.physics!=eIPhys_NotPhysicalized), (m_properties.physics==eIPhys_PhysicalizedRigid));
		
		Pickalize(!hasOwner, false);
	}
	else
		DeferPhysicalize((m_properties.physics!=eIPhys_NotPhysicalized), (m_properties.physics==eIPhys_PhysicalizedRigid));

	// Added to remove detonator (left hand 'attachToOwner' item) after a checkpoint
	// reload when it's equipped.
	i32k numAccessories = m_accessories.size();
	for (i32 i = 0; i < numAccessories; i++)
	{
		if (CItem* pItem = static_cast<CItem*>(m_pGameFramework->GetIItemSystem()->GetItem(m_accessories[i].accessoryId)))
		{
			const SAccessoryParams* params = GetAccessoryParams( m_accessories[i].pClass );

			ResetCharacterAttachment(eIGS_FirstPerson, params->attach_helper.c_str(), params->attachToOwner);
		}
	}

	GetEntity()->InvalidateTM();
}

//------------------------------------------------------------------------
void CItem::OnHit(float damage, i32 hitType)
{  
	if(!m_properties.hitpoints)
		return;

	static i32 repairHitType = g_pGame->GetGameRules()->GetHitTypeId("repair");

	if (hitType && hitType == repairHitType)
	{
		if (m_stats.health < m_properties.hitpoints) //repair only to maximum 
		{
			bool destroyed = m_stats.health<=0.f;
			m_stats.health = min(float(m_properties.hitpoints),m_stats.health+damage);

			UpdateDamageLevel();

			if(destroyed && m_stats.health>0.f)
				OnRepaired();
		}
	}
	else
	{
		if (m_stats.health > 0.0f)
		{ 
			m_stats.health -= damage;

			UpdateDamageLevel();

			if (m_stats.health <= 0.0f)
			{
				m_stats.health = 0.0f;
				OnDestroyed();
				
				i32 n=(i32)m_sharedparams->damageLevels.size();
				for (i32 i=0; i<n; ++i)
				{
					const SDamageLevel &level = m_sharedparams->damageLevels[i];
					if (level.min_health == 0 && level.max_health == 0)
					{
						i32 slot=(m_stats.viewmode&eIVM_FirstPerson)?eIGS_FirstPerson:eIGS_ThirdPerson;

						EntityEffects::SEffectSpawnParams spawnParams(ZERO, Vec3Constants<float>::fVec3_OneZ, level.scale, 0.f, false);
						EntityEffects::SpawnParticleWithEntity(GetEntity(), slot, level.effect.c_str(), level.helper.c_str(), spawnParams);
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------
void CItem::UpdateDamageLevel()
{
	if (m_properties.hitpoints<=0 || m_sharedparams->damageLevels.empty())
		return;

	i32 slot=(m_stats.viewmode&eIVM_FirstPerson)?eIGS_FirstPerson:eIGS_ThirdPerson;

	i32 n=(i32)m_sharedparams->damageLevels.size();
	i32 health=(i32)((100.0f*MAX(0.0f, m_stats.health))/m_properties.hitpoints);
	for (i32 i=0; i<n; ++i)
	{
		const SDamageLevel &level = m_sharedparams->damageLevels[i];
		if (level.min_health == 0 && level.max_health == 0)
			continue;

		if (level.min_health <= health && health < level.max_health)
		{
			if (m_damageLevelEffects[i] == -1)
				m_damageLevelEffects[i] = AttachEffect(slot, false, level.effect.c_str(), level.helper.c_str(), 
																									Vec3Constants<float>::fVec3_Zero, Vec3Constants<float>::fVec3_OneZ, 
																									level.scale, true);
		}
		else if (m_damageLevelEffects[i] != -1)
		{
			DetachEffect(m_damageLevelEffects[i]);
			m_damageLevelEffects[i] = -1;
		}
	}
}

//------------------------------------------------------------------------
void CItem::OnDestroyed()
{ 
  /* MR, 2007-02-09: shouldn't be needed 
	for (i32 i=0; i<eIGS_Last; i++)
	{
		ICharacterInstance *pCharacter = GetEntity()->GetCharacter(i);
		if (pCharacter)
			pCharacter->SetAnimationSpeed(0);
	}*/

	DestroyedGeometry(true);

	if(!gEnv->pSystem->IsSerializingFile()) //don't replay destroy animations/effects
		PlayAction(GetFragmentIds().destroy);

	EnableUpdate(false);
}

//------------------------------------------------------------------------
void CItem::OnRepaired()
{
	for (i32 i=0; i<eIGS_Last; i++)
	{
		ICharacterInstance *pCharacter = GetEntity()->GetCharacter(i);
		if (pCharacter)
			pCharacter->SetPlaybackScale(1.0f);
	}

	DestroyedGeometry(false);

	EnableUpdate(true);
}

//------------------------------------------------------------------------
void CItem::OnDropped(EntityId actorId, bool ownerWasAI)
{
	UnRegisterAsUser();

	m_pItemSystem->RegisterForCollection(GetEntityId());
}

//------------------------------------------------------------------------
void CItem::OnPickedUp(EntityId actorId, bool destroyed)
{
	if(GetISystem()->IsSerializingFile() == 1)
		return;

	CActor *pActor=GetActor(actorId);
	if (!pActor)
		return;

	RegisterAsUser();

	if (!IsServer())
		return;

	// Corpses have weapons with 'no save' flag set, restore it after pick up
	GetEntity()->SetFlags( GetEntity()->GetFlags() & ~ENTITY_FLAG_NO_SAVE );

	//Once picked up, remove it from bound layer 
	//Prevents hide/unload with the level layer, when the player goes to a different part of the level
	bool removeEntityFromLayer = pActor->IsPlayer();
	if (removeEntityFromLayer)
	{
		gEnv->pEntitySystem->RemoveEntityFromLayers(GetEntityId());
	}

	m_pItemSystem->UnregisterForCollection(GetEntityId());
}

//------------------------------------------------------------------------
void CItem::OnBeginCutScene()
{
}

//------------------------------------------------------------------------
void CItem::OnEndCutScene()
{
}

//------------------------------------------------------------------------
void CItem::OnOwnerActivated()
{
	RegisterAsUser();
}

//------------------------------------------------------------------------
void CItem::OnOwnerDeactivated()
{
	UnRegisterAsUser();
}

void CItem::OnOwnerStanceChanged( const EStance stance )
{

}
