#ifndef DRX3D_CONVEX_2D_CONVEX_2D_ALGORITHM_H
#define DRX3D_CONVEX_2D_CONVEX_2D_ALGORITHM_H

#include <drx3D/Physics/Collision/Dispatch/ActivatingCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/NarrowPhase/PersistentManifold.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseProxy.h>
#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Maths/Linear/Transform2Util.h>  //for ConvexSeparatingDistanceUtil

class ConvexPenetrationDepthSolver;

///The convex2dConvex2dAlgorithm collision algorithm support 2d collision detection for Convex2dShape
///Currently it requires the MinkowskiPenetrationDepthSolver, it has support for 2d penetration depth computation
class Convex2dConvex2dAlgorithm : public ActivatingCollisionAlgorithm
{
	SimplexSolverInterface* m_simplexSolver;
	ConvexPenetrationDepthSolver* m_pdSolver;

	bool m_ownManifold;
	PersistentManifold* m_manifoldPtr;
	bool m_lowLevelOfDetail;

public:
	Convex2dConvex2dAlgorithm(PersistentManifold* mf, const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* pdSolver, i32 numPerturbationIterations, i32 minimumPointsPerturbationThreshold);

	virtual ~Convex2dConvex2dAlgorithm();

	virtual void processCollision(const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual Scalar calculateTimeOfImpact(CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut);

	virtual void getAllContactManifolds(ManifoldArray& manifoldArray)
	{
		///should we use m_ownManifold to avoid adding duplicates?
		if (m_manifoldPtr && m_ownManifold)
			manifoldArray.push_back(m_manifoldPtr);
	}

	void setLowLevelOfDetail(bool useLowLevel);

	const PersistentManifold* getManifold()
	{
		return m_manifoldPtr;
	}

	struct CreateFunc : public CollisionAlgorithmCreateFunc
	{
		ConvexPenetrationDepthSolver* m_pdSolver;
		SimplexSolverInterface* m_simplexSolver;
		i32 m_numPerturbationIterations;
		i32 m_minimumPointsPerturbationThreshold;

		CreateFunc(SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* pdSolver);

		virtual ~CreateFunc();

		virtual CollisionAlgorithm* CreateCollisionAlgorithm(CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap)
		{
			uk mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(Convex2dConvex2dAlgorithm));
			return new (mem) Convex2dConvex2dAlgorithm(ci.m_manifold, ci, body0Wrap, body1Wrap, m_simplexSolver, m_pdSolver, m_numPerturbationIterations, m_minimumPointsPerturbationThreshold);
		}
	};
};

#endif  //DRX3D_CONVEX_2D_CONVEX_2D_ALGORITHM_H
