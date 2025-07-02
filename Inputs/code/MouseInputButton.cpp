#include <drx3D/Inputs/Buttons/MouseInputButton.h>

namespace drx3d {
MouseInputButton::MouseInputButton(MouseButton button) :
	button(button) {
	Windows::Get()->GetWindow(0)->OnMouseButton().connect(this, [this](MouseButton button, InputAction action, bitmask::bitmask<InputMod> mods) {
		if (this->button == button) {
			onButton(action, mods);
		}
	});
}

bool MouseInputButton::IsDown() const {
	return (Windows::Get()->GetWindow(0)->GetMouseButton(button) != InputAction::Release) ^ inverted;
}

InputAxis::ArgumentDescription MouseInputButton::GetArgumentDescription() const {
	return {
		{"inverted", "bool", "If the down reading will be inverted"},
		{"key", "i32", "The mouse button on the mouse being checked"}
	};
}

const Node &operator>>(const Node &node, MouseInputButton &inputButton) {
	node["inverted"].Get(inputButton.inverted);
	node["button"].Get(inputButton.button);
	return node;
}

Node &operator<<(Node &node, const MouseInputButton &inputButton) {
	node["inverted"].Set(inputButton.inverted);
	node["button"].Set(inputButton.button);
	return node;
}
}
