
#include <drx3D/Physics/Dynamics/ConstraintSolver/Solve2LinearConstraint.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/JacobianEntry.h>

void Solve2LinearConstraint::resolveUnilateralPairConstraint(
	RigidBody* body1,
	RigidBody* body2,

	const Matrix3x3& world2A,
	const Matrix3x3& world2B,

	const Vec3& invInertiaADiag,
	const Scalar invMassA,
	const Vec3& linvelA, const Vec3& angvelA,
	const Vec3& rel_posA1,
	const Vec3& invInertiaBDiag,
	const Scalar invMassB,
	const Vec3& linvelB, const Vec3& angvelB,
	const Vec3& rel_posA2,

	Scalar depthA, const Vec3& normalA,
	const Vec3& rel_posB1, const Vec3& rel_posB2,
	Scalar depthB, const Vec3& normalB,
	Scalar& imp0, Scalar& imp1)
{
	(void)linvelA;
	(void)linvelB;
	(void)angvelB;
	(void)angvelA;

	imp0 = Scalar(0.);
	imp1 = Scalar(0.);

	Scalar len = Fabs(normalA.length()) - Scalar(1.);
	if (Fabs(len) >= SIMD_EPSILON)
		return;

	Assert(len < SIMD_EPSILON);

	//this jacobian entry could be re-used for all iterations
	JacobianEntry jacA(world2A, world2B, rel_posA1, rel_posA2, normalA, invInertiaADiag, invMassA,
						 invInertiaBDiag, invMassB);
	JacobianEntry jacB(world2A, world2B, rel_posB1, rel_posB2, normalB, invInertiaADiag, invMassA,
						 invInertiaBDiag, invMassB);

	//const Scalar vel0 = jacA.getRelativeVelocity(linvelA,angvelA,linvelB,angvelB);
	//const Scalar vel1 = jacB.getRelativeVelocity(linvelA,angvelA,linvelB,angvelB);

	const Scalar vel0 = normalA.dot(body1->getVelocityInLocalPoint(rel_posA1) - body2->getVelocityInLocalPoint(rel_posA1));
	const Scalar vel1 = normalB.dot(body1->getVelocityInLocalPoint(rel_posB1) - body2->getVelocityInLocalPoint(rel_posB1));

	//	Scalar penetrationImpulse = (depth*contactTau*timeCorrection)  * massTerm;//jacDiagABInv
	Scalar massTerm = Scalar(1.) / (invMassA + invMassB);

	// calculate rhs (or error) terms
	const Scalar dv0 = depthA * m_tau * massTerm - vel0 * m_damping;
	const Scalar dv1 = depthB * m_tau * massTerm - vel1 * m_damping;

	// dC/dv * dv = -C

	// jacobian * impulse = -error
	//

	//impulse = jacobianInverse * -error

	// inverting 2x2 symmetric system (offdiagonal are equal!)
	//

	Scalar nonDiag = jacA.getNonDiagonal(jacB, invMassA, invMassB);
	Scalar invDet = Scalar(1.0) / (jacA.getDiagonal() * jacB.getDiagonal() - nonDiag * nonDiag);

	//imp0 = dv0 * jacA.getDiagonal() * invDet + dv1 * -nonDiag * invDet;
	//imp1 = dv1 * jacB.getDiagonal() * invDet + dv0 * - nonDiag * invDet;

	imp0 = dv0 * jacA.getDiagonal() * invDet + dv1 * -nonDiag * invDet;
	imp1 = dv1 * jacB.getDiagonal() * invDet + dv0 * -nonDiag * invDet;

	//[a b]								  [d -c]
	//[c d] inverse = (1 / determinant) * [-b a] where determinant is (ad - bc)

	//[jA nD] * [imp0] = [dv0]
	//[nD jB]   [imp1]   [dv1]
}

void Solve2LinearConstraint::resolveBilateralPairConstraint(
	RigidBody* body1,
	RigidBody* body2,
	const Matrix3x3& world2A,
	const Matrix3x3& world2B,

	const Vec3& invInertiaADiag,
	const Scalar invMassA,
	const Vec3& linvelA, const Vec3& angvelA,
	const Vec3& rel_posA1,
	const Vec3& invInertiaBDiag,
	const Scalar invMassB,
	const Vec3& linvelB, const Vec3& angvelB,
	const Vec3& rel_posA2,

	Scalar depthA, const Vec3& normalA,
	const Vec3& rel_posB1, const Vec3& rel_posB2,
	Scalar depthB, const Vec3& normalB,
	Scalar& imp0, Scalar& imp1)
{
	(void)linvelA;
	(void)linvelB;
	(void)angvelA;
	(void)angvelB;

	imp0 = Scalar(0.);
	imp1 = Scalar(0.);

	Scalar len = Fabs(normalA.length()) - Scalar(1.);
	if (Fabs(len) >= SIMD_EPSILON)
		return;

	Assert(len < SIMD_EPSILON);

	//this jacobian entry could be re-used for all iterations
	JacobianEntry jacA(world2A, world2B, rel_posA1, rel_posA2, normalA, invInertiaADiag, invMassA,
						 invInertiaBDiag, invMassB);
	JacobianEntry jacB(world2A, world2B, rel_posB1, rel_posB2, normalB, invInertiaADiag, invMassA,
						 invInertiaBDiag, invMassB);

	//const Scalar vel0 = jacA.getRelativeVelocity(linvelA,angvelA,linvelB,angvelB);
	//const Scalar vel1 = jacB.getRelativeVelocity(linvelA,angvelA,linvelB,angvelB);

	const Scalar vel0 = normalA.dot(body1->getVelocityInLocalPoint(rel_posA1) - body2->getVelocityInLocalPoint(rel_posA1));
	const Scalar vel1 = normalB.dot(body1->getVelocityInLocalPoint(rel_posB1) - body2->getVelocityInLocalPoint(rel_posB1));

	// calculate rhs (or error) terms
	const Scalar dv0 = depthA * m_tau - vel0 * m_damping;
	const Scalar dv1 = depthB * m_tau - vel1 * m_damping;

	// dC/dv * dv = -C

	// jacobian * impulse = -error
	//

	//impulse = jacobianInverse * -error

	// inverting 2x2 symmetric system (offdiagonal are equal!)
	//

	Scalar nonDiag = jacA.getNonDiagonal(jacB, invMassA, invMassB);
	Scalar invDet = Scalar(1.0) / (jacA.getDiagonal() * jacB.getDiagonal() - nonDiag * nonDiag);

	//imp0 = dv0 * jacA.getDiagonal() * invDet + dv1 * -nonDiag * invDet;
	//imp1 = dv1 * jacB.getDiagonal() * invDet + dv0 * - nonDiag * invDet;

	imp0 = dv0 * jacA.getDiagonal() * invDet + dv1 * -nonDiag * invDet;
	imp1 = dv1 * jacB.getDiagonal() * invDet + dv0 * -nonDiag * invDet;

	//[a b]								  [d -c]
	//[c d] inverse = (1 / determinant) * [-b a] where determinant is (ad - bc)

	//[jA nD] * [imp0] = [dv0]
	//[nD jB]   [imp1]   [dv1]

	if (imp0 > Scalar(0.0))
	{
		if (imp1 > Scalar(0.0))
		{
			//both positive
		}
		else
		{
			imp1 = Scalar(0.);

			// now imp0>0 imp1<0
			imp0 = dv0 / jacA.getDiagonal();
			if (imp0 > Scalar(0.0))
			{
			}
			else
			{
				imp0 = Scalar(0.);
			}
		}
	}
	else
	{
		imp0 = Scalar(0.);

		imp1 = dv1 / jacB.getDiagonal();
		if (imp1 <= Scalar(0.0))
		{
			imp1 = Scalar(0.);
			// now imp0>0 imp1<0
			imp0 = dv0 / jacA.getDiagonal();
			if (imp0 > Scalar(0.0))
			{
			}
			else
			{
				imp0 = Scalar(0.);
			}
		}
		else
		{
		}
	}
}

/*
void Solve2LinearConstraint::resolveAngularConstraint(	const Matrix3x3& invInertiaAWS,
											const Scalar invMassA,
											const Vec3& linvelA,const Vec3& angvelA,
											const Vec3& rel_posA1,
											const Matrix3x3& invInertiaBWS,
											const Scalar invMassB,
											const Vec3& linvelB,const Vec3& angvelB,
											const Vec3& rel_posA2,

											Scalar depthA, const Vec3& normalA, 
											const Vec3& rel_posB1,const Vec3& rel_posB2,
											Scalar depthB, const Vec3& normalB, 
											Scalar& imp0,Scalar& imp1)
{

}
*/
