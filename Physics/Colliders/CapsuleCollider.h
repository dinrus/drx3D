#pragma once

#include <drx3D/Physics/Colliders/Collider.h>

class CapsuleShape;

namespace drx3d {
class DRX3D_EXPORT CapsuleCollider : public Collider::Registry<CapsuleCollider> {
	inline static const bool Registered = Register("capsule");
public:
	explicit CapsuleCollider(float radius = 0.5f, float height = 1.0f, const Transform &localTransform = {});
	~CapsuleCollider();

	CollisionShape *GetCollisionShape() const override;

	float GetRadius() const { return radius; }
	void SetRadius(float radius);

	float GetHeight() const { return height; }
	void SetHeight(float height);

	friend const Node &operator>>(const Node &node, CapsuleCollider &collider);
	friend Node &operator<<(Node &node, const CapsuleCollider &collider);

private:
	std::unique_ptr<CapsuleShape> shape;
	float radius;
	float height;
};
}
