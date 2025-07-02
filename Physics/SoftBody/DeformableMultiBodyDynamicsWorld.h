#ifndef DRX3D_DEFORMABLE_MULTIBODY_DYNAMICS_WORLD_H
#define DRX3D_DEFORMABLE_MULTIBODY_DYNAMICS_WORLD_H

#include <drx3D/Physics/SoftBody/SoftMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>
#include <drx3D/Physics/SoftBody/DeformableMassSpringForce.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyConstraintSolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/Collision/Dispatch/SimulationIslandManager.h>
#include <functional>
typedef AlignedObjectArray<SoftBody*> SoftBodyArray;

class DeformableBodySolver;
class DeformableLagrangianForce;
struct MultiBodyInplaceSolverIslandCallback;
struct DeformableBodyInplaceSolverIslandCallback;
class DeformableMultiBodyConstraintSolver;

typedef AlignedObjectArray<SoftBody*> SoftBodyArray;

class DeformableMultiBodyDynamicsWorld : public MultiBodyDynamicsWorld
{
	typedef AlignedObjectArray<Vec3> TVStack;
	///Solver classes that encapsulate multiple deformable bodies for solving
	DeformableBodySolver* m_deformableBodySolver;
	SoftBodyArray m_softBodies;
	i32 m_drawFlags;
	bool m_drawNodeTree;
	bool m_drawFaceTree;
	bool m_drawClusterTree;
	SoftBodyWorldInfo m_sbi;
	Scalar m_internalTime;
	i32 m_ccdIterations;
	bool m_implicit;
	bool m_lineSearch;
	bool m_useProjection;
	DeformableBodyInplaceSolverIslandCallback* m_solverDeformableBodyIslandCallback;

	typedef void (*SolverCallback)(Scalar time, DeformableMultiBodyDynamicsWorld* world);
	SolverCallback m_solverCallback;

protected:
	virtual void internalSingleStepSimulation(Scalar timeStep);

	virtual void integrateTransforms(Scalar timeStep);

	void positionCorrection(Scalar timeStep);

	void solveConstraints(Scalar timeStep);

	void updateActivationState(Scalar timeStep);

	void clearGravity();

public:
	DeformableMultiBodyDynamicsWorld(Dispatcher* dispatcher, BroadphaseInterface* pairCache, DeformableMultiBodyConstraintSolver* constraintSolver, CollisionConfiguration* collisionConfiguration, DeformableBodySolver* deformableBodySolver = 0);

	virtual i32 stepSimulation(Scalar timeStep, i32 maxSubSteps = 1, Scalar fixedTimeStep = Scalar(1.) / Scalar(60.));

	virtual void debugDrawWorld();

	void setSolverCallback(SolverCallback cb)
	{
		m_solverCallback = cb;
	}

	virtual ~DeformableMultiBodyDynamicsWorld();

	virtual MultiBodyDynamicsWorld* getMultiBodyDynamicsWorld()
	{
		return (MultiBodyDynamicsWorld*)(this);
	}

	virtual const MultiBodyDynamicsWorld* getMultiBodyDynamicsWorld() const
	{
		return (const MultiBodyDynamicsWorld*)(this);
	}

	virtual DynamicsWorldType getWorldType() const
	{
		return DRX3D_DEFORMABLE_MULTIBODY_DYNAMICS_WORLD;
	}

	virtual void predictUnconstraintMotion(Scalar timeStep);

	virtual void addSoftBody(SoftBody* body, i32 collisionFilterGroup = BroadphaseProxy::DefaultFilter, i32 collisionFilterMask = BroadphaseProxy::AllFilter);

	SoftBodyArray& getSoftBodyArray()
	{
		return m_softBodies;
	}

	const SoftBodyArray& getSoftBodyArray() const
	{
		return m_softBodies;
	}

	SoftBodyWorldInfo& getWorldInfo()
	{
		return m_sbi;
	}

	const SoftBodyWorldInfo& getWorldInfo() const
	{
		return m_sbi;
	}

	virtual void setGravity(const Vec3& gravity);

	void reinitialize(Scalar timeStep);

	void applyRigidBodyGravity(Scalar timeStep);

	void beforeSolverCallbacks(Scalar timeStep);

	void afterSolverCallbacks(Scalar timeStep);

	void addForce(SoftBody* psb, DeformableLagrangianForce* force);

	void removeForce(SoftBody* psb, DeformableLagrangianForce* force);

	void removeSoftBodyForce(SoftBody* psb);

	void removeSoftBody(SoftBody* body);

	void removeCollisionObject(CollisionObject2* collisionObject);

	i32 getDrawFlags() const { return (m_drawFlags); }
	void setDrawFlags(i32 f) { m_drawFlags = f; }

	void setupConstraints();

	void performDeformableCollisionDetection();

	void solveMultiBodyConstraints();

	void solveContactConstraints();

	void sortConstraints();

	void softBodySelfCollision();

	void setImplicit(bool implicit)
	{
		m_implicit = implicit;
	}

	void setLineSearch(bool lineSearch)
	{
		m_lineSearch = lineSearch;
	}

	void setUseProjection(bool useProjection)
	{
		m_useProjection = useProjection;
	}

	void applyRepulsionForce(Scalar timeStep);

	void performGeometricCollisions(Scalar timeStep);

	struct DeformableSingleRayCallback : public BroadphaseRayCallback
	{
		Vec3 m_rayFromWorld;
		Vec3 m_rayToWorld;
		Transform2 m_rayFromTrans;
		Transform2 m_rayToTrans;
		Vec3 m_hitNormal;

		const DeformableMultiBodyDynamicsWorld* m_world;
		CollisionWorld::RayResultCallback& m_resultCallback;

		DeformableSingleRayCallback(const Vec3& rayFromWorld, const Vec3& rayToWorld, const DeformableMultiBodyDynamicsWorld* world, CollisionWorld::RayResultCallback& resultCallback)
			: m_rayFromWorld(rayFromWorld),
			  m_rayToWorld(rayToWorld),
			  m_world(world),
			  m_resultCallback(resultCallback)
		{
			m_rayFromTrans.setIdentity();
			m_rayFromTrans.setOrigin(m_rayFromWorld);
			m_rayToTrans.setIdentity();
			m_rayToTrans.setOrigin(m_rayToWorld);

			Vec3 rayDir = (rayToWorld - rayFromWorld);

			rayDir.normalize();
			///what about division by zero? --> just set rayDirection[i] to INF/1e30
			m_rayDirectionInverse[0] = rayDir[0] == Scalar(0.0) ? Scalar(1e30) : Scalar(1.0) / rayDir[0];
			m_rayDirectionInverse[1] = rayDir[1] == Scalar(0.0) ? Scalar(1e30) : Scalar(1.0) / rayDir[1];
			m_rayDirectionInverse[2] = rayDir[2] == Scalar(0.0) ? Scalar(1e30) : Scalar(1.0) / rayDir[2];
			m_signs[0] = m_rayDirectionInverse[0] < 0.0;
			m_signs[1] = m_rayDirectionInverse[1] < 0.0;
			m_signs[2] = m_rayDirectionInverse[2] < 0.0;

			m_lambda_max = rayDir.dot(m_rayToWorld - m_rayFromWorld);
		}

		virtual bool process(const BroadphaseProxy* proxy)
		{
			///terminate further ray tests, once the closestHitFraction reached zero
			if (m_resultCallback.m_closestHitFraction == Scalar(0.f))
				return false;

			CollisionObject2* collisionObject = (CollisionObject2*)proxy->m_clientObject;

			//only perform raycast if filterMask matches
			if (m_resultCallback.needsCollision(collisionObject->getBroadphaseHandle()))
			{
				//RigidcollisionObject* collisionObject = ctrl->GetRigidcollisionObject();
				//Vec3 collisionObjectAabbMin,collisionObjectAabbMax;
#if 0
#ifdef RECALCULATE_AABB
                Vec3 collisionObjectAabbMin,collisionObjectAabbMax;
                collisionObject->getCollisionShape()->getAabb(collisionObject->getWorldTransform(),collisionObjectAabbMin,collisionObjectAabbMax);
#else
                //getBroadphase()->getAabb(collisionObject->getBroadphaseHandle(),collisionObjectAabbMin,collisionObjectAabbMax);
                const Vec3& collisionObjectAabbMin = collisionObject->getBroadphaseHandle()->m_aabbMin;
                const Vec3& collisionObjectAabbMax = collisionObject->getBroadphaseHandle()->m_aabbMax;
#endif
#endif
				//Scalar hitLambda = m_resultCallback.m_closestHitFraction;
				//culling already done by broadphase
				//if (RayAabb(m_rayFromWorld,m_rayToWorld,collisionObjectAabbMin,collisionObjectAabbMax,hitLambda,m_hitNormal))
				{
					m_world->rayTestSingle(m_rayFromTrans, m_rayToTrans,
										   collisionObject,
										   collisionObject->getCollisionShape(),
										   collisionObject->getWorldTransform(),
										   m_resultCallback);
				}
			}
			return true;
		}
	};

	void rayTest(const Vec3& rayFromWorld, const Vec3& rayToWorld, RayResultCallback& resultCallback) const
	{
		DRX3D_PROFILE("rayTest");
		/// use the broadphase to accelerate the search for objects, based on their aabb
		/// and for each object with ray-aabb overlap, perform an exact ray test
		DeformableSingleRayCallback rayCB(rayFromWorld, rayToWorld, this, resultCallback);

#ifndef USE_BRUTEFORCE_RAYBROADPHASE
		m_broadphasePairCache->rayTest(rayFromWorld, rayToWorld, rayCB);
#else
		for (i32 i = 0; i < this->getNumCollisionObjects(); i++)
		{
			rayCB.process(m_collisionObjects[i]->getBroadphaseHandle());
		}
#endif  //USE_BRUTEFORCE_RAYBROADPHASE
	}

	void rayTestSingle(const Transform2& rayFromTrans, const Transform2& rayToTrans,
					   CollisionObject2* collisionObject,
					   const CollisionShape* collisionShape,
					   const Transform2& colObjWorldTransform,
					   RayResultCallback& resultCallback) const
	{
		if (collisionShape->isSoftBody())
		{
			SoftBody* softBody = SoftBody::upcast(collisionObject);
			if (softBody)
			{
				SoftBody::sRayCast softResult;
				if (softBody->rayFaceTest(rayFromTrans.getOrigin(), rayToTrans.getOrigin(), softResult))
				{
					if (softResult.fraction <= resultCallback.m_closestHitFraction)
					{
						CollisionWorld::LocalShapeInfo shapeInfo;
						shapeInfo.m_shapePart = 0;
						shapeInfo.m_triangleIndex = softResult.index;
						// get the normal
						Vec3 rayDir = rayToTrans.getOrigin() - rayFromTrans.getOrigin();
						Vec3 normal = -rayDir;
						normal.normalize();
						{
							normal = softBody->m_faces[softResult.index].m_normal;
							if (normal.dot(rayDir) > 0)
							{
								// normal always point toward origin of the ray
								normal = -normal;
							}
						}

						CollisionWorld::LocalRayResult rayResult(collisionObject,
																   &shapeInfo,
																   normal,
																   softResult.fraction);
						bool normalInWorldSpace = true;
						resultCallback.addSingleResult(rayResult, normalInWorldSpace);
					}
				}
			}
		}
		else
		{
			CollisionWorld::rayTestSingle(rayFromTrans, rayToTrans, collisionObject, collisionShape, colObjWorldTransform, resultCallback);
		}
	}
};

#endif  //DRX3D_DEFORMABLE_MULTIBODY_DYNAMICS_WORLD_H
