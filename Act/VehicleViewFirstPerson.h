// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements the first person pit view for vehicles

   -------------------------------------------------------------------------
   История:
   - 29:01:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEVIEWFIRSTPERSON_H__
#define __VEHICLEVIEWFIRSTPERSON_H__

#include "VehicleViewBase.h"

class CVehicleViewFirstPerson
	: public CVehicleViewBase
{
	IMPLEMENT_VEHICLEOBJECT;
public:

	CVehicleViewFirstPerson();
	virtual ~CVehicleViewFirstPerson() {}

	// IVehicleView
	virtual bool        Init(IVehicleSeat* pSeat, const CVehicleParams& table) override;
	virtual void        Reset() override;

	virtual tukk GetName() override       { return m_name; }
	virtual bool        IsThirdPerson() override { return false; }

	virtual void        OnStartUsing(EntityId playerId) override;
	virtual void        OnStopUsing() override;

	virtual void        Update(const float frameTime) override;
	virtual void        UpdateView(SViewParams& viewParams, EntityId playerId = 0) override;

	virtual void        GetMemoryUsage(IDrxSizer* s) const override;

	void                OffsetPosition(const Vec3& delta) override
	{
		m_viewPosition += delta;
	}
	// ~IVehicleView

	// IVehicleEventListener
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) override;
	// ~IVehicleEventListener

protected:

	virtual Vec3 GetWorldPosGoal();
	virtual Quat GetWorldRotGoal();
	Quat         GetVehicleRotGoal();

	void         HideEntitySlots(IEntity* pEnt, bool hide);

	static tukk m_name;

	IVehicleHelper*    m_pHelper;
	string             m_sCharacterBoneName;
	Vec3               m_offset;
	float              m_fov;
	float              m_relToHorizon;
	bool               m_hideVehicle;
	i32                m_frameSlot;
	Matrix34           m_invFrame;
	Vec3               m_frameObjectOffset;

	typedef std::multimap<EntityId, i32> TSlots;
	TSlots   m_slotFlags;

	Vec3     m_viewPosition;
	Quat     m_viewRotation;

	float    m_speedRot;

	EntityId m_passengerId;
};

#endif
