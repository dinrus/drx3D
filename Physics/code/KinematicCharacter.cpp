#include <drx3D/Physics/KinematicCharacter.h>
#include <drx3D/Physics/Collision/Dispatch/GhostObject.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/SoftBody/SoftRigidDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/Character/KinematicCharacterController.h>
#include <drx3D/Maths/Transform.h>
#include <drx3D/Scenes/Entity.h>
#include <drx3D/Scenes/Scenes.h>
#include <drx3D/Physics/Physics.h>

namespace drx3d {
KinematicCharacter::KinematicCharacter(std::unique_ptr<Collider> &&collider, float mass, float friction) :
	CollisionObject({}, mass, friction),
	up(Vector3f::Up),
	stepHeight(0.0f),
	fallSpeed(55.0f),
	jumpSpeed(10.0f),
	maxHeight(1.5f),
	interpolate(true) {
	AddCollider(std::move(collider));
}

KinematicCharacter::~KinematicCharacter() {
	if (auto physics = Scenes::Get()->GetScene()->GetSystem<Physics>()) {
		// TODO: Are these being deleted?
		physics->GetDynamicsWorld()->removeCollisionObject(ghostObject.get());
		physics->GetDynamicsWorld()->removeAction(controller.get());
	}
}

void KinematicCharacter::Start() {
	auto physics = Scenes::Get()->GetScene()->GetSystem<Physics>();
	
	if (ghostObject)
		physics->GetDynamicsWorld()->removeCollisionObject(ghostObject.get());

	if (controller)
		physics->GetDynamicsWorld()->removeAction(controller.get());

	CreateShape(true);
	assert((shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE) && "Invalid ghost object shape!");
	gravity = physics->GetGravity();
	Vec3 localInertia;

	// Rigidbody is dynamic if and only if mass is non zero, otherwise static.
	if (mass != 0.0f)
		shape->calculateLocalInertia(mass, localInertia);

	auto worldTransform = Collider::Convert(*GetEntity()->GetComponent<Transform>());

	ghostObject = std::make_unique<::PairCachingGhostObject>();
	ghostObject->setWorldTransform(worldTransform);
	physics->GetBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new ::GhostPairCallback());
	ghostObject->setCollisionShape(shape.get());
	ghostObject->setCollisionFlags(CollisionObject2::CF_CHARACTER_OBJECT);
	ghostObject->setFriction(friction);
	ghostObject->setRollingFriction(frictionRolling);
	ghostObject->setSpinningFriction(frictionSpinning);
	ghostObject->setUserPointer(dynamic_cast<CollisionObject *>(this));
	physics->GetDynamicsWorld()->addCollisionObject(ghostObject.get(), ::BroadphaseProxy::CharacterFilter, ::BroadphaseProxy::AllFilter);
	body = ghostObject.get();

	controller = std::make_unique<::KinematicCharacterController>(ghostObject.get(), static_cast<::ConvexShape *>(shape.get()), 0.03f);
	controller->setGravity(Collider::Convert(gravity));
	controller->setUp(Collider::Convert(up));
	controller->setStepHeight(stepHeight);
	controller->setFallSpeed(fallSpeed);
	controller->setJumpSpeed(jumpSpeed);
	controller->setMaxJumpHeight(maxHeight);
	controller->setUpInterpolate(interpolate);
	physics->GetDynamicsWorld()->addAction(controller.get());
}

void KinematicCharacter::Update() {
	if (shape.get() != body->getCollisionShape())
		body->setCollisionShape(shape.get());

	auto &transform = *GetEntity()->GetComponent<Transform>();
	auto worldTransform = ghostObject->getWorldTransform();
	transform = Collider::Convert(worldTransform, transform.GetScale());

	linearVelocity = Collider::Convert(controller->getLinearVelocity());
	angularVelocity = Collider::Convert(controller->getAngularVelocity());
}

bool KinematicCharacter::InFrustum(const Frustum &frustum) {
	Vec3 min;
	Vec3 max;

	if (body && shape)
		shape->getAabb(Collider::Convert(*GetEntity()->GetComponent<Transform>()), min, max);

	return frustum.CubeInFrustum(Collider::Convert(min), Collider::Convert(max));
}

void KinematicCharacter::ClearForces() {
	//controller->clearForces();
}

void KinematicCharacter::SetMass(float mass) {
	this->mass = mass;
	RecalculateMass();
}

void KinematicCharacter::SetGravity(const Vector3f &gravity) {
	this->gravity = gravity;
	controller->setGravity(Collider::Convert(gravity));
}

void KinematicCharacter::SetLinearFactor(const Vector3f &linearFactor) {
	this->linearFactor = linearFactor;
	//controller->setLinearFactor(Collider::Convert(linearFactor)); // TODO
}

void KinematicCharacter::SetAngularFactor(const Vector3f &angularFactor) {
	this->angularFactor = angularFactor;
	//controller->setAngularFactor(Collider::Convert(angularFactor)); // TODO
}

void KinematicCharacter::SetLinearVelocity(const Vector3f &linearVelocity) {
	this->linearVelocity = linearVelocity;
	controller->setLinearVelocity(Collider::Convert(linearVelocity));
}

void KinematicCharacter::SetAngularVelocity(const Vector3f &angularVelocity) {
	this->angularVelocity = angularVelocity;
	controller->setAngularVelocity(Collider::Convert(angularVelocity));
}

void KinematicCharacter::SetUp(const Vector3f &up) {
	this->up = up;
	controller->setUp(Collider::Convert(up));
}

void KinematicCharacter::SetStepHeight(float stepHeight) {
	this->stepHeight = stepHeight;
	controller->setStepHeight(stepHeight);
}

void KinematicCharacter::SetFallSpeed(float fallSpeed) {
	this->fallSpeed = fallSpeed;
	controller->setFallSpeed(fallSpeed);
}

void KinematicCharacter::SetJumpSpeed(float jumpSpeed) {
	this->jumpSpeed = jumpSpeed;
	controller->setJumpSpeed(jumpSpeed);
}

void KinematicCharacter::SetMaxJumpHeight(float maxHeight) {
	this->maxHeight = maxHeight;
	controller->setMaxJumpHeight(maxHeight);
}

void KinematicCharacter::SetInterpolate(bool interpolate) {
	this->interpolate = interpolate;
	controller->setUpInterpolate(interpolate);
}

bool KinematicCharacter::IsOnGround() const {
	return controller->onGround();
}

void KinematicCharacter::Jump(const Vector3f &direction) {
	controller->jump(Collider::Convert(direction));
}

void KinematicCharacter::SetWalkDirection(const Vector3f &direction) {
	controller->setWalkDirection(Collider::Convert(direction));
}

const Node &operator>>(const Node &node, KinematicCharacter &character) {
	node["colliders"].Get(character.colliders);
	node["mass"].Get(character.mass);
	node["friction"].Get(character.friction);
	node["frictionRolling"].Get(character.frictionRolling);
	node["frictionSpinning"].Get(character.frictionSpinning);
	node["up"].Get(character.up);
	node["stepHeight"].Get(character.stepHeight);
	node["fallSpeed"].Get(character.fallSpeed);
	node["jumpSpeed"].Get(character.jumpSpeed);
	node["maxHeight"].Get(character.maxHeight);
	node["interpolate"].Get(character.interpolate);
	return node;
}

Node &operator<<(Node &node, const KinematicCharacter &character) {
	node["colliders"].Set(character.colliders);
	node["mass"].Set(character.mass);
	node["friction"].Set(character.friction);
	node["frictionRolling"].Set(character.frictionRolling);
	node["frictionSpinning"].Set(character.frictionSpinning);
	node["up"].Set(character.up);
	node["stepHeight"].Set(character.stepHeight);
	node["fallSpeed"].Set(character.fallSpeed);
	node["jumpSpeed"].Set(character.jumpSpeed);
	node["maxHeight"].Set(character.maxHeight);
	node["interpolate"].Set(character.interpolate);
	return node;
}

void KinematicCharacter::RecalculateMass() {
	// TODO
}
}
