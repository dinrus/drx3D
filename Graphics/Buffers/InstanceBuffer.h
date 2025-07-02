#pragma once

#include <drx3D/Graphics/Buffers/Buffer.h>

namespace drx3d {
class DRX3D_EXPORT InstanceBuffer : public Buffer {
public:
	explicit InstanceBuffer(VkDeviceSize size);

	void Update(const CommandBuffer &commandBuffer, ukk newData);
};
}
