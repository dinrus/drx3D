#ifndef DRX3D_MPR_PENETRATION_H
#define DRX3D_MPR_PENETRATION_H

#define DRX3D_DEBUG_MPR1

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

//#define MPR_AVERAGE_CONTACT_POSITIONS

struct MprCollisionDescription
{
	Vec3 m_firstDir;
	i32 m_maxGjkIterations;
	Scalar m_maximumDistanceSquared;
	Scalar m_gjkRelError2;

	MprCollisionDescription()
		: m_firstDir(0, 1, 0),
		  m_maxGjkIterations(1000),
		  m_maximumDistanceSquared(1e30f),
		  m_gjkRelError2(1.0e-6)
	{
	}
	virtual ~MprCollisionDescription()
	{
	}
};

struct MprDistanceInfo
{
	Vec3 m_pointOnA;
	Vec3 m_pointOnB;
	Vec3 m_normalBtoA;
	Scalar m_distance;
};

#ifdef __cplusplus
#define DRX3D_MPR_SQRT sqrtf
#else
#define DRX3D_MPR_SQRT sqrt
#endif
#define DRX3D_MPR_FMIN(x, y) ((x) < (y) ? (x) : (y))
#define DRX3D_MPR_FABS fabs

#define DRX3D_MPR_TOLERANCE 1E-6f
#define DRX3D_MPR_MAX_ITERATIONS 1000

struct _MprSupport_t
{
	Vec3 v;   //!< Support point in minkowski sum
	Vec3 v1;  //!< Support point in obj1
	Vec3 v2;  //!< Support point in obj2
};
typedef struct _MprSupport_t MprSupport_t;

struct _MprSimplex_t
{
	MprSupport_t ps[4];
	i32 last;  //!< index of last added point
};
typedef struct _MprSimplex_t MprSimplex_t;

inline MprSupport_t *MprSimplexPointW(MprSimplex_t *s, i32 idx)
{
	return &s->ps[idx];
}

inline void MprSimplexSetSize(MprSimplex_t *s, i32 size)
{
	s->last = size - 1;
}

#ifdef DEBUG_MPR
inline void PrintPortalVertex(_MprSimplex_t *portal, i32 index)
{
	printf("portal[%d].v = %f,%f,%f, v1=%f,%f,%f, v2=%f,%f,%f\n", index, portal->ps[index].v.x(), portal->ps[index].v.y(), portal->ps[index].v.z(),
		   portal->ps[index].v1.x(), portal->ps[index].v1.y(), portal->ps[index].v1.z(),
		   portal->ps[index].v2.x(), portal->ps[index].v2.y(), portal->ps[index].v2.z());
}
#endif  //DEBUG_MPR

inline i32 MprSimplexSize(const MprSimplex_t *s)
{
	return s->last + 1;
}

inline const MprSupport_t *MprSimplexPoint(const MprSimplex_t *s, i32 idx)
{
	// here is no check on boundaries
	return &s->ps[idx];
}

inline void MprSupportCopy(MprSupport_t *d, const MprSupport_t *s)
{
	*d = *s;
}

inline void MprSimplexSet(MprSimplex_t *s, size_t pos, const MprSupport_t *a)
{
	MprSupportCopy(s->ps + pos, a);
}

inline void MprSimplexSwap(MprSimplex_t *s, size_t pos1, size_t pos2)
{
	MprSupport_t supp;

	MprSupportCopy(&supp, &s->ps[pos1]);
	MprSupportCopy(&s->ps[pos1], &s->ps[pos2]);
	MprSupportCopy(&s->ps[pos2], &supp);
}

inline i32 MprIsZero(float val)
{
	return DRX3D_MPR_FABS(val) < FLT_EPSILON;
}

inline i32 MprEq(float _a, float _b)
{
	float ab;
	float a, b;

	ab = DRX3D_MPR_FABS(_a - _b);
	if (DRX3D_MPR_FABS(ab) < FLT_EPSILON)
		return 1;

	a = DRX3D_MPR_FABS(_a);
	b = DRX3D_MPR_FABS(_b);
	if (b > a)
	{
		return ab < FLT_EPSILON * b;
	}
	else
	{
		return ab < FLT_EPSILON * a;
	}
}

inline i32 MprVec3Eq(const Vec3 *a, const Vec3 *b)
{
	return MprEq((*a).x(), (*b).x()) && MprEq((*a).y(), (*b).y()) && MprEq((*a).z(), (*b).z());
}

template <typename ConvexTemplate>
inline void FindOrigin(const ConvexTemplate &a, const ConvexTemplate &b, const MprCollisionDescription &colDesc, MprSupport_t *center)
{
	center->v1 = a.getObjectCenterInWorld();
	center->v2 = b.getObjectCenterInWorld();
	center->v = center->v1 - center->v2;
}

inline void MprVec3Set(Vec3 *v, float x, float y, float z)
{
	v->setVal(x, y, z);
}

inline void MprVec3Add(Vec3 *v, const Vec3 *w)
{
	*v += *w;
}

inline void MprVec3Copy(Vec3 *v, const Vec3 *w)
{
	*v = *w;
}

inline void MprVec3Scale(Vec3 *d, float k)
{
	*d *= k;
}

inline float MprVec3Dot(const Vec3 *a, const Vec3 *b)
{
	float dot;

	dot = Dot(*a, *b);
	return dot;
}

inline float MprVec3Len2(const Vec3 *v)
{
	return MprVec3Dot(v, v);
}

inline void MprVec3Normalize(Vec3 *d)
{
	float k = 1.f / DRX3D_MPR_SQRT(MprVec3Len2(d));
	MprVec3Scale(d, k);
}

inline void MprVec3Cross(Vec3 *d, const Vec3 *a, const Vec3 *b)
{
	*d = Cross(*a, *b);
}

inline void MprVec3Sub2(Vec3 *d, const Vec3 *v, const Vec3 *w)
{
	*d = *v - *w;
}

inline void PortalDir(const MprSimplex_t *portal, Vec3 *dir)
{
	Vec3 v2v1, v3v1;

	MprVec3Sub2(&v2v1, &MprSimplexPoint(portal, 2)->v,
				  &MprSimplexPoint(portal, 1)->v);
	MprVec3Sub2(&v3v1, &MprSimplexPoint(portal, 3)->v,
				  &MprSimplexPoint(portal, 1)->v);
	MprVec3Cross(dir, &v2v1, &v3v1);
	MprVec3Normalize(dir);
}

inline i32 portalEncapsulesOrigin(const MprSimplex_t *portal,
								  const Vec3 *dir)
{
	float dot;
	dot = MprVec3Dot(dir, &MprSimplexPoint(portal, 1)->v);
	return MprIsZero(dot) || dot > 0.f;
}

inline i32 portalReachTolerance(const MprSimplex_t *portal,
								const MprSupport_t *v4,
								const Vec3 *dir)
{
	float dv1, dv2, dv3, dv4;
	float dot1, dot2, dot3;

	// find the smallest dot product of dir and {v1-v4, v2-v4, v3-v4}

	dv1 = MprVec3Dot(&MprSimplexPoint(portal, 1)->v, dir);
	dv2 = MprVec3Dot(&MprSimplexPoint(portal, 2)->v, dir);
	dv3 = MprVec3Dot(&MprSimplexPoint(portal, 3)->v, dir);
	dv4 = MprVec3Dot(&v4->v, dir);

	dot1 = dv4 - dv1;
	dot2 = dv4 - dv2;
	dot3 = dv4 - dv3;

	dot1 = DRX3D_MPR_FMIN(dot1, dot2);
	dot1 = DRX3D_MPR_FMIN(dot1, dot3);

	return MprEq(dot1, DRX3D_MPR_TOLERANCE) || dot1 < DRX3D_MPR_TOLERANCE;
}

inline i32 portalCanEncapsuleOrigin(const MprSimplex_t *portal,
									const MprSupport_t *v4,
									const Vec3 *dir)
{
	float dot;
	dot = MprVec3Dot(&v4->v, dir);
	return MprIsZero(dot) || dot > 0.f;
}

inline void ExpandPortal(MprSimplex_t *portal,
						   const MprSupport_t *v4)
{
	float dot;
	Vec3 v4v0;

	MprVec3Cross(&v4v0, &v4->v, &MprSimplexPoint(portal, 0)->v);
	dot = MprVec3Dot(&MprSimplexPoint(portal, 1)->v, &v4v0);
	if (dot > 0.f)
	{
		dot = MprVec3Dot(&MprSimplexPoint(portal, 2)->v, &v4v0);
		if (dot > 0.f)
		{
			MprSimplexSet(portal, 1, v4);
		}
		else
		{
			MprSimplexSet(portal, 3, v4);
		}
	}
	else
	{
		dot = MprVec3Dot(&MprSimplexPoint(portal, 3)->v, &v4v0);
		if (dot > 0.f)
		{
			MprSimplexSet(portal, 2, v4);
		}
		else
		{
			MprSimplexSet(portal, 1, v4);
		}
	}
}
template <typename ConvexTemplate>
inline void MprSupport(const ConvexTemplate &a, const ConvexTemplate &b,
						 const MprCollisionDescription &colDesc,
						 const Vec3 &dir, MprSupport_t *supp)
{
	Vec3 separatingAxisInA = dir * a.getWorldTransform().getBasis();
	Vec3 separatingAxisInB = -dir * b.getWorldTransform().getBasis();

	Vec3 pInA = a.getLocalSupportWithMargin(separatingAxisInA);
	Vec3 qInB = b.getLocalSupportWithMargin(separatingAxisInB);

	supp->v1 = a.getWorldTransform()(pInA);
	supp->v2 = b.getWorldTransform()(qInB);
	supp->v = supp->v1 - supp->v2;
}

template <typename ConvexTemplate>
static i32 DiscoverPortal(const ConvexTemplate &a, const ConvexTemplate &b,
							const MprCollisionDescription &colDesc,
							MprSimplex_t *portal)
{
	Vec3 dir, va, vb;
	float dot;
	i32 cont;

	// vertex 0 is center of portal
	FindOrigin(a, b, colDesc, MprSimplexPointW(portal, 0));

	// vertex 0 is center of portal
	MprSimplexSetSize(portal, 1);

	Vec3 zero = Vec3(0, 0, 0);
	Vec3 *org = &zero;

	if (MprVec3Eq(&MprSimplexPoint(portal, 0)->v, org))
	{
		// Portal's center lies on origin (0,0,0) => we know that objects
		// intersect but we would need to know penetration info.
		// So move center little bit...
		MprVec3Set(&va, FLT_EPSILON * 10.f, 0.f, 0.f);
		MprVec3Add(&MprSimplexPointW(portal, 0)->v, &va);
	}

	// vertex 1 = support in direction of origin
	MprVec3Copy(&dir, &MprSimplexPoint(portal, 0)->v);
	MprVec3Scale(&dir, -1.f);
	MprVec3Normalize(&dir);

	MprSupport(a, b, colDesc, dir, MprSimplexPointW(portal, 1));

	MprSimplexSetSize(portal, 2);

	// test if origin isn't outside of v1
	dot = MprVec3Dot(&MprSimplexPoint(portal, 1)->v, &dir);

	if (MprIsZero(dot) || dot < 0.f)
		return -1;

	// vertex 2
	MprVec3Cross(&dir, &MprSimplexPoint(portal, 0)->v,
				   &MprSimplexPoint(portal, 1)->v);
	if (MprIsZero(MprVec3Len2(&dir)))
	{
		if (MprVec3Eq(&MprSimplexPoint(portal, 1)->v, org))
		{
			// origin lies on v1
			return 1;
		}
		else
		{
			// origin lies on v0-v1 segment
			return 2;
		}
	}

	MprVec3Normalize(&dir);
	MprSupport(a, b, colDesc, dir, MprSimplexPointW(portal, 2));

	dot = MprVec3Dot(&MprSimplexPoint(portal, 2)->v, &dir);
	if (MprIsZero(dot) || dot < 0.f)
		return -1;

	MprSimplexSetSize(portal, 3);

	// vertex 3 direction
	MprVec3Sub2(&va, &MprSimplexPoint(portal, 1)->v,
				  &MprSimplexPoint(portal, 0)->v);
	MprVec3Sub2(&vb, &MprSimplexPoint(portal, 2)->v,
				  &MprSimplexPoint(portal, 0)->v);
	MprVec3Cross(&dir, &va, &vb);
	MprVec3Normalize(&dir);

	// it is better to form portal faces to be oriented "outside" origin
	dot = MprVec3Dot(&dir, &MprSimplexPoint(portal, 0)->v);
	if (dot > 0.f)
	{
		MprSimplexSwap(portal, 1, 2);
		MprVec3Scale(&dir, -1.f);
	}

	while (MprSimplexSize(portal) < 4)
	{
		MprSupport(a, b, colDesc, dir, MprSimplexPointW(portal, 3));

		dot = MprVec3Dot(&MprSimplexPoint(portal, 3)->v, &dir);
		if (MprIsZero(dot) || dot < 0.f)
			return -1;

		cont = 0;

		// test if origin is outside (v1, v0, v3) - set v2 as v3 and
		// continue
		MprVec3Cross(&va, &MprSimplexPoint(portal, 1)->v,
					   &MprSimplexPoint(portal, 3)->v);
		dot = MprVec3Dot(&va, &MprSimplexPoint(portal, 0)->v);
		if (dot < 0.f && !MprIsZero(dot))
		{
			MprSimplexSet(portal, 2, MprSimplexPoint(portal, 3));
			cont = 1;
		}

		if (!cont)
		{
			// test if origin is outside (v3, v0, v2) - set v1 as v3 and
			// continue
			MprVec3Cross(&va, &MprSimplexPoint(portal, 3)->v,
						   &MprSimplexPoint(portal, 2)->v);
			dot = MprVec3Dot(&va, &MprSimplexPoint(portal, 0)->v);
			if (dot < 0.f && !MprIsZero(dot))
			{
				MprSimplexSet(portal, 1, MprSimplexPoint(portal, 3));
				cont = 1;
			}
		}

		if (cont)
		{
			MprVec3Sub2(&va, &MprSimplexPoint(portal, 1)->v,
						  &MprSimplexPoint(portal, 0)->v);
			MprVec3Sub2(&vb, &MprSimplexPoint(portal, 2)->v,
						  &MprSimplexPoint(portal, 0)->v);
			MprVec3Cross(&dir, &va, &vb);
			MprVec3Normalize(&dir);
		}
		else
		{
			MprSimplexSetSize(portal, 4);
		}
	}

	return 0;
}

template <typename ConvexTemplate>
static i32 RefinePortal(const ConvexTemplate &a, const ConvexTemplate &b, const MprCollisionDescription &colDesc,
						  MprSimplex_t *portal)
{
	Vec3 dir;
	MprSupport_t v4;

	for (i32 i = 0; i < DRX3D_MPR_MAX_ITERATIONS; i++)
	//while (1)
	{
		// compute direction outside the portal (from v0 through v1,v2,v3
		// face)
		PortalDir(portal, &dir);

		// test if origin is inside the portal
		if (portalEncapsulesOrigin(portal, &dir))
			return 0;

		// get next support point

		MprSupport(a, b, colDesc, dir, &v4);

		// test if v4 can expand portal to contain origin and if portal
		// expanding doesn't reach given tolerance
		if (!portalCanEncapsuleOrigin(portal, &v4, &dir) || portalReachTolerance(portal, &v4, &dir))
		{
			return -1;
		}

		// v1-v2-v3 triangle must be rearranged to face outside Minkowski
		// difference (direction from v0).
		ExpandPortal(portal, &v4);
	}

	return -1;
}

static void FindPos(const MprSimplex_t *portal, Vec3 *pos)
{
	Vec3 zero = Vec3(0, 0, 0);
	Vec3 *origin = &zero;

	Vec3 dir;
	size_t i;
	float b[4], sum, inv;
	Vec3 vec, p1, p2;

	PortalDir(portal, &dir);

	// use barycentric coordinates of tetrahedron to find origin
	MprVec3Cross(&vec, &MprSimplexPoint(portal, 1)->v,
				   &MprSimplexPoint(portal, 2)->v);
	b[0] = MprVec3Dot(&vec, &MprSimplexPoint(portal, 3)->v);

	MprVec3Cross(&vec, &MprSimplexPoint(portal, 3)->v,
				   &MprSimplexPoint(portal, 2)->v);
	b[1] = MprVec3Dot(&vec, &MprSimplexPoint(portal, 0)->v);

	MprVec3Cross(&vec, &MprSimplexPoint(portal, 0)->v,
				   &MprSimplexPoint(portal, 1)->v);
	b[2] = MprVec3Dot(&vec, &MprSimplexPoint(portal, 3)->v);

	MprVec3Cross(&vec, &MprSimplexPoint(portal, 2)->v,
				   &MprSimplexPoint(portal, 1)->v);
	b[3] = MprVec3Dot(&vec, &MprSimplexPoint(portal, 0)->v);

	sum = b[0] + b[1] + b[2] + b[3];

	if (MprIsZero(sum) || sum < 0.f)
	{
		b[0] = 0.f;

		MprVec3Cross(&vec, &MprSimplexPoint(portal, 2)->v,
					   &MprSimplexPoint(portal, 3)->v);
		b[1] = MprVec3Dot(&vec, &dir);
		MprVec3Cross(&vec, &MprSimplexPoint(portal, 3)->v,
					   &MprSimplexPoint(portal, 1)->v);
		b[2] = MprVec3Dot(&vec, &dir);
		MprVec3Cross(&vec, &MprSimplexPoint(portal, 1)->v,
					   &MprSimplexPoint(portal, 2)->v);
		b[3] = MprVec3Dot(&vec, &dir);

		sum = b[1] + b[2] + b[3];
	}

	inv = 1.f / sum;

	MprVec3Copy(&p1, origin);
	MprVec3Copy(&p2, origin);
	for (i = 0; i < 4; i++)
	{
		MprVec3Copy(&vec, &MprSimplexPoint(portal, i)->v1);
		MprVec3Scale(&vec, b[i]);
		MprVec3Add(&p1, &vec);

		MprVec3Copy(&vec, &MprSimplexPoint(portal, i)->v2);
		MprVec3Scale(&vec, b[i]);
		MprVec3Add(&p2, &vec);
	}
	MprVec3Scale(&p1, inv);
	MprVec3Scale(&p2, inv);
#ifdef MPR_AVERAGE_CONTACT_POSITIONS
	MprVec3Copy(pos, &p1);
	MprVec3Add(pos, &p2);
	MprVec3Scale(pos, 0.5);
#else
	MprVec3Copy(pos, &p2);
#endif  //MPR_AVERAGE_CONTACT_POSITIONS
}

inline float MprVec3Dist2(const Vec3 *a, const Vec3 *b)
{
	Vec3 ab;
	MprVec3Sub2(&ab, a, b);
	return MprVec3Len2(&ab);
}

inline float _MprVec3PointSegmentDist2(const Vec3 *P,
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

	float dist, t;
	Vec3 d, a;

	// direction of segment
	MprVec3Sub2(&d, b, x0);

	// precompute vector from P to x0
	MprVec3Sub2(&a, x0, P);

	t = -1.f * MprVec3Dot(&a, &d);
	t /= MprVec3Len2(&d);

	if (t < 0.f || MprIsZero(t))
	{
		dist = MprVec3Dist2(x0, P);
		if (witness)
			MprVec3Copy(witness, x0);
	}
	else if (t > 1.f || MprEq(t, 1.f))
	{
		dist = MprVec3Dist2(b, P);
		if (witness)
			MprVec3Copy(witness, b);
	}
	else
	{
		if (witness)
		{
			MprVec3Copy(witness, &d);
			MprVec3Scale(witness, t);
			MprVec3Add(witness, x0);
			dist = MprVec3Dist2(witness, P);
		}
		else
		{
			// recycling variables
			MprVec3Scale(&d, t);
			MprVec3Add(&d, &a);
			dist = MprVec3Len2(&d);
		}
	}

	return dist;
}

inline float MprVec3PointTriDist2(const Vec3 *P,
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
	float u, v, w, p, q, r;
	float s, t, dist, dist2;
	Vec3 witness2;

	MprVec3Sub2(&d1, B, x0);
	MprVec3Sub2(&d2, C, x0);
	MprVec3Sub2(&a, x0, P);

	u = MprVec3Dot(&a, &a);
	v = MprVec3Dot(&d1, &d1);
	w = MprVec3Dot(&d2, &d2);
	p = MprVec3Dot(&a, &d1);
	q = MprVec3Dot(&a, &d2);
	r = MprVec3Dot(&d1, &d2);

	Scalar div = (w * v - r * r);
	if (MprIsZero(div))
	{
		s = -1;
	}
	else
	{
		s = (q * r - w * p) / div;
		t = (-s * r - q) / w;
	}

	if ((MprIsZero(s) || s > 0.f) && (MprEq(s, 1.f) || s < 1.f) && (MprIsZero(t) || t > 0.f) && (MprEq(t, 1.f) || t < 1.f) && (MprEq(t + s, 1.f) || t + s < 1.f))
	{
		if (witness)
		{
			MprVec3Scale(&d1, s);
			MprVec3Scale(&d2, t);
			MprVec3Copy(witness, x0);
			MprVec3Add(witness, &d1);
			MprVec3Add(witness, &d2);

			dist = MprVec3Dist2(witness, P);
		}
		else
		{
			dist = s * s * v;
			dist += t * t * w;
			dist += 2.f * s * t * r;
			dist += 2.f * s * p;
			dist += 2.f * t * q;
			dist += u;
		}
	}
	else
	{
		dist = _MprVec3PointSegmentDist2(P, x0, B, witness);

		dist2 = _MprVec3PointSegmentDist2(P, x0, C, &witness2);
		if (dist2 < dist)
		{
			dist = dist2;
			if (witness)
				MprVec3Copy(witness, &witness2);
		}

		dist2 = _MprVec3PointSegmentDist2(P, B, C, &witness2);
		if (dist2 < dist)
		{
			dist = dist2;
			if (witness)
				MprVec3Copy(witness, &witness2);
		}
	}

	return dist;
}

template <typename ConvexTemplate>
static void FindPenetr(const ConvexTemplate &a, const ConvexTemplate &b,
						 const MprCollisionDescription &colDesc,
						 MprSimplex_t *portal,
						 float *depth, Vec3 *pdir, Vec3 *pos)
{
	Vec3 dir;
	MprSupport_t v4;
	u64 iterations;

	Vec3 zero = Vec3(0, 0, 0);
	Vec3 *origin = &zero;

	iterations = 1UL;
	for (i32 i = 0; i < DRX3D_MPR_MAX_ITERATIONS; i++)
	//while (1)
	{
		// compute portal direction and obtain next support point
		PortalDir(portal, &dir);

		MprSupport(a, b, colDesc, dir, &v4);

		// reached tolerance -> find penetration info
		if (portalReachTolerance(portal, &v4, &dir) || iterations == DRX3D_MPR_MAX_ITERATIONS)
		{
			*depth = MprVec3PointTriDist2(origin, &MprSimplexPoint(portal, 1)->v, &MprSimplexPoint(portal, 2)->v, &MprSimplexPoint(portal, 3)->v, pdir);
			*depth = DRX3D_MPR_SQRT(*depth);

			if (MprIsZero((*pdir).x()) && MprIsZero((*pdir).y()) && MprIsZero((*pdir).z()))
			{
				*pdir = dir;
			}
			MprVec3Normalize(pdir);

			// barycentric coordinates:
			FindPos(portal, pos);

			return;
		}

		ExpandPortal(portal, &v4);

		iterations++;
	}
}

static void FindPenetrTouch(MprSimplex_t *portal, float *depth, Vec3 *dir, Vec3 *pos)
{
	// Touching contact on portal's v1 - so depth is zero and direction
	// is unimportant and pos can be guessed
	*depth = 0.f;
	Vec3 zero = Vec3(0, 0, 0);
	Vec3 *origin = &zero;

	MprVec3Copy(dir, origin);
#ifdef MPR_AVERAGE_CONTACT_POSITIONS
	MprVec3Copy(pos, &MprSimplexPoint(portal, 1)->v1);
	MprVec3Add(pos, &MprSimplexPoint(portal, 1)->v2);
	MprVec3Scale(pos, 0.5);
#else
	MprVec3Copy(pos, &MprSimplexPoint(portal, 1)->v2);
#endif
}

static void FindPenetrSegment(MprSimplex_t *portal,
								float *depth, Vec3 *dir, Vec3 *pos)
{
	// Origin lies on v0-v1 segment.
	// Depth is distance to v1, direction also and position must be
	// computed
#ifdef MPR_AVERAGE_CONTACT_POSITIONS
	MprVec3Copy(pos, &MprSimplexPoint(portal, 1)->v1);
	MprVec3Add(pos, &MprSimplexPoint(portal, 1)->v2);
	MprVec3Scale(pos, 0.5f);
#else
	MprVec3Copy(pos, &MprSimplexPoint(portal, 1)->v2);
#endif  //MPR_AVERAGE_CONTACT_POSITIONS

	MprVec3Copy(dir, &MprSimplexPoint(portal, 1)->v);
	*depth = DRX3D_MPR_SQRT(MprVec3Len2(dir));
	MprVec3Normalize(dir);
}

template <typename ConvexTemplate>
inline i32 MprPenetration(const ConvexTemplate &a, const ConvexTemplate &b,
							const MprCollisionDescription &colDesc,
							float *depthOut, Vec3 *dirOut, Vec3 *posOut)
{
	MprSimplex_t portal;

	// Phase 1: Portal discovery
	i32 result = DiscoverPortal(a, b, colDesc, &portal);

	//sepAxis[pairIndex] = *pdir;//or -dir?

	switch (result)
	{
		case 0:
		{
			// Phase 2: Portal refinement

			result = RefinePortal(a, b, colDesc, &portal);
			if (result < 0)
				return -1;

			// Phase 3. Penetration info
			FindPenetr(a, b, colDesc, &portal, depthOut, dirOut, posOut);

			break;
		}
		case 1:
		{
			// Touching contact on portal's v1.
			FindPenetrTouch(&portal, depthOut, dirOut, posOut);
			result = 0;
			break;
		}
		case 2:
		{
			FindPenetrSegment(&portal, depthOut, dirOut, posOut);
			result = 0;
			break;
		}
		default:
		{
			//if (res < 0)
			//{
			// Origin isn't inside portal - no collision.
			result = -1;
			//}
		}
	};

	return result;
};

template <typename ConvexTemplate, typename MprDistanceTemplate>
inline i32 ComputeMprPenetration(const ConvexTemplate &a, const ConvexTemplate &b, const MprCollisionDescription &colDesc, MprDistanceTemplate *distInfo)
{
	Vec3 dir, pos;
	float depth;

	i32 res = MprPenetration(a, b, colDesc, &depth, &dir, &pos);
	if (res == 0)
	{
		distInfo->m_distance = -depth;
		distInfo->m_pointOnB = pos;
		distInfo->m_normalBtoA = -dir;
		distInfo->m_pointOnA = pos - distInfo->m_distance * dir;
		return 0;
	}

	return -1;
}

#endif  //DRX3D_MPR_PENETRATION_H
