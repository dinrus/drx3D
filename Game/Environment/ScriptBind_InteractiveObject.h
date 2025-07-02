// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script bind functions forCrysis2 interactive object

-------------------------------------------------------------------------
История:
- 14:12:2009: Created by Benito G.R.

*************************************************************************/

#pragma once

#ifndef _SCRIPTBIND_INTERACTIVE_OBJECT_H_
#define _SCRIPTBIND_INTERACTIVE_OBJECT_H_

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

#include <drx3D/Game/InteractiveObjectRegistry.h>


class CInteractiveObjectEx;

class CScriptBind_InteractiveObject :
	public CScriptableBase
{

private:

	typedef std::map<EntityId, CInteractiveObjectEx*> TInteractiveObjectsMap;

public:
	CScriptBind_InteractiveObject(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_InteractiveObject();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const;
	

	void AttachTo(CInteractiveObjectEx *pInteractiveObject);
	void Detach(EntityId entityId);

	i32 CanUse(IFunctionHandler *pH, ScriptHandle userId);
	i32 Use(IFunctionHandler *pH, ScriptHandle userId);
	i32 StopUse(IFunctionHandler *pH, ScriptHandle userId);
	i32 AbortUse(IFunctionHandler *pH);

	ILINE CInteractiveObjectRegistry& GetObjectDataRegistry() { return m_objectDataRegistry; }

private:
	void RegisterMethods();

	CInteractiveObjectEx *GetInteractiveObject(IFunctionHandler *pH);

	ISystem					*m_pSystem;
	IGameFramework	*m_pGameFrameWork;

	TInteractiveObjectsMap m_interactiveObjectsMap;
	CInteractiveObjectRegistry m_objectDataRegistry;
};

#endif