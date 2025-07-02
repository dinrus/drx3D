#include <drx3D/Post/Filters/BlitFilter.h>

namespace drx3d {
BlitFilter::BlitFilter(const Pipeline::Stage &pipelineStage) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Blit.frag"}) {
}

void BlitFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates descriptors.
	descriptorSet.Push("samplerColor", Graphics::Get()->GetAttachment("swapchain"));

	if (!descriptorSet.Update(pipeline))
		return;

	// Draws the object.
	pipeline.BindPipeline(commandBuffer);

	descriptorSet.BindDescriptor(commandBuffer, pipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}
}
