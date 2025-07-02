// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a part for vehicles which uses static objects

   -------------------------------------------------------------------------
   История:
   - 23:08:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEPARTSTATIC_H__
#define __VEHICLEPARTSTATIC_H__

class CVehicle;

class CVehiclePartStatic
	: public CVehiclePartBase
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehiclePartStatic() {}
	~CVehiclePartStatic() {}

	// IVehiclePart
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* parent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;
	virtual void InitGeometry();
	virtual void Release() override;
	virtual void Reset() override;

	virtual void Physicalize() override;

	virtual void Update(const float frameTime) override;

	virtual void SetLocalTM(const Matrix34& localTM) override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override
	{
		s->Add(*this);
		s->AddObject(m_filename);
		s->AddObject(m_filenameDestroyed);
		s->AddObject(m_geometry);
		CVehiclePartBase::GetMemoryUsageInternal(s);
	}
	// ~IVehiclePart

protected:

	string m_filename;
	string m_filenameDestroyed;
	string m_geometry;
};

#endif
