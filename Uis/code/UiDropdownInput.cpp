#include <drx3D/Uis/Inputs/UiDropdownInput.h>

#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiDropdownInput::UiDropdownInput() {
	//slider.SetTransform({{0.5f, 0.0f}, UiAnchor::TopCentre, UiAspect::Position | UiAspect::Scale});
	//slider.SetImage(Image2d::Create("Guis/Button_Filled.png"));
	//slider.SetNinePatches({0.125f, 0.125f, 0.875f, 0.875f});
	//slider.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	//slider.SetHeight(1.0f);
	//AddChild(&slider);

	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/Button.png"));
	background.SetNinePatches({0.125f, 0.125f, 0.875f, 0.875f});
	background.SetColorDriver<ConstantDriver>(UiButtonInput::PrimaryColor);
	AddChild(&background);

	//textTitle.SetTransform({UiMargins::None, UiButtonInput::Padding, -UiButtonInput::Padding});
	textTitle.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	textTitle.SetFontSize(UiButtonInput::FontSize);
	textTitle.SetTextColor(UiButtonInput::TitleColor);
	AddChild(&textTitle);

	SetCursorHover(std::make_unique<Cursor>(CursorStandard::Hand));
	OnSelected().connect(this, [this](bool selected) {
		background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(),
			selected ? UiButtonInput::SelectedColor : UiButtonInput::PrimaryColor, UiButtonInput::SlideTime);
	});
}

void UiDropdownInput::UpdateObject() {
	//slider.GetTransform().SetSize({1.0f, 2.0f * static_cast<float>(options.size())});
}

void UiDropdownInput::SetValue(uint32_t value) {
	this->value = value;
	//onValue(value);
}

void UiDropdownInput::SetOptions(const std::vector<STxt> &options) {
	this->options = options;
}
}
