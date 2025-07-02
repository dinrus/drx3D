#include <drx3D/Physics/Colliders/Collider.h>

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Maths/Transform.h>

namespace drx3d {
Collider::Collider(const Transform &localTransform) :
	localTransform(localTransform) {
}

void Collider::SetLocalTransform(const Transform &localTransform) {
	this->localTransform = localTransform;
	onTransformChange(this, this->localTransform);
}

Vec3 Collider::Convert(const Vector3f &vector) {
	return {vector.x, vector.y, vector.z};
}

Vector3f Collider::Convert(const Vec3 &vector) {
	return {vector.getX(), vector.getY(), vector.getZ()};
}
/*
Quaternion Collider::Convert(const Quaternion &quaternion) {
	return {quaternion.x, quaternion.y, quaternion.z, quaternion.w};
}

Quaternion Collider::Convert(const Quaternion &quaternion) {
	return {quaternion.getX(), quaternion.getY(), quaternion.getZ(), quaternion.getW()};
}
*/
Transform2 Collider::Convert(const Transform &transform) {
	Quat rotation;
	rotation.setEulerZYX(transform.GetRotation().y, transform.GetRotation().x, transform.GetRotation().z);

	Transform2 worldTransform;
	worldTransform.setIdentity();
	worldTransform.setOrigin(Convert(transform.GetPosition()));
	worldTransform.setRotation(rotation);
	return worldTransform;
}

Transform Collider::Convert(const Transform2 &transform, const Vector3f &scaling) {
	auto position = transform.getOrigin();
	float yaw, pitch, roll;
	transform.getBasis().getEulerYPR(yaw, pitch, roll);
	return {Convert(position), {pitch, yaw, roll}, scaling};
}
}