#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>

namespace drx3d {
class DRX3D_EXPORT UiStartLogo : public UiObject {
public:
	explicit UiStartLogo();

	void UpdateObject() override;

	bool IsFinished() const { return finished; }
	rocket::signal<void()> &OnFinished() { return onFinished; }

#ifdef DRX3D_DEBUG
	constexpr static Time StartDelay = 1s;
#else
	constexpr static Time StartDelay = 3s;
#endif
	
private:
	Gui background;
	Gui logodrx3D;
	Text textCopyright;

	bool finished = false;

	rocket::signal<void()> onFinished;
};
}
