#pragma once

#include <drx3D/Inputs/InputAxis.h>

namespace drx3d {
/**
 * @brief InputAxis input from a mouse.
 */
class DRX3D_EXPORT MouseInputAxis : public InputAxis::Registry<MouseInputAxis> {
	inline static const bool Registered = Register("mouse");
public:
	/**
	 * Creates a new axis mouse.
	 * @param axis The axis on the mouse delta is being checked.
	 */
	explicit MouseInputAxis(uint8_t axis = 0);

	float GetAmount() const override;

	ArgumentDescription GetArgumentDescription() const override;

	uint8_t GetAxis() const { return axis; }
	void SetAxis(uint8_t axis) { this->axis = axis; }

	friend const Node &operator>>(const Node &node, MouseInputAxis &inputAxis);
	friend Node &operator<<(Node &node, const MouseInputAxis &inputAxis);

private:
	uint8_t axis;
};
}
