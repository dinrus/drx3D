// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROCEDURAL_CONTEXT_LOOK__H__
#define __PROCEDURAL_CONTEXT_LOOK__H__

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAnimationPoseModifier.h>

#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/RegFactoryNode.h>


#define PROCEDURAL_CONTEXT_LOOK_NAME "ProceduralContextLook"

class CProceduralContextLook
	: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextLook, PROCEDURAL_CONTEXT_LOOK_NAME, 0x0928592BD91648A5, 0x9024C8221945BB17 );

	// IProceduralContext
	virtual void Initialise( IEntity& entity, IActionController& actionController ) override;
	virtual void Update( float timePassedSeconds ) override;
	// ~IProceduralContext

	void UpdateGameLookingRequest( const bool lookRequest );
	void UpdateProcClipLookingRequest( const bool lookRequest );

	void UpdateGameLookTarget( const Vec3& lookTarget );

	void SetBlendInTime( const float blendInTime );
	void SetBlendOutTime( const float blendOutTime );
	void SetFovRadians( const float fovRadians );

private:
	void InitialisePoseBlenderLook();
	void InitialiseGameLookTarget();

private:
	IAnimationPoseBlenderDir* m_pPoseBlenderLook;

	bool m_gameRequestsLooking;
	bool m_procClipRequestsLooking;
	Vec3 m_gameLookTarget;
};


#endif
