#include <drx3D/Physics/Colliders/CubeCollider.h>

#include <drx3D/Physics/Collision/Shapes/BoxShape.h>

namespace drx3d {
CubeCollider::CubeCollider(const Vector3f &extents, const Transform &localTransform) :
	//Collider(localTransform),
	shape(std::make_unique<BoxShape>(Convert(extents / 2.0f))),
	extents(extents) {
	this->localTransform = localTransform;
	this->localTransform.SetLocalScale(extents);
}

CubeCollider::~CubeCollider() {
}

CollisionShape *CubeCollider::GetCollisionShape() const {
	return shape.get();
}

void CubeCollider::SetExtents(const Vector3f &extents) {
	this->extents = extents;
	shape->setImplicitShapeDimensions(Convert(extents));
	localTransform.SetLocalScale(extents);
}

const Node &operator>>(const Node &node, CubeCollider &collider) {
	node["localTransform"].Get(collider.localTransform);
	node["extents"].Get(collider.extents);
	return node;
}

Node &operator<<(Node &node, const CubeCollider &collider) {
	node["localTransform"].Set(collider.localTransform);
	node["extents"].Set(collider.extents);
	return node;
}
}
