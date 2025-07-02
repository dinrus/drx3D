#pragma once

#include <drx3D/Graphics/Subrender.h>
#include <drx3D/Graphics/Pipelines/PipelineGraphics.h>

namespace drx3d {
class DRX3D_EXPORT GuisSubrender : public Subrender {
public:
	explicit GuisSubrender(const Pipeline::Stage &pipelineStage);

	void Render(const CommandBuffer &commandBuffer) override;

private:
	PipelineGraphics pipeline;
};
}
