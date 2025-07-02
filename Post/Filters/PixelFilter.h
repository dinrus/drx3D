#pragma once

#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT PixelFilter : public PostFilter {
public:
	explicit PixelFilter(const Pipeline::Stage &pipelineStage, float pixelSize = 2.0f);

	void Render(const CommandBuffer &commandBuffer) override;

	float GetPixelSize() const { return pixelSize; }
	void SetPixelSize(float pixelSize) { this->pixelSize = pixelSize; }

private:
	PushHandler pushScene;

	float pixelSize;
};
}
