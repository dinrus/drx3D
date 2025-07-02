#include <drx3D/Graphics/Buffers/StorageBuffer.h>

#include <cstring>

#include <drx3D/Graphics/Graphics.h>

namespace drx3d {
StorageBuffer::StorageBuffer(VkDeviceSize size, ukk data) :
	Buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data) {
}

void StorageBuffer::Update(ukk newData) {
	uk data;
	MapMemory(&data);
	std::memcpy(data, newData, static_cast<std::size_t>(size));
	UnmapMemory();
}

WriteDescriptorSet StorageBuffer::GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType, const std::optional<OffsetSize> &offsetSize) const {
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = size;

	if (offsetSize) {
		bufferInfo.offset = offsetSize->GetOffset();
		bufferInfo.range = offsetSize->GetSize();
	}

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = VK_NULL_HANDLE; // Will be set in the descriptor handler.
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.descriptorType = descriptorType;
	//descriptorWrite.pBufferInfo = &bufferInfo;
	return {descriptorWrite, bufferInfo};
}

VkDescriptorSetLayoutBinding StorageBuffer::GetDescriptorSetLayout(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stage, uint32_t count) {
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
	descriptorSetLayoutBinding.binding = binding;
	descriptorSetLayoutBinding.descriptorType = descriptorType;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = stage;
	descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	return descriptorSetLayoutBinding;
}
}
