// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Implements a dummy vehicle movement, for prop vehicles

-------------------------------------------------------------------------
История:
- 05:03:2010: Created by Steve Humphreys

*************************************************************************/

#pragma once

#ifndef __VEHICLEMOVEMENTDUMMY_H__
#define __VEHICLEMOVEMENTDUMMY_H__

#include <drx3D/Act/IVehicleSystem.h>

class CVehicleMovementDummy : public IVehicleMovement
{
public:
	// IVehicleMovement
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table);
	virtual void PostInit() {}
	virtual void Physicalize();
	virtual void PostPhysicalize() {}
	virtual void Reset() {}
	virtual void Release();
	virtual EVehicleMovementType GetMovementType() { return eVMT_Dummy; }
	virtual void ResetInput() {}
	virtual bool StartDriving(EntityId driverId) { assert(false); return true; }
	virtual void StopDriving() {}
	virtual bool StartEngine() { return true; }
	virtual void StopEngine() {}


	virtual bool IsPowered() { return false; }
	virtual float GetEnginePedal(){ return 0.0f; }
	virtual float GetDamageRatio() { return 0.0f; }
	virtual i32 GetWheelContacts() const { return 0; }
	virtual void OnEvent(EVehicleMovementEvent event, const SVehicleMovementEventParams& params) {}

	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) {}
	virtual void ProcessMovement(const float deltaTime) {}
	virtual void EnableMovementProcessing(bool enable) {}
	virtual bool IsMovementProcessingEnabled() { return false; }; 
	virtual void DisableEngine(bool disable) {}

	virtual void Update(const float deltaTime) {}

	virtual void Serialize(TSerialize ser, EEntityAspects aspects) {}	  
	virtual void SetAuthority(bool auth) {}
	virtual void PostSerialize() {}

	virtual SVehicleMovementAction GetMovementActions() { return SVehicleMovementAction(); }
	virtual void RequestActions(const SVehicleMovementAction& movementAction) {}
	virtual bool RequestMovement(CMovementRequest& movementRequest) { return true; }
	virtual void GetMovementState(SMovementState& movementState) {}
	virtual bool GetStanceState( const SStanceStateQuery& query, SStanceState& state ) { return false; }
	virtual SVehicleNetState GetVehicleNetState() { return SVehicleNetState(); }
	virtual void SetVehicleNetState(const SVehicleNetState& state) {}

	virtual pe_type GetPhysicalizationType() const { return PE_WHEELEDVEHICLE; }
	virtual bool UseDrivingProxy() const { return false; }

	virtual void RegisterActionFilter(IVehicleMovementActionFilter* pActionFilter) {}
	virtual void UnregisterActionFilter(IVehicleMovementActionFilter* pActionFilter) {}

	virtual void ProcessEvent(SEntityEvent& event) {} 
	virtual DrxCriticalSection* GetNetworkLock() { return NULL; }

	virtual void PostUpdateView(SViewParams &viewParams, EntityId playerId) {}
	virtual i32 GetStatus(SVehicleMovementStatus* status) { return 0; }

	virtual void GetMemoryUsage(IDrxSizer * pSizer) const 
	{
		pSizer->AddObject(this, sizeof(*this));
	}
	// ~IVehicleMovement

	IVehicle* m_pVehicle;
};

#endif //__VEHICLEMOVEMENTDUMMY_H__
