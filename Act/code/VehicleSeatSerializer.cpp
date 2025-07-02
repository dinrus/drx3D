// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements an entity class which can serialize vehicle parts

   -------------------------------------------------------------------------
   История:
   - 16:09:2005: Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/VehicleSeatSerializer.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleSeat.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameContext.h>

//------------------------------------------------------------------------
CVehicleSeatSerializer::CVehicleSeatSerializer()
	: m_pVehicle(0),
	m_pSeat(0)
{
}

//------------------------------------------------------------------------
CVehicleSeatSerializer::~CVehicleSeatSerializer()
{
}

//------------------------------------------------------------------------
bool CVehicleSeatSerializer::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	if (m_pSeat) // needs to be done here since anywhere earlier, EntityId is not known
		m_pSeat->SetSerializer(this);
	else if (!gEnv->bServer)
		return false;

	CVehicleSeat* pSeat = NULL;
	EntityId parentId = 0;
	if (gEnv->bServer)
	{
		pSeat = static_cast<CVehicleSystem*>(CDrxAction::GetDrxAction()->GetIVehicleSystem())->GetInitializingSeat();
		DRX_ASSERT(pSeat);
		SetVehicle(static_cast<CVehicle*>(pSeat->GetVehicle()));
		parentId = m_pVehicle->GetEntityId();
	}

	if (0 == (GetEntity()->GetFlags() & (ENTITY_FLAG_CLIENT_ONLY | ENTITY_FLAG_SERVER_ONLY)))
		if (!GetGameObject()->BindToNetworkWithParent(eBTNM_Normal, parentId))
			return false;

	GetEntity()->Hide(true);

	if (!IsDemoPlayback())
	{
		if (gEnv->bServer)
		{
			pSeat->SetSerializer(this);
			SetSeat(pSeat);
		}
		else
		{
			if (m_pVehicle)
			{
				GetGameObject()->SetNetworkParent(m_pVehicle->GetEntityId());
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::InitClient(i32 channelId)
{
}

//------------------------------------------------------------------------
bool CVehicleSeatSerializer::ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params)
{
	ResetGameObject();

	DRX_ASSERT_MESSAGE(false, "CVehicleSeatSerializer::ReloadExtension not implemented");

	return false;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::FullSerialize(TSerialize ser)
{
}

//------------------------------------------------------------------------
bool CVehicleSeatSerializer::NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags)
{
	if (m_pSeat)
		m_pSeat->SerializeActions(ser, aspect);
	return true;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::Update(SEntityUpdateContext& ctx, i32)
{
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::HandleEvent(const SGameObjectEvent& event)
{
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::SerializeSpawnInfo(TSerialize ser)
{
	EntityId vehicle;
	TVehicleSeatId seatId;

	ser.Value("vehicle", vehicle, 'eid');
	ser.Value("seat", seatId, 'seat');

	DRX_ASSERT(ser.IsReading());

	// warning GameObject not set at this point
	// GetGameObject calls will fail miserably

	m_pVehicle = static_cast<CVehicle*>(CDrxAction::GetDrxAction()->GetIVehicleSystem()->GetVehicle(vehicle));
	if (!m_pVehicle)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to find vehicle with id '%u' for the seat id '%d', this will most likely result in context corruption", vehicle, seatId);
		return;
	}

	CVehicleSeat* pSeat = static_cast<CVehicleSeat*>(m_pVehicle->GetSeatById(seatId));
	m_pSeat = pSeat;

	// the serializer is set on the seat in ::Init
}

namespace CVehicleSeatSerializerGetSpawnInfo
{
struct SInfo : public ISerializableInfo
{
	EntityId       vehicle;
	TVehicleSeatId seatId;
	void SerializeWith(TSerialize ser)
	{
		ser.Value("vehicle", vehicle, 'eid');
		ser.Value("seat", seatId, 'seat');
	}
};
}

//------------------------------------------------------------------------
ISerializableInfoPtr CVehicleSeatSerializer::GetSpawnInfo()
{

	CVehicleSeatSerializerGetSpawnInfo::SInfo* p = new CVehicleSeatSerializerGetSpawnInfo::SInfo;
	p->vehicle = m_pVehicle->GetEntityId();
	p->seatId = m_pSeat->GetSeatId();
	return p;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::SetVehicle(CVehicle* pVehicle)
{
	m_pVehicle = pVehicle;
}

//------------------------------------------------------------------------
void CVehicleSeatSerializer::SetSeat(CVehicleSeat* pSeat)
{
	m_pSeat = pSeat;
}
