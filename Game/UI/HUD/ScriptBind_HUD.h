// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Описание: Script Binding for HUD
  
 -------------------------------------------------------------------------
  История:
  - 14:02:2006   11:30 : Created by AlexL
	- 04:04:2006	 17:30 : Extended by Jan M�ller

*************************************************************************/
#ifndef __SCRIPTBIND_HUD_H__
#define __SCRIPTBIND_HUD_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>

struct IGameFramework;

class CScriptBind_HUD :
	public CScriptableBase
{
public:
	CScriptBind_HUD(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_HUD();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

protected:
	i32 SetObjectiveStatus(IFunctionHandler *pH,tukk pObjectiveID, i32 status, bool silent);
	i32 SetObjectiveEntity(IFunctionHandler *pH,tukk pObjectiveID,tukk entityName);
	i32 ClearObjectiveEntity(IFunctionHandler *pH,tukk pObjectiveID);
	i32 AddEntityToRadar(IFunctionHandler *pH, ScriptHandle entityId);
	
	i32 RemoveEntityFromRadar(IFunctionHandler *pH, ScriptHandle entityId);
	i32 AddEntitySilhouette(IFunctionHandler *pH, ScriptHandle entityId, float r, float g, float b, float a);
	
	i32 OnGameStatusUpdate(IFunctionHandler *pH, i32 goodBad, tukk msg);
	i32 RemoveObjective(IFunctionHandler *pH, ScriptHandle entityId);


private:
	void RegisterGlobals();
	void RegisterMethods();

	ISystem						*m_pSystem;
	IGameFramework		*m_pGameFW;
};

#endif //__SCRIPTBIND_HUD_H__
