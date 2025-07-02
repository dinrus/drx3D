// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __ANIM_ACTION_AI_LOOK_POSE_H__
#define __ANIM_ACTION_AI_LOOK_POSE_H__

#include <drx3D/Act/IDrxMannequin.h>


class CAnimActionAILookPose
	: public TAction< SAnimationContext >
{
public:
	typedef TAction< SAnimationContext > TBase;

	DEFINE_ACTION( "AnimActionAILookPose" );

	CAnimActionAILookPose();

	// IAction
	virtual void OnInitialise() override;
	virtual EStatus Update( float timePassed ) override;
	virtual void Install() override;
	// ~IAction

	static bool IsSupported( const SAnimationContext& context );

private:
	void InitialiseLookPoseBlender();
};


#endif