#ifndef GWEN_PARAMETER_INTERFACE_H
#define GWEN_PARAMETER_INTERFACE_H

#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

struct GwenParameterInterface : public CommonParameterInterface
{
	struct GwenInternalData* m_gwenInternalData;

	struct GwenParameters* m_paramInternalData;

	GwenParameterInterface(struct GwenInternalData* gwenInternalData);
	virtual ~GwenParameterInterface();
	virtual void registerSliderFloatParameter(SliderParams& params);
	virtual void registerButtonParameter(ButtonParams& params);
	virtual void registerComboBox(ComboBoxParams& params);

	virtual void setSliderValue(i32 sliderIndex, double sliderValue);
	virtual void syncParameters();
	virtual void removeAllParameters();
};

#endif  //GWEN_PARAMETER_INTERFACE_H
