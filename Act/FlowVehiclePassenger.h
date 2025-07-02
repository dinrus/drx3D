// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a flow node to handle a vehicle passenger

   -------------------------------------------------------------------------
   История:
   - 09:12:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __FLOWVEHICLEPASSENGER_H__
#define __FLOWVEHICLEPASSENGER_H__

#include "FlowVehicleBase.h"

class CFlowVehiclePassenger
	: public CFlowVehicleBase
{
public:

	CFlowVehiclePassenger(SActivationInfo* pActivationInfo);
	~CFlowVehiclePassenger() { Delete(); }

	// CFlowBaseNode
	virtual IFlowNodePtr Clone(SActivationInfo* pActivationInfo);
	virtual void         GetConfiguration(SFlowNodeConfig& nodeConfig);
	virtual void         ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo);
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
		IN_TRIGGERPASSENGERIN,
		IN_TRIGGERPASSENGEROUT,
		IN_ACTORID,
		IN_SEATID,
	};

	enum EOutputs
	{
		OUT_ACTORIN,
		OUT_ACTOROUT,
	};

	EntityId       m_actorId;
	TVehicleSeatId m_seatId;
	i32            m_passengerCount;
};

#endif
