#pragma once

#include <drx3D/Maths/Color.h>
#include <drx3D/Files/Node.h>
#include <drx3D/Scenes/Component.h>

namespace drx3d {
/**
 * @brief Component that represents a 3d fog.
 */
class DRX3D_EXPORT Fog : public Component::Registry<Fog> {
	inline static const bool Registered = Register("fog");
public:
	/**
	 * Creates a new hazy fog.
	 * @param color The color of the fog.
	 * @param density How dense the fog will be.
	 * @param gradient The gradient of the fog.
	 * @param lowerLimit At what height will the skybox fog begin to appear.
	 * @param upperLimit At what height will there be skybox no fog.
	 */
	explicit Fog(const Color &color = Color::White, float density = 0.0f, float gradient = -1.0f, float lowerLimit = 0.0f, float upperLimit = 0.0f);

	void Start() override;
	void Update() override;

	const Color &GetColor() const { return color; }
	void SetColor(const Color &color) { this->color = color; }

	float GetDensity() const { return density; }
	void SetDensity(float density) { this->density = density; }

	float GetGradient() const { return gradient; }
	void SetGradient(float gradient) { this->gradient = gradient; }

	float GetLowerLimit() const { return lowerLimit; }
	void SetLowerLimit(float lowerLimit) { this->lowerLimit = lowerLimit; }

	float GetUpperLimit() const { return upperLimit; }
	void SetUpperLimit(float upperLimit) { this->upperLimit = upperLimit; }

	friend const Node &operator>>(const Node &node, Fog &fog);
	friend Node &operator<<(Node &node, const Fog &fog);

private:
	Color color;
	float density;
	float gradient;
	float lowerLimit, upperLimit;
};
}
