#ifndef DRX3D_DISPATCHER_H
#define DRX3D_DISPATCHER_H
#include <drx3D/Maths/Linear/Scalar.h>

class CollisionAlgorithm;
struct BroadphaseProxy;
class RigidBody;
class CollisionObject2;
class OverlappingPairCache;
struct CollisionObject2Wrapper;

class PersistentManifold;
class PoolAllocator;

struct DispatcherInfo
{
	enum DispatchFunc
	{
		DISPATCH_DISCRETE = 1,
		DISPATCH_CONTINUOUS
	};
	DispatcherInfo()
		: m_timeStep(Scalar(0.)),
		  m_stepCount(0),
		  m_dispatchFunc(DISPATCH_DISCRETE),
		  m_timeOfImpact(Scalar(1.)),
		  m_useContinuous(true),
		  m_debugDraw(0),
		  m_enableSatConvex(false),
		  m_enableSPU(true),
		  m_useEpa(true),
		  m_allowedCcdPenetration(Scalar(0.04)),
		  m_useConvexConservativeDistanceUtil(false),
		  m_convexConservativeDistanceThreshold(0.0f),
		  m_deterministicOverlappingPairs(false)
	{
	}
	Scalar m_timeStep;
	i32 m_stepCount;
	i32 m_dispatchFunc;
	mutable Scalar m_timeOfImpact;
	bool m_useContinuous;
	class IDebugDraw* m_debugDraw;
	bool m_enableSatConvex;
	bool m_enableSPU;
	bool m_useEpa;
	Scalar m_allowedCcdPenetration;
	bool m_useConvexConservativeDistanceUtil;
	Scalar m_convexConservativeDistanceThreshold;
	bool m_deterministicOverlappingPairs;
};

enum eDispatcherQueryType
{
	DRX3D_CONTACT_POINT_ALGORITHMS = 1,
	DRX3D_CLOSEST_POINT_ALGORITHMS = 2
};

///The Dispatcher interface class can be used in combination with broadphase to dispatch calculations for overlapping pairs.
///For example for pairwise collision detection, calculating contact points stored in PersistentManifold or user callbacks (game logic).
class Dispatcher
{
public:
	virtual ~Dispatcher();

	virtual CollisionAlgorithm* findAlgorithm(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, PersistentManifold* sharedManifold, eDispatcherQueryType queryType) = 0;

	virtual PersistentManifold* getNewManifold(const CollisionObject2* b0, const CollisionObject2* b1) = 0;

	virtual void releaseManifold(PersistentManifold* manifold) = 0;

	virtual void clearManifold(PersistentManifold* manifold) = 0;

	virtual bool needsCollision(const CollisionObject2* body0, const CollisionObject2* body1) = 0;

	virtual bool needsResponse(const CollisionObject2* body0, const CollisionObject2* body1) = 0;

	virtual void dispatchAllCollisionPairs(OverlappingPairCache* pairCache, const DispatcherInfo& dispatchInfo, Dispatcher* dispatcher) = 0;

	virtual i32 getNumManifolds() const = 0;

	virtual PersistentManifold* getManifoldByIndexInternal(i32 index) = 0;

	virtual PersistentManifold** getInternalManifoldPointer() = 0;

	virtual PoolAllocator* getInternalManifoldPool() = 0;

	virtual const PoolAllocator* getInternalManifoldPool() const = 0;

	virtual uk allocateCollisionAlgorithm(i32 size) = 0;

	virtual void freeCollisionAlgorithm(uk ptr) = 0;
};

#endif  //DRX3D_DISPATCHER_H
