// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/AnimActionAILookPose.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAnimationPoseModifier.h>

#include <drx3D/Game/PlayerAnimation.h>


//////////////////////////////////////////////////////////////////////////
#define MAN_AILOOKPOSE_FRAGMENTS( x ) \
	x( LookPose )

#define MAN_AILOOKPOSE_TAGS( x )

#define MAN_AILOOKPOSE_TAGGROUPS( x )

#define MAN_AILOOKPOSE_SCOPES( x )

#define MAN_AILOOKPOSE_CONTEXTS( x )

#define MAN_AILOOKPOSE_CHANGEFRAGMENT_FRAGMENT_TAGS( x )

#define MAN_AILOOKPOSE_FRAGMENT_TAGS( x )

MANNEQUIN_USER_PARAMS( SMannequinAiLookPoseUserParams, MAN_AILOOKPOSE_FRAGMENTS, MAN_AILOOKPOSE_TAGS, MAN_AILOOKPOSE_TAGGROUPS, MAN_AILOOKPOSE_SCOPES, MAN_AILOOKPOSE_CONTEXTS, MAN_AILOOKPOSE_FRAGMENT_TAGS );


//////////////////////////////////////////////////////////////////////////
namespace
{
	FragmentID FindFragmentId( const SAnimationContext& context )
	{
		const SMannequinAiLookPoseUserParams* pUserParams = GetMannequinUserParams< SMannequinAiLookPoseUserParams >( context );
		DRX_ASSERT( pUserParams != NULL );

		return pUserParams->fragmentIDs.LookPose;
	}
}



//////////////////////////////////////////////////////////////////////////
CAnimActionAILookPose::CAnimActionAILookPose()
	: TBase( PP_Lowest, FRAGMENT_ID_INVALID, TAG_STATE_EMPTY, IAction::NoAutoBlendOut | IAction::Interruptable )
{
}


//////////////////////////////////////////////////////////////////////////
void CAnimActionAILookPose::OnInitialise()
{
	const FragmentID fragmentId = FindFragmentId( *m_context );
	DRX_ASSERT( fragmentId != FRAGMENT_ID_INVALID );
	SetFragment( fragmentId );
}


//////////////////////////////////////////////////////////////////////////
void CAnimActionAILookPose::Install()
{
	TBase::Install();

	InitialiseLookPoseBlender();
}


//////////////////////////////////////////////////////////////////////////
void CAnimActionAILookPose::InitialiseLookPoseBlender()
{
	IScope& rootScope = GetRootScope();
	ICharacterInstance* pCharacterInstance = rootScope.GetCharInst();
	DRX_ASSERT( pCharacterInstance );
	if ( ! pCharacterInstance )
	{
		return;
	}

	ISkeletonPose* pSkeletonPose = pCharacterInstance->GetISkeletonPose();
	DRX_ASSERT( pSkeletonPose );

	IAnimationPoseBlenderDir* pPoseBlenderLook = pSkeletonPose->GetIPoseBlenderLook();
	DRX_ASSERT( pPoseBlenderLook );
	if ( ! pPoseBlenderLook )
	{
		return;
	}

	u32k lookPoseAnimationLayer = rootScope.GetBaseLayer();
	pPoseBlenderLook->SetLayer( lookPoseAnimationLayer );
}


//////////////////////////////////////////////////////////////////////////
IAction::EStatus CAnimActionAILookPose::Update( float timePassed )
{
	TBase::Update( timePassed );

	const IScope& rootScope = GetRootScope();
	const bool foundNewBestMatchingFragment = rootScope.IsDifferent( m_fragmentID, m_fragTags );
	if ( foundNewBestMatchingFragment )
	{
		SetFragment( m_fragmentID, m_fragTags );
	}

	return m_eStatus;
}


//////////////////////////////////////////////////////////////////////////
bool CAnimActionAILookPose::IsSupported( const SAnimationContext& context )
{
	const FragmentID fragmentId = FindFragmentId( context );
	const bool isSupported = ( fragmentId != FRAGMENT_ID_INVALID );
	return isSupported;
}