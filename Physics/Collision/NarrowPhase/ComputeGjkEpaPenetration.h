#ifndef DRX3D_GJK_EPA_PENETATION_CONVEX_COLLISION_H
#define DRX3D_GJK_EPA_PENETATION_CONVEX_COLLISION_H

#include <drx3D/Maths/Linear/Transform2.h>  // Note that Vec3 might be double precision...
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa3.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkCollisionDescription.h>
#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>

template <typename ConvexTemplate>
bool GjkEpaCalcPenDepth(const ConvexTemplate& a, const ConvexTemplate& b,
						  const GjkCollisionDescription& colDesc,
						  Vec3& v, Vec3& wWitnessOnA, Vec3& wWitnessOnB)
{
	(void)v;

	//	const Scalar				radialmargin(Scalar(0.));

	Vec3 guessVector(b.getWorldTransform().getOrigin() - a.getWorldTransform().getOrigin());  //?? why not use the GJK input?

	GjkEpaSolver3::sResults results;

	if (GjkEpaSolver3_Penetration(a, b, guessVector, results))

	{
		//	debugDraw->drawLine(results.witnesses[1],results.witnesses[1]+results.normal,Vec3(255,0,0));
		//resultOut->addContactPoint(results.normal,results.witnesses[1],-results.depth);
		wWitnessOnA = results.witnesses[0];
		wWitnessOnB = results.witnesses[1];
		v = results.normal;
		return true;
	}
	else
	{
		if (GjkEpaSolver3_Distance(a, b, guessVector, results))
		{
			wWitnessOnA = results.witnesses[0];
			wWitnessOnB = results.witnesses[1];
			v = results.normal;
			return false;
		}
	}
	return false;
}

template <typename ConvexTemplate, typename GjkDistanceTemplate>
i32 ComputeGjkEpaPenetration(const ConvexTemplate& a, const ConvexTemplate& b, const GjkCollisionDescription& colDesc, VoronoiSimplexSolver& simplexSolver, GjkDistanceTemplate* distInfo)
{
	bool m_catchDegeneracies = true;
	Scalar m_cachedSeparatingDistance = 0.f;

	Scalar distance = Scalar(0.);
	Vec3 normalInB(Scalar(0.), Scalar(0.), Scalar(0.));

	Vec3 pointOnA, pointOnB;
	Transform2 localTransA = a.getWorldTransform();
	Transform2 localTransB = b.getWorldTransform();

	Scalar marginA = a.getMargin();
	Scalar marginB = b.getMargin();

	i32 m_curIter = 0;
	i32 gGjkMaxIter = colDesc.m_maxGjkIterations;  //this is to catch invalid input, perhaps check for #NaN?
	Vec3 m_cachedSeparatingAxis = colDesc.m_firstDir;

	bool isValid = false;
	bool checkSimplex = false;
	bool checkPenetration = true;
	i32 m_degenerateSimplex = 0;

	i32 m_lastUsedMethod = -1;

	{
		Scalar squaredDistance = DRX3D_LARGE_FLOAT;
		Scalar delta = Scalar(0.);

		Scalar margin = marginA + marginB;

		simplexSolver.reset();

		for (;;)
		//while (true)
		{
			Vec3 separatingAxisInA = (-m_cachedSeparatingAxis) * localTransA.getBasis();
			Vec3 separatingAxisInB = m_cachedSeparatingAxis * localTransB.getBasis();

			Vec3 pInA = a.getLocalSupportWithoutMargin(separatingAxisInA);
			Vec3 qInB = b.getLocalSupportWithoutMargin(separatingAxisInB);

			Vec3 pWorld = localTransA(pInA);
			Vec3 qWorld = localTransB(qInB);

			Vec3 w = pWorld - qWorld;
			delta = m_cachedSeparatingAxis.dot(w);

			// potential exit, they don't overlap
			if ((delta > Scalar(0.0)) && (delta * delta > squaredDistance * colDesc.m_maximumDistanceSquared))
			{
				m_degenerateSimplex = 10;
				checkSimplex = true;
				//checkPenetration = false;
				break;
			}

			//exit 0: the new point is already in the simplex, or we didn't come any closer
			if (simplexSolver.inSimplex(w))
			{
				m_degenerateSimplex = 1;
				checkSimplex = true;
				break;
			}
			// are we getting any closer ?
			Scalar f0 = squaredDistance - delta;
			Scalar f1 = squaredDistance * colDesc.m_gjkRelError2;

			if (f0 <= f1)
			{
				if (f0 <= Scalar(0.))
				{
					m_degenerateSimplex = 2;
				}
				else
				{
					m_degenerateSimplex = 11;
				}
				checkSimplex = true;
				break;
			}

			//add current vertex to simplex
			simplexSolver.addVertex(w, pWorld, qWorld);
			Vec3 newCachedSeparatingAxis;

			//calculate the closest point to the origin (update vector v)
			if (!simplexSolver.closest(newCachedSeparatingAxis))
			{
				m_degenerateSimplex = 3;
				checkSimplex = true;
				break;
			}

			if (newCachedSeparatingAxis.length2() < colDesc.m_gjkRelError2)
			{
				m_cachedSeparatingAxis = newCachedSeparatingAxis;
				m_degenerateSimplex = 6;
				checkSimplex = true;
				break;
			}

			Scalar previousSquaredDistance = squaredDistance;
			squaredDistance = newCachedSeparatingAxis.length2();
#if 0
            ///warning: this termination condition leads to some problems in 2d test case see drx3D/Demos/Box2dDemo
            if (squaredDistance>previousSquaredDistance)
            {
                m_degenerateSimplex = 7;
                squaredDistance = previousSquaredDistance;
                checkSimplex = false;
                break;
            }
#endif  //

			//redundant m_simplexSolver->compute_points(pointOnA, pointOnB);

			//are we getting any closer ?
			if (previousSquaredDistance - squaredDistance <= SIMD_EPSILON * previousSquaredDistance)
			{
				//				m_simplexSolver->backup_closest(m_cachedSeparatingAxis);
				checkSimplex = true;
				m_degenerateSimplex = 12;

				break;
			}

			m_cachedSeparatingAxis = newCachedSeparatingAxis;

			//degeneracy, this is typically due to invalid/uninitialized worldtransforms for a CollisionObject2
			if (m_curIter++ > gGjkMaxIter)
			{
#if defined(DEBUG) || defined(_DEBUG)

				printf("GjkPairDetector maxIter exceeded:%i\n", m_curIter);
				printf("sepAxis=(%f,%f,%f), squaredDistance = %f\n",
					   m_cachedSeparatingAxis.getX(),
					   m_cachedSeparatingAxis.getY(),
					   m_cachedSeparatingAxis.getZ(),
					   squaredDistance);
#endif

				break;
			}

			bool check = (!simplexSolver.fullSimplex());
			//bool check = (!m_simplexSolver->fullSimplex() && squaredDistance > SIMD_EPSILON * m_simplexSolver->maxVertex());

			if (!check)
			{
				//do we need this backup_closest here ?
				//				m_simplexSolver->backup_closest(m_cachedSeparatingAxis);
				m_degenerateSimplex = 13;
				break;
			}
		}

		if (checkSimplex)
		{
			simplexSolver.compute_points(pointOnA, pointOnB);
			normalInB = m_cachedSeparatingAxis;

			Scalar lenSqr = m_cachedSeparatingAxis.length2();

			//valid normal
			if (lenSqr < 0.0001)
			{
				m_degenerateSimplex = 5;
			}
			if (lenSqr > SIMD_EPSILON * SIMD_EPSILON)
			{
				Scalar rlen = Scalar(1.) / Sqrt(lenSqr);
				normalInB *= rlen;  //normalize

				Scalar s = Sqrt(squaredDistance);

				Assert(s > Scalar(0.0));
				pointOnA -= m_cachedSeparatingAxis * (marginA / s);
				pointOnB += m_cachedSeparatingAxis * (marginB / s);
				distance = ((Scalar(1.) / rlen) - margin);
				isValid = true;

				m_lastUsedMethod = 1;
			}
			else
			{
				m_lastUsedMethod = 2;
			}
		}

		bool catchDegeneratePenetrationCase =
			(m_catchDegeneracies && m_degenerateSimplex && ((distance + margin) < 0.01));

		//if (checkPenetration && !isValid)
		if (checkPenetration && (!isValid || catchDegeneratePenetrationCase))
		{
			//penetration case

			//if there is no way to handle penetrations, bail out

			// Penetration depth case.
			Vec3 tmpPointOnA, tmpPointOnB;

			m_cachedSeparatingAxis.setZero();

			bool isValid2 = GjkEpaCalcPenDepth(a, b,
												 colDesc,
												 m_cachedSeparatingAxis, tmpPointOnA, tmpPointOnB);

			if (isValid2)
			{
				Vec3 tmpNormalInB = tmpPointOnB - tmpPointOnA;
				Scalar lenSqr = tmpNormalInB.length2();
				if (lenSqr <= (SIMD_EPSILON * SIMD_EPSILON))
				{
					tmpNormalInB = m_cachedSeparatingAxis;
					lenSqr = m_cachedSeparatingAxis.length2();
				}

				if (lenSqr > (SIMD_EPSILON * SIMD_EPSILON))
				{
					tmpNormalInB /= Sqrt(lenSqr);
					Scalar distance2 = -(tmpPointOnA - tmpPointOnB).length();
					//only replace valid penetrations when the result is deeper (check)
					if (!isValid || (distance2 < distance))
					{
						distance = distance2;
						pointOnA = tmpPointOnA;
						pointOnB = tmpPointOnB;
						normalInB = tmpNormalInB;

						isValid = true;
						m_lastUsedMethod = 3;
					}
					else
					{
						m_lastUsedMethod = 8;
					}
				}
				else
				{
					m_lastUsedMethod = 9;
				}
			}
			else

			{
				///this is another degenerate case, where the initial GJK calculation reports a degenerate case
				///EPA reports no penetration, and the second GJK (using the supporting vector without margin)
				///reports a valid positive distance. Use the results of the second GJK instead of failing.
				///thanks to Jacob.Langford for the reproduction case
				///http://code.google.com/p/bullet/issues/detail?id=250

				if (m_cachedSeparatingAxis.length2() > Scalar(0.))
				{
					Scalar distance2 = (tmpPointOnA - tmpPointOnB).length() - margin;
					//only replace valid distances when the distance is less
					if (!isValid || (distance2 < distance))
					{
						distance = distance2;
						pointOnA = tmpPointOnA;
						pointOnB = tmpPointOnB;
						pointOnA -= m_cachedSeparatingAxis * marginA;
						pointOnB += m_cachedSeparatingAxis * marginB;
						normalInB = m_cachedSeparatingAxis;
						normalInB.normalize();

						isValid = true;
						m_lastUsedMethod = 6;
					}
					else
					{
						m_lastUsedMethod = 5;
					}
				}
			}
		}
	}

	if (isValid && ((distance < 0) || (distance * distance < colDesc.m_maximumDistanceSquared)))
	{
		m_cachedSeparatingAxis = normalInB;
		m_cachedSeparatingDistance = distance;
		distInfo->m_distance = distance;
		distInfo->m_normalBtoA = normalInB;
		distInfo->m_pointOnB = pointOnB;
		distInfo->m_pointOnA = pointOnB + normalInB * distance;
		return 0;
	}
	return -m_lastUsedMethod;
}

#endif  //DRX3D_GJK_EPA_PENETATION_CONVEX_COLLISION_H
