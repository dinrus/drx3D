// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAnimationPoseModifier.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/Game/ProceduralContextLook.h>

class CProceduralClipLooking
	: public TProceduralContextualClip< CProceduralContextLook, SNoProceduralParams >
{
public:
		virtual void OnEnter( float blendTime, float duration, const SNoProceduralParams& params )
	{
		m_context->SetBlendInTime( blendTime );
		m_context->UpdateProcClipLookingRequest( true );
	}

	virtual void OnExit( float blendTime )
	{
		m_context->SetBlendOutTime( blendTime );
		m_context->UpdateProcClipLookingRequest( false );
	}

	virtual void Update( float timePassed )
	{
		m_context->UpdateProcClipLookingRequest( true );
	}
};

REGISTER_PROCEDURAL_CLIP(CProceduralClipLooking, "Looking");
