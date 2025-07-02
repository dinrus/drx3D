// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/MPPathFollowingUpr.h>
#include <drx3D/Game/GameRules.h>

CMPPathFollowingUpr::CMPPathFollowingUpr()
{
	m_Paths.reserve(4);
}

CMPPathFollowingUpr::~CMPPathFollowingUpr()
{

}

void CMPPathFollowingUpr::RegisterClassFollower(u16 classId, IMPPathFollower* pFollower)
{
#ifndef _RELEASE
	PathFollowers::iterator iter = m_PathFollowers.find(classId);
	DRX_ASSERT_MESSAGE(iter == m_PathFollowers.end(), "CMPPathFollowingUpr::RegisterClassFollower - this class has already been registered!");
#endif
	m_PathFollowers[classId] = pFollower;
}

void CMPPathFollowingUpr::UnregisterClassFollower(u16 classId)
{
	PathFollowers::iterator iter = m_PathFollowers.find(classId);
	if(iter != m_PathFollowers.end())
	{
		m_PathFollowers.erase(iter);
		return;
	}

	DRX_ASSERT_MESSAGE(false, "CMPPathFollowingUpr::UnregisterClassFollower - tried to unregister class but class not found");
}

void CMPPathFollowingUpr::RequestAttachEntityToPath( const SPathFollowingAttachToPathParameters& params )
{
	PathFollowers::const_iterator iter = m_PathFollowers.find(params.classId);
	if(iter != m_PathFollowers.end())
	{
		DRX_ASSERT_MESSAGE(params.pathIndex < m_Paths.size(), "CMPPathFollowingUpr::RequestAttachEntityToPath - path index out of range");
		iter->second->OnAttachRequest(params, &m_Paths[params.pathIndex].path);
		if(gEnv->bServer)
		{
			SPathFollowingAttachToPathParameters sendParams(params);
			sendParams.forceSnap = true;
			g_pGame->GetGameRules()->GetGameObject()->InvokeRMI(CGameRules::ClPathFollowingAttachToPath(), params, eRMI_ToRemoteClients);
		}
	}
}

void CMPPathFollowingUpr::RequestUpdateSpeed(u16 classId, EntityId attachEntityId, float newSpeed)
{
	PathFollowers::const_iterator iter = m_PathFollowers.find(classId);
	if(iter != m_PathFollowers.end())
	{
		iter->second->OnUpdateSpeedRequest(attachEntityId, newSpeed);
	}
}

bool CMPPathFollowingUpr::RegisterPath(EntityId pathEntityId)
{
	IEntity* pPathEntity = gEnv->pEntitySystem->GetEntity(pathEntityId);
	if(pPathEntity)
	{
#ifndef _RELEASE
		Paths::const_iterator iter = m_Paths.begin();
		Paths::const_iterator end = m_Paths.end();
		while(iter != end)
		{
			if(iter->pathId == pathEntityId)
			{
				DRX_ASSERT_MESSAGE(iter == m_Paths.end(), "CMPPathFollowingUpr::RegisterPath - this path has already been registered!");
				break;
			}
			++iter;
		}
#endif

		m_Paths.push_back( SPathEntry(pathEntityId) );
		bool success = m_Paths.back().path.CreatePath(pPathEntity);
		if(success)
		{
			//Editor support for changing path nodes
			if(gEnv->IsEditor())
			{
				gEnv->pEntitySystem->AddEntityEventListener(pathEntityId, ENTITY_EVENT_RESET, this);
			}

			return true;
		}
		else
		{
			m_Paths.pop_back();
		}
	}

	return false;
}

void CMPPathFollowingUpr::UnregisterPath(EntityId pathEntityId)
{
	Paths::iterator iter = m_Paths.begin();
	Paths::iterator end = m_Paths.end();
	while(iter != end)
	{
		if(iter->pathId == pathEntityId)
		{
			if(gEnv->IsEditor())
			{
				gEnv->pEntitySystem->RemoveEntityEventListener(pathEntityId, ENTITY_EVENT_RESET, this);
			}
			m_Paths.erase(iter);
			return;
		}
		++iter;
	}
}

void CMPPathFollowingUpr::RegisterListener(EntityId listenToEntityId, IMPPathFollowingListener* pListener)
{
#ifndef _RELEASE
	PathListeners::iterator iter = m_PathListeners.find(listenToEntityId);
	DRX_ASSERT_MESSAGE(iter == m_PathListeners.end(), "CMPPathFollowingUpr::RegisterListener - this listener has already been registered!");
#endif

	m_PathListeners[listenToEntityId] = pListener;
}

void CMPPathFollowingUpr::UnregisterListener(EntityId listenToEntityId)
{
	PathListeners::iterator iter = m_PathListeners.find(listenToEntityId);
	if(iter != m_PathListeners.end())
	{
		m_PathListeners.erase(iter);
		return;
	}

	DRX_ASSERT_MESSAGE(false, "CMPPathFollowingUpr::UnregisterListener - tried to unregister listener but listener not found");
}

const CWaypointPath* CMPPathFollowingUpr::GetPath(EntityId pathEntityId, IMPPathFollower::MPPathIndex& outIndex) const
{
	for(u32 i = 0; i < m_Paths.size(); ++i)
	{
		if(m_Paths[i].pathId == pathEntityId)
		{
			outIndex = i;
			return &m_Paths[i].path;
		}
	}

	return NULL;
}

void CMPPathFollowingUpr::NotifyListenersOfPathCompletion(EntityId pathFollowingEntityId)
{
	PathListeners::iterator iter = m_PathListeners.find(pathFollowingEntityId);
	if(iter != m_PathListeners.end())
	{
		iter->second->OnPathCompleted(pathFollowingEntityId);
	}
}

 void CMPPathFollowingUpr::OnEntityEvent( IEntity *pEntity,SEntityEvent &event )
 {
		if(event.event == ENTITY_EVENT_RESET && event.nParam[0] == 0) //Only on leaving the editor gamemode
		{
			UnregisterPath(pEntity->GetId());
		}
 }

 #ifndef _RELEASE
 void CMPPathFollowingUpr::Update()
 {
	 if(g_pGameCVars->g_mpPathFollowingRenderAllPaths)
	 {
		 Paths::const_iterator iter = m_Paths.begin();
		 Paths::const_iterator end = m_Paths.end();
		 while(iter != end)
		 {
			 iter->path.DebugDraw(true);
			 ++iter;
		 }
	 }
 }
#endif //_RELEASE
