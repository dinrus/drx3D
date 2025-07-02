// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: 

Ltag version of single firemode. Requires some extra functionality
for animation, projectile spawning, etc

-------------------------------------------------------------------------
История:
- 15:09:09   Benito Gangoso Rodriguez

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/LTagSingle.h>

#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/WeaponSharedParams.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/WeaponSystem.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DRX_IMPLEMENT_GTI(CLTagSingle, CSingle);

struct CLTagSingle::EndCockingAction
{
public:
	EndCockingAction(CLTagSingle *_pLtagSingle): m_pLTagSingle(_pLtagSingle) {};

	void execute(CItem *item) 
	{
		m_pLTagSingle->m_cocking = false;
	}

private:
	CLTagSingle *m_pLTagSingle;
};


struct CLTagSingle::ScheduleReload
{
public:
	ScheduleReload(CWeapon *pWeapon)
	{
		m_pWeapon = pWeapon;
	}
	void execute(CItem *item) 
	{
		m_pWeapon->SetBusy(false);
		m_pWeapon->OnFireWhenOutOfAmmo();
	}
private:
	CWeapon* m_pWeapon;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CLTagSingle::CLTagSingle()
: m_grenadeType((ELTAGGrenadeType)0)
, m_fpModelInitialised(false)
{

}

CLTagSingle::~CLTagSingle()
{

}

void CLTagSingle::Activate(bool activate)
{
	Parent::Activate(activate);

	//Ensure correct grenade type is selected and the attachments are updated first and third person
	if(activate)
	{
		if(m_fireParams->lTagGrenades.grenades[m_grenadeType].c_str()[0] == '\0')
		{
			NextGrenadeType();
		}
		else
		{
			const ItemString& grenadeModel = m_fireParams->lTagGrenades.grenades[m_grenadeType];
			if (!grenadeModel.empty())
			{
				if (ICharacterInstance* pCharacterTP = m_pWeapon->GetEntity()->GetCharacter(eIGS_ThirdPerson))
				{
					UpdateGrenadeAttachment(pCharacterTP, "currentshell", grenadeModel.c_str());
					UpdateGrenadeAttachment(pCharacterTP, "newshell", grenadeModel.c_str());
				}
			}
		}
	}
	else
	{
		//the first person model is not necessarily available at this point so defer to updateFPView
		m_fpModelInitialised = false;
	}
}

void CLTagSingle::UpdateFPView(float frameTime)
{
	Parent::UpdateFPView(frameTime);

	if(!m_fpModelInitialised)
	{
		const ItemString& grenadeModel = m_fireParams->lTagGrenades.grenades[m_grenadeType];
		if (!grenadeModel.empty())
		{
			if (ICharacterInstance* pCharacterFP = m_pWeapon->GetEntity()->GetCharacter(eIGS_FirstPerson))
			{
				UpdateGrenadeAttachment(pCharacterFP, "currentshell", grenadeModel.c_str());
				UpdateGrenadeAttachment(pCharacterFP, "newshell", grenadeModel.c_str());
				m_fpModelInitialised = true;
			}
		}
	}
}

void CLTagSingle::OnShoot(EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3 &pos, const Vec3 &dir, const Vec3 &vel)
{
	Parent::OnShoot(shooterId, ammoId, pAmmoType, pos, dir, vel);

	CProjectile* pProjectile = g_pGame->GetWeaponSystem()->GetProjectile(ammoId);
	CLTAGGrenade* pLtagGrenade = crygti_cast<CLTAGGrenade*>(pProjectile);
	if (pLtagGrenade)
		pLtagGrenade->SetGrenadeType(m_grenadeType);
}

bool CLTagSingle::NextGrenadeType()
{
	u32 grenadeType = (m_grenadeType + 1) % ELTAGGrenadeType_LAST;

	while (grenadeType != m_grenadeType)
	{
		assert(grenadeType < ELTAGGrenadeType_LAST);

		if (m_fireParams->lTagGrenades.grenades[grenadeType].c_str()[0] != '\0')
		{
			m_grenadeType = static_cast<ELTAGGrenadeType>(grenadeType);
			m_pWeapon->PlayAction(GetFragmentIds().change_firemode);
			SwitchGrenades();
		
			//m_pWeapon->GetGameObject()->ChangedNetworkState(eEA_GameClientStatic);

			return true;
		}

		if (++grenadeType >= ELTAGGrenadeType_LAST)
		{
			grenadeType = static_cast<ELTAGGrenadeType>(0);
		}
	}

	return false;
}

const ELTAGGrenadeType CLTagSingle::GetCurrentGrenadeType() const
{
	return m_grenadeType;
}

void CLTagSingle::SetProjectileLaunchParams(const SProjectileLaunchParams &launchParams)
{
	SetProjectileSpeedScale(launchParams.fSpeedScale);
}

void CLTagSingle::SwitchGrenades()
{
	CActor* pOwner = m_pWeapon->GetOwnerActor();
	if (pOwner == NULL)
		return;

	const ItemString& grenadeModel = m_fireParams->lTagGrenades.grenades[m_grenadeType];
	if (grenadeModel.empty())
		return;

	if (ICharacterInstance* pCharacterTP = m_pWeapon->GetEntity()->GetCharacter(eIGS_ThirdPerson))
	{
		UpdateGrenadeAttachment(pCharacterTP, "currentshell", grenadeModel.c_str());
		UpdateGrenadeAttachment(pCharacterTP, "newshell", grenadeModel.c_str());
	}

	if (!pOwner->IsClient())
		return;

	if (ICharacterInstance* pCharacterFP = m_pWeapon->GetEntity()->GetCharacter(eIGS_FirstPerson))
	{
		UpdateGrenadeAttachment(pCharacterFP, "currentshell", grenadeModel.c_str());
		UpdateGrenadeAttachment(pCharacterFP, "newshell", grenadeModel.c_str());
	}
}

void CLTagSingle::UpdateGrenadeAttachment( ICharacterInstance* pCharacter, tukk attachmentName, tukk model )
{
	DRX_ASSERT(pCharacter);

	IAttachment* pAttachment = pCharacter->GetIAttachmentUpr()->GetInterfaceByName(attachmentName);
	if (pAttachment == NULL)
		return;

	IStatObj* pGrenadeModel = gEnv->p3DEngine->LoadStatObj(model);
	if (pGrenadeModel == NULL)
		return;

	IAttachmentObject* pAttachmentObject = pAttachment->GetIAttachmentObject();
	if (pAttachmentObject && (pAttachmentObject->GetAttachmentType() == IAttachmentObject::eAttachment_StatObj))
	{
		CCGFAttachment* pCGFAttachment = static_cast<CCGFAttachment*>(pAttachmentObject);
		pCGFAttachment->pObj = pGrenadeModel;  //pObj is an smart pointer, it should take care of releasing/AddRef
	}
	else
	{
		CCGFAttachment* pCGFAttachment = new CCGFAttachment();
		pCGFAttachment->pObj = pGrenadeModel;
		pAttachment->AddBinding(pCGFAttachment);
	}
}

void CLTagSingle::SetReloadFragmentTags(CTagState& fragTags, i32 ammoCount)
{
	if(ammoCount <= 1)
	{
		i32 inventoryCount = m_pWeapon->GetInventoryAmmoCount(GetAmmoType());
		TagID ammoCountID = TAG_ID_INVALID;
		
		switch(inventoryCount)
		{
		case 1:
			ammoCountID = fragTags.GetDef().Find(CItem::sFragmentTagCRCs.inventory_last1);
			break;

		case 2:
			ammoCountID = fragTags.GetDef().Find(CItem::sFragmentTagCRCs.inventory_last2);
			break;
		}

		if(ammoCountID != TAG_ID_INVALID)
		{
			fragTags.Set(ammoCountID, true);
		}
	}

	Parent::SetReloadFragmentTags(fragTags, ammoCount);
}

void CLTagSingle::NetShootEx(const Vec3 &pos, const Vec3 &dir, const Vec3 &vel, const Vec3 &hit, float extra, i32 predictionHandle)
{
	IEntityClass* ammo = m_fireParams->fireparams.ammo_type_class;
	i32 weaponAmmoCount = m_pWeapon->GetAmmoCount(ammo);
	i32 ammoCount = weaponAmmoCount;

	CActor* pOwnerActor = m_pWeapon->GetOwnerActor();
	bool playerIsShooter = pOwnerActor ? pOwnerActor->IsPlayer() : false;

	if (IsProceduralRecoilEnabled() && pOwnerActor)
	{
		pOwnerActor->ProceduralRecoil(m_fireParams->proceduralRecoilParams.duration, m_fireParams->proceduralRecoilParams.strength, m_fireParams->proceduralRecoilParams.kickIn, m_fireParams->proceduralRecoilParams.arms);
	}

	m_pWeapon->PlayAction(GetFragmentIds().fire, 0, false, CItem::eIPAF_Default);
	
	CProjectile *pAmmo = m_pWeapon->SpawnAmmo(m_fireParams->fireparams.spawn_ammo_class, true);
	CLTAGGrenade* pGrenade = static_cast<CLTAGGrenade*>(pAmmo);

	if (pGrenade)
	{
		DRX_ASSERT_MESSAGE(m_fireParams->fireparams.hitTypeId, string().Format("Invalid hit type '%s' in fire params for '%s'", m_fireParams->fireparams.hit_type.c_str(), m_pWeapon->GetEntity()->GetName()));
		DRX_ASSERT_MESSAGE(m_fireParams->fireparams.hitTypeId == g_pGame->GetGameRules()->GetHitTypeId(m_fireParams->fireparams.hit_type.c_str()), "Sanity Check Failed: Stored hit type id does not match the type string, possibly CacheResources wasn't called on this weapon type");

		pGrenade->SetParams(CProjectile::SProjectileDesc(
			m_pWeapon->GetOwnerId(), m_pWeapon->GetHostId(), m_pWeapon->GetEntityId(), 
			m_fireParams->fireparams.damage, 0.f, 0.f, 0.f, m_fireParams->fireparams.hitTypeId,
			m_fireParams->fireparams.bullet_pierceability_modifier, m_pWeapon->IsZoomed()));
		// this must be done after owner is set
		pGrenade->InitWithAI();
		pGrenade->SetDestination(m_pWeapon->GetDestination());
		pGrenade->SetGrenadeType(m_grenadeType);
		pGrenade->Launch(pos, dir, vel, m_speed_scale);

		m_projectileId = pGrenade->GetEntity()->GetId();
	}

	if (m_pWeapon->IsServer())
	{
		tukk ammoName = ammo != NULL ? ammo->GetName() : NULL;
		g_pGame->GetIGameFramework()->GetIGameplayRecorder()->Event(m_pWeapon->GetOwner(), GameplayEvent(eGE_WeaponShot, ammoName, 1, (uk )(EXPAND_PTR)m_pWeapon->GetEntityId()));
	}

	m_muzzleEffect.Shoot(this, hit, m_barrelId);
	RecoilImpulse(pos, dir);

	m_fired = true;
	m_next_shot = 0.0f;

	ammoCount--;

	if(ammoCount<0)
		ammoCount = 0;

	if (m_pWeapon->IsServer())
	{
		i32 clipSize = GetClipSize();
		if (clipSize != -1)
		{
			if (clipSize != 0)
				m_pWeapon->SetAmmoCount(ammo, ammoCount);
			else
				m_pWeapon->SetInventoryAmmoCount(ammo, ammoCount);
		}
	}

	OnShoot(m_pWeapon->GetOwnerId(), pAmmo?pAmmo->GetEntity()->GetId():0, ammo, pos, dir, vel);

	if(playerIsShooter)
	{
		const SThermalVisionParams& thermalParams = m_fireParams->thermalVisionParams;
		m_pWeapon->AddShootHeatPulse(pOwnerActor, thermalParams.weapon_shootHeatPulse, thermalParams.weapon_shootHeatPulseTime,
			thermalParams.owner_shootHeatPulse, thermalParams.owner_shootHeatPulseTime);
	}

	if (OutOfAmmo())
		m_pWeapon->OnOutOfAmmo(ammo);

	if (pAmmo && predictionHandle && pOwnerActor)
	{
		pAmmo->GetGameObject()->RegisterAsValidated(pOwnerActor->GetGameObject(), predictionHandle);
		pAmmo->GetGameObject()->BindToNetwork();
	}
	else if (pAmmo && pAmmo->IsPredicted() && gEnv->IsClient() && gEnv->bServer && pOwnerActor && pOwnerActor->IsClient())
	{
		pAmmo->GetGameObject()->BindToNetwork();
	}

	m_pWeapon->RequireUpdate(eIUS_FireMode);
}

