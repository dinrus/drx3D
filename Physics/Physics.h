#pragma once

#include <set>
#include <memory>

#include <drx3D/Maths/Vector3.h>
#include <drx3D/Scenes/System.h>

class CollisionObject2;
class CollisionConfiguration;
class BroadphaseInterface;
class Dispatcher;
class ConstraintSolver;
class DiscreteDynamicsWorld;

namespace drx3d {
class Entity;
class CollisionObject;

using CollisionPair = std::pair<const CollisionObject2 *, const CollisionObject2 *>;
using CollisionPairs = std::set<CollisionPair>;

class DRX3D_EXPORT Raycast {
public:
	Raycast(bool hasHit, const Vector3f &pointWorld, CollisionObject *collisionObject) :
		hasHit(hasHit),
		pointWorld(pointWorld),
		collisionObject(collisionObject) {
	}

	bool HasHit() const { return hasHit; }
	const Vector3f &GetPointWorld() const { return pointWorld; }
	CollisionObject *GetCollisionObject() const { return collisionObject; }

private:
	bool hasHit;
	Vector3f pointWorld;
	CollisionObject *collisionObject;
};

class DRX3D_EXPORT Physics : public System {
public:
	Physics();
	~Physics();

	void Update() override;

	Raycast Raytest(const Vector3f &start, const Vector3f &end) const;

	const Vector3f &GetGravity() const { return gravity; }
	void SetGravity(const Vector3f &gravity);

	float GetAirDensity() const { return airDensity; }
	void SetAirDensity(float airDensity);

	BroadphaseInterface *GetBroadphase() { return broadphase.get(); }

	DiscreteDynamicsWorld *GetDynamicsWorld() { return dynamicsWorld.get(); }

private:
	void CheckForCollisionEvents();

	std::unique_ptr<CollisionConfiguration> collisionConfiguration;
	std::unique_ptr<BroadphaseInterface> broadphase;
	std::unique_ptr<Dispatcher> dispatcher;
	std::unique_ptr<ConstraintSolver> solver;
	std::unique_ptr<DiscreteDynamicsWorld> dynamicsWorld;
	CollisionPairs pairsLastUpdate;

	Vector3f gravity;
	float airDensity;
};
}
