// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: Util functions relating to script entity operations.

-------------------------------------------------------------------------
История:
- 22:02:2011: Created by Benito G.R.

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/EntityScriptCalls.h>

bool EntityScripts::CallScriptFunction(IEntity* pEntity, IScriptTable *pScriptTable, tukk functionName)
{
	bool result = false;

	if ((pEntity != NULL) && (pScriptTable != NULL))
	{
		IScriptSystem *pScriptSystem = pScriptTable->GetScriptSystem();
		if (pScriptTable->GetValueType(functionName) == svtFunction)
		{
			pScriptSystem->BeginCall(pScriptTable, functionName); 
			pScriptSystem->PushFuncParam(pEntity->GetScriptTable());
			pScriptSystem->EndCall(result);
		}
	}

	return result;
}