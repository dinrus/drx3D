#ifndef DRX3D_CG_PROJECTION_H
#define DRX3D_CG_PROJECTION_H

#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>

struct SBDeformableContactConstraint
{
	const SoftBody::Node* m_node;
	AlignedObjectArray<const SoftBody::RContact*> m_contact;
	AlignedObjectArray<Vec3> m_total_normal_dv;
	AlignedObjectArray<Vec3> m_total_tangent_dv;
	AlignedObjectArray<bool> m_static;
	AlignedObjectArray<bool> m_can_be_dynamic;

	SBDeformableContactConstraint(const SoftBody::RContact& rcontact) : m_node(rcontact.m_node)
	{
		append(rcontact);
	}

	SBDeformableContactConstraint() : m_node(nullptr)
	{
		m_contact.push_back(nullptr);
	}

	void append(const SoftBody::RContact& rcontact)
	{
		m_contact.push_back(&rcontact);
		m_total_normal_dv.push_back(Vec3(0, 0, 0));
		m_total_tangent_dv.push_back(Vec3(0, 0, 0));
		m_static.push_back(false);
		m_can_be_dynamic.push_back(true);
	}

	void replace(const SoftBody::RContact& rcontact)
	{
		m_contact.clear();
		m_total_normal_dv.clear();
		m_total_tangent_dv.clear();
		m_static.clear();
		m_can_be_dynamic.clear();
		append(rcontact);
	}

	~SBDeformableContactConstraint()
	{
	}
};

class CGProjection
{
public:
	typedef AlignedObjectArray<Vec3> TVStack;
	typedef AlignedObjectArray<AlignedObjectArray<Vec3> > TVArrayStack;
	typedef AlignedObjectArray<AlignedObjectArray<Scalar> > TArrayStack;
	AlignedObjectArray<SoftBody*>& m_softBodies;
	const Scalar& m_dt;
	// map from node indices to node pointers
	const AlignedObjectArray<SoftBody::Node*>* m_nodes;

	CGProjection(AlignedObjectArray<SoftBody*>& softBodies, const Scalar& dt)
		: m_softBodies(softBodies), m_dt(dt)
	{
	}

	virtual ~CGProjection()
	{
	}

	// apply the constraints
	virtual void project(TVStack& x) = 0;

	virtual void setConstraints() = 0;

	// update the constraints
	virtual Scalar update() = 0;

	virtual void reinitialize(bool nodeUpdated)
	{
	}

	virtual void setIndices(const AlignedObjectArray<SoftBody::Node*>* nodes)
	{
		m_nodes = nodes;
	}
};

#endif /* CGProjection_h */
