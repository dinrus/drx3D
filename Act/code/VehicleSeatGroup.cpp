// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat group

   -------------------------------------------------------------------------
   История:
   - 12:03:2006: Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/IActorSystem.h>
#include <drx3D/Act/IAnimatedCharacter.h>
#include <drx3D/Act/IVehicleSystem.h>
#include <drx3D/Act/Vehicle.h>
#include <drx3D/Act/VehicleSeat.h>
#include <drx3D/Act/VehicleSeatGroup.h>

//------------------------------------------------------------------------
bool CVehicleSeatGroup::Init(IVehicle* pVehicle, const CVehicleParams& paramsTable)
{
	m_pVehicle = static_cast<CVehicle*>(pVehicle);
	m_isSwitchingReverse = false;

	if (CVehicleParams seatsTable = paramsTable.findChild("Seats"))
	{
		i32 i = 0;
		i32 c = seatsTable.getChildCount();
		m_seats.reserve(c);

		for (; i < c; i++)
		{
			string seatName = seatsTable.getChild(i).getAttr("value");
			if (!seatName.empty())
			{
				TVehicleSeatId seatId = m_pVehicle->GetSeatId(seatName);
				if (CVehicleSeat* pSeat = (CVehicleSeat*)m_pVehicle->GetSeatById(seatId))
				{
					pSeat->SetSeatGroup(this);
					m_seats.push_back(pSeat);
				}
			}
		}
	}

	return !m_seats.empty();
}

//------------------------------------------------------------------------
void CVehicleSeatGroup::Reset()
{
	m_isSwitchingReverse = false;
}

//------------------------------------------------------------------------
CVehicleSeat* CVehicleSeatGroup::GetSeatByIndex(u32 index)
{
	if (index < m_seats.size())
		return m_seats[index];

	return NULL;
}

//------------------------------------------------------------------------
CVehicleSeat* CVehicleSeatGroup::GetNextSeat(CVehicleSeat* pCurrentSeat)
{
	for (TVehicleSeatVector::iterator ite = m_seats.begin(); ite != m_seats.end(); ++ite)
	{
		CVehicleSeat* pSeat = *ite;
		if (pSeat == pCurrentSeat)
		{
			if (m_isSwitchingReverse)
			{
				if (ite == m_seats.begin())
				{
					m_isSwitchingReverse = false;
					++ite;
				}
				else
					--ite;
			}
			else
			{
				++ite;

				if (ite == m_seats.end())
				{
					m_isSwitchingReverse = true;
					--ite;
					--ite;
				}
			}

			return *ite;
		}
	}

	return NULL;
}

//------------------------------------------------------------------------
CVehicleSeat* CVehicleSeatGroup::GetNextFreeSeat(CVehicleSeat* pCurrentSeat)
{
	TVehicleSeatVector::iterator itCurrent = std::find(m_seats.begin(), m_seats.end(), pCurrentSeat);

	if (itCurrent == m_seats.end())
		return NULL;

	TVehicleSeatVector::iterator itFwd = itCurrent, itBack = itCurrent;

	IActor* pActor = pCurrentSeat->GetPassengerActor();

	while (!(itFwd == m_seats.end() && itBack == m_seats.begin()))
	{
		if (m_isSwitchingReverse)
		{
			if (itBack != m_seats.begin())
			{
				--itBack;

				if ((*itBack)->IsFree(pActor))
					return *itBack;
			}
			else
			{
				m_isSwitchingReverse = false;
			}
		}
		else
		{
			if (itFwd != m_seats.end())
			{
				++itFwd;

				if (itFwd == m_seats.end())
				{
					m_isSwitchingReverse = true;
				}
				else
				{
					if ((*itFwd)->IsFree(pActor))
						return *itFwd;
				}
			}
			else
			{
				m_isSwitchingReverse = true;
			}
		}
	}

	return NULL;
}

//------------------------------------------------------------------------
bool CVehicleSeatGroup::IsGroupEmpty()
{
	const TVehicleSeatVector::const_iterator seatsEnd = m_seats.end();
	for (TVehicleSeatVector::const_iterator ite = m_seats.begin(); ite != seatsEnd; ++ite)
	{
		CVehicleSeat* pSeat = *ite;
		if (!pSeat->IsFree(NULL))
			return false;
	}

	return true;
}

//------------------------------------------------------------------------
void CVehicleSeatGroup::OnPassengerExit(CVehicleSeat* pSeat, EntityId passengerId)
{

}

//------------------------------------------------------------------------
void CVehicleSeatGroup::OnPassengerChangeSeat(CVehicleSeat* pNewSeat, CVehicleSeat* pOldSeat)
{

}

void CVehicleSeatGroup::GetMemoryUsage(IDrxSizer* s) const
{
	s->Add(*this);
	s->AddContainer(m_seats);
}
