// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SCRIPTBIND_LIGHTNING_ARC_H_
#define _SCRIPTBIND_LIGHTNING_ARC_H_

#pragma once

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>


class CLightningArc;



class CScriptBind_LightningArc: public CScriptableBase
{
public:
	CScriptBind_LightningArc(ISystem* pSystem);

	void AttachTo(CLightningArc* pLightingArc);

	i32 TriggerSpark(IFunctionHandler* pFunction);
	i32 Enable(IFunctionHandler* pFunction, bool enable);
	i32 ReadLuaParameters(IFunctionHandler* pFunction);
};



#endif
