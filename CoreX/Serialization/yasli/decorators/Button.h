// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Archive.h>

namespace yasli{

struct Button
{
	Button(tukk _text = 0)
	: pressed(false)
	, text(_text) {}

	operator bool() const{
		return pressed;
	}
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar) {}

	bool pressed;
	tukk text;
};

inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, Button& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serializer(button), name, label);
	else
	{
		button.pressed = false;
		return false;
	}
}

}
