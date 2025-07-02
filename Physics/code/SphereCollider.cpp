#include <drx3D/Physics/Colliders/SphereCollider.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>

namespace drx3d {
SphereCollider::SphereCollider(float radius, const Transform &localTransform) :
	//Collider(localTransform),
	shape(std::make_unique<SphereShape>(radius)),
	radius(radius) {
	this->localTransform = localTransform;
	this->localTransform.SetLocalScale({radius, radius, radius});
}

SphereCollider::~SphereCollider() {
}

CollisionShape *SphereCollider::GetCollisionShape() const {
	return shape.get();
}

void SphereCollider::SetRadius(float radius) {
	this->radius = radius;
	shape->setUnscaledRadius(radius);
	localTransform.SetLocalScale({radius, radius, radius});
}

const Node &operator>>(const Node &node, SphereCollider &collider) {
	node["localTransform"].Get(collider.localTransform);
	node["radius"].Get(collider.radius);
	return node;
}

Node &operator<<(Node &node, const SphereCollider &collider) {
	node["localTransform"].Set(collider.localTransform);
	node["radius"].Set(collider.radius);
	return node;
}
}
