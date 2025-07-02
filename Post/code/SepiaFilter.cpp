#include <drx3D/Post/Filters/SepiaFilter.h>

namespace drx3d {
SepiaFilter::SepiaFilter(const Pipeline::Stage &pipelineStage) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Sepia.frag"}) {
}

void SepiaFilter::Render(const CommandBuffer &commandBuffer) {
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
