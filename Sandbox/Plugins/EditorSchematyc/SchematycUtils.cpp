// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SchematycUtils.h"

#include <QtUtil.h>

namespace DrxSchematycEditor {

void MakeScriptElementNameUnique(stack_string& name, Schematyc::IScriptElement* pScope)
{
	Schematyc::IScriptRegistry& scriptRegistry = gEnv->pSchematyc->GetScriptRegistry();
	if (pScope == nullptr)
	{
		pScope = &scriptRegistry.GetRootElement();
	}

	if (!scriptRegistry.IsElementNameUnique(name.c_str(), pScope))
	{
		stack_string::size_type counterPos = name.find(".");
		if (counterPos == stack_string::npos)
		{
			counterPos = name.length();
		}

		char stringBuffer[16] = "";
		stack_string::size_type counterLength = 0;
		u32 counter = 1;

		do
		{
			ltoa(counter, stringBuffer, 10);
			name.replace(counterPos, counterLength, stringBuffer);
			counterLength = strlen(stringBuffer);
			++counter;
		}
		while (!scriptRegistry.IsElementNameUnique(name.c_str(), pScope));
	}
}

}

