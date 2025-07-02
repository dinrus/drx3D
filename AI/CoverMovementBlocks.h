// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/MovementBlock.h>
#include <drx3D/AI/ICoverSystem.h>

class CEntityAICoverUserComponent;

class LeaveCoverMovementBlock : public Movement::Block
{
public:
	LeaveCoverMovementBlock(CEntityAICoverUserComponent* pComponentOwner)
		: m_pComponentOwner(pComponentOwner)
	{}

	virtual tukk GetName() const override { return "LeaveCover"; }
	virtual void        Begin(IMovementActor& actor) override;

private:
	CEntityAICoverUserComponent* m_pComponentOwner;
};

class SetupCoverMovementBlock : public Movement::Block
{
public:
	SetupCoverMovementBlock(CEntityAICoverUserComponent* pComponentOwner, CoverID coverId)
		: m_pComponentOwner(pComponentOwner)
		, m_coverId(coverId)
	{}

	virtual tukk GetName() const override { return "SetCoverInfo"; }
	virtual void        Begin(IMovementActor& actor) override;

private:
	CEntityAICoverUserComponent* m_pComponentOwner;
	CoverID                      m_coverId;
};

class EnterCoverMovementBlock : public Movement::Block
{
public:
	EnterCoverMovementBlock(CEntityAICoverUserComponent* pComponentOwner)
		: m_pComponentOwner(pComponentOwner)
	{}

	virtual tukk GetName() const override { return "EnterCover"; }
	virtual void        Begin(IMovementActor& actor) override;
private:
	CEntityAICoverUserComponent* m_pComponentOwner;
};
