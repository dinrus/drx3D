// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a usable action to enter a vehicle seat

   -------------------------------------------------------------------------
   История:
   - 19:01:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEUSABLEACTIONENTER_H__
#define __VEHICLEUSABLEACTIONENTER_H__

class CVehicleUsableActionEnter
	: public IVehicleAction
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleUsableActionEnter()
		: m_pVehicle(nullptr)
	{
	}

	~CVehicleUsableActionEnter() {}

	// IVehicleAction
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override   {}
	virtual void Release() override { delete this; }

	virtual i32  OnEvent(i32 eventType, SVehicleEventParams& eventParams) override;
	void         GetMemoryStatistics(IDrxSizer* s);
	// ~IVehicleAction

	// IVehicleObject
	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override                      {}
	virtual void Update(const float deltaTime) override                                          {}
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}
	// ~IVehicleObject

protected:

	bool IsSeatAvailable(IVehicleSeat* pSeat, EntityId userId);

	CVehicle* m_pVehicle;

	typedef std::vector<TVehicleSeatId> TVehicleSeatIdVector;
	TVehicleSeatIdVector m_seatIds;
};

#endif
