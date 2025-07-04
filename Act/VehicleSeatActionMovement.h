// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat action which handle the vehicle movement

   -------------------------------------------------------------------------
   История:
   - 20:10:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLESEATACTIONMOVEMENT_H__
#define __VEHICLESEATACTIONMOVEMENT_H__

class CVehicleSeatActionMovement
	: public IVehicleSeatAction
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleSeatActionMovement();
	~CVehicleSeatActionMovement();

	// IVehicleSeatAction
	virtual bool Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual bool Init(IVehicle* pVehicle, IVehicleSeat* pSeat);
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual void StartUsing(EntityId passengerId) override;
	virtual void ForceUsage() override {}
	virtual void StopUsing() override;
	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override                      {}
	virtual void PostSerialize() override                                                        {}
	virtual void Update(const float deltaTime) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override { s->Add(*this); }
	// ~IVehicleSeatAction

protected:

	IVehicle*     m_pVehicle;
	IVehicleSeat* m_pSeat;

	float         m_actionForward;
	float         m_delayedStop;
};

#endif
