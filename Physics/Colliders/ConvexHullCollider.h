#pragma once

#include <drx3D/Geometry/Model.h>
#include <drx3D/Physics/Colliders/Collider.h>

class ConvexHullShape;

namespace drx3d {
class DRX3D_EXPORT ConvexHullCollider : public Collider::Registry<ConvexHullCollider> {
	inline static const bool Registered = Register("convexHull");
public:
	explicit ConvexHullCollider(const std::vector<float> &pointCloud = {}, const Transform &localTransform = {});
	~ConvexHullCollider();

	CollisionShape *GetCollisionShape() const override;

	uint32_t GetPointCount() const { return pointCount; }
	void SetPointCount(const std::vector<float> &pointCloud);

	friend const Node &operator>>(const Node &node, ConvexHullCollider &collider);
	friend Node &operator<<(Node &node, const ConvexHullCollider &collider);

private:
	std::unique_ptr<ConvexHullShape> shape;
	std::shared_ptr<Model> model;
	uint32_t pointCount = 0;
};
}
