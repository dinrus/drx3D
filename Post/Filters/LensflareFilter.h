#pragma once

#include <drx3D/Maths/Vector3.h>
#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT LensflareFilter : public PostFilter {
public:
	explicit LensflareFilter(const Pipeline::Stage &pipelineStage);

	void Render(const CommandBuffer &commandBuffer) override;

	const Vector3f &GetSunPosition() const { return sunPosition; }
	void SetSunPosition(const Vector3f &sunPosition);

	float GetSunHeight() const { return sunHeight; }
	void SetSunHeight(float sunHeight) { this->sunHeight = sunHeight; }

private:
	PushHandler pushScene;

	Vector3f sunPosition;
	float sunHeight = 0.0f;
};
}
