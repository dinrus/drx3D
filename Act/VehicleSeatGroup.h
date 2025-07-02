// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a seat group

   -------------------------------------------------------------------------
   История:
   - 12:03:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLESEATGROUP_H__
#define __VEHICLESEATGROUP_H__

#include "vector"

class CVehicleSeat;

class CVehicleSeatGroup
{
public:

	bool          Init(IVehicle* pVehicle, const CVehicleParams& paramsTable);
	void          Reset();
	void          Release() { delete this; }

	void          GetMemoryUsage(IDrxSizer* s) const;

	u32  GetSeatCount() { return m_seats.size(); }
	CVehicleSeat* GetSeatByIndex(u32 index);
	CVehicleSeat* GetNextSeat(CVehicleSeat* pCurrentSeat);
	CVehicleSeat* GetNextFreeSeat(CVehicleSeat* pCurrentSeat);

	bool          IsGroupEmpty();

	void          OnPassengerEnter(CVehicleSeat* pSeat, EntityId passengerId) {}
	void          OnPassengerExit(CVehicleSeat* pSeat, EntityId passengerId);
	void          OnPassengerChangeSeat(CVehicleSeat* pNewSeat, CVehicleSeat* pOldSeat);

protected:

	CVehicle* m_pVehicle;

	typedef std::vector<CVehicleSeat*> TVehicleSeatVector;
	TVehicleSeatVector m_seats;

	bool               m_isSwitchingReverse;

	friend class CVehicleSeat;
};

#endif
