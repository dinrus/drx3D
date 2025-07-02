#ifndef DRX3D_COMPOUND_COLLISION_ALGORITHM_H
#define DRX3D_COMPOUND_COLLISION_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/BroadPhase/Dispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>

#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
class Dispatcher;
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
class Dispatcher;
class CollisionObject2;

class CollisionShape;
typedef bool (*ShapePairCallback)(const CollisionShape* pShape0, const CollisionShape* pShape1);
extern ShapePairCallback gCompoundChildShapePairCallback;

/// CompoundCollisionAlgorithm  supports collision between CompoundShapes and other collision shapes
class CompoundCollisionAlgorithm : public ActivatingCollisionAlgorithm
{
	NodeStack stack2;
	ManifoldArray manifoldArray;

protected:
	AlignedObjectArray<CollisionAlgorithm*> m_childCollisionAlgorithms;
	bool m_isSwapped;

	class PersistentManifold* m_sharedManifold;
	bool m_ownsManifold;

	i32 m_compoundShapeRevision;  //to keep track of changes, so that childAlgorithm array can be updated

	void removeChildAlgorithms();

	void preallocateChildAlgorithms(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap);

public:
	CompoundCollisionAlgorithm(const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, bool isSwapped);

	virtual ~CompoundCollisionAlgorithm();

	CollisionAlgorithm* getChildAlgorithm(i32 n) const
	{
		return m_childCollisionAlgorithms[n];
	}

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		i32 i;
		for (i = 0; i < m_childCollisionAlgorithms.size(); i++)
		{
			if (m_childCollisionAlgorithms[i])
				m_childCollisionAlgorithms[i]->getAllContactManifolds(manifoldArray);
		}
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(CompoundCollisionAlgorithm));
			return new (mem) CompoundCollisionAlgorithm(ci, body0Wrap, body1Wrap, false);
		}
	};

	struct SwappedCreateFunc : public CollisionAlgorithmCreateFunc
	{
		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(CompoundCollisionAlgorithm));
			return new (mem) CompoundCollisionAlgorithm(ci, body0Wrap, body1Wrap, true);
		}
	};
};

#endif  //DRX3D_COMPOUND_COLLISION_ALGORITHM_H
