// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "FlowVehicleBase.h"

class CFlowVehicleTurret
	: public CFlowVehicleBase
{
public:

	CFlowVehicleTurret(SActivationInfo* pActivationInfo)
	{
		Init(pActivationInfo);
	}

	~CFlowVehicleTurret() { Delete(); }

	// CFlowBaseNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActivationInfo);
	virtual void         GetConfiguration(SFlowNodeConfig& nodeConfig);
	virtual void         ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo);
	virtual void         Serialize(SActivationInfo* pActivationInfo, TSerialize ser);
	// ~CFlowBaseNode

	// IVehicleEventListener
	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params);
	// ~IVehicleEventListener

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}

protected:

	enum EInputs
	{
		eIn_Trigger,
		eIn_Seat,
		eIn_AimPos,
	};

	enum EOutputs
	{
		eOut_Success,
	};
};
