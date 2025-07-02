#pragma once

#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>

namespace drx3d {
enum class ScrollBar {
	None = 0,
	Vertical = 1,
	Horizontal = 2,
	Both = Vertical | Horizontal
};
ENABLE_BITMASK_OPERATORS(ScrollBar)

class DRX3D_EXPORT UiScrollBar : public UiObject {
public:
	UiScrollBar();

	void UpdateObject() override;

	float GetProgress();
	void SetSize(const Vector2f &size);
	void SetType(ScrollBar type);

	constexpr static uint32_t Size = 8;

private:
	float ScrollByDelta(float delta) const;

	float ScrollByPosition(float position) const;

	Gui background;
	Gui scroll;
	uint32_t index = 0;
	bool updating = false;
	bool mouseOver = false;
};
}
