// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Implements a dummy vehicle movement, for prop vehicles

-------------------------------------------------------------------------
История:
- 05:03:2010: Created by Steve Humphreys

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/VehicleMovementDummy.h>

#include <drx3D/Game/GameCVars.h>

bool CVehicleMovementDummy::Init(IVehicle* pVehicle, const CVehicleParams& table)
{
	m_pVehicle = pVehicle;
	return true;
}

void CVehicleMovementDummy::Release()
{
	delete this;
}

void CVehicleMovementDummy::Physicalize()
{
	SEntityPhysicalizeParams physicsParams(m_pVehicle->GetPhysicsParams());	

	physicsParams.type = PE_RIGID;	  

	m_pVehicle->GetEntity()->Physicalize(physicsParams);
}