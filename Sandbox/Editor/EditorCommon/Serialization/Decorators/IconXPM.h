// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace Serialization {

// Icon, stored in XPM format
struct IconXPM
{
	tukk const* source;
	i32                lineCount;

	IconXPM()
		: source(0)
		, lineCount(0)
	{
	}
	template<size_t Size>
	explicit IconXPM(tukk (&xpm)[Size])
	{
		source = xpm;
		lineCount = Size;
	}

	void Serialize(Serialization::IArchive& ar) {}
	bool operator<(const IconXPM& rhs) const    { return source < rhs.source; }

};

struct IconXPMToggle
{
	bool*   variable_;
	bool    value_;
	IconXPM iconTrue_;
	IconXPM iconFalse_;

	template<size_t Size1, size_t Size2>
	IconXPMToggle(bool& variable, tuk (&xpmTrue)[Size1], tuk (&xpmFalse)[Size2])
		: iconTrue_(xpmTrue)
		, iconFalse_(xpmFalse)
		, variable_(&variable)
		, value_(variable)
	{
	}

	IconXPMToggle(bool& variable, const IconXPM& iconTrue, const IconXPM& iconFalse)
		: iconTrue_(iconTrue)
		, iconFalse_(iconFalse)
		, variable_(&variable)
		, value_(variable)
	{
	}

	IconXPMToggle(const IconXPMToggle& orig)
		: variable_(0)
		, value_(orig.value_)
		, iconTrue_(orig.iconTrue_)
		, iconFalse_(orig.iconFalse_)
	{
	}

	IconXPMToggle()
		: variable_(0)
	{
	}
	IconXPMToggle& operator=(const IconXPMToggle& rhs)
	{
		value_ = rhs.value_;
		return *this;
	}
	~IconXPMToggle()
	{
		if (variable_)
			*variable_ = value_;
	}

	template<class TArchive>
	void Serialize(TArchive& ar)
	{
		ar(value_, "value", "Value");
	}
};

}

