// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id: ReplayObject.h$
$DateTime$
Описание: A replay entity spawned during KillCam replay

-------------------------------------------------------------------------
История:
- 03/19/2010 09:15:00: Created by Martin Sherburn

*************************************************************************/

#ifndef __REPLAYOBJECT_H__
#define __REPLAYOBJECT_H__

#include <drx3D/Act/IDrxMannequin.h>

class CReplayObjectAction : public TAction<SAnimationContext>
{
private:
	typedef TAction<SAnimationContext> BaseClass;
public:
	CReplayObjectAction(FragmentID fragID, const TagState &fragTags, u32 optionIdx, bool trumpsPrevious, i32k priority );
	virtual EPriorityComparison ComparePriority(const IAction &actionCurrent) const;
private:
	bool m_trumpsPrevious;
};

class CReplayItemList
{
public:
	CReplayItemList(){}
	~CReplayItemList(){}
	void AddItem( const EntityId itemId );
	void OnActionControllerDeleted();

private:
	typedef std::vector<EntityId> TItemVec;
	TItemVec m_items;
};

class CReplayObject : public CGameObjectExtensionHelper<CReplayObject, IGameObjectExtension>
{
public:
	CReplayObject();
	virtual ~CReplayObject();

	// IGameObjectExtension
	virtual bool Init(IGameObject *pGameObject);
	virtual void InitClient(i32 channelId) {}
	virtual void PostInit(IGameObject *pGameObject) {}
	virtual void PostInitClient(i32 channelId) {}
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params ) {}
	virtual bool GetEntityPoolSignature( TSerialize signature );
	virtual void Release() { delete this; }
	virtual void FullSerialize( TSerialize ser ) {}
	virtual bool NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags ) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() {return 0;}
	virtual void Update( SEntityUpdateContext &ctx, i32 updateSlot) {}
	virtual void PostUpdate(float frameTime ) {}
	virtual void PostRemoteSpawn() {}
	virtual void HandleEvent( const SGameObjectEvent &) {}
	virtual void ProcessEvent(SEntityEvent &) {}
	virtual void SetChannelId(u16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void GetMemoryUsage(IDrxSizer * s) const {}
	//~IGameObjectExtension

	void SetTimeSinceSpawn(float time) { assert(time >= 0); m_timeSinceSpawn = time; }

protected:
	float m_timeSinceSpawn;
};

#endif //!__REPLAYOBJECT_H__
