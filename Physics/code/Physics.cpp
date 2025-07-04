#include <drx3D/Physics/Physics.h>

#include <iterator>

#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/BroadPhase/DbvtBroadphase.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/SequentialImpulseConstraintSolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/SoftBody/SoftRigidDynamicsWorld.h>
#include <drx3D/Engine/Engine.h>
#include <drx3D/Physics/Colliders/Collider.h>
#include <drx3D/Physics/CollisionObject.h>

namespace drx3d {
Physics::Physics() :
	collisionConfiguration(std::make_unique<SoftBodyRigidBodyCollisionConfiguration>()),
	broadphase(std::make_unique<DbvtBroadphase>()),
	dispatcher(std::make_unique<CollisionDispatcher>(collisionConfiguration.get())),
	solver(std::make_unique<SequentialImpulseConstraintSolver>()),
	dynamicsWorld(std::make_unique<SoftRigidDynamicsWorld>(dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get())),
	gravity(0.0f, -9.81f, 0.0f),
	airDensity(1.2f) {
	dynamicsWorld->setGravity(Collider::Convert(gravity));
	dynamicsWorld->getDispatchInfo().m_enableSPU = true;
	dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 128;
	dynamicsWorld->getSolverInfo().m_globalCfm = 0.00001f;

	auto softDynamicsWorld = static_cast<SoftRigidDynamicsWorld *>(dynamicsWorld.get());
	softDynamicsWorld->getWorldInfo().water_density = 0.0f;
	softDynamicsWorld->getWorldInfo().water_offset = 0.0f;
	softDynamicsWorld->getWorldInfo().water_normal = Vec3(0.0f, 0.0f, 0.0f);
	softDynamicsWorld->getWorldInfo().m_gravity.setVal(0.0f, -9.81f, 0.0f);
	softDynamicsWorld->getWorldInfo().air_density = airDensity;
	softDynamicsWorld->getWorldInfo().m_sparsesdf.Initialize();
}

Physics::~Physics() {
	for (int32_t i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		auto obj = dynamicsWorld->getCollisionObjectArray()[i];
		auto body = RigidBody::upcast(obj);

		if (body && body->getMotionState()) {
			delete body->getMotionState();
			body->setMotionState(nullptr);
		}

		dynamicsWorld->removeCollisionObject(obj);
	}
}

void Physics::Update() {
	dynamicsWorld->stepSimulation(Engine::Get()->GetDelta().AsSeconds());
	CheckForCollisionEvents();
}

Raycast Physics::Raytest(const Vector3f &start, const Vector3f &end) const {
	Vec3 startBt = Collider::Convert(start);
	Vec3 endBt = Collider::Convert(end);
	CollisionWorld::ClosestRayResultCallback result(startBt, endBt);
	dynamicsWorld->getCollisionWorld()->rayTest(startBt, endBt, result);

	return Raycast(result.hasHit(), Collider::Convert(result.m_hitPointWorld),
		result.m_collisionObject ? static_cast<CollisionObject *>(result.m_collisionObject->getUserPointer()) : nullptr);
}

void Physics::SetGravity(const Vector3f &gravity) {
	this->gravity = gravity;
	dynamicsWorld->setGravity(Collider::Convert(gravity));
}

void Physics::SetAirDensity(float airDensity) {
	this->airDensity = airDensity;
	auto softDynamicsWorld = static_cast<SoftRigidDynamicsWorld *>(dynamicsWorld.get());
	softDynamicsWorld->getWorldInfo().air_density = airDensity;
	softDynamicsWorld->getWorldInfo().m_sparsesdf.Initialize();
}

void Physics::CheckForCollisionEvents() {
	// Keep a list of the collision pairs found during the current update.
	CollisionPairs pairsThisUpdate;

	// Iterate through all of the manifolds in the dispatcher.
	for (int32_t i = 0; i < dispatcher->getNumManifolds(); ++i) {
		// Get the manifold.
		auto manifold = dispatcher->getManifoldByIndexInternal(i);

		// Ignore manifolds that have no contact points..
		if (manifold->getNumContacts() == 0)
			continue;

		// Get the two rigid bodies involved in the collision.
		auto body0 = manifold->getBody0();
		auto body1 = manifold->getBody1();

		// Always create the pair in a predictable order (use the pointer value..).
		const auto swapped = body0 > body1;
		const auto sortedBodyA = swapped ? body1 : body0;
		const auto sortedBodyB = swapped ? body0 : body1;

		// Create the pair.
		auto thisPair = std::make_pair(sortedBodyA, sortedBodyB);

		// Insert the pair into the current list.
		pairsThisUpdate.insert(thisPair);

		// If this pair doesn't exist in the list from the previous update, it is a new pair and we must send a collision event.
		if (pairsLastUpdate.find(thisPair) == pairsLastUpdate.end()) {
			// Gets the user pointer (entity).
			auto collisionObjectA = static_cast<CollisionObject *>(sortedBodyA->getUserPointer());
			auto collisionObjectB = static_cast<CollisionObject *>(sortedBodyB->getUserPointer());

			collisionObjectA->OnCollision()(collisionObjectB);
		}
	}

	// Creates another list for pairs that were removed this update.
	CollisionPairs removedPairs;

	// This handy function gets the difference between two sets. It takes the difference between collision pairs from the last update,
	// and this update and pushes them into the removed pairs list.
	std::set_difference(pairsLastUpdate.begin(), pairsLastUpdate.end(), pairsThisUpdate.begin(), pairsThisUpdate.end(), std::inserter(removedPairs, removedPairs.begin()));

	// Iterate through all of the removed pairs sending separation events for them.
	for (const auto &[removedObject0, removedObject1] : removedPairs) {
		// Gets the user pointer (entity).
		auto collisionObjectA = static_cast<CollisionObject *>(removedObject0->getUserPointer());
		auto collisionObjectB = static_cast<CollisionObject *>(removedObject1->getUserPointer());

		collisionObjectA->OnSeparation()(collisionObjectB);
	}

	// In the next iteration we'll want to compare against the pairs we found in this iteration.
	pairsLastUpdate = pairsThisUpdate;
}
}
