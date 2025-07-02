#include <drx3D/Physics/Colliders/ConeCollider.h>
#include <drx3D/Physics/Collision/Shapes/ConeShape.h>

namespace drx3d {
ConeCollider::ConeCollider(float radius, float height, const Transform &localTransform) :
	//Collider(localTransform),
	shape(std::make_unique<ConeShape>(radius, height)),
	radius(radius),
	height(height) {
	this->localTransform = localTransform;
	this->localTransform.SetLocalScale({radius, height, radius});
}

ConeCollider::~ConeCollider() {
}

CollisionShape *ConeCollider::GetCollisionShape() const {
	return shape.get();
}

void ConeCollider::SetRadius(float radius) {
	this->radius = radius;
	shape->setRadius(radius);
	localTransform.SetLocalScale({radius, height, radius});
}

void ConeCollider::SetHeight(float height) {
	this->height = height;
	shape->setHeight(height);
	localTransform.SetLocalScale({radius, height, radius});
}

const Node &operator>>(const Node &node, ConeCollider &collider) {
	node["localTransform"].Get(collider.localTransform);
	node["radius"].Get(collider.radius);
	node["height"].Get(collider.height);
	return node;
}

Node &operator<<(Node &node, const ConeCollider &collider) {
	node["localTransform"].Set(collider.localTransform);
	node["radius"].Set(collider.radius);
	node["height"].Set(collider.height);
	return node;
}
}
