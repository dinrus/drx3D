#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT ToneFilter : public PostFilter {
public:
	explicit ToneFilter(const Pipeline::Stage &pipelineStage);

	void Render(const CommandBuffer &commandBuffer) override;
};
}
