// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   VTOLVehicleUpr.h
//  Version:     v1.00
//  Created:     02/09/2011
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Handles entities being attached to paths in multiplayer
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __MPPATHFOLLOWINGMANAGER_H
#define __MPPATHFOLLOWINGMANAGER_H

#include <drx3D/Game/WaypointPath.h>
#include <drx3D/Entity/IEntitySystem.h>

struct IMPPathFollowingListener
{
	virtual void OnPathCompleted(EntityId attachedEntityId) = 0;
	virtual ~IMPPathFollowingListener() {};
};

struct IMPPathFollower
{
	typedef u8 MPPathIndex;

	virtual void OnAttachRequest(const struct SPathFollowingAttachToPathParameters& params, const CWaypointPath* pPath) = 0;
	virtual void OnUpdateSpeedRequest(EntityId attachEntityId, float speed) = 0;
	virtual ~IMPPathFollower() {};
};

class CMPPathFollowingUpr : public IEntityEventListener
{
public:
	CMPPathFollowingUpr();
	~CMPPathFollowingUpr();

	//Note: Currently supports one listener per map entry.
	void RegisterClassFollower(u16 classId, IMPPathFollower* pFollower);
	void UnregisterClassFollower(u16 classId);

	bool RegisterPath(EntityId pathEntityId);
	void UnregisterPath(EntityId pathEntityId);

	void RegisterListener(EntityId listenToEntityId, IMPPathFollowingListener* pListener);
	void UnregisterListener(EntityId listenToEntityId);

	const CWaypointPath* GetPath(EntityId pathEntityId, IMPPathFollower::MPPathIndex& outIndex) const;

	void RequestAttachEntityToPath(const struct SPathFollowingAttachToPathParameters& params);

	void RequestUpdateSpeed(u16 classId, EntityId attachEntityId, float newSpeed);

	void NotifyListenersOfPathCompletion(EntityId pathFollowingEntityId);

	// IEntityEventListener
	virtual void OnEntityEvent( IEntity *pEntity,SEntityEvent &event );
	// ~IEntityEventListener

#ifndef _RELEASE
	void Update();
#endif //_RELEASE

private:
	typedef std::map<u16, IMPPathFollower*> PathFollowers;
	PathFollowers m_PathFollowers;

	struct SPathEntry
	{
		SPathEntry(EntityId pathEntityId) : pathId(pathEntityId)
		{
		}

		EntityId pathId;
		CWaypointPath path;
	};

	typedef std::vector<SPathEntry> Paths;
	Paths m_Paths;

	typedef std::map<EntityId, IMPPathFollowingListener*> PathListeners;
	PathListeners m_PathListeners;
};

#endif