// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <StdAfx.h>
#include "MannPreferences.h"

#include <drx3D/CoreX/Serialization/yasli/decorators/Range.h>

SMannequinGeneralPreferences gMannequinPreferences;
REGISTER_PREFERENCES_PAGE_PTR(SMannequinGeneralPreferences, &gMannequinPreferences)

SMannequinGeneralPreferences::SMannequinGeneralPreferences()
	: SPreferencePage("General", "Mannequin/General")
	, defaultPreviewFile("Animations/Mannequin/Preview/SDK_playerPreview1P.xml")
	, trackSize(32)
	, timelineWheelZoomSpeed(1.f)
	, bCtrlForScrubSnapping(false)
{
}

bool SMannequinGeneralPreferences::Serialize(yasli::Archive& ar)
{
	i32k kMannequinTrackSizeMin = 14;
	i32k kMannequinTrackSizeMax = 32;

	ar.openBlock("general", "General");
	ar(defaultPreviewFile, "defaultPreviewFile", "Default Preview File"); //file
	ar(yasli::Range(trackSize, kMannequinTrackSizeMin, kMannequinTrackSizeMax), "trackSize", "Size of tracks");
	ar(bCtrlForScrubSnapping, "bCtrlForScrubSnapping", "Hold Ctrl to Snap Scrubbing");
	ar(yasli::Range(timelineWheelZoomSpeed, 0.1f, 5.f), "timelineWheelZoomSpeed", "Timeline Wheel Zoom Speed");
	ar.closeBlock();

	return true;
}

