// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace yasli {

class Archive;

// Icon, stored in XPM format
struct IconXPM
{
	tukk const* source;
	i32 lineCount;

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
  template<size_t Size>
	explicit IconXPM(tuk (&xpm)[Size])
	{
		source = xpm;
		lineCount = Size;
	}

	IconXPM(tukk const* source, i32 lineCount)
	: source(source)
	, lineCount(lineCount)
	{
	}

	void YASLI_SERIALIZE_METHOD(Archive& ar) 	{}
	bool operator<(const IconXPM& rhs) const { return source < rhs.source; }

};

struct IconXPMToggle
{
	bool* variable_;
	bool value_;
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

	template<size_t Size1, size_t Size2>
	IconXPMToggle(bool& variable, tukk (&xpmTrue)[Size1], tukk (&xpmFalse)[Size2])
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
	IconXPMToggle& operator=(const IconXPMToggle& rhs){
		value_ = rhs.value_;
		return *this;
	}
	~IconXPMToggle()
	{
		if (variable_)
			*variable_ = value_;
	}

	template<class TArchive>
	void YASLI_SERIALIZE_METHOD(TArchive& ar)
	{
		ar(value_, "value", "Value");
	}
};

}
