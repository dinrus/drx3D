#include <drx3D/Post/Filters/DarkenFilter.h>

namespace drx3d {
DarkenFilter::DarkenFilter(const Pipeline::Stage &pipelineStage, float factor) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Darken.frag"}),
	factor(factor) {
}

void DarkenFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates uniforms.
	pushScene.Push("factor", factor);

	// Updates descriptors.
	descriptorSet.Push("PushScene", pushScene);
	PushConditional("writeColor", "samplerColor", "resolved", "diffuse");

	if (!descriptorSet.Update(pipeline))
		return;

	// Draws the object.
	pipeline.BindPipeline(commandBuffer);

	descriptorSet.BindDescriptor(commandBuffer, pipeline);
	pushScene.BindPush(commandBuffer, pipeline);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}
}
