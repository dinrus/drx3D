#ifndef _DRX3D_SOFT_BODY_INTERNALS_H
#define _DRX3D_SOFT_BODY_INTERNALS_H

#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/PolarDecomposition.h>
#include <drx3D/Physics/Collision/BroadPhase/BroadphaseInterface.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/Shapes/ConvexInternalShape.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <string.h>  //for memset
#include <cmath>
#include "poly34.h"

// Given a multibody link, a contact point and a contact direction, fill in the jacobian data needed to calculate the velocity change given an impulse in the contact direction
static SIMD_FORCE_INLINE void findJacobian(const MultiBodyLinkCollider* multibodyLinkCol,
										   MultiBodyJacobianData& jacobianData,
										   const Vec3& contact_point,
										   const Vec3& dir)
{
	i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
	jacobianData.m_jacobians.resize(ndof);
	jacobianData.m_deltaVelocitiesUnitImpulse.resize(ndof);
	Scalar* jac = &jacobianData.m_jacobians[0];

	multibodyLinkCol->m_multiBody->fillContactJacobianMultiDof(multibodyLinkCol->m_link, contact_point, dir, jac, jacobianData.scratch_r, jacobianData.scratch_v, jacobianData.scratch_m);
	multibodyLinkCol->m_multiBody->calcAccelerationDeltasMultiDof(&jacobianData.m_jacobians[0], &jacobianData.m_deltaVelocitiesUnitImpulse[0], jacobianData.scratch_r, jacobianData.scratch_v);
}
static SIMD_FORCE_INLINE Vec3 generateUnitOrthogonalVector(const Vec3& u)
{
	Scalar ux = u.getX();
	Scalar uy = u.getY();
	Scalar uz = u.getZ();
	Scalar ax = std::abs(ux);
	Scalar ay = std::abs(uy);
	Scalar az = std::abs(uz);
	Vec3 v;
	if (ax <= ay && ax <= az)
		v = Vec3(0, -uz, uy);
	else if (ay <= ax && ay <= az)
		v = Vec3(-uz, 0, ux);
	else
		v = Vec3(-uy, ux, 0);
	v.normalize();
	return v;
}

static SIMD_FORCE_INLINE bool proximityTest(const Vec3& x1, const Vec3& x2, const Vec3& x3, const Vec3& x4, const Vec3& normal, const Scalar& mrg, Vec3& bary)
{
	Vec3 x43 = x4 - x3;
	if (std::abs(x43.dot(normal)) > mrg)
		return false;
	Vec3 x13 = x1 - x3;
	Vec3 x23 = x2 - x3;
	Scalar a11 = x13.length2();
	Scalar a22 = x23.length2();
	Scalar a12 = x13.dot(x23);
	Scalar b1 = x13.dot(x43);
	Scalar b2 = x23.dot(x43);
	Scalar det = a11 * a22 - a12 * a12;
	if (det < SIMD_EPSILON)
		return false;
	Scalar w1 = (b1 * a22 - b2 * a12) / det;
	Scalar w2 = (b2 * a11 - b1 * a12) / det;
	Scalar w3 = 1 - w1 - w2;
	Scalar delta = mrg / std::sqrt(0.5 * std::abs(x13.cross(x23).safeNorm()));
	bary = Vec3(w1, w2, w3);
	for (i32 i = 0; i < 3; ++i)
	{
		if (bary[i] < -delta || bary[i] > 1 + delta)
			return false;
	}
	return true;
}
static i32k KDOP_COUNT = 13;
static Vec3 dop[KDOP_COUNT] = {Vec3(1, 0, 0),
									Vec3(0, 1, 0),
									Vec3(0, 0, 1),
									Vec3(1, 1, 0),
									Vec3(1, 0, 1),
									Vec3(0, 1, 1),
									Vec3(1, -1, 0),
									Vec3(1, 0, -1),
									Vec3(0, 1, -1),
									Vec3(1, 1, 1),
									Vec3(1, -1, 1),
									Vec3(1, 1, -1),
									Vec3(1, -1, -1)};

static inline i32 getSign(const Vec3& n, const Vec3& x)
{
	Scalar d = n.dot(x);
	if (d > SIMD_EPSILON)
		return 1;
	if (d < -SIMD_EPSILON)
		return -1;
	return 0;
}

static SIMD_FORCE_INLINE bool hasSeparatingPlane(const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt)
{
	Vec3 hex[6] = {face->m_n[0]->m_x - node->m_x,
						face->m_n[1]->m_x - node->m_x,
						face->m_n[2]->m_x - node->m_x,
						face->m_n[0]->m_x + dt * face->m_n[0]->m_v - node->m_x,
						face->m_n[1]->m_x + dt * face->m_n[1]->m_v - node->m_x,
						face->m_n[2]->m_x + dt * face->m_n[2]->m_v - node->m_x};
	Vec3 segment = dt * node->m_v;
	for (i32 i = 0; i < KDOP_COUNT; ++i)
	{
		i32 s = getSign(dop[i], segment);
		i32 j = 0;
		for (; j < 6; ++j)
		{
			if (getSign(dop[i], hex[j]) == s)
				break;
		}
		if (j == 6)
			return true;
	}
	return false;
}

static SIMD_FORCE_INLINE bool nearZero(const Scalar& a)
{
	return (a > -SAFE_EPSILON && a < SAFE_EPSILON);
}
static SIMD_FORCE_INLINE bool sameSign(const Scalar& a, const Scalar& b)
{
	return (nearZero(a) || nearZero(b) || (a > SAFE_EPSILON && b > SAFE_EPSILON) || (a < -SAFE_EPSILON && b < -SAFE_EPSILON));
}
static SIMD_FORCE_INLINE bool diffSign(const Scalar& a, const Scalar& b)
{
	return !sameSign(a, b);
}
inline Scalar evaluateBezier2(const Scalar& p0, const Scalar& p1, const Scalar& p2, const Scalar& t, const Scalar& s)
{
	Scalar s2 = s * s;
	Scalar t2 = t * t;

	return p0 * s2 + p1 * Scalar(2.0) * s * t + p2 * t2;
}
inline Scalar evaluateBezier(const Scalar& p0, const Scalar& p1, const Scalar& p2, const Scalar& p3, const Scalar& t, const Scalar& s)
{
	Scalar s2 = s * s;
	Scalar s3 = s2 * s;
	Scalar t2 = t * t;
	Scalar t3 = t2 * t;

	return p0 * s3 + p1 * Scalar(3.0) * s2 * t + p2 * Scalar(3.0) * s * t2 + p3 * t3;
}
static SIMD_FORCE_INLINE bool getSigns(bool type_c, const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& t0, const Scalar& t1, Scalar& lt0, Scalar& lt1)
{
	if (sameSign(t0, t1))
	{
		lt0 = t0;
		lt1 = t0;
		return true;
	}

	if (type_c || diffSign(k0, k3))
	{
		Scalar ft = evaluateBezier(k0, k1, k2, k3, t0, -t1);
		if (t0 < -0)
			ft = -ft;

		if (sameSign(ft, k0))
		{
			lt0 = t1;
			lt1 = t1;
		}
		else
		{
			lt0 = t0;
			lt1 = t0;
		}
		return true;
	}

	if (!type_c)
	{
		Scalar ft = evaluateBezier(k0, k1, k2, k3, t0, -t1);
		if (t0 < -0)
			ft = -ft;

		if (diffSign(ft, k0))
		{
			lt0 = t0;
			lt1 = t1;
			return true;
		}

		Scalar fk = evaluateBezier2(k1 - k0, k2 - k1, k3 - k2, t0, -t1);

		if (sameSign(fk, k1 - k0))
			lt0 = lt1 = t1;
		else
			lt0 = lt1 = t0;

		return true;
	}
	return false;
}

static SIMD_FORCE_INLINE void getBernsteinCoeff(const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt, Scalar& k0, Scalar& k1, Scalar& k2, Scalar& k3)
{
	const Vec3& n0 = face->m_n0;
	const Vec3& n1 = face->m_n1;
	Vec3 n_hat = n0 + n1 - face->m_vn;
	Vec3 p0ma0 = node->m_x - face->m_n[0]->m_x;
	Vec3 p1ma1 = node->m_q - face->m_n[0]->m_q;
	k0 = (p0ma0).dot(n0) * 3.0;
	k1 = (p0ma0).dot(n_hat) + (p1ma1).dot(n0);
	k2 = (p1ma1).dot(n_hat) + (p0ma0).dot(n1);
	k3 = (p1ma1).dot(n1) * 3.0;
}

static SIMD_FORCE_INLINE void polyDecomposition(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& j0, const Scalar& j1, const Scalar& j2, Scalar& u0, Scalar& u1, Scalar& v0, Scalar& v1)
{
	Scalar denom = 4.0 * (j1 - j2) * (j1 - j0) + (j2 - j0) * (j2 - j0);
	u0 = (2.0 * (j1 - j2) * (3.0 * k1 - 2.0 * k0 - k3) - (j0 - j2) * (3.0 * k2 - 2.0 * k3 - k0)) / denom;
	u1 = (2.0 * (j1 - j0) * (3.0 * k2 - 2.0 * k3 - k0) - (j2 - j0) * (3.0 * k1 - 2.0 * k0 - k3)) / denom;
	v0 = k0 - u0 * j0;
	v1 = k3 - u1 * j2;
}

static SIMD_FORCE_INLINE bool rootFindingLemma(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3)
{
	Scalar u0, u1, v0, v1;
	Scalar j0 = 3.0 * (k1 - k0);
	Scalar j1 = 3.0 * (k2 - k1);
	Scalar j2 = 3.0 * (k3 - k2);
	polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
	if (sameSign(v0, v1))
	{
		Scalar Ypa = j0 * (1.0 - v0) * (1.0 - v0) + 2.0 * j1 * v0 * (1.0 - v0) + j2 * v0 * v0;  // Y'(v0)
		if (sameSign(Ypa, j0))
		{
			return (diffSign(k0, v1));
		}
	}
	return diffSign(k0, v0);
}

static SIMD_FORCE_INLINE void getJs(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const SoftBody::Node* a, const SoftBody::Node* b, const SoftBody::Node* c, const SoftBody::Node* p, const Scalar& dt, Scalar& j0, Scalar& j1, Scalar& j2)
{
	const Vec3& a0 = a->m_x;
	const Vec3& b0 = b->m_x;
	const Vec3& c0 = c->m_x;
	const Vec3& va = a->m_v;
	const Vec3& vb = b->m_v;
	const Vec3& vc = c->m_v;
	const Vec3 a1 = a0 + dt * va;
	const Vec3 b1 = b0 + dt * vb;
	const Vec3 c1 = c0 + dt * vc;
	Vec3 n0 = (b0 - a0).cross(c0 - a0);
	Vec3 n1 = (b1 - a1).cross(c1 - a1);
	Vec3 n_hat = n0 + n1 - dt * dt * (vb - va).cross(vc - va);
	const Vec3& p0 = p->m_x;
	const Vec3& vp = p->m_v;
	Vec3 p1 = p0 + dt * vp;
	Vec3 m0 = (b0 - p0).cross(c0 - p0);
	Vec3 m1 = (b1 - p1).cross(c1 - p1);
	Vec3 m_hat = m0 + m1 - dt * dt * (vb - vp).cross(vc - vp);
	Scalar l0 = m0.dot(n0);
	Scalar l1 = 0.25 * (m0.dot(n_hat) + m_hat.dot(n0));
	Scalar l2 = Scalar(1) / Scalar(6) * (m0.dot(n1) + m_hat.dot(n_hat) + m1.dot(n0));
	Scalar l3 = 0.25 * (m_hat.dot(n1) + m1.dot(n_hat));
	Scalar l4 = m1.dot(n1);

	Scalar k1p = 0.25 * k0 + 0.75 * k1;
	Scalar k2p = 0.5 * k1 + 0.5 * k2;
	Scalar k3p = 0.75 * k2 + 0.25 * k3;

	Scalar s0 = (l1 * k0 - l0 * k1p) * 4.0;
	Scalar s1 = (l2 * k0 - l0 * k2p) * 2.0;
	Scalar s2 = (l3 * k0 - l0 * k3p) * Scalar(4) / Scalar(3);
	Scalar s3 = l4 * k0 - l0 * k3;

	j0 = (s1 * k0 - s0 * k1) * 3.0;
	j1 = (s2 * k0 - s0 * k2) * 1.5;
	j2 = (s3 * k0 - s0 * k3);
}

static SIMD_FORCE_INLINE bool signDetermination1Internal(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& u0, const Scalar& u1, const Scalar& v0, const Scalar& v1)
{
	Scalar Yu0 = k0 * (1.0 - u0) * (1.0 - u0) * (1.0 - u0) + 3.0 * k1 * u0 * (1.0 - u0) * (1.0 - u0) + 3.0 * k2 * u0 * u0 * (1.0 - u0) + k3 * u0 * u0 * u0;  // Y(u0)
	Scalar Yv0 = k0 * (1.0 - v0) * (1.0 - v0) * (1.0 - v0) + 3.0 * k1 * v0 * (1.0 - v0) * (1.0 - v0) + 3.0 * k2 * v0 * v0 * (1.0 - v0) + k3 * v0 * v0 * v0;  // Y(v0)

	Scalar sign_Ytp = (u0 > u1) ? Yu0 : -Yu0;
	Scalar L = sameSign(sign_Ytp, k0) ? u1 : u0;
	sign_Ytp = (v0 > v1) ? Yv0 : -Yv0;
	Scalar K = (sameSign(sign_Ytp, k0)) ? v1 : v0;
	return diffSign(L, K);
}

static SIMD_FORCE_INLINE bool signDetermination2Internal(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& j0, const Scalar& j1, const Scalar& j2, const Scalar& u0, const Scalar& u1, const Scalar& v0, const Scalar& v1)
{
	Scalar Yu0 = k0 * (1.0 - u0) * (1.0 - u0) * (1.0 - u0) + 3.0 * k1 * u0 * (1.0 - u0) * (1.0 - u0) + 3.0 * k2 * u0 * u0 * (1.0 - u0) + k3 * u0 * u0 * u0;  // Y(u0)
	Scalar sign_Ytp = (u0 > u1) ? Yu0 : -Yu0, L1, L2;
	if (diffSign(sign_Ytp, k0))
	{
		L1 = u0;
		L2 = u1;
	}
	else
	{
		Scalar Yp_u0 = j0 * (1.0 - u0) * (1.0 - u0) + 2.0 * j1 * (1.0 - u0) * u0 + j2 * u0 * u0;
		if (sameSign(Yp_u0, j0))
		{
			L1 = u1;
			L2 = u1;
		}
		else
		{
			L1 = u0;
			L2 = u0;
		}
	}
	Scalar Yv0 = k0 * (1.0 - v0) * (1.0 - v0) * (1.0 - v0) + 3.0 * k1 * v0 * (1.0 - v0) * (1.0 - v0) + 3.0 * k2 * v0 * v0 * (1.0 - v0) + k3 * v0 * v0 * v0;  // Y(uv0)
	sign_Ytp = (v0 > v1) ? Yv0 : -Yv0;
	Scalar K1, K2;
	if (diffSign(sign_Ytp, k0))
	{
		K1 = v0;
		K2 = v1;
	}
	else
	{
		Scalar Yp_v0 = j0 * (1.0 - v0) * (1.0 - v0) + 2.0 * j1 * (1.0 - v0) * v0 + j2 * v0 * v0;
		if (sameSign(Yp_v0, j0))
		{
			K1 = v1;
			K2 = v1;
		}
		else
		{
			K1 = v0;
			K2 = v0;
		}
	}
	return (diffSign(K1, L1) || diffSign(L2, K2));
}

static SIMD_FORCE_INLINE bool signDetermination1(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt)
{
	Scalar j0, j1, j2, u0, u1, v0, v1;
	// p1
	getJs(k0, k1, k2, k3, face->m_n[0], face->m_n[1], face->m_n[2], node, dt, j0, j1, j2);
	if (nearZero(j0 + j2 - j1 * 2.0))
	{
		Scalar lt0, lt1;
		getSigns(true, k0, k1, k2, k3, j0, j2, lt0, lt1);
		if (lt0 < -SAFE_EPSILON)
			return false;
	}
	else
	{
		polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
		if (!signDetermination1Internal(k0, k1, k2, k3, u0, u1, v0, v1))
			return false;
	}
	// p2
	getJs(k0, k1, k2, k3, face->m_n[1], face->m_n[2], face->m_n[0], node, dt, j0, j1, j2);
	if (nearZero(j0 + j2 - j1 * 2.0))
	{
		Scalar lt0, lt1;
		getSigns(true, k0, k1, k2, k3, j0, j2, lt0, lt1);
		if (lt0 < -SAFE_EPSILON)
			return false;
	}
	else
	{
		polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
		if (!signDetermination1Internal(k0, k1, k2, k3, u0, u1, v0, v1))
			return false;
	}
	// p3
	getJs(k0, k1, k2, k3, face->m_n[2], face->m_n[0], face->m_n[1], node, dt, j0, j1, j2);
	if (nearZero(j0 + j2 - j1 * 2.0))
	{
		Scalar lt0, lt1;
		getSigns(true, k0, k1, k2, k3, j0, j2, lt0, lt1);
		if (lt0 < -SAFE_EPSILON)
			return false;
	}
	else
	{
		polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
		if (!signDetermination1Internal(k0, k1, k2, k3, u0, u1, v0, v1))
			return false;
	}
	return true;
}

static SIMD_FORCE_INLINE bool signDetermination2(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt)
{
	Scalar j0, j1, j2, u0, u1, v0, v1;
	// p1
	getJs(k0, k1, k2, k3, face->m_n[0], face->m_n[1], face->m_n[2], node, dt, j0, j1, j2);
	if (nearZero(j0 + j2 - j1 * 2.0))
	{
		Scalar lt0, lt1;
		bool bt0 = true, bt1 = true;
		getSigns(false, k0, k1, k2, k3, j0, j2, lt0, lt1);
		if (lt0 < -SAFE_EPSILON)
			bt0 = false;
		if (lt1 < -SAFE_EPSILON)
			bt1 = false;
		if (!bt0 && !bt1)
			return false;
	}
	else
	{
		polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
		if (!signDetermination2Internal(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1))
			return false;
	}
	// p2
	getJs(k0, k1, k2, k3, face->m_n[1], face->m_n[2], face->m_n[0], node, dt, j0, j1, j2);
	if (nearZero(j0 + j2 - j1 * 2.0))
	{
		Scalar lt0, lt1;
		bool bt0 = true, bt1 = true;
		getSigns(false, k0, k1, k2, k3, j0, j2, lt0, lt1);
		if (lt0 < -SAFE_EPSILON)
			bt0 = false;
		if (lt1 < -SAFE_EPSILON)
			bt1 = false;
		if (!bt0 && !bt1)
			return false;
	}
	else
	{
		polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
		if (!signDetermination2Internal(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1))
			return false;
	}
	// p3
	getJs(k0, k1, k2, k3, face->m_n[2], face->m_n[0], face->m_n[1], node, dt, j0, j1, j2);
	if (nearZero(j0 + j2 - j1 * 2.0))
	{
		Scalar lt0, lt1;
		bool bt0 = true, bt1 = true;
		getSigns(false, k0, k1, k2, k3, j0, j2, lt0, lt1);
		if (lt0 < -SAFE_EPSILON)
			bt0 = false;
		if (lt1 < -SAFE_EPSILON)
			bt1 = false;
		if (!bt0 && !bt1)
			return false;
	}
	else
	{
		polyDecomposition(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1);
		if (!signDetermination2Internal(k0, k1, k2, k3, j0, j1, j2, u0, u1, v0, v1))
			return false;
	}
	return true;
}

static SIMD_FORCE_INLINE bool coplanarAndInsideTest(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt)
{
	// Coplanar test
	if (diffSign(k1 - k0, k3 - k2))
	{
		// Case b:
		if (sameSign(k0, k3) && !rootFindingLemma(k0, k1, k2, k3))
			return false;
		// inside test
		return signDetermination2(k0, k1, k2, k3, face, node, dt);
	}
	else
	{
		// Case c:
		if (sameSign(k0, k3))
			return false;
		// inside test
		return signDetermination1(k0, k1, k2, k3, face, node, dt);
	}
	return false;
}
static SIMD_FORCE_INLINE bool conservativeCulling(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& mrg)
{
	if (k0 > mrg && k1 > mrg && k2 > mrg && k3 > mrg)
		return true;
	if (k0 < -mrg && k1 < -mrg && k2 < -mrg && k3 < -mrg)
		return true;
	return false;
}

static SIMD_FORCE_INLINE bool bernsteinVFTest(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& mrg, const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt)
{
	if (conservativeCulling(k0, k1, k2, k3, mrg))
		return false;
	return coplanarAndInsideTest(k0, k1, k2, k3, face, node, dt);
}

static SIMD_FORCE_INLINE void deCasteljau(const Scalar& k0, const Scalar& k1, const Scalar& k2, const Scalar& k3, const Scalar& t0, Scalar& k10, Scalar& k20, Scalar& k30, Scalar& k21, Scalar& k12)
{
	k10 = k0 * (1.0 - t0) + k1 * t0;
	Scalar k11 = k1 * (1.0 - t0) + k2 * t0;
	k12 = k2 * (1.0 - t0) + k3 * t0;
	k20 = k10 * (1.0 - t0) + k11 * t0;
	k21 = k11 * (1.0 - t0) + k12 * t0;
	k30 = k20 * (1.0 - t0) + k21 * t0;
}
static SIMD_FORCE_INLINE bool bernsteinVFTest(const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt, const Scalar& mrg)
{
	Scalar k0, k1, k2, k3;
	getBernsteinCoeff(face, node, dt, k0, k1, k2, k3);
	if (conservativeCulling(k0, k1, k2, k3, mrg))
		return false;
	return true;
	if (diffSign(k2 - 2.0 * k1 + k0, k3 - 2.0 * k2 + k1))
	{
		Scalar k10, k20, k30, k21, k12;
		Scalar t0 = (k2 - 2.0 * k1 + k0) / (k0 - 3.0 * k1 + 3.0 * k2 - k3);
		deCasteljau(k0, k1, k2, k3, t0, k10, k20, k30, k21, k12);
		return bernsteinVFTest(k0, k10, k20, k30, mrg, face, node, dt) || bernsteinVFTest(k30, k21, k12, k3, mrg, face, node, dt);
	}
	return coplanarAndInsideTest(k0, k1, k2, k3, face, node, dt);
}

static SIMD_FORCE_INLINE bool continuousCollisionDetection(const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt, const Scalar& mrg, Vec3& bary)
{
	if (hasSeparatingPlane(face, node, dt))
		return false;
	Vec3 x21 = face->m_n[1]->m_x - face->m_n[0]->m_x;
	Vec3 x31 = face->m_n[2]->m_x - face->m_n[0]->m_x;
	Vec3 x41 = node->m_x - face->m_n[0]->m_x;
	Vec3 v21 = face->m_n[1]->m_v - face->m_n[0]->m_v;
	Vec3 v31 = face->m_n[2]->m_v - face->m_n[0]->m_v;
	Vec3 v41 = node->m_v - face->m_n[0]->m_v;
	Vec3 a = x21.cross(x31);
	Vec3 b = x21.cross(v31) + v21.cross(x31);
	Vec3 c = v21.cross(v31);
	Vec3 d = x41;
	Vec3 e = v41;
	Scalar a0 = a.dot(d);
	Scalar a1 = a.dot(e) + b.dot(d);
	Scalar a2 = c.dot(d) + b.dot(e);
	Scalar a3 = c.dot(e);
	Scalar eps = SAFE_EPSILON;
	i32 num_roots = 0;
	Scalar roots[3];
	if (std::abs(a3) < eps)
	{
		// cubic term is zero
		if (std::abs(a2) < eps)
		{
			if (std::abs(a1) < eps)
			{
				if (std::abs(a0) < eps)
				{
					num_roots = 2;
					roots[0] = 0;
					roots[1] = dt;
				}
			}
			else
			{
				num_roots = 1;
				roots[0] = -a0 / a1;
			}
		}
		else
		{
			num_roots = SolveP2(roots, a1 / a2, a0 / a2);
		}
	}
	else
	{
		num_roots = SolveP3(roots, a2 / a3, a1 / a3, a0 / a3);
	}
	//    std::sort(roots, roots+num_roots);
	if (num_roots > 1)
	{
		if (roots[0] > roots[1])
			Swap(roots[0], roots[1]);
	}
	if (num_roots > 2)
	{
		if (roots[0] > roots[2])
			Swap(roots[0], roots[2]);
		if (roots[1] > roots[2])
			Swap(roots[1], roots[2]);
	}
	for (i32 r = 0; r < num_roots; ++r)
	{
		double root = roots[r];
		if (root <= 0)
			continue;
		if (root > dt + SIMD_EPSILON)
			return false;
		Vec3 x1 = face->m_n[0]->m_x + root * face->m_n[0]->m_v;
		Vec3 x2 = face->m_n[1]->m_x + root * face->m_n[1]->m_v;
		Vec3 x3 = face->m_n[2]->m_x + root * face->m_n[2]->m_v;
		Vec3 x4 = node->m_x + root * node->m_v;
		Vec3 normal = (x2 - x1).cross(x3 - x1);
		normal.safeNormalize();
		if (proximityTest(x1, x2, x3, x4, normal, mrg, bary))
			return true;
	}
	return false;
}
static SIMD_FORCE_INLINE bool bernsteinCCD(const SoftBody::Face* face, const SoftBody::Node* node, const Scalar& dt, const Scalar& mrg, Vec3& bary)
{
	if (!bernsteinVFTest(face, node, dt, mrg))
		return false;
	if (!continuousCollisionDetection(face, node, dt, 1e-6, bary))
		return false;
	return true;
}

//
// SymMatrix
//
template <typename T>
struct SymMatrix
{
	SymMatrix() : dim(0) {}
	SymMatrix(i32 n, const T& init = T()) { resize(n, init); }
	void resize(i32 n, const T& init = T())
	{
		dim = n;
		store.resize((n * (n + 1)) / 2, init);
	}
	i32 index(i32 c, i32 r) const
	{
		if (c > r) Swap(c, r);
		Assert(r < dim);
		return ((r * (r + 1)) / 2 + c);
	}
	T& operator()(i32 c, i32 r) { return (store[index(c, r)]); }
	const T& operator()(i32 c, i32 r) const { return (store[index(c, r)]); }
	AlignedObjectArray<T> store;
	i32 dim;
};

//
// SoftBodyCollisionShape
//
class SoftBodyCollisionShape : public ConcaveShape
{
public:
	SoftBody* m_body;

	SoftBodyCollisionShape(SoftBody* backptr)
	{
		m_shapeType = SOFTBODY_SHAPE_PROXYTYPE;
		m_body = backptr;
	}

	virtual ~SoftBodyCollisionShape()
	{
	}

	void processAllTriangles(TriangleCallback* /*callback*/, const Vec3& /*aabbMin*/, const Vec3& /*aabbMax*/) const
	{
		//not yet
		Assert(0);
	}

	///getAabb returns the axis aligned bounding box in the coordinate frame of the given transform t.
	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		/* t is usually identity, except when colliding against CompoundShape. See Issue 512 */
		const Vec3 mins = m_body->m_bounds[0];
		const Vec3 maxs = m_body->m_bounds[1];
		const Vec3 crns[] = {t * Vec3(mins.x(), mins.y(), mins.z()),
								  t * Vec3(maxs.x(), mins.y(), mins.z()),
								  t * Vec3(maxs.x(), maxs.y(), mins.z()),
								  t * Vec3(mins.x(), maxs.y(), mins.z()),
								  t * Vec3(mins.x(), mins.y(), maxs.z()),
								  t * Vec3(maxs.x(), mins.y(), maxs.z()),
								  t * Vec3(maxs.x(), maxs.y(), maxs.z()),
								  t * Vec3(mins.x(), maxs.y(), maxs.z())};
		aabbMin = aabbMax = crns[0];
		for (i32 i = 1; i < 8; ++i)
		{
			aabbMin.setMin(crns[i]);
			aabbMax.setMax(crns[i]);
		}
	}

	virtual void setLocalScaling(const Vec3& /*scaling*/)
	{
		///na
	}
	virtual const Vec3& getLocalScaling() const
	{
		static const Vec3 dummy(1, 1, 1);
		return dummy;
	}
	virtual void calculateLocalInertia(Scalar /*mass*/, Vec3& /*inertia*/) const
	{
		///not yet
		Assert(0);
	}
	virtual tukk getName() const
	{
		return "SoftBody";
	}
};

//
// SoftClusterCollisionShape
//
class SoftClusterCollisionShape : public ConvexInternalShape
{
public:
	const SoftBody::Cluster* m_cluster;

	SoftClusterCollisionShape(const SoftBody::Cluster* cluster) : m_cluster(cluster) { setMargin(0); }

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const
	{
		SoftBody::Node* const* n = &m_cluster->m_nodes[0];
		Scalar d = Dot(vec, n[0]->m_x);
		i32 j = 0;
		for (i32 i = 1, ni = m_cluster->m_nodes.size(); i < ni; ++i)
		{
			const Scalar k = Dot(vec, n[i]->m_x);
			if (k > d)
			{
				d = k;
				j = i;
			}
		}
		return (n[j]->m_x);
	}
	virtual Vec3 localGetSupportingVertexWithoutMargin(const Vec3& vec) const
	{
		return (localGetSupportingVertex(vec));
	}
	//notice that the vectors should be unit length
	virtual void batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
	{
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3& inertia) const
	{
	}

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
	}

	virtual i32 getShapeType() const { return SOFTBODY_SHAPE_PROXYTYPE; }

	//debugging
	virtual tukk getName() const { return "SOFTCLUSTER"; }

	virtual void setMargin(Scalar margin)
	{
		ConvexInternalShape::setMargin(margin);
	}
	virtual Scalar getMargin() const
	{
		return ConvexInternalShape::getMargin();
	}
};

//
// Inline's
//

//
template <typename T>
static inline void ZeroInitialize(T& value)
{
	memset(&value, 0, sizeof(T));
}
//
template <typename T>
static inline bool CompLess(const T& a, const T& b)
{
	return (a < b);
}
//
template <typename T>
static inline bool CompGreater(const T& a, const T& b)
{
	return (a > b);
}
//
template <typename T>
static inline T Lerp(const T& a, const T& b, Scalar t)
{
	return (a + (b - a) * t);
}
//
template <typename T>
static inline T InvLerp(const T& a, const T& b, Scalar t)
{
	return ((b + a * t - b * t) / (a * b));
}
//
static inline Matrix3x3 Lerp(const Matrix3x3& a,
							   const Matrix3x3& b,
							   Scalar t)
{
	Matrix3x3 r;
	r[0] = Lerp(a[0], b[0], t);
	r[1] = Lerp(a[1], b[1], t);
	r[2] = Lerp(a[2], b[2], t);
	return (r);
}
//
static inline Vec3 Clamp(const Vec3& v, Scalar maxlength)
{
	const Scalar sql = v.length2();
	if (sql > (maxlength * maxlength))
		return ((v * maxlength) / Sqrt(sql));
	else
		return (v);
}
//
template <typename T>
static inline T Clamp(const T& x, const T& l, const T& h)
{
	return (x < l ? l : x > h ? h : x);
}
//
template <typename T>
static inline T Sq(const T& x)
{
	return (x * x);
}
//
template <typename T>
static inline T Cube(const T& x)
{
	return (x * x * x);
}
//
template <typename T>
static inline T Sign(const T& x)
{
	return ((T)(x < 0 ? -1 : +1));
}
//
template <typename T>
static inline bool SameSign(const T& x, const T& y)
{
	return ((x * y) > 0);
}
//
static inline Scalar ClusterMetric(const Vec3& x, const Vec3& y)
{
	const Vec3 d = x - y;
	return (Fabs(d[0]) + Fabs(d[1]) + Fabs(d[2]));
}
//
static inline Matrix3x3 ScaleAlongAxis(const Vec3& a, Scalar s)
{
	const Scalar xx = a.x() * a.x();
	const Scalar yy = a.y() * a.y();
	const Scalar zz = a.z() * a.z();
	const Scalar xy = a.x() * a.y();
	const Scalar yz = a.y() * a.z();
	const Scalar zx = a.z() * a.x();
	Matrix3x3 m;
	m[0] = Vec3(1 - xx + xx * s, xy * s - xy, zx * s - zx);
	m[1] = Vec3(xy * s - xy, 1 - yy + yy * s, yz * s - yz);
	m[2] = Vec3(zx * s - zx, yz * s - yz, 1 - zz + zz * s);
	return (m);
}
//
static inline Matrix3x3 Cross(const Vec3& v)
{
	Matrix3x3 m;
	m[0] = Vec3(0, -v.z(), +v.y());
	m[1] = Vec3(+v.z(), 0, -v.x());
	m[2] = Vec3(-v.y(), +v.x(), 0);
	return (m);
}
//
static inline Matrix3x3 Diagonal(Scalar x)
{
	Matrix3x3 m;
	m[0] = Vec3(x, 0, 0);
	m[1] = Vec3(0, x, 0);
	m[2] = Vec3(0, 0, x);
	return (m);
}

static inline Matrix3x3 Diagonal(const Vec3& v)
{
	Matrix3x3 m;
	m[0] = Vec3(v.getX(), 0, 0);
	m[1] = Vec3(0, v.getY(), 0);
	m[2] = Vec3(0, 0, v.getZ());
	return (m);
}

static inline Scalar Dot(const Scalar* a, const Scalar* b, i32 ndof)
{
	Scalar result = 0;
	for (i32 i = 0; i < ndof; ++i)
		result += a[i] * b[i];
	return result;
}

static inline Matrix3x3 OuterProduct(const Scalar* v1, const Scalar* v2, const Scalar* v3,
									   const Scalar* u1, const Scalar* u2, const Scalar* u3, i32 ndof)
{
	Matrix3x3 m;
	Scalar a11 = Dot(v1, u1, ndof);
	Scalar a12 = Dot(v1, u2, ndof);
	Scalar a13 = Dot(v1, u3, ndof);

	Scalar a21 = Dot(v2, u1, ndof);
	Scalar a22 = Dot(v2, u2, ndof);
	Scalar a23 = Dot(v2, u3, ndof);

	Scalar a31 = Dot(v3, u1, ndof);
	Scalar a32 = Dot(v3, u2, ndof);
	Scalar a33 = Dot(v3, u3, ndof);
	m[0] = Vec3(a11, a12, a13);
	m[1] = Vec3(a21, a22, a23);
	m[2] = Vec3(a31, a32, a33);
	return (m);
}

static inline Matrix3x3 OuterProduct(const Vec3& v1, const Vec3& v2)
{
	Matrix3x3 m;
	Scalar a11 = v1[0] * v2[0];
	Scalar a12 = v1[0] * v2[1];
	Scalar a13 = v1[0] * v2[2];

	Scalar a21 = v1[1] * v2[0];
	Scalar a22 = v1[1] * v2[1];
	Scalar a23 = v1[1] * v2[2];

	Scalar a31 = v1[2] * v2[0];
	Scalar a32 = v1[2] * v2[1];
	Scalar a33 = v1[2] * v2[2];
	m[0] = Vec3(a11, a12, a13);
	m[1] = Vec3(a21, a22, a23);
	m[2] = Vec3(a31, a32, a33);
	return (m);
}

//
static inline Matrix3x3 Add(const Matrix3x3& a,
							  const Matrix3x3& b)
{
	Matrix3x3 r;
	for (i32 i = 0; i < 3; ++i) r[i] = a[i] + b[i];
	return (r);
}
//
static inline Matrix3x3 Sub(const Matrix3x3& a,
							  const Matrix3x3& b)
{
	Matrix3x3 r;
	for (i32 i = 0; i < 3; ++i) r[i] = a[i] - b[i];
	return (r);
}
//
static inline Matrix3x3 Mul(const Matrix3x3& a,
							  Scalar b)
{
	Matrix3x3 r;
	for (i32 i = 0; i < 3; ++i) r[i] = a[i] * b;
	return (r);
}
//
static inline void Orthogonalize(Matrix3x3& m)
{
	m[2] = Cross(m[0], m[1]).normalized();
	m[1] = Cross(m[2], m[0]).normalized();
	m[0] = Cross(m[1], m[2]).normalized();
}
//
static inline Matrix3x3 MassMatrix(Scalar im, const Matrix3x3& iwi, const Vec3& r)
{
	const Matrix3x3 cr = Cross(r);
	return (Sub(Diagonal(im), cr * iwi * cr));
}

//
static inline Matrix3x3 ImpulseMatrix(Scalar dt,
										Scalar ima,
										Scalar imb,
										const Matrix3x3& iwi,
										const Vec3& r)
{
	return (Diagonal(1 / dt) * Add(Diagonal(ima), MassMatrix(imb, iwi, r)).inverse());
}

//
static inline Matrix3x3 ImpulseMatrix(Scalar dt,
										const Matrix3x3& effective_mass_inv,
										Scalar imb,
										const Matrix3x3& iwi,
										const Vec3& r)
{
	return (Diagonal(1 / dt) * Add(effective_mass_inv, MassMatrix(imb, iwi, r)).inverse());
	//    Matrix3x3 iimb = MassMatrix(imb, iwi, r);
	//    if (iimb.determinant() == 0)
	//        return effective_mass_inv.inverse();
	//    return effective_mass_inv.inverse() *  Add(effective_mass_inv.inverse(), iimb.inverse()).inverse() * iimb.inverse();
}

//
static inline Matrix3x3 ImpulseMatrix(Scalar ima, const Matrix3x3& iia, const Vec3& ra,
										Scalar imb, const Matrix3x3& iib, const Vec3& rb)
{
	return (Add(MassMatrix(ima, iia, ra), MassMatrix(imb, iib, rb)).inverse());
}

//
static inline Matrix3x3 AngularImpulseMatrix(const Matrix3x3& iia,
											   const Matrix3x3& iib)
{
	return (Add(iia, iib).inverse());
}

//
static inline Vec3 ProjectOnAxis(const Vec3& v,
									  const Vec3& a)
{
	return (a * Dot(v, a));
}
//
static inline Vec3 ProjectOnPlane(const Vec3& v,
									   const Vec3& a)
{
	return (v - ProjectOnAxis(v, a));
}

//
static inline void ProjectOrigin(const Vec3& a,
								 const Vec3& b,
								 Vec3& prj,
								 Scalar& sqd)
{
	const Vec3 d = b - a;
	const Scalar m2 = d.length2();
	if (m2 > SIMD_EPSILON)
	{
		const Scalar t = Clamp<Scalar>(-Dot(a, d) / m2, 0, 1);
		const Vec3 p = a + d * t;
		const Scalar l2 = p.length2();
		if (l2 < sqd)
		{
			prj = p;
			sqd = l2;
		}
	}
}
//
static inline void ProjectOrigin(const Vec3& a,
								 const Vec3& b,
								 const Vec3& c,
								 Vec3& prj,
								 Scalar& sqd)
{
	const Vec3& q = Cross(b - a, c - a);
	const Scalar m2 = q.length2();
	if (m2 > SIMD_EPSILON)
	{
		const Vec3 n = q / Sqrt(m2);
		const Scalar k = Dot(a, n);
		const Scalar k2 = k * k;
		if (k2 < sqd)
		{
			const Vec3 p = n * k;
			if ((Dot(Cross(a - p, b - p), q) > 0) &&
				(Dot(Cross(b - p, c - p), q) > 0) &&
				(Dot(Cross(c - p, a - p), q) > 0))
			{
				prj = p;
				sqd = k2;
			}
			else
			{
				ProjectOrigin(a, b, prj, sqd);
				ProjectOrigin(b, c, prj, sqd);
				ProjectOrigin(c, a, prj, sqd);
			}
		}
	}
}

//
static inline bool rayIntersectsTriangle(const Vec3& origin, const Vec3& dir, const Vec3& v0, const Vec3& v1, const Vec3& v2, Scalar& t)
{
	Scalar a, f, u, v;

	Vec3 e1 = v1 - v0;
	Vec3 e2 = v2 - v0;
	Vec3 h = dir.cross(e2);
	a = e1.dot(h);

	if (a > -0.00001 && a < 0.00001)
		return (false);

	f = Scalar(1) / a;
	Vec3 s = origin - v0;
	u = f * s.dot(h);

	if (u < 0.0 || u > 1.0)
		return (false);

	Vec3 q = s.cross(e1);
	v = f * dir.dot(q);
	if (v < 0.0 || u + v > 1.0)
		return (false);
	// at this stage we can compute t to find out where
	// the intersection point is on the line
	t = f * e2.dot(q);
	if (t > 0)  // ray intersection
		return (true);
	else  // this means that there is a line intersection
		// but not a ray intersection
		return (false);
}

static inline bool lineIntersectsTriangle(const Vec3& rayStart, const Vec3& rayEnd, const Vec3& p1, const Vec3& p2, const Vec3& p3, Vec3& sect, Vec3& normal)
{
	Vec3 dir = rayEnd - rayStart;
	Scalar dir_norm = dir.norm();
	if (dir_norm < SIMD_EPSILON)
		return false;
	dir.normalize();
	Scalar t;
	bool ret = rayIntersectsTriangle(rayStart, dir, p1, p2, p3, t);

	if (ret)
	{
		if (t <= dir_norm)
		{
			sect = rayStart + dir * t;
		}
		else
		{
			ret = false;
		}
	}

	if (ret)
	{
		Vec3 n = (p3 - p1).cross(p2 - p1);
		n.safeNormalize();
		if (n.dot(dir) < 0)
			normal = n;
		else
			normal = -n;
	}
	return ret;
}

//
template <typename T>
static inline T BaryEval(const T& a,
						 const T& b,
						 const T& c,
						 const Vec3& coord)
{
	return (a * coord.x() + b * coord.y() + c * coord.z());
}
//
static inline Vec3 BaryCoord(const Vec3& a,
								  const Vec3& b,
								  const Vec3& c,
								  const Vec3& p)
{
	const Scalar w[] = {Cross(a - p, b - p).length(),
						  Cross(b - p, c - p).length(),
						  Cross(c - p, a - p).length()};
	const Scalar isum = 1 / (w[0] + w[1] + w[2]);
	return (Vec3(w[1] * isum, w[2] * isum, w[0] * isum));
}

//
inline static Scalar ImplicitSolve(SoftBody::ImplicitFn* fn,
									 const Vec3& a,
									 const Vec3& b,
									 const Scalar accuracy,
									 i32k maxiterations = 256)
{
	Scalar span[2] = {0, 1};
	Scalar values[2] = {fn->Eval(a), fn->Eval(b)};
	if (values[0] > values[1])
	{
		Swap(span[0], span[1]);
		Swap(values[0], values[1]);
	}
	if (values[0] > -accuracy) return (-1);
	if (values[1] < +accuracy) return (-1);
	for (i32 i = 0; i < maxiterations; ++i)
	{
		const Scalar t = Lerp(span[0], span[1], values[0] / (values[0] - values[1]));
		const Scalar v = fn->Eval(Lerp(a, b, t));
		if ((t <= 0) || (t >= 1)) break;
		if (Fabs(v) < accuracy) return (t);
		if (v < 0)
		{
			span[0] = t;
			values[0] = v;
		}
		else
		{
			span[1] = t;
			values[1] = v;
		}
	}
	return (-1);
}

inline static void EvaluateMedium(const SoftBodyWorldInfo* wfi,
								  const Vec3& x,
								  SoftBody::sMedium& medium)
{
	medium.m_velocity = Vec3(0, 0, 0);
	medium.m_pressure = 0;
	medium.m_density = wfi->air_density;
	if (wfi->water_density > 0)
	{
		const Scalar depth = -(Dot(x, wfi->water_normal) + wfi->water_offset);
		if (depth > 0)
		{
			medium.m_density = wfi->water_density;
			medium.m_pressure = depth * wfi->water_density * wfi->m_gravity.length();
		}
	}
}

//
static inline Vec3 NormalizeAny(const Vec3& v)
{
	const Scalar l = v.length();
	if (l > SIMD_EPSILON)
		return (v / l);
	else
		return (Vec3(0, 0, 0));
}

//
static inline DbvtVolume VolumeOf(const SoftBody::Face& f,
									Scalar margin)
{
	const Vec3* pts[] = {&f.m_n[0]->m_x,
							  &f.m_n[1]->m_x,
							  &f.m_n[2]->m_x};
	DbvtVolume vol = DbvtVolume::FromPoints(pts, 3);
	vol.Expand(Vec3(margin, margin, margin));
	return (vol);
}

//
static inline Vec3 CenterOf(const SoftBody::Face& f)
{
	return ((f.m_n[0]->m_x + f.m_n[1]->m_x + f.m_n[2]->m_x) / 3);
}

//
static inline Scalar AreaOf(const Vec3& x0,
							  const Vec3& x1,
							  const Vec3& x2)
{
	const Vec3 a = x1 - x0;
	const Vec3 b = x2 - x0;
	const Vec3 cr = Cross(a, b);
	const Scalar area = cr.length();
	return (area);
}

//
static inline Scalar VolumeOf(const Vec3& x0,
								const Vec3& x1,
								const Vec3& x2,
								const Vec3& x3)
{
	const Vec3 a = x1 - x0;
	const Vec3 b = x2 - x0;
	const Vec3 c = x3 - x0;
	return (Dot(a, Cross(b, c)));
}

//

//
static inline void ApplyClampedForce(SoftBody::Node& n,
									 const Vec3& f,
									 Scalar dt)
{
	const Scalar dtim = dt * n.m_im;
	if ((f * dtim).length2() > n.m_v.length2())
	{ /* Clamp	*/
		n.m_f -= ProjectOnAxis(n.m_v, f.normalized()) / dtim;
	}
	else
	{ /* Apply	*/
		n.m_f += f;
	}
}

//
static inline i32 MatchEdge(const SoftBody::Node* a,
							const SoftBody::Node* b,
							const SoftBody::Node* ma,
							const SoftBody::Node* mb)
{
	if ((a == ma) && (b == mb)) return (0);
	if ((a == mb) && (b == ma)) return (1);
	return (-1);
}

//
// Eigen : Extract eigen system,
// straitforward implementation of http://math.fullerton.edu/mathews/n2003/JacobiMethodMod.html
// outputs are NOT sorted.
//
struct Eigen
{
	static i32 system(Matrix3x3& a, Matrix3x3* vectors, Vec3* values = 0)
	{
		static i32k maxiterations = 16;
		static const Scalar accuracy = (Scalar)0.0001;
		Matrix3x3& v = *vectors;
		i32 iterations = 0;
		vectors->setIdentity();
		do
		{
			i32 p = 0, q = 1;
			if (Fabs(a[p][q]) < Fabs(a[0][2]))
			{
				p = 0;
				q = 2;
			}
			if (Fabs(a[p][q]) < Fabs(a[1][2]))
			{
				p = 1;
				q = 2;
			}
			if (Fabs(a[p][q]) > accuracy)
			{
				const Scalar w = (a[q][q] - a[p][p]) / (2 * a[p][q]);
				const Scalar z = Fabs(w);
				const Scalar t = w / (z * (Sqrt(1 + w * w) + z));
				if (t == t) /* [WARNING] let hope that one does not get thrown aways by some compilers... */
				{
					const Scalar c = 1 / Sqrt(t * t + 1);
					const Scalar s = c * t;
					mulPQ(a, c, s, p, q);
					mulTPQ(a, c, s, p, q);
					mulPQ(v, c, s, p, q);
				}
				else
					break;
			}
			else
				break;
		} while ((++iterations) < maxiterations);
		if (values)
		{
			*values = Vec3(a[0][0], a[1][1], a[2][2]);
		}
		return (iterations);
	}

private:
	static inline void mulTPQ(Matrix3x3& a, Scalar c, Scalar s, i32 p, i32 q)
	{
		const Scalar m[2][3] = {{a[p][0], a[p][1], a[p][2]},
								  {a[q][0], a[q][1], a[q][2]}};
		i32 i;

		for (i = 0; i < 3; ++i) a[p][i] = c * m[0][i] - s * m[1][i];
		for (i = 0; i < 3; ++i) a[q][i] = c * m[1][i] + s * m[0][i];
	}
	static inline void mulPQ(Matrix3x3& a, Scalar c, Scalar s, i32 p, i32 q)
	{
		const Scalar m[2][3] = {{a[0][p], a[1][p], a[2][p]},
								  {a[0][q], a[1][q], a[2][q]}};
		i32 i;

		for (i = 0; i < 3; ++i) a[i][p] = c * m[0][i] - s * m[1][i];
		for (i = 0; i < 3; ++i) a[i][q] = c * m[1][i] + s * m[0][i];
	}
};

//
// Polar decomposition,
// "Computing the Polar Decomposition with Applications", Nicholas J. Higham, 1986.
//
static inline i32 PolarDecompose(const Matrix3x3& m, Matrix3x3& q, Matrix3x3& s)
{
	static const class PolarDecomposition polar;
	return polar.decompose(m, q, s);
}

//
// SoftColliders
//
struct SoftColliders
{
	//
	// ClusterBase
	//
	struct ClusterBase : Dbvt::ICollide
	{
		Scalar erp;
		Scalar idt;
		Scalar m_margin;
		Scalar friction;
		Scalar threshold;
		ClusterBase()
		{
			erp = (Scalar)1;
			idt = 0;
			m_margin = 0;
			friction = 0;
			threshold = (Scalar)0;
		}
		bool SolveContact(const GjkEpaSolver2::sResults& res,
						  SoftBody::Body ba, const SoftBody::Body bb,
						  SoftBody::CJoint& joint)
		{
			if (res.distance < m_margin)
			{
				Vec3 norm = res.normal;
				norm.normalize();  //is it necessary?

				const Vec3 ra = res.witnesses[0] - ba.xform().getOrigin();
				const Vec3 rb = res.witnesses[1] - bb.xform().getOrigin();
				const Vec3 va = ba.velocity(ra);
				const Vec3 vb = bb.velocity(rb);
				const Vec3 vrel = va - vb;
				const Scalar rvac = Dot(vrel, norm);
				Scalar depth = res.distance - m_margin;

				//				printf("depth=%f\n",depth);
				const Vec3 iv = norm * rvac;
				const Vec3 fv = vrel - iv;
				joint.m_bodies[0] = ba;
				joint.m_bodies[1] = bb;
				joint.m_refs[0] = ra * ba.xform().getBasis();
				joint.m_refs[1] = rb * bb.xform().getBasis();
				joint.m_rpos[0] = ra;
				joint.m_rpos[1] = rb;
				joint.m_cfm = 1;
				joint.m_erp = 1;
				joint.m_life = 0;
				joint.m_maxlife = 0;
				joint.m_split = 1;

				joint.m_drift = depth * norm;

				joint.m_normal = norm;
				//				printf("normal=%f,%f,%f\n",res.normal.getX(),res.normal.getY(),res.normal.getZ());
				joint.m_delete = false;
				joint.m_friction = fv.length2() < (rvac * friction * rvac * friction) ? 1 : friction;
				joint.m_massmatrix = ImpulseMatrix(ba.invMass(), ba.invWorldInertia(), joint.m_rpos[0],
												   bb.invMass(), bb.invWorldInertia(), joint.m_rpos[1]);

				return (true);
			}
			return (false);
		}
	};
	//
	// CollideCL_RS
	//
	struct CollideCL_RS : ClusterBase
	{
		SoftBody* psb;
		const CollisionObject2Wrapper* m_colObjWrap;

		void Process(const DbvtNode* leaf)
		{
			SoftBody::Cluster* cluster = (SoftBody::Cluster*)leaf->data;
			SoftClusterCollisionShape cshape(cluster);

			const ConvexShape* rshape = (const ConvexShape*)m_colObjWrap->getCollisionShape();

			///don't collide an anchored cluster with a static/kinematic object
			if (m_colObjWrap->getCollisionObject()->isStaticOrKinematicObject() && cluster->m_containsAnchor)
				return;

			GjkEpaSolver2::sResults res;
			if (GjkEpaSolver2::SignedDistance(&cshape, Transform2::getIdentity(),
												rshape, m_colObjWrap->getWorldTransform(),
												Vec3(1, 0, 0), res))
			{
				SoftBody::CJoint joint;
				if (SolveContact(res, cluster, m_colObjWrap->getCollisionObject(), joint))  //prb,joint))
				{
					SoftBody::CJoint* pj = new (AlignedAlloc(sizeof(SoftBody::CJoint), 16)) SoftBody::CJoint();
					*pj = joint;
					psb->m_joints.push_back(pj);
					if (m_colObjWrap->getCollisionObject()->isStaticOrKinematicObject())
					{
						pj->m_erp *= psb->m_cfg.kSKHR_CL;
						pj->m_split *= psb->m_cfg.kSK_SPLT_CL;
					}
					else
					{
						pj->m_erp *= psb->m_cfg.kSRHR_CL;
						pj->m_split *= psb->m_cfg.kSR_SPLT_CL;
					}
				}
			}
		}
		void ProcessColObj(SoftBody* ps, const CollisionObject2Wrapper* colObWrap)
		{
			psb = ps;
			m_colObjWrap = colObWrap;
			idt = ps->m_sst.isdt;
			m_margin = m_colObjWrap->getCollisionShape()->getMargin() + psb->getCollisionShape()->getMargin();
			///drx3D rigid body uses multiply instead of minimum to determine combined friction. Some customization would be useful.
			friction = d3Min(psb->m_cfg.kDF, m_colObjWrap->getCollisionObject()->getFriction());
			Vec3 mins;
			Vec3 maxs;

			ATTRIBUTE_ALIGNED16(DbvtVolume)
			volume;
			colObWrap->getCollisionShape()->getAabb(colObWrap->getWorldTransform(), mins, maxs);
			volume = DbvtVolume::FromMM(mins, maxs);
			volume.Expand(Vec3(1, 1, 1) * m_margin);
			ps->m_cdbvt.collideTV(ps->m_cdbvt.m_root, volume, *this);
		}
	};
	//
	// CollideCL_SS
	//
	struct CollideCL_SS : ClusterBase
	{
		SoftBody* bodies[2];
		void Process(const DbvtNode* la, const DbvtNode* lb)
		{
			SoftBody::Cluster* cla = (SoftBody::Cluster*)la->data;
			SoftBody::Cluster* clb = (SoftBody::Cluster*)lb->data;

			bool connected = false;
			if ((bodies[0] == bodies[1]) && (bodies[0]->m_clusterConnectivity.size()))
			{
				connected = bodies[0]->m_clusterConnectivity[cla->m_clusterIndex + bodies[0]->m_clusters.size() * clb->m_clusterIndex];
			}

			if (!connected)
			{
				SoftClusterCollisionShape csa(cla);
				SoftClusterCollisionShape csb(clb);
				GjkEpaSolver2::sResults res;
				if (GjkEpaSolver2::SignedDistance(&csa, Transform2::getIdentity(),
													&csb, Transform2::getIdentity(),
													cla->m_com - clb->m_com, res))
				{
					SoftBody::CJoint joint;
					if (SolveContact(res, cla, clb, joint))
					{
						SoftBody::CJoint* pj = new (AlignedAlloc(sizeof(SoftBody::CJoint), 16)) SoftBody::CJoint();
						*pj = joint;
						bodies[0]->m_joints.push_back(pj);
						pj->m_erp *= d3Max(bodies[0]->m_cfg.kSSHR_CL, bodies[1]->m_cfg.kSSHR_CL);
						pj->m_split *= (bodies[0]->m_cfg.kSS_SPLT_CL + bodies[1]->m_cfg.kSS_SPLT_CL) / 2;
					}
				}
			}
			else
			{
				static i32 count = 0;
				count++;
				//printf("count=%d\n",count);
			}
		}
		void ProcessSoftSoft(SoftBody* psa, SoftBody* psb)
		{
			idt = psa->m_sst.isdt;
			//m_margin		=	(psa->getCollisionShape()->getMargin()+psb->getCollisionShape()->getMargin())/2;
			m_margin = (psa->getCollisionShape()->getMargin() + psb->getCollisionShape()->getMargin());
			friction = d3Min(psa->m_cfg.kDF, psb->m_cfg.kDF);
			bodies[0] = psa;
			bodies[1] = psb;
			psa->m_cdbvt.collideTT(psa->m_cdbvt.m_root, psb->m_cdbvt.m_root, *this);
		}
	};
	//
	// CollideSDF_RS
	//
	struct CollideSDF_RS : Dbvt::ICollide
	{
		void Process(const DbvtNode* leaf)
		{
			SoftBody::Node* node = (SoftBody::Node*)leaf->data;
			DoNode(*node);
		}
		void DoNode(SoftBody::Node& n) const
		{
			const Scalar m = n.m_im > 0 ? dynmargin : stamargin;
			SoftBody::RContact c;

			if ((!n.m_battach) &&
				psb->checkContact(m_colObj1Wrap, n.m_x, m, c.m_cti))
			{
				const Scalar ima = n.m_im;
				const Scalar imb = m_rigidBody ? m_rigidBody->getInvMass() : 0.f;
				const Scalar ms = ima + imb;
				if (ms > 0)
				{
					const Transform2& wtr = m_rigidBody ? m_rigidBody->getWorldTransform() : m_colObj1Wrap->getCollisionObject()->getWorldTransform();
					static const Matrix3x3 iwiStatic(0, 0, 0, 0, 0, 0, 0, 0, 0);
					const Matrix3x3& iwi = m_rigidBody ? m_rigidBody->getInvInertiaTensorWorld() : iwiStatic;
					const Vec3 ra = n.m_x - wtr.getOrigin();
					const Vec3 va = m_rigidBody ? m_rigidBody->getVelocityInLocalPoint(ra) * psb->m_sst.sdt : Vec3(0, 0, 0);
					const Vec3 vb = n.m_x - n.m_q;
					const Vec3 vr = vb - va;
					const Scalar dn = Dot(vr, c.m_cti.m_normal);
					const Vec3 fv = vr - c.m_cti.m_normal * dn;
					const Scalar fc = psb->m_cfg.kDF * m_colObj1Wrap->getCollisionObject()->getFriction();
					c.m_node = &n;
					c.m_c0 = ImpulseMatrix(psb->m_sst.sdt, ima, imb, iwi, ra);
					c.m_c1 = ra;
					c.m_c2 = ima * psb->m_sst.sdt;
					c.m_c3 = fv.length2() < (dn * fc * dn * fc) ? 0 : 1 - fc;
					c.m_c4 = m_colObj1Wrap->getCollisionObject()->isStaticOrKinematicObject() ? psb->m_cfg.kKHR : psb->m_cfg.kCHR;
					psb->m_rcontacts.push_back(c);
					if (m_rigidBody)
						m_rigidBody->activate();
				}
			}
		}
		SoftBody* psb;
		const CollisionObject2Wrapper* m_colObj1Wrap;
		RigidBody* m_rigidBody;
		Scalar dynmargin;
		Scalar stamargin;
	};

	//
	// CollideSDF_RD
	//
	struct CollideSDF_RD : Dbvt::ICollide
	{
		void Process(const DbvtNode* leaf)
		{
			SoftBody::Node* node = (SoftBody::Node*)leaf->data;
			DoNode(*node);
		}
		void DoNode(SoftBody::Node& n) const
		{
			const Scalar m = n.m_im > 0 ? dynmargin : stamargin;
			SoftBody::DeformableNodeRigidContact c;

			if (!n.m_battach)
			{
				// check for collision at x_{n+1}^*
				if (psb->checkDeformableContact(m_colObj1Wrap, n.m_q, m, c.m_cti, /*predict = */ true))
				{
					const Scalar ima = n.m_im;
					// todo: collision between multibody and fixed deformable node will be missed.
					const Scalar imb = m_rigidBody ? m_rigidBody->getInvMass() : 0.f;
					const Scalar ms = ima + imb;
					if (ms > 0)
					{
						// resolve contact at x_n
						psb->checkDeformableContact(m_colObj1Wrap, n.m_x, m, c.m_cti, /*predict = */ false);
						SoftBody::sCti& cti = c.m_cti;
						c.m_node = &n;
						const Scalar fc = psb->m_cfg.kDF * m_colObj1Wrap->getCollisionObject()->getFriction();
						c.m_c2 = ima;
						c.m_c3 = fc;
						c.m_c4 = m_colObj1Wrap->getCollisionObject()->isStaticOrKinematicObject() ? psb->m_cfg.kKHR : psb->m_cfg.kCHR;
						c.m_c5 = n.m_effectiveMass_inv;

						if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
						{
							const Transform2& wtr = m_rigidBody ? m_rigidBody->getWorldTransform() : m_colObj1Wrap->getCollisionObject()->getWorldTransform();
							const Vec3 ra = n.m_x - wtr.getOrigin();

							static const Matrix3x3 iwiStatic(0, 0, 0, 0, 0, 0, 0, 0, 0);
							const Matrix3x3& iwi = m_rigidBody ? m_rigidBody->getInvInertiaTensorWorld() : iwiStatic;
							if (psb->m_reducedModel)
							{
								c.m_c0 = MassMatrix(imb, iwi, ra); //impulse factor K of the rigid body only (not the inverse)
							}
							else
							{
								c.m_c0 = ImpulseMatrix(1, n.m_effectiveMass_inv, imb, iwi, ra);
								//                            c.m_c0 = ImpulseMatrix(1, ima, imb, iwi, ra);
							}
							c.m_c1 = ra;
						}
						else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
						{
							MultiBodyLinkCollider* multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
							if (multibodyLinkCol)
							{
								Vec3 normal = cti.m_normal;
								Vec3 t1 = generateUnitOrthogonalVector(normal);
								Vec3 t2 = Cross(normal, t1);
								MultiBodyJacobianData jacobianData_normal, jacobianData_t1, jacobianData_t2;
								findJacobian(multibodyLinkCol, jacobianData_normal, c.m_node->m_x, normal);
								findJacobian(multibodyLinkCol, jacobianData_t1, c.m_node->m_x, t1);
								findJacobian(multibodyLinkCol, jacobianData_t2, c.m_node->m_x, t2);

								Scalar* J_n = &jacobianData_normal.m_jacobians[0];
								Scalar* J_t1 = &jacobianData_t1.m_jacobians[0];
								Scalar* J_t2 = &jacobianData_t2.m_jacobians[0];

								Scalar* u_n = &jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
								Scalar* u_t1 = &jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
								Scalar* u_t2 = &jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];

								Matrix3x3 rot(normal.getX(), normal.getY(), normal.getZ(),
												t1.getX(), t1.getY(), t1.getZ(),
												t2.getX(), t2.getY(), t2.getZ());  // world frame to local frame
								i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
								
								Matrix3x3 local_impulse_matrix;
								if (psb->m_reducedModel)
								{
									local_impulse_matrix = OuterProduct(J_n, J_t1, J_t2, u_n, u_t1, u_t2, ndof);
								}
								else
								{
									local_impulse_matrix = (n.m_effectiveMass_inv + OuterProduct(J_n, J_t1, J_t2, u_n, u_t1, u_t2, ndof)).inverse();
								}
								c.m_c0 = rot.transpose() * local_impulse_matrix * rot;
								c.jacobianData_normal = jacobianData_normal;
								c.jacobianData_t1 = jacobianData_t1;
								c.jacobianData_t2 = jacobianData_t2;
								c.t1 = t1;
								c.t2 = t2;
							}
						}
						psb->m_nodeRigidContacts.push_back(c);
					}
				}
			}
		}
		SoftBody* psb;
		const CollisionObject2Wrapper* m_colObj1Wrap;
		RigidBody* m_rigidBody;
		Scalar dynmargin;
		Scalar stamargin;
	};

	//
	// CollideSDF_RDF
	//
	struct CollideSDF_RDF : Dbvt::ICollide
	{
		void Process(const DbvtNode* leaf)
		{
			SoftBody::Face* face = (SoftBody::Face*)leaf->data;
			DoNode(*face);
		}
		void DoNode(SoftBody::Face& f) const
		{
			SoftBody::Node* n0 = f.m_n[0];
			SoftBody::Node* n1 = f.m_n[1];
			SoftBody::Node* n2 = f.m_n[2];
			const Scalar m = (n0->m_im > 0 && n1->m_im > 0 && n2->m_im > 0) ? dynmargin : stamargin;
			SoftBody::DeformableFaceRigidContact c;
			Vec3 contact_point;
			Vec3 bary;
			if (psb->checkDeformableFaceContact(m_colObj1Wrap, f, contact_point, bary, m, c.m_cti, true))
			{
				Scalar ima = n0->m_im + n1->m_im + n2->m_im;
				const Scalar imb = m_rigidBody ? m_rigidBody->getInvMass() : 0.f;
				// todo: collision between multibody and fixed deformable face will be missed.
				const Scalar ms = ima + imb;
				if (ms > 0)
				{
					// resolve contact at x_n
					//                    psb->checkDeformableFaceContact(m_colObj1Wrap, f, contact_point, bary, m, c.m_cti, /*predict = */ false);
					SoftBody::sCti& cti = c.m_cti;
					c.m_contactPoint = contact_point;
					c.m_bary = bary;
					// todo xuchenhan@: this is assuming mass of all vertices are the same. Need to modify if mass are different for distinct vertices
					c.m_weights = Scalar(2) / (Scalar(1) + bary.length2()) * bary;
					c.m_face = &f;
					// friction is handled by the nodes to prevent sticking
					//                    const Scalar fc = 0;
					const Scalar fc = psb->m_cfg.kDF * m_colObj1Wrap->getCollisionObject()->getFriction();

					// the effective inverse mass of the face as in https://graphics.stanford.edu/papers/cloth-sig02/cloth.pdf
					ima = bary.getX() * c.m_weights.getX() * n0->m_im + bary.getY() * c.m_weights.getY() * n1->m_im + bary.getZ() * c.m_weights.getZ() * n2->m_im;
					c.m_c2 = ima;
					c.m_c3 = fc;
					c.m_c4 = m_colObj1Wrap->getCollisionObject()->isStaticOrKinematicObject() ? psb->m_cfg.kKHR : psb->m_cfg.kCHR;
					c.m_c5 = Diagonal(ima);
					if (cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY)
					{
						const Transform2& wtr = m_rigidBody ? m_rigidBody->getWorldTransform() : m_colObj1Wrap->getCollisionObject()->getWorldTransform();
						static const Matrix3x3 iwiStatic(0, 0, 0, 0, 0, 0, 0, 0, 0);
						const Matrix3x3& iwi = m_rigidBody ? m_rigidBody->getInvInertiaTensorWorld() : iwiStatic;
						const Vec3 ra = contact_point - wtr.getOrigin();

						// we do not scale the impulse matrix by dt
						c.m_c0 = ImpulseMatrix(1, ima, imb, iwi, ra);
						c.m_c1 = ra;
					}
					else if (cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK)
					{
						MultiBodyLinkCollider* multibodyLinkCol = (MultiBodyLinkCollider*)MultiBodyLinkCollider::upcast(cti.m_colObj);
						if (multibodyLinkCol)
						{
							Vec3 normal = cti.m_normal;
							Vec3 t1 = generateUnitOrthogonalVector(normal);
							Vec3 t2 = Cross(normal, t1);
							MultiBodyJacobianData jacobianData_normal, jacobianData_t1, jacobianData_t2;
							findJacobian(multibodyLinkCol, jacobianData_normal, contact_point, normal);
							findJacobian(multibodyLinkCol, jacobianData_t1, contact_point, t1);
							findJacobian(multibodyLinkCol, jacobianData_t2, contact_point, t2);

							Scalar* J_n = &jacobianData_normal.m_jacobians[0];
							Scalar* J_t1 = &jacobianData_t1.m_jacobians[0];
							Scalar* J_t2 = &jacobianData_t2.m_jacobians[0];

							Scalar* u_n = &jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
							Scalar* u_t1 = &jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
							Scalar* u_t2 = &jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];

							Matrix3x3 rot(normal.getX(), normal.getY(), normal.getZ(),
											t1.getX(), t1.getY(), t1.getZ(),
											t2.getX(), t2.getY(), t2.getZ());  // world frame to local frame
							i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
							Matrix3x3 local_impulse_matrix = (Diagonal(ima) + OuterProduct(J_n, J_t1, J_t2, u_n, u_t1, u_t2, ndof)).inverse();
							c.m_c0 = rot.transpose() * local_impulse_matrix * rot;
							c.jacobianData_normal = jacobianData_normal;
							c.jacobianData_t1 = jacobianData_t1;
							c.jacobianData_t2 = jacobianData_t2;
							c.t1 = t1;
							c.t2 = t2;
						}
					}
					psb->m_faceRigidContacts.push_back(c);
				}
			}
			// Set caching barycenters to be false after collision detection.
			// Only turn on when contact is static.
			f.m_pcontact[3] = 0;
		}
		SoftBody* psb;
		const CollisionObject2Wrapper* m_colObj1Wrap;
		RigidBody* m_rigidBody;
		Scalar dynmargin;
		Scalar stamargin;
	};

	//
	// CollideVF_SS
	//
	struct CollideVF_SS : Dbvt::ICollide
	{
		void Process(const DbvtNode* lnode,
					 const DbvtNode* lface)
		{
			SoftBody::Node* node = (SoftBody::Node*)lnode->data;
			SoftBody::Face* face = (SoftBody::Face*)lface->data;
			for (i32 i = 0; i < 3; ++i)
			{
				if (face->m_n[i] == node)
					continue;
			}

			Vec3 o = node->m_x;
			Vec3 p;
			Scalar d = SIMD_INFINITY;
			ProjectOrigin(face->m_n[0]->m_x - o,
						  face->m_n[1]->m_x - o,
						  face->m_n[2]->m_x - o,
						  p, d);
			const Scalar m = mrg + (o - node->m_q).length() * 2;
			if (d < (m * m))
			{
				const SoftBody::Node* n[] = {face->m_n[0], face->m_n[1], face->m_n[2]};
				const Vec3 w = BaryCoord(n[0]->m_x, n[1]->m_x, n[2]->m_x, p + o);
				const Scalar ma = node->m_im;
				Scalar mb = BaryEval(n[0]->m_im, n[1]->m_im, n[2]->m_im, w);
				if ((n[0]->m_im <= 0) ||
					(n[1]->m_im <= 0) ||
					(n[2]->m_im <= 0))
				{
					mb = 0;
				}
				const Scalar ms = ma + mb;
				if (ms > 0)
				{
					SoftBody::SContact c;
					c.m_normal = p / -Sqrt(d);
					c.m_margin = m;
					c.m_node = node;
					c.m_face = face;
					c.m_weights = w;
					c.m_friction = d3Max(psb[0]->m_cfg.kDF, psb[1]->m_cfg.kDF);
					c.m_cfm[0] = ma / ms * psb[0]->m_cfg.kSHR;
					c.m_cfm[1] = mb / ms * psb[1]->m_cfg.kSHR;
					psb[0]->m_scontacts.push_back(c);
				}
			}
		}
		SoftBody* psb[2];
		Scalar mrg;
	};

	//
	// CollideVF_DD
	//
	struct CollideVF_DD : Dbvt::ICollide
	{
		void Process(const DbvtNode* lnode,
					 const DbvtNode* lface)
		{
			SoftBody::Node* node = (SoftBody::Node*)lnode->data;
			SoftBody::Face* face = (SoftBody::Face*)lface->data;
			Vec3 bary;
			if (proximityTest(face->m_n[0]->m_x, face->m_n[1]->m_x, face->m_n[2]->m_x, node->m_x, face->m_normal, mrg, bary))
			{
				const SoftBody::Node* n[] = {face->m_n[0], face->m_n[1], face->m_n[2]};
				const Vec3 w = bary;
				const Scalar ma = node->m_im;
				Scalar mb = BaryEval(n[0]->m_im, n[1]->m_im, n[2]->m_im, w);
				if ((n[0]->m_im <= 0) ||
					(n[1]->m_im <= 0) ||
					(n[2]->m_im <= 0))
				{
					mb = 0;
				}
				const Scalar ms = ma + mb;
				if (ms > 0)
				{
					SoftBody::DeformableFaceNodeContact c;
					c.m_normal = face->m_normal;
					if (!useFaceNormal && c.m_normal.dot(node->m_x - face->m_n[2]->m_x) < 0)
						c.m_normal = -face->m_normal;
					c.m_margin = mrg;
					c.m_node = node;
					c.m_face = face;
					c.m_bary = w;
					c.m_friction = psb[0]->m_cfg.kDF * psb[1]->m_cfg.kDF;
					// Initialize unused fields.
					c.m_weights = Vec3(0, 0, 0);
					c.m_imf = 0;
					c.m_c0 = 0;
					c.m_colObj = psb[1];
					psb[0]->m_faceNodeContacts.push_back(c);
				}
			}
		}
		SoftBody* psb[2];
		Scalar mrg;
		bool useFaceNormal;
	};

	//
	// CollideFF_DD
	//
	struct CollideFF_DD : Dbvt::ICollide
	{
		void Process(const DbvntNode* lface1,
					 const DbvntNode* lface2)
		{
			SoftBody::Face* f1 = (SoftBody::Face*)lface1->data;
			SoftBody::Face* f2 = (SoftBody::Face*)lface2->data;
			if (f1 != f2)
			{
				Repel(f1, f2);
				Repel(f2, f1);
			}
		}
		void Repel(SoftBody::Face* f1, SoftBody::Face* f2)
		{
			//#define REPEL_NEIGHBOR 1
#ifndef REPEL_NEIGHBOR
			for (i32 node_id = 0; node_id < 3; ++node_id)
			{
				SoftBody::Node* node = f1->m_n[node_id];
				for (i32 i = 0; i < 3; ++i)
				{
					if (f2->m_n[i] == node)
						return;
				}
			}
#endif
			bool skip = false;
			for (i32 node_id = 0; node_id < 3; ++node_id)
			{
				SoftBody::Node* node = f1->m_n[node_id];
#ifdef REPEL_NEIGHBOR
				for (i32 i = 0; i < 3; ++i)
				{
					if (f2->m_n[i] == node)
					{
						skip = true;
						break;
					}
				}
				if (skip)
				{
					skip = false;
					continue;
				}
#endif
				SoftBody::Face* face = f2;
				Vec3 bary;
				if (!proximityTest(face->m_n[0]->m_x, face->m_n[1]->m_x, face->m_n[2]->m_x, node->m_x, face->m_normal, mrg, bary))
					continue;
				SoftBody::DeformableFaceNodeContact c;
				c.m_normal = face->m_normal;
				if (!useFaceNormal && c.m_normal.dot(node->m_x - face->m_n[2]->m_x) < 0)
					c.m_normal = -face->m_normal;
				c.m_margin = mrg;
				c.m_node = node;
				c.m_face = face;
				c.m_bary = bary;
				c.m_friction = psb[0]->m_cfg.kDF * psb[1]->m_cfg.kDF;
				// Initialize unused fields.
				c.m_weights = Vec3(0, 0, 0);
				c.m_imf = 0;
				c.m_c0 = 0;
				c.m_colObj = psb[1];
				psb[0]->m_faceNodeContacts.push_back(c);
			}
		}
		SoftBody* psb[2];
		Scalar mrg;
		bool useFaceNormal;
	};

	struct CollideCCD : Dbvt::ICollide
	{
		void Process(const DbvtNode* lnode,
					 const DbvtNode* lface)
		{
			SoftBody::Node* node = (SoftBody::Node*)lnode->data;
			SoftBody::Face* face = (SoftBody::Face*)lface->data;
			Vec3 bary;
			if (bernsteinCCD(face, node, dt, SAFE_EPSILON, bary))
			{
				SoftBody::DeformableFaceNodeContact c;
				c.m_normal = face->m_normal;
				if (!useFaceNormal && c.m_normal.dot(node->m_x - face->m_n[2]->m_x) < 0)
					c.m_normal = -face->m_normal;
				c.m_node = node;
				c.m_face = face;
				c.m_bary = bary;
				c.m_friction = psb[0]->m_cfg.kDF * psb[1]->m_cfg.kDF;
				// Initialize unused fields.
				c.m_weights = Vec3(0, 0, 0);
				c.m_margin = mrg;
				c.m_imf = 0;
				c.m_c0 = 0;
				c.m_colObj = psb[1];
				psb[0]->m_faceNodeContactsCCD.push_back(c);
			}
		}
		void Process(const DbvntNode* lface1,
					 const DbvntNode* lface2)
		{
			SoftBody::Face* f1 = (SoftBody::Face*)lface1->data;
			SoftBody::Face* f2 = (SoftBody::Face*)lface2->data;
			if (f1 != f2)
			{
				Repel(f1, f2);
				Repel(f2, f1);
			}
		}
		void Repel(SoftBody::Face* f1, SoftBody::Face* f2)
		{
			//#define REPEL_NEIGHBOR 1
#ifndef REPEL_NEIGHBOR
			for (i32 node_id = 0; node_id < 3; ++node_id)
			{
				SoftBody::Node* node = f1->m_n[node_id];
				for (i32 i = 0; i < 3; ++i)
				{
					if (f2->m_n[i] == node)
						return;
				}
			}
#endif
			bool skip = false;
			for (i32 node_id = 0; node_id < 3; ++node_id)
			{
				SoftBody::Node* node = f1->m_n[node_id];
#ifdef REPEL_NEIGHBOR
				for (i32 i = 0; i < 3; ++i)
				{
					if (f2->m_n[i] == node)
					{
						skip = true;
						break;
					}
				}
				if (skip)
				{
					skip = false;
					continue;
				}
#endif
				SoftBody::Face* face = f2;
				Vec3 bary;
				if (bernsteinCCD(face, node, dt, SAFE_EPSILON, bary))
				{
					SoftBody::DeformableFaceNodeContact c;
					c.m_normal = face->m_normal;
					if (!useFaceNormal && c.m_normal.dot(node->m_x - face->m_n[2]->m_x) < 0)
						c.m_normal = -face->m_normal;
					c.m_node = node;
					c.m_face = face;
					c.m_bary = bary;
					c.m_friction = psb[0]->m_cfg.kDF * psb[1]->m_cfg.kDF;
					// Initialize unused fields.
					c.m_weights = Vec3(0, 0, 0);
					c.m_margin = mrg;
					c.m_imf = 0;
					c.m_c0 = 0;
					c.m_colObj = psb[1];
					psb[0]->m_faceNodeContactsCCD.push_back(c);
				}
			}
		}
		SoftBody* psb[2];
		Scalar dt, mrg;
		bool useFaceNormal;
	};
};
#endif  //_DRX3D_SOFT_BODY_INTERNALS_H
