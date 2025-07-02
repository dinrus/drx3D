#pragma once

#include <thread>
#include <drx3D/Devices/volk.h>

#include <drx3D/Export.h>

namespace drx3d {
/**
 * @brief Class that represents a command pool.
 */
class DRX3D_EXPORT CommandPool {
public:
	explicit CommandPool(const std::thread::id &threadId = std::this_thread::get_id());

	~CommandPool();

	operator const VkCommandPool &() const { return commandPool; }

	const VkCommandPool &GetCommandPool() const { return commandPool; }
	const std::thread::id &GetThreadId() const { return threadId; }

private:
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::thread::id threadId;
};
}
