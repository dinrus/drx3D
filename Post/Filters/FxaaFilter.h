#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT FxaaFilter : public PostFilter {
public:
	explicit FxaaFilter(const Pipeline::Stage &pipelineStage, float spanMax = 8.0f);

	void Render(const CommandBuffer &commandBuffer) override;

	float GetSpanMax() const { return spanMax; }
	void SetSpanMax(float spanMax) { this->spanMax = spanMax; }

private:
	PushHandler pushScene;

	float spanMax;
};
}
