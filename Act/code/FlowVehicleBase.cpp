// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/FlowVehicleBase.h>
#include <drx3D/FlowGraph/IFlowBaseNode.h>

//------------------------------------------------------------------------
void CFlowVehicleBase::Init(SActivationInfo* pActivationInfo)
{
	m_nodeID = pActivationInfo->myID;
	m_pGraph = pActivationInfo->pGraph;

	if (IEntity* pEntity = pActivationInfo->pEntity)
	{
		IVehicleSystem* pVehicleSystem = CDrxAction::GetDrxAction()->GetIVehicleSystem();
		DRX_ASSERT(pVehicleSystem);

		if (pVehicleSystem->GetVehicle(pEntity->GetId()))
			m_vehicleId = pEntity->GetId();
	}
	else
		m_vehicleId = 0;
}

//------------------------------------------------------------------------
void CFlowVehicleBase::Delete()
{
	if (IVehicle* pVehicle = GetVehicle())
		pVehicle->UnregisterVehicleEventListener(this);
}

//------------------------------------------------------------------------
void CFlowVehicleBase::GetConfiguration(SFlowNodeConfig& nodeConfig)
{
}

//------------------------------------------------------------------------
void CFlowVehicleBase::ProcessEvent(EFlowEvent flowEvent, SActivationInfo* pActivationInfo)
{
	if (flowEvent == eFE_SetEntityId)
	{
		IEntity* pEntity = pActivationInfo->pEntity;

		if (pEntity)
		{
			IVehicleSystem* pVehicleSystem = CDrxAction::GetDrxAction()->GetIVehicleSystem();
			DRX_ASSERT(pVehicleSystem);

			if (pEntity->GetId() != m_vehicleId)
			{
				if (IVehicle* pVehicle = GetVehicle())
					pVehicle->UnregisterVehicleEventListener(this);

				m_vehicleId = 0;
			}

			if (IVehicle* pVehicle = pVehicleSystem->GetVehicle(pEntity->GetId()))
			{
				pVehicle->RegisterVehicleEventListener(this, "FlowVehicleBase");
				m_vehicleId = pEntity->GetId();
			}
		}
		else
		{
			if (IVehicle* pVehicle = GetVehicle())
				pVehicle->UnregisterVehicleEventListener(this);
		}
	}
}

//------------------------------------------------------------------------
void CFlowVehicleBase::Serialize(SActivationInfo* pActivationInfo, TSerialize ser)
{
}

//------------------------------------------------------------------------
void CFlowVehicleBase::OnVehicleEvent(EVehicleEvent event, const SVehicleEventParams& params)
{
	if (event == eVE_VehicleDeleted)
		m_vehicleId = 0;
}

//------------------------------------------------------------------------
IVehicle* CFlowVehicleBase::GetVehicle()
{
	if (!m_vehicleId)
		return NULL;

	return CDrxAction::GetDrxAction()->GetIVehicleSystem()->GetVehicle(m_vehicleId);
}
