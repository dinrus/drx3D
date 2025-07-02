// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct IBlackBoard
{
	// <interfuscator:shuffle>
	virtual SmartScriptTable& GetForScript() = 0;
	virtual void              SetFromScript(SmartScriptTable&) = 0;
	virtual void              Clear() = 0;
	virtual ~IBlackBoard(){}
	// </interfuscator:shuffle>
};
