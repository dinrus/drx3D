#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Uis/Inputs/UiButtonInput.h>

namespace drx3d {
class DRX3D_EXPORT UiSliderInput : public UiObject {
public:
	UiSliderInput();

	void UpdateObject() override;

	float GetValueMin() const { return valueMin; }
	void SetValueMin(float valueMin) { this->valueMin = valueMin; }

	float GetValueMax() const { return valueMax; }
	void SetValueMax(float valueMax) { this->valueMax = valueMax; }

	const STxt &GetTitle() const { return textTitle.GetString(); }
	void SetTitle(const STxt &string) { textTitle.SetString(string); }

	float GetValue() const { return value; }
	void SetValue(float value);

	float GetProgress() const { return progress; }

	int32_t GetRoundTo() const { return roundTo; }
	void SetRoundTo(int32_t roundTo) { this->roundTo = roundTo; }

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(float)> &OnValue() { return onValue; }

private:
	void UpdateProgress();

	Gui slider;
	Gui background;
	Text textTitle;
	Text textValue;

	float value = 0.5f;
	float valueMin = 0.0f, valueMax = 1.0f;
	float progress = 0.0f;
	int32_t roundTo = 2;

	bool updating = false;
	bool mouseOver = false;

	rocket::signal<void(float)> onValue;
};
}
