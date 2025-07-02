#include <drx3D/Uis/Inputs/UiSliderInput.h>

#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiSliderInput::UiSliderInput() {
	//slider.SetTransform( {UiMargins::All});
	slider.SetImage(Image2d::Create("Guis/Button_Filled.png"));
	slider.SetNinePatches({0.125f, 0.125f, 0.875f, 0.875f});
	slider.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	AddChild(&slider);

	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/Button.png"));
	background.SetNinePatches({0.125f, 0.125f, 0.875f, 0.875f});
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

	SetCursorHover(std::make_unique<Cursor>(CursorStandard::ResizeX));
	UpdateProgress();
}

void UiSliderInput::UpdateObject() {
	if (background.IsSelected() && Uis::Get()->WasDown(MouseButton::Left)) {
		updating = true;
		CancelEvent(MouseButton::Left);
	} else if (!Uis::Get()->IsDown(MouseButton::Left)) {
		updating = false;
	} else if (updating) {
		auto width = background.GetScreenSize().x;
		auto positionX = background.GetScreenPosition().x;
		auto cursorX = static_cast<float>(Windows::Get()->GetWindow(0)->GetMousePosition().x) - positionX;
		progress = cursorX / width;
		progress = std::clamp(progress, 0.0f, 1.0f);
		value = (progress * (valueMax - valueMin)) + valueMin;
		onValue(value);

		CancelEvent(MouseButton::Left);
	}

	UpdateProgress();

	if (background.IsSelected() && !mouseOver) {
		background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(), UiButtonInput::SelectedColor, UiButtonInput::SlideTime);
		mouseOver = true;
	} else if (!background.IsSelected() && mouseOver && !updating) {
		background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(), UiButtonInput::PrimaryColor, UiButtonInput::SlideTime);
		mouseOver = false;
	}

//	slider.GetTransform().SetAnchor1({progress - 1.0f, 0.0f});
}

void UiSliderInput::SetValue(float value) {
	this->value = value;
	UpdateProgress();
	//onValue(value);
}

void UiSliderInput::UpdateProgress() {
	progress = (value - valueMin) / (valueMax - valueMin);
	std::stringstream rounded;
	rounded << std::fixed << std::setprecision(roundTo) << value;
	textValue.SetString(rounded.str());
}
}
