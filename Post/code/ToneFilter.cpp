#include <drx3D/Post/Filters/ToneFilter.h>

namespace drx3d {
ToneFilter::ToneFilter(const Pipeline::Stage &pipelineStage) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Tone.frag"}) {
}

void ToneFilter::Render(const CommandBuffer &commandBuffer) {
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
