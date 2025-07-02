// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "dll_string.h"

struct ICVar;

class EDITOR_COMMON_API CConfigurationManager
{
public:
	CConfigurationManager();
	void                           Init();
	const std::vector<dll_string>& GetPlatformNames() const { return m_platformNames; }

private:
	std::vector<dll_string> m_platformNames;

	ICVar*                  m_pPlatformsCvar;
};

