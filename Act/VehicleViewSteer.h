// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a third person view for vehicles

   -------------------------------------------------------------------------
   История:
   - Created by Rich Semmens/Stan Fichele

*************************************************************************/
#ifndef __VEHICLEVIEWSTEER_H__
#define __VEHICLEVIEWSTEER_H__

#include "VehicleViewBase.h"

enum EVehicleViewSteerFlags
{
	eVCam_rotationClamp  = 1 << 0,        // Clamp any rotation
	eVCam_rotationSpring = 1 << 1,        // Allow spring relaxation to default position
	eVCam_goingForwards  = 1 << 2,        // Direction of travel
	eVCam_canRotate      = 1 << 3,        // Use input to rotate camera
};

class CVehicleViewSteer
	: public CVehicleViewBase
{
	IMPLEMENT_VEHICLEOBJECT;
public:

	CVehicleViewSteer();
	virtual ~CVehicleViewSteer();

	// IVehicleView
	virtual bool Init(IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual void Reset() override;
	virtual void ResetPosition() override
	{
		m_position = m_pVehicle->GetEntity()->GetWorldPos();
	}

	virtual tukk GetName() override           { return m_name; }
	virtual bool        IsThirdPerson() override     { return true; }
	virtual bool        IsPassengerHidden() override { return false; }

	virtual void        OnAction(const TVehicleActionId actionId, i32 activationMode, float value) override;
	virtual void        UpdateView(SViewParams& viewParams, EntityId playerId) override;

	virtual void        OnStartUsing(EntityId passengerId) override;
	virtual void        OnStopUsing() override;

	virtual void        Update(const float frameTime) override;
	virtual void Serialize(TSerialize serialize, EEntityAspects) override;

	virtual bool ShootToCrosshair() override { return false; }
	virtual void OffsetPosition(const Vec3& delta) override;
	// ~IVehicleView

	bool         Init(CVehicleSeat* pSeat);
	void         CalcLookAt(const Matrix34& entityTM);

	virtual void GetMemoryUsage(IDrxSizer* s) const override { s->Add(*this); }

protected:
	IVehiclePart*      m_pAimPart;

	Vec3               m_maxRotation;
	Vec3               m_maxRotation2;
	Vec3               m_stickSensitivity;
	Vec3               m_stickSensitivity2;

	static tukk m_name;

	Vec3               m_position;
	Vec3               m_localSpaceCameraOffset;
	Vec3               m_lastOffsetBeforeElev;
	Vec3               m_lastOffset;
	Vec3               m_lookAt0;
	Vec3               m_lookAt;
	Vec3               m_lastVehVel;

	i32                m_flags; /* EVehicleViewSteerFlags */
	i32                m_forwardFlags;
	i32                m_backwardsFlags;

	float              m_angReturnSpeed1;
	float              m_angReturnSpeed2;
	float              m_angReturnSpeed;
	float              m_curYaw;

	float              m_angSpeedCorrection0;
	float              m_angSpeedCorrection;
	float              m_radius;
	float              m_radiusVel;          // Velocity change of radius
	float              m_radiusRelaxRate;    // Relax spring to default radius
	float              m_radiusDampRate;     // Damp the velocity change of the radius
	float              m_radiusVelInfluence; // How much the vehicle velocity influences the camera distance/radius
	float              m_radiusMin;
	float              m_radiusMax;
	float              m_inheritedElev;
	float              m_pivotOffset;
};

#endif
