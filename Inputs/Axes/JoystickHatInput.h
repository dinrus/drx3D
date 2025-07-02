#pragma once

#include <drx3D/Devices/Joysticks.h>
#include <drx3D/Inputs/InputButton.h>
#include <drx3D/Inputs/InputAxis.h>

namespace drx3d {
/**
 * @brief InputButton from a joystick.
 */
class DRX3D_EXPORT JoystickHatInput : public InputAxis::Registry<JoystickHatInput>, public InputButton::Registry<JoystickHatInput> {
	inline static const bool Registered = InputAxis::Registry<JoystickHatInput>::Register("joystickHat") &&
		InputButton::Registry<JoystickHatInput>::Register("joystickHat");
public:
	/**
	 * Creates a new joystick button.
	 * @param port The joystick port.
	 * @param hat The hat that will be checked.
	 * @param hatFlags If this bit is found the hat will trigger {@link JoystickHatInput#IsDown}.
	 */
	explicit JoystickHatInput(JoystickPort port = JoystickPort::_1, JoystickHat hat = 0, const bitmask::bitmask<JoystickHatValue> &hatFlags = JoystickHatValue::Centered);

	ArgumentDescription GetArgumentDescription() const override;

	float GetAmount() const override;
	bool IsDown() const override;

	bool IsConnected() const { return joystick->IsConnected(); }

	JoystickPort GetPort() const { return joystick->GetPort(); }
	void SetPort(JoystickPort port);

	JoystickHat GetHat() const { return hat; }
	void SetHat(JoystickHat hat) { this->hat = hat; }

	const bitmask::bitmask<JoystickHatValue> &GetHatFlags() const { return hatFlags; }
	void SetHatFlags(JoystickHatValue hatFlags) { this->hatFlags = hatFlags; }

	friend const Node &operator>>(const Node &node, JoystickHatInput &input);
	friend Node &operator<<(Node &node, const JoystickHatInput &input);

private:
	Joystick *joystick;
	JoystickHat hat;
	bitmask::bitmask<JoystickHatValue> hatFlags;
	bool lastDown = false;
};

using JoystickHatInputAxis = JoystickHatInput;
using JoystickHatInputButton = JoystickHatInput;
}
