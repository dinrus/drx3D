#include <drx3D/Physics/SoftBody/DeformableContactProjection.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <algorithm>
#include <cmath>
Scalar DeformableContactProjection::update(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal)
{
	Scalar residualSquare = 0;
	for (i32 i = 0; i < numDeformableBodies; ++i)
	{
		for (i32 j = 0; j < m_softBodies.size(); ++j)
		{
			CollisionObject2* psb = m_softBodies[j];
			if (psb != deformableBodies[i])
			{
				continue;
			}
			for (i32 k = 0; k < m_nodeRigidConstraints[j].size(); ++k)
			{
				DeformableNodeRigidContactConstraint& constraint = m_nodeRigidConstraints[j][k];
				Scalar localResidualSquare = constraint.solveConstraint(infoGlobal);
				residualSquare = d3Max(residualSquare, localResidualSquare);
			}
			for (i32 k = 0; k < m_nodeAnchorConstraints[j].size(); ++k)
			{
				DeformableNodeAnchorConstraint& constraint = m_nodeAnchorConstraints[j][k];
				Scalar localResidualSquare = constraint.solveConstraint(infoGlobal);
				residualSquare = d3Max(residualSquare, localResidualSquare);
			}
			for (i32 k = 0; k < m_faceRigidConstraints[j].size(); ++k)
			{
				DeformableFaceRigidContactConstraint& constraint = m_faceRigidConstraints[j][k];
				Scalar localResidualSquare = constraint.solveConstraint(infoGlobal);
				residualSquare = d3Max(residualSquare, localResidualSquare);
			}
			for (i32 k = 0; k < m_deformableConstraints[j].size(); ++k)
			{
				DeformableFaceNodeContactConstraint& constraint = m_deformableConstraints[j][k];
				Scalar localResidualSquare = constraint.solveConstraint(infoGlobal);
				residualSquare = d3Max(residualSquare, localResidualSquare);
			}
		}
	}
	return residualSquare;
}

Scalar DeformableContactProjection::solveSplitImpulse(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal)
{
	Scalar residualSquare = 0;
	for (i32 i = 0; i < numDeformableBodies; ++i)
	{
		for (i32 j = 0; j < m_softBodies.size(); ++j)
		{
			CollisionObject2* psb = m_softBodies[j];
			if (psb != deformableBodies[i])
			{
				continue;
			}
			for (i32 k = 0; k < m_nodeRigidConstraints[j].size(); ++k)
			{
				DeformableNodeRigidContactConstraint& constraint = m_nodeRigidConstraints[j][k];
				Scalar localResidualSquare = constraint.solveSplitImpulse(infoGlobal);
				residualSquare = d3Max(residualSquare, localResidualSquare);
			}
			for (i32 k = 0; k < m_faceRigidConstraints[j].size(); ++k)
			{
				DeformableFaceRigidContactConstraint& constraint = m_faceRigidConstraints[j][k];
				Scalar localResidualSquare = constraint.solveSplitImpulse(infoGlobal);
				residualSquare = d3Max(residualSquare, localResidualSquare);
			}
		}
	}
	return residualSquare;
}

void DeformableContactProjection::setConstraints(const ContactSolverInfo& infoGlobal)
{
	DRX3D_PROFILE("setConstraints");
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			continue;
		}

		// set Dirichlet constraint
		for (i32 j = 0; j < psb->m_nodes.size(); ++j)
		{
			if (psb->m_nodes[j].m_im == 0)
			{
				DeformableStaticConstraint static_constraint(&psb->m_nodes[j], infoGlobal);
				m_staticConstraints[i].push_back(static_constraint);
			}
		}

		// set up deformable anchors
		for (i32 j = 0; j < psb->m_deformableAnchors.size(); ++j)
		{
			SoftBody::DeformableNodeRigidAnchor& anchor = psb->m_deformableAnchors[j];
			// skip fixed points
			if (anchor.m_node->m_im == 0)
			{
				continue;
			}
			anchor.m_c1 = anchor.m_cti.m_colObj->getWorldTransform().getBasis() * anchor.m_local;
			DeformableNodeAnchorConstraint constraint(anchor, infoGlobal);
			m_nodeAnchorConstraints[i].push_back(constraint);
		}

		// set Deformable Node vs. Rigid constraint
		for (i32 j = 0; j < psb->m_nodeRigidContacts.size(); ++j)
		{
			const SoftBody::DeformableNodeRigidContact& contact = psb->m_nodeRigidContacts[j];
			// skip fixed points
			if (contact.m_node->m_im == 0)
			{
				continue;
			}
			DeformableNodeRigidContactConstraint constraint(contact, infoGlobal);
			m_nodeRigidConstraints[i].push_back(constraint);
		}

		// set Deformable Face vs. Rigid constraint
		for (i32 j = 0; j < psb->m_faceRigidContacts.size(); ++j)
		{
			const SoftBody::DeformableFaceRigidContact& contact = psb->m_faceRigidContacts[j];
			// skip fixed faces
			if (contact.m_c2 == 0)
			{
				continue;
			}
			DeformableFaceRigidContactConstraint constraint(contact, infoGlobal, m_useStrainLimiting);
			m_faceRigidConstraints[i].push_back(constraint);
		}
	}
}

void DeformableContactProjection::project(TVStack& x)
{
#ifndef USE_MGS
	i32k dim = 3;
	for (i32 index = 0; index < m_projectionsDict.size(); ++index)
	{
		AlignedObjectArray<Vec3>& projectionDirs = *m_projectionsDict.getAtIndex(index);
		size_t i = m_projectionsDict.getKeyAtIndex(index).getUid1();
		if (projectionDirs.size() >= dim)
		{
			// static node
			x[i].setZero();
			continue;
		}
		else if (projectionDirs.size() == 2)
		{
			Vec3 dir0 = projectionDirs[0];
			Vec3 dir1 = projectionDirs[1];
			Vec3 free_dir = Cross(dir0, dir1);
			if (free_dir.safeNorm() < SIMD_EPSILON)
			{
				x[i] -= x[i].dot(dir0) * dir0;
			}
			else
			{
				free_dir.normalize();
				x[i] = x[i].dot(free_dir) * free_dir;
			}
		}
		else
		{
			Assert(projectionDirs.size() == 1);
			Vec3 dir0 = projectionDirs[0];
			x[i] -= x[i].dot(dir0) * dir0;
		}
	}
#else
	ReducedVector p(x.size());
	for (i32 i = 0; i < m_projections.size(); ++i)
	{
		p += (m_projections[i].dot(x) * m_projections[i]);
	}
	for (i32 i = 0; i < p.m_indices.size(); ++i)
	{
		x[p.m_indices[i]] -= p.m_vecs[i];
	}
#endif
}

void DeformableContactProjection::setProjection()
{
#ifndef USE_MGS
	DRX3D_PROFILE("DeformableContactProjection::setProjection");
	AlignedObjectArray<Vec3> units;
	units.push_back(Vec3(1, 0, 0));
	units.push_back(Vec3(0, 1, 0));
	units.push_back(Vec3(0, 0, 1));
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			continue;
		}
		for (i32 j = 0; j < m_staticConstraints[i].size(); ++j)
		{
			i32 index = m_staticConstraints[i][j].m_node->index;
			m_staticConstraints[i][j].m_node->m_constrained = true;
			if (m_projectionsDict.find(index) == NULL)
			{
				m_projectionsDict.insert(index, units);
			}
			else
			{
				AlignedObjectArray<Vec3>& projections = *m_projectionsDict[index];
				for (i32 k = 0; k < 3; ++k)
				{
					projections.push_back(units[k]);
				}
			}
		}
		for (i32 j = 0; j < m_nodeAnchorConstraints[i].size(); ++j)
		{
			i32 index = m_nodeAnchorConstraints[i][j].m_anchor->m_node->index;
			m_nodeAnchorConstraints[i][j].m_anchor->m_node->m_constrained = true;
			if (m_projectionsDict.find(index) == NULL)
			{
				m_projectionsDict.insert(index, units);
			}
			else
			{
				AlignedObjectArray<Vec3>& projections = *m_projectionsDict[index];
				for (i32 k = 0; k < 3; ++k)
				{
					projections.push_back(units[k]);
				}
			}
		}
		for (i32 j = 0; j < m_nodeRigidConstraints[i].size(); ++j)
		{
			i32 index = m_nodeRigidConstraints[i][j].m_node->index;
			m_nodeRigidConstraints[i][j].m_node->m_constrained = true;
			if (m_nodeRigidConstraints[i][j].m_binding)
			{
				if (m_nodeRigidConstraints[i][j].m_static)
				{
					if (m_projectionsDict.find(index) == NULL)
					{
						m_projectionsDict.insert(index, units);
					}
					else
					{
						AlignedObjectArray<Vec3>& projections = *m_projectionsDict[index];
						for (i32 k = 0; k < 3; ++k)
						{
							projections.push_back(units[k]);
						}
					}
				}
				else
				{
					if (m_projectionsDict.find(index) == NULL)
					{
						AlignedObjectArray<Vec3> projections;
						projections.push_back(m_nodeRigidConstraints[i][j].m_normal);
						m_projectionsDict.insert(index, projections);
					}
					else
					{
						AlignedObjectArray<Vec3>& projections = *m_projectionsDict[index];
						projections.push_back(m_nodeRigidConstraints[i][j].m_normal);
					}
				}
			}
		}
		for (i32 j = 0; j < m_faceRigidConstraints[i].size(); ++j)
		{
			const SoftBody::Face* face = m_faceRigidConstraints[i][j].m_face;
			if (m_faceRigidConstraints[i][j].m_binding)
			{
				for (i32 k = 0; k < 3; ++k)
				{
					face->m_n[k]->m_constrained = true;
				}
			}
			for (i32 k = 0; k < 3; ++k)
			{
				SoftBody::Node* node = face->m_n[k];
				i32 index = node->index;
				if (m_faceRigidConstraints[i][j].m_static)
				{
					if (m_projectionsDict.find(index) == NULL)
					{
						m_projectionsDict.insert(index, units);
					}
					else
					{
						AlignedObjectArray<Vec3>& projections = *m_projectionsDict[index];
						for (i32 l = 0; l < 3; ++l)
						{
							projections.push_back(units[l]);
						}
					}
				}
				else
				{
					if (m_projectionsDict.find(index) == NULL)
					{
						AlignedObjectArray<Vec3> projections;
						projections.push_back(m_faceRigidConstraints[i][j].m_normal);
						m_projectionsDict.insert(index, projections);
					}
					else
					{
						AlignedObjectArray<Vec3>& projections = *m_projectionsDict[index];
						projections.push_back(m_faceRigidConstraints[i][j].m_normal);
					}
				}
			}
		}
	}
#else
	i32 dof = 0;
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		dof += m_softBodies[i]->m_nodes.size();
	}
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			continue;
		}
		for (i32 j = 0; j < m_staticConstraints[i].size(); ++j)
		{
			i32 index = m_staticConstraints[i][j].m_node->index;
			m_staticConstraints[i][j].m_node->m_penetration = SIMD_INFINITY;
			AlignedObjectArray<i32> indices;
			AlignedObjectArray<Vec3> vecs1, vecs2, vecs3;
			indices.push_back(index);
			vecs1.push_back(Vec3(1, 0, 0));
			vecs2.push_back(Vec3(0, 1, 0));
			vecs3.push_back(Vec3(0, 0, 1));
			m_projections.push_back(ReducedVector(dof, indices, vecs1));
			m_projections.push_back(ReducedVector(dof, indices, vecs2));
			m_projections.push_back(ReducedVector(dof, indices, vecs3));
		}

		for (i32 j = 0; j < m_nodeAnchorConstraints[i].size(); ++j)
		{
			i32 index = m_nodeAnchorConstraints[i][j].m_anchor->m_node->index;
			m_nodeAnchorConstraints[i][j].m_anchor->m_node->m_penetration = SIMD_INFINITY;
			AlignedObjectArray<i32> indices;
			AlignedObjectArray<Vec3> vecs1, vecs2, vecs3;
			indices.push_back(index);
			vecs1.push_back(Vec3(1, 0, 0));
			vecs2.push_back(Vec3(0, 1, 0));
			vecs3.push_back(Vec3(0, 0, 1));
			m_projections.push_back(ReducedVector(dof, indices, vecs1));
			m_projections.push_back(ReducedVector(dof, indices, vecs2));
			m_projections.push_back(ReducedVector(dof, indices, vecs3));
		}
		for (i32 j = 0; j < m_nodeRigidConstraints[i].size(); ++j)
		{
			i32 index = m_nodeRigidConstraints[i][j].m_node->index;
			m_nodeRigidConstraints[i][j].m_node->m_penetration = -m_nodeRigidConstraints[i][j].getContact()->m_cti.m_offset;
			AlignedObjectArray<i32> indices;
			indices.push_back(index);
			AlignedObjectArray<Vec3> vecs1, vecs2, vecs3;
			if (m_nodeRigidConstraints[i][j].m_static)
			{
				vecs1.push_back(Vec3(1, 0, 0));
				vecs2.push_back(Vec3(0, 1, 0));
				vecs3.push_back(Vec3(0, 0, 1));
				m_projections.push_back(ReducedVector(dof, indices, vecs1));
				m_projections.push_back(ReducedVector(dof, indices, vecs2));
				m_projections.push_back(ReducedVector(dof, indices, vecs3));
			}
			else
			{
				vecs1.push_back(m_nodeRigidConstraints[i][j].m_normal);
				m_projections.push_back(ReducedVector(dof, indices, vecs1));
			}
		}
		for (i32 j = 0; j < m_faceRigidConstraints[i].size(); ++j)
		{
			const SoftBody::Face* face = m_faceRigidConstraints[i][j].m_face;
			Vec3 bary = m_faceRigidConstraints[i][j].getContact()->m_bary;
			Scalar penetration = -m_faceRigidConstraints[i][j].getContact()->m_cti.m_offset;
			for (i32 k = 0; k < 3; ++k)
			{
				face->m_n[k]->m_penetration = d3Max(face->m_n[k]->m_penetration, penetration);
			}
			if (m_faceRigidConstraints[i][j].m_static)
			{
				for (i32 l = 0; l < 3; ++l)
				{
					ReducedVector rv(dof);
					for (i32 k = 0; k < 3; ++k)
					{
						rv.m_indices.push_back(face->m_n[k]->index);
						Vec3 v(0, 0, 0);
						v[l] = bary[k];
						rv.m_vecs.push_back(v);
						rv.sort();
					}
					m_projections.push_back(rv);
				}
			}
			else
			{
				ReducedVector rv(dof);
				for (i32 k = 0; k < 3; ++k)
				{
					rv.m_indices.push_back(face->m_n[k]->index);
					rv.m_vecs.push_back(bary[k] * m_faceRigidConstraints[i][j].m_normal);
					rv.sort();
				}
				m_projections.push_back(rv);
			}
		}
	}
	ModifiedGramSchmidt<ReducedVector> mgs(m_projections);
	mgs.solve();
	m_projections = mgs.m_out;
#endif
}

void DeformableContactProjection::checkConstraints(const TVStack& x)
{
	for (i32 i = 0; i < m_lagrangeMultipliers.size(); ++i)
	{
		Vec3 d(0, 0, 0);
		const LagrangeMultiplier& lm = m_lagrangeMultipliers[i];
		for (i32 j = 0; j < lm.m_num_constraints; ++j)
		{
			for (i32 k = 0; k < lm.m_num_nodes; ++k)
			{
				d[j] += lm.m_weights[k] * x[lm.m_indices[k]].dot(lm.m_dirs[j]);
			}
		}
		//		printf("d = %f, %f, %f\n", d[0], d[1], d[2]);
		//        printf("val = %f, %f, %f\n", lm.m_vals[0], lm.m_vals[1], lm.m_vals[2]);
	}
}

void DeformableContactProjection::setLagrangeMultiplier()
{
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		SoftBody* psb = m_softBodies[i];
		if (!psb->isActive())
		{
			continue;
		}
		for (i32 j = 0; j < m_staticConstraints[i].size(); ++j)
		{
			i32 index = m_staticConstraints[i][j].m_node->index;
			m_staticConstraints[i][j].m_node->m_constrained = true;
			LagrangeMultiplier lm;
			lm.m_num_nodes = 1;
			lm.m_indices[0] = index;
			lm.m_weights[0] = 1.0;
			lm.m_num_constraints = 3;
			lm.m_dirs[0] = Vec3(1, 0, 0);
			lm.m_dirs[1] = Vec3(0, 1, 0);
			lm.m_dirs[2] = Vec3(0, 0, 1);
			m_lagrangeMultipliers.push_back(lm);
		}
		for (i32 j = 0; j < m_nodeAnchorConstraints[i].size(); ++j)
		{
			i32 index = m_nodeAnchorConstraints[i][j].m_anchor->m_node->index;
			m_nodeAnchorConstraints[i][j].m_anchor->m_node->m_constrained = true;
			LagrangeMultiplier lm;
			lm.m_num_nodes = 1;
			lm.m_indices[0] = index;
			lm.m_weights[0] = 1.0;
			lm.m_num_constraints = 3;
			lm.m_dirs[0] = Vec3(1, 0, 0);
			lm.m_dirs[1] = Vec3(0, 1, 0);
			lm.m_dirs[2] = Vec3(0, 0, 1);
			m_lagrangeMultipliers.push_back(lm);
		}

		for (i32 j = 0; j < m_nodeRigidConstraints[i].size(); ++j)
		{
			if (!m_nodeRigidConstraints[i][j].m_binding)
			{
				continue;
			}
			i32 index = m_nodeRigidConstraints[i][j].m_node->index;
			m_nodeRigidConstraints[i][j].m_node->m_constrained = true;
			LagrangeMultiplier lm;
			lm.m_num_nodes = 1;
			lm.m_indices[0] = index;
			lm.m_weights[0] = 1.0;
			if (m_nodeRigidConstraints[i][j].m_static)
			{
				lm.m_num_constraints = 3;
				lm.m_dirs[0] = Vec3(1, 0, 0);
				lm.m_dirs[1] = Vec3(0, 1, 0);
				lm.m_dirs[2] = Vec3(0, 0, 1);
			}
			else
			{
				lm.m_num_constraints = 1;
				lm.m_dirs[0] = m_nodeRigidConstraints[i][j].m_normal;
			}
			m_lagrangeMultipliers.push_back(lm);
		}

		for (i32 j = 0; j < m_faceRigidConstraints[i].size(); ++j)
		{
			if (!m_faceRigidConstraints[i][j].m_binding)
			{
				continue;
			}
			SoftBody::Face* face = m_faceRigidConstraints[i][j].m_face;

			Vec3 bary = m_faceRigidConstraints[i][j].getContact()->m_bary;
			LagrangeMultiplier lm;
			lm.m_num_nodes = 3;

			for (i32 k = 0; k < 3; ++k)
			{
				face->m_n[k]->m_constrained = true;
				lm.m_indices[k] = face->m_n[k]->index;
				lm.m_weights[k] = bary[k];
			}
			if (m_faceRigidConstraints[i][j].m_static)
			{
				face->m_pcontact[3] = 1;
				lm.m_num_constraints = 3;
				lm.m_dirs[0] = Vec3(1, 0, 0);
				lm.m_dirs[1] = Vec3(0, 1, 0);
				lm.m_dirs[2] = Vec3(0, 0, 1);
			}
			else
			{
				face->m_pcontact[3] = 0;
				lm.m_num_constraints = 1;
				lm.m_dirs[0] = m_faceRigidConstraints[i][j].m_normal;
			}
			m_lagrangeMultipliers.push_back(lm);
		}
	}
}

//
void DeformableContactProjection::applyDynamicFriction(TVStack& f)
{
	for (i32 i = 0; i < m_softBodies.size(); ++i)
	{
		for (i32 j = 0; j < m_nodeRigidConstraints[i].size(); ++j)
		{
			const DeformableNodeRigidContactConstraint& constraint = m_nodeRigidConstraints[i][j];
			const SoftBody::Node* node = constraint.m_node;
			if (node->m_im != 0)
			{
				i32 index = node->index;
				f[index] += constraint.getDv(node) * (1. / node->m_im);
			}
		}
		for (i32 j = 0; j < m_faceRigidConstraints[i].size(); ++j)
		{
			const DeformableFaceRigidContactConstraint& constraint = m_faceRigidConstraints[i][j];
			const SoftBody::Face* face = constraint.getContact()->m_face;
			for (i32 k = 0; k < 3; ++k)
			{
				const SoftBody::Node* node = face->m_n[k];
				if (node->m_im != 0)
				{
					i32 index = node->index;
					f[index] += constraint.getDv(node) * (1. / node->m_im);
				}
			}
		}
		for (i32 j = 0; j < m_deformableConstraints[i].size(); ++j)
		{
			const DeformableFaceNodeContactConstraint& constraint = m_deformableConstraints[i][j];
			const SoftBody::Face* face = constraint.getContact()->m_face;
			const SoftBody::Node* node = constraint.getContact()->m_node;
			if (node->m_im != 0)
			{
				i32 index = node->index;
				f[index] += constraint.getDv(node) * (1. / node->m_im);
			}
			for (i32 k = 0; k < 3; ++k)
			{
				const SoftBody::Node* node = face->m_n[k];
				if (node->m_im != 0)
				{
					i32 index = node->index;
					f[index] += constraint.getDv(node) * (1. / node->m_im);
				}
			}
		}
	}
}

void DeformableContactProjection::reinitialize(bool nodeUpdated)
{
	i32 N = m_softBodies.size();
	if (nodeUpdated)
	{
		m_staticConstraints.resize(N);
		m_nodeAnchorConstraints.resize(N);
		m_nodeRigidConstraints.resize(N);
		m_faceRigidConstraints.resize(N);
		m_deformableConstraints.resize(N);
	}
	for (i32 i = 0; i < N; ++i)
	{
		m_staticConstraints[i].clear();
		m_nodeAnchorConstraints[i].clear();
		m_nodeRigidConstraints[i].clear();
		m_faceRigidConstraints[i].clear();
		m_deformableConstraints[i].clear();
	}
#ifndef USE_MGS
	m_projectionsDict.clear();
#else
	m_projections.clear();
#endif
	m_lagrangeMultipliers.clear();
}
