#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT SepiaFilter : public PostFilter {
public:
	explicit SepiaFilter(const Pipeline::Stage &pipelineStage);

	void Render(const CommandBuffer &commandBuffer) override;
};
}
