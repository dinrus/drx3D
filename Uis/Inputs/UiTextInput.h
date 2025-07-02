#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Inputs/InputDelay.h>
#include <drx3D/Uis/Inputs/UiButtonInput.h>

namespace drx3d {
class DRX3D_EXPORT UiTextInput : public UiObject {
public:
	UiTextInput();

	void UpdateObject() override;

	const STxt &GetTitle() const { return textTitle.GetString(); }
	void SetTitle(const STxt &string) { textTitle.SetString(string); }

	const STxt &GetValue() const { return value; }
	void SetValue(const STxt &value);

	int32_t GetMaxLength() const { return maxLength; }
	void SetMaxLength(int32_t maxLength) { this->maxLength = maxLength; }

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(STxt)> &OnValue() { return onValue; }

private:
	void SetUpdating(bool updating);

	Gui background;
	Text textTitle;
	Text textValue;

	STxt value;
	int32_t maxLength = 16;

	InputDelay inputDelay;
	int32_t lastKey = 0;

	bool updating = false;
	bool mouseOver = false;

	rocket::signal<void(STxt)> onValue;
};
}
