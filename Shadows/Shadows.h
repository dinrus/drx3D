#pragma once

#include <drx3D/Maths/Vector3.h>
#include <drx3D/Scenes/System.h>
#include <drx3D/Shadows/ShadowBox.h>

namespace drx3d {
/**
 * @brief Module used for managing a shadow map.
 */
class DRX3D_EXPORT Shadows : public System {
public:
	Shadows();

	void Update() override;

	const Vector3f &GetLightDirection() const { return lightDirection; }
	void SetLightDirection(const Vector3f &lightDirection) { this->lightDirection = lightDirection; }

	uint32_t GetShadowSize() const { return shadowSize; }
	void SetShadowSize(uint32_t shadowSize) { this->shadowSize = shadowSize; }

	int32_t GetShadowPcf() const { return shadowPcf; }
	void SetShadowPcf(int32_t shadowPcf) { this->shadowPcf = shadowPcf; }

	float GetShadowBias() const { return shadowBias; }
	void SetShadowBias(float shadowBias) { this->shadowBias = shadowBias; }

	float GetShadowDarkness() const { return shadowDarkness; }
	void SetShadowDarkness(float shadowDarkness) { this->shadowDarkness = shadowDarkness; }

	float GetShadowTransition() const { return shadowTransition; }
	void SetShadowTransition(float shadowTransition) { this->shadowTransition = shadowTransition; }

	float GetShadowBoxOffset() const { return shadowBoxOffset; }
	void SetShadowBoxOffset(float shadowBoxOffset) { this->shadowBoxOffset = shadowBoxOffset; }

	float GetShadowBoxDistance() const { return shadowBoxDistance; }
	void SetShadowBoxDistance(float shadowBoxDistance) { this->shadowBoxDistance = shadowBoxDistance; }

	/**
	 * Get the shadow box, so that it can be used by other class to test if engine.entities are inside the box.
	 * @return The shadow box.
	 */
	const ShadowBox &GetShadowBox() const { return shadowBox; }

private:
	Vector3f lightDirection;

	uint32_t shadowSize;
	int32_t shadowPcf;
	float shadowBias;
	float shadowDarkness;
	float shadowTransition;

	float shadowBoxOffset;
	float shadowBoxDistance;

	ShadowBox shadowBox;
};
}
