#include <drx3D/Post/Filters/PixelFilter.h>

namespace drx3d {
PixelFilter::PixelFilter(const Pipeline::Stage &pipelineStage, float pixelSize) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Pixel.frag"}),
	pixelSize(pixelSize) {
}

void PixelFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates uniforms.
	pushScene.Push("pixelSize", pixelSize);

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
