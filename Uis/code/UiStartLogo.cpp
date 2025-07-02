#include <drx3D/Uis/UiStartLogo.h>

#include <drx3D/Uis/Constraints/PixelConstraint.h>
#include <drx3D/Uis/Constraints/RatioConstraint.h>
#include <drx3D/Uis/Drivers/SlideDriver.h>
#include <drx3D/Timers/Timers.h>

namespace drx3d {
UiStartLogo::UiStartLogo() {
	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/Black.png"));
	AddChild(&background);

	//logodrx3D.SetTransform({{300, 300}, UiAnchor::Centre, {0, -100}});
	logodrx3D.GetConstraints().SetWidth<PixelConstraint>(300)
		.SetHeight<RatioConstraint>(1.0f)
		.SetX<PixelConstraint>(0, UiAnchor::Centre)
		.SetY<PixelConstraint>(-100, UiAnchor::Centre);
	logodrx3D.SetImage(Image2d::Create("Logos/drx3D_01.png"));
	AddChild(&logodrx3D);

	//textCopyright.SetTransform({{460, 64}, UiAnchor::Centre, {0, 128}});
	textCopyright.GetConstraints().SetWidth<PixelConstraint>(460)
		.SetHeight<PixelConstraint>(64)
		.SetX<PixelConstraint>(0, UiAnchor::Centre)
		.SetY<PixelConstraint>(128, UiAnchor::Centre);
	textCopyright.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	textCopyright.SetJustify(Text::Justify::Centre);
	textCopyright.SetTextColor(Color::White);
	textCopyright.SetString("Copyright (C) 2019, Equilibrium Games - All Rights Reserved.");
	AddChild(&textCopyright);

	Timers::Get()->Once(this, [this]() {
		SetAlphaDriver<SlideDriver>(1.0f, 0.0f, 1.4s);
	}, StartDelay);
}

void UiStartLogo::UpdateObject() {
	if (GetScreenAlpha() <= 0.0f && !finished) {
		finished = true;
		onFinished();
	}
}
}
