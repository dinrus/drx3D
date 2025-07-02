// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROCEDURAL_CONTEXT_COLLIDER_MODE__H__
#define __PROCEDURAL_CONTEXT_COLLIDER_MODE__H__

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/RegFactoryNode.h>

#include <IAnimatedCharacter.h>

#include <drx3D/Game/ProceduralContextHelpers.h>


#define PROCEDURAL_CONTEXT_COLLIDER_MODE_NAME "ProceduralContextColliderMode"


class CProceduralContextColliderMode
	: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextColliderMode, PROCEDURAL_CONTEXT_COLLIDER_MODE_NAME, 0x2857e483964b45e4, 0x8e9e6a481db8c166 );

	// IProceduralContext
	virtual void Update( float timePassedSeconds ) override;
	// ~IProceduralContext

	u32 RequestColliderMode( const EColliderMode colliderMode );
	void CancelRequest( u32k requestId );

private:
	IAnimatedCharacter* GetAnimatedCharacter() const;

private:
	struct SColliderModeRequest
	{
		u32 id;
		EColliderMode mode;
	};

	typedef ProceduralContextHelpers::CRequestList< SColliderModeRequest > TColliderModeRequestList;
	TColliderModeRequestList m_requestList;
};


#endif
