#include <drx3D/Shadows/ShadowsSubrender.h>

#include <drx3D/Geometry/Vertex3d.h>
#include <drx3D/Scenes/Scenes.h>
#include <drx3D/Shadows/ShadowRender.h>
#include <drx3D/Shadows/Shadows.h>

namespace drx3d {
ShadowsSubrender::ShadowsSubrender(const Pipeline::Stage &pipelineStage) :
	Subrender(pipelineStage),
	pipeline(pipelineStage, {"Shaders/Shadows/Shadow.vert", "Shaders/Shadows/Shadow.frag"}, {Vertex3d::GetVertexInput()}, {},
		PipelineGraphics::Mode::Polygon, PipelineGraphics::Depth::None, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT) {
}

void ShadowsSubrender::Render(const CommandBuffer &commandBuffer) {
	auto camera = Scenes::Get()->GetScene()->GetCamera();

	pipeline.BindPipeline(commandBuffer);

	auto sceneShadowRenders = Scenes::Get()->GetScene()->QueryComponents<ShadowRender>();

	for (const auto &shadowRender : sceneShadowRenders)
		shadowRender->CmdRender(commandBuffer, pipeline);
}
}
