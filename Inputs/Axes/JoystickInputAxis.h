#pragma once

#include <drx3D/Devices/Joysticks.h>
#include <drx3D/Inputs/InputAxis.h>

namespace drx3d {
/**
 * @brief InputAxis input from a joystick input device.
 */
class DRX3D_EXPORT JoystickInputAxis : public InputAxis::Registry<JoystickInputAxis> {
	inline static const bool Registered = Register("joystick");
public:
	/**
	 * Creates a new axis joystick.
	 * @param port The joystick port.
	 * @param axis The axis on the joystick being checked.
	 */
	explicit JoystickInputAxis(JoystickPort port = JoystickPort::_1, JoystickAxis axis = 0);

	float GetAmount() const override;

	ArgumentDescription GetArgumentDescription() const override;

	bool IsConnected() const { return joystick->IsConnected(); }

	JoystickPort GetPort() const { return joystick->GetPort(); }
	void SetPort(JoystickPort port);

	JoystickAxis GetAxis() const { return axis; }
	void SetAxis(JoystickAxis axis) { this->axis = axis; }

	friend const Node &operator>>(const Node &node, JoystickInputAxis &joystick);
	friend Node &operator<<(Node &node, const JoystickInputAxis &joystick);

private:
	Joystick *joystick;
	JoystickAxis axis;
};
}
