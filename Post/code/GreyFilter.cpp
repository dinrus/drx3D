#include <drx3D/Post/Filters/GreyFilter.h>

namespace drx3d {
GreyFilter::GreyFilter(const Pipeline::Stage &pipelineStage) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Grey.frag"}) {
}

void GreyFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates descriptors.
	PushConditional("writeColor", "samplerColor", "resolved", "diffuse");

	if (!descriptorSet.Update(pipeline))
		return;

	// Draws the object.
	pipeline.BindPipeline(commandBuffer);

	descriptorSet.BindDescriptor(commandBuffer, pipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}
}
