// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: ProceduralClip for Ragdolling from DrxMannequin

-------------------------------------------------------------------------
История:
- 11.18.11: Created by Stephen M. North

*************************************************************************/
#include <drx3D/Game/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/Game/Player.h>

#include <drx3D/Game/ProceduralContextRagdoll.h>
#include <drx3D/CoreX/Serialization/IArchive.h>

struct SRagdollParams : public IProceduralParams
{
	SRagdollParams()
		: sleep(0.0f)
		, stiffness(500.0f)
	{
	}

	virtual void Serialize(Serialization::IArchive& ar)
	{
		ar(sleep, "Sleep", "Sleep");
		ar(stiffness, "Stiffness", "Stiffness");
	}
	float sleep;
	float stiffness;
};

class CProceduralClipRagdoll : public TProceduralContextualClip<CProceduralContextRagdoll, SRagdollParams>
{
public:
	CProceduralClipRagdoll()
		: m_stiffness(0.0f)
		, m_randomTriggerTime(0.0f)
		, m_totalTimePassed(0.0f)
		, m_ragdollSleep(false)		
	{
	}

protected:
	virtual void OnEnter(float blendTime, float duration, const SRagdollParams &params)
	{
		m_randomTriggerTime = drx_random(0.0f, blendTime);
		m_ragdollSleep = bool(params.sleep>0.0f);
		m_stiffness = params.stiffness;
	}
	virtual void Update(float timePassed)
	{
		m_totalTimePassed += timePassed;
		if( !m_context->IsInRagdoll() && m_totalTimePassed > m_randomTriggerTime ) 
		{
			m_context->EnableRagdoll( m_entity->GetId(), m_ragdollSleep, m_stiffness, true );
		}
	}
	virtual void OnExit(float blendTime)
	{
		m_totalTimePassed = 0.0f;
		m_context->DisableRagdoll( blendTime );
	}

private:
	float m_stiffness;
	float m_randomTriggerTime;
	float m_totalTimePassed;
	bool  m_ragdollSleep;
};

REGISTER_PROCEDURAL_CLIP(CProceduralClipRagdoll, "Ragdoll");
