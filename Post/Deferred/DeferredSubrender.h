#pragma once

#include <drx3D/Common/Future.h>
#include <drx3D/Lights/Fog.h>
#include <drx3D/Maths/Vector3.h>
#include <drx3D/Graphics/Subrender.h>
#include <drx3D/Graphics/Descriptors/DescriptorsHandler.h>
#include <drx3D/Graphics/Buffers/UniformHandler.h>
#include <drx3D/Graphics/Images/ImageCube.h>
#include <drx3D/Graphics/Pipelines/PipelineGraphics.h>

namespace drx3d {
class DRX3D_EXPORT DeferredSubrender : public Subrender {
public:
	explicit DeferredSubrender(const Pipeline::Stage &pipelineStage);

	void Render(const CommandBuffer &commandBuffer) override;

	static std::unique_ptr<Image2d> ComputeBRDF(uint32_t size);
	static std::unique_ptr<ImageCube> ComputeIrradiance(const std::shared_ptr<ImageCube> &source, uint32_t size);
	static std::unique_ptr<ImageCube> ComputePrefiltered(const std::shared_ptr<ImageCube> &source, uint32_t size);

	const Fog &GetFog() const { return fog; }
	void SetFog(const Fog &fog) { this->fog = fog; }

private:
	class DeferredLight {
	public:
		Color color;
		Vector3f position;
		float radius = 0.0f;
	};

	PipelineGraphics pipeline;

	DescriptorsHandler descriptorSet;
	UniformHandler uniformScene;
	StorageHandler storageLights;

	Future<std::unique_ptr<Image2d>> brdf;

	std::shared_ptr<ImageCube> skybox;

	Future<std::unique_ptr<ImageCube>> irradiance;
	Future<std::unique_ptr<ImageCube>> prefiltered;

	Fog fog;
};
}
