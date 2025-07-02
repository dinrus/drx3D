#pragma once

#include <drx3D/Maths/Color.h>
#include <drx3D/Maths/Vector3.h>
#include <drx3D/Scenes/Component.h>

namespace drx3d {
/**
 * @brief Component that represents a point light.
 */
class DRX3D_EXPORT Light : public Component::Registry<Light> {
	inline static const bool Registered = Register("light");
public:
	/**
	 * Creates a new point light.
	 * @param color The color of the light.
	 * @param radius How far the light will have influence (-1 sets this to a directional light).
	 */
	explicit Light(const Color &color = Color::White, float radius = -1.0f);

	void Start() override;
	void Update() override;

	const Color &GetColor() const { return color; }
	void SetColor(const Color &color) { this->color = color; }

	float GetRadius() const { return radius; }
	void SetRadius(float radius) { this->radius = radius; }

	friend const Node &operator>>(const Node &node, Light &light);
	friend Node &operator<<(Node &node, const Light &light);

private:
	Color color;
	Vector3f position;
	float radius;
};
}
