// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __ANIM_ACTION_AI_AIM_POSE_H__
#define __ANIM_ACTION_AI_AIM_POSE_H__

#include <drx3D/Act/IDrxMannequin.h>


class CAnimActionAIAimPose
: public TAction< SAnimationContext >
{
public:
	typedef TAction< SAnimationContext > TBase;

	DEFINE_ACTION( "AnimActionAIAimPose" );

	CAnimActionAIAimPose();

	// IAction
	virtual void OnInitialise() override;
	virtual EStatus Update( float timePassed ) override;
	virtual void Install() override;
	// ~IAction

	static bool IsSupported( const SAnimationContext& context );

private:
	void InitialiseAimPoseBlender();
	static FragmentID FindFragmentId( const SAnimationContext& context );
};


#endif