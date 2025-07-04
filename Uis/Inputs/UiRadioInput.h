#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Uis/Inputs/UiButtonInput.h>

namespace drx3d {
class DRX3D_EXPORT UiRadioInput : public UiObject {
public:
	enum class Type {
		Filled, X, Dot, Check
	};

	UiRadioInput();

	void UpdateObject() override;

	const STxt &GetTitle() const { return title.GetString(); }
	void SetTitle(const STxt &string) { title.SetString(string); }

	bool GetValue() const { return value; }
	void SetValue(bool value);

	const Type &GetType() const { return type; }
	void SetType(const Type &type);

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(bool)> &OnValue() { return onValue; }

private:
	void UpdateValue();

	Gui background;
	Gui fill;
	Text title;

	bool value = false;
	Type type = Type::Filled;

	rocket::signal<void(bool)> onValue;
};

class DRX3D_EXPORT UiRadioManager : public virtual rocket::trackable {
public:
	UiRadioManager() = default;

	const UiRadioInput::Type &GetMarkType() const { return type; }

	void SetMarkType(const UiRadioInput::Type &type) {
		this->type = type;

		for (auto &input : inputs) {
			input->SetType(type);
		}
	}

	const std::vector<UiRadioInput *> &GetInputs() const { return inputs; }

	void AddInputs(const std::vector<UiRadioInput *> &inputs) {
		for (auto &input : inputs) {
			AddInput(input);
		}
	}

	void AddInput(UiRadioInput *input) {
		inputs.emplace_back(input);
		input->SetType(type);
		input->OnValue().connect(this, [this, input](bool value) {
			if (!multiple) {
				for (auto &input2 : inputs) {
					if (input2 != input) {
						input2->SetValue(false);
					}
				}
			}
		});
	}

	void RemoveInput(UiRadioInput *input) {
		inputs.erase(std::remove(inputs.begin(), inputs.end(), input), inputs.end());
	}

	void ClearInputs() { inputs.clear(); }

private:
	UiRadioInput::Type type = UiRadioInput::Type::X;
	bool multiple = false;
	std::vector<UiRadioInput *> inputs;
};
}
