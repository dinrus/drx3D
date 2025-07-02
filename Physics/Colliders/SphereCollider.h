#pragma once

#include <drx3D/Physics/Colliders/Collider.h>

class SphereShape;

namespace drx3d {
class DRX3D_EXPORT SphereCollider : public Collider::Registry<SphereCollider> {
	inline static const bool Registered = Register("sphere");
public:
	explicit SphereCollider(float radius = 0.5f, const Transform &localTransform = {});
	~SphereCollider();

	CollisionShape *GetCollisionShape() const override;

	float GetRadius() const { return radius; }
	void SetRadius(float radius);

	friend const Node &operator>>(const Node &node, SphereCollider &collider);
	friend Node &operator<<(Node &node, const SphereCollider &collider);

private:
	std::unique_ptr<SphereShape> shape;
	float radius;
};
}
