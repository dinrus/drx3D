#include <drx3D/Uis/Inputs/UiGrabberInput.h>

#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiGrabberInput::UiGrabberInput() {
	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/Button.png"));
	background.SetNinePatches(Vector4f(0.125f, 0.125f, 0.875f, 0.875f));
	background.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	AddChild(&background);

	//textTitle.SetTransform({UiMargins::None, UiButtonInput::Padding, -UiButtonInput::Padding});
	textTitle.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	textTitle.SetFontSize(UiButtonInput::FontSize);
	textTitle.SetJustify(Text::Justify::Right);
	textTitle.SetTextColor(UiButtonInput::TitleColor);
	AddChild(&textTitle);

	//textValue.SetTransform({UiMargins::None, UiButtonInput::Padding, -UiButtonInput::Padding});
	textValue.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	textValue.SetFontSize(UiButtonInput::FontSize);
	textValue.SetJustify(Text::Justify::Left);
	textValue.SetTextColor(UiButtonInput::ValueColor);
	AddChild(&textValue);
	
	SetCursorHover(std::make_unique<Cursor>(CursorStandard::Hand));
}

void UiGrabberInput::UpdateObject() {
	if (Uis::Get()->WasDown(MouseButton::Left)) {
		if (background.IsSelected()) {
			SetUpdating(true);
			CancelEvent(MouseButton::Left);
		} else if (updating) {
			SetUpdating(false);
			CancelEvent(MouseButton::Left);
		}
	}
	if (!updating) {
		if (background.IsSelected() && !mouseOver) {
			background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(), UiButtonInput::SelectedColor, UiButtonInput::SlideTime);
			mouseOver = true;
		} else if (!background.IsSelected() && mouseOver) {
			background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(), UiButtonInput::PrimaryColor, UiButtonInput::SlideTime);
			mouseOver = false;
		}
	}
}

void UiGrabberInput::SetUpdating(bool updating) {
	this->updating = updating;
	mouseOver = true;
}

void UiGrabberInput::UpdateValue() {
	textValue.SetString(GetTextString());
}

UiGrabberJoystick::UiGrabberJoystick() {
	UpdateValue();

	Joysticks::Get()->GetJoystick(port)->OnButton().connect(this, [this](uint32_t button, InputAction action) {
		if (!updating)
			return;

		value = button;
		onValue(port, value);
		SetUpdating(false);
		UpdateValue();
	});
}

void UiGrabberJoystick::SetValue(uint32_t value) {
	this->value = value;
	UpdateValue();
	//onValue(value);
}

UiGrabberKeyboard::UiGrabberKeyboard() {
	UpdateValue();

	Windows::Get()->GetWindow(0)->OnKey().connect(this, [this](Key key, InputAction action, bitmask::bitmask<InputMod> mods) {
		if (!updating)
			return;

		value = key;
		onValue(value);
		SetUpdating(false);
		UpdateValue();
	});
}

void UiGrabberKeyboard::SetValue(Key value) {
	this->value = value;
	UpdateValue();
	//onValue(value);
}

UiGrabberMouse::UiGrabberMouse() {
	UpdateValue();

	Windows::Get()->GetWindow(0)->OnMouseButton().connect(this, [this](MouseButton button, InputAction action, bitmask::bitmask<InputMod> mods) {
		if (!updating || action != InputAction::Press)
			return;

		if (button == MouseButton::Left) {
			if (!background.IsSelected()) {
				SetUpdating(false);
				return;
			}

			CancelEvent(MouseButton::Left);
		}

		value = button;
		onValue(value);
		SetUpdating(false);
		UpdateValue();
	});
}

void UiGrabberMouse::SetValue(MouseButton value) {
	this->value = value;
	UpdateValue();
	//onValue(value);
}
}
