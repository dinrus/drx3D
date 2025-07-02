#ifndef DRX3D_CONVEX_CAST_H
#define DRX3D_CONVEX_CAST_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Scalar.h>
class MinkowskiSumShape;
#include <drx3D/Maths/Linear/IDebugDraw.h>

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define MAX_CONVEX_CAST_ITERATIONS 64
#define MAX_CONVEX_CAST_EPSILON (SIMD_EPSILON * 10)
#else
#define MAX_CONVEX_CAST_ITERATIONS 32
#define MAX_CONVEX_CAST_EPSILON Scalar(0.0001)
#endif
///Typically the conservative advancement reaches solution in a few iterations, clip it to 32 for degenerate cases.
///See discussion about this here https://bulletphysics.orgphpBB2/viewtopic.php?t=565
//will need to digg deeper to make the algorithm more robust
//since, a large epsilon can cause an early termination with false
//positive results (ray intersections that shouldn't be there)

/// ConvexCast is an interface for Casting
class ConvexCast
{
public:
	virtual ~ConvexCast();

	///RayResult stores the closest result
	/// alternatively, add a callback method to decide about closest/all results
	struct CastResult
	{
		//virtual bool	addRayResult(const Vec3& normal,Scalar	fraction) = 0;

		virtual void DebugDraw(Scalar fraction) { (void)fraction; }
		virtual void drawCoordSystem(const Transform2& trans) { (void)trans; }
		virtual void reportFailure(i32 errNo, i32 numIterations)
		{
			(void)errNo;
			(void)numIterations;
		}
		CastResult()
			: m_fraction(Scalar(DRX3D_LARGE_FLOAT)),
			  m_debugDrawer(0),
			  m_allowedPenetration(Scalar(0)),
			  m_subSimplexCastMaxIterations(MAX_CONVEX_CAST_ITERATIONS),
			  m_subSimplexCastEpsilon(MAX_CONVEX_CAST_EPSILON)
		{
		}

		virtual ~CastResult(){};

		Transform2 m_hitTransform2A;
		Transform2 m_hitTransform2B;
		Vec3 m_normal;
		Vec3 m_hitPoint;
		Scalar m_fraction;  //input and output
		IDebugDraw* m_debugDrawer;
		Scalar m_allowedPenetration;
		
		i32 m_subSimplexCastMaxIterations;
		Scalar m_subSimplexCastEpsilon;

	};

	/// cast a convex against another convex object
	virtual bool calcTimeOfImpact(
		const Transform2& fromA,
		const Transform2& toA,
		const Transform2& fromB,
		const Transform2& toB,
		CastResult& result) = 0;
};

#endif  //DRX3D_CONVEX_CAST_H
