#pragma once

#include <drx3D/Maths/Vector3.h>
#include <drx3D/Scenes/Component.h>
#include <drx3D/Physics/Colliders/Collider.h>
#include <drx3D/Physics/CollisionObject.h>

class PairCachingGhostObject;
class KinematicCharacterController;

namespace drx3d {
/**
 * @brief Представляет собой контроллер кинематического персонажа.
 */
class DRX3D_EXPORT KinematicCharacter : public Component::Registry<KinematicCharacter>, public CollisionObject {
	inline static const bool Registered = Register("kinematicCharacter");
public:
	/**
	 * CreaСоздаёт новый контроллер кинематического персонажа.
	 * @param collider The collider shape to represent this rigidbody.
	 * @param mass Масса объекта.
	 * @param friction Количество поверхностного трения.
	 */
	explicit KinematicCharacter(std::unique_ptr<Collider> &&collider = nullptr, float mass = 1.0f, float friction = 0.2f);
	~KinematicCharacter();

	void Start() override;
	void Update() override;

	bool InFrustum(const Frustum &frustum) override;
	void ClearForces() override;
	void SetMass(float mass) override;
	void SetGravity(const Vector3f &gravity) override;
	void SetLinearFactor(const Vector3f &linearFactor) override;
	void SetAngularFactor(const Vector3f &angularFactor) override;
	void SetLinearVelocity(const Vector3f &linearVelocity) override;
	void SetAngularVelocity(const Vector3f &angularVelocity) override;

	const Vector3f &GetUp() const { return up; }
	void SetUp(const Vector3f &up);

	float GetStepHeight() const { return stepHeight; }
	void SetStepHeight(float stepHeight);

	float GetFallSpeed() const { return fallSpeed; }
	void SetFallSpeed(float fallSpeed);

	float GetJumpSpeed() const { return jumpSpeed; }
	void SetJumpSpeed(float jumpSpeed);

	float GetMaxJumpHeight() const { return maxHeight; }
	void SetMaxJumpHeight(float maxHeight);

	bool IsInterpolate() const { return interpolate; }
	void SetInterpolate(bool interpolate);

	bool IsOnGround() const;
	void Jump(const Vector3f &direction);
	void SetWalkDirection(const Vector3f &direction);

	friend const Node &operator>>(const Node &node, KinematicCharacter &character);
	friend Node &operator<<(Node &node, const KinematicCharacter &character);

protected:
	void RecalculateMass() override;

private:
	Vector3f up;
	float stepHeight;
	float fallSpeed;
	float jumpSpeed;
	float maxHeight;
	bool interpolate;

	std::unique_ptr<::PairCachingGhostObject> ghostObject;
	std::unique_ptr<::KinematicCharacterController> controller;
};
}
