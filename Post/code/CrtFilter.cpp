#include <drx3D/Post/Filters/CrtFilter.h>

#include <drx3D/Graphics/Graphics.h>

namespace drx3d {
CrtFilter::CrtFilter(const Pipeline::Stage &pipelineStage, const Color &screenColor, float curveAmountX, float curveAmountY, float scanLineSize,
	float scanIntensity) :
	PostFilter(pipelineStage, {"Shaders/Post/Default.vert", "Shaders/Post/Crt.frag"}),
	screenColor(screenColor),
	curveAmountX(curveAmountX),
	curveAmountY(curveAmountY),
	scanLineSize(scanLineSize),
	scanIntensity(scanIntensity) {
}

void CrtFilter::Render(const CommandBuffer &commandBuffer) {
	// Updates uniforms.
	pushScene.Push("screenColor", screenColor);
	pushScene.Push("curveAmountX", curveAmountX * pipeline.GetRenderArea().GetAspectRatio());
	pushScene.Push("curveAmountY", curveAmountY);
	pushScene.Push("scanLineSize", scanLineSize);
	pushScene.Push("scanIntensity", scanIntensity);
	pushScene.Push("moveTime", Time::Now() / 100.0f);

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
