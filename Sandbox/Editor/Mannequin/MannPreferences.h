// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <EditorFramework/Preferences.h>

struct SMannequinGeneralPreferences : public SPreferencePage
{
	SMannequinGeneralPreferences();
	virtual bool Serialize(yasli::Archive& ar) override;

	string defaultPreviewFile;
	i32    trackSize;
	float  timelineWheelZoomSpeed;
	bool   bCtrlForScrubSnapping;
};

extern SMannequinGeneralPreferences gMannequinPreferences;

