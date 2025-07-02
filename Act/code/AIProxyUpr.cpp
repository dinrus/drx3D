// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Управление прокси ИИ.

   -------------------------------------------------------------------------
   История:
   - 16:04:2010: Created by Kevin Kirst

*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/AIProxyUpr.h>
#include <drx3D/Act/AIProxy.h>
#include <drx3D/Act/AIGroupProxy.h>

//////////////////////////////////////////////////////////////////////////
CAIProxyUpr::CAIProxyUpr()
{

}

//////////////////////////////////////////////////////////////////////////
CAIProxyUpr::~CAIProxyUpr()
{

}

//////////////////////////////////////////////////////////////////////////
void CAIProxyUpr::Init()
{
	gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnReused);

	gEnv->pAISystem->SetActorProxyFactory(this);
	gEnv->pAISystem->SetGroupProxyFactory(this);
}

//////////////////////////////////////////////////////////////////////////
void CAIProxyUpr::Shutdown()
{
	gEnv->pEntitySystem->RemoveSink(this);

	gEnv->pAISystem->SetActorProxyFactory(NULL);
	gEnv->pAISystem->SetGroupProxyFactory(NULL);
}

//////////////////////////////////////////////////////////////////////////
IAIActorProxy* CAIProxyUpr::GetAIActorProxy(EntityId id) const
{
	TAIProxyMap::const_iterator it = m_aiProxyMap.find(id);
	if (it != m_aiProxyMap.end())
		return it->second;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
IAIActorProxy* CAIProxyUpr::CreateActorProxy(EntityId entityID)
{
	CAIProxy* pResult = NULL;

	if (IGameObject* gameObject = CDrxAction::GetDrxAction()->GetGameObject(entityID))
	{
		pResult = new CAIProxy(gameObject);

		// (MATT) Check if this instance already has a proxy - right now there's no good reason to change proxies on an instance {2009/04/06}
		TAIProxyMap::iterator it = m_aiProxyMap.find(entityID);
		if (it != m_aiProxyMap.end())
		{
			DRX_ASSERT_TRACE(false, ("Entity ID %d already has an actor proxy! possible memory leak", entityID));
			it->second = pResult;
		}
		else
		{
			m_aiProxyMap.insert(std::make_pair(entityID, pResult));
		}
	}
	else
	{
		GameWarning("Trying to create AIActorProxy for un-existent game object.");
	}

	return pResult;
}

//////////////////////////////////////////////////////////////////////////
IAIGroupProxy* CAIProxyUpr::CreateGroupProxy(i32 groupID)
{
	return new CAIGroupProxy(groupID);
}

//////////////////////////////////////////////////////////////////////////
void CAIProxyUpr::OnAIProxyDestroyed(IAIActorProxy* pProxy)
{
	assert(pProxy);

	TAIProxyMap::iterator itProxy = m_aiProxyMap.begin();
	TAIProxyMap::iterator itProxyEnd = m_aiProxyMap.end();
	for (; itProxy != itProxyEnd; ++itProxy)
	{
		if (itProxy->second == pProxy)
		{
			m_aiProxyMap.erase(itProxy);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CAIProxyUpr::OnBeforeSpawn(SEntitySpawnParams& params)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CAIProxyUpr::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{

}

//////////////////////////////////////////////////////////////////////////
bool CAIProxyUpr::OnRemove(IEntity* pEntity)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CAIProxyUpr::OnReused(IEntity* pEntity, SEntitySpawnParams& params)
{
	TAIProxyMap::iterator itProxy = m_aiProxyMap.find(params.prevId);
	if (itProxy != m_aiProxyMap.end())
	{
		CAIProxy* pProxy = itProxy->second;
		assert(pProxy);

		pProxy->OnReused(pEntity, params);

		// Update lookup map
		m_aiProxyMap.erase(itProxy);
		m_aiProxyMap[pEntity->GetId()] = pProxy;
	}
}
