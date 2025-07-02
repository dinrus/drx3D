#pragma once

#include <drx3D/Common/Future.h>
#include <drx3D/Post/PostFilter.h>
#include <drx3D/Maths/Vector3.h>

namespace drx3d {
class DRX3D_EXPORT SsaoFilter : public PostFilter {
public:
	explicit SsaoFilter(const Pipeline::Stage &pipelineStage);

	void Render(const CommandBuffer &commandBuffer) override;

private:
	std::vector<Shader::Define> GetDefines() const;

	static std::shared_ptr<Image2d> ComputeNoise(uint32_t size);

	UniformHandler uniformScene;

	Future<std::shared_ptr<Image2d>> noise;
	std::vector<Vector3f> kernel;
};
}
