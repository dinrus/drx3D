// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Exposes basic Actor System to the Script System.

   -------------------------------------------------------------------------
   История:
   - 21:9:2004   3:00 : Created by Mathieu Pinard

*************************************************************************/
#ifndef __SCRIPTBIND_ACTORSYSTEM_H__
#define __SCRIPTBIND_ACTORSYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

struct IActorSystem;
struct IGameFramework;

class CScriptBind_ActorSystem :
	public CScriptableBase
{
public:
	CScriptBind_ActorSystem(ISystem* pSystem, IGameFramework* pGameFW);
	virtual ~CScriptBind_ActorSystem();

	void         Release() { delete this; };

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! <code>ActorSystem.CreateActor( channelId, actorParams )</code>
	//!		<param name="channelId">Identifier for the network channel.</param>
	//!		<param name="actorParams">Parameters for the actor.</param>
	//! <description>Creates an actor.</description>
	i32 CreateActor(IFunctionHandler* pH, i32 channelId, SmartScriptTable actorParams);

private:
	void RegisterGlobals();
	void RegisterMethods();

	ISystem*        m_pSystem;
	IGameFramework* m_pGameFW;
};

#endif //__SCRIPTBIND_ACTORSYSTEM_H__
