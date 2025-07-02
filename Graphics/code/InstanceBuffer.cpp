#include <drx3D/Graphics/Buffers/InstanceBuffer.h>

#include <cstring>

#include <drx3D/Graphics/Graphics.h>

namespace drx3d {
InstanceBuffer::InstanceBuffer(VkDeviceSize size) :
	Buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
}

void InstanceBuffer::Update(const CommandBuffer &commandBuffer, ukk newData) {
	uk data;
	MapMemory(&data);
	std::memcpy(data, newData, static_cast<std::size_t>(size));
	UnmapMemory();
}
}
