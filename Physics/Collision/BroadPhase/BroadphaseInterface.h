#ifndef DRX3D_BROADPHASE_INTERFACE_H
#define DRX3D_BROADPHASE_INTERFACE_H

struct DispatcherInfo;
class Dispatcher;
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>

class OverlappingPairCache;

struct BroadphaseAabbCallback
{
	virtual ~BroadphaseAabbCallback() {}
	virtual bool process(const BroadphaseProxy* proxy) = 0;
};

struct BroadphaseRayCallback : public BroadphaseAabbCallback
{
	///added some cached data to accelerate ray-AABB tests
	Vec3 m_rayDirectionInverse;
	u32 m_signs[3];
	Scalar m_lambda_max;

	virtual ~BroadphaseRayCallback() {}

protected:
	BroadphaseRayCallback() {}
};

#include <drx3D/Maths/Linear/Vec3.h>

///The BroadphaseInterface class provides an interface to detect aabb-overlapping object pairs.
///Some implementations for this broadphase interface include AxisSweep3, drx3D32BitAxisSweep3 and DbvtBroadphase.
///The actual overlapping pair management, storage, adding and removing of pairs is dealt by the OverlappingPairCache class.
class BroadphaseInterface
{
public:
	virtual ~BroadphaseInterface() {}

	virtual BroadphaseProxy* createProxy(const Vec3& aabbMin, const Vec3& aabbMax, i32 shapeType, uk userPtr, i32 collisionFilterGroup, i32 collisionFilterMask, Dispatcher* dispatcher) = 0;
	virtual void destroyProxy(BroadphaseProxy* proxy, Dispatcher* dispatcher) = 0;
	virtual void setAabb(BroadphaseProxy* proxy, const Vec3& aabbMin, const Vec3& aabbMax, Dispatcher* dispatcher) = 0;
	virtual void getAabb(BroadphaseProxy* proxy, Vec3& aabbMin, Vec3& aabbMax) const = 0;

	virtual void rayTest(const Vec3& rayFrom, const Vec3& rayTo, BroadphaseRayCallback& rayCallback, const Vec3& aabbMin = Vec3(0, 0, 0), const Vec3& aabbMax = Vec3(0, 0, 0)) = 0;

	virtual void aabbTest(const Vec3& aabbMin, const Vec3& aabbMax, BroadphaseAabbCallback& callback) = 0;

	///calculateOverlappingPairs is optional: incremental algorithms (sweep and prune) might do it during the set aabb
	virtual void calculateOverlappingPairs(Dispatcher* dispatcher) = 0;

	virtual OverlappingPairCache* getOverlappingPairCache() = 0;
	virtual const OverlappingPairCache* getOverlappingPairCache() const = 0;

	///getAabb returns the axis aligned bounding box in the 'global' coordinate frame
	///will add some transform later
	virtual void getBroadphaseAabb(Vec3& aabbMin, Vec3& aabbMax) const = 0;

	///reset broadphase internal structures, to ensure determinism/reproducability
	virtual void resetPool(Dispatcher* dispatcher) { (void)dispatcher; };

	virtual void printStats() = 0;
};

#endif  //DRX3D_BROADPHASE_INTERFACE_H
