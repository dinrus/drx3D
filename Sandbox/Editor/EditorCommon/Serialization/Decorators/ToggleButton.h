// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace Serialization
{

struct ToggleButton
{
	bool*       value;
	tukk iconOn;
	tukk iconOff;

	ToggleButton(bool& value, tukk iconOn = "", tukk iconOff = "")
		: value(&value)
		, iconOn(iconOn)
		, iconOff(iconOff)
	{
	}
};

struct RadioButton
{
	i32* value;
	i32  buttonValue;

	RadioButton(i32& value, i32 buttonValue)
		: value(&value)
		, buttonValue(buttonValue)
	{
	}
};

bool Serialize(Serialization::IArchive& ar, Serialization::ToggleButton& button, tukk name, tukk label);
bool Serialize(Serialization::IArchive& ar, Serialization::RadioButton& button, tukk name, tukk label);

}

#include "ToggleButtonImpl.h"

