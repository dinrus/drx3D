#include <drx3D/Post/Filters/VignetteFilter.h>

namespace drx3d {
VignetteFilter::VignetteFilter(const Pipeline::Stage &pipelineStage, float innerRadius, float outerRadius, float opacity) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Vignette.frag"}),
	innerRadius(innerRadius),
	outerRadius(outerRadius),
	opacity(opacity) {
}

void VignetteFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates uniforms.
	pushScene.Push("innerRadius", innerRadius);
	pushScene.Push("outerRadius", outerRadius);
	pushScene.Push("opacity", opacity);

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
