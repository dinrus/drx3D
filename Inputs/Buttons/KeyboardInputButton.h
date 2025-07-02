#pragma once

#include <drx3D/Inputs/InputButton.h>

namespace drx3d {
/**
 * @brief InputButton input from the keyboard input device.
 */
class DRX3D_EXPORT KeyboardInputButton : public InputButton::Registry<KeyboardInputButton> {
	inline static const bool Registered = Register("keyboard");
public:
	/**
	 * Creates a new button keyboard.
	 * @param key The key on the keyboard being checked.
	 */
	explicit KeyboardInputButton(Key key = Key::Unknown);

	bool IsDown() const override;

	InputAxis::ArgumentDescription GetArgumentDescription() const override;

	Key GetKey() const { return key; }
	void SetKey(Key key) { this->key = key; }

	friend const Node &operator>>(const Node &node, KeyboardInputButton &inputButton);
	friend Node &operator<<(Node &node, const KeyboardInputButton &inputButton);

private:
	Key key;
};
}
