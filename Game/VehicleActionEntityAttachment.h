// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Implements a vehicle action for Entity Attachment

-------------------------------------------------------------------------
История:
- 07:12:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEACTIONENTITYATTACHMENT_H__
#define __VEHICLEACTIONENTITYATTACHMENT_H__

class CVehicleActionEntityAttachment
	: public IVehicleAction
{
	IMPLEMENT_VEHICLEOBJECT;

public:

	CVehicleActionEntityAttachment();
	virtual ~CVehicleActionEntityAttachment();

	// IVehicleAction
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual i32 OnEvent(i32 eventType, SVehicleEventParams& eventParams) override;
	void GetMemoryStatistics(IDrxSizer * s) { s->Add(*this); }
	// ~IVehicleAction

	// IVehicleObject
	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override {}
	virtual void Update(const float deltaTime) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}
	// ~IVehicleObject

	bool DetachEntity();
	bool IsEntityAttached() { return m_isAttached; }

	EntityId GetAttachmentId() { return m_entityId; }

protected:

	void SpawnEntity();

	IVehicle* m_pVehicle;

	string m_entityClassName;
	IVehicleHelper* m_pHelper;

	EntityId m_entityId;
	bool m_isAttached;

	float m_timer;
};

#endif
