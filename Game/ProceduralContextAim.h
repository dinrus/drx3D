// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROCEDURAL_CONTEXT_AIM__H__
#define __PROCEDURAL_CONTEXT_AIM__H__

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAnimationPoseModifier.h>

#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/RegFactoryNode.h>

#include <drx3D/Game/ProceduralContextHelpers.h>

#define PROCEDURAL_CONTEXT_AIM_NAME "ProceduralContextAim"

class CProceduralContextAim
: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextAim, PROCEDURAL_CONTEXT_AIM_NAME, 0x4a5625bb01d149c6, 0xb5632cf301b58e38 );

	// IProceduralContext
	virtual void Initialise( IEntity& entity, IActionController& actionController ) override;
	virtual void Update( float timePassedSeconds ) override;
	// ~IProceduralContext

	void UpdateGameAimingRequest( const bool aimRequest );
	void UpdateProcClipAimingRequest( const bool aimRequest );

	void UpdateGameAimTarget( const Vec3& aimTarget );

	void SetBlendInTime( const float blendInTime );
	void SetBlendOutTime( const float blendOutTime );

	u32 RequestPolarCoordinatesSmoothingParameters( const Vec2& maxSmoothRateRadiansPerSecond, const float smoothTimeSeconds );
	void CancelPolarCoordinatesSmoothingParameters( u32k requestId );

private:
	void InitialisePoseBlenderAim();
	void InitialiseGameAimTarget();

	void UpdatePolarCoordinatesSmoothingParameters();

private:
	IAnimationPoseBlenderDir* m_pPoseBlenderAim;

	bool m_gameRequestsAiming;
	bool m_procClipRequestsAiming;
	Vec3 m_gameAimTarget;

	Vec2 m_defaultPolarCoordinatesMaxSmoothRateRadiansPerSecond;
	float m_defaultPolarCoordinatesSmoothTimeSeconds;
	struct SPolarCoordinatesSmoothingParametersRequest
	{
		u32 id;
		Vec2 maxSmoothRateRadiansPerSecond;
		float smoothTimeSeconds;
	};

	typedef ProceduralContextHelpers::CRequestList< SPolarCoordinatesSmoothingParametersRequest > TPolarCoordinatesSmoothingParametersRequestList;
	TPolarCoordinatesSmoothingParametersRequestList m_polarCoordinatesSmoothingParametersRequestList;
};


#endif
