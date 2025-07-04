#include <drx3D/Post/Filters/GrainFilter.h>

namespace drx3d {
GrainFilter::GrainFilter(const Pipeline::Stage &pipelineStage, float strength) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Grain.frag"}),
	strength(strength) {
}

void GrainFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates uniforms.
	pushScene.Push("strength", strength);

	// Updates descriptors.
	descriptorSet.Push("PushScene", pushScene);
	PushConditional("writeColor", "samplerColor", "resolved", "diffuse");

	if (!descriptorSet.Update(pipeline))
		return;

	// Binds the pipeline.
	pipeline.BindPipeline(commandBuffer);

	// Draws the object.
	descriptorSet.BindDescriptor(commandBuffer, pipeline);
	pushScene.BindPush(commandBuffer, pipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}
}
