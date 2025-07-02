#pragma once

#include <drx3D/Inputs/InputButton.h>
#include <drx3D/Inputs/InputAxis.h>

namespace drx3d {
/**
 * @brief InputAxis composed of two buttons.
 */
class DRX3D_EXPORT ButtonInputAxis : public InputAxis::Registry<ButtonInputAxis> {
	inline static const bool Registered = Register("button");
public:
	/**
	 * Creates a new axis button.
	 */
	ButtonInputAxis() = default;

	/**
	 * Creates a new axis button.
	 * @param negative When this button is down, the axis is negative.
	 * @param positive When this button is down, the axis is positive.
	 */
	ButtonInputAxis(std::unique_ptr<InputButton> &&negative, std::unique_ptr<InputButton> &&positive);

	float GetAmount() const override;

	ArgumentDescription GetArgumentDescription() const override;

	const InputButton *GetNegative() const { return negative.get(); }
	void SetNegative(std::unique_ptr<InputButton> &&negative);
	const InputButton *GetPositive() const { return positive.get(); }
	void SetPositive(std::unique_ptr<InputButton> &&positive);

	friend const Node &operator>>(const Node &node, ButtonInputAxis &inputAxis);
	friend Node &operator<<(Node &node, const ButtonInputAxis &inputAxis);

private:
	std::unique_ptr<InputButton> negative;
	std::unique_ptr<InputButton> positive;
};
}
