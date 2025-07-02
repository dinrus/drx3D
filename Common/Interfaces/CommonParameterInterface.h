
#ifndef PARAM_INTERFACE_H
#define PARAM_INTERFACE_H

#pragma once

#include <drxtypes.h>

typedef void (*SliderParamChangedCallback)(float newVal, uk userPointer);
#include <drx3D/Maths/Linear/Scalar.h>

struct SliderParams
{
	tukk m_name;
	float m_minVal;
	float m_maxVal;
	SliderParamChangedCallback m_callback;
	Scalar* m_paramValuePointer;
	uk m_userPointer;
	bool m_clampToNotches;
	bool m_clampToIntegers;
	bool m_showValues;

	SliderParams(tukk name, Scalar* targetValuePointer)
		: m_name(name),
		  m_minVal(-100),
		  m_maxVal(100),
		  m_callback(0),
		  m_paramValuePointer(targetValuePointer),
		  m_userPointer(0),
		  m_clampToNotches(false),
		  m_clampToIntegers(false),
		  m_showValues(true)
	{
	}
};

typedef void (*ButtonParamChangedCallback)(i32 buttonId, bool buttonState, uk userPointer);
typedef void (*ComboBoxCallback)(i32 combobox, tukk item, uk userPointer);

struct ButtonParams
{
	tukk m_name;
	i32 m_buttonId;
	uk m_userPointer;
	bool m_isTrigger;
	bool m_initialState;

	ButtonParamChangedCallback m_callback;
	ButtonParams(tukk name, i32 buttonId, bool isTrigger)
		: m_name(name),
		  m_buttonId(buttonId),
		  m_userPointer(0),
		  m_isTrigger(isTrigger),
		  m_initialState(false),
		  m_callback(0)
	{
	}
};

struct ComboBoxParams
{
	i32 m_comboboxId;
	i32 m_numItems;
	tukk* m_items;
	i32 m_startItem;
	ComboBoxCallback m_callback;
	uk m_userPointer;

	ComboBoxParams()
		: m_comboboxId(-1),
		  m_numItems(0),
		  m_items(0),
		  m_startItem(0),
		  m_callback(0),
		  m_userPointer(0)
	{
	}
};

struct CommonParameterInterface
{
	virtual ~CommonParameterInterface() {}
	virtual void registerSliderFloatParameter(SliderParams& params) = 0;
	virtual void registerButtonParameter(ButtonParams& params) = 0;
	virtual void registerComboBox(ComboBoxParams& params) = 0;

	virtual void syncParameters() = 0;
	virtual void removeAllParameters() = 0;
	virtual void setSliderValue(i32 sliderIndex, double sliderValue) = 0;
};

#endif  //PARAM_INTERFACE_H
