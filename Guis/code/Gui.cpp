#include <drx3D/Guis/Gui.h>

#include <drx3D/Graphics/Graphics.h>
#include <drx3D/Uis/Drivers/ConstantDriver.h>
#include <drx3D/Geometry/Vertex2d.h>

namespace drx3d {
static const std::vector<Vertex2d> VERTICES = {
	{{0.0f, 0.0f}, {0.0f, 0.0f}},
	{{1.0f, 0.0f}, {1.0f, 0.0f}},
	{{1.0f, 1.0f}, {1.0f, 1.0f}},
	{{0.0f, 1.0f}, {0.0f, 1.0f}}
};
static const std::vector<uint32_t> INDICES = {
	0, 1, 2,
	2, 3, 0
};

Gui::Gui() :
	model(std::make_unique<Model>(VERTICES, INDICES)),
	colorDriver(std::make_unique<ConstantDriver<Color>>(Color::White)) {
}

void Gui::UpdateObject() {
	auto numberOfRows = image ? this->numberOfRows : 1;
	auto column = selectedRow % numberOfRows;
	auto row = selectedRow / numberOfRows;
	atlasOffset = Vector2f(static_cast<float>(column), static_cast<float>(row)) / static_cast<float>(numberOfRows);

	colorDriver->Update(Engine::Get()->GetDelta());

	// Updates uniforms.
	uniformObject.Push("modelView", GetModelView());
	uniformObject.Push("alpha", GetScreenAlpha());

	uniformObject.Push("aspectRatio", static_cast<float>(GetScreenSize().x) / static_cast<float>(GetScreenSize().y));

	uniformObject.Push("colorOffset", colorDriver->Get());
	uniformObject.Push("atlasOffset", atlasOffset);
	uniformObject.Push("atlasScale", atlasScale);
	uniformObject.Push("atlasRows", static_cast<float>(numberOfRows));
	uniformObject.Push("ninePatches", ninePatches);
}

bool Gui::CmdRender(const CommandBuffer &commandBuffer, const PipelineGraphics &pipeline) {
	// Gets if this should be rendered.
	if (!image || !IsEnabled())
		return false;

	// Updates descriptors.
	descriptorSet.Push("UniformObject", uniformObject);
	descriptorSet.Push("samplerColor", image);

	if (!descriptorSet.Update(pipeline))
		return false;

	auto scissor = GetScissor();
	VkRect2D scissorRect = {};
	scissorRect.offset.x = scissor ? static_cast<int32_t>(scissor->x) : 0;
	scissorRect.offset.y = scissor ? static_cast<int32_t>(scissor->y) : 0;
	scissorRect.extent.width = scissor ? static_cast<int32_t>(scissor->z) : Windows::Get()->GetWindow(0)->GetSize().x;
	scissorRect.extent.height = scissor ? static_cast<int32_t>(scissor->w) : Windows::Get()->GetWindow(0)->GetSize().y;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

	// Draws the object.
	descriptorSet.BindDescriptor(commandBuffer, pipeline);
	return model->CmdRender(commandBuffer);
}
}
