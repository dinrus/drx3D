#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Uis/Inputs/UiButtonInput.h>

namespace drx3d {
class DRX3D_EXPORT UiBooleanInput : public UiObject {
public:
	UiBooleanInput();

	void UpdateObject() override;

	const STxt &GetTitle() const { return textTitle.GetString(); }
	void SetTitle(const STxt &string) { textTitle.SetString(string); }

	bool GetValue() const { return value; }
	void SetValue(bool value);

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(bool)> &OnValue() { return onValue; }

private:
	void UpdateValue();

	Gui slider;
	Gui background;
	Text textTitle;
	Text textValue;

	bool value = false;

	rocket::signal<void(bool)> onValue;
};
}
