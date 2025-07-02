// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"

#include "LightingPreferences.h"

EDITOR_COMMON_API SLightingPreferences gLightingPreferences;
REGISTER_PREFERENCES_PAGE_PTR(SLightingPreferences, &gLightingPreferences)

SLightingPreferences::SLightingPreferences()
	: SPreferencePage("Lighting", "Lighting/General")
	, bForceSkyUpdate(true)
	, bTotalIlluminationEnabled(false)
{
}

bool SLightingPreferences::Serialize(yasli::Archive& ar)
{
	ar.openBlock("general", "General");
	ar(bForceSkyUpdate, "forceSkyUpdate", "Force Sky Update");
	ar.closeBlock();
	ar.openBlock("experimental", "Experimental");
	ar(bTotalIlluminationEnabled, "bTotalIlluminationEnabled", "Total Illumination");
	ar.closeBlock();

	return true;
}

