// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Game/Actor.h>

#include <drx3D/Game/HandGrenades.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Game/GameActions.h>
#include <drx3D/Game/Throw.h>
#include <drx3D/Game/LTagSingle.h>

//-------------------------------------------------
CHandGrenades::CHandGrenades()
	: m_pThrow(nullptr)
	, m_numStowedCopies(0)
	, m_stowSlot(-1)
	, m_quickThrowRequested(false)
	, m_bInQuickThrow(false)
	, m_throwCancelled(false)
{
}

//-------------------------------------------------
CHandGrenades::~CHandGrenades()
{
}

//-------------------------------------------------
void CHandGrenades::InitFireModes()
{
	inherited::InitFireModes();

	i32 firemodeCount = m_firemodes.size();

	m_pThrow = NULL;
	i32 throwId = -1;

	for(i32 i = 0; i < firemodeCount; i++)
	{
		if (crygti_isof<CThrow>(m_firemodes[i]))
		{
			DRX_ASSERT_MESSAGE(!m_pThrow, "Multiple Throw firemodes assigned to weapon");
			m_pThrow = crygti_cast<CThrow*>(m_firemodes[i]);
			throwId = i;
		}
	}

	DRX_ASSERT_MESSAGE(m_pThrow, "No Throw firemode assigned to weapon");

	if(m_pThrow)
	{
		SetCurrentFireMode(throwId);
	}
}

//-------------------------------------------------
bool CHandGrenades::OnActionAttack(EntityId actorId, const ActionId& actionId, i32 activationMode, float value)
{
	m_throwCancelled &= (activationMode & (eAAM_OnRelease | eAAM_OnPress)) == 0; //New throw request so disable the cancellation

	if (!CanOwnerThrowGrenade() || m_throwCancelled)
		return true;

	if(activationMode == eAAM_OnPress || (gEnv->bMultiplayer && activationMode == eAAM_OnHold))
	{
		if(m_pThrow && !m_pThrow->IsFiring())
		{
			DRX_ASSERT_MESSAGE(m_pThrow == m_fm, "Currently not in throw firemode");
			m_pThrow->Prime();
			UpdateStowedWeapons();
		}
	}
	else if (activationMode == eAAM_OnRelease)
	{
		StopFire();
	}

	return true;
}

//-------------------------------------------------
bool CHandGrenades::CanSelect() const
{
	return (inherited::CanSelect() && !OutOfAmmo(false));
}

//-------------------------------------------------
bool CHandGrenades::CanDeselect() const
{
	if (gEnv->bMultiplayer)
	{
		return (!m_fm || !m_fm->IsFiring());
	}
	else
	{
		return inherited::CanDeselect();
	}
}

void CHandGrenades::StartSprint(CActor* pOwnerActor)
{
	//don't stop firing grenades
}

bool CHandGrenades::ShouldSendOnShootHUDEvent() const
{
	return false;
}

void CHandGrenades::OnPickedUp(EntityId actorId, bool destroyed)
{
	inherited::OnPickedUp(actorId, destroyed);
	UpdateStowedWeapons();
}

void CHandGrenades::OnDropped(EntityId actorId, bool ownerWasAI)
{
	inherited::OnDropped(actorId, ownerWasAI);
	UpdateStowedWeapons();
}

void CHandGrenades::OnSetAmmoCount(EntityId shooterId)
{
	inherited::OnSetAmmoCount(shooterId);
	UpdateStowedWeapons();
}

void CHandGrenades::OnSelected(bool selected)
{
	inherited::OnSelected(selected);
	UpdateStowedWeapons();
}

i32 PickStowSlot(IAttachmentUpr *pAttachmentUpr, tukk slot1, tukk slot2)
{
	i32 ret = -1;
	IAttachment *pAttachment = pAttachmentUpr->GetInterfaceByName(slot1);
	if(pAttachment && !pAttachment->GetIAttachmentObject())
	{
		ret = 0;
	}
	else
	{
		pAttachment = pAttachmentUpr->GetInterfaceByName(slot2);
		if(pAttachment && !pAttachment->GetIAttachmentObject())
		{
			ret = 1;
		}
	}

	return ret;
}

void CHandGrenades::UpdateStowedWeapons()
{
	CActor *pOwnerActor = GetOwnerActor();
	if (!pOwnerActor)
		return;

	ICharacterInstance *pOwnerCharacter = pOwnerActor->GetEntity()->GetCharacter(0);
	if (!pOwnerCharacter)
		return;

	IStatObj *pTPObj = GetEntity()->GetStatObj(eIGS_ThirdPerson);
	if (!pTPObj)
		return;


	i32 ammoCount = m_fm ? pOwnerActor->GetInventory()->GetAmmoCount(m_fm->GetAmmoType()) : 0;
	if (IsSelected() && (ammoCount > 0))
	{
		ammoCount--;
	}
	if (!pOwnerActor->IsThirdPerson())
	{
		ammoCount = 0;
	}

	i32 numGrenDiff = ammoCount - m_numStowedCopies;
	if(numGrenDiff != 0)
	{
		if (m_stowSlot < 0)
		{
			m_stowSlot = PickStowSlot(pOwnerCharacter->GetIAttachmentUpr(), m_sharedparams->params.bone_attachment_01.c_str(), m_sharedparams->params.bone_attachment_02.c_str());
		}

		if (m_stowSlot >= 0)
		{
			bool attach = numGrenDiff > 0;
			i32 tot = abs(numGrenDiff);

			IAttachmentUpr *pAttachmentUpr = pOwnerCharacter->GetIAttachmentUpr();
			IAttachment *pAttachment = NULL;

			for (i32 i=0; i<tot; i++)
			{
				//--- Generate the secondary slot from the first by adding one to the attachment name, is all we need at present...
				tukk attach1 = (m_stowSlot == 0) ? m_sharedparams->params.bone_attachment_01.c_str() : m_sharedparams->params.bone_attachment_02.c_str();
				i32 lenAttachName = strlen(attach1);
				stack_string attach2(attach1, lenAttachName-1);
				attach2 += (attach1[lenAttachName-1]+1);

				if (attach)
				{
					pAttachment = pAttachmentUpr->GetInterfaceByName(attach1);
					if(pAttachment && pAttachment->GetIAttachmentObject())
					{
						pAttachment = pAttachmentUpr->GetInterfaceByName(attach2);
					}

					if (pAttachment && !pAttachment->GetIAttachmentObject())
					{
						CCGFAttachment *pCGFAttachment = new CCGFAttachment();
						pCGFAttachment->pObj = pTPObj;
						pAttachment->AddBinding(pCGFAttachment);
						pAttachment->HideAttachment(0);
						pAttachment->HideInShadow(0);

						m_numStowedCopies++;
					}
				}
				else
				{
					pAttachment = pAttachmentUpr->GetInterfaceByName(attach2);
					if(!pAttachment || !pAttachment->GetIAttachmentObject())
					{
						pAttachment = pAttachmentUpr->GetInterfaceByName(attach1);
					}

					if (pAttachment && pAttachment->GetIAttachmentObject())
					{
						pAttachment->ClearBinding();
						m_numStowedCopies--;					
					}
				}
			}
		}
	}
}

bool CHandGrenades::CanOwnerThrowGrenade() const
{
	CActor* pOwnerActor = GetOwnerActor();

	return pOwnerActor ? pOwnerActor->CanFire() : true;
}

void CHandGrenades::FumbleGrenade()
{
	if(m_pThrow)
	{
		DRX_ASSERT_MESSAGE(m_pThrow == m_fm, "Currently not in throw firemode");
		m_pThrow->FumbleGrenade();
	}
}

bool CHandGrenades::AllowInteraction(EntityId interactionEntity, EInteractionType interactionType)
{
	return (interactionType == eInteraction_PickupAmmo) || (inherited::AllowInteraction(interactionEntity, interactionType) && !(m_fm && m_fm->IsFiring()));
}

void CHandGrenades::StartQuickGrenadeThrow()
{
	if (!CanOwnerThrowGrenade())
		return;

	m_quickThrowRequested = true;
	m_bInQuickThrow = true;

	if (m_pThrow && !m_pThrow->IsFiring())
	{
		DRX_ASSERT_MESSAGE(m_pThrow == m_fm, "Currently not in throw firemode");
		m_pThrow->Prime();
		UpdateStowedWeapons();

		if (m_pThrow->IsReadyToThrow())
		{
			m_quickThrowRequested = false;
		}
	}
}

void CHandGrenades::StopQuickGrenadeThrow()
{
	if (m_bInQuickThrow)
	{
		m_bInQuickThrow = false;
		StopFire();
	}
}

void CHandGrenades::ForcePendingActions( u8 blockedActions /*= 0*/ )
{
	inherited::ForcePendingActions(blockedActions);

	if (m_quickThrowRequested)
	{
		bool bDoCompleteThrow = !m_bInQuickThrow;

		StartQuickGrenadeThrow();

		if (bDoCompleteThrow)
		{
			StopQuickGrenadeThrow();
		}
	}
}

bool CHandGrenades::CancelCharge()
{
	if(gEnv->bMultiplayer)
	{
		m_throwCancelled = true;
		return true;
	}

	return false;
}

u32 CHandGrenades::StartDeselection(bool fastDeselect)
{
	if (m_zm)
		m_zm->StopZoom();

	return CWeapon::StartDeselection(fastDeselect);
}
