
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>

#include <drx3D/Physics/Collision/Shapes/MinkowskiSumShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
#include <drx3D/Physics/Collision/NarrowPhase/PointCollector.h>
#include <drx3D/Maths/Linear/Transform2Util.h>

SubsimplexConvexCast::SubsimplexConvexCast(const ConvexShape* convexA, const ConvexShape* convexB, SimplexSolverInterface* simplexSolver)
	: m_simplexSolver(simplexSolver),
	  m_convexA(convexA),
	  m_convexB(convexB)
{
}


bool SubsimplexConvexCast::calcTimeOfImpact(
	const Transform2& fromA,
	const Transform2& toA,
	const Transform2& fromB,
	const Transform2& toB,
	CastResult& result)
{
	m_simplexSolver->reset();

	Vec3 linVelA, linVelB;
	linVelA = toA.getOrigin() - fromA.getOrigin();
	linVelB = toB.getOrigin() - fromB.getOrigin();

	Scalar lambda = Scalar(0.);

	Transform2 interpolatedTransA = fromA;
	Transform2 interpolatedTransB = fromB;

	///take relative motion
	Vec3 r = (linVelA - linVelB);
	Vec3 v;

	Vec3 supVertexA = fromA(m_convexA->localGetSupportingVertex(-r * fromA.getBasis()));
	Vec3 supVertexB = fromB(m_convexB->localGetSupportingVertex(r * fromB.getBasis()));
	v = supVertexA - supVertexB;
	i32 maxIter = result.m_subSimplexCastMaxIterations;

	Vec3 n;
	n.setVal(Scalar(0.), Scalar(0.), Scalar(0.));

	Vec3 c;

	Scalar dist2 = v.length2();



	Vec3 w, p;
	Scalar VdotR;

	while ((dist2 > result.m_subSimplexCastEpsilon) && maxIter--)
	{
		supVertexA = interpolatedTransA(m_convexA->localGetSupportingVertex(-v * interpolatedTransA.getBasis()));
		supVertexB = interpolatedTransB(m_convexB->localGetSupportingVertex(v * interpolatedTransB.getBasis()));
		w = supVertexA - supVertexB;

		Scalar VdotW = v.dot(w);

		if (lambda > Scalar(1.0))
		{
			return false;
		}

		if (VdotW > Scalar(0.))
		{
			VdotR = v.dot(r);

			if (VdotR >= -(SIMD_EPSILON * SIMD_EPSILON))
				return false;
			else
			{
				lambda = lambda - VdotW / VdotR;
				//interpolate to next lambda
				//	x = s + lambda * r;
				interpolatedTransA.getOrigin().setInterpolate3(fromA.getOrigin(), toA.getOrigin(), lambda);
				interpolatedTransB.getOrigin().setInterpolate3(fromB.getOrigin(), toB.getOrigin(), lambda);
				//m_simplexSolver->reset();
				//check next line
				w = supVertexA - supVertexB;

				n = v;
			}
		}
		///Just like regular GJK only add the vertex if it isn't already (close) to current vertex, it would lead to divisions by zero and NaN etc.
		if (!m_simplexSolver->inSimplex(w))
			m_simplexSolver->addVertex(w, supVertexA, supVertexB);

		if (m_simplexSolver->closest(v))
		{
			dist2 = v.length2();

			//todo: check this normal for validity
			//n=v;
			//printf("V=%f , %f, %f\n",v[0],v[1],v[2]);
			//printf("DIST2=%f\n",dist2);
			//printf("numverts = %i\n",m_simplexSolver->numVertices());
		}
		else
		{
			dist2 = Scalar(0.);
		}
	}

	//i32 numiter = MAX_ITERATIONS - maxIter;
	//	printf("number of iterations: %d", numiter);

	//don't report a time of impact when moving 'away' from the hitnormal

	result.m_fraction = lambda;
	if (n.length2() >= (SIMD_EPSILON * SIMD_EPSILON))
		result.m_normal = n.normalized();
	else
		result.m_normal = Vec3(Scalar(0.0), Scalar(0.0), Scalar(0.0));

	//don't report time of impact for motion away from the contact normal (or causes minor penetration)
	if (result.m_normal.dot(r) >= -result.m_allowedPenetration)
		return false;

	Vec3 hitA, hitB;
	m_simplexSolver->compute_points(hitA, hitB);
	result.m_hitPoint = hitB;
	return true;
}
