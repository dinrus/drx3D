// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat action to head/spot lights

   -------------------------------------------------------------------------
   История:
   - 01:03:2006: Created by Mathieu Pinard
   - 20:07:2010: Refactored by Paul Slinger

*************************************************************************/
#ifndef __VEHICLESEATACTIONLIGHTS_H__
#define __VEHICLESEATACTIONLIGHTS_H__

#include <drx3D/Act/ISharedParams.h>

class CVehiclePartLight;

class CVehicleSeatActionLights : public IVehicleSeatAction
{
	IMPLEMENT_VEHICLEOBJECT

public:
	~CVehicleSeatActionLights();

	// IVehicleSeatAction

	virtual bool Init(IVehicle* pVehicle, IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void Release() override                        { delete this; }
	virtual void StartUsing(EntityId passengerId) override {}
	virtual void ForceUsage() override                     {}
	virtual void StopUsing() override                      {}
	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override;
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;

	// ~IVehicleSeatAction

	// IVehicleObject

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) override;
	virtual void PostSerialize() override               {}
	virtual void Update(const float deltaTime) override {}

	// IVehicleObject

	// IVehicleEventListener

	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;

	// ~IVehicleEventListener

protected:

	enum ELightActivation
	{
		eLA_Toggle = 0,
		eLA_Brake,
		eLA_Reversing
	};

	BEGIN_SHARED_PARAMS(SSharedParams)

	TVehicleSeatId seatId;

	ELightActivation activation;

	string           onSound, offSound;

	END_SHARED_PARAMS

	struct SLightPart
	{
		inline SLightPart() : pPart(NULL)
		{
		}

		inline SLightPart(CVehiclePartLight* pPart) : pPart(pPart)
		{
		}

		CVehiclePartLight* pPart;

		void               GetMemoryUsage(IDrxSizer* pSizer) const {}
	};

	typedef std::vector<SLightPart> TVehiclePartLightVector;

	void ToggleLights(bool enable);
	void PlaySound(const string& name);

	IVehicle*               m_pVehicle;

	SSharedParamsConstPtr   m_pSharedParams;

	TVehiclePartLightVector m_lightParts;

	bool                    m_enabled;
};

#endif //__VEHICLESEATACTIONLIGHTS_H__
