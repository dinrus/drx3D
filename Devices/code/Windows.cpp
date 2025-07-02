#include <drx3D/Devices/Windows.h>

#include <algorithm>
#include <X/glfw/glfw3.h>

namespace drx3d {
void CallbackError(int32_t error, tukk description) {
	Windows::CheckGlfw(error);
	Log::Error("GLFW - ошибка: ", description, ", ", error, '\n');
}

void CallbackMonitor(GLFWmonitor *glfwMonitor, int32_t event) {
	auto &monitors = Windows::Get()->monitors;

	if (event == GLFW_CONNECTED) {
		auto monitor = monitors.emplace_back(std::make_unique<Monitor>(glfwMonitor)).get();
		Windows::Get()->onMonitorConnect(monitor, true);
	} else if (event == GLFW_DISCONNECTED) {
		for (const auto &monitor : monitors) {
			if (monitor->GetMonitor() == glfwMonitor) {
				Windows::Get()->onMonitorConnect(monitor.get(), false);
			}
		}

		monitors.erase(std::remove_if(monitors.begin(), monitors.end(), [glfwMonitor](const auto &monitor) {
			return glfwMonitor == monitor->GetMonitor();
		}));
	}
}

Windows::Windows() {
	// Set the error error callback
	glfwSetErrorCallback(CallbackError);

	// Initialize the GLFW library.
	if (glfwInit() == GLFW_FALSE)
		throw drx::Exc("Неудачная инициализация GLFW");

	// Checks Vulkan support on GLFW.
	if (glfwVulkanSupported() == GLFW_FALSE)
		throw drx::Exc("GLFW не смог поддерживать Vulkan");

	// Set the monitor callback
	glfwSetMonitorCallback(CallbackMonitor);

	// The window will stay hidden until after creation.
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	// Disable context creation.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// Fixes 16 bit stencil bits in macOS.
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	// No stereo view!
	glfwWindowHint(GLFW_STEREO, GLFW_FALSE);

	// Get connected monitors.
	int32_t monitorCount;
	auto monitors = glfwGetMonitors(&monitorCount);

	for (uint32_t i = 0; i < static_cast<uint32_t>(monitorCount); i++)
		this->monitors.emplace_back(std::make_unique<Monitor>(monitors[i]));
}

Windows::~Windows() {
	glfwTerminate();
}

void Windows::Update() {
	glfwPollEvents();
	for (auto &window : windows)
		window->Update();
}

Window *Windows::AddWindow() {
	auto window = windows.emplace_back(std::make_unique<Window>(windows.size())).get();
	onAddWindow(window, true);
	return window;
}

const Window *Windows::GetWindow(WindowId id) const {
	if (id >= windows.size())
		return nullptr;
	return windows.at(id).get();
}

Window *Windows::GetWindow(WindowId id) {
	if (id >= windows.size())
		return nullptr;
	return windows.at(id).get();
}

const Monitor *Windows::GetPrimaryMonitor() const {
	for (const auto &monitor : monitors) {
		if (monitor->IsPrimary())
			return monitor.get();
	}
	return nullptr;
}

STxt Windows::StringifyResultGlfw(int32_t result) {
	switch (result) {
	case GLFW_TRUE:
		return "Успешно";
	case GLFW_NOT_INITIALIZED:
		return "GLFW не был инициализован";
	case GLFW_NO_CURRENT_CONTEXT:
		return "Для этого потока отсутствует текущий контекст";
	case GLFW_INVALID_ENUM:
		return "Один из аргументов функции имеет неполноценное значение перечня";
	case GLFW_INVALID_VALUE:
		return "Один из аргументов функции имеет неполноценное значение";
	case GLFW_OUT_OF_MEMORY:
		return "Неудачное размещение в память";
	case GLFW_API_UNAVAILABLE:
		return "GLFW не нашёд поддержки в системе для требуемого API";
	case GLFW_VERSION_UNAVAILABLE:
		return "Недоступна запрашиваемая версия OpenGL или OpenGL ES";
	case GLFW_PLATFORM_ERROR:
		return "A platform-specific error occurred that does not match any of the more specific categories";
	case GLFW_FORMAT_UNAVAILABLE:
		return "The requested format is not supported or available";
	case GLFW_NO_WINDOW_CONTEXT:
		return "The specified window does not have an OpenGL or OpenGL ES context";
	default:
		return "ОШИБКА: НЕИЗВЕСТНАЯ ОШИБКА GLFW";
	}
}

void Windows::CheckGlfw(int32_t result) {
	if (result) return;

	auto failure = StringifyResultGlfw(result);
	Log::Error("Ошибка GLFW: ", failure, ", ", result, '\n');
	throw drx::Exc("Ошибка GLFW : " + failure);
}

std::pair<tukk* , uint32_t> Windows::GetInstanceExtensions() const {
	uint32_t glfwExtensionCount;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	return std::make_pair(glfwExtensions, glfwExtensionCount);
}
}
