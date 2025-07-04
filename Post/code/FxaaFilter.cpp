#include <drx3D/Post/Filters/FxaaFilter.h>

namespace drx3d {
FxaaFilter::FxaaFilter(const Pipeline::Stage &pipelineStage, float spanMax) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Fxaa.frag"}),
	spanMax(spanMax) {
}

void FxaaFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates uniforms.
	pushScene.Push("spanMax", spanMax);

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
