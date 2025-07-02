// Разработка 2018-2023 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
История:
- 7:9:2005   11:24 : Created by M�rcio Martins

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/CoreX/ParticleSys/ParticleParams.h>

#include <drx3D/Game/ItemSharedParams.h>

//------------------------------------------------------------------------
EntityEffects::TAttachedEffectId CItem::AttachEffect(i32 slot, bool attachToAccessory, tukk effectName, tukk helper, const Vec3 &offset, const Vec3 &dir, float scale, bool prime)
{
	if(!g_pGameCVars->i_particleeffects)
	{
		return 0;
	}
	
	Vec3 finalOffset(offset);
	string helperName(helper);

	if(attachToAccessory)
	{
		SEntitySlotInfo slotInfo;
		QuatT accessoryOffset;

		accessoryOffset.SetIdentity();
		tukk accessoryHelper = "";
		tukk accessoryName = NULL;

		i32k numAccessories = m_accessories.size();

		for (i32 curIndex = 0; curIndex < numAccessories; curIndex++)
		{
			IEntity* pAccessory = gEnv->pEntitySystem->GetEntity(m_accessories[curIndex].accessoryId);

			if(pAccessory && pAccessory->GetSlotInfo(slot, slotInfo))
			{
				if(slotInfo.pStatObj)
				{
					accessoryOffset.t = slotInfo.pStatObj->GetHelperPos(helper);

					if(!accessoryOffset.t.IsZero())
					{
						accessoryOffset.q = pAccessory->GetRotation();
						accessoryOffset.t += pAccessory->GetPos();

						accessoryName = m_accessories[curIndex].pClass->GetName();
						break;
					}
				}

				if(slotInfo.pCharacter)
				{
					IAttachmentUpr *pAttachmentUpr = slotInfo.pCharacter->GetIAttachmentUpr();
					IAttachment *pAttachment = pAttachmentUpr->GetInterfaceByName(helper);

					if(pAttachment)
					{
						accessoryHelper = GetAccessoryParams(m_accessories[curIndex].pClass)->attach_helper.c_str();
						accessoryName = m_accessories[curIndex].pClass->GetName();

						accessoryOffset = pAttachment->GetAttAbsoluteDefault();

						break;
					}
				}
			}
		}

		if(accessoryName)
		{
			bool validSlot = GetEntity()->GetSlotInfo(slot, slotInfo) && (slotInfo.pCharacter || slotInfo.pStatObj);

			if (!validSlot || slotInfo.pStatObj)
			{
				if (validSlot)
				{
					Matrix34 mtx = GetEntity()->GetSlotLocalTM(slot, false) * Matrix34(accessoryOffset);
					finalOffset += mtx.GetTranslation();				
				}
			
				EntityEffects::SEffectAttachParams attachParams(finalOffset, dir, scale, prime, eIGS_Last);

				return m_effectsController.AttachParticleEffect(effectName, attachParams);
			}
			else if (slotInfo.pCharacter)	// bone attachment
			{
				ICharacterInstance *pCharacter = slotInfo.pCharacter;
				IAttachmentUpr *pAttachmentUpr = pCharacter->GetIAttachmentUpr();
				IAttachment *pAttachment = NULL; 

				helperName = string().Format("%s_%s", helper, accessoryName);
				pAttachment = pAttachmentUpr->GetInterfaceByName(helperName.c_str());

				if(!pAttachment)
				{
					IAttachment* pAccessoryAttachment = pAttachmentUpr->GetInterfaceByName(accessoryHelper);

					if(pAccessoryAttachment)
					{
						tukk bone = pCharacter->GetIDefaultSkeleton().GetJointNameByID(pAccessoryAttachment->GetJointID());
						pAttachment = pAttachmentUpr->CreateAttachment(helperName.c_str(), CA_BONE, bone);
						if (pAttachment)
						{
							QuatT relative = pAccessoryAttachment->GetAttRelativeDefault();
							relative = relative * accessoryOffset;
							relative.t = relative * finalOffset;

							finalOffset.zero();

							pAttachment->SetAttRelativeDefault(relative);
						}
					}
				}
			}
		}
	}

	EntityEffects::SEffectAttachParams attachParams(finalOffset, dir, scale, prime, eIGS_Last);
	return m_effectsController.AttachParticleEffect(effectName, slot, helperName, attachParams);
}

//------------------------------------------------------------------------
EntityEffects::TAttachedEffectId CItem::AttachLight(i32 slot, tukk helper, const EntityEffects::SLightAttachParams& lightParams)
{
	if (lightParams.radius < 0.1f || !g_pGameCVars->i_lighteffects)
	{
		return 0;
	}

	return m_effectsController.AttachLight(slot, helper, lightParams);
}

//------------------------------------------------------------------------
void CItem::DetachEffect(EntityEffects::TAttachedEffectId id)
{
	if(id)
	{
		m_effectsController.DetachEffect(id);
	}
}

//------------------------------------------------------------------------
IParticleEmitter* CItem::GetEffectEmitter(EntityEffects::TAttachedEffectId id) const
{
	if(id)
	{
		return m_effectsController.GetEffectEmitter(id);
	}
	
	return NULL;
}
