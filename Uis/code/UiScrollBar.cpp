#include <drx3D/Uis/UiScrollBar.h>

#include <drx3D/Uis/Inputs/UiButtonInput.h>
#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiScrollBar::UiScrollBar() {
	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/White.png"));
	background.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	AddChild(&background);

	//scroll.SetTransform({UiMargins::RightBottom});
	scroll.SetImage(Image2d::Create("Guis/White.png"));
	scroll.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	AddChild(&scroll);

	Windows::Get()->GetWindow(0)->OnMouseScroll().connect(this, [this](Vector2d wheelDelta) {
		if (GetParent()->IsSelected() && !updating && scroll.IsEnabled()) {
			Vector2f position;
			position[index] = ScrollByDelta(wheelDelta[index]);
//			scroll.GetTransform().SetPosition(position);
		}
	});
}

void UiScrollBar::UpdateObject() {
	if (scroll.IsSelected() && Uis::Get()->WasDown(MouseButton::Left)) {
		updating = true;
	} else if (updating) {
		if (!Uis::Get()->IsDown(MouseButton::Left)) {
			updating = false;
		}

		Vector2d position;
		position[index] = Windows::Get()->GetWindow(0)->GetMousePosition()[index] - GetScreenPosition()[index]; // ScrollByPosition(Mouse::Get()->GetPosition()[index]);
//		scroll.GetTransform().SetPosition(position);
		CancelEvent(MouseButton::Left);
	}

	if (!updating) {
		if (scroll.IsSelected() && !mouseOver) {
			scroll.SetColorDriver<SlideDriver>(scroll.GetColorDriver()->Get(), UiButtonInput::SelectedColor, UiButtonInput::SlideTime);
			mouseOver = true;
		} else if (!scroll.IsSelected() && mouseOver) {
			scroll.SetColorDriver<SlideDriver>(scroll.GetColorDriver()->Get(), UiButtonInput::ButtonColor, UiButtonInput::SlideTime);
			mouseOver = false;
		}
	}
}

float UiScrollBar::GetProgress() {
	// TODO: Mark const
//	return scroll.GetTransform().GetPosition()[index];
	return 0.0f;
}

void UiScrollBar::SetSize(const Vector2f &size) {
//	scroll.GetTransform().SetAnchor0(scroll.GetTransform().GetPosition());
//	scroll.GetTransform().SetAnchor1(scroll.GetTransform().GetPosition() + size);
}

void UiScrollBar::SetType(ScrollBar type) {
	index = type == ScrollBar::Vertical;
}

float UiScrollBar::ScrollByDelta(float delta) const {
	float puckLength = scroll.GetScreenSize()[index];
	float barLength = GetParent()->GetScreenSize()[index];
	auto maxValue = (barLength - puckLength) / barLength;
	float value = scroll.GetScreenPosition()[index];
	value += delta;
	return std::clamp(value, 0.0f, maxValue);
}

float UiScrollBar::ScrollByPosition(float position) const {
	float puckLength = scroll.GetScreenSize()[index];
	float barLength = GetParent()->GetScreenSize()[index];
	auto maxValue = (barLength - puckLength) / barLength;
	float positionLength = GetParent()->GetScreenPosition()[index];
	auto cursorLength = (position - positionLength) - (puckLength / 2.0f);
	return std::clamp(cursorLength / barLength, 0.0f, maxValue);
}
}
