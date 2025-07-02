// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/CoverMovementBlocks.h>

#include <drx3D/AI/CoverUserComponent.h>

void LeaveCoverMovementBlock::Begin(IMovementActor& actor)
{
	m_pComponentOwner->SetState(ICoverUser::EStateFlags::None);
	m_pComponentOwner->SetCurrentCover(CoverID());
}

void SetupCoverMovementBlock::Begin(IMovementActor& actor)
{
	m_pComponentOwner->ReserveNextCover(CoverID());
	m_pComponentOwner->SetCurrentCover(m_coverId);
	m_pComponentOwner->SetState(ICoverUser::EStateFlags::MovingToCover);
}

void EnterCoverMovementBlock::Begin(IMovementActor& actor)
{
	m_pComponentOwner->SetState(ICoverUser::EStateFlags::InCover);
}