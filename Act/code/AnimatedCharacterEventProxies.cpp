// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/AnimatedCharacterEventProxies.h>
#include <drx3D/Act/AnimatedCharacter.h>

#include <drx3D/Act/DrxActionCVars.h>

#include <drx3D/Entity/IEntityComponent.h>

#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>

//////////////////////////////////////////////////////////////////////////

DRXREGISTER_CLASS(CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate)
DRXREGISTER_CLASS(CAnimatedCharacterComponent_StartAnimProc)
DRXREGISTER_CLASS(CAnimatedCharacterComponent_GenerateMoveRequest)

CAnimatedCharacterComponent_Base::CAnimatedCharacterComponent_Base()
	: m_pAnimCharacter(nullptr)
{
}

void CAnimatedCharacterComponent_Base::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_PREPHYSICSUPDATE:
		OnPrePhysicsUpdate(event.fParam[0]);
		break;
	}
}

uint64 CAnimatedCharacterComponent_Base::GetEventMask() const
{
	return ENTITY_EVENT_BIT(ENTITY_EVENT_PREPHYSICSUPDATE);
}

//////////////////////////////////////////////////////////////////////////

CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate()
	: m_queuedRotation(IDENTITY)
	, m_hasQueuedRotation(false)
{
}

void CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::OnPrePhysicsUpdate(float)
{
	DRX_ASSERT(m_pAnimCharacter);

	if (CAnimationGraphCVars::Get().m_useQueuedRotation && m_hasQueuedRotation)
	{
		m_pEntity->SetRotation(m_queuedRotation, { ENTITY_XFORM_USER, ENTITY_XFORM_NOT_REREGISTER });
		ClearQueuedRotation();
	}

	m_pAnimCharacter->PrepareAnimatedCharacterForUpdate();
}

IEntityComponent::ComponentEventPriority CAnimatedCharacterComponent_PrepareAnimatedCharacterForUpdate::GetEventPriority() const
{
	return ENTITY_PROXY_USER + EEntityEventPriority_PrepareAnimatedCharacterForUpdate;
}

//////////////////////////////////////////////////////////////////////////

void CAnimatedCharacterComponent_StartAnimProc::OnPrePhysicsUpdate(float elapsedTime)
{
	DRX_ASSERT(m_pAnimCharacter);

	m_pAnimCharacter->PrepareAndStartAnimProc();
}

IEntityComponent::ComponentEventPriority CAnimatedCharacterComponent_StartAnimProc::GetEventPriority() const
{
	return ENTITY_PROXY_USER + EEntityEventPriority_StartAnimProc;
}

//////////////////////////////////////////////////////////////////////////

void CAnimatedCharacterComponent_GenerateMoveRequest::OnPrePhysicsUpdate(float elapsedTime)
{
	DRX_ASSERT(m_pAnimCharacter);

	m_pAnimCharacter->GenerateMovementRequest();
}

IEntityComponent::ComponentEventPriority CAnimatedCharacterComponent_GenerateMoveRequest::GetEventPriority() const
{
	return ENTITY_PROXY_USER + EEntityEventPriority_AnimatedCharacter;
}
