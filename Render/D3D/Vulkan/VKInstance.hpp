// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>

namespace NDrxVulkan
{

class CDevice;

struct SLayerInfo
{

	VkLayerProperties                  properties;
	std::vector<VkExtensionProperties> extensions;
};

struct SInstanceInfo
{
	std::vector<SLayerInfo> instanceLayers;
	std::vector<VkExtensionProperties> implicitExtensions;
};

struct SPhysicalDeviceInfo
{
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	std::vector<SLayerInfo> devicelayers;
	std::vector<VkExtensionProperties> implicitExtensions;
	std::array<VkFormatProperties, VK_FORMAT_RANGE_SIZE> formatProperties;
};

//CInstance handles the initalization of the Vulkan API and instance, querying the hardware capabilities and creating a device and surface
class CInstance
{

public:
	CInstance();
	~CInstance();

#if defined(USE_SDL2_VIDEO)
	static bool CreateSDLWindow(tukk szTitle, u32 uWidth, u32 uHeight, bool bFullScreen, HWND * pHandle);
	static void DestroySDLWindow(HWND kHandle);
#endif //defined(USE_SDL2_VIDEO)

	bool Initialize(tukk appName, uint32_t appVersion, tukk engineName, uint32_t engineVersion);

	size_t GetPhysicalDeviceCount() const { return m_physicalDevices.size(); }

	_smart_ptr<CDevice> CreateDevice(size_t physicalDeviceIndex);

	//platform dependant function to create platform independant VkSurfaceKHR handle
	VkResult CreateSurface(const SSurfaceCreationInfo& info, VkSurfaceKHR* surface);
	void DestroySurface(VkSurfaceKHR surface);

private:
	VkResult InitializeInstanceLayerInfos();
	VkResult InitializeInstanceExtensions(tukk layerName, std::vector<VkExtensionProperties>& extensions);
	VkResult InitializeInstance(tukk appName, uint32_t appVersion, tukk engineName, uint32_t engineVersion);
	VkResult InitializeDebugLayerCallback();

	VkResult InitializePhysicalDeviceInfos();
	VkResult InitializePhysicalDeviceLayerInfos(SPhysicalDeviceInfo& info);
	VkResult InitializePhysicalDeviceExtensions(VkPhysicalDevice& device, tukk layerName, std::vector<VkExtensionProperties>& extensions);

	//Gather* functions fill in the names of the layers & extensions we want to enable. For now, the only debug layer queried is VK_LAYER_LUNARG_standard_validation, could potentially try to
	//query specific ones if the standard validation layer not present.
	void GatherInstanceLayersToEnable();
	void GatherInstanceExtensionsToEnable();
	void GatherPhysicalDeviceLayersToEnable();
	void GatherPhysicalDeviceExtensionsToEnable();

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugLayerCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj, size_t location, int32_t code, tukk layerPrefix, tukk msg, uk userData);

private:
	struct SExtensionInfo
	{
		SExtensionInfo(tukk extensionName, bool isRequired)
			: name(extensionName)
			, bRequired(isRequired)
		{}

		tukk name;
		bool        bRequired;
	};

	SInstanceInfo m_instanceInfo;
	CHostAllocator m_Allocator;
	std::vector<SPhysicalDeviceInfo> m_physicalDevices;

	std::vector<tukk >    m_enabledInstanceLayers;
	std::vector<tukk >    m_enabledInstanceExtensions;
	std::vector<tukk >    m_enabledPhysicalDeviceLayers;
	std::vector<SExtensionInfo> m_enabledPhysicalDeviceExtensions;

	VkInstance                  m_instanceHandle;
	VkDebugReportCallbackEXT    m_debugLayerCallbackHandle;

};

}
