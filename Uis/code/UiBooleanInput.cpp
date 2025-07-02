#include <drx3D/Uis/Inputs/UiBooleanInput.h>

#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiBooleanInput::UiBooleanInput() {
	//slider.SetTransform({UiMargins::All});
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

	SetCursorHover(std::make_unique<Cursor>(CursorStandard::Hand));
	OnSelected().connect([this](bool selected) {
		background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(), 
			selected ? UiButtonInput::SelectedColor : UiButtonInput::PrimaryColor,
			UiButtonInput::SlideTime);
	});
	OnClick().connect(this, [this](MouseButton button) {
		if (button == MouseButton::Left) {
			CancelEvent(MouseButton::Left);
			value = !value;
			onValue(value);
			UpdateValue();
		}
	});
	UpdateValue();
}

void UiBooleanInput::UpdateObject() {
}

void UiBooleanInput::SetValue(bool value) {
	this->value = value;
	UpdateValue();
	//onValue(value);
}

void UiBooleanInput::UpdateValue() {
	textValue.SetString(String::To(value));
	slider.SetEnabled(value);
}
}
