// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a wheel vehicle part

   -------------------------------------------------------------------------
   История:
   - 25:08:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEPARTSUBPARTWHEEL_H__
#define __VEHICLEPARTSUBPARTWHEEL_H__

#include "VehiclePartSubPart.h"

class CVehicle;

class CVehiclePartSubPartWheel
	: public CVehiclePartSubPart
	  , public IVehicleWheel
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehiclePartSubPartWheel();
	virtual ~CVehiclePartSubPartWheel() {}

	// IVehiclePart
	virtual void           Reset() override;
	virtual bool           Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* parent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;

	virtual bool           ChangeState(EVehiclePartState state, i32 flags = 0) override;
	virtual void           Physicalize() override;
	virtual void           OnEvent(const SVehiclePartEvent& event) override;

	virtual void           Serialize(TSerialize ser, EEntityAspects aspects) override;
	virtual void           PostSerialize() override;

	virtual IVehicleWheel* GetIWheel() override { return this; }
	virtual void           GetMemoryUsage(IDrxSizer* s) const override
	{
		s->Add(*this);
		CVehiclePartSubPart::GetMemoryUsageInternal(s);
	}
	// ~IVehiclePart

	// IVehicleWheel
	virtual i32                     GetSlot() const override          { return m_slot; }
	virtual i32                     GetWheelIndex() const override    { return m_pSharedParameters->m_wheelIndex; }
	virtual float                   GetTorqueScale() const override   { return m_pSharedParameters->m_torqueScale; }
	virtual float                   GetSlipFrictionMod(float slip) const override;
	virtual const pe_cargeomparams* GetCarGeomParams() const override { return &m_physGeomParams; }
	// ~IVehicleWheel

	float        GetRimRadius() const { return m_pSharedParameters->m_rimRadius; }

	virtual void GetGeometryName(EVehiclePartState state, string& name) override;

protected:

	virtual void InitGeometry() override;
	bool         IsFront() const { return m_physGeomParams.pivot.y > 0.f; }

	pe_cargeomparams m_physGeomParams;

	friend class CVehiclePartTread;
	friend class CVehicle;
};

#endif
