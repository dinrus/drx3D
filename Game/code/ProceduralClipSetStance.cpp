// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <Mannequin/Serialization.h>
#include <drx3D/AI/IAgent.h>
#include <drx3D/Game/Player.h>

struct SSetStanceParams : public IProceduralParams
{
	virtual void Serialize(Serialization::IArchive& ar)
	{
		ar(stance, "Stance", "Stance");
	}

	TProcClipString stance;
};

class CProceduralClipSetStance : public TProceduralClip<SSetStanceParams>
{
protected:
	virtual void OnEnter( float blendTime, float duration, const SSetStanceParams& params )
	{
		const EntityId entityId = m_scope->GetEntityId();
		IEntity* entity = gEnv->pEntitySystem->GetEntity( entityId );
		IF_UNLIKELY( !entity )
			return;

		IAIObject* aiObject = entity->GetAI();
		IF_UNLIKELY( !aiObject )
			return;

		CPlayer* player = static_cast<CPlayer*>( g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor( entityId ) );
		IF_UNLIKELY( !player )
			return;

		CAIAnimationComponent* aiAnimationComponent = player->GetAIAnimationComponent();
		IF_UNLIKELY( !aiAnimationComponent )
			return;

		tukk stanceName = params.stance.c_str();
		IF_UNLIKELY( !stanceName )
			return;

		i32 stance = STANCE_NULL;
		for(; stance < STANCE_LAST; ++stance)
		{
			if ( strcmpi( stanceName, GetStanceName( (EStance)stance) ) == 0 )
				break;
		}

		IF_UNLIKELY( (EStance)stance == STANCE_LAST )
			return;

		aiAnimationComponent->ForceStanceTo( *player, (EStance)stance );
		aiAnimationComponent->ForceStanceInAIActorTo( *player, (EStance)stance );
	}

	virtual void Update( float timePassed )
	{
	}

	virtual void OnExit( float blendTime )
	{
	}
};

REGISTER_PROCEDURAL_CLIP(CProceduralClipSetStance, "SetStance");
