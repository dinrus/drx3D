#ifndef DRX3D_DEFORMABLE_LAGRANGIAN_FORCE_H
#define DRX3D_DEFORMABLE_LAGRANGIAN_FORCE_H

#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Maths/Linear/HashMap.h>
#include <iostream>

enum DeformableLagrangianForceType
{
	DRX3D_GRAVITY_FORCE = 1,
	DRX3D_MASSSPRING_FORCE = 2,
	DRX3D_COROTATED_FORCE = 3,
	DRX3D_NEOHOOKEAN_FORCE = 4,
	DRX3D_LINEAR_ELASTICITY_FORCE = 5,
	DRX3D_MOUSE_PICKING_FORCE = 6
};

static inline double randomDouble(double low, double high)
{
	return low + static_cast<double>(rand()) / RAND_MAX * (high - low);
}

class DeformableLagrangianForce
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	AlignedObjectArray<SoftBody*> m_softBodies;
	const AlignedObjectArray<SoftBody::Node*>* m_nodes;

	DeformableLagrangianForce()
	{
	}

	virtual ~DeformableLagrangianForce() {}

	// add all forces
	virtual void addScaledForces(Scalar scale, TVStack& force) = 0;

	// add damping df
	virtual void addScaledDampingForceDifferential(Scalar scale, const TVStack& dv, TVStack& df) = 0;

	// build diagonal of A matrix
	virtual void buildDampingForceDifferentialDiagonal(Scalar scale, TVStack& diagA) = 0;

	// add elastic df
	virtual void addScaledElasticForceDifferential(Scalar scale, const TVStack& dx, TVStack& df) = 0;

	// add all forces that are explicit in explicit solve
	virtual void addScaledExplicitForce(Scalar scale, TVStack& force) = 0;

	// add all damping forces
	virtual void addScaledDampingForce(Scalar scale, TVStack& force) = 0;

	virtual void addScaledHessian(Scalar scale) {}

	virtual DeformableLagrangianForceType getForceType() = 0;

	virtual void reinitialize(bool nodeUpdated)
	{
	}

	// get number of nodes that have the force
	virtual i32 getNumNodes()
	{
		i32 numNodes = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			numNodes += m_softBodies[i]->m_nodes.size();
		}
		return numNodes;
	}

	// add a soft body to be affected by the particular lagrangian force
	virtual void addSoftBody(SoftBody* psb)
	{
		m_softBodies.push_back(psb);
	}

	virtual void removeSoftBody(SoftBody* psb)
	{
		m_softBodies.remove(psb);
	}

	virtual void setIndices(const AlignedObjectArray<SoftBody::Node*>* nodes)
	{
		m_nodes = nodes;
	}

	// Calculate the incremental deformable generated from the input dx
	virtual Matrix3x3 Ds(i32 id0, i32 id1, i32 id2, i32 id3, const TVStack& dx)
	{
		Vec3 c1 = dx[id1] - dx[id0];
		Vec3 c2 = dx[id2] - dx[id0];
		Vec3 c3 = dx[id3] - dx[id0];
		return Matrix3x3(c1, c2, c3).transpose();
	}

	// Calculate the incremental deformable generated from the current velocity
	virtual Matrix3x3 DsFromVelocity(const SoftBody::Node* n0, const SoftBody::Node* n1, const SoftBody::Node* n2, const SoftBody::Node* n3)
	{
		Vec3 c1 = n1->m_v - n0->m_v;
		Vec3 c2 = n2->m_v - n0->m_v;
		Vec3 c3 = n3->m_v - n0->m_v;
		return Matrix3x3(c1, c2, c3).transpose();
	}

	// test for addScaledElasticForce function
	virtual void testDerivative()
	{
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				psb->m_nodes[j].m_q += Vec3(randomDouble(-.1, .1), randomDouble(-.1, .1), randomDouble(-.1, .1));
			}
			psb->updateDeformation();
		}

		TVStack dx;
		dx.resize(getNumNodes());
		TVStack dphi_dx;
		dphi_dx.resize(dx.size());
		for (i32 i = 0; i < dphi_dx.size(); ++i)
		{
			dphi_dx[i].setZero();
		}
		addScaledForces(-1, dphi_dx);

		// write down the current position
		TVStack x;
		x.resize(dx.size());
		i32 counter = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				x[counter] = psb->m_nodes[j].m_q;
				counter++;
			}
		}
		counter = 0;

		// populate dx with random vectors
		for (i32 i = 0; i < dx.size(); ++i)
		{
			dx[i].setX(randomDouble(-1, 1));
			dx[i].setY(randomDouble(-1, 1));
			dx[i].setZ(randomDouble(-1, 1));
		}

		AlignedObjectArray<double> errors;
		for (i32 it = 0; it < 10; ++it)
		{
			for (i32 i = 0; i < dx.size(); ++i)
			{
				dx[i] *= 0.5;
			}

			// get dphi/dx * dx
			double dphi = 0;
			for (i32 i = 0; i < dx.size(); ++i)
			{
				dphi += dphi_dx[i].dot(dx[i]);
			}

			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = x[counter] + dx[counter];
					counter++;
				}
				psb->updateDeformation();
			}
			counter = 0;
			double f1 = totalElasticEnergy(0);

			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = x[counter] - dx[counter];
					counter++;
				}
				psb->updateDeformation();
			}
			counter = 0;

			double f2 = totalElasticEnergy(0);

			//restore m_q
			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = x[counter];
					counter++;
				}
				psb->updateDeformation();
			}
			counter = 0;
			double error = f1 - f2 - 2 * dphi;
			errors.push_back(error);
			std::cout << "Iteration = " << it << ", f1 = " << f1 << ", f2 = " << f2 << ", error = " << error << std::endl;
		}
		for (i32 i = 1; i < errors.size(); ++i)
		{
			std::cout << "Iteration = " << i << ", ratio = " << errors[i - 1] / errors[i] << std::endl;
		}
	}

	// test for addScaledElasticForce function
	virtual void testHessian()
	{
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				psb->m_nodes[j].m_q += Vec3(randomDouble(-.1, .1), randomDouble(-.1, .1), randomDouble(-.1, .1));
			}
			psb->updateDeformation();
		}

		TVStack dx;
		dx.resize(getNumNodes());
		TVStack df;
		df.resize(dx.size());
		TVStack f1;
		f1.resize(dx.size());
		TVStack f2;
		f2.resize(dx.size());

		// write down the current position
		TVStack x;
		x.resize(dx.size());
		i32 counter = 0;
		for (i32 i = 0; i < m_softBodies.size(); ++i)
		{
			SoftBody* psb = m_softBodies[i];
			for (i32 j = 0; j < psb->m_nodes.size(); ++j)
			{
				x[counter] = psb->m_nodes[j].m_q;
				counter++;
			}
		}
		counter = 0;

		// populate dx with random vectors
		for (i32 i = 0; i < dx.size(); ++i)
		{
			dx[i].setX(randomDouble(-1, 1));
			dx[i].setY(randomDouble(-1, 1));
			dx[i].setZ(randomDouble(-1, 1));
		}

		AlignedObjectArray<double> errors;
		for (i32 it = 0; it < 10; ++it)
		{
			for (i32 i = 0; i < dx.size(); ++i)
			{
				dx[i] *= 0.5;
			}

			// get df
			for (i32 i = 0; i < df.size(); ++i)
			{
				df[i].setZero();
				f1[i].setZero();
				f2[i].setZero();
			}

			//set df
			addScaledElasticForceDifferential(-1, dx, df);

			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = x[counter] + dx[counter];
					counter++;
				}
				psb->updateDeformation();
			}
			counter = 0;

			//set f1
			addScaledForces(-1, f1);

			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = x[counter] - dx[counter];
					counter++;
				}
				psb->updateDeformation();
			}
			counter = 0;

			//set f2
			addScaledForces(-1, f2);

			//restore m_q
			for (i32 i = 0; i < m_softBodies.size(); ++i)
			{
				SoftBody* psb = m_softBodies[i];
				for (i32 j = 0; j < psb->m_nodes.size(); ++j)
				{
					psb->m_nodes[j].m_q = x[counter];
					counter++;
				}
				psb->updateDeformation();
			}
			counter = 0;
			double error = 0;
			for (i32 i = 0; i < df.size(); ++i)
			{
				Vec3 error_vector = f1[i] - f2[i] - 2 * df[i];
				error += error_vector.length2();
			}
			error = Sqrt(error);
			errors.push_back(error);
			std::cout << "Iteration = " << it << ", error = " << error << std::endl;
		}
		for (i32 i = 1; i < errors.size(); ++i)
		{
			std::cout << "Iteration = " << i << ", ratio = " << errors[i - 1] / errors[i] << std::endl;
		}
	}

	//
	virtual double totalElasticEnergy(Scalar dt)
	{
		return 0;
	}

	//
	virtual double totalDampingEnergy(Scalar dt)
	{
		return 0;
	}

	// total Energy takes dt as input because certain energies depend on dt
	virtual double totalEnergy(Scalar dt)
	{
		return totalElasticEnergy(dt) + totalDampingEnergy(dt);
	}
};
#endif /* DRX3D_DEFORMABLE_LAGRANGIAN_FORCE */
