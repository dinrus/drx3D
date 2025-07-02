#pragma once

#include <drx3D/Materials/Material.h>
#include <drx3D/Maths/Color.h>
#include <drx3D/Maths/Vector2.h>
#include <drx3D/Graphics/Images/ImageCube.h>

namespace drx3d {
/**
 * @brief Class that represents a skybox material shader.
 */
class DRX3D_EXPORT SkyboxMaterial : public Material::Registry<SkyboxMaterial> {
	inline static const bool Registered = Register("skybox");
public:
	explicit SkyboxMaterial(std::shared_ptr<ImageCube> image = nullptr, const Color &baseColor = Color::White);

	void CreatePipeline(const Shader::VertexInput &vertexInput, bool animated) override;
	void PushUniforms(UniformHandler &uniformObject, const Transform *transform) override;
	void PushDescriptors(DescriptorsHandler &descriptorSet) override;

	const std::shared_ptr<ImageCube> &GetImage() const { return image; }
	void SetImage(const std::shared_ptr<ImageCube> &image) { this->image = image; }

	const Color &GetBaseColor() const { return baseColor; }
	void SetBaseColor(const Color &baseColor) { this->baseColor = baseColor; }

	float GetBlend() const { return blend; }
	void SetBlend(float blend) { this->blend = blend; }

	const Color &GetFogColor() const { return fogColor; }
	void SetFogColor(const Color &fogColor) { this->fogColor = fogColor; }

	const Vector2f &GetFogLimits() const { return fogLimits; }
	void SetFogLimits(const Vector2f &fogLimits) { this->fogLimits = fogLimits; }

	friend const Node &operator>>(const Node &node, SkyboxMaterial &material);
	friend Node &operator<<(Node &node, const SkyboxMaterial &material);

private:
	std::shared_ptr<ImageCube> image;
	Color baseColor;
	float blend;
	Color fogColor;
	Vector2f fogLimits;
};
}
