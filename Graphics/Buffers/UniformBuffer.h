#pragma once

#include <drx3D/Graphics/Descriptors/Descriptor.h>
#include <drx3D/Graphics/Buffers/Buffer.h>

namespace drx3d {
class DRX3D_EXPORT UniformBuffer : public Descriptor, public Buffer {
public:
	explicit UniformBuffer(VkDeviceSize size, ukk data = nullptr);

	void Update(ukk newData);

	WriteDescriptorSet GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType, const std::optional<OffsetSize> &offsetSize) const override;

	static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage, uint32_t count);
};
}
