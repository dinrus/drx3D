
#ifndef DRX3D_GJK_PAIR_DETECTOR_H
#define DRX3D_GJK_PAIR_DETECTOR_H

#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>

class ConvexShape;
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
class ConvexPenetrationDepthSolver;

/// GjkPairDetector uses GJK to implement the DiscreteCollisionDetectorInterface
class GjkPairDetector : public DiscreteCollisionDetectorInterface
{
	Vec3 m_cachedSeparatingAxis;
	ConvexPenetrationDepthSolver* m_penetrationDepthSolver;
	SimplexSolverInterface* m_simplexSolver;
	const ConvexShape* m_minkowskiA;
	const ConvexShape* m_minkowskiB;
	i32 m_shapeTypeA;
	i32 m_shapeTypeB;
	Scalar m_marginA;
	Scalar m_marginB;

	bool m_ignoreMargin;
	Scalar m_cachedSeparatingDistance;

public:
	//some debugging to fix degeneracy problems
	i32 m_lastUsedMethod;
	i32 m_curIter;
	i32 m_degenerateSimplex;
	i32 m_catchDegeneracies;
	i32 m_fixContactNormalDirection;

	GjkPairDetector(const ConvexShape* objectA, const ConvexShape* objectB, SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* penetrationDepthSolver);
	GjkPairDetector(const ConvexShape* objectA, const ConvexShape* objectB, i32 shapeTypeA, i32 shapeTypeB, Scalar marginA, Scalar marginB, SimplexSolverInterface* simplexSolver, ConvexPenetrationDepthSolver* penetrationDepthSolver);
	virtual ~GjkPairDetector(){};

	virtual void getClosestPoints(const ClosestPointInput& input, Result& output, class IDebugDraw* debugDraw, bool swapResults = false);

	void getClosestPointsNonVirtual(const ClosestPointInput& input, Result& output, class IDebugDraw* debugDraw);

	void setMinkowskiA(const ConvexShape* minkA)
	{
		m_minkowskiA = minkA;
	}

	void setMinkowskiB(const ConvexShape* minkB)
	{
		m_minkowskiB = minkB;
	}
	void setCachedSeparatingAxis(const Vec3& separatingAxis)
	{
		m_cachedSeparatingAxis = separatingAxis;
	}

	const Vec3& getCachedSeparatingAxis() const
	{
		return m_cachedSeparatingAxis;
	}
	Scalar getCachedSeparatingDistance() const
	{
		return m_cachedSeparatingDistance;
	}

	void setPenetrationDepthSolver(ConvexPenetrationDepthSolver* penetrationDepthSolver)
	{
		m_penetrationDepthSolver = penetrationDepthSolver;
	}

	///don't use setIgnoreMargin, it's for drx3D's internal use
	void setIgnoreMargin(bool ignoreMargin)
	{
		m_ignoreMargin = ignoreMargin;
	}
};

#endif  //DRX3D_GJK_PAIR_DETECTOR_H
