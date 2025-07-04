#include "../invdyn_bullet_comparison.h"
#include <cmath>
#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTree.h>
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyPoint2Point.h>
#include <drx/Core/Core.h>
#include <drx/Core/Core.h>

namespace drx3d_inverse
{
i32 compareInverseAndForwardDynamics(vecx &q, vecx &u, vecx &dot_u, Vec3 &gravity, bool verbose,
									 MultiBody *mb, MultiBodyTree *id_tree, double *pos_error,
									 double *acc_error)
{
// call function and return -1 if it does, printing an throw drx::Exc
#define RETURN_ON_FAILURE(x)                         \
	do                                               \
	{                                                \
		if (-1 == x)                                 \
		{                                            \
			throw drx::Exc("calling " #x "\n"); \
			return -1;                               \
		}                                            \
	} while (0)

	if (verbose)
	{
		printf("\n ===================================== \n");
	}
	vecx joint_forces(q.size());

	// set positions and velocities for btMultiBody
	// base link
	mat33 world_T_base;
	vec3 world_pos_base;
	Transform2 base_transform;
	vec3 base_velocity;
	vec3 base_angular_velocity;

	RETURN_ON_FAILURE(id_tree->setGravityInWorldFrame(gravity));
	RETURN_ON_FAILURE(id_tree->getBodyOrigin(0, &world_pos_base));
	RETURN_ON_FAILURE(id_tree->getBodyTransform(0, &world_T_base));
	RETURN_ON_FAILURE(id_tree->getBodyAngularVelocity(0, &base_angular_velocity));
	RETURN_ON_FAILURE(id_tree->getBodyLinearVelocityCoM(0, &base_velocity));

	base_transform.setBasis(world_T_base);
	base_transform.setOrigin(world_pos_base);
	mb->setBaseWorldTransform(base_transform);
	mb->setBaseOmega(base_angular_velocity);
	mb->setBaseVel(base_velocity);
	mb->setLinearDamping(0);
	mb->setAngularDamping(0);

	// remaining links
	i32 q_index;
	if (mb->hasFixedBase())
	{
		q_index = 0;
	}
	else
	{
		q_index = 6;
	}
	if (verbose)
	{
		printf(":num_links= %d, num_dofs= %d\n", mb->getNumLinks(), mb->getNumDofs());
	}
	for (i32 l = 0; l < mb->getNumLinks(); l++)
	{
		const MultibodyLink &link = mb->getLink(l);
		if (verbose)
		{
			printf("link %d, pos_var_count= %d, dof_count= %d\n", l, link.m_posVarCount,
				   link.m_dofCount);
		}
		if (link.m_posVarCount == 1)
		{
			mb->setJointPosMultiDof(l, &q(q_index));
			mb->setJointVelMultiDof(l, &u(q_index));
			if (verbose)
			{
				printf("set q[%d]= %f, u[%d]= %f\n", q_index, q(q_index), q_index, u(q_index));
			}
			q_index++;
		}
	}
	// sanity check
	if (q_index != q.size())
	{
		throw drx::Exc("error in number of dofs for Multibody and MultiBodyTree\n");
		return -1;
	}

	// run inverse dynamics to determine joint_forces for given q, u, dot_u
	if (-1 == id_tree->calculatedrx3d_inverse(q, u, dot_u, &joint_forces))
	{
		throw drx::Exc("calculating inverse dynamics\n");
		return -1;
	}

	// set up bullet forward dynamics model
	Scalar dt = 0;
	AlignedObjectArray<Scalar> scratch_r;
	AlignedObjectArray<Vec3> scratch_v;
	AlignedObjectArray<Matrix3x3> scratch_m;
	// this triggers switch between using either appliedConstraintForce or appliedForce
	bool isConstraintPass = false;
	// apply gravity forces for btMultiBody model. Must be done manually.
	mb->addBaseForce(mb->getBaseMass() * gravity);

	for (i32 link = 0; link < mb->getNumLinks(); link++)
	{
		mb->addLinkForce(link, gravity * mb->getLinkMass(link));
		if (verbose)
		{
			printf("link %d, applying gravity %f %f %f\n", link,
				   gravity[0] * mb->getLinkMass(link), gravity[1] * mb->getLinkMass(link),
				   gravity[2] * mb->getLinkMass(link));
		}
	}

	// apply generalized forces
	if (mb->hasFixedBase())
	{
		q_index = 0;
	}
	else
	{
		vec3 base_force;
		base_force(0) = joint_forces(3);
		base_force(1) = joint_forces(4);
		base_force(2) = joint_forces(5);

		vec3 base_moment;
		base_moment(0) = joint_forces(0);
		base_moment(1) = joint_forces(1);
		base_moment(2) = joint_forces(2);

		mb->addBaseForce(world_T_base * base_force);
		mb->addBaseTorque(world_T_base * base_moment);
		if (verbose)
		{
			printf("base force from id: %f %f %f\n", joint_forces(3), joint_forces(4),
				   joint_forces(5));
			printf("base moment from id: %f %f %f\n", joint_forces(0), joint_forces(1),
				   joint_forces(2));
		}
		q_index = 6;
	}

	for (i32 l = 0; l < mb->getNumLinks(); l++)
	{
		const MultibodyLink &link = mb->getLink(l);
		if (link.m_posVarCount == 1)
		{
			if (verbose)
			{
				printf("id:joint_force[%d]= %f, applied to link %d\n", q_index,
					   joint_forces(q_index), l);
			}
			mb->addJointTorque(l, joint_forces(q_index));
			q_index++;
		}
	}

	// sanity check
	if (q_index != q.size())
	{
		throw drx::Exc("error in number of dofs for btMultibody and MultiBodyTree\n");
		return -1;
	}

	// run forward kinematics & forward dynamics
	AlignedObjectArray<Quat> world_to_local;
	AlignedObjectArray<Vec3> local_origin;
	mb->forwardKinematics(world_to_local, local_origin);
	mb->computeAccelerationsArticulatedBodyAlgorithmMultiDof(dt, scratch_r, scratch_v, scratch_m, isConstraintPass, false, false);

	// read generalized accelerations back from btMultiBody
	// the mapping from scratch variables to accelerations is taken from the implementation
	// of stepVelocitiesMultiDof
	Scalar *base_accel = &scratch_r[mb->getNumDofs()];
	Scalar *joint_accel = base_accel + 6;
	*acc_error = 0;
	i32 dot_u_offset = 0;
	if (mb->hasFixedBase())
	{
		dot_u_offset = 0;
	}
	else
	{
		dot_u_offset = 6;
	}

	if (true == mb->hasFixedBase())
	{
		for (i32 i = 0; i < mb->getNumDofs(); i++)
		{
			if (verbose)
			{
				printf(":ddot_q[%d]= %f, id:ddot_q= %e, diff= %e\n", i, joint_accel[i],
					   dot_u(i + dot_u_offset), joint_accel[i] - dot_u(i));
			}
			*acc_error += DRX3D_ID_POW(joint_accel[i] - dot_u(i + dot_u_offset), 2);
		}
	}
	else
	{
		vec3 base_dot_omega;
		vec3 world_dot_omega;
		world_dot_omega(0) = base_accel[0];
		world_dot_omega(1) = base_accel[1];
		world_dot_omega(2) = base_accel[2];
		base_dot_omega = world_T_base.transpose() * world_dot_omega;

		// com happens to coincide with link origin here. If that changes, we need to calculate
		// ddot_com
		vec3 base_ddot_com;
		vec3 world_ddot_com;
		world_ddot_com(0) = base_accel[3];
		world_ddot_com(1) = base_accel[4];
		world_ddot_com(2) = base_accel[5];
		base_ddot_com = world_T_base.transpose() * world_ddot_com;

		for (i32 i = 0; i < 3; i++)
		{
			if (verbose)
			{
				printf("::base_dot_omega(%d)= %e dot_u[%d]= %e, diff= %e\n", i, base_dot_omega(i),
					   i, dot_u[i], base_dot_omega(i) - dot_u[i]);
			}
			*acc_error += DRX3D_ID_POW(base_dot_omega(i) - dot_u(i), 2);
		}
		for (i32 i = 0; i < 3; i++)
		{
			if (verbose)
			{
				printf("::base_ddot_com(%d)= %e dot_u[%d]= %e, diff= %e\n", i, base_ddot_com(i),
					   i, dot_u[i + 3], base_ddot_com(i) - dot_u[i + 3]);
			}
			*acc_error += DRX3D_ID_POW(base_ddot_com(i) - dot_u(i + 3), 2);
		}

		for (i32 i = 0; i < mb->getNumDofs(); i++)
		{
			if (verbose)
			{
				printf(":ddot_q[%d]= %f, id:ddot_q= %e, diff= %e\n", i, joint_accel[i],
					   dot_u(i + 6), joint_accel[i] - dot_u(i + 6));
			}
			*acc_error += DRX3D_ID_POW(joint_accel[i] - dot_u(i + 6), 2);
		}
	}
	*acc_error = std::sqrt(*acc_error);
	if (verbose)
	{
		printf("======dynamics-err: %e\n", *acc_error);
	}
	*pos_error = 0.0;

	{
		mat33 world_T_body;
		if (-1 == id_tree->getBodyTransform(0, &world_T_body))
		{
			throw drx::Exc(drx::Format("получение трансформа для тела %d\n", 0));
		}
		vec3 world_com;
		if (-1 == id_tree->getBodyCoM(0, &world_com))
		{
			throw drx::Exc(drx::Format("getting com for body %d\n", 0));
		}
		if (verbose)
		{
			printf("id:com:       %f %f %f\n", world_com(0), world_com(1), world_com(2));

			printf(
				"id:transform: %f %f %f\n"
				"              %f %f %f\n"
				"              %f %f %f\n",
				world_T_body(0, 0), world_T_body(0, 1), world_T_body(0, 2), world_T_body(1, 0),
				world_T_body(1, 1), world_T_body(1, 2), world_T_body(2, 0), world_T_body(2, 1),
				world_T_body(2, 2));
		}
	}

	for (i32 l = 0; l < mb->getNumLinks(); l++)
	{
		const MultibodyLink &bt_link = mb->getLink(l);

		vec3 bt_origin = bt_link.m_cachedWorldTransform.getOrigin();
		mat33 bt_basis = bt_link.m_cachedWorldTransform.getBasis();
		if (verbose)
		{
			printf("------------- link %d\n", l + 1);
			printf(":com:       %f %f %f\n", bt_origin(0), bt_origin(1), bt_origin(2));
			printf(
				":transform: %f %f %f\n"
				"              %f %f %f\n"
				"              %f %f %f\n",
				bt_basis(0, 0), bt_basis(0, 1), bt_basis(0, 2), bt_basis(1, 0), bt_basis(1, 1),
				bt_basis(1, 2), bt_basis(2, 0), bt_basis(2, 1), bt_basis(2, 2));
		}
		mat33 id_world_T_body;
		vec3 id_world_com;

		if (-1 == id_tree->getBodyTransform(l + 1, &id_world_T_body))
		{
			throw drx::Exc(drx::Format("получение трансформа для тела %d\n", l));
			return -1;
		}
		if (-1 == id_tree->getBodyCoM(l + 1, &id_world_com))
		{
			throw drx::Exc(drx::Format("getting com for body %d\n", l));
			return -1;
		}
		if (verbose)
		{
			printf("id:com:       %f %f %f\n", id_world_com(0), id_world_com(1), id_world_com(2));

			printf(
				"id:transform: %f %f %f\n"
				"              %f %f %f\n"
				"              %f %f %f\n",
				id_world_T_body(0, 0), id_world_T_body(0, 1), id_world_T_body(0, 2),
				id_world_T_body(1, 0), id_world_T_body(1, 1), id_world_T_body(1, 2),
				id_world_T_body(2, 0), id_world_T_body(2, 1), id_world_T_body(2, 2));
		}
		vec3 diff_com = bt_origin - id_world_com;
		mat33 diff_basis = bt_basis - id_world_T_body;
		if (verbose)
		{
			printf("diff-com:    %e %e %e\n", diff_com(0), diff_com(1), diff_com(2));

			printf("diff-transform: %e %e %e %e %e %e %e %e %e\n", diff_basis(0, 0),
				   diff_basis(0, 1), diff_basis(0, 2), diff_basis(1, 0), diff_basis(1, 1),
				   diff_basis(1, 2), diff_basis(2, 0), diff_basis(2, 1), diff_basis(2, 2));
		}
		double total_pos_err =
			DRX3D_ID_SQRT(DRX3D_ID_POW(diff_com(0), 2) + DRX3D_ID_POW(diff_com(1), 2) +
					   DRX3D_ID_POW(diff_com(2), 2) + DRX3D_ID_POW(diff_basis(0, 0), 2) +
					   DRX3D_ID_POW(diff_basis(0, 1), 2) + DRX3D_ID_POW(diff_basis(0, 2), 2) +
					   DRX3D_ID_POW(diff_basis(1, 0), 2) + DRX3D_ID_POW(diff_basis(1, 1), 2) +
					   DRX3D_ID_POW(diff_basis(1, 2), 2) + DRX3D_ID_POW(diff_basis(2, 0), 2) +
					   DRX3D_ID_POW(diff_basis(2, 1), 2) + DRX3D_ID_POW(diff_basis(2, 2), 2));
		if (verbose)
		{
			printf("======kin-pos-err: %e\n", total_pos_err);
		}
		if (total_pos_err > *pos_error)
		{
			*pos_error = total_pos_err;
		}
	}

	return 0;
}
}  // namespace drx3d_inverse
