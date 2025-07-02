#pragma once

#include <drx3D/Engine/Engine.h>
#include <drx3D/Devices/Windows.h>
#include <drx3D/Uis/UiObject.h>

namespace drx3d {
/**
 * @brief Module used for managing gui textures in a container.
 */
class DRX3D_EXPORT Uis : public Module::Registry<Uis> {
	inline static const bool Registered = Register(Stage::Normal);
public:
	Uis();

	void Update() override;

	bool IsDown(MouseButton button);
	bool WasDown(MouseButton button);
	void CancelWasEvent(MouseButton button);

	/**
	 * Gets the screen canvas.
	 * @return The screen canvas.
	 */
	UiObject &GetCanvas() { return canvas; }

	/**
	 * The rendering objects from the canvas.
	 * @return The objects.
	 */
	const std::vector<UiObject *> &GetObjects() const { return objects; };
private:
	class SelectorMouse {
	public:
		bool isDown;
		bool wasDown;
	};

	std::map<MouseButton, SelectorMouse> selectors;
	UiObject canvas;
	UiObject *cursorSelect = nullptr;
	std::vector<UiObject *> objects;
};
}
