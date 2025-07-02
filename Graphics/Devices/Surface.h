#pragma once

#include <drx3D/Devices/volk.h>

#include <drx3D/Export.h>

namespace drx3d {
class Instance;
class LogicalDevice;
class PhysicalDevice;
class Window;

class DRX3D_EXPORT Surface {
	friend class Graphics;
public:
	Surface(const Instance &instance, const PhysicalDevice &physicalDevice, const LogicalDevice &logicalDevice, const Window &window);
	~Surface();

	operator const VkSurfaceKHR &() const { return surface; }

	const VkSurfaceKHR &GetSurface() const { return surface; }
	const VkSurfaceCapabilitiesKHR &GetCapabilities() const { return capabilities; }
	const VkSurfaceFormatKHR &GetFormat() const { return format; }

private:
	const Instance &instance;
	const PhysicalDevice &physicalDevice;
	const LogicalDevice &logicalDevice;
	const Window &window;

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSurfaceCapabilitiesKHR capabilities = {};
	VkSurfaceFormatKHR format = {};
};
}
