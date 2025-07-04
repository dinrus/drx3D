// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a flow node to handle a vehicle movement features

   -------------------------------------------------------------------------
   История:
   - 11:07:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __FLOWVEHICLEMOVEMENT_H__
#define __FLOWVEHICLEMOVEMENT_H__

#include "FlowVehicleBase.h"

class CFlowVehicleMovement
	: public CFlowVehicleBase
{
public:

	CFlowVehicleMovement(SActivationInfo* pActivationInfo)
	{
		Init(pActivationInfo);
		m_shouldZeroMass = false;
	}
	~CFlowVehicleMovement() { Delete(); }

	// CFlowBaseNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActivationInfo);
	virtual void         GetConfiguration(SFlowNodeConfig& nodeConfig);
	virtual void         ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo);
	virtual void         Serialize(SActivationInfo* pActivationInfo, TSerialize ser);
	// ~CFlowBaseNode

	// IVehicleEventListener
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) {}
	// ~IVehicleEventListener

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}

protected:

	enum EInputs
	{
		In_WarmupEngine,
		In_ZeroMass,
		In_RestoreMass,
	};

	enum EOutputs
	{
	};

	void ZeroMass(bool enable);

	struct SPartMass
	{
		i32   partid;
		float mass;

		SPartMass() : partid(0), mass(0.0f) {}
		SPartMass(i32 id, float m) : partid(id), mass(m) {}
	};

	std::vector<SPartMass> m_partMass;
	bool                   m_shouldZeroMass; // on loading defer ZeroMass(true) call to next update (after vehicle physicalisation)
};

//////////////////////////////////////////////////////////////////////////

class CFlowVehicleMovementParams
	: public CFlowVehicleBase
{
public:

	CFlowVehicleMovementParams(SActivationInfo* pActivationInfo)
	{
		Init(pActivationInfo);
	}
	~CFlowVehicleMovementParams() { Delete(); }

	// CFlowBaseNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActivationInfo);
	virtual void         GetConfiguration(SFlowNodeConfig& nodeConfig);
	virtual void         ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo);
	// ~CFlowBaseNode

	// IVehicleEventListener
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params) {}
	// ~IVehicleEventListener

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}

protected:

	enum EInputs
	{
		In_Trigger = 0,
		In_MaxSpeedFactor,
		In_MaxAcceleration
	};

	enum EOutputs
	{
	};
};

#endif
