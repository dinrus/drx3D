#pragma once

#include <drx3D/Devices/Joysticks.h>
#include <drx3D/Inputs/InputButton.h>

namespace drx3d {
/**
 * @brief InputButton input from a joystick input device.
 */
class DRX3D_EXPORT JoystickInputButton : public InputButton::Registry<JoystickInputButton> {
	inline static const bool Registered = Register("joystick");
public:
	/**
	 * Creates a new joystick button.
	 * @param port The joystick port.
	 * @param button The button on the joystick being checked.
	 */
	explicit JoystickInputButton(JoystickPort port = JoystickPort::_1, JoystickButton button = 0);

	bool IsDown() const override;

	InputAxis::ArgumentDescription GetArgumentDescription() const override;

	bool IsConnected() const { return joystick->IsConnected(); }

	JoystickPort GetPort() const { return joystick->GetPort(); }
	void SetPort(JoystickPort port);

	JoystickButton GetButton() const { return button; }
	void SetButton(JoystickButton button) { this->button = button; }

	friend const Node &operator>>(const Node &node, JoystickInputButton &inputButton);
	friend Node &operator<<(Node &node, const JoystickInputButton &inputButton);

private:
	Joystick *joystick;
	JoystickButton button;
};
}
