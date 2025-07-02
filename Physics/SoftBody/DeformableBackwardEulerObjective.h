#ifndef DRX3D_BACKWARD_EULER_OBJECTIVE_H
#define DRX3D_BACKWARD_EULER_OBJECTIVE_H

#include <drx3D/Physics/SoftBody/DeformableLagrangianForce.h>
#include <drx3D/Physics/SoftBody/DeformableMassSpringForce.h>
#include <drx3D/Physics/SoftBody/DeformableGravityForce.h>
#include <drx3D/Physics/SoftBody/DeformableCorotatedForce.h>
#include <drx3D/Physics/SoftBody/DeformableMousePickingForce.h>
#include <drx3D/Physics/SoftBody/DeformableLinearElasticityForce.h>
#include <drx3D/Physics/SoftBody/DeformableNeoHookeanForce.h>
#include <drx3D/Physics/SoftBody/DeformableContactProjection.h>
#include <drx3D/Physics/SoftBody/Preconditioner.h>
#include <drx3D/Maths/Linear/Quickprof.h>

class DeformableBackwardEulerObjective
{
public:
	enum _
	{
		Mass_preconditioner,
		KKT_preconditioner
	};

	typedef AlignedObjectArray<Vec3> TVStack;
	Scalar m_dt;
	AlignedObjectArray<DeformableLagrangianForce*> m_lf;
	AlignedObjectArray<SoftBody*>& m_softBodies;
	Preconditioner* m_preconditioner;
	DeformableContactProjection m_projection;
	const TVStack& m_backupVelocity;
	AlignedObjectArray<SoftBody::Node*> m_nodes;
	bool m_implicit;
	MassPreconditioner* m_massPreconditioner;
	KKTPreconditioner* m_KKTPreconditioner;

	DeformableBackwardEulerObjective(AlignedObjectArray<SoftBody*>& softBodies, const TVStack& backup_v);

	virtual ~DeformableBackwardEulerObjective();

	void initialize() {}

	// compute the rhs for CG solve, i.e, add the dt scaled implicit force to residual
	void computeResidual(Scalar dt, TVStack& residual);

	// add explicit force to the velocity
	void applyExplicitForce(TVStack& force);

	// apply force to velocity and optionally reset the force to zero
	void applyForce(TVStack& force, bool setZero);

	// compute the norm of the residual
	Scalar computeNorm(const TVStack& residual) const;

	// compute one step of the solve (there is only one solve if the system is linear)
	void computeStep(TVStack& dv, const TVStack& residual, const Scalar& dt);

	// perform A*x = b
	void multiply(const TVStack& x, TVStack& b) const;

	// set initial guess for CG solve
	void initialGuess(TVStack& dv, const TVStack& residual);

	// reset data structure and reset dt
	void reinitialize(bool nodeUpdated, Scalar dt);

	void setDt(Scalar dt);

	// add friction force to residual
	void applyDynamicFriction(TVStack& r);

	// add dv to velocity
	void updateVelocity(const TVStack& dv);

	//set constraints as projections
	void setConstraints(const ContactSolverInfo& infoGlobal);

	// update the projections and project the residual
	void project(TVStack& r)
	{
		DRX3D_PROFILE("project");
		m_projection.project(r);
	}

	// perform precondition M^(-1) x = b
	void precondition(const TVStack& x, TVStack& b)
	{
		m_preconditioner->operator()(x, b);
	}

	// reindex all the vertices
	virtual void updateId()
	{
		size_t node_id = 0;
		size_t face_id = 0;
		m_nodes.clear();
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				psb->m_nodes[j].index = node_id;
				m_nodes.push_back(&psb->m_nodes[j]);
				++node_id;
			}
			for (i32 j = 0; j < psb->m_faces.size(); ++j)
			{
				psb->m_faces[j].m_index = face_id;
				++face_id;
			}
		}
	}

	const AlignedObjectArray<SoftBody::Node*>* getIndices() const
	{
		return &m_nodes;
	}

	void setImplicit(bool implicit)
	{
		m_implicit = implicit;
	}

	// Calculate the total potential energy in the system
	Scalar totalEnergy(Scalar dt);

	void addLagrangeMultiplier(const TVStack& vec, TVStack& extended_vec)
	{
		extended_vec.resize(vec.size() + m_projection.m_lagrangeMultipliers.size());
		for (i32 i = 0; i < vec.size(); ++i)
		{
			extended_vec[i] = vec[i];
		}
		i32 offset = vec.size();
		for (i32 i = 0; i < m_projection.m_lagrangeMultipliers.size(); ++i)
		{
			extended_vec[offset + i].setZero();
		}
	}

	void addLagrangeMultiplierRHS(const TVStack& residual, const TVStack& m_dv, TVStack& extended_residual)
	{
		extended_residual.resize(residual.size() + m_projection.m_lagrangeMultipliers.size());
		for (i32 i = 0; i < residual.size(); ++i)
		{
			extended_residual[i] = residual[i];
		}
		i32 offset = residual.size();
		for (i32 i = 0; i < m_projection.m_lagrangeMultipliers.size(); ++i)
		{
			const LagrangeMultiplier& lm = m_projection.m_lagrangeMultipliers[i];
			extended_residual[offset + i].setZero();
			for (i32 d = 0; d < lm.m_num_constraints; ++d)
			{
				for (i32 n = 0; n < lm.m_num_nodes; ++n)
				{
					extended_residual[offset + i][d] += lm.m_weights[n] * m_dv[lm.m_indices[n]].dot(lm.m_dirs[d]);
				}
			}
		}
	}

	void calculateContactForce(const TVStack& dv, const TVStack& rhs, TVStack& f)
	{
		size_t counter = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				const SoftBody::Node& node = psb->m_nodes[j];
				f[counter] = (node.m_im == 0) ? Vec3(0, 0, 0) : dv[counter] / node.m_im;
				++counter;
			}
		}
		for (i32 i = 0; i < m_lf.size(); ++i)
		{
			// add damping matrix
			m_lf[i]->addScaledDampingForceDifferential(-m_dt, dv, f);
		}
		counter = 0;
		for (; counter < f.size(); ++counter)
		{
			f[counter] = rhs[counter] - f[counter];
		}
	}
};

#endif /* BackwardEulerObjective_h */
