// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <EditorFramework/Preferences.h>

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

struct EDITOR_COMMON_API SLightingPreferences : public SPreferencePage
{
	SLightingPreferences();
	virtual bool Serialize(yasli::Archive& ar) override;

	bool bForceSkyUpdate;
	bool bTotalIlluminationEnabled;
};

EDITOR_COMMON_API extern SLightingPreferences gLightingPreferences;

