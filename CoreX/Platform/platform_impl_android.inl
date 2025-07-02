// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

void DrxFindEngineRootFolder(unsigned int engineRootPathSize, char* szEngineRootPath)
{
	// Hack! Android currently does not support a directory layout, there is an explicit search in main for GameSDK/GameData.pak
	// and the executable folder is not related to the engine or game folder. - 18/03/2016
	drx_strcpy(szEngineRootPath, engineRootPathSize, DrxGetProjectStoragePath());
}