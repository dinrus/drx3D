// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat action for orientate one or multiple vehicle parts
      towards the view direction of the seat passanger

   -------------------------------------------------------------------------
   История:
   - 16:11:2005: Created by Benito G.R.

*************************************************************************/

#ifndef __VEHICLESEATACTIONORIENTATEPARTTOVIEW_H__
#define __VEHICLESEATACTIONORIENTATEPARTTOVIEW_H__

class CVehicleSeatActionOrientatePartToView
	: public IVehicleSeatAction
{
	IMPLEMENT_VEHICLEOBJECT

private:

	struct SOrientatePartInfo
	{
		SOrientatePartInfo()
			: partIndex(-1)
			, orientationAxis(1.0f, 1.0f, 1.0f)
		{

		}

		i32  partIndex;
		Vec3 orientationAxis;
	};

public:

	CVehicleSeatActionOrientatePartToView();

	virtual bool Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual void Reset() override   {}
	virtual void Release() override { delete this; }

	virtual void StartUsing(EntityId passengerId) override;
	virtual void ForceUsage() override                                                               {}
	virtual void StopUsing() override;
	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override {}

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override                          {}
	virtual void PostSerialize() override                                                            {}
	virtual void Update(const float deltaTime) override;

	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override {}

	virtual void GetMemoryUsage(IDrxSizer* s) const override;

protected:

	Vec3 GetViewDirection() const;

	typedef std::vector<SOrientatePartInfo> TPartIndices;

	IVehicle*     m_pVehicle;
	IVehicleSeat* m_pSeat;

	TPartIndices  m_controlledParts;
};

#endif //__VEHICLESEATACTIONORIENTATEPARTTOVIEW_H__
