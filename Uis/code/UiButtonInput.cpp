#include <drx3D/Uis/Inputs/UiButtonInput.h>

#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiButtonInput::UiButtonInput() {
	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/Button_Filled.png"));
	background.SetNinePatches({0.125f, 0.125f, 0.875f, 0.875f});
	background.SetColorDriver<ConstantDriver>(ButtonColor);
	AddChild(&background);

	//title.SetTransform({UiMargins::None, Padding, -Padding});
	title.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	title.SetFontSize(FontSize);
	title.SetTextColor(ValueColor);
	AddChild(&title);
	
	SetCursorHover(std::make_unique<Cursor>(CursorStandard::Hand));
	OnSelected().connect(this, [this](bool selected) {
		background.SetColorDriver<SlideDriver>(background.GetColorDriver()->Get(), selected ? SelectedColor : ButtonColor, SlideTime);
	});
}

void UiButtonInput::UpdateObject() {
}
}
