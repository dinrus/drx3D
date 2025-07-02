// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a flow node to toggel vehicle lights

   -------------------------------------------------------------------------
   История:
   - 03:03:2007: Created by MichaelR

*************************************************************************/
#ifndef __FLOWVEHICLELights_H__
#define __FLOWVEHICLELights_H__

#include "FlowVehicleBase.h"

class CFlowVehicleLights
	: public CFlowVehicleBase
{
public:

	CFlowVehicleLights(SActivationInfo* pActivationInfo) { Init(pActivationInfo); }
	~CFlowVehicleLights() { Delete(); }

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
		eIn_LightType = 0,
		eIn_Activate,
		eIn_Deactivate
	};
};

#endif
