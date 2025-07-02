// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEVIEWTHIRDPERSON_BONE_H
#define __VEHICLEVIEWTHIRDPERSON_BONE_H

#include "VehicleViewBase.h"

class CVehicleViewThirdPersonBone : public CVehicleViewBase
{
	IMPLEMENT_VEHICLEOBJECT;
public:
	CVehicleViewThirdPersonBone();
	~CVehicleViewThirdPersonBone();

	// IVehicleView
	virtual bool        Init(IVehicleSeat* pSeat, const CVehicleParams& table);

	virtual tukk GetName()           { return m_name; }
	virtual bool        IsThirdPerson()     { return true; }
	virtual bool        IsPassengerHidden() { return false; }

	virtual void        OnAction(const TVehicleActionId actionId, i32 activationMode, float value);
	virtual void        UpdateView(SViewParams& viewParams, EntityId playerId);

	virtual void        OnStartUsing(EntityId passengerId);

	virtual void        Update(const float frameTime);

	virtual bool        ShootToCrosshair() { return true; }

	virtual void        OffsetPosition(const Vec3& delta);
	// ~IVehicleView

	void GetMemoryUsage(IDrxSizer* s) const { s->Add(*this); }

protected:
	static tukk m_name;

	i32                m_directionBoneId;
	Vec3               m_offset;
	float              m_distance;
	Vec3               m_position;
	Quat               m_baseRotation;
	Quat               m_additionalRotation;

};

#endif
