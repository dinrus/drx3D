// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "LevelIndependentFileMan.h"

CLevelIndependentFileMan::CLevelIndependentFileMan()
{

}

CLevelIndependentFileMan::~CLevelIndependentFileMan()
{
	assert(m_Modules.size() == 0);
}

bool CLevelIndependentFileMan::PromptChangedFiles()
{
	for (std::vector<ILevelIndependentFileModule*>::iterator it = m_Modules.begin(); it != m_Modules.end(); ++it)
	{
		if (!(*it)->PromptChanges())
			return false;
	}
	return true;
}

void CLevelIndependentFileMan::RegisterModule(ILevelIndependentFileModule* pModule)
{
	stl::push_back_unique(m_Modules, pModule);
}
void CLevelIndependentFileMan::UnregisterModule(ILevelIndependentFileModule* pModule)
{
	stl::find_and_erase(m_Modules, pModule);
}

