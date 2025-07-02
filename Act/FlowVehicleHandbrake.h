// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a flow node to enable/disable vehicle handbrake.

   -------------------------------------------------------------------------
   История:
   - 15:04:2010: Created by Paul Slinger

*************************************************************************/

#ifndef __FLOW_VEHICLE_HANDBRAKE_H__
#define __FLOW_VEHICLE_HANDBRAKE_H__

#include "FlowVehicleBase.h"

class CFlowVehicleHandbrake : public CFlowVehicleBase
{
public:

	CFlowVehicleHandbrake(SActivationInfo* pActivationInfo);

	virtual ~CFlowVehicleHandbrake();

	// CFlowBaseNode

	virtual IFlowNodePtr Clone(SActivationInfo* pActivationInfo);

	virtual void         GetConfiguration(SFlowNodeConfig& nodeConfig);

	virtual void         ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo);

	// ~CFlowBaseNode

	// IVehicleEventListener

	virtual void OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params);

	// ~IVehicleEventListener

	virtual void GetMemoryUsage(IDrxSizer* pDrxSizer) const;

protected:

	enum EInputs
	{
		eIn_Activate = 0,
		eIn_Deactivate,
		eIn_ResetTimer
	};
};

#endif //__FLOW_VEHICLE_HANDBRAKE_H__
