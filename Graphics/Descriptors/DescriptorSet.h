#pragma once

#include <drx3D/Graphics/Commands/CommandBuffer.h>
#include <drx3D/Graphics/Pipelines/Pipeline.h>

namespace drx3d {
class DRX3D_EXPORT DescriptorSet {
public:
	explicit DescriptorSet(const Pipeline &pipeline);
	~DescriptorSet();

	static void Update(const std::vector<VkWriteDescriptorSet> &descriptorWrites);

	void BindDescriptor(const CommandBuffer &commandBuffer) const;

	const VkDescriptorSet &GetDescriptorSet() const { return descriptorSet; }

private:
	VkPipelineLayout pipelineLayout;
	VkPipelineBindPoint pipelineBindPoint;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};
}
