#pragma once

#include <drx3D/Graphics/Subrender.h>
#include <drx3D/Graphics/Buffers/UniformHandler.h>
#include <drx3D/Graphics/Pipelines/PipelineGraphics.h>

namespace drx3d {
class DRX3D_EXPORT MeshesSubrender : public Subrender {
public:
	enum class Sort {
		None, Front, Back
	};

	explicit MeshesSubrender(const Pipeline::Stage &pipelineStage, Sort sort = Sort::None);

	void Render(const CommandBuffer &commandBuffer) override;

private:
	Sort sort;
	UniformHandler uniformScene;
};
}
