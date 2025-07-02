// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Manages AI proxies

   -------------------------------------------------------------------------
   История:
   - 16:04:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __AIPROXYMANAGER_H__
#define __AIPROXYMANAGER_H__

#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/AI/IAIActorProxy.h>
#include <drx3D/AI/IAIGroupProxy.h>

class CAIProxyUpr
	: public IEntitySystemSink
	  , public IAIActorProxyFactory
	  , public IAIGroupProxyFactory
{
public:
	CAIProxyUpr();
	virtual ~CAIProxyUpr();

	void           Init();
	void           Shutdown();

	IAIActorProxy* GetAIActorProxy(EntityId entityid) const;
	void           OnAIProxyDestroyed(IAIActorProxy* pProxy);

	// IAIActorProxyFactory
	virtual IAIActorProxy* CreateActorProxy(EntityId entityID);
	// ~IAIActorProxyFactory

	// IAIGroupProxyFactory
	virtual IAIGroupProxy* CreateGroupProxy(i32 groupID);
	// ~IAIGroupProxyFactory

	// IEntitySystemSink
	virtual bool OnBeforeSpawn(SEntitySpawnParams& params);
	virtual void OnSpawn(IEntity* pEntity, SEntitySpawnParams& params);
	virtual bool OnRemove(IEntity* pEntity);
	virtual void OnReused(IEntity* pEntity, SEntitySpawnParams& params);
	//~IEntitySystemSink

private:
	typedef std::map<EntityId, CAIProxy*> TAIProxyMap;
	TAIProxyMap m_aiProxyMap;
};

#endif //__AIPROXYMANAGER_H__
