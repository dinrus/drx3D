// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef Agent_h
#define Agent_h

#include <drx3D/AI/IAgent.h>
#include <drx3D/AI/IAIActorProxy.h>
#include <drx3D/Entity/IEntity.h>
#include <drx3D/AI/IAIActor.h>
#include <drx3D/AI/IAIObject.h>
#include <drx3D/CoreX/Math/Drx_Vector3.h>
#include <drx3D/Animation/IDrxAnimation.h>

struct VisionID;
struct IAnimationGraphState;

// Serves as a unified interface for an agent
class Agent
{
public:
	Agent(IAIObject* pAIObject)
		: m_pAIObject(NULL)
		, m_pAIActor(NULL)
	{
		if (pAIObject)
		{
			SetFrom(pAIObject);
		}
	}

	Agent(EntityId entityID)
		: m_pAIObject(NULL)
		, m_pAIActor(NULL)
	{
		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityID))
		{
			if (IAIObject* pAIObject = pEntity->GetAI())
			{
				SetFrom(pAIObject);
			}
		}
	}

	Agent(IEntity* pEntity)
		: m_pAIObject(NULL)
		, m_pAIActor(NULL)
	{
		if (pEntity != NULL)
		{
			if (IAIObject* pAIObject = pEntity->GetAI())
			{
				SetFrom(pAIObject);
			}
		}
	}


	void SetFrom(IAIObject* pAIObject)
	{
		if (IAIActor* pAIActor = pAIObject->CastToIAIActor())
		{
			m_pAIObject = pAIObject;
			m_pAIActor = pAIActor;
			return;
		}

		m_pAIObject = NULL;
		m_pAIActor = NULL;
	}

	IAIObject* GetAIObject() const
	{
		return m_pAIObject;
	}

	IAIActor* GetAIActor() const
	{
		return m_pAIActor;
	}

	tAIObjectID GetAIObjectID() const
	{
		return m_pAIObject->GetAIObjectID();
	}

	const Vec3& GetPos() const
	{
		return m_pAIObject->GetPos();
	}

	const Vec3 GetEntityPos() const
	{
		return m_pAIObject->GetEntity()->GetWorldPos();
	}

	const Vec3& GetEntityForwardDir() const
	{
		return m_pAIObject->GetEntity()->GetForwardDir();
	}

	const Vec3 GetVelocity() const
	{
		return m_pAIObject->GetVelocity();
	}

	IEntity* GetEntity()
	{
		return m_pAIObject->GetEntity();
	}

	const IEntity* GetEntity() const
	{
		return m_pAIObject->GetEntity();
	}

	const Vec3& GetViewDir() const
	{
		return m_pAIObject->GetViewDir();
	}

	i32 GetGroupID() const
	{
		return m_pAIObject->GetGroupId();
	}

	const VisionID& GetVisionID() const
	{
		return m_pAIObject->GetVisionID();
	}

	bool CanSee(const VisionID& otherVisionID) const
	{
		assert(m_pAIActor);
		return m_pAIActor->CanSee(otherVisionID);
	}

	bool CanSee(const Agent& otherAgent) const
	{
		assert(m_pAIActor);
		return m_pAIActor->CanSee(otherAgent.GetVisionID());
	}

 	EntityId GetEntityID() const
 	{
 		return m_pAIObject->GetEntityID();
 	}

	bool IsValid() const
	{
		return m_pAIObject && m_pAIActor;
	}

	bool IsEnabled() const
	{
		return m_pAIObject->IsEnabled();
	}

	bool IsHostile(EntityId entityID) const
	{
		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityID))
		{
			if (IAIObject* pAIObject = pEntity->GetAI())
			{
				return m_pAIObject && m_pAIObject->IsHostile(pAIObject);
			}
		}

		return false;
	}

	tukk GetName() const
	{
		assert(m_pAIActor);
		return m_pAIObject->GetName();
	}

	u8 GetFactionID() const
	{
		assert(m_pAIActor);
		return m_pAIObject->GetFactionID();
	}

	EAITargetThreat GetTargetThreat() const
	{
		assert(m_pAIActor);
		return m_pAIActor->GetState().eTargetThreat;
	}

	IAIObject* GetAttentionTarget() const
	{
		return m_pAIActor ? m_pAIActor->GetAttentionTarget() : NULL;
	}

	IAIObject* GetAttentionTargetAssociation()
	{
		IPipeUser* pipeUser = m_pAIObject ? m_pAIObject->CastToIPipeUser() : NULL;
		return pipeUser ? pipeUser->GetAttentionTargetAssociation() : NULL;
	}

	// Returns attention target and if possible it's association.
	// This is good when you want the agent to be able to "cheat"
	// by getting access to the present target status, even though
	// the target might be behind a wall or too far away to be seen.
	IAIObject* GetLiveTarget()
	{
		if (IsValid())
		{
			if (IPipeUser* pipeUser = m_pAIObject->CastToIPipeUser())
				if (IAIObject* association = pipeUser->GetAttentionTargetAssociation())
					return association;

			return m_pAIActor->GetAttentionTarget();
		}

		return NULL;
	}

	void SetSignal(i32 signalID, tukk text, IAISignalExtraData* data = NULL)
	{
		assert(m_pAIActor);
		m_pAIActor->SetSignal(signalID, text, NULL, data);
	}

	bool IsPointInFOV(const Vec3& pos) const
	{
		return m_pAIObject->IsPointInFOV(pos) != IAIObject::eFOV_Outside;
	}

	void GetPhysicalSkipEntities(PhysSkipList& skipList)
	{
		assert(m_pAIActor);
		return m_pAIActor->GetPhysicalSkipEntities(skipList);
	}

	operator bool () const
	{
		return IsValid();
	}

	const Agent& operator = (const Agent& rhs)
	{
		m_pAIObject = rhs.m_pAIObject;
		m_pAIActor = rhs.m_pAIActor;
		return *this;
	}

	IAnimationGraphState* GetAnimationGraphState()
	{
		IActor* actor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(GetEntityID());
		return actor ? actor->GetAnimationGraphState() : NULL;
	}

	bool SetAnimationGraphInput(EAIAGInput input, tukk animationName)
	{
		if (IAIActorProxy* aiActorProxy = m_pAIObject->GetProxy())
			return aiActorProxy->SetAGInput(input, animationName);
		return false;
	}

	bool IsSignalAnimationPlayed(tukk animationName)
	{
		if (IAIActorProxy* aiActorProxy = m_pAIObject->GetProxy())
			return aiActorProxy->IsSignalAnimationPlayed(animationName);
		return false;
	}

	void ResetAnimationGraphInput(EAIAGInput input)
	{
		if (IAIActorProxy* aiActorProxy = m_pAIObject->GetProxy())
			aiActorProxy->ResetAGInput(input);
	}

	ISkeletonPose* GetSkeletonPose(uint slotIndex = 0)
	{
		ICharacterInstance* characterInstance = GetEntity()->GetCharacter((i32)slotIndex);
		ISkeletonPose* skeletonPose = characterInstance ? characterInstance->GetISkeletonPose() : NULL;
		return skeletonPose;
	}

	IScriptTable* GetScriptTable()
	{
		return m_pAIObject->GetEntity()->GetScriptTable();
	}

	const IScriptTable* GetScriptTable() const
	{
		return m_pAIObject->GetEntity()->GetScriptTable();
	}

	const Matrix34& GetTransform() const
	{
		return GetEntity()->GetSlotWorldTM(0);
	}


	bool IsDead() const;
	bool IsHidden() const;
	const IActor* GetActor() const;
	IActor* GetActor()
	{
		return g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(GetEntityID());
	}

	i32 GetAlertness() const
	{
		if (IAIActorProxy* proxy = m_pAIActor->GetProxy())
		{
			return proxy->GetAlertnessState();
		}

		return 0;
	}

private:
	Agent() {}

	IAIObject* m_pAIObject;
	IAIActor* m_pAIActor;
};

inline float SquaredDistance(const Agent& agentA, const Agent& agentB)
{
	return agentA.GetPos().GetSquaredDistance(agentB.GetPos());
}

#include <drx3D/Game/Agent.inl"

#endif // Agent_h
