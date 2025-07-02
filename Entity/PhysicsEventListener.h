// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// forward declaration.
class CEntitySystem;
struct IPhysicalWorld;

//////////////////////////////////////////////////////////////////////////
class CPhysicsEventListener
{
public:
	explicit CPhysicsEventListener(IPhysicalWorld* pPhysics);
	~CPhysicsEventListener();

	static i32 OnStateChange(const EventPhys* pEvent);
	static i32 OnPostStep(const EventPhys* pEvent);
	static i32 OnPostStepImmediate(const EventPhys* pEvent);
	static i32 OnUpdateMesh(const EventPhys* pEvent);
	static i32 OnCreatePhysEntityPart(const EventPhys* pEvent);
	static i32 OnRemovePhysEntityParts(const EventPhys* pEvent);
	static i32 OnRevealPhysEntityPart(const EventPhys* pEvent);
	static i32 OnPreUpdateMesh(const EventPhys* pEvent);
	static i32 OnPreCreatePhysEntityPart(const EventPhys* pEvent);
	static i32 OnCollisionLogged(const EventPhys* pEvent);
	static i32 OnCollisionImmediate(const EventPhys* pEvent);
	static i32 OnJointBreak(const EventPhys* pEvent);
	static i32 OnPostPump(const EventPhys* pEvent);

	void       RegisterPhysicCallbacks();
	void       UnregisterPhysicCallbacks();

protected:
	static void SendCollisionEventToEntity(SEntityEvent& event);

private:
	static CEntity* GetEntity(uk pForeignData, i32 iForeignData);
	static CEntity* GetEntity(IPhysicalEntity* pPhysEntity);

	IPhysicalWorld*                      m_pPhysics;

	static std::vector<IPhysicalEntity*> m_physVisAreaUpdateVector;
	static i32                           m_jointFxFrameId, m_jointFxCount;
	static i32 FxAllowed()
	{
		i32 frameId = gEnv->pRenderer->GetFrameID();
		m_jointFxCount &= -iszero(frameId - m_jointFxFrameId);
		m_jointFxFrameId = frameId;
		return m_jointFxCount < CVar::es_MaxJointFx ? ++m_jointFxCount : 0;
	}
};
