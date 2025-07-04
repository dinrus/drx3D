#include <drx3D/Uis/Uis.h>

#include <drx3D/Uis/Constraints/PixelConstraint.h>

namespace drx3d {
Uis::Uis() {
	canvas.GetConstraints().SetWidth<PixelConstraint>(0)
		.SetHeight<PixelConstraint>(0);
	for (auto button : magic_enum::enum_values<MouseButton>())
		selectors.emplace(button, SelectorMouse());
}

void Uis::Update() {
	for (auto &[button, selector] : selectors) {
		auto isDown = Windows::Get()->GetWindow(0)->GetMouseButton(button) != InputAction::Release;
		selector.wasDown = !selector.isDown && isDown;
		selector.isDown = isDown;
	}

	auto lastCursorSelect = cursorSelect;
	cursorSelect = nullptr;

	objects.clear();
	auto viewMatrix = Matrix4::OrthographicMatrix(0.0f, Windows::Get()->GetWindow(0)->GetSize().x, 
		0.0f, Windows::Get()->GetWindow(0)->GetSize().y, -1.0f, 1.0f);
	canvas.GetConstraints().GetWidth()->SetOffset(Windows::Get()->GetWindow(0)->GetSize().x);
	canvas.GetConstraints().GetHeight()->SetOffset(Windows::Get()->GetWindow(0)->GetSize().y);
	canvas.Update(viewMatrix, objects, cursorSelect);

	if (lastCursorSelect != cursorSelect) {
		Windows::Get()->GetWindow(0)->SetCursor(cursorSelect ? cursorSelect->GetCursorHover() : nullptr);
	}
}

bool Uis::IsDown(MouseButton button) {
	return selectors[button].isDown;
}

bool Uis::WasDown(MouseButton button) {
	return selectors[button].wasDown;
}

void Uis::CancelWasEvent(MouseButton button) {
	selectors[button].wasDown = false;
}
}
