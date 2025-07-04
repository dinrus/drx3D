// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a part for vehicles which is the an attachment
   of a parent Animated part

   -------------------------------------------------------------------------
   История:
   - 25:08:2005: Created by Mathieu Pinard

*************************************************************************/

#ifndef __VEHICLEPARTSUBPART_H__
#define __VEHICLEPARTSUBPART_H__

#include "VehiclePartBase.h"

class CVehicle;

class CVehiclePartSubPart : public CVehiclePartBase
{
	IMPLEMENT_VEHICLEOBJECT

public:

	CVehiclePartSubPart();

	virtual ~CVehiclePartSubPart();

	// IVehiclePart
	virtual bool     Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* pParent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;
	virtual void     Reset() override;
	virtual void     Release() override;
	virtual void     OnEvent(const SVehiclePartEvent& event) override;
	virtual bool     ChangeState(EVehiclePartState state, i32 flags = 0) override;
	virtual void     Physicalize() override;
	virtual void     Update(const float frameTime) override;
	virtual void     GetMemoryUsage(IDrxSizer* pSizer) const override;
	virtual void     GetMemoryUsageInternal(IDrxSizer* pSizer) const;
	virtual Matrix34 GetLocalInitialTM() override;
	// ~IVehiclePart

protected:

	virtual void InitGeometry();

	void         RegisterStateGeom(EVehiclePartState state, IStatObj* pStatObj);

	Matrix34 m_savedTM;

private:

	struct StateGeom
	{
		inline StateGeom() : state(eVGS_Default), pStatObj(NULL)
		{
		}

		inline StateGeom(EVehiclePartState state, IStatObj* pStatObj) : state(state), pStatObj(pStatObj)
		{
			if (pStatObj)
			{
				pStatObj->AddRef();
			}
		}

		inline StateGeom(const StateGeom& other) : state(other.state), pStatObj(other.pStatObj)
		{
			if (pStatObj)
			{
				pStatObj->AddRef();
			}
		}

		~StateGeom()
		{
			if (pStatObj)
			{
				pStatObj->Release();
			}
		}

		inline StateGeom& operator=(const StateGeom& other)
		{
			state = other.state;
			pStatObj = other.pStatObj;

			if (pStatObj)
			{
				pStatObj->AddRef();
			}

			return *this;
		}

		EVehiclePartState state;
		IStatObj*         pStatObj;
	};

	typedef std::vector<StateGeom> TStateGeomVector;

	TStateGeomVector m_stateGeoms;
};

#endif //__VEHICLEPARTSUBPART_H__
