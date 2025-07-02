#include "../GwenParameterInterface.h"
#include "../gwenInternalData.h"
#include <cstring>
#ifdef _WIN32
#define safe_printf _snprintf
#else
#define safe_printf snprintf
#endif
struct MyButtonEventHandler : public Gwen::Event::Handler
{
	Gwen::Controls::Button* m_buttonControl;
	ButtonParamChangedCallback m_callback;
	uk m_userPointer;
	i32 m_buttonId;

	MyButtonEventHandler(Gwen::Controls::Button* buttonControl, ButtonParamChangedCallback callback, i32 buttonId, uk userPointer)
		: m_buttonControl(buttonControl),
		  m_callback(callback),
		  m_userPointer(userPointer),
		  m_buttonId(buttonId)
	{
	}

	void onButtonPress(Gwen::Controls::Base* pControl)
	{
		if (m_callback)
		{
			bool buttonState = true;
			if (m_buttonControl->IsToggle())
			{
				buttonState = m_buttonControl->GetToggleState();
			}
			(*m_callback)(m_buttonId, buttonState, m_userPointer);
		}
	}
};

template <typename T>
struct MySliderEventHandler : public Gwen::Event::Handler
{
	SliderParamChangedCallback m_callback;
	uk m_userPointer;
	Gwen::Controls::TextBox* m_label;
	Gwen::Controls::Slider* m_pSlider;
	char m_variableName[1024];
	T* m_targetValue;
	bool m_showValue;

	MySliderEventHandler(tukk varName, Gwen::Controls::TextBox* label, Gwen::Controls::Slider* pSlider, T* target, SliderParamChangedCallback callback, uk userPtr)
		: m_callback(callback),
		  m_userPointer(userPtr),
		  m_label(label),
		  m_pSlider(pSlider),
		  m_targetValue(target),
		  m_showValue(true)

	{
		strncpy(m_variableName, varName, sizeof(m_variableName));
	}

	void SliderMoved(Gwen::Controls::Base* pControl)
	{
		Gwen::Controls::Slider* pSlider = (Gwen::Controls::Slider*)pControl;
		//printf("value = %f\n", pSlider->GetVal());//UnitPrint( Utility::Format( L"Slider Value: %.2f", pSlider->GetValue() ) );
		float bla = pSlider->GetVal();
		T v = T(bla);
		SetValue(v);

		if (m_callback)
		{
			(*m_callback)(v, m_userPointer);
		}
	}

	void SetValue(T v)
	{
		if (v < m_pSlider->GetRangeMin())
		{
			printf("?\n");
		}

		if (v > m_pSlider->GetRangeMax())
		{
			printf("?\n");
		}
		m_pSlider->SetVal(v, true);
		(*m_targetValue) = v;
		float val = float(v);  //todo: specialize on template type
		if (m_showValue)
		{
			char txt[1024];
			safe_printf(txt, sizeof(txt), "%s : %.3f", m_variableName, val);
			m_label->SetText(txt);
		}
	}
};

struct GwenParameters
{
	b3AlignedObjectArray<MySliderEventHandler<Scalar>*> m_sliderEventHandlers;
	b3AlignedObjectArray<Gwen::Controls::HorizontalSlider*> m_sliders;
	b3AlignedObjectArray<Gwen::Controls::ComboBox*> m_comboBoxes;
	b3AlignedObjectArray<Gwen::Controls::Button*> m_buttons;
	b3AlignedObjectArray<MyButtonEventHandler*> m_buttonEventHandlers;
	b3AlignedObjectArray<Gwen::Controls::TextBox*> m_textLabels;
	i32 m_savedYposition;
};

GwenParameterInterface::GwenParameterInterface(GwenInternalData* gwenInternalData)
	: m_gwenInternalData(gwenInternalData)
{
	m_paramInternalData = new GwenParameters;
	m_paramInternalData->m_savedYposition = m_gwenInternalData->m_curYposition;
}

GwenParameterInterface::~GwenParameterInterface()
{
	removeAllParameters();
	delete m_paramInternalData;
}

void GwenParameterInterface::setSliderValue(i32 sliderIndex, double sliderValue)
{
	i32 sliderCapped = sliderValue + 4;
	sliderCapped /= 8;
	sliderCapped *= 8;

	if (sliderIndex >= 0 && sliderIndex < m_paramInternalData->m_sliders.size())
	{
		m_paramInternalData->m_sliders[sliderIndex]->GetRangeMin();

		m_paramInternalData->m_sliders[sliderIndex]->GetRangeMax();
		float mappedValue = m_paramInternalData->m_sliders[sliderIndex]->GetRangeMin() +
							(m_paramInternalData->m_sliders[sliderIndex]->GetRangeMax() -
							 m_paramInternalData->m_sliders[sliderIndex]->GetRangeMin()) *
								sliderCapped / 128.f;
		printf("mappedValue = %f\n", mappedValue);
		m_paramInternalData->m_sliders[sliderIndex]->SetVal(mappedValue);
	}
}

#include <stdio.h>

void GwenParameterInterface::registerButtonParameter(ButtonParams& params)
{
	Gwen::Controls::Button* button = new Gwen::Controls::Button(m_gwenInternalData->m_demoPage->GetPage());
	MyButtonEventHandler* handler = new MyButtonEventHandler(button, params.m_callback, params.m_buttonId, params.m_userPointer);
	button->SetText(params.m_name);
	button->onPress.Add(handler, &MyButtonEventHandler::onButtonPress);
	button->SetIsToggle(params.m_isTrigger);
	button->SetToggleState(params.m_initialState);

	m_paramInternalData->m_buttons.push_back(button);
	m_paramInternalData->m_buttonEventHandlers.push_back(handler);

	button->SetPos(5, m_gwenInternalData->m_curYposition);
	button->SetWidth(220);

	m_gwenInternalData->m_curYposition += 22;
}

struct MyComboBoxHander2 : public Gwen::Event::Handler
{
	GwenInternalData* m_data;
	i32 m_buttonId;
	ComboBoxCallback m_callback;
	uk m_userPointer;

	MyComboBoxHander2(GwenInternalData* data, i32 buttonId, ComboBoxCallback callback, uk userPointer)
		: m_data(data),
		  m_buttonId(buttonId),
		  m_callback(callback),
		  m_userPointer(userPointer)
	{
	}

	void onSelect(Gwen::Controls::Base* pControl)
	{
		Gwen::Controls::ComboBox* but = (Gwen::Controls::ComboBox*)pControl;

		Gwen::Txt str = but->GetSelectedItem()->GetText().ToTxt();

		if (m_callback)
			(*m_callback)(m_buttonId, str.ToStd().c_str(), m_userPointer);
	}
};

void GwenParameterInterface::registerComboBox(ComboBoxParams& params)
{
	Gwen::Controls::ComboBox* combobox = new Gwen::Controls::ComboBox(m_gwenInternalData->m_demoPage->GetPage());
	m_paramInternalData->m_comboBoxes.push_back(combobox);
	MyComboBoxHander2* handler = new MyComboBoxHander2(m_gwenInternalData, params.m_comboboxId, params.m_callback, params.m_userPointer);
	m_gwenInternalData->m_handlers.push_back(handler);

	combobox->onSelection.Add(handler, &MyComboBoxHander2::onSelect);
	i32 ypos = m_gwenInternalData->m_curYposition;
	m_gwenInternalData->m_curYposition += 22;
	combobox->SetPos(5, ypos);
	combobox->SetWidth(220);
	//box->SetPos(120,130);
	for (i32 i = 0; i < params.m_numItems; i++)
	{
		Gwen::Controls::MenuItem* item = combobox->AddItem(params.m_items[i]);
		if (i == params.m_startItem)
			combobox->OnItemSelected(item);
	}
}

void GwenParameterInterface::registerSliderFloatParameter(SliderParams& params)
{
	Gwen::Controls::TextBox* label = new Gwen::Controls::TextBox(m_gwenInternalData->m_demoPage->GetPage());
	m_paramInternalData->m_textLabels.push_back(label);
	//m_data->m_myControls.push_back(label);
	label->SetText(params.m_name);
	label->SetPos(10, 10 + 25);
	label->SetWidth(210);
	label->SetPos(10, m_gwenInternalData->m_curYposition);
	m_gwenInternalData->m_curYposition += 22;

	Gwen::Controls::HorizontalSlider* pSlider = new Gwen::Controls::HorizontalSlider(m_gwenInternalData->m_demoPage->GetPage());
	m_paramInternalData->m_sliders.push_back(pSlider);
	//m_data->m_myControls.push_back(pSlider);
	pSlider->SetPos(10, m_gwenInternalData->m_curYposition);
	pSlider->SetSize(200, 20);
	pSlider->SetRange(params.m_minVal, params.m_maxVal);
	if (params.m_clampToIntegers)
	{
		pSlider->SetNotchCount(i32(params.m_maxVal - params.m_minVal));
		pSlider->SetClampToNotches(true);
	}
	else
	{
		pSlider->SetNotchCount(16);  //float(params.m_maxVal-params.m_minVal)/100.f);
		pSlider->SetClampToNotches(params.m_clampToNotches);
	}
	pSlider->SetVal(*params.m_paramValuePointer);  //dimensions[i] );
	char labelName[1024];
	safe_printf(labelName, sizeof(labelName), "%s", params.m_name);  //axisNames[0]);
	MySliderEventHandler<Scalar>* handler = new MySliderEventHandler<Scalar>(labelName, label, pSlider, params.m_paramValuePointer, params.m_callback, params.m_userPointer);
	handler->m_showValue = params.m_showValues;
	m_paramInternalData->m_sliderEventHandlers.push_back(handler);

	pSlider->onValChanged.Add(handler, &MySliderEventHandler<Scalar>::SliderMoved);
	handler->SliderMoved(pSlider);
	//	float v = pSlider->GetValue();
	m_gwenInternalData->m_curYposition += 22;
}

void GwenParameterInterface::syncParameters()
{
	for (i32 i = 0; i < m_paramInternalData->m_sliderEventHandlers.size(); i++)
	{
		MySliderEventHandler<Scalar>* handler = m_paramInternalData->m_sliderEventHandlers[i];
		handler->m_pSlider->SetVal(*handler->m_targetValue, true);
	}
}

void GwenParameterInterface::removeAllParameters()
{
	for (i32 i = 0; i < m_paramInternalData->m_buttons.size(); i++)
	{
		delete m_paramInternalData->m_buttons[i];
	}
	m_paramInternalData->m_buttons.clear();

	for (i32 i = 0; i < m_paramInternalData->m_buttonEventHandlers.size(); i++)
	{
		delete m_paramInternalData->m_buttonEventHandlers[i];
	}
	m_paramInternalData->m_buttonEventHandlers.clear();

	m_gwenInternalData->m_curYposition += 22;

	for (i32 i = 0; i < m_paramInternalData->m_sliders.size(); i++)
	{
		delete m_paramInternalData->m_sliders[i];
	}
	m_paramInternalData->m_sliders.clear();

	for (i32 i = 0; i < m_paramInternalData->m_sliderEventHandlers.size(); i++)
	{
		delete m_paramInternalData->m_sliderEventHandlers[i];
	}
	m_paramInternalData->m_sliderEventHandlers.clear();

	for (i32 i = 0; i < m_paramInternalData->m_textLabels.size(); i++)
	{
		delete m_paramInternalData->m_textLabels[i];
	}
	m_paramInternalData->m_textLabels.clear();

	for (i32 i = 0; i < m_paramInternalData->m_comboBoxes.size(); i++)
	{
		delete m_paramInternalData->m_comboBoxes[i];
	}
	m_paramInternalData->m_comboBoxes.clear();

	m_gwenInternalData->m_curYposition = this->m_paramInternalData->m_savedYposition;
	for (i32 i = 0; i < m_gwenInternalData->m_handlers.size(); i++)
	{
		delete m_gwenInternalData->m_handlers[i];
	}
	m_gwenInternalData->m_handlers.clear();
}
