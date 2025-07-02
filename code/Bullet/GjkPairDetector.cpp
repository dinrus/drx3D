#include <drx3D/Physics/Collision/NarrowPhase/GjkPairDetector.h>
#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>
#include <drx3D/Physics/Collision/NarrowPhase/ConvexPenetrationDepthSolver.h>

#if defined(DEBUG) || defined(_DEBUG)
//#define TEST_NON_VIRTUAL 1
#include <stdio.h>  //for debug printf
#ifdef __SPU__
#include <spu_printf.h>
#define printf spu_printf
#endif  //__SPU__
#endif

//must be above the machine epsilon
#ifdef DRX3D_USE_DOUBLE_PRECISION
#define REL_ERROR2 Scalar(1.0e-12)
Scalar gGjkEpaPenetrationTolerance = 1.0e-12;
#else
#define REL_ERROR2 Scalar(1.0e-6)
Scalar gGjkEpaPenetrationTolerance = 0.001;
#endif


GjkPairDetector::GjkPairDetector(const ConvexShape *objectA, const ConvexShape *objectB, SimplexSolverInterface *simplexSolver, ConvexPenetrationDepthSolver *penetrationDepthSolver)
	: m_cachedSeparatingAxis(Scalar(0.), Scalar(1.), Scalar(0.)),
	  m_penetrationDepthSolver(penetrationDepthSolver),
	  m_simplexSolver(simplexSolver),
	  m_minkowskiA(objectA),
	  m_minkowskiB(objectB),
	  m_shapeTypeA(objectA->getShapeType()),
	  m_shapeTypeB(objectB->getShapeType()),
	  m_marginA(objectA->getMargin()),
	  m_marginB(objectB->getMargin()),
	  m_ignoreMargin(false),
	  m_lastUsedMethod(-1),
	  m_catchDegeneracies(1),
	  m_fixContactNormalDirection(1)
{
}
GjkPairDetector::GjkPairDetector(const ConvexShape *objectA, const ConvexShape *objectB, i32 shapeTypeA, i32 shapeTypeB, Scalar marginA, Scalar marginB, SimplexSolverInterface *simplexSolver, ConvexPenetrationDepthSolver *penetrationDepthSolver)
	: m_cachedSeparatingAxis(Scalar(0.), Scalar(1.), Scalar(0.)),
	  m_penetrationDepthSolver(penetrationDepthSolver),
	  m_simplexSolver(simplexSolver),
	  m_minkowskiA(objectA),
	  m_minkowskiB(objectB),
	  m_shapeTypeA(shapeTypeA),
	  m_shapeTypeB(shapeTypeB),
	  m_marginA(marginA),
	  m_marginB(marginB),
	  m_ignoreMargin(false),
	  m_lastUsedMethod(-1),
	  m_catchDegeneracies(1),
	  m_fixContactNormalDirection(1)
{
}

void GjkPairDetector::getClosestPoints(const ClosestPointInput &input, Result &output, class IDebugDraw *debugDraw, bool swapResults)
{
	(void)swapResults;

	getClosestPointsNonVirtual(input, output, debugDraw);
}

static void ComputeSupport(const ConvexShape *convexA, const Transform2 &localTransA, const ConvexShape *convexB, const Transform2 &localTransB, const Vec3 &dir, bool check2d, Vec3 &supAworld, Vec3 &supBworld, Vec3 &aMinb)
{
	Vec3 separatingAxisInA = (dir)*localTransA.getBasis();
	Vec3 separatingAxisInB = (-dir) * localTransB.getBasis();

	Vec3 pInANoMargin = convexA->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInA);
	Vec3 qInBNoMargin = convexB->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInB);

	Vec3 pInA = pInANoMargin;
	Vec3 qInB = qInBNoMargin;

	supAworld = localTransA(pInA);
	supBworld = localTransB(qInB);

	if (check2d)
	{
		supAworld[2] = 0.f;
		supBworld[2] = 0.f;
	}

	aMinb = supAworld - supBworld;
}

struct SupportVector
{
	Vec3 v;   //!< Support point in minkowski sum
	Vec3 v1;  //!< Support point in obj1
	Vec3 v2;  //!< Support point in obj2
};

struct Simplex
{
	SupportVector ps[4];
	i32 last;  //!< index of last added point
};

static Vec3 ccd_vec3_origin(0, 0, 0);

inline void SimplexInit(Simplex *s)
{
	s->last = -1;
}

inline i32 SimplexSize(const Simplex *s)
{
	return s->last + 1;
}

inline const SupportVector *SimplexPoint(const Simplex *s, i32 idx)
{
	// here is no check on boundaries
	return &s->ps[idx];
}
inline void SupportCopy(SupportVector *d, const SupportVector *s)
{
	*d = *s;
}

inline void Vec3Copy(Vec3 *v, const Vec3 *w)
{
	*v = *w;
}

inline void ccdVec3Add(Vec3 *v, const Vec3 *w)
{
	v->m_floats[0] += w->m_floats[0];
	v->m_floats[1] += w->m_floats[1];
	v->m_floats[2] += w->m_floats[2];
}

inline void ccdVec3Sub(Vec3 *v, const Vec3 *w)
{
	*v -= *w;
}
inline void Vec3Sub2(Vec3 *d, const Vec3 *v, const Vec3 *w)
{
	*d = (*v) - (*w);
}
inline Scalar Vec3Dot(const Vec3 *a, const Vec3 *b)
{
	Scalar dot;
	dot = a->dot(*b);

	return dot;
}

inline Scalar ccdVec3Dist2(const Vec3 *a, const Vec3 *b)
{
	Vec3 ab;
	Vec3Sub2(&ab, a, b);
	return Vec3Dot(&ab, &ab);
}

inline void Vec3Scale(Vec3 *d, Scalar k)
{
	d->m_floats[0] *= k;
	d->m_floats[1] *= k;
	d->m_floats[2] *= k;
}

inline void Vec3Cross(Vec3 *d, const Vec3 *a, const Vec3 *b)
{
	d->m_floats[0] = (a->m_floats[1] * b->m_floats[2]) - (a->m_floats[2] * b->m_floats[1]);
	d->m_floats[1] = (a->m_floats[2] * b->m_floats[0]) - (a->m_floats[0] * b->m_floats[2]);
	d->m_floats[2] = (a->m_floats[0] * b->m_floats[1]) - (a->m_floats[1] * b->m_floats[0]);
}

inline void TripleCross(const Vec3 *a, const Vec3 *b,
						  const Vec3 *c, Vec3 *d)
{
	Vec3 e;
	Vec3Cross(&e, a, b);
	Vec3Cross(d, &e, c);
}

inline i32 ccdEq(Scalar _a, Scalar _b)
{
	Scalar ab;
	Scalar a, b;

	ab = Fabs(_a - _b);
	if (Fabs(ab) < SIMD_EPSILON)
		return 1;

	a = Fabs(_a);
	b = Fabs(_b);
	if (b > a)
	{
		return ab < SIMD_EPSILON * b;
	}
	else
	{
		return ab < SIMD_EPSILON * a;
	}
}

Scalar ccdVec3X(const Vec3 *v)
{
	return v->x();
}

Scalar ccdVec3Y(const Vec3 *v)
{
	return v->y();
}

Scalar ccdVec3Z(const Vec3 *v)
{
	return v->z();
}
inline i32 Vec3Eq(const Vec3 *a, const Vec3 *b)
{
	return ccdEq(ccdVec3X(a), ccdVec3X(b)) && ccdEq(ccdVec3Y(a), ccdVec3Y(b)) && ccdEq(ccdVec3Z(a), ccdVec3Z(b));
}

inline void SimplexAdd(Simplex *s, const SupportVector *v)
{
	// here is no check on boundaries in sake of speed
	++s->last;
	SupportCopy(s->ps + s->last, v);
}

inline void SimplexSet(Simplex *s, size_t pos, const SupportVector *a)
{
	SupportCopy(s->ps + pos, a);
}

inline void SimplexSetSize(Simplex *s, i32 size)
{
	s->last = size - 1;
}

inline const SupportVector *ccdSimplexLast(const Simplex *s)
{
	return SimplexPoint(s, s->last);
}

inline i32 ccdSign(Scalar val)
{
	if (FuzzyZero(val))
	{
		return 0;
	}
	else if (val < Scalar(0))
	{
		return -1;
	}
	return 1;
}

inline Scalar Vec3PointSegmentDist2(const Vec3 *P,
										const Vec3 *x0,
										const Vec3 *b,
										Vec3 *witness)
{
	// The computation comes from solving equation of segment:
	//      S(t) = x0 + t.d
	//          where - x0 is initial point of segment
	//                - d is direction of segment from x0 (|d| > 0)
	//                - t belongs to <0, 1> interval
	//
	// Than, distance from a segment to some point P can be expressed:
	//      D(t) = |x0 + t.d - P|^2
	//          which is distance from any point on segment. Minimization
	//          of this function brings distance from P to segment.
	// Minimization of D(t) leads to simple quadratic equation that's
	// solving is straightforward.
	//
	// Bonus of this method is witness point for free.

	Scalar dist, t;
	Vec3 d, a;

	// direction of segment
	Vec3Sub2(&d, b, x0);

	// precompute vector from P to x0
	Vec3Sub2(&a, x0, P);

	t = -Scalar(1.) * Vec3Dot(&a, &d);
	t /= Vec3Dot(&d, &d);

	if (t < Scalar(0) || FuzzyZero(t))
	{
		dist = ccdVec3Dist2(x0, P);
		if (witness)
			Vec3Copy(witness, x0);
	}
	else if (t > Scalar(1) || ccdEq(t, Scalar(1)))
	{
		dist = ccdVec3Dist2(b, P);
		if (witness)
			Vec3Copy(witness, b);
	}
	else
	{
		if (witness)
		{
			Vec3Copy(witness, &d);
			Vec3Scale(witness, t);
			ccdVec3Add(witness, x0);
			dist = ccdVec3Dist2(witness, P);
		}
		else
		{
			// recycling variables
			Vec3Scale(&d, t);
			ccdVec3Add(&d, &a);
			dist = Vec3Dot(&d, &d);
		}
	}

	return dist;
}

Scalar Vec3PointTriDist2(const Vec3 *P,
							 const Vec3 *x0, const Vec3 *B,
							 const Vec3 *C,
							 Vec3 *witness)
{
	// Computation comes from analytic expression for triangle (x0, B, C)
	//      T(s, t) = x0 + s.d1 + t.d2, where d1 = B - x0 and d2 = C - x0 and
	// Then equation for distance is:
	//      D(s, t) = | T(s, t) - P |^2
	// This leads to minimization of quadratic function of two variables.
	// The solution from is taken only if s is between 0 and 1, t is
	// between 0 and 1 and t + s < 1, otherwise distance from segment is
	// computed.

	Vec3 d1, d2, a;
	double u, v, w, p, q, r;
	double s, t, dist, dist2;
	Vec3 witness2;

	Vec3Sub2(&d1, B, x0);
	Vec3Sub2(&d2, C, x0);
	Vec3Sub2(&a, x0, P);

	u = Vec3Dot(&a, &a);
	v = Vec3Dot(&d1, &d1);
	w = Vec3Dot(&d2, &d2);
	p = Vec3Dot(&a, &d1);
	q = Vec3Dot(&a, &d2);
	r = Vec3Dot(&d1, &d2);

	s = (q * r - w * p) / (w * v - r * r);
	t = (-s * r - q) / w;

	if ((FuzzyZero(s) || s > Scalar(0)) && (ccdEq(s, Scalar(1)) || s < Scalar(1)) && (FuzzyZero(t) || t > Scalar(0)) && (ccdEq(t, Scalar(1)) || t < Scalar(1)) && (ccdEq(t + s, Scalar(1)) || t + s < Scalar(1)))
	{
		if (witness)
		{
			Vec3Scale(&d1, s);
			Vec3Scale(&d2, t);
			Vec3Copy(witness, x0);
			ccdVec3Add(witness, &d1);
			ccdVec3Add(witness, &d2);

			dist = ccdVec3Dist2(witness, P);
		}
		else
		{
			dist = s * s * v;
			dist += t * t * w;
			dist += Scalar(2.) * s * t * r;
			dist += Scalar(2.) * s * p;
			dist += Scalar(2.) * t * q;
			dist += u;
		}
	}
	else
	{
		dist = Vec3PointSegmentDist2(P, x0, B, witness);

		dist2 = Vec3PointSegmentDist2(P, x0, C, &witness2);
		if (dist2 < dist)
		{
			dist = dist2;
			if (witness)
				Vec3Copy(witness, &witness2);
		}

		dist2 = Vec3PointSegmentDist2(P, B, C, &witness2);
		if (dist2 < dist)
		{
			dist = dist2;
			if (witness)
				Vec3Copy(witness, &witness2);
		}
	}

	return dist;
}

static i32 DoSimplex2(Simplex *simplex, Vec3 *dir)
{
	const SupportVector *A, *B;
	Vec3 AB, AO, tmp;
	Scalar dot;

	// get last added as A
	A = ccdSimplexLast(simplex);
	// get the other point
	B = SimplexPoint(simplex, 0);
	// compute AB oriented segment
	Vec3Sub2(&AB, &B->v, &A->v);
	// compute AO vector
	Vec3Copy(&AO, &A->v);
	Vec3Scale(&AO, -Scalar(1));

	// dot product AB . AO
	dot = Vec3Dot(&AB, &AO);

	// check if origin doesn't lie on AB segment
	Vec3Cross(&tmp, &AB, &AO);
	if (FuzzyZero(Vec3Dot(&tmp, &tmp)) && dot > Scalar(0))
	{
		return 1;
	}

	// check if origin is in area where AB segment is
	if (FuzzyZero(dot) || dot < Scalar(0))
	{
		// origin is in outside are of A
		SimplexSet(simplex, 0, A);
		SimplexSetSize(simplex, 1);
		Vec3Copy(dir, &AO);
	}
	else
	{
		// origin is in area where AB segment is

		// keep simplex untouched and set direction to
		// AB x AO x AB
		TripleCross(&AB, &AO, &AB, dir);
	}

	return 0;
}

static i32 DoSimplex3(Simplex *simplex, Vec3 *dir)
{
	const SupportVector *A, *B, *C;
	Vec3 AO, AB, AC, ABC, tmp;
	Scalar dot, dist;

	// get last added as A
	A = ccdSimplexLast(simplex);
	// get the other points
	B = SimplexPoint(simplex, 1);
	C = SimplexPoint(simplex, 0);

	// check touching contact
	dist = Vec3PointTriDist2(&ccd_vec3_origin, &A->v, &B->v, &C->v, 0);
	if (FuzzyZero(dist))
	{
		return 1;
	}

	// check if triangle is really triangle (has area > 0)
	// if not simplex can't be expanded and thus no itersection is found
	if (Vec3Eq(&A->v, &B->v) || Vec3Eq(&A->v, &C->v))
	{
		return -1;
	}

	// compute AO vector
	Vec3Copy(&AO, &A->v);
	Vec3Scale(&AO, -Scalar(1));

	// compute AB and AC segments and ABC vector (perpendircular to triangle)
	Vec3Sub2(&AB, &B->v, &A->v);
	Vec3Sub2(&AC, &C->v, &A->v);
	Vec3Cross(&ABC, &AB, &AC);

	Vec3Cross(&tmp, &ABC, &AC);
	dot = Vec3Dot(&tmp, &AO);
	if (FuzzyZero(dot) || dot > Scalar(0))
	{
		dot = Vec3Dot(&AC, &AO);
		if (FuzzyZero(dot) || dot > Scalar(0))
		{
			// C is already in place
			SimplexSet(simplex, 1, A);
			SimplexSetSize(simplex, 2);
			TripleCross(&AC, &AO, &AC, dir);
		}
		else
		{
			dot = Vec3Dot(&AB, &AO);
			if (FuzzyZero(dot) || dot > Scalar(0))
			{
				SimplexSet(simplex, 0, B);
				SimplexSet(simplex, 1, A);
				SimplexSetSize(simplex, 2);
				TripleCross(&AB, &AO, &AB, dir);
			}
			else
			{
				SimplexSet(simplex, 0, A);
				SimplexSetSize(simplex, 1);
				Vec3Copy(dir, &AO);
			}
		}
	}
	else
	{
		Vec3Cross(&tmp, &AB, &ABC);
		dot = Vec3Dot(&tmp, &AO);
		if (FuzzyZero(dot) || dot > Scalar(0))
		{
			dot = Vec3Dot(&AB, &AO);
			if (FuzzyZero(dot) || dot > Scalar(0))
			{
				SimplexSet(simplex, 0, B);
				SimplexSet(simplex, 1, A);
				SimplexSetSize(simplex, 2);
				TripleCross(&AB, &AO, &AB, dir);
			}
			else
			{
				SimplexSet(simplex, 0, A);
				SimplexSetSize(simplex, 1);
				Vec3Copy(dir, &AO);
			}
		}
		else
		{
			dot = Vec3Dot(&ABC, &AO);
			if (FuzzyZero(dot) || dot > Scalar(0))
			{
				Vec3Copy(dir, &ABC);
			}
			else
			{
				SupportVector tmp;
				SupportCopy(&tmp, C);
				SimplexSet(simplex, 0, B);
				SimplexSet(simplex, 1, &tmp);

				Vec3Copy(dir, &ABC);
				Vec3Scale(dir, -Scalar(1));
			}
		}
	}

	return 0;
}

static i32 DoSimplex4(Simplex *simplex, Vec3 *dir)
{
	const SupportVector *A, *B, *C, *D;
	Vec3 AO, AB, AC, AD, ABC, ACD, ADB;
	i32 B_on_ACD, C_on_ADB, D_on_ABC;
	i32 AB_O, AC_O, AD_O;
	Scalar dist;

	// get last added as A
	A = ccdSimplexLast(simplex);
	// get the other points
	B = SimplexPoint(simplex, 2);
	C = SimplexPoint(simplex, 1);
	D = SimplexPoint(simplex, 0);

	// check if tetrahedron is really tetrahedron (has volume > 0)
	// if it is not simplex can't be expanded and thus no intersection is
	// found
	dist = Vec3PointTriDist2(&A->v, &B->v, &C->v, &D->v, 0);
	if (FuzzyZero(dist))
	{
		return -1;
	}

	// check if origin lies on some of tetrahedron's face - if so objects
	// intersect
	dist = Vec3PointTriDist2(&ccd_vec3_origin, &A->v, &B->v, &C->v, 0);
	if (FuzzyZero(dist))
		return 1;
	dist = Vec3PointTriDist2(&ccd_vec3_origin, &A->v, &C->v, &D->v, 0);
	if (FuzzyZero(dist))
		return 1;
	dist = Vec3PointTriDist2(&ccd_vec3_origin, &A->v, &B->v, &D->v, 0);
	if (FuzzyZero(dist))
		return 1;
	dist = Vec3PointTriDist2(&ccd_vec3_origin, &B->v, &C->v, &D->v, 0);
	if (FuzzyZero(dist))
		return 1;

	// compute AO, AB, AC, AD segments and ABC, ACD, ADB normal vectors
	Vec3Copy(&AO, &A->v);
	Vec3Scale(&AO, -Scalar(1));
	Vec3Sub2(&AB, &B->v, &A->v);
	Vec3Sub2(&AC, &C->v, &A->v);
	Vec3Sub2(&AD, &D->v, &A->v);
	Vec3Cross(&ABC, &AB, &AC);
	Vec3Cross(&ACD, &AC, &AD);
	Vec3Cross(&ADB, &AD, &AB);

	// side (positive or negative) of B, C, D relative to planes ACD, ADB
	// and ABC respectively
	B_on_ACD = ccdSign(Vec3Dot(&ACD, &AB));
	C_on_ADB = ccdSign(Vec3Dot(&ADB, &AC));
	D_on_ABC = ccdSign(Vec3Dot(&ABC, &AD));

	// whether origin is on same side of ACD, ADB, ABC as B, C, D
	// respectively
	AB_O = ccdSign(Vec3Dot(&ACD, &AO)) == B_on_ACD;
	AC_O = ccdSign(Vec3Dot(&ADB, &AO)) == C_on_ADB;
	AD_O = ccdSign(Vec3Dot(&ABC, &AO)) == D_on_ABC;

	if (AB_O && AC_O && AD_O)
	{
		// origin is in tetrahedron
		return 1;
		// rearrange simplex to triangle and call DoSimplex3()
	}
	else if (!AB_O)
	{
		// B is farthest from the origin among all of the tetrahedron's
		// points, so remove it from the list and go on with the triangle
		// case

		// D and C are in place
		SimplexSet(simplex, 2, A);
		SimplexSetSize(simplex, 3);
	}
	else if (!AC_O)
	{
		// C is farthest
		SimplexSet(simplex, 1, D);
		SimplexSet(simplex, 0, B);
		SimplexSet(simplex, 2, A);
		SimplexSetSize(simplex, 3);
	}
	else
	{  // (!AD_O)
		SimplexSet(simplex, 0, C);
		SimplexSet(simplex, 1, B);
		SimplexSet(simplex, 2, A);
		SimplexSetSize(simplex, 3);
	}

	return DoSimplex3(simplex, dir);
}

static i32 DoSimplex(Simplex *simplex, Vec3 *dir)
{
	if (SimplexSize(simplex) == 2)
	{
		// simplex contains segment only one segment
		return DoSimplex2(simplex, dir);
	}
	else if (SimplexSize(simplex) == 3)
	{
		// simplex contains triangle
		return DoSimplex3(simplex, dir);
	}
	else
	{  // SimplexSize(simplex) == 4
		// tetrahedron - this is the only shape which can encapsule origin
		// so DoSimplex4() also contains test on it
		return DoSimplex4(simplex, dir);
	}
}

#ifdef __SPU__
void GjkPairDetector::getClosestPointsNonVirtual(const ClosestPointInput &input, Result &output, class IDebugDraw *debugDraw)
#else
void GjkPairDetector::getClosestPointsNonVirtual(const ClosestPointInput &input, Result &output, class IDebugDraw *debugDraw)
#endif
{
	m_cachedSeparatingDistance = 0.f;

	Scalar distance = Scalar(0.);
	Vec3 normalInB(Scalar(0.), Scalar(0.), Scalar(0.));

	Vec3 pointOnA, pointOnB;
	Transform2 localTransA = input.m_transformA;
	Transform2 localTransB = input.m_transformB;
	Vec3 positionOffset = (localTransA.getOrigin() + localTransB.getOrigin()) * Scalar(0.5);
	localTransA.getOrigin() -= positionOffset;
	localTransB.getOrigin() -= positionOffset;

	bool check2d = m_minkowskiA->isConvex2d() && m_minkowskiB->isConvex2d();

	Scalar marginA = m_marginA;
	Scalar marginB = m_marginB;


	//for CCD we don't use margins
	if (m_ignoreMargin)
	{
		marginA = Scalar(0.);
		marginB = Scalar(0.);
	}

	m_curIter = 0;
	i32 gGjkMaxIter = 1000;  //this is to catch invalid input, perhaps check for #NaN?
	m_cachedSeparatingAxis.setVal(0, 1, 0);

	bool isValid = false;
	bool checkSimplex = false;
	bool checkPenetration = true;
	m_degenerateSimplex = 0;

	m_lastUsedMethod = -1;
	i32 status = -2;
	Vec3 orgNormalInB(0, 0, 0);
	Scalar margin = marginA + marginB;

	//we add a separate implementation to check if the convex shapes intersect
	//See also "Real-time Collision Detection with Implicit Objects" by Leif Olvang
	//Todo: integrate the simplex penetration check directly inside the drx3D VoronoiSimplexSolver
	//and remove this temporary code from libCCD
	//this fixes issue https://github.com/bulletphysics/bullet3/issues/1703
	//note, for large differences in shapes, use double precision build!
	{
		Scalar squaredDistance = DRX3D_LARGE_FLOAT;
		Scalar delta = Scalar(0.);

		Simplex simplex1;
		Simplex *simplex = &simplex1;
		SimplexInit(simplex);

		Vec3 dir(1, 0, 0);

		{
			Vec3 lastSupV;
			Vec3 supAworld;
			Vec3 supBworld;
			ComputeSupport(m_minkowskiA, localTransA, m_minkowskiB, localTransB, dir, check2d, supAworld, supBworld, lastSupV);

			SupportVector last;
			last.v = lastSupV;
			last.v1 = supAworld;
			last.v2 = supBworld;

			SimplexAdd(simplex, &last);

			dir = -lastSupV;

			// start iterations
			for (i32 iterations = 0; iterations < gGjkMaxIter; iterations++)
			{
				// obtain support point
				ComputeSupport(m_minkowskiA, localTransA, m_minkowskiB, localTransB, dir, check2d, supAworld, supBworld, lastSupV);

				// check if farthest point in Minkowski difference in direction dir
				// isn't somewhere before origin (the test on negative dot product)
				// - because if it is, objects are not intersecting at all.
				Scalar delta = lastSupV.dot(dir);
				if (delta < 0)
				{
					//no intersection, besides margin
					status = -1;
					break;
				}

				// add last support vector to simplex
				last.v = lastSupV;
				last.v1 = supAworld;
				last.v2 = supBworld;

				SimplexAdd(simplex, &last);

				// if DoSimplex returns 1 if objects intersect, -1 if objects don't
				// intersect and 0 if algorithm should continue

				Vec3 newDir;
				i32 do_simplex_res = DoSimplex(simplex, &dir);

				if (do_simplex_res == 1)
				{
					status = 0;  // intersection found
					break;
				}
				else if (do_simplex_res == -1)
				{
					// intersection not found
					status = -1;
					break;
				}

				if (FuzzyZero(Vec3Dot(&dir, &dir)))
				{
					// intersection not found
					status = -1;
				}

				if (dir.length2() < SIMD_EPSILON)
				{
					//no intersection, besides margin
					status = -1;
					break;
				}

				if (dir.fuzzyZero())
				{
					// intersection not found
					status = -1;
					break;
				}
			}
		}

		m_simplexSolver->reset();
		if (status == 0)
		{
			//status = 0;
			//printf("Intersect!\n");
		}

		if (status == -1)
		{
			//printf("not intersect\n");
		}
		//printf("dir=%f,%f,%f\n",dir[0],dir[1],dir[2]);
		if (1)
		{
			for (;;)
			//while (true)
			{
				Vec3 separatingAxisInA = (-m_cachedSeparatingAxis) * localTransA.getBasis();
				Vec3 separatingAxisInB = m_cachedSeparatingAxis * localTransB.getBasis();

				Vec3 pInA = m_minkowskiA->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInA);
				Vec3 qInB = m_minkowskiB->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInB);

				Vec3 pWorld = localTransA(pInA);
				Vec3 qWorld = localTransB(qInB);

				if (check2d)
				{
					pWorld[2] = 0.f;
					qWorld[2] = 0.f;
				}

				Vec3 w = pWorld - qWorld;
				delta = m_cachedSeparatingAxis.dot(w);

				// potential exit, they don't overlap
				if ((delta > Scalar(0.0)) && (delta * delta > squaredDistance * input.m_maximumDistanceSquared))
				{
					m_degenerateSimplex = 10;
					checkSimplex = true;
					//checkPenetration = false;
					break;
				}

				//exit 0: the new point is already in the simplex, or we didn't come any closer
				if (m_simplexSolver->inSimplex(w))
				{
					m_degenerateSimplex = 1;
					checkSimplex = true;
					break;
				}
				// are we getting any closer ?
				Scalar f0 = squaredDistance - delta;
				Scalar f1 = squaredDistance * REL_ERROR2;

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
				m_simplexSolver->addVertex(w, pWorld, qWorld);
				Vec3 newCachedSeparatingAxis;

				//calculate the closest point to the origin (update vector v)
				if (!m_simplexSolver->closest(newCachedSeparatingAxis))
				{
					m_degenerateSimplex = 3;
					checkSimplex = true;
					break;
				}

				if (newCachedSeparatingAxis.length2() < REL_ERROR2)
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
				if (squaredDistance > previousSquaredDistance)
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
					printf("sepAxis=(%f,%f,%f), squaredDistance = %f, shapeTypeA=%i,shapeTypeB=%i\n",
						   m_cachedSeparatingAxis.getX(),
						   m_cachedSeparatingAxis.getY(),
						   m_cachedSeparatingAxis.getZ(),
						   squaredDistance,
						   m_minkowskiA->getShapeType(),
						   m_minkowskiB->getShapeType());

#endif
					break;
				}

				bool check = (!m_simplexSolver->fullSimplex());
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
				m_simplexSolver->compute_points(pointOnA, pointOnB);
				normalInB = m_cachedSeparatingAxis;

				Scalar lenSqr = m_cachedSeparatingAxis.length2();

				//valid normal
				if (lenSqr < REL_ERROR2)
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
					orgNormalInB = normalInB;

					m_lastUsedMethod = 1;
				}
				else
				{
					m_lastUsedMethod = 2;
				}
			}
		}

		bool catchDegeneratePenetrationCase =
			(m_catchDegeneracies && m_penetrationDepthSolver && m_degenerateSimplex && ((distance + margin) < gGjkEpaPenetrationTolerance));

		//if (checkPenetration && !isValid)
		if ((checkPenetration && (!isValid || catchDegeneratePenetrationCase)) || (status == 0))
		{
			//penetration case

			//if there is no way to handle penetrations, bail out
			if (m_penetrationDepthSolver)
			{
				// Penetration depth case.
				Vec3 tmpPointOnA, tmpPointOnB;

				m_cachedSeparatingAxis.setZero();

				bool isValid2 = m_penetrationDepthSolver->calcPenDepth(
					*m_simplexSolver,
					m_minkowskiA, m_minkowskiB,
					localTransA, localTransB,
					m_cachedSeparatingAxis, tmpPointOnA, tmpPointOnB,
					debugDraw);

				if (m_cachedSeparatingAxis.length2())
				{
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
							m_lastUsedMethod = 3;
							//only replace valid penetrations when the result is deeper (check)
							if (!isValid || (distance2 < distance))
							{
								distance = distance2;
								pointOnA = tmpPointOnA;
								pointOnB = tmpPointOnB;
								normalInB = tmpNormalInB;
								isValid = true;
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
				else
				{
					//printf("EPA didn't return a valid value\n");
				}
			}
		}
	}

	if (isValid && ((distance < 0) || (distance * distance < input.m_maximumDistanceSquared)))
	{
		m_cachedSeparatingAxis = normalInB;
		m_cachedSeparatingDistance = distance;
		if (1)
		{
			///todo: need to track down this EPA penetration solver degeneracy
			///the penetration solver reports penetration but the contact normal
			///connecting the contact points is pointing in the opposite direction
			///until then, detect the issue and revert the normal

			Scalar d2 = 0.f;
			{
				Vec3 separatingAxisInA = (-orgNormalInB) * localTransA.getBasis();
				Vec3 separatingAxisInB = orgNormalInB * localTransB.getBasis();

				Vec3 pInA = m_minkowskiA->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInA);
				Vec3 qInB = m_minkowskiB->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInB);

				Vec3 pWorld = localTransA(pInA);
				Vec3 qWorld = localTransB(qInB);
				Vec3 w = pWorld - qWorld;
				d2 = orgNormalInB.dot(w) - margin;
			}

			Scalar d1 = 0;
			{
				Vec3 separatingAxisInA = (normalInB)*localTransA.getBasis();
				Vec3 separatingAxisInB = -normalInB * localTransB.getBasis();

				Vec3 pInA = m_minkowskiA->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInA);
				Vec3 qInB = m_minkowskiB->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInB);

				Vec3 pWorld = localTransA(pInA);
				Vec3 qWorld = localTransB(qInB);
				Vec3 w = pWorld - qWorld;
				d1 = (-normalInB).dot(w) - margin;
			}
			Scalar d0 = 0.f;
			{
				Vec3 separatingAxisInA = (-normalInB) * input.m_transformA.getBasis();
				Vec3 separatingAxisInB = normalInB * input.m_transformB.getBasis();

				Vec3 pInA = m_minkowskiA->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInA);
				Vec3 qInB = m_minkowskiB->localGetSupportVertexWithoutMarginNonVirtual(separatingAxisInB);

				Vec3 pWorld = localTransA(pInA);
				Vec3 qWorld = localTransB(qInB);
				Vec3 w = pWorld - qWorld;
				d0 = normalInB.dot(w) - margin;
			}

			if (d1 > d0)
			{
				m_lastUsedMethod = 10;
				normalInB *= -1;
			}

			if (orgNormalInB.length2())
			{
				if (d2 > d0 && d2 > d1 && d2 > distance)
				{
					normalInB = orgNormalInB;
					distance = d2;
				}
			}
		}

		output.addContactPoint(
			normalInB,
			pointOnB + positionOffset,
			distance);
	}
	else
	{
		//printf("invalid gjk query\n");
	}
}
