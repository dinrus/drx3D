// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////
#ifndef __ANIM_ACTION_AI_LOOKING_H__
#define __ANIM_ACTION_AI_LOOKING_H__

#include <drx3D/Act/IDrxMannequin.h>


class CAnimActionAILooking
	: public TAction< SAnimationContext >
{
public:
	typedef TAction< SAnimationContext > TBase;

	DEFINE_ACTION( "AnimActionAILooking" );

	CAnimActionAILooking();

	// IAction
	virtual void OnInitialise() override;
	virtual EStatus Update( float timePassed ) override;
	// ~IAction

	static bool IsSupported( const SAnimationContext& context );

private:
	static FragmentID FindFragmentId( const SAnimationContext& context );
};


#endif