// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements an entity class for detached parts

   -------------------------------------------------------------------------
   История:
   - 26:10:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEPARTDETACHEDENTITY_H__
#define __VEHICLEPARTDETACHEDENTITY_H__

#include "IGameObject.h"

class CVehiclePartDetachedEntity
	: public CGameObjectExtensionHelper<CVehiclePartDetachedEntity, IGameObjectExtension>
{
public:
	~CVehiclePartDetachedEntity();

	virtual bool                 Init(IGameObject* pGameObject);
	virtual void                 InitClient(i32 channelId)                                                       {};
	virtual void                 PostInit(IGameObject*);
	virtual void                 PostInitClient(i32 channelId)                                                   {};
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) {}
	virtual void                 Release()                                                                       { delete this; }

	virtual void                 FullSerialize(TSerialize ser);
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) { return true; }
	virtual void                 PostSerialize()                                                               {}
	virtual void                 SerializeSpawnInfo(TSerialize ser)                                            {}
	virtual ISerializableInfoPtr GetSpawnInfo()                                                                { return 0; }
	virtual void                 Update(SEntityUpdateContext& ctx, i32 slot);
	virtual void                 HandleEvent(const SGameObjectEvent& event);
	virtual void                 ProcessEvent(const SEntityEvent& event);
	virtual uint64               GetEventMask() const;
	virtual void                 SetChannelId(u16 id)                 {};
	virtual void                 PostUpdate(float frameTime)             { DRX_ASSERT(false); }
	virtual void                 PostRemoteSpawn()                       {};
	virtual void                 GetMemoryUsage(IDrxSizer* pSizer) const { pSizer->Add(*this); }

protected:
	void InitVehiclePart(IGameObject* pGameObject);

	float m_timeUntilStartIsOver;
};

#endif
