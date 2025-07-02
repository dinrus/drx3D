/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "PropertyTree.h"
#include <drx3D/CoreX/Serialization/yasli/MemoryWriter.h>
#include <drx3D/CoreX/Serialization/yasli/decorators/Range.h>
#include "PropertyRowNumberField.h"

#include <limits>
#include <float.h>
#include <math.h>

template<class T>
yasli::string numberAsString(T value)
{
	yasli::MemoryWriter buf;
	buf << value;
	return buf.c_str();
}

inline long long stringToSignedInteger(tukk str)
{
	long long value;
#ifdef _MSC_VER
	value = _atoi64(str);
#else
    tuk endptr = (tuk)str;
    value = strtoll(str, &endptr, 10);
#endif
	return value;
}

inline zu64 stringToUnsignedInteger(tukk str)
{
	zu64 value;
	if (*str == '-') {
		value = 0;
	}
	else {
#ifdef _MSC_VER
		tuk endptr = (tuk)str;
		value = _strtoui64(str, &endptr, 10);
#else
        tuk endptr = (tuk)str;
		value = strtoull(str, &endptr, 10);
#endif
	}
	return value;
}

template<class Output, class Input>
Output clamp(Input value, Output min, Output max)
{
	if (value < Input(min))
		return min;
	if (value > Input(max))
		return max;
	return Output(value);
}

// workaround for VS2005 missing numeric_limits<>::lowest()
inline float limit_min(float) { return -FLT_MAX; }
inline float limit_max(float) { return FLT_MAX; }
inline double limit_min(double) { return -DBL_MAX; }
inline double limit_max(double) { return DBL_MAX; }
template<class T> T limit_min(T) { return std::numeric_limits<T>::min(); }
template<class T> T limit_max(T) { return std::numeric_limits<T>::max(); }

template<class Out, class In> void clampToType(Out* out, In value) { *out = clamp(value, limit_min((Out)value), limit_max((Out)value)); }

bool isDigit(i32 ch);

double parseFloat(tukk s);

inline void clampedNumberFromString(tuk value, tukk str)        { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(signed tuk value, tukk str) { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(short* value, tukk str)		 { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(i32* value, tukk str)         { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(long* value, tukk str)		 { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(long long* value, tukk str)   { clampToType(value, stringToSignedInteger(str)); }
inline void clampedNumberFromString(u8* value, tukk str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(unsigned short* value, tukk str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(u32* value, tukk str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(u64* value, tukk str)		{ clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(zu64* value, tukk str) { clampToType(value, stringToUnsignedInteger(str)); }
inline void clampedNumberFromString(float* value, tukk str)
{
	double v = parseFloat(str);
	if (v > FLT_MAX)
		v = FLT_MAX;
	if (v < -FLT_MAX)
		v = -FLT_MAX;
	*value = float(v);
}

inline void clampedNumberFromString(double* value, tukk str)
{
	*value = parseFloat(str);
}


template<class Type>
class PropertyRowNumber : public PropertyRowNumberField{
public:
	PropertyRowNumber()
	{
		hardLimit_.min = limit_min((Type)0);
		hardLimit_.max = limit_max((Type)0);
		softLimit_ = hardLimit_;
		singleStep_ = yasli::DefaultSinglestep<Type>::value();
	}

	void setVal(Type value, ukk handle, const yasli::TypeID& type) {
		value_ = value;
		serializer_.setPointer((uk )handle);
		serializer_.setType(type);
	}
	bool setValueFromString(tukk str) override{
        Type value = value_;
		clampedNumberFromString(&value_, str);
        return value_ != value;
	}
	yasli::string valueAsString() const override{ 
        return numberAsString(Type(value_));
	}

	bool assignToPrimitive(uk object, size_t size) const override{
		*reinterpret_cast<Type*>(object) = value_;
		return true;
	}

	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		yasli::RangeDecorator<Type>* range = (yasli::RangeDecorator<Type>*)ser.pointer();
		serializer_.setPointer((uk )range->value);
		serializer_.setType(yasli::TypeID::get<Type>());
		value_ = *range->value;
		hardLimit_.min = range->hardMin;
		hardLimit_.max = range->hardMax;
		softLimit_.min = range->softMin;
		softLimit_.max = range->softMax;
		singleStep_ = (double)range->singleStep;
	}

  bool assignTo(const yasli::Serializer& ser) const override 
	{
		if (ser.type() == yasli::TypeID::get<yasli::RangeDecorator<Type>>()) {
			yasli::RangeDecorator<Type>* range = (yasli::RangeDecorator<Type>*)ser.pointer();
			*range->value = value_;
		}
		else if (ser.type() == yasli::TypeID::get<Type>()) {
			*(Type*)ser.pointer() = value_;
		}			
    return true;
	}

	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(hardLimit_.min, "hardMin", "HardMin");
		ar(hardLimit_.max, "hardMax", "HardMax");
		ar(softLimit_.min, "softMin", "SoftMin");
		ar(softLimit_.max, "softMax", "SoftMax");
	}

	void startIncrement() override
	{
		incrementStartValue_ = value_;
	}

	void endIncrement(PropertyTree* tree) override
	{
		if (value_ != incrementStartValue_) {
			Type value = value_;
			value_ = incrementStartValue_;
			value_ = value;
			tree->model()->rowChanged(this, true);
		}
	}

	void incrementLog(float screenFraction, float valueFieldFraction) override
	{
		double screenFractionMultiplier = 1000.0;
		if (yasli::TypeID::get<Type>() == yasli::TypeID::get<float>() ||
			 	yasli::TypeID::get<Type>() == yasli::TypeID::get<double>()) 
			screenFractionMultiplier = 10.0;

		double startPower = log10(fabs(double(incrementStartValue_)) + 1.0) - 3.0;
		double power = startPower + fabs(screenFraction) * 10.0f;
		double delta = pow(10.0, power) - pow(10.0, startPower) + screenFractionMultiplier * fabs(screenFraction);
		double newValue;
		if (screenFraction > 0.0f)
			newValue = double(incrementStartValue_) + delta;
		else
			newValue = double(incrementStartValue_) - delta;
#ifdef _MSC_VER
		if (_isnan(newValue)) {
#else
		if (isnan(newValue)) {
#endif
			if (screenFraction > 0.0f)
				newValue = DBL_MAX;
			else
				newValue = -DBL_MAX;
		}
		value_ = clamp(newValue, softLimit_.min, softLimit_.max);
	}

	void increment(PropertyTree* tree, i32 mouseDiff, Modifier modifier) override
	{
		double add = 0.0;
		if (softLimit_.min != std::numeric_limits<Type>::min() && softLimit_.max != std::numeric_limits<Type>::max())
		{
			add = (softLimit_.max - softLimit_.min) * double(mouseDiff) / double(widgetRect(tree).width());
		}
		else
		{
			add = 0.1 * double(mouseDiff);
		}
		
		if (MODIFIER_CONTROL == modifier)
		{
			add *= 0.01;
		}
		else if (MODIFIER_SHIFT == modifier)
		{
			add *= 100.0;
		}

		value_ = clamp(value_ + add, softLimit_.min, softLimit_.max);
	}

	void addValue(PropertyTree* tree, double value) override
	{
		tree->model()->rowAboutToBeChanged(this);
		value_ = clamp(value_ + value, hardLimit_.min, hardLimit_.max);
		tree->model()->rowChanged(this, true);
	}

	bool hasMinMax() const
	{
		return hardLimit_.min != std::numeric_limits<Type>::lowest() && hardLimit_.max != std::numeric_limits<Type>::max();
	}

	double minValue() const override { return hasMinMax() ? hardLimit_.min : -DBL_MAX; }
	double maxValue() const override { return hasMinMax() ? hardLimit_.max : DBL_MAX; }

	double singlestep() const override
	{
		return singleStep_;
	}

protected:
	Type incrementStartValue_; 
	Type value_; 
	struct SLimits { Type min, max; };
	// Enforced limit that the property can absolutely not exceed
	SLimits hardLimit_;
	// Soft limit enforced by UI sliders etc, should be lower than the hard limit
	SLimits softLimit_;
	double singleStep_;
};

