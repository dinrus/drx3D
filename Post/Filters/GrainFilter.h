#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT GrainFilter : public PostFilter {
public:
	explicit GrainFilter(const Pipeline::Stage &pipelineStage, float strength = 2.3f);

	void Render(const CommandBuffer &commandBuffer) override;

	float GetStrength() const { return strength; }
	void SetStrength(float strength) { this->strength = strength; }

private:
	PushHandler pushScene;

	float strength;
};
}
