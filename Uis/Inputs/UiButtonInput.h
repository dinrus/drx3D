#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>

namespace drx3d {
class DRX3D_EXPORT UiButtonInput : public UiObject {
public:
	UiButtonInput();

	void UpdateObject() override;

	const STxt &GetTitle() const { return title.GetString(); }
	void SetTitle(const STxt &string) { title.SetString(string); }
	
	constexpr static Vector2i Size = {175, 28};
	constexpr static Vector2i Padding = {5, 5};
	constexpr static float FontSize = 13.0f;
	constexpr static Time SlideTime = 0.07s;

	constexpr static Color ValueColor = 0xFEFCFE;
	constexpr static Color TitleColor = 0x9C9A9C;

	constexpr static Color BackgroundColor = 0x282729;
	constexpr static Color PrimaryColor = 0x121113;
	constexpr static Color SelectedColor = 0xFEA62A;
	constexpr static Color ButtonColor = 0x3C3B3C;

private:
	Gui background;
	Text title;
};
}
