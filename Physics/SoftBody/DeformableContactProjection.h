#ifndef DRX3D_CONTACT_PROJECTION_H
#define DRX3D_CONTACT_PROJECTION_H
#include <drx3D/Physics/SoftBody/CGProjection.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <drx3D/Physics/SoftBody/DeformableContactConstraint.h>
#include <drx3D/Maths/Linear/HashMap.h>
#include <drx3D/Maths/Linear/ReducedVector.h>
#include <drx3D/Maths/Linear/ModifiedGramSchmidt.h>
#include <vector>

struct LagrangeMultiplier
{
	i32 m_num_constraints;  // Number of constraints
	i32 m_num_nodes;        // Number of nodes in these constraints
	Scalar m_weights[3];  // weights of the nodes involved, same size as m_num_nodes
	Vec3 m_dirs[3];    // Constraint directions, same size of m_num_constraints;
	i32 m_indices[3];       // indices of the nodes involved, same size as m_num_nodes;
};

class DeformableContactProjection
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	AlignedObjectArray<SoftBody*>& m_softBodies;

	// all constraints involving face
	AlignedObjectArray<DeformableContactConstraint*> m_allFaceConstraints;
#ifndef USE_MGS
	// map from node index to projection directions
	HashMap<HashInt, AlignedObjectArray<Vec3> > m_projectionsDict;
#else
	AlignedObjectArray<ReducedVector> m_projections;
#endif

	AlignedObjectArray<LagrangeMultiplier> m_lagrangeMultipliers;

	// map from node index to static constraint
	AlignedObjectArray<AlignedObjectArray<DeformableStaticConstraint> > m_staticConstraints;
	// map from node index to node rigid constraint
	AlignedObjectArray<AlignedObjectArray<DeformableNodeRigidContactConstraint> > m_nodeRigidConstraints;
	// map from node index to face rigid constraint
	AlignedObjectArray<AlignedObjectArray<DeformableFaceRigidContactConstraint> > m_faceRigidConstraints;
	// map from node index to deformable constraint
	AlignedObjectArray<AlignedObjectArray<DeformableFaceNodeContactConstraint> > m_deformableConstraints;
	// map from node index to node anchor constraint
	AlignedObjectArray<AlignedObjectArray<DeformableNodeAnchorConstraint> > m_nodeAnchorConstraints;

	bool m_useStrainLimiting;

	DeformableContactProjection(AlignedObjectArray<SoftBody*>& softBodies)
		: m_softBodies(softBodies)
	{
	}

	virtual ~DeformableContactProjection()
	{
	}

	// apply the constraints to the rhs of the linear solve
	virtual void project(TVStack& x);

	// add friction force to the rhs of the linear solve
	virtual void applyDynamicFriction(TVStack& f);

	// update and solve the constraints
	virtual Scalar update(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal);

	// Add constraints to m_constraints. In addition, the constraints that each vertex own are recorded in m_constraintsDict.
	virtual void setConstraints(const ContactSolverInfo& infoGlobal);

	// Set up projections for each vertex by adding the projection direction to
	virtual void setProjection();

	virtual void reinitialize(bool nodeUpdated);

	Scalar solveSplitImpulse(CollisionObject2** deformableBodies, i32 numDeformableBodies, const ContactSolverInfo& infoGlobal);

	virtual void setLagrangeMultiplier();

	void checkConstraints(const TVStack& x);
};
#endif /* DeformableContactProjection_h */
