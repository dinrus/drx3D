#pragma once

#include <cstdint>
#include <optional>
#include <memory>
#include <drx3D/Devices/volk.h>

#include <drx3D/Export.h>

namespace drx3d {
class DRX3D_EXPORT OffsetSize {
public:
	OffsetSize(uint32_t offset, uint32_t size) :
		offset(offset),
		size(size) {
	}

	uint32_t GetOffset() const { return offset; }
	uint32_t GetSize() const { return size; }

	bool operator==(const OffsetSize &rhs) const {
		return offset == rhs.offset && size == rhs.size;
	}

	bool operator!=(const OffsetSize &rhs) const {
		return !operator==(rhs);
	}

private:
	uint32_t offset;
	uint32_t size;
};

class DRX3D_EXPORT WriteDescriptorSet {
public:
	WriteDescriptorSet(const VkWriteDescriptorSet &writeDescriptorSet, const VkDescriptorImageInfo &imageInfo) :
		writeDescriptorSet(writeDescriptorSet),
		imageInfo(std::make_unique<VkDescriptorImageInfo>(imageInfo)) {
		this->writeDescriptorSet.pImageInfo = this->imageInfo.get();
	}

	WriteDescriptorSet(const VkWriteDescriptorSet &writeDescriptorSet, const VkDescriptorBufferInfo &bufferInfo) :
		writeDescriptorSet(writeDescriptorSet),
		bufferInfo(std::make_unique<VkDescriptorBufferInfo>(bufferInfo)) {
		this->writeDescriptorSet.pBufferInfo = this->bufferInfo.get();
	}

	const VkWriteDescriptorSet &GetWriteDescriptorSet() const { return writeDescriptorSet; }

private:
	VkWriteDescriptorSet writeDescriptorSet;
	std::unique_ptr<VkDescriptorImageInfo> imageInfo;
	std::unique_ptr<VkDescriptorBufferInfo> bufferInfo;
};

class DRX3D_EXPORT Descriptor {
public:
	Descriptor() = default;
	virtual ~Descriptor() = default;

	virtual WriteDescriptorSet GetWriteDescriptor(uint32_t binding, VkDescriptorType descriptorType, const std::optional<OffsetSize> &offsetSize) const = 0;
};
}
