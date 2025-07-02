#pragma once

#include <drx3D/Physics/Colliders/Collider.h>
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>

namespace drx3d {
class DRX3D_EXPORT CylinderCollider : public Collider::Registry<CylinderCollider> {
	inline static const bool Registered = Register("cylinder");
public:
	explicit CylinderCollider(float radius = 1.0f, float height = 1.0f, const Transform &localTransform = {});
	~CylinderCollider();

	::CollisionShape *GetCollisionShape() const override;

	float GetRadius() const { return radius; }
	void SetRadius(float radius);

	float GetHeight() const { return height; }
	void SetHeight(float height);

	friend const Node &operator>>(const Node &node, CylinderCollider &collider);
	friend Node &operator<<(Node &node, const CylinderCollider &collider);

private:
	std::unique_ptr<::CylinderShape> shape;
	float radius;
	float height;
};
}
