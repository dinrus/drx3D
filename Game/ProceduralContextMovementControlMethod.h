// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD__H__
#define __PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD__H__

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/RegFactoryNode.h>

#include <IAnimatedCharacter.h>

#include <drx3D/Game/ProceduralContextHelpers.h>


#define PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD_NAME "ProceduralContextMCM"


class CProceduralContextMovementControlMethod
	: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextMovementControlMethod, PROCEDURAL_CONTEXT_MOVEMENT_CONTROL_METHOD_NAME, 0x80140507bdc64be4, 0xa24190f322270e82 );

	// IProceduralContext
	virtual void Update( float timePassedSeconds ) override;
	// ~IProceduralContext

	u32 RequestMovementControlMethod( const EMovementControlMethod horizontal, const EMovementControlMethod vertical );
	void CancelRequest( u32k requestId );

private:
	IAnimatedCharacter* GetAnimatedCharacter() const;

private:
	struct SMCMRequest
	{
		u32 id;
		EMovementControlMethod horizontal;
		EMovementControlMethod vertical;
	};

	typedef ProceduralContextHelpers::CRequestList< SMCMRequest > TMCMRequestList;
	TMCMRequestList m_requestList;
};


#endif
