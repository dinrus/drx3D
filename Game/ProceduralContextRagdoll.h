// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: ProceduralContext for the ProceduralClipRagdoll from DrxMannequin

-------------------------------------------------------------------------
История:
- 27.09.2012: Created by Stephen M. North

*************************************************************************/
#pragma once

#ifndef __ProceduralContextRagdoll_h__
#define __ProceduralContextRagdoll_h__

#include <drx3D/Act/IDrxMannequin.h>

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/IAnimationPoseModifier.h>

#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/RegFactoryNode.h>


#define PROCEDURAL_CONTEXT_RAGDOLL_NAME "ProceduralContextRagdoll"

class CProceduralContextRagdoll
	: public IProceduralContext
{
public:
	PROCEDURAL_CONTEXT( CProceduralContextRagdoll, PROCEDURAL_CONTEXT_RAGDOLL_NAME, 0x37856d62bd5f42f0, 0xad8a4314a0de6dd2 );

	ILINE EntityId GetEntityTarget() const { return m_targetEntityId; }
	ILINE void SetEntityTarget( const EntityId entityID ) { m_targetEntityId = entityID; }
	ILINE void SetAspectProfileScope( bool bScope ) { m_bDispatchedAspectProfile = bScope; }
	ILINE bool GetAspectProfileScope() const { return m_bDispatchedAspectProfile; }
	ILINE bool IsInRagdoll() const{ return m_bInRagdoll; }
	ILINE void SetBlendOut( float blendOutTime )
	{
		m_blendOutTime = blendOutTime;
		m_bInBlendOut = true;
	}
	void EnableRagdoll( const EntityId entityID, const bool bAlive, const float stiffness, const bool bFromProcClip = false );
	void DisableRagdoll( float blendOutTime );
	void QueueRagdoll( bool bAlive );
	void ForceRagdollFinish(IActor* piActor, bool bForceDead);

protected:
	// IProceduralContext
	virtual void Update( float timePassedSeconds ) override;
	// ~IProceduralContext

	void Reset();

private:
	EntityId m_targetEntityId;
	float m_stiffness;
	float m_blendOutTime;
	float m_blendOutTimeCurrent;
	bool m_bInRagdoll;
	bool m_bInBlendOut;
	bool m_bEntityAlive;
	bool m_bDispatchedAspectProfile;
	bool m_bForceRagdollFinish;
	bool m_bFromProcClip;
};

#endif //__ProceduralContextRagdoll_h__