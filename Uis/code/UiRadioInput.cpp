#include <drx3D/Uis/Inputs/UiRadioInput.h>

#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiRadioInput::UiRadioInput() {
	//background.SetTransform({{24, 24}, UiAnchor::LeftCentre});
	background.SetImage(Image2d::Create("Guis/Radio.png"));
	background.SetNinePatches({0.125f, 0.125f, 0.875f, 0.875f});
	background.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	AddChild(&background);

	//fill.SetTransform({{24, 24}, UiAnchor::Centre});
	fill.SetColorDriver<ConstantDriver>(UiButtonInput::SelectedColor);
	background.AddChild(&fill);

	//title.SetTransform({{140, 24}, UiAnchor::LeftCentre, {29, 0}});
	title.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	title.SetFontSize(UiButtonInput::FontSize);
	title.SetTextColor(UiButtonInput::ValueColor);
	AddChild(&title);
	
	SetCursorHover(std::make_unique<Cursor>(CursorStandard::Hand));
	OnSelected().connect(this, [this](bool selected) {
		background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(),
			selected ? UiButtonInput::SelectedColor : UiButtonInput::PrimaryColor, UiButtonInput::SlideTime);
	});
	OnClick().connect(this, [this](MouseButton button) {
		if (button == MouseButton::Left) {
			CancelEvent(MouseButton::Left);

			value = !value;
			UpdateValue();
			onValue(value);
		}
	});
	UpdateValue();
}

void UiRadioInput::UpdateObject() {
}

void UiRadioInput::SetValue(bool value) {
	this->value = value;
	UpdateValue();
	//onValue(value);
}

void UiRadioInput::SetType(const Type &type) {
	this->type = type;
	UpdateValue();
}

void UiRadioInput::UpdateValue() {
	switch (type) {
	case Type::Filled:
		fill.SetImage(Image2d::Create("Guis/Radio_Filled.png"));
		break;
	case Type::X:
		fill.SetImage(Image2d::Create("Guis/Radio_X.png"));
		break;
	case Type::Check:
		fill.SetImage(Image2d::Create("Guis/Radio_Check.png"));
		break;
	case Type::Dot:
		fill.SetImage(Image2d::Create("Guis/Radio_Dot.png"));
		break;
	}

	fill.SetAlphaDriver<SlideDriver>(fill.GetAlphaDriver()->Get(), value, UiButtonInput::SlideTime);
}
}
