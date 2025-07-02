#include <drx3D/Physics/Colliders/CapsuleCollider.h>

#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>

namespace drx3d {
CapsuleCollider::CapsuleCollider(float radius, float height, const Transform &localTransform) :
	//Collider(localTransform),
	shape(std::make_unique<CapsuleShape>(radius, height)),
	radius(radius),
	height(height) {
	this->localTransform = localTransform;
	this->localTransform.SetLocalScale({radius, height, radius});
}

CapsuleCollider::~CapsuleCollider() {
	// TODO: Intentional memory leak, for some reason `CapsuleShape *` can't be deleted.
	shape.release();
}

CollisionShape *CapsuleCollider::GetCollisionShape() const {
	return shape.get();
}

void CapsuleCollider::SetRadius(float radius) {
	this->radius = radius;
	shape->setImplicitShapeDimensions({radius, 0.5f * height, radius});
	localTransform.SetLocalScale({radius, height, radius});
}

void CapsuleCollider::SetHeight(float height) {
	this->height = height;
	shape->setImplicitShapeDimensions({radius, 0.5f * height, radius});
	localTransform.SetLocalScale({radius, height, radius});
}

const Node &operator>>(const Node &node, CapsuleCollider &collider) {
	node["localTransform"].Get(collider.localTransform);
	node["radius"].Get(collider.radius);
	node["height"].Get(collider.height);
	return node;
}

Node &operator<<(Node &node, const CapsuleCollider &collider) {
	node["localTransform"].Set(collider.localTransform);
	node["radius"].Set(collider.radius);
	node["height"].Set(collider.height);
	return node;
}
}
