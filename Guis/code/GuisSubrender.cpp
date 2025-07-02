#include <drx3D/Guis/GuisSubrender.h>

#include <drx3D/Geometry/Vertex2d.h>
#include <drx3D/Uis/Uis.h>
#include <drx3D/Guis/Gui.h>

namespace drx3d {
GuisSubrender::GuisSubrender(const Pipeline::Stage &pipelineStage) :
	Subrender(pipelineStage),
	pipeline(pipelineStage, {"Shaders/Guis/Gui.vert", "Shaders/Guis/Gui.frag"}, {Vertex2d::GetVertexInput()}) {
}

void GuisSubrender::Render(const CommandBuffer &commandBuffer) {
	pipeline.BindPipeline(commandBuffer);

	for (const auto &screenObject : Uis::Get()->GetObjects()) {
		if (!screenObject->IsEnabled())
			continue;

		if (auto object = dynamic_cast<Gui *>(screenObject))
			object->CmdRender(commandBuffer, pipeline);
	}
}
}
