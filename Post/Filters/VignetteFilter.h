#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT VignetteFilter : public PostFilter {
public:
	explicit VignetteFilter(const Pipeline::Stage &pipelineStage, float innerRadius = 0.15f, float outerRadius = 1.35f, float opacity = 0.85f);

	void Render(const CommandBuffer &commandBuffer) override;

	float GetInnerRadius() const { return innerRadius; }
	void SetInnerRadius(float innerRadius) { this->innerRadius = innerRadius; }

	float GetOuterRadius() const { return outerRadius; }
	void SetOuterRadius(float outerRadius) { this->outerRadius = outerRadius; }

	float GetOpacity() const { return opacity; }
	void SetOpacity(float opacity) { this->opacity = opacity; }

private:
	PushHandler pushScene;

	float innerRadius, outerRadius;
	float opacity;
};
}
