// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "../stdafx.h"
#include "../ConfigurationManager.h"
#include <drx3D/Sys/IConsole.h>

CConfigurationManager::CConfigurationManager()
{

}

void CConfigurationManager::Init()
{
	m_pPlatformsCvar = REGISTER_STRING("sys_target_platforms", "pc", VF_READONLY,
	                                   "Specifies the name of the target platforms for this project.\n"
	                                   "Usage: sys_target_platforms platform1,platform2,..\n"
	                                   "Where 'name' refers to the name of the platform\n"
	                                   "More than one platform can be specified by using a comma separated list");

	string platformList = m_pPlatformsCvar->GetString();

	i32 curPos = 0;
	string platformName = platformList.Tokenize(", ", curPos);
	while (!platformName.empty())
	{
		platformName.Trim();
		m_platformNames.push_back(platformName.c_str());
		platformName = platformList.Tokenize(", ", curPos);
	}

	// There has to be at least one target platform defined
	DRX_ASSERT(!m_platformNames.empty());
}

