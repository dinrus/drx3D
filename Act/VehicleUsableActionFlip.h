// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a usable action to recover a vehicle by flipping it over

   -------------------------------------------------------------------------
   История:
   - Created by Stan Fichele

*************************************************************************/
#ifndef __VEHICLEUSABLEACTIONFLIP_H__
#define __VEHICLEUSABLEACTIONFLIP_H__

class CVehicleUsableActionFlip
	: public IVehicleAction
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehicleUsableActionFlip();
	~CVehicleUsableActionFlip() {}

	// IVehicleAction
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override { delete this; }

	virtual i32  OnEvent(i32 eventType, SVehicleEventParams& eventParams) override;
	void         GetMemoryStatistics(IDrxSizer* s);
	// ~IVehicleAction

	// IVehicleObject
	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override                      {}
	virtual void Update(const float deltaTime) override;
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}
	// ~IVehicleObject

protected:

	CVehicle* m_pVehicle;
	float     m_timer;
	float     m_postReorientedTimer;
	Vec3      m_localAngVel;
};

#endif
