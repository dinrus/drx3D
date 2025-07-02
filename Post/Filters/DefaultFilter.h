#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT DefaultFilter : public PostFilter {
public:
	explicit DefaultFilter(const Pipeline::Stage &pipelineStage, bool lastFilter = false);

	void Render(const CommandBuffer &commandBuffer) override;

private:
	bool lastFilter;
};
}
