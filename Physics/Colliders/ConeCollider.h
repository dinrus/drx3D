#pragma once

#include <drx3D/Physics/Colliders/Collider.h>

class ConeShape;

namespace drx3d {
class DRX3D_EXPORT ConeCollider : public Collider::Registry<ConeCollider> {
	inline static const bool Registered = Register("cone");
public:
	explicit ConeCollider(float radius = 1.0f, float height = 1.0f, const Transform &localTransform = {});
	~ConeCollider();

	CollisionShape *GetCollisionShape() const override;

	float GetRadius() const { return radius; }
	void SetRadius(float radius);

	float GetHeight() const { return height; }
	void SetHeight(float height);

	friend const Node &operator>>(const Node &node, ConeCollider &collider);
	friend Node &operator<<(Node &node, const ConeCollider &collider);

private:
	std::unique_ptr<ConeShape> shape;
	float radius;
	float height;
};
}
