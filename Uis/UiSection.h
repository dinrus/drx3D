#pragma once

#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>

namespace drx3d {
class DRX3D_EXPORT UiSection : public UiObject {
public:
	UiSection();

	void UpdateObject() override;

	void AddChild(UiObject *child) override;

	const STxt &GetTitle() const { return title.GetString(); }
	void SetTitle(const STxt &string) { title.SetString(string); }

	/**
	 * Called when this section has been collapsed or uncollapsed.
	 * @return The delegate.
	 */
	rocket::signal<void(UiSection *, bool)> &OnCollapsed() { return onCollapsed; }

private:
	Gui icon;
	Text title;
	UiObject content;

	bool collapsed = false;

	rocket::signal<void(UiSection *, bool)> onCollapsed;
};
}
