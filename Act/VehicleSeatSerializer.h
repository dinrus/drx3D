// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements an entity class which can serialize vehicle parts

   -------------------------------------------------------------------------
   История:
   - 16:09:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLESEATSERIALIZER_H__
#define __VEHICLESEATSERIALIZER_H__

#include "IGameObject.h"

class CVehicle;
class CVehicleSeat;

class CVehicleSeatSerializer
	: public CGameObjectExtensionHelper<CVehicleSeatSerializer, IGameObjectExtension>
{
public:

	CVehicleSeatSerializer();
	virtual ~CVehicleSeatSerializer();

	virtual bool                 Init(IGameObject* pGameObject);
	virtual void                 InitClient(i32 channelId);
	virtual void                 PostInit(IGameObject*)                                                          {}
	virtual void                 PostInitClient(i32 channelId)                                                   {};
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params);
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) {}
	virtual void                 Release()                                                                       { delete this; }

	virtual void                 FullSerialize(TSerialize ser);
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags);
	virtual void                 PostSerialize() {}
	virtual void                 SerializeSpawnInfo(TSerialize ser);
	virtual ISerializableInfoPtr GetSpawnInfo();
	virtual void                 Update(SEntityUpdateContext& ctx, i32);
	virtual void                 HandleEvent(const SGameObjectEvent& event);
	virtual void                 ProcessEvent(const SEntityEvent& event)  {};
	virtual uint64               GetEventMask() const { return 0; }
	virtual void                 SetChannelId(u16 id)            {};
	virtual void                 PostUpdate(float frameTime)        { DRX_ASSERT(false); };
	virtual void                 PostRemoteSpawn()                  {};
	virtual void                 GetMemoryUsage(IDrxSizer* s) const { s->Add(*this); }

	void                         SetVehicle(CVehicle* pVehicle);
	void                         SetSeat(CVehicleSeat* pSeat);

protected:
	CVehicle*     m_pVehicle;
	CVehicleSeat* m_pSeat;
};

#endif
