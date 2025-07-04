// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:
   Implements a seat action for shaking a list of parts based on
   vehicle movement, speed and acceleration

   -------------------------------------------------------------------------
   История:
   - Created by Stan Fichele

*************************************************************************/

#ifndef __VEHICLESEATACTION_SHAKE_PARTS__H__
#define __VEHICLESEATACTION_SHAKE_PARTS__H__

#include <drx3D/Act/ISharedParams.h>
#include "VehicleNoiseGenerator.h"

class CVehicleSeatActionShakeParts
	: public IVehicleSeatAction
{
	IMPLEMENT_VEHICLEOBJECT

public:

	CVehicleSeatActionShakeParts();

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

	BEGIN_SHARED_PARAMS(SSharedParams)

	struct SPartInfo
	{
		u32 partIndex;
		float        amplitudeUpDown;
		float        amplitudeRot;
		float        freq;
		float        suspensionAmp;
		float        suspensionResponse;
		float        suspensionSharpness;
	};
	typedef std::vector<SPartInfo> TPartInfos;
	typedef const TPartInfos       TPartInfosConst;
	TPartInfos partInfos;

	END_SHARED_PARAMS

	struct SPartInstance
	{
		// Updated at runtime
		CVehicleNoiseValue noiseUpDown;
		CVehicleNoiseValue noiseRot;
		float              zpos;
	};

	typedef std::vector<SPartInstance> TParts;
	IVehicle*             m_pVehicle;
	TParts                m_controlledParts;
	SSharedParamsConstPtr m_pSharedParams;
};

#endif //__VEHICLESEATACTION_SHAKE_PARTS__H__
