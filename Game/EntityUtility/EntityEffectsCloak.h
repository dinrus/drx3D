// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Utility functions related to cloaking effect

-------------------------------------------------------------------------
История:
- 15:09:2010: Original by Colin Gulliver
- 02:08:2011: Created by Benito G.R. from EntityCloak.h (renamed namespaces/folder)

*************************************************************************/

#pragma once

#ifndef __ENTITY_EFFECTS_CLOAK_H__
#define __ENTITY_EFFECTS_CLOAK_H__

namespace EntityEffects
{
	namespace Cloak
	{
		enum ECloakColorChannels
		{
			eCCC_Friendly=0,
			eCCC_Hostile
		};

		struct CloakSyncParams
		{
			CloakSyncParams(const EntityId _masterId, const EntityId _slaveId, const bool _fadeToDesiredCloakTarget = false, const bool _forceDecloakOfSlave = false)
				: cloakMasterId(_masterId)
				, cloakSlaveId(_slaveId)
				, fadeToDesiredCloakTarget(_fadeToDesiredCloakTarget)	// Snap if false
				, forceDecloakOfSlave(_forceDecloakOfSlave)            // Handy override for when weapon dropped
			{

			}

			EntityId cloakMasterId;
			EntityId cloakSlaveId; 
			bool fadeToDesiredCloakTarget;
			bool forceDecloakOfSlave; 
		};

		void CloakEntity(IEntity *pEntity, bool bEnable, bool bFade, float blendSpeedScale, bool bCloakFadeByDistance, u8 cloakPaletteColorChannel,bool bIgnoreCloakRefractionColor);
		void CloakEntity(EntityId entityId, bool bEnable, bool bFade, float blendSpeedScale, bool bCloakFadeByDistance, u8 cloakPaletteColorChannel,bool bIgnoreCloakRefractionColor);
		bool CloakSyncEntities(const CloakSyncParams& params);

		bool DoesCloakFadeByDistance(EntityId ownerEntityId);
		u8 GetCloakColorChannel(EntityId ownerEntityId);
		ILINE u8 GetCloakColorChannel ( const bool bIsFriendly ) { return bIsFriendly ? eCCC_Friendly : eCCC_Hostile; }
		bool IgnoreRefractionColor(EntityId ownerEntityId);
	};
};

#endif // __ENTITY_CLOAK_H__