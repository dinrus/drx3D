#pragma once

#include <drx3D/Inputs/InputButton.h>

namespace drx3d {
/**
 * @brief InputButton input from the mouse input device.
 */
class DRX3D_EXPORT MouseInputButton : public InputButton::Registry<MouseInputButton> {
	inline static const bool Registered = Register("mouse");
public:
	/**
	 * Creates a new button mouse.
	 * @param button The button on the mouse being checked.
	 */
	explicit MouseInputButton(MouseButton button = MouseButton::_1);

	bool IsDown() const override;

	InputAxis::ArgumentDescription GetArgumentDescription() const override;

	MouseButton GetButton() const { return button; }
	void SetButton(MouseButton button) { this->button = button; }

	friend const Node &operator>>(const Node &node, MouseInputButton &inputButton);
	friend Node &operator<<(Node &node, const MouseInputButton &inputButton);

private:
	MouseButton button;
};
}
