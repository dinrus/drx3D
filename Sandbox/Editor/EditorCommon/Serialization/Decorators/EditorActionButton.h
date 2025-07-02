// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Decorators/ActionButton.h>
#include <functional>

namespace Serialization
{

typedef std::function<void ()> StdFunctionActionButtonCalback;

struct StdFunctionActionButton : public IActionButton
{
	StdFunctionActionButtonCalback callback;
	string                         icon;

	explicit StdFunctionActionButton(const StdFunctionActionButtonCalback& callback, tukk icon = "")
		: callback(callback)
		, icon(icon)
	{
	}

	// IActionButton

	virtual void Callback() const override
	{
		if (callback)
		{
			callback();
		}
	}

	virtual tukk Icon() const override
	{
		return icon.c_str();
	}

	virtual IActionButtonPtr Clone() const override
	{
		return IActionButtonPtr(new StdFunctionActionButton(callback, icon.c_str()));
	}

	// ~IActionButton
};

inline bool Serialize(Serialization::IArchive& ar, StdFunctionActionButton& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(static_cast<Serialization::IActionButton&>(button)), name, label);
	else
		return false;
}

inline StdFunctionActionButton ActionButton(const StdFunctionActionButtonCalback& callback, tukk icon = "")
{
	return StdFunctionActionButton(callback, icon);
}

}

