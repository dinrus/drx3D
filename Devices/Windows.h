#pragma once

#include <drx3D/Engine/Engine.h>
#include <drx3D/Devices/Window.h>

namespace drx3d {
/**
 * @brief Module used for managing a window.
 */
class DRX3D_EXPORT Windows : public Module::Registry<Windows> {
	inline static const bool Registered = Register(Stage::Pre);
public:
	Windows();
	~Windows();

	void Update() override;

	Window *AddWindow();
	const Window *GetWindow(WindowId id) const;
	Window *GetWindow(WindowId id);

	const std::vector<std::unique_ptr<Monitor>> &GetMonitors() const { return monitors; };

	const Monitor *GetPrimaryMonitor() const;

	/**
	 * Called when a window has been added or closed.
	 * @return The rocket::signal.
	 */
	rocket::signal<void(Window *, bool)> &OnAddWindow() { return onAddWindow; }

	/**
	 * Called when a monitor has been connected or disconnected.
	 * @return The rocket::signal.
	 */
	rocket::signal<void(Monitor *, bool)> &OnMonitorConnect() { return onMonitorConnect; }

	DRX3D_NO_EXPORT static STxt StringifyResultGlfw(int32_t result);
	DRX3D_NO_EXPORT static void CheckGlfw(int32_t result);

	std::pair<tukk* , uint32_t> GetInstanceExtensions() const;

private:
	friend void CallbackError(int32_t error, tukk description);
	friend void CallbackMonitor(GLFWmonitor *glfwMonitor, int32_t event);

	std::vector<std::unique_ptr<Window>> windows;

	std::vector<std::unique_ptr<Monitor>> monitors;

	rocket::signal<void(Window *, bool)> onAddWindow;
	rocket::signal<void(Monitor *, bool)> onMonitorConnect;
};
}
