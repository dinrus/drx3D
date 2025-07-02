#pragma once

#include <drx3D/Physics/Colliders/Collider.h>

class BoxShape;

namespace drx3d {
class DRX3D_EXPORT CubeCollider : public Collider::Registry<CubeCollider> {
	inline static const bool Registered = Register("cube");
public:
	explicit CubeCollider(const Vector3f &extents = Vector3f(1.0f), const Transform &localTransform = {});
	~CubeCollider();

	CollisionShape *GetCollisionShape() const override;

	const Vector3f &GetExtents() const { return extents; }
	void SetExtents(const Vector3f &extents);

	friend const Node &operator>>(const Node &node, CubeCollider &collider);
	friend Node &operator<<(Node &node, const CubeCollider &collider);

private:
	std::unique_ptr<BoxShape> shape;
	Vector3f extents;
};
}
