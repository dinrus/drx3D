#pragma once

#include <drx3D/Devices/rocket.h>

#include <drx3D/Maths/Transform.h>
#include <drx3D/Maths/Quaternion.h>
#include <drx3D/Common/StreamFactory.h>
#include <drx3D/Maths/Vector3.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
class Vec3;
class Quaternion;
class Transform2;

namespace drx3d {
/**
 * @brief Класс, представляющий физическую форму.
 */
class DRX3D_EXPORT Collider : public StreamFactory<Collider> {
public:
	/**
	 * Создаёт новый коллайдер.
	 * @param localTransform The parent offset of the body.
	 */
	explicit Collider(const Transform &localTransform = {});
	virtual ~Collider() = default;

	/**
	 * Gets the collision shape defined in this collider.
	 * @return The collision shape.
	 */
	virtual CollisionShape *GetCollisionShape() const = 0;

	const Transform &GetLocalTransform() const { return localTransform; }
	void SetLocalTransform(const Transform &localTransform);

	rocket::signal<void(Collider *, const Transform &)> &OnTransformChange() { return onTransformChange; }

	static Vec3 Convert(const Vector3f &vector);
	static Vector3f Convert(const Vec3 &vector);
	/*
	static Quaternion Convert(const Quaternion &quaternion);
	static Quaternion Convert(const Quaternion &quaternion);
	*/
	static Transform2 Convert(const Transform &transform);
	static Transform Convert(const Transform2 &transform, const Vector3f &scaling = Vector3f(1.0f));

protected:
	Transform localTransform;
	rocket::signal<void(Collider*, const Transform &)> onTransformChange;
};
}
