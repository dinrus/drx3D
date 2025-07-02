// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Interactive objects are registered here with required params

-------------------------------------------------------------------------
История:
- 10:12:2009: Created by Benito G.R.

*************************************************************************/

#pragma once

#ifndef __INTERACTIVEOBJECT_REGISTRY_H__
#define __INTERACTIVEOBJECT_REGISTRY_H__

#include #include <drx3D/Game/Utility/DrxHash.h>
#include <drx3D/Game/ItemString.h>

#include <IDrxMannequinDefs.h>
#include <drx3D/Game/InteractiveObjectEnums.h>
#include <drx3D/Game/IDrxMannequinProceduralClipFactory.h>

class IActionController;
class IAnimationDatabase;
struct SControllerDef;

class CInteractiveObjectRegistry
{
public : 

	struct SInteraction
	{
		tukk helper;
		TagID			  targetStateTagID;
	};

	struct SMannequinParams
	{
		TagID interactionTagID;
		TagID stateTagID;

		SMannequinParams()
			: interactionTagID(TAG_ID_INVALID)
			, stateTagID(TAG_ID_INVALID)
		{

		}

		void GetMemoryUsage(IDrxSizer *pSizer) const
		{

		}
	};

private:

	typedef std::map< EntityId, SMannequinParams > TEntityToTagMap;

public:

	void Init();
	void Shutdown();

	void OnInteractiveObjectSutdown(const EntityId entityId);

	TagID GetInteractionTag(tukk interactionName) const;
	const SMannequinParams GetInteractionTagForEntity(const EntityId entityId, i32k interactionIndex) const;

	void QueryInteractiveActionOptions( const EntityId entityId, const TagID interactionTag, const TagID stateTagId, std::vector<SInteraction> &options);
	void ApplyInteractionToObject(IEntity *pEntity, const TagID interactionTypeTag, i32k interactionIndex) const;

	void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddContainer(m_entityToTag);
	}

private:

	TEntityToTagMap			m_entityToTag;
	IProceduralClipFactory::THash m_clipLocatorHash;

	FragmentID m_interactFragID;
	const IAnimationDatabase *m_pDatabasePlayer;
	const IAnimationDatabase *m_pDatabaseObject;
	const SControllerDef *m_pControllerDef;
};

#endif
