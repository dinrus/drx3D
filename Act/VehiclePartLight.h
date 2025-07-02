// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a light part

   -------------------------------------------------------------------------
   История:
   - 24:10:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEPARTLIGHT_H__
#define __VEHICLEPARTLIGHT_H__

#include "VehiclePartBase.h"

class CVehiclePartLight
	: public CVehiclePartBase
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehiclePartLight();
	~CVehiclePartLight();

	// IVehiclePart
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* parent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;
	virtual void PostInit() override;
	virtual void Reset() override;

	virtual void OnEvent(const SVehiclePartEvent& event) override;

	virtual void Physicalize() override {}

	virtual void Update(const float frameTime) override;

	virtual void Serialize(TSerialize serialize, EEntityAspects) override;
	virtual void RegisterSerializer(IGameObjectExtension* gameObjectExt) override {}
	virtual void GetMemoryUsage(IDrxSizer* s) const override
	{
		s->Add(*this);
		s->AddObject(m_components);
		s->AddObject(m_pHelper);
		CVehiclePartBase::GetMemoryUsageInternal(s);
	}
	// ~IVehiclePart

	virtual void  ToggleLight(bool enable);
	bool          IsEnabled()          { return m_enabled; }
	const string& GetLightType() const { return m_lightType; }

protected:

	virtual void UpdateLight(const float frameTime);

	string                          m_lightType;
	SRenderLight                         m_light;
	i32                             m_lightViewDistanceRatio;
	_smart_ptr<IMaterial>           m_pMaterial;

	std::vector<IVehicleComponent*> m_components;

	IVehicleHelper*                 m_pHelper;

	float                           m_diffuseMult[2];
	Vec3                            m_diffuseCol;

	bool                            m_enabled;
};

#endif
