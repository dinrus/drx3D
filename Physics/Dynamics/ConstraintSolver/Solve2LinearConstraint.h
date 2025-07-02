#ifndef DRX3D_SOLVE_2LINEAR_CONSTRAINT_H
#define DRX3D_SOLVE_2LINEAR_CONSTRAINT_H

#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Maths/Linear/Vec3.h>

class RigidBody;

/// constraint class used for lateral tyre friction.
class Solve2LinearConstraint
{
	Scalar m_tau;
	Scalar m_damping;

public:
	Solve2LinearConstraint(Scalar tau, Scalar damping)
	{
		m_tau = tau;
		m_damping = damping;
	}
	//
	// solve unilateral constraint (equality, direct method)
	//
	void resolveUnilateralPairConstraint(
		RigidBody* body0,
		RigidBody* body1,

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
		Scalar& imp0, Scalar& imp1);

	//
	// solving 2x2 lcp problem (inequality, direct solution )
	//
	void resolveBilateralPairConstraint(
		RigidBody* body0,
		RigidBody* body1,
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
		Scalar& imp0, Scalar& imp1);

	/*
	void resolveAngularConstraint(	const Matrix3x3& invInertiaAWS,
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
					  Scalar& imp0,Scalar& imp1);

*/
};

#endif  //DRX3D_SOLVE_2LINEAR_CONSTRAINT_H
