// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Game/VehicleMovementArcadeWheeled.h>

#if !defined(_RELEASE)
#define DEBUG_TANK_AI
#endif

class CVehicleMovementTank
	: public CVehicleMovementArcadeWheeled
{
private:
	typedef CVehicleMovementArcadeWheeled inherited;

public:
	CVehicleMovementTank();
	virtual ~CVehicleMovementTank();

	// overrides from StdWheeled
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table) override;
	virtual void PostInit() override;
	virtual void Reset() override;

	virtual void ProcessAI(const float deltaTime) override;
	virtual void ProcessMovement(const float deltaTime) override;

	virtual void OnEvent(EVehicleMovementEvent event, const SVehicleMovementEventParams& params) override;
	virtual void OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override;

	virtual void Update(const float deltaTime) override;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override;
	// ~StdWheeled

	virtual float GetEnginePedal() override { return m_currPedal; }

protected:
	virtual bool DoGearSound() override { return false; }
	virtual float GetMinRPMSoundRatio() override { return 0.6f; }  
#if ENABLE_VEHICLE_DEBUG
	virtual void DebugDrawMovement(const float deltaTime) override;
#endif
	virtual float GetWheelCondition() const override;

	float m_currPedal;
	float m_currSteer;
	bool m_bOnTurnTurret;
	bool m_bTurretTurning;
	float m_turretTurnSpeed;
	float m_lastTurretTurnSpeed;
	AudioControlId m_turretTurnRtpcId;

	typedef std::vector<IVehiclePart*> TTreadParts;
	TTreadParts m_treadParts;    

#ifdef DEBUG_TANK_AI
	struct Debug
	{
		Vec3 targetPos;
		float inputSpeed;
	};
	Debug m_debug;
#endif
};
