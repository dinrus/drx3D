#pragma once

#include <iomanip>

#include <drx3D/Engine/Log.h>
#include <drx3D/Common/ConstExpr.h>
#include <drx3D/Graphics/Descriptors/DescriptorSet.h>
#include <drx3D/Graphics/Buffers/UniformHandler.h>
#include <drx3D/Graphics/Buffers/StorageHandler.h>
#include <drx3D/Graphics/Buffers/PushHandler.h>
#include <drx3D/Graphics/Pipelines/Shader.h>

namespace drx3d {
/**
 * @brief Class that handles a descriptor set.
 */
class DRX3D_EXPORT DescriptorsHandler {
public:
	DescriptorsHandler() = default;
	explicit DescriptorsHandler(const Pipeline &pipeline);

	template<typename T>
	void Push(const STxt &descriptorName, const T &descriptor, const std::optional<OffsetSize> &offsetSize = std::nullopt) {
		if (!shader)
			return;

		// Finds the local value given to the descriptor name.
		auto it = descriptors.find(descriptorName);

		if (it != descriptors.end()) {
			// If the descriptor and size have not changed then the write is not modified.
			if (it->second.descriptor == to_address(descriptor) && it->second.offsetSize == offsetSize) {
				return;
			}

			descriptors.erase(it);
		}

		// Only non-null descriptors can be mapped.
		if (!to_address(descriptor)) {
			return;
		}

		// When adding the descriptor find the location in the shader.
		auto location = shader->GetDescriptorLocation(descriptorName);

		if (!location) {
#ifdef DRX3D_DEBUG
			if (shader->ReportedNotFound(descriptorName, true)) {
				Log::Error("Could not find descriptor in shader ", shader->GetName(), " of name ", std::quoted(descriptorName), '\n');
			}
#endif

			return;
		}

		auto descriptorType = shader->GetDescriptorType(*location);

		if (!descriptorType) {
#ifdef DRX3D_DEBUG
			if (shader->ReportedNotFound(descriptorName, true)) {
				Log::Error("Could not find descriptor in shader ", shader->GetName(), " of name ", std::quoted(descriptorName), " at location ", *location, '\n');
			}
#endif
			return;
		}

		// Adds the new descriptor value.
		auto writeDescriptor = to_address(descriptor)->GetWriteDescriptor(*location, *descriptorType, offsetSize);
		descriptors.emplace(descriptorName, DescriptorValue{to_address(descriptor), std::move(writeDescriptor), offsetSize, *location});
		changed = true;
	}

	template<typename T>
	void Push(const STxt &descriptorName, const T &descriptor, WriteDescriptorSet writeDescriptorSet) {
		if (!shader)
			return;

		if (auto it = descriptors.find(descriptorName); it != descriptors.end()) {
			descriptors.erase(it);
		}

		auto location = shader->GetDescriptorLocation(descriptorName);
		//auto descriptorType = shader->GetDescriptorType(*location);

		descriptors.emplace(descriptorName, DescriptorValue{to_address(descriptor), std::move(writeDescriptorSet), std::nullopt, *location});
		changed = true;
	}

	void Push(const STxt &descriptorName, UniformHandler &uniformHandler, const std::optional<OffsetSize> &offsetSize = std::nullopt);
	void Push(const STxt &descriptorName, StorageHandler &storageHandler, const std::optional<OffsetSize> &offsetSize = std::nullopt);
	void Push(const STxt &descriptorName, PushHandler &pushHandler, const std::optional<OffsetSize> &offsetSize = std::nullopt);

	bool Update(const Pipeline &pipeline);

	void BindDescriptor(const CommandBuffer &commandBuffer, const Pipeline &pipeline);

	const DescriptorSet *GetDescriptorSet() const { return descriptorSet.get(); }

private:
	class DescriptorValue {
	public:
		const Descriptor *descriptor;
		WriteDescriptorSet writeDescriptor;
		std::optional<OffsetSize> offsetSize;
		uint32_t location;
	};

	const Shader *shader = nullptr;
	bool pushDescriptors = false;
	std::unique_ptr<DescriptorSet> descriptorSet;

	std::map<STxt, DescriptorValue> descriptors;
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	bool changed = false;
};
}
