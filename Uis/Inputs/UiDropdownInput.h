#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Uis/Inputs/UiButtonInput.h>

namespace drx3d {
class DRX3D_EXPORT UiDropdownInput : public UiObject {
public:
	UiDropdownInput();

	void UpdateObject() override;

	const STxt &GetTitle() const { return textTitle.GetString(); }
	void SetTitle(const STxt &string) { textTitle.SetString(string); }

	uint32_t GetValue() const { return value; }
	void SetValue(uint32_t value);

	const std::vector<STxt> &GetOptions() const { return options; }
	void SetOptions(const std::vector<STxt> &options);

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(uint32_t)> &OnValue() { return onValue; }

private:
	//Gui slider;
	Gui background;
	Text textTitle;

	uint32_t value;
	std::vector<STxt> options;

	rocket::signal<void(uint32_t)> onValue;
};
}
