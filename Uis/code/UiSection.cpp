#include <drx3D/Uis/UiSection.h>

#include <drx3D/Uis/Uis.h>

namespace drx3d {
UiSection::UiSection() {
	icon.SetImage(Image2d::Create("Guis/Triangle_Down.png"));
	UiObject::AddChild(&icon);

	title.SetFontType(FontType::Create("Fonts/ProximaNova-Regular.ttf"));
	title.SetTextColor(Color::White);
	UiObject::AddChild(&title);
	
	UiObject::AddChild(&content);

	OnClick().connect(this, [this](MouseButton button) {
		if (button == MouseButton::Left) {
			CancelEvent(MouseButton::Left);

			collapsed = !collapsed;

			if (collapsed) {
				icon.SetImage(Image2d::Create("Guis/Triangle_Right.png"));
			} else {
				icon.SetImage(Image2d::Create("Guis/Triangle_Down.png"));
			}

			onCollapsed(this, collapsed);
		}
	});
}

void UiSection::UpdateObject() {
	content.SetEnabled(!collapsed);
}

void UiSection::AddChild(UiObject *child) {
	content.AddChild(child);
}
}
