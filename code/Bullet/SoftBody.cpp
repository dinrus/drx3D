
#include <drx3D/Physics/SoftBody/SoftBodyInternals.h>
#include <drx3D/Physics/SoftBody/SoftBodySolvers.h>
#include <drx3D/Physics/SoftBody/SoftBodyData.h>
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Maths/Linear/ImplicitQRSVD.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <iostream>
//
static inline DbvtNode* buildTreeBottomUp ( AlignedObjectArray<DbvtNode*>& leafNodes, AlignedObjectArray<AlignedObjectArray<i32> >& adj )
{
	i32 N = leafNodes.size();

	if ( N == 0 )
	{
		return NULL;
	}

	while ( N > 1 )
	{
		AlignedObjectArray<bool> marked;
		AlignedObjectArray<DbvtNode*> newLeafNodes;
		AlignedObjectArray<std::pair<i32, i32> > childIds;
		AlignedObjectArray<AlignedObjectArray<i32> > newAdj;
		marked.resize ( N );

		for ( i32 i = 0; i < N; ++i )
			marked[i] = false;

		// pair adjacent nodes into new(parent) node
		for ( i32 i = 0; i < N; ++i )
		{
			if ( marked[i] )
				continue;

			bool merged = false;

			for ( i32 j = 0; j < adj[i].size(); ++j )
			{
				i32 n = adj[i][j];

				if ( !marked[adj[i][j]] )
				{
					DbvtNode* node = new ( AlignedAlloc ( sizeof ( DbvtNode ), 16 ) ) DbvtNode();
					node->parent = NULL;
					node->childs[0] = leafNodes[i];
					node->childs[1] = leafNodes[n];
					leafNodes[i]->parent = node;
					leafNodes[n]->parent = node;
					newLeafNodes.push_back ( node );
					childIds.push_back ( std::make_pair ( i, n ) );
					merged = true;
					marked[n] = true;
					break;
				}
			}

			if ( !merged )
			{
				newLeafNodes.push_back ( leafNodes[i] );
				childIds.push_back ( std::make_pair ( i, -1 ) );
			}

			marked[i] = true;
		}

		// update adjacency matrix
		newAdj.resize ( newLeafNodes.size() );

		for ( i32 i = 0; i < newLeafNodes.size(); ++i )
		{
			for ( i32 j = i + 1; j < newLeafNodes.size(); ++j )
			{
				bool neighbor = false;
				const AlignedObjectArray<i32>& leftChildNeighbors = adj[childIds[i].first];

				for ( i32 k = 0; k < leftChildNeighbors.size(); ++k )
				{
					if ( leftChildNeighbors[k] == childIds[j].first || leftChildNeighbors[k] == childIds[j].second )
					{
						neighbor = true;
						break;
					}
				}

				if ( !neighbor && childIds[i].second != -1 )
				{
					const AlignedObjectArray<i32>& rightChildNeighbors = adj[childIds[i].second];

					for ( i32 k = 0; k < rightChildNeighbors.size(); ++k )
					{
						if ( rightChildNeighbors[k] == childIds[j].first || rightChildNeighbors[k] == childIds[j].second )
						{
							neighbor = true;
							break;
						}
					}
				}

				if ( neighbor )
				{
					newAdj[i].push_back ( j );
					newAdj[j].push_back ( i );
				}
			}
		}

		leafNodes = newLeafNodes;

		//this assignment leaks memory, the assignment doesn't do a deep copy, for now a manual copy
		//adj = newAdj;
		adj.clear();
		adj.resize ( newAdj.size() );

		for ( i32 i = 0; i < newAdj.size(); i++ )
		{
			for ( i32 j = 0; j < newAdj[i].size(); j++ )
			{
				adj[i].push_back ( newAdj[i][j] );
			}
		}

		N = leafNodes.size();
	}

	return leafNodes[0];
}

//
SoftBody::SoftBody ( SoftBodyWorldInfo* worldInfo, i32 node_count, const Vec3* x, const Scalar* m )
		: m_softBodySolver ( 0 ), m_worldInfo ( worldInfo )
{
	/* Init     */
	initDefaults();

	/* Default material */
	Material* pm = appendMaterial();
	pm->m_kLST = 1;
	pm->m_kAST = 1;
	pm->m_kVST = 1;
	pm->m_flags = fMaterial::Default;

	/* Nodes            */
	const Scalar margin = getCollisionShape()->getMargin();
	m_nodes.resize ( node_count );
	m_X.resize ( node_count );

	for ( i32 i = 0, ni = node_count; i < ni; ++i )
	{
		Node& n = m_nodes[i];
		ZeroInitialize ( n );
		n.m_x = x ? *x++ : Vec3 ( 0, 0, 0 );
		n.m_q = n.m_x;
		n.m_im = m ? *m++ : 1;
		n.m_im = n.m_im > 0 ? 1 / n.m_im : 0;
		n.m_leaf = m_ndbvt.insert ( DbvtVolume::FromCR ( n.m_x, margin ), &n );
		n.m_material = pm;
		m_X[i] = n.m_x;
	}

	updateBounds();

	setCollisionQuadrature ( 3 );
	m_fdbvnt = 0;
}

SoftBody::SoftBody ( SoftBodyWorldInfo* worldInfo )
		: m_worldInfo ( worldInfo )
{
	initDefaults();
}

void SoftBody::initDefaults()
{
	m_internalType = CO_SOFT_BODY;
	m_cfg.aeromodel = eAeroModel::V_Point;
	m_cfg.kVCF = 1;
	m_cfg.kDG = 0;
	m_cfg.kLF = 0;
	m_cfg.kDP = 0;
	m_cfg.kPR = 0;
	m_cfg.kVC = 0;
	m_cfg.kDF = ( Scalar ) 0.2;
	m_cfg.kMT = 0;
	m_cfg.kCHR = ( Scalar ) 1.0;
	m_cfg.kKHR = ( Scalar ) 0.1;
	m_cfg.kSHR = ( Scalar ) 1.0;
	m_cfg.kAHR = ( Scalar ) 0.7;
	m_cfg.kSRHR_CL = ( Scalar ) 0.1;
	m_cfg.kSKHR_CL = ( Scalar ) 1;
	m_cfg.kSSHR_CL = ( Scalar ) 0.5;
	m_cfg.kSR_SPLT_CL = ( Scalar ) 0.5;
	m_cfg.kSK_SPLT_CL = ( Scalar ) 0.5;
	m_cfg.kSS_SPLT_CL = ( Scalar ) 0.5;
	m_cfg.maxvolume = ( Scalar ) 1;
	m_cfg.timescale = 1;
	m_cfg.viterations = 0;
	m_cfg.piterations = 1;
	m_cfg.diterations = 0;
	m_cfg.citerations = 4;
	m_cfg.drag = 0;
	m_cfg.m_maxStress = 0;
	m_cfg.collisions = fCollision::Default;
	m_pose.m_bvolume = false;
	m_pose.m_bframe = false;
	m_pose.m_volume = 0;
	m_pose.m_com = Vec3 ( 0, 0, 0 );
	m_pose.m_rot.setIdentity();
	m_pose.m_scl.setIdentity();
	m_tag = 0;
	m_timeacc = 0;
	m_bUpdateRtCst = true;
	m_bounds[0] = Vec3 ( 0, 0, 0 );
	m_bounds[1] = Vec3 ( 0, 0, 0 );
	m_worldTransform.setIdentity();
	setSolver ( eSolverPresets::Positions );

	/* Collision shape  */
	///for now, create a collision shape internally
	m_collisionShape = new SoftBodyCollisionShape ( this );
	m_collisionShape->setMargin ( 0.25f );

	m_worldTransform.setIdentity();

	m_windVelocity = Vec3 ( 0, 0, 0 );
	m_restLengthScale = Scalar ( 1.0 );
	m_dampingCoefficient = 1.0;
	m_sleepingThreshold = .04;
	m_useSelfCollision = false;
	m_collisionFlags = 0;
	m_softSoftCollision = false;
	m_maxSpeedSquared = 0;
	m_repulsionStiffness = 0.5;
	m_gravityFactor = 1;
	m_cacheBarycenter = false;
	m_fdbvnt = 0;

	// reduced flag
	m_reducedModel = false;
}

//
SoftBody::~SoftBody()
{
	//for now, delete the internal shape
	delete m_collisionShape;
	i32 i;

	releaseClusters();

	for ( i = 0; i < m_materials.size(); ++i )
		AlignedFree ( m_materials[i] );

	for ( i = 0; i < m_joints.size(); ++i )
		AlignedFree ( m_joints[i] );

	if ( m_fdbvnt )
		delete m_fdbvnt;
}

//
bool SoftBody::checkLink ( i32 node0, i32 node1 ) const
{
	return ( checkLink ( &m_nodes[node0], &m_nodes[node1] ) );
}

//
bool SoftBody::checkLink ( const Node* node0, const Node* node1 ) const
{
	const Node* n[] = {node0, node1};

	for ( i32 i = 0, ni = m_links.size(); i < ni; ++i )
	{
		const Link& l = m_links[i];

		if ( ( l.m_n[0] == n[0] && l.m_n[1] == n[1] ) ||
			 ( l.m_n[0] == n[1] && l.m_n[1] == n[0] ) )
		{
			return ( true );
		}
	}

	return ( false );
}

//
bool SoftBody::checkFace ( i32 node0, i32 node1, i32 node2 ) const
{
	const Node* n[] = {&m_nodes[node0],
					   &m_nodes[node1],
					   &m_nodes[node2]
					  };

	for ( i32 i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		const Face& f = m_faces[i];
		i32 c = 0;

		for ( i32 j = 0; j < 3; ++j )
		{
			if ( ( f.m_n[j] == n[0] ) ||
				 ( f.m_n[j] == n[1] ) ||
				 ( f.m_n[j] == n[2] ) )
				c |= 1 << j;
			else
				break;
		}

		if ( c == 7 )
			return ( true );
	}

	return ( false );
}

//
SoftBody::Material* SoftBody::appendMaterial()
{
	Material* pm = new ( AlignedAlloc ( sizeof ( Material ), 16 ) ) Material();

	if ( m_materials.size() > 0 )
		*pm = *m_materials[0];
	else
		ZeroInitialize ( *pm );

	m_materials.push_back ( pm );

	return ( pm );
}

//
void SoftBody::appendNote ( tukk text,
		const Vec3& o,
		const Vec4& c,
		Node* n0,
		Node* n1,
		Node* n2,
		Node* n3 )
{
	Note n;
	ZeroInitialize ( n );
	n.m_rank = 0;
	n.m_text = text;
	n.m_offset = o;
	n.m_coords[0] = c.x();
	n.m_coords[1] = c.y();
	n.m_coords[2] = c.z();
	n.m_coords[3] = c.w();
	n.m_nodes[0] = n0;
	n.m_rank += n0 ? 1 : 0;
	n.m_nodes[1] = n1;
	n.m_rank += n1 ? 1 : 0;
	n.m_nodes[2] = n2;
	n.m_rank += n2 ? 1 : 0;
	n.m_nodes[3] = n3;
	n.m_rank += n3 ? 1 : 0;
	m_notes.push_back ( n );
}

//
void SoftBody::appendNote ( tukk text,
		const Vec3& o,
		Node* feature )
{
	appendNote ( text, o, Vec4 ( 1, 0, 0, 0 ), feature );
}

//
void SoftBody::appendNote ( tukk text,
		const Vec3& o,
		Link* feature )
{
	static const Scalar w = 1 / ( Scalar ) 2;
	appendNote ( text, o, Vec4 ( w, w, 0, 0 ), feature->m_n[0],
				 feature->m_n[1] );
}

//
void SoftBody::appendNote ( tukk text,
		const Vec3& o,
		Face* feature )
{
	static const Scalar w = 1 / ( Scalar ) 3;
	appendNote ( text, o, Vec4 ( w, w, w, 0 ), feature->m_n[0],
				 feature->m_n[1],
				 feature->m_n[2] );
}

//
void SoftBody::appendNode ( const Vec3& x, Scalar m )
{
	if ( m_nodes.capacity() == m_nodes.size() )
	{
		pointersToIndices();
		m_nodes.reserve ( m_nodes.size() * 2 + 1 );
		indicesToPointers();
	}

	const Scalar margin = getCollisionShape()->getMargin();

	m_nodes.push_back ( Node() );

	Node& n = m_nodes[m_nodes.size() - 1];

	ZeroInitialize ( n );

	n.m_x = x;

	n.m_q = n.m_x;

	n.m_im = m > 0 ? 1 / m : 0;

	n.m_material = m_materials[0];

	n.m_leaf = m_ndbvt.insert ( DbvtVolume::FromCR ( n.m_x, margin ), &n );
}

//
void SoftBody::appendLink ( i32 model, Material* mat )
{
	Link l;

	if ( model >= 0 )
		l = m_links[model];
	else
	{
		ZeroInitialize ( l );
		l.m_material = mat ? mat : m_materials[0];
	}

	m_links.push_back ( l );
}

//
void SoftBody::appendLink ( i32 node0,
		i32 node1,
		Material* mat,
		bool bcheckexist )
{
	appendLink ( &m_nodes[node0], &m_nodes[node1], mat, bcheckexist );
}

//
void SoftBody::appendLink ( Node* node0,
		Node* node1,
		Material* mat,
		bool bcheckexist )
{
	if ( ( !bcheckexist ) || ( !checkLink ( node0, node1 ) ) )
	{
		appendLink ( -1, mat );
		Link& l = m_links[m_links.size() - 1];
		l.m_n[0] = node0;
		l.m_n[1] = node1;
		l.m_rl = ( l.m_n[0]->m_x - l.m_n[1]->m_x ).length();
		m_bUpdateRtCst = true;
	}
}

//
void SoftBody::appendFace ( i32 model, Material* mat )
{
	Face f;

	if ( model >= 0 )
	{
		f = m_faces[model];
	}

	else
	{
		ZeroInitialize ( f );
		f.m_material = mat ? mat : m_materials[0];
	}

	m_faces.push_back ( f );
}

//
void SoftBody::appendFace ( i32 node0, i32 node1, i32 node2, Material* mat )
{
	if ( node0 == node1 )
		return;

	if ( node1 == node2 )
		return;

	if ( node2 == node0 )
		return;

	appendFace ( -1, mat );

	Face& f = m_faces[m_faces.size() - 1];

	Assert ( node0 != node1 );

	Assert ( node1 != node2 );

	Assert ( node2 != node0 );

	f.m_n[0] = &m_nodes[node0];

	f.m_n[1] = &m_nodes[node1];

	f.m_n[2] = &m_nodes[node2];

	f.m_ra = AreaOf ( f.m_n[0]->m_x,
					  f.m_n[1]->m_x,
					  f.m_n[2]->m_x );

	m_bUpdateRtCst = true;
}

//
void SoftBody::appendTetra ( i32 model, Material* mat )
{
	Tetra t;

	if ( model >= 0 )
		t = m_tetras[model];
	else
	{
		ZeroInitialize ( t );
		t.m_material = mat ? mat : m_materials[0];
	}

	m_tetras.push_back ( t );
}

//
void SoftBody::appendTetra ( i32 node0,
		i32 node1,
		i32 node2,
		i32 node3,
		Material* mat )
{
	appendTetra ( -1, mat );
	Tetra& t = m_tetras[m_tetras.size() - 1];
	t.m_n[0] = &m_nodes[node0];
	t.m_n[1] = &m_nodes[node1];
	t.m_n[2] = &m_nodes[node2];
	t.m_n[3] = &m_nodes[node3];
	t.m_rv = VolumeOf ( t.m_n[0]->m_x, t.m_n[1]->m_x, t.m_n[2]->m_x, t.m_n[3]->m_x );
	m_bUpdateRtCst = true;
}

//

void SoftBody::appendAnchor ( i32 node, RigidBody* body, bool disableCollisionBetweenLinkedBodies, Scalar influence )
{
	Vec3 local = body->getWorldTransform().inverse() * m_nodes[node].m_x;
	appendAnchor ( node, body, local, disableCollisionBetweenLinkedBodies, influence );
}

//
void SoftBody::appendAnchor ( i32 node, RigidBody* body, const Vec3& localPivot, bool disableCollisionBetweenLinkedBodies, Scalar influence )
{
	if ( disableCollisionBetweenLinkedBodies )
	{
		if ( m_collisionDisabledObjects.findLinearSearch ( body ) == m_collisionDisabledObjects.size() )
		{
			m_collisionDisabledObjects.push_back ( body );
		}
	}

	Anchor a;

	a.m_node = &m_nodes[node];
	a.m_body = body;
	a.m_local = localPivot;
	a.m_node->m_battach = 1;
	a.m_influence = influence;
	m_anchors.push_back ( a );
}

//
void SoftBody::appendDeformableAnchor ( i32 node, RigidBody* body )
{
	DeformableNodeRigidAnchor c;
	SoftBody::Node& n = m_nodes[node];
	const Scalar ima = n.m_im;
	const Scalar imb = body->getInvMass();
	Vec3 nrm;
	const CollisionShape* shp = body->getCollisionShape();
	const Transform2& wtr = body->getWorldTransform();
	Scalar dst =
		m_worldInfo->m_sparsesdf.Evaluate (
			wtr.invXform ( m_nodes[node].m_x ),
			shp,
			nrm,
			0 );

	c.m_cti.m_colObj = body;
	c.m_cti.m_normal = wtr.getBasis() * nrm;
	c.m_cti.m_offset = dst;
	c.m_node = &m_nodes[node];
	const Scalar fc = m_cfg.kDF * body->getFriction();
	c.m_c2 = ima;
	c.m_c3 = fc;
	c.m_c4 = body->isStaticOrKinematicObject() ? m_cfg.kKHR : m_cfg.kCHR;
	static const Matrix3x3 iwiStatic ( 0, 0, 0, 0, 0, 0, 0, 0, 0 );
	const Matrix3x3& iwi = body->getInvInertiaTensorWorld();
	const Vec3 ra = n.m_x - wtr.getOrigin();

	c.m_c0 = ImpulseMatrix ( 1, ima, imb, iwi, ra );
	c.m_c1 = ra;
	c.m_local = body->getWorldTransform().inverse() * m_nodes[node].m_x;
	c.m_node->m_battach = 1;
	m_deformableAnchors.push_back ( c );
}

void SoftBody::removeAnchor ( i32 node )
{
	const SoftBody::Node& n = m_nodes[node];

	for ( i32 i = 0; i < m_deformableAnchors.size(); )
	{
		const DeformableNodeRigidAnchor& c = m_deformableAnchors[i];

		if ( c.m_node == &n )
		{
			m_deformableAnchors.removeAtIndex ( i );
		}

		else
		{
			i++;
		}
	}
}

//
void SoftBody::appendDeformableAnchor ( i32 node, MultiBodyLinkCollider* link )
{
	DeformableNodeRigidAnchor c;
	SoftBody::Node& n = m_nodes[node];
	const Scalar ima = n.m_im;
	Vec3 nrm;
	const CollisionShape* shp = link->getCollisionShape();
	const Transform2& wtr = link->getWorldTransform();
	Scalar dst =
		m_worldInfo->m_sparsesdf.Evaluate (
			wtr.invXform ( m_nodes[node].m_x ),
			shp,
			nrm,
			0 );
	c.m_cti.m_colObj = link;
	c.m_cti.m_normal = wtr.getBasis() * nrm;
	c.m_cti.m_offset = dst;
	c.m_node = &m_nodes[node];
	const Scalar fc = m_cfg.kDF * link->getFriction();
	c.m_c2 = ima;
	c.m_c3 = fc;
	c.m_c4 = link->isStaticOrKinematicObject() ? m_cfg.kKHR : m_cfg.kCHR;
	Vec3 normal = c.m_cti.m_normal;
	Vec3 t1 = generateUnitOrthogonalVector ( normal );
	Vec3 t2 = Cross ( normal, t1 );
	MultiBodyJacobianData jacobianData_normal, jacobianData_t1, jacobianData_t2;
	findJacobian ( link, jacobianData_normal, c.m_node->m_x, normal );
	findJacobian ( link, jacobianData_t1, c.m_node->m_x, t1 );
	findJacobian ( link, jacobianData_t2, c.m_node->m_x, t2 );

	Scalar* J_n = &jacobianData_normal.m_jacobians[0];
	Scalar* J_t1 = &jacobianData_t1.m_jacobians[0];
	Scalar* J_t2 = &jacobianData_t2.m_jacobians[0];

	Scalar* u_n = &jacobianData_normal.m_deltaVelocitiesUnitImpulse[0];
	Scalar* u_t1 = &jacobianData_t1.m_deltaVelocitiesUnitImpulse[0];
	Scalar* u_t2 = &jacobianData_t2.m_deltaVelocitiesUnitImpulse[0];

	Matrix3x3 rot ( normal.getX(), normal.getY(), normal.getZ(),
					t1.getX(), t1.getY(), t1.getZ(),
					t2.getX(), t2.getY(), t2.getZ() ); // world frame to local frame
	i32k ndof = link->m_multiBody->getNumDofs() + 6;
	Matrix3x3 local_impulse_matrix = ( Diagonal ( n.m_im ) + OuterProduct ( J_n, J_t1, J_t2, u_n, u_t1, u_t2, ndof ) ).inverse();
	c.m_c0 = rot.transpose() * local_impulse_matrix * rot;
	c.jacobianData_normal = jacobianData_normal;
	c.jacobianData_t1 = jacobianData_t1;
	c.jacobianData_t2 = jacobianData_t2;
	c.t1 = t1;
	c.t2 = t2;
	const Vec3 ra = n.m_x - wtr.getOrigin();
	c.m_c1 = ra;
	c.m_local = link->getWorldTransform().inverse() * m_nodes[node].m_x;
	c.m_node->m_battach = 1;
	m_deformableAnchors.push_back ( c );
}

//
void SoftBody::appendLinearJoint ( const LJoint::Specs& specs, Cluster* body0, Body body1 )
{
	LJoint* pj = new ( AlignedAlloc ( sizeof ( LJoint ), 16 ) ) LJoint();
	pj->m_bodies[0] = body0;
	pj->m_bodies[1] = body1;
	pj->m_refs[0] = pj->m_bodies[0].xform().inverse() * specs.position;
	pj->m_refs[1] = pj->m_bodies[1].xform().inverse() * specs.position;
	pj->m_cfm = specs.cfm;
	pj->m_erp = specs.erp;
	pj->m_split = specs.split;
	m_joints.push_back ( pj );
}

//
void SoftBody::appendLinearJoint ( const LJoint::Specs& specs, Body body )
{
	appendLinearJoint ( specs, m_clusters[0], body );
}

//
void SoftBody::appendLinearJoint ( const LJoint::Specs& specs, SoftBody* body )
{
	appendLinearJoint ( specs, m_clusters[0], body->m_clusters[0] );
}

//
void SoftBody::appendAngularJoint ( const AJoint::Specs& specs, Cluster* body0, Body body1 )
{
	AJoint* pj = new ( AlignedAlloc ( sizeof ( AJoint ), 16 ) ) AJoint();
	pj->m_bodies[0] = body0;
	pj->m_bodies[1] = body1;
	pj->m_refs[0] = pj->m_bodies[0].xform().inverse().getBasis() * specs.axis;
	pj->m_refs[1] = pj->m_bodies[1].xform().inverse().getBasis() * specs.axis;
	pj->m_cfm = specs.cfm;
	pj->m_erp = specs.erp;
	pj->m_split = specs.split;
	pj->m_icontrol = specs.icontrol;
	m_joints.push_back ( pj );
}

//
void SoftBody::appendAngularJoint ( const AJoint::Specs& specs, Body body )
{
	appendAngularJoint ( specs, m_clusters[0], body );
}

//
void SoftBody::appendAngularJoint ( const AJoint::Specs& specs, SoftBody* body )
{
	appendAngularJoint ( specs, m_clusters[0], body->m_clusters[0] );
}

//
void SoftBody::addForce ( const Vec3& force )
{
	for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
		addForce ( force, i );
}

//
void SoftBody::addForce ( const Vec3& force, i32 node )
{
	Node& n = m_nodes[node];

	if ( n.m_im > 0 )
	{
		n.m_f += force;
	}
}

void SoftBody::addAeroForceToNode ( const Vec3& windVelocity, i32 nodeIndex )
{
	Assert ( nodeIndex >= 0 && nodeIndex < m_nodes.size() );

	const Scalar dt = m_sst.sdt;
	const Scalar kLF = m_cfg.kLF;
	const Scalar kDG = m_cfg.kDG;
	//const Scalar kPR = m_cfg.kPR;
	//const Scalar kVC = m_cfg.kVC;
	const bool as_lift = kLF > 0;
	const bool as_drag = kDG > 0;
	const bool as_aero = as_lift || as_drag;
	const bool as_vaero = as_aero && ( m_cfg.aeromodel < SoftBody::eAeroModel::F_TwoSided );

	Node& n = m_nodes[nodeIndex];

	if ( n.m_im > 0 )
	{
		SoftBody::sMedium medium;

		EvaluateMedium ( m_worldInfo, n.m_x, medium );
		medium.m_velocity = windVelocity;
		medium.m_density = m_worldInfo->air_density;

		/* Aerodynamics         */

		if ( as_vaero )
		{
			const Vec3 rel_v = n.m_v - medium.m_velocity;
			const Scalar rel_v_len = rel_v.length();
			const Scalar rel_v2 = rel_v.length2();

			if ( rel_v2 > SIMD_EPSILON )
			{
				const Vec3 rel_v_nrm = rel_v.normalized();
				Vec3 nrm = n.m_n;

				if ( m_cfg.aeromodel == SoftBody::eAeroModel::V_TwoSidedLiftDrag )
				{
					nrm *= ( Scalar ) ( ( Dot ( nrm, rel_v ) < 0 ) ? -1 : + 1 );
					Vec3 fDrag ( 0, 0, 0 );
					Vec3 fLift ( 0, 0, 0 );

					Scalar n_dot_v = nrm.dot ( rel_v_nrm );
					Scalar tri_area = 0.5f * n.m_area;

					fDrag = 0.5f * kDG * medium.m_density * rel_v2 * tri_area * n_dot_v * ( -rel_v_nrm );

					// Check angle of attack
					// cos(10º) = 0.98480

					if ( 0 < n_dot_v && n_dot_v < 0.98480f )
						fLift = 0.5f * kLF * medium.m_density * rel_v_len * tri_area * Sqrt ( 1.0f - n_dot_v * n_dot_v ) * ( nrm.cross ( rel_v_nrm ).cross ( rel_v_nrm ) );

					// Check if the velocity change resulted by aero drag force exceeds the current velocity of the node.
					Vec3 del_v_by_fDrag = fDrag * n.m_im * m_sst.sdt;

					Scalar del_v_by_fDrag_len2 = del_v_by_fDrag.length2();

					Scalar v_len2 = n.m_v.length2();

					if ( del_v_by_fDrag_len2 >= v_len2 && del_v_by_fDrag_len2 > 0 )
					{
						Scalar del_v_by_fDrag_len = del_v_by_fDrag.length();
						Scalar v_len = n.m_v.length();
						fDrag *= Scalar ( 0.8 ) * ( v_len / del_v_by_fDrag_len );
					}

					n.m_f += fDrag;

					n.m_f += fLift;
				}

				else
					if ( m_cfg.aeromodel == SoftBody::eAeroModel::V_Point || m_cfg.aeromodel == SoftBody::eAeroModel::V_OneSided || m_cfg.aeromodel == SoftBody::eAeroModel::V_TwoSided )
					{
						if ( m_cfg.aeromodel == SoftBody::eAeroModel::V_TwoSided )
							nrm *= ( Scalar ) ( ( Dot ( nrm, rel_v ) < 0 ) ? -1 : + 1 );

						const Scalar dvn = Dot ( rel_v, nrm );

						/* Compute forces   */
						if ( dvn > 0 )
						{
							Vec3 force ( 0, 0, 0 );
							const Scalar c0 = n.m_area * dvn * rel_v2 / 2;
							const Scalar c1 = c0 * medium.m_density;
							force += nrm * ( -c1 * kLF );
							force += rel_v.normalized() * ( -c1 * kDG );
							ApplyClampedForce ( n, force, dt );
						}
					}
			}
		}
	}
}

void SoftBody::addAeroForceToFace ( const Vec3& windVelocity, i32 faceIndex )
{
	const Scalar dt = m_sst.sdt;
	const Scalar kLF = m_cfg.kLF;
	const Scalar kDG = m_cfg.kDG;
	//  const Scalar kPR = m_cfg.kPR;
	//  const Scalar kVC = m_cfg.kVC;
	const bool as_lift = kLF > 0;
	const bool as_drag = kDG > 0;
	const bool as_aero = as_lift || as_drag;
	const bool as_faero = as_aero && ( m_cfg.aeromodel >= SoftBody::eAeroModel::F_TwoSided );

	if ( as_faero )
	{
		SoftBody::Face& f = m_faces[faceIndex];

		SoftBody::sMedium medium;

		const Vec3 v = ( f.m_n[0]->m_v + f.m_n[1]->m_v + f.m_n[2]->m_v ) / 3;
		const Vec3 x = ( f.m_n[0]->m_x + f.m_n[1]->m_x + f.m_n[2]->m_x ) / 3;
		EvaluateMedium ( m_worldInfo, x, medium );
		medium.m_velocity = windVelocity;
		medium.m_density = m_worldInfo->air_density;
		const Vec3 rel_v = v - medium.m_velocity;
		const Scalar rel_v_len = rel_v.length();
		const Scalar rel_v2 = rel_v.length2();

		if ( rel_v2 > SIMD_EPSILON )
		{
			const Vec3 rel_v_nrm = rel_v.normalized();
			Vec3 nrm = f.m_normal;

			if ( m_cfg.aeromodel == SoftBody::eAeroModel::F_TwoSidedLiftDrag )
			{
				nrm *= ( Scalar ) ( ( Dot ( nrm, rel_v ) < 0 ) ? -1 : + 1 );

				Vec3 fDrag ( 0, 0, 0 );
				Vec3 fLift ( 0, 0, 0 );

				Scalar n_dot_v = nrm.dot ( rel_v_nrm );
				Scalar tri_area = 0.5f * f.m_ra;

				fDrag = 0.5f * kDG * medium.m_density * rel_v2 * tri_area * n_dot_v * ( -rel_v_nrm );

				// Check angle of attack
				// cos(10º) = 0.98480

				if ( 0 < n_dot_v && n_dot_v < 0.98480f )
					fLift = 0.5f * kLF * medium.m_density * rel_v_len * tri_area * Sqrt ( 1.0f - n_dot_v * n_dot_v ) * ( nrm.cross ( rel_v_nrm ).cross ( rel_v_nrm ) );

				fDrag /= 3;

				fLift /= 3;

				for ( i32 j = 0; j < 3; ++j )
				{
					if ( f.m_n[j]->m_im > 0 )
					{
						// Check if the velocity change resulted by aero drag force exceeds the current velocity of the node.
						Vec3 del_v_by_fDrag = fDrag * f.m_n[j]->m_im * m_sst.sdt;
						Scalar del_v_by_fDrag_len2 = del_v_by_fDrag.length2();
						Scalar v_len2 = f.m_n[j]->m_v.length2();

						if ( del_v_by_fDrag_len2 >= v_len2 && del_v_by_fDrag_len2 > 0 )
						{
							Scalar del_v_by_fDrag_len = del_v_by_fDrag.length();
							Scalar v_len = f.m_n[j]->m_v.length();
							fDrag *= Scalar ( 0.8 ) * ( v_len / del_v_by_fDrag_len );
						}

						f.m_n[j]->m_f += fDrag;

						f.m_n[j]->m_f += fLift;
					}
				}
			}

			else
				if ( m_cfg.aeromodel == SoftBody::eAeroModel::F_OneSided || m_cfg.aeromodel == SoftBody::eAeroModel::F_TwoSided )
				{
					if ( m_cfg.aeromodel == SoftBody::eAeroModel::F_TwoSided )
						nrm *= ( Scalar ) ( ( Dot ( nrm, rel_v ) < 0 ) ? -1 : + 1 );

					const Scalar dvn = Dot ( rel_v, nrm );

					/* Compute forces   */
					if ( dvn > 0 )
					{
						Vec3 force ( 0, 0, 0 );
						const Scalar c0 = f.m_ra * dvn * rel_v2;
						const Scalar c1 = c0 * medium.m_density;
						force += nrm * ( -c1 * kLF );
						force += rel_v.normalized() * ( -c1 * kDG );
						force /= 3;

						for ( i32 j = 0; j < 3; ++j )
							ApplyClampedForce ( *f.m_n[j], force, dt );
					}
				}
		}
	}
}

//
void SoftBody::addVelocity ( const Vec3& velocity )
{
	for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
		addVelocity ( velocity, i );
}

/* Set velocity for the entire body                                     */
void SoftBody::setVelocity ( const Vec3& velocity )
{
	for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Node& n = m_nodes[i];

		if ( n.m_im > 0 )
		{
			n.m_v = velocity;
			n.m_vn = velocity;
		}
	}
}

//
void SoftBody::addVelocity ( const Vec3& velocity, i32 node )
{
	Node& n = m_nodes[node];

	if ( n.m_im > 0 )
	{
		n.m_v += velocity;
	}
}

//
void SoftBody::setMass ( i32 node, Scalar mass )
{
	m_nodes[node].m_im = mass > 0 ? 1 / mass : 0;
	m_bUpdateRtCst = true;
}

//
Scalar SoftBody::getMass ( i32 node ) const
{
	return ( m_nodes[node].m_im > 0 ? 1 / m_nodes[node].m_im : 0 );
}

//
Scalar SoftBody::getTotalMass() const
{
	Scalar mass = 0;

	for ( i32 i = 0; i < m_nodes.size(); ++i )
	{
		mass += getMass ( i );
	}

	return ( mass );
}

//
void SoftBody::setTotalMass ( Scalar mass, bool fromfaces )
{
	i32 i;

	if ( fromfaces )
	{
		for ( i = 0; i < m_nodes.size(); ++i )
		{
			m_nodes[i].m_im = 0;
		}

		for ( i = 0; i < m_faces.size(); ++i )
		{
			const Face& f = m_faces[i];
			const Scalar twicearea = AreaOf ( f.m_n[0]->m_x,
					f.m_n[1]->m_x,
					f.m_n[2]->m_x );

			for ( i32 j = 0; j < 3; ++j )
			{
				f.m_n[j]->m_im += twicearea;
			}
		}

		for ( i = 0; i < m_nodes.size(); ++i )
		{
			m_nodes[i].m_im = 1 / m_nodes[i].m_im;
		}
	}

	const Scalar tm = getTotalMass();

	const Scalar itm = 1 / tm;

	for ( i = 0; i < m_nodes.size(); ++i )
	{
		m_nodes[i].m_im /= itm * mass;
	}

	m_bUpdateRtCst = true;
}

//
void SoftBody::setTotalDensity ( Scalar density )
{
	setTotalMass ( getVolume() * density, true );
}

//
void SoftBody::setVolumeMass ( Scalar mass )
{
	AlignedObjectArray<Scalar> ranks;
	ranks.resize ( m_nodes.size(), 0 );
	i32 i;

	for ( i = 0; i < m_nodes.size(); ++i )
	{
		m_nodes[i].m_im = 0;
	}

	for ( i = 0; i < m_tetras.size(); ++i )
	{
		const Tetra& t = m_tetras[i];

		for ( i32 j = 0; j < 4; ++j )
		{
			t.m_n[j]->m_im += Fabs ( t.m_rv );
			ranks[i32 ( t.m_n[j] - &m_nodes[0] ) ] += 1;
		}
	}

	for ( i = 0; i < m_nodes.size(); ++i )
	{
		if ( m_nodes[i].m_im > 0 )
		{
			m_nodes[i].m_im = ranks[i] / m_nodes[i].m_im;
		}
	}

	setTotalMass ( mass, false );
}

//
void SoftBody::setVolumeDensity ( Scalar density )
{
	Scalar volume = 0;

	for ( i32 i = 0; i < m_tetras.size(); ++i )
	{
		const Tetra& t = m_tetras[i];

		for ( i32 j = 0; j < 4; ++j )
		{
			volume += Fabs ( t.m_rv );
		}
	}

	setVolumeMass ( volume * density / 6 );
}

//
Vec3 SoftBody::getLinearVelocity()
{
	Vec3 total_momentum = Vec3 ( 0, 0, 0 );

	for ( i32 i = 0; i < m_nodes.size(); ++i )
	{
		Scalar mass = m_nodes[i].m_im == 0 ? 0 : 1.0 / m_nodes[i].m_im;
		total_momentum += mass * m_nodes[i].m_v;
	}

	Scalar total_mass = getTotalMass();

	return total_mass == 0 ? total_momentum : total_momentum / total_mass;
}

//
void SoftBody::setLinearVelocity ( const Vec3& linVel )
{
	Vec3 old_vel = getLinearVelocity();
	Vec3 diff = linVel - old_vel;

	for ( i32 i = 0; i < m_nodes.size(); ++i )
		m_nodes[i].m_v += diff;
}

//
void SoftBody::setAngularVelocity ( const Vec3& angVel )
{
	Vec3 old_vel = getLinearVelocity();
	Vec3 com = getCenterOfMass();

	for ( i32 i = 0; i < m_nodes.size(); ++i )
	{
		m_nodes[i].m_v = angVel.cross ( m_nodes[i].m_x - com ) + old_vel;
	}
}

//
Transform2 SoftBody::getRigidTransform()
{
	Vec3 t = getCenterOfMass();
	Matrix3x3 S;
	S.setZero();
	// Get rotation that minimizes L2 difference: \sum_i || RX_i + t - x_i ||
	// It's important to make sure that S has the correct signs.
	// SVD is only unique up to the ordering of singular values.
	// SVD will manipulate U and V to ensure the ordering of singular values. If all three singular
	// vaues are negative, SVD will permute colums of U to make two of them positive.

	for ( i32 i = 0; i < m_nodes.size(); ++i )
	{
		S -= OuterProduct ( m_X[i], t - m_nodes[i].m_x );
	}

	Vec3 sigma;

	Matrix3x3 U, V;
	singularValueDecomposition ( S, U, sigma, V );
	Matrix3x3 R = V * U.transpose();
	Transform2 trs;
	trs.setIdentity();
	trs.setOrigin ( t );
	trs.setBasis ( R );
	return trs;
}

//
void SoftBody::transformTo ( const Transform2& trs )
{
	// get the current best rigid fit
	Transform2 current_transform = getRigidTransform();
	// apply transform in material space
	Transform2 new_transform = trs * current_transform.inverse();
	transform ( new_transform );
}

//
void SoftBody::transform ( const Transform2& trs )
{
	const Scalar margin = getCollisionShape()->getMargin();
	ATTRIBUTE_ALIGNED16 ( DbvtVolume )
	vol;

	for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Node& n = m_nodes[i];
		n.m_x = trs * n.m_x;
		n.m_q = trs * n.m_q;
		n.m_n = trs.getBasis() * n.m_n;
		vol = DbvtVolume::FromCR ( n.m_x, margin );

		m_ndbvt.update ( n.m_leaf, vol );
	}

	updateNormals();

	updateBounds();
	updateConstants();
}

//
void SoftBody::translate ( const Vec3& trs )
{
	Transform2 t;
	t.setIdentity();
	t.setOrigin ( trs );
	transform ( t );
}

//
void SoftBody::rotate ( const Quat& rot )
{
	Transform2 t;
	t.setIdentity();
	t.setRotation ( rot );
	transform ( t );
}

//
void SoftBody::scale ( const Vec3& scl )
{
	const Scalar margin = getCollisionShape()->getMargin();
	ATTRIBUTE_ALIGNED16 ( DbvtVolume )
	vol;

	for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Node& n = m_nodes[i];
		n.m_x *= scl;
		n.m_q *= scl;
		vol = DbvtVolume::FromCR ( n.m_x, margin );
		m_ndbvt.update ( n.m_leaf, vol );
	}

	updateNormals();

	updateBounds();
	updateConstants();
	initializeDmInverse();
}

//
Scalar SoftBody::getRestLengthScale()
{
	return m_restLengthScale;
}

//
void SoftBody::setRestLengthScale ( Scalar restLengthScale )
{
	for ( i32 i = 0, ni = m_links.size(); i < ni; ++i )
	{
		Link& l = m_links[i];
		l.m_rl = l.m_rl / m_restLengthScale * restLengthScale;
		l.m_c1 = l.m_rl * l.m_rl;
	}

	m_restLengthScale = restLengthScale;

	if ( getActivationState() == ISLAND_SLEEPING )
		activate();
}

//
void SoftBody::setPose ( bool bvolume, bool bframe )
{
	m_pose.m_bvolume = bvolume;
	m_pose.m_bframe = bframe;
	i32 i, ni;

	/* Weights      */
	const Scalar omass = getTotalMass();
	const Scalar kmass = omass * m_nodes.size() * 1000;
	Scalar tmass = omass;
	m_pose.m_wgh.resize ( m_nodes.size() );

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		if ( m_nodes[i].m_im <= 0 )
			tmass += kmass;
	}

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Node& n = m_nodes[i];
		m_pose.m_wgh[i] = n.m_im > 0 ? 1 / ( m_nodes[i].m_im * tmass ) : kmass / tmass;
	}

	/* Pos      */
	const Vec3 com = evaluateCom();

	m_pose.m_pos.resize ( m_nodes.size() );

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		m_pose.m_pos[i] = m_nodes[i].m_x - com;
	}

	m_pose.m_volume = bvolume ? getVolume() : 0;

	m_pose.m_com = com;
	m_pose.m_rot.setIdentity();
	m_pose.m_scl.setIdentity();
	/* Aqq      */
	m_pose.m_aqq[0] =
		m_pose.m_aqq[1] =
			m_pose.m_aqq[2] = Vec3 ( 0, 0, 0 );

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		const Vec3& q = m_pose.m_pos[i];
		const Vec3 mq = m_pose.m_wgh[i] * q;
		m_pose.m_aqq[0] += mq.x() * q;
		m_pose.m_aqq[1] += mq.y() * q;
		m_pose.m_aqq[2] += mq.z() * q;
	}

	m_pose.m_aqq = m_pose.m_aqq.inverse();

	updateConstants();
}

void SoftBody::resetLinkRestLengths()
{
	for ( i32 i = 0, ni = m_links.size(); i < ni; ++i )
	{
		Link& l = m_links[i];
		l.m_rl = ( l.m_n[0]->m_x - l.m_n[1]->m_x ).length();
		l.m_c1 = l.m_rl * l.m_rl;
	}
}

//
Scalar SoftBody::getVolume() const
{
	Scalar vol = 0;

	if ( m_nodes.size() > 0 )
	{
		i32 i, ni;

		const Vec3 org = m_nodes[0].m_x;

		for ( i = 0, ni = m_faces.size(); i < ni; ++i )
		{
			const Face& f = m_faces[i];
			vol += Dot ( f.m_n[0]->m_x - org, Cross ( f.m_n[1]->m_x - org, f.m_n[2]->m_x - org ) );
		}

		vol /= ( Scalar ) 6;
	}

	return ( vol );
}

//
i32 SoftBody::clusterCount() const
{
	return ( m_clusters.size() );
}

//
Vec3 SoftBody::clusterCom ( const Cluster* cluster )
{
	Vec3 com ( 0, 0, 0 );

	for ( i32 i = 0, ni = cluster->m_nodes.size(); i < ni; ++i )
	{
		com += cluster->m_nodes[i]->m_x * cluster->m_masses[i];
	}

	return ( com * cluster->m_imass );
}

//
Vec3 SoftBody::clusterCom ( i32 cluster ) const
{
	return ( clusterCom ( m_clusters[cluster] ) );
}

//
Vec3 SoftBody::clusterVelocity ( const Cluster* cluster, const Vec3& rpos )
{
	return ( cluster->m_lv + Cross ( cluster->m_av, rpos ) );
}

//
void SoftBody::clusterVImpulse ( Cluster* cluster, const Vec3& rpos, const Vec3& impulse )
{
	const Vec3 li = cluster->m_imass * impulse;
	const Vec3 ai = cluster->m_invwi * Cross ( rpos, impulse );
	cluster->m_vimpulses[0] += li;
	cluster->m_lv += li;
	cluster->m_vimpulses[1] += ai;
	cluster->m_av += ai;
	cluster->m_nvimpulses++;
}

//
void SoftBody::clusterDImpulse ( Cluster* cluster, const Vec3& rpos, const Vec3& impulse )
{
	const Vec3 li = cluster->m_imass * impulse;
	const Vec3 ai = cluster->m_invwi * Cross ( rpos, impulse );
	cluster->m_dimpulses[0] += li;
	cluster->m_dimpulses[1] += ai;
	cluster->m_ndimpulses++;
}

//
void SoftBody::clusterImpulse ( Cluster* cluster, const Vec3& rpos, const Impulse& impulse )
{
	if ( impulse.m_asVelocity )
		clusterVImpulse ( cluster, rpos, impulse.m_velocity );

	if ( impulse.m_asDrift )
		clusterDImpulse ( cluster, rpos, impulse.m_drift );
}

//
void SoftBody::clusterVAImpulse ( Cluster* cluster, const Vec3& impulse )
{
	const Vec3 ai = cluster->m_invwi * impulse;
	cluster->m_vimpulses[1] += ai;
	cluster->m_av += ai;
	cluster->m_nvimpulses++;
}

//
void SoftBody::clusterDAImpulse ( Cluster* cluster, const Vec3& impulse )
{
	const Vec3 ai = cluster->m_invwi * impulse;
	cluster->m_dimpulses[1] += ai;
	cluster->m_ndimpulses++;
}

//
void SoftBody::clusterAImpulse ( Cluster* cluster, const Impulse& impulse )
{
	if ( impulse.m_asVelocity )
		clusterVAImpulse ( cluster, impulse.m_velocity );

	if ( impulse.m_asDrift )
		clusterDAImpulse ( cluster, impulse.m_drift );
}

//
void SoftBody::clusterDCImpulse ( Cluster* cluster, const Vec3& impulse )
{
	cluster->m_dimpulses[0] += impulse * cluster->m_imass;
	cluster->m_ndimpulses++;
}

struct NodeLinks
{
	AlignedObjectArray<i32> m_links;
};

//
i32 SoftBody::generateBendingConstraints ( i32 distance, Material* mat )
{
	i32 i, j;

	if ( distance > 1 )
	{
		/* Build graph  */
		i32k n = m_nodes.size();
		const unsigned inf = ( ~ ( unsigned ) 0 ) >> 1;
		unsigned* adj = new unsigned[n * n];

#define IDX(_x_, _y_) ((_y_)*n + (_x_))

		for ( j = 0; j < n; ++j )
		{
			for ( i = 0; i < n; ++i )
			{
				if ( i != j )
				{
					adj[IDX ( i, j ) ] = adj[IDX ( j, i ) ] = inf;
				}

				else
				{
					adj[IDX ( i, j ) ] = adj[IDX ( j, i ) ] = 0;
				}
			}
		}

		for ( i = 0; i < m_links.size(); ++i )
		{
			i32k ia = ( i32 ) ( m_links[i].m_n[0] - &m_nodes[0] );
			i32k ib = ( i32 ) ( m_links[i].m_n[1] - &m_nodes[0] );
			adj[IDX ( ia, ib ) ] = 1;
			adj[IDX ( ib, ia ) ] = 1;
		}

		//special optimized case for distance == 2

		if ( distance == 2 )
		{
			AlignedObjectArray<NodeLinks> nodeLinks;

			/* Build node links */
			nodeLinks.resize ( m_nodes.size() );

			for ( i = 0; i < m_links.size(); ++i )
			{
				i32k ia = ( i32 ) ( m_links[i].m_n[0] - &m_nodes[0] );
				i32k ib = ( i32 ) ( m_links[i].m_n[1] - &m_nodes[0] );

				if ( nodeLinks[ia].m_links.findLinearSearch ( ib ) == nodeLinks[ia].m_links.size() )
					nodeLinks[ia].m_links.push_back ( ib );

				if ( nodeLinks[ib].m_links.findLinearSearch ( ia ) == nodeLinks[ib].m_links.size() )
					nodeLinks[ib].m_links.push_back ( ia );
			}

			for ( i32 ii = 0; ii < nodeLinks.size(); ii++ )
			{
				i32 i = ii;

				for ( i32 jj = 0; jj < nodeLinks[ii].m_links.size(); jj++ )
				{
					i32 k = nodeLinks[ii].m_links[jj];

					for ( i32 kk = 0; kk < nodeLinks[k].m_links.size(); kk++ )
					{
						i32 j = nodeLinks[k].m_links[kk];

						if ( i != j )
						{
							const unsigned sum = adj[IDX ( i, k ) ] + adj[IDX ( k, j ) ];
							Assert ( sum == 2 );

							if ( adj[IDX ( i, j ) ] > sum )
							{
								adj[IDX ( i, j ) ] = adj[IDX ( j, i ) ] = sum;
							}
						}
					}
				}
			}
		}

		else
		{
			///generic Floyd's algorithm
			for ( i32 k = 0; k < n; ++k )
			{
				for ( j = 0; j < n; ++j )
				{
					for ( i = j + 1; i < n; ++i )
					{
						const unsigned sum = adj[IDX ( i, k ) ] + adj[IDX ( k, j ) ];

						if ( adj[IDX ( i, j ) ] > sum )
						{
							adj[IDX ( i, j ) ] = adj[IDX ( j, i ) ] = sum;
						}
					}
				}
			}
		}

		/* Build links  */
		i32 nlinks = 0;

		for ( j = 0; j < n; ++j )
		{
			for ( i = j + 1; i < n; ++i )
			{
				if ( adj[IDX ( i, j ) ] == ( unsigned ) distance )
				{
					appendLink ( i, j, mat );
					m_links[m_links.size() - 1].m_bbending = 1;
					++nlinks;
				}
			}
		}

		delete[] adj;

		return ( nlinks );
	}

	return ( 0 );
}

//
void SoftBody::randomizeConstraints()
{
	u64 seed = 243703;
#define NEXTRAND (seed = (1664525L * seed + 1013904223L) & 0xffffffff)
	i32 i, ni;

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		Swap ( m_links[i], m_links[NEXTRAND % ni] );
	}

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		Swap ( m_faces[i], m_faces[NEXTRAND % ni] );
	}

#undef NEXTRAND
}

void SoftBody::updateState ( const AlignedObjectArray<Vec3>& q, const AlignedObjectArray<Vec3>& v )
{
	i32 node_count = m_nodes.size();
	Assert ( node_count == q.size() );
	Assert ( node_count == v.size() );

	for ( i32 i = 0; i < node_count; i++ )
	{
		Node& n = m_nodes[i];
		n.m_x = q[i];
		n.m_q = q[i];
		n.m_v = v[i];
		n.m_vn = v[i];
	}
}

//
void SoftBody::releaseCluster ( i32 index )
{
	Cluster* c = m_clusters[index];

	if ( c->m_leaf )
		m_cdbvt.remove ( c->m_leaf );

	c->~Cluster();

	AlignedFree ( c );

	m_clusters.remove ( c );
}

//
void SoftBody::releaseClusters()
{
	while ( m_clusters.size() > 0 )
		releaseCluster ( 0 );
}

//
i32 SoftBody::generateClusters ( i32 k, i32 maxiterations )
{
	i32 i;
	releaseClusters();
	m_clusters.resize ( d3Min ( k, m_nodes.size() ) );

	for ( i = 0; i < m_clusters.size(); ++i )
	{
		m_clusters[i] = new ( AlignedAlloc ( sizeof ( Cluster ), 16 ) ) Cluster();
		m_clusters[i]->m_collide = true;
	}

	k = m_clusters.size();

	if ( k > 0 )
	{
		/* Initialize       */
		AlignedObjectArray<Vec3> centers;
		Vec3 cog ( 0, 0, 0 );
		i32 i;

		for ( i = 0; i < m_nodes.size(); ++i )
		{
			cog += m_nodes[i].m_x;
			m_clusters[ ( i * 29873 ) % m_clusters.size() ]->m_nodes.push_back ( &m_nodes[i] );
		}

		cog /= ( Scalar ) m_nodes.size();

		centers.resize ( k, cog );
		/* Iterate          */
		const Scalar slope = 16;
		bool changed;
		i32 iterations = 0;

		do
		{
			const Scalar w = 2 - d3Min<Scalar> ( 1, iterations / slope );
			changed = false;
			iterations++;
			i32 i;

			for ( i = 0; i < k; ++i )
			{
				Vec3 c ( 0, 0, 0 );

				for ( i32 j = 0; j < m_clusters[i]->m_nodes.size(); ++j )
				{
					c += m_clusters[i]->m_nodes[j]->m_x;
				}

				if ( m_clusters[i]->m_nodes.size() )
				{
					c /= ( Scalar ) m_clusters[i]->m_nodes.size();
					c = centers[i] + ( c - centers[i] ) * w;
					changed |= ( ( c - centers[i] ).length2() > SIMD_EPSILON );
					centers[i] = c;
					m_clusters[i]->m_nodes.resize ( 0 );
				}
			}

			for ( i = 0; i < m_nodes.size(); ++i )
			{
				const Vec3 nx = m_nodes[i].m_x;
				i32 kbest = 0;
				Scalar kdist = ClusterMetric ( centers[0], nx );

				for ( i32 j = 1; j < k; ++j )
				{
					const Scalar d = ClusterMetric ( centers[j], nx );

					if ( d < kdist )
					{
						kbest = j;
						kdist = d;
					}
				}

				m_clusters[kbest]->m_nodes.push_back ( &m_nodes[i] );
			}
		}

		while ( changed && ( iterations < maxiterations ) );

		/* Merge        */
		AlignedObjectArray<i32> cids;

		cids.resize ( m_nodes.size(), -1 );

		for ( i = 0; i < m_clusters.size(); ++i )
		{
			for ( i32 j = 0; j < m_clusters[i]->m_nodes.size(); ++j )
			{
				cids[i32 ( m_clusters[i]->m_nodes[j] - &m_nodes[0] ) ] = i;
			}
		}

		for ( i = 0; i < m_faces.size(); ++i )
		{
			i32k idx[] = {i32 ( m_faces[i].m_n[0] - &m_nodes[0] ),
							   i32 ( m_faces[i].m_n[1] - &m_nodes[0] ),
							   i32 ( m_faces[i].m_n[2] - &m_nodes[0] )
							  };

			for ( i32 j = 0; j < 3; ++j )
			{
				i32k cid = cids[idx[j]];

				for ( i32 q = 1; q < 3; ++q )
				{
					i32k kid = idx[ ( j + q ) % 3];

					if ( cids[kid] != cid )
					{
						if ( m_clusters[cid]->m_nodes.findLinearSearch ( &m_nodes[kid] ) == m_clusters[cid]->m_nodes.size() )
						{
							m_clusters[cid]->m_nodes.push_back ( &m_nodes[kid] );
						}
					}
				}
			}
		}

		/* Master       */

		if ( m_clusters.size() > 1 )
		{
			Cluster* pmaster = new ( AlignedAlloc ( sizeof ( Cluster ), 16 ) ) Cluster();
			pmaster->m_collide = false;
			pmaster->m_nodes.reserve ( m_nodes.size() );

			for ( i32 i = 0; i < m_nodes.size(); ++i )
				pmaster->m_nodes.push_back ( &m_nodes[i] );

			m_clusters.push_back ( pmaster );

			Swap ( m_clusters[0], m_clusters[m_clusters.size() - 1] );
		}

		/* Terminate    */

		for ( i = 0; i < m_clusters.size(); ++i )
		{
			if ( m_clusters[i]->m_nodes.size() == 0 )
			{
				releaseCluster ( i-- );
			}
		}
	}

	else
	{
		//create a cluster for each tetrahedron (if tetrahedra exist) or each face
		if ( m_tetras.size() )
		{
			m_clusters.resize ( m_tetras.size() );

			for ( i = 0; i < m_clusters.size(); ++i )
			{
				m_clusters[i] = new ( AlignedAlloc ( sizeof ( Cluster ), 16 ) ) Cluster();
				m_clusters[i]->m_collide = true;
			}

			for ( i = 0; i < m_tetras.size(); i++ )
			{
				for ( i32 j = 0; j < 4; j++ )
				{
					m_clusters[i]->m_nodes.push_back ( m_tetras[i].m_n[j] );
				}
			}
		}

		else
		{
			m_clusters.resize ( m_faces.size() );

			for ( i = 0; i < m_clusters.size(); ++i )
			{
				m_clusters[i] = new ( AlignedAlloc ( sizeof ( Cluster ), 16 ) ) Cluster();
				m_clusters[i]->m_collide = true;
			}

			for ( i = 0; i < m_faces.size(); ++i )
			{
				for ( i32 j = 0; j < 3; ++j )
				{
					m_clusters[i]->m_nodes.push_back ( m_faces[i].m_n[j] );
				}
			}
		}
	}

	if ( m_clusters.size() )
	{
		initializeClusters();
		updateClusters();

		//for self-collision
		m_clusterConnectivity.resize ( m_clusters.size() * m_clusters.size() );
		{
			for ( i32 c0 = 0; c0 < m_clusters.size(); c0++ )
			{
				m_clusters[c0]->m_clusterIndex = c0;

				for ( i32 c1 = 0; c1 < m_clusters.size(); c1++ )
				{
					bool connected = false;
					Cluster* cla = m_clusters[c0];
					Cluster* clb = m_clusters[c1];

					for ( i32 i = 0; !connected && i < cla->m_nodes.size(); i++ )
					{
						for ( i32 j = 0; j < clb->m_nodes.size(); j++ )
						{
							if ( cla->m_nodes[i] == clb->m_nodes[j] )
							{
								connected = true;
								break;
							}
						}
					}

					m_clusterConnectivity[c0 + c1 * m_clusters.size() ] = connected;
				}
			}
		}
	}

	return ( m_clusters.size() );
}

//
void SoftBody::refine ( ImplicitFn* ifn, Scalar accurary, bool cut )
{
	const Node* nbase = &m_nodes[0];
	i32 ncount = m_nodes.size();
	SymMatrix<i32> edges ( ncount, -2 );
	i32 newnodes = 0;
	i32 i, j, k, ni;

	/* Filter out       */

	for ( i = 0; i < m_links.size(); ++i )
	{
		Link& l = m_links[i];

		if ( l.m_bbending )
		{
			if ( !SameSign ( ifn->Eval ( l.m_n[0]->m_x ), ifn->Eval ( l.m_n[1]->m_x ) ) )
			{
				Swap ( m_links[i], m_links[m_links.size() - 1] );
				m_links.pop_back();
				--i;
			}
		}
	}

	/* Fill edges       */

	for ( i = 0; i < m_links.size(); ++i )
	{
		Link& l = m_links[i];
		edges ( i32 ( l.m_n[0] - nbase ), i32 ( l.m_n[1] - nbase ) ) = -1;
	}

	for ( i = 0; i < m_faces.size(); ++i )
	{
		Face& f = m_faces[i];
		edges ( i32 ( f.m_n[0] - nbase ), i32 ( f.m_n[1] - nbase ) ) = -1;
		edges ( i32 ( f.m_n[1] - nbase ), i32 ( f.m_n[2] - nbase ) ) = -1;
		edges ( i32 ( f.m_n[2] - nbase ), i32 ( f.m_n[0] - nbase ) ) = -1;
	}

	/* Intersect        */

	for ( i = 0; i < ncount; ++i )
	{
		for ( j = i + 1; j < ncount; ++j )
		{
			if ( edges ( i, j ) == -1 )
			{
				Node& a = m_nodes[i];
				Node& b = m_nodes[j];
				const Scalar t = ImplicitSolve ( ifn, a.m_x, b.m_x, accurary );

				if ( t > 0 )
				{
					const Vec3 x = Lerp ( a.m_x, b.m_x, t );
					const Vec3 v = Lerp ( a.m_v, b.m_v, t );
					Scalar m = 0;

					if ( a.m_im > 0 )
					{
						if ( b.m_im > 0 )
						{
							const Scalar ma = 1 / a.m_im;
							const Scalar mb = 1 / b.m_im;
							const Scalar mc = Lerp ( ma, mb, t );
							const Scalar f = ( ma + mb ) / ( ma + mb + mc );
							a.m_im = 1 / ( ma * f );
							b.m_im = 1 / ( mb * f );
							m = mc * f;
						}

						else
						{
							a.m_im /= 0.5f;
							m = 1 / a.m_im;
						}
					}

					else
					{
						if ( b.m_im > 0 )
						{
							b.m_im /= 0.5f;
							m = 1 / b.m_im;
						}

						else
							m = 0;
					}

					appendNode ( x, m );

					edges ( i, j ) = m_nodes.size() - 1;
					m_nodes[edges ( i, j ) ].m_v = v;
					++newnodes;
				}
			}
		}
	}

	nbase = &m_nodes[0];

	/* Refine links     */

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		Link& feat = m_links[i];
		i32k idx[] = {i32 ( feat.m_n[0] - nbase ),
						   i32 ( feat.m_n[1] - nbase )
						  };

		if ( ( idx[0] < ncount ) && ( idx[1] < ncount ) )
		{
			i32k ni = edges ( idx[0], idx[1] );

			if ( ni > 0 )
			{
				appendLink ( i );
				Link* pft[] = {&m_links[i],
							   &m_links[m_links.size() - 1]
							  };
				pft[0]->m_n[0] = &m_nodes[idx[0]];
				pft[0]->m_n[1] = &m_nodes[ni];
				pft[1]->m_n[0] = &m_nodes[ni];
				pft[1]->m_n[1] = &m_nodes[idx[1]];
			}
		}
	}

	/* Refine faces     */

	for ( i = 0; i < m_faces.size(); ++i )
	{
		const Face& feat = m_faces[i];
		i32k idx[] = {i32 ( feat.m_n[0] - nbase ),
						   i32 ( feat.m_n[1] - nbase ),
						   i32 ( feat.m_n[2] - nbase )
						  };

		for ( j = 2, k = 0; k < 3; j = k++ )
		{
			if ( ( idx[j] < ncount ) && ( idx[k] < ncount ) )
			{
				i32k ni = edges ( idx[j], idx[k] );

				if ( ni > 0 )
				{
					appendFace ( i );
					i32k l = ( k + 1 ) % 3;
					Face* pft[] = {&m_faces[i],
								   &m_faces[m_faces.size() - 1]
								  };
					pft[0]->m_n[0] = &m_nodes[idx[l]];
					pft[0]->m_n[1] = &m_nodes[idx[j]];
					pft[0]->m_n[2] = &m_nodes[ni];
					pft[1]->m_n[0] = &m_nodes[ni];
					pft[1]->m_n[1] = &m_nodes[idx[k]];
					pft[1]->m_n[2] = &m_nodes[idx[l]];
					appendLink ( ni, idx[l], pft[0]->m_material );
					--i;
					break;
				}
			}
		}
	}

	/* Cut              */

	if ( cut )
	{
		AlignedObjectArray<i32> cnodes;
		i32k pcount = ncount;
		i32 i;
		ncount = m_nodes.size();
		cnodes.resize ( ncount, 0 );
		/* Nodes        */

		for ( i = 0; i < ncount; ++i )
		{
			const Vec3 x = m_nodes[i].m_x;

			if ( ( i >= pcount ) || ( Fabs ( ifn->Eval ( x ) ) < accurary ) )
			{
				const Vec3 v = m_nodes[i].m_v;
				Scalar m = getMass ( i );

				if ( m > 0 )
				{
					m *= 0.5f;
					m_nodes[i].m_im /= 0.5f;
				}

				appendNode ( x, m );

				cnodes[i] = m_nodes.size() - 1;
				m_nodes[cnodes[i]].m_v = v;
			}
		}

		nbase = &m_nodes[0];

		/* Links        */

		for ( i = 0, ni = m_links.size(); i < ni; ++i )
		{
			i32k id[] = {i32 ( m_links[i].m_n[0] - nbase ),
							  i32 ( m_links[i].m_n[1] - nbase )
							 };
			i32 todetach = 0;

			if ( cnodes[id[0]] && cnodes[id[1]] )
			{
				appendLink ( i );
				todetach = m_links.size() - 1;
			}

			else
			{
				if ( ( ( ifn->Eval ( m_nodes[id[0]].m_x ) < accurary ) &&
					   ( ifn->Eval ( m_nodes[id[1]].m_x ) < accurary ) ) )
					todetach = i;
			}

			if ( todetach )
			{
				Link& l = m_links[todetach];

				for ( i32 j = 0; j < 2; ++j )
				{
					i32 cn = cnodes[i32 ( l.m_n[j] - nbase ) ];

					if ( cn )
						l.m_n[j] = &m_nodes[cn];
				}
			}
		}

		/* Faces        */

		for ( i = 0, ni = m_faces.size(); i < ni; ++i )
		{
			Node** n = m_faces[i].m_n;

			if ( ( ifn->Eval ( n[0]->m_x ) < accurary ) &&
				 ( ifn->Eval ( n[1]->m_x ) < accurary ) &&
				 ( ifn->Eval ( n[2]->m_x ) < accurary ) )
			{
				for ( i32 j = 0; j < 3; ++j )
				{
					i32 cn = cnodes[i32 ( n[j] - nbase ) ];

					if ( cn )
						n[j] = &m_nodes[cn];
				}
			}
		}

		/* Clean orphans    */
		i32 nnodes = m_nodes.size();

		AlignedObjectArray<i32> ranks;

		AlignedObjectArray<i32> todelete;

		ranks.resize ( nnodes, 0 );

		for ( i = 0, ni = m_links.size(); i < ni; ++i )
		{
			for ( i32 j = 0; j < 2; ++j )
				ranks[i32 ( m_links[i].m_n[j] - nbase ) ]++;
		}

		for ( i = 0, ni = m_faces.size(); i < ni; ++i )
		{
			for ( i32 j = 0; j < 3; ++j )
				ranks[i32 ( m_faces[i].m_n[j] - nbase ) ]++;
		}

		for ( i = 0; i < m_links.size(); ++i )
		{
			i32k id[] = {i32 ( m_links[i].m_n[0] - nbase ),
							  i32 ( m_links[i].m_n[1] - nbase )
							 };
			const bool sg[] = {ranks[id[0]] == 1,
							   ranks[id[1]] == 1
							  };

			if ( sg[0] || sg[1] )
			{
				--ranks[id[0]];
				--ranks[id[1]];
				Swap ( m_links[i], m_links[m_links.size() - 1] );
				m_links.pop_back();
				--i;
			}
		}

#if 0
		for ( i = nnodes - 1;i >= 0;--i )
		{
			if ( !ranks[i] )
				todelete.push_back ( i );
		}

		if ( todelete.size() )
		{
			AlignedObjectArray<i32>&    map = ranks;

			for ( i32 i = 0;i < nnodes;++i )
				map[i] = i;

			PointersToIndices ( this );

			for ( i32 i = 0, ni = todelete.size();i < ni;++i )
			{
				i32     j = todelete[i];
				i32&    a = map[j];
				i32&    b = map[--nnodes];
				m_ndbvt.remove ( m_nodes[a].m_leaf );
				m_nodes[a].m_leaf = 0;
				Swap ( m_nodes[a], m_nodes[b] );
				j = a;
				a = b;
				b = j;
			}

			IndicesToPointers ( this, &map[0] );

			m_nodes.resize ( nnodes );
		}

#endif
	}

	m_bUpdateRtCst = true;
}

//
bool SoftBody::cutLink ( const Node* node0, const Node* node1, Scalar position )
{
	return ( cutLink ( i32 ( node0 - &m_nodes[0] ), i32 ( node1 - &m_nodes[0] ), position ) );
}

//
bool SoftBody::cutLink ( i32 node0, i32 node1, Scalar position )
{
	bool done = false;
	i32 i, ni;
	//  const Vec3  d=m_nodes[node0].m_x-m_nodes[node1].m_x;
	const Vec3 x = Lerp ( m_nodes[node0].m_x, m_nodes[node1].m_x, position );
	const Vec3 v = Lerp ( m_nodes[node0].m_v, m_nodes[node1].m_v, position );
	const Scalar m = 1;
	appendNode ( x, m );
	appendNode ( x, m );
	Node* pa = &m_nodes[node0];
	Node* pb = &m_nodes[node1];
	Node* pn[2] = {&m_nodes[m_nodes.size() - 2],
				   &m_nodes[m_nodes.size() - 1]
				  };
	pn[0]->m_v = v;
	pn[1]->m_v = v;

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		i32k mtch = MatchEdge ( m_links[i].m_n[0], m_links[i].m_n[1], pa, pb );

		if ( mtch != -1 )
		{
			appendLink ( i );
			Link* pft[] = {&m_links[i], &m_links[m_links.size() - 1]};
			pft[0]->m_n[1] = pn[mtch];
			pft[1]->m_n[0] = pn[1 - mtch];
			done = true;
		}
	}

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		for ( i32 k = 2, l = 0; l < 3; k = l++ )
		{
			i32k mtch = MatchEdge ( m_faces[i].m_n[k], m_faces[i].m_n[l], pa, pb );

			if ( mtch != -1 )
			{
				appendFace ( i );
				Face* pft[] = {&m_faces[i], &m_faces[m_faces.size() - 1]};
				pft[0]->m_n[l] = pn[mtch];
				pft[1]->m_n[k] = pn[1 - mtch];
				appendLink ( pn[0], pft[0]->m_n[ ( l + 1 ) % 3], pft[0]->m_material, true );
				appendLink ( pn[1], pft[0]->m_n[ ( l + 1 ) % 3], pft[0]->m_material, true );
			}
		}
	}

	if ( !done )
	{
		m_ndbvt.remove ( pn[0]->m_leaf );
		m_ndbvt.remove ( pn[1]->m_leaf );
		m_nodes.pop_back();
		m_nodes.pop_back();
	}

	return ( done );
}

//
bool SoftBody::rayTest ( const Vec3& rayFrom,
		const Vec3& rayTo,
		sRayCast& results )
{
	if ( m_faces.size() && m_fdbvt.empty() )
		initializeFaceTree();

	results.body = this;

	results.fraction = 1.f;

	results.feature = eFeature::None;

	results.index = -1;

	return ( rayTest ( rayFrom, rayTo, results.fraction, results.feature, results.index, false ) != 0 );
}

bool SoftBody::rayFaceTest ( const Vec3& rayFrom,
		const Vec3& rayTo,
		sRayCast& results )
{
	if ( m_faces.size() == 0 )
		return false;
	else
	{
		if ( m_fdbvt.empty() )
			initializeFaceTree();
	}

	results.body = this;

	results.fraction = 1.f;
	results.index = -1;

	return ( rayFaceTest ( rayFrom, rayTo, results.fraction, results.index ) != 0 );
}

//
void SoftBody::setSolver ( eSolverPresets::_ preset )
{
	m_cfg.m_vsequence.clear();
	m_cfg.m_psequence.clear();
	m_cfg.m_dsequence.clear();

	switch ( preset )
	{

		case eSolverPresets::Positions:
			m_cfg.m_psequence.push_back ( ePSolver::Anchors );
			m_cfg.m_psequence.push_back ( ePSolver::RContacts );
			m_cfg.m_psequence.push_back ( ePSolver::SContacts );
			m_cfg.m_psequence.push_back ( ePSolver::Linear );
			break;

		case eSolverPresets::Velocities:
			m_cfg.m_vsequence.push_back ( eVSolver::Linear );

			m_cfg.m_psequence.push_back ( ePSolver::Anchors );
			m_cfg.m_psequence.push_back ( ePSolver::RContacts );
			m_cfg.m_psequence.push_back ( ePSolver::SContacts );

			m_cfg.m_dsequence.push_back ( ePSolver::Linear );
			break;
	}
}

void SoftBody::predictMotion ( Scalar dt )
{
	i32 i, ni;

	/* Update                */

	if ( m_bUpdateRtCst )
	{
		m_bUpdateRtCst = false;
		updateConstants();
		m_fdbvt.clear();

		if ( m_cfg.collisions & fCollision::VF_SS )
		{
			initializeFaceTree();
		}
	}

	/* Prepare                */
	m_sst.sdt = dt * m_cfg.timescale;

	m_sst.isdt = 1 / m_sst.sdt;

	m_sst.velmrg = m_sst.sdt * 3;

	m_sst.radmrg = getCollisionShape()->getMargin();

	m_sst.updmrg = m_sst.radmrg * ( Scalar ) 0.25;

	/* Forces                */
	addVelocity ( m_worldInfo->m_gravity * m_sst.sdt );

	applyForces();

	/* Integrate            */
	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Node& n = m_nodes[i];
		n.m_q = n.m_x;
		Vec3 deltaV = n.m_f * n.m_im * m_sst.sdt;
		{
			Scalar maxDisplacement = m_worldInfo->m_maxDisplacement;
			Scalar clampDeltaV = maxDisplacement / m_sst.sdt;

			for ( i32 c = 0; c < 3; c++ )
			{
				if ( deltaV[c] > clampDeltaV )
				{
					deltaV[c] = clampDeltaV;
				}

				if ( deltaV[c] < -clampDeltaV )
				{
					deltaV[c] = -clampDeltaV;
				}
			}
		}

		n.m_v += deltaV;
		n.m_x += n.m_v * m_sst.sdt;
		n.m_f = Vec3 ( 0, 0, 0 );
	}

	/* Clusters                */
	updateClusters();

	/* Bounds                */
	updateBounds();

	/* Nodes                */
	ATTRIBUTE_ALIGNED16 ( DbvtVolume )
	vol;

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Node& n = m_nodes[i];
		vol = DbvtVolume::FromCR ( n.m_x, m_sst.radmrg );
		m_ndbvt.update ( n.m_leaf,
						 vol,
						 n.m_v * m_sst.velmrg,
						 m_sst.updmrg );
	}

	/* Faces                */

	if ( !m_fdbvt.empty() )
	{
		for ( i32 i = 0; i < m_faces.size(); ++i )
		{
			Face& f = m_faces[i];
			const Vec3 v = ( f.m_n[0]->m_v +
							 f.m_n[1]->m_v +
							 f.m_n[2]->m_v ) /
						   3;
			vol = VolumeOf ( f, m_sst.radmrg );
			m_fdbvt.update ( f.m_leaf,
							 vol,
							 v * m_sst.velmrg,
							 m_sst.updmrg );
		}
	}

	/* Pose                    */
	updatePose();

	/* Match                */
	if ( m_pose.m_bframe && ( m_cfg.kMT > 0 ) )
	{
		const Matrix3x3 posetrs = m_pose.m_rot;

		for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			Node& n = m_nodes[i];

			if ( n.m_im > 0 )
			{
				const Vec3 x = posetrs * m_pose.m_pos[i] + m_pose.m_com;
				n.m_x = Lerp ( n.m_x, x, m_cfg.kMT );
			}
		}
	}

	/* Clear contacts        */
	m_rcontacts.resize ( 0 );

	m_scontacts.resize ( 0 );

	/* Optimize dbvt's        */
	m_ndbvt.optimizeIncremental ( 1 );

	m_fdbvt.optimizeIncremental ( 1 );

	m_cdbvt.optimizeIncremental ( 1 );
}

//
void SoftBody::solveConstraints()
{
	/* Apply clusters       */
	applyClusters ( false );
	/* Prepare links        */

	i32 i, ni;

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		Link& l = m_links[i];
		l.m_c3 = l.m_n[1]->m_q - l.m_n[0]->m_q;
		l.m_c2 = 1 / ( l.m_c3.length2() * l.m_c0 );
	}

	/* Prepare anchors      */

	for ( i = 0, ni = m_anchors.size(); i < ni; ++i )
	{
		Anchor& a = m_anchors[i];
		const Vec3 ra = a.m_body->getWorldTransform().getBasis() * a.m_local;
		a.m_c0 = ImpulseMatrix ( m_sst.sdt,
				 a.m_node->m_im,
				 a.m_body->getInvMass(),
				 a.m_body->getInvInertiaTensorWorld(),
				 ra );
		a.m_c1 = ra;
		a.m_c2 = m_sst.sdt * a.m_node->m_im;
		a.m_body->activate();
	}

	/* Solve velocities     */

	if ( m_cfg.viterations > 0 )
	{
		/* Solve            */
		for ( i32 isolve = 0; isolve < m_cfg.viterations; ++isolve )
		{
			for ( i32 iseq = 0; iseq < m_cfg.m_vsequence.size(); ++iseq )
			{
				getSolver ( m_cfg.m_vsequence[iseq] ) ( this, 1 );
			}
		}

		/* Update           */

		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			Node& n = m_nodes[i];
			n.m_x = n.m_q + n.m_v * m_sst.sdt;
		}
	}

	/* Solve positions      */

	if ( m_cfg.piterations > 0 )
	{
		for ( i32 isolve = 0; isolve < m_cfg.piterations; ++isolve )
		{
			const Scalar ti = isolve / ( Scalar ) m_cfg.piterations;

			for ( i32 iseq = 0; iseq < m_cfg.m_psequence.size(); ++iseq )
			{
				getSolver ( m_cfg.m_psequence[iseq] ) ( this, 1, ti );
			}
		}

		const Scalar vc = m_sst.isdt * ( 1 - m_cfg.kDP );

		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			Node& n = m_nodes[i];
			n.m_v = ( n.m_x - n.m_q ) * vc;
			n.m_f = Vec3 ( 0, 0, 0 );
		}
	}

	/* Solve drift          */

	if ( m_cfg.diterations > 0 )
	{
		const Scalar vcf = m_cfg.kVCF * m_sst.isdt;

		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			Node& n = m_nodes[i];
			n.m_q = n.m_x;
		}

		for ( i32 idrift = 0; idrift < m_cfg.diterations; ++idrift )
		{
			for ( i32 iseq = 0; iseq < m_cfg.m_dsequence.size(); ++iseq )
			{
				getSolver ( m_cfg.m_dsequence[iseq] ) ( this, 1, 0 );
			}
		}

		for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			Node& n = m_nodes[i];
			n.m_v += ( n.m_x - n.m_q ) * vcf;
		}
	}

	/* Apply clusters       */
	dampClusters();

	applyClusters ( true );
}

//
void SoftBody::staticSolve ( i32 iterations )
{
	for ( i32 isolve = 0; isolve < iterations; ++isolve )
	{
		for ( i32 iseq = 0; iseq < m_cfg.m_psequence.size(); ++iseq )
		{
			getSolver ( m_cfg.m_psequence[iseq] ) ( this, 1, 0 );
		}
	}
}

//
void SoftBody::solveCommonConstraints ( SoftBody** /*bodies*/, i32 /*count*/, i32 /*iterations*/ )
{
	/// placeholder
}

//
void SoftBody::solveClusters ( const AlignedObjectArray<SoftBody*>& bodies )
{
	i32k nb = bodies.size();
	i32 iterations = 0;
	i32 i;

	for ( i = 0; i < nb; ++i )
	{
		iterations = d3Max ( iterations, bodies[i]->m_cfg.citerations );
	}

	for ( i = 0; i < nb; ++i )
	{
		bodies[i]->prepareClusters ( iterations );
	}

	for ( i = 0; i < iterations; ++i )
	{
		const Scalar sor = 1;

		for ( i32 j = 0; j < nb; ++j )
		{
			bodies[j]->solveClusters ( sor );
		}
	}

	for ( i = 0; i < nb; ++i )
	{
		bodies[i]->cleanupClusters();
	}
}

//
void SoftBody::integrateMotion()
{
	/* Update           */
	updateNormals();
}

//
SoftBody::RayFromToCaster::RayFromToCaster ( const Vec3& rayFrom, const Vec3& rayTo, Scalar mxt )
{
	m_rayFrom = rayFrom;
	m_rayNormalizedDirection = ( rayTo - rayFrom );
	m_rayTo = rayTo;
	m_mint = mxt;
	m_face = 0;
	m_tests = 0;
}

//
void SoftBody::RayFromToCaster::Process ( const DbvtNode* leaf )
{
	SoftBody::Face& f = * ( SoftBody::Face* ) leaf->data;
	const Scalar t = rayFromToTriangle ( m_rayFrom, m_rayTo, m_rayNormalizedDirection,
					 f.m_n[0]->m_x,
					 f.m_n[1]->m_x,
					 f.m_n[2]->m_x,
					 m_mint );

	if ( ( t > 0 ) && ( t < m_mint ) )
	{
		m_mint = t;
		m_face = &f;
	}

	++m_tests;
}

//
Scalar SoftBody::RayFromToCaster::rayFromToTriangle ( const Vec3& rayFrom,
		const Vec3& rayTo,
		const Vec3& rayNormalizedDirection,
		const Vec3& a,
		const Vec3& b,
		const Vec3& c,
		Scalar maxt )
{
	static const Scalar ceps = -SIMD_EPSILON * 10;
	static const Scalar teps = SIMD_EPSILON * 10;

	const Vec3 n = Cross ( b - a, c - a );
	const Scalar d = Dot ( a, n );
	const Scalar den = Dot ( rayNormalizedDirection, n );

	if ( !FuzzyZero ( den ) )
	{
		const Scalar num = Dot ( rayFrom, n ) - d;
		const Scalar t = -num / den;

		if ( ( t > teps ) && ( t < maxt ) )
		{
			const Vec3 hit = rayFrom + rayNormalizedDirection * t;

			if ( ( Dot ( n, Cross ( a - hit, b - hit ) ) > ceps ) &&
				 ( Dot ( n, Cross ( b - hit, c - hit ) ) > ceps ) &&
				 ( Dot ( n, Cross ( c - hit, a - hit ) ) > ceps ) )
			{
				return ( t );
			}
		}
	}

	return ( -1 );
}

//
void SoftBody::pointersToIndices()
{
#define PTR2IDX(_p_, _b_) reinterpret_cast<SoftBody::Node*>((_p_) - (_b_))
	SoftBody::Node* base = m_nodes.size() ? &m_nodes[0] : 0;
	i32 i, ni;

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		if ( m_nodes[i].m_leaf )
		{
			m_nodes[i].m_leaf->data = * ( uk * ) & i;
		}
	}

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		m_links[i].m_n[0] = PTR2IDX ( m_links[i].m_n[0], base );
		m_links[i].m_n[1] = PTR2IDX ( m_links[i].m_n[1], base );
	}

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		m_faces[i].m_n[0] = PTR2IDX ( m_faces[i].m_n[0], base );
		m_faces[i].m_n[1] = PTR2IDX ( m_faces[i].m_n[1], base );
		m_faces[i].m_n[2] = PTR2IDX ( m_faces[i].m_n[2], base );

		if ( m_faces[i].m_leaf )
		{
			m_faces[i].m_leaf->data = * ( uk * ) & i;
		}
	}

	for ( i = 0, ni = m_anchors.size(); i < ni; ++i )
	{
		m_anchors[i].m_node = PTR2IDX ( m_anchors[i].m_node, base );
	}

	for ( i = 0, ni = m_notes.size(); i < ni; ++i )
	{
		for ( i32 j = 0; j < m_notes[i].m_rank; ++j )
		{
			m_notes[i].m_nodes[j] = PTR2IDX ( m_notes[i].m_nodes[j], base );
		}
	}

#undef PTR2IDX
}

//
void SoftBody::indicesToPointers ( i32k* map )
{
#define IDX2PTR(_p_, _b_) map ? (&(_b_)[map[(((tuk)_p_) - (tuk)0)]]) : (&(_b_)[(((tuk)_p_) - (tuk)0)])
	SoftBody::Node* base = m_nodes.size() ? &m_nodes[0] : 0;
	i32 i, ni;

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		if ( m_nodes[i].m_leaf )
		{
			m_nodes[i].m_leaf->data = &m_nodes[i];
		}
	}

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		m_links[i].m_n[0] = IDX2PTR ( m_links[i].m_n[0], base );
		m_links[i].m_n[1] = IDX2PTR ( m_links[i].m_n[1], base );
	}

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		m_faces[i].m_n[0] = IDX2PTR ( m_faces[i].m_n[0], base );
		m_faces[i].m_n[1] = IDX2PTR ( m_faces[i].m_n[1], base );
		m_faces[i].m_n[2] = IDX2PTR ( m_faces[i].m_n[2], base );

		if ( m_faces[i].m_leaf )
		{
			m_faces[i].m_leaf->data = &m_faces[i];
		}
	}

	for ( i = 0, ni = m_anchors.size(); i < ni; ++i )
	{
		m_anchors[i].m_node = IDX2PTR ( m_anchors[i].m_node, base );
	}

	for ( i = 0, ni = m_notes.size(); i < ni; ++i )
	{
		for ( i32 j = 0; j < m_notes[i].m_rank; ++j )
		{
			m_notes[i].m_nodes[j] = IDX2PTR ( m_notes[i].m_nodes[j], base );
		}
	}

#undef IDX2PTR
}

//
i32 SoftBody::rayTest ( const Vec3& rayFrom, const Vec3& rayTo,
		Scalar& mint, eFeature::_& feature, i32& index, bool bcountonly ) const
{
	i32 cnt = 0;
	Vec3 dir = rayTo - rayFrom;

	if ( bcountonly || m_fdbvt.empty() )
	{
		/* Full search    */

		for ( i32 i = 0, ni = m_faces.size(); i < ni; ++i )
		{
			const SoftBody::Face& f = m_faces[i];

			const Scalar t = RayFromToCaster::rayFromToTriangle ( rayFrom, rayTo, dir,
							 f.m_n[0]->m_x,
							 f.m_n[1]->m_x,
							 f.m_n[2]->m_x,
							 mint );

			if ( t > 0 )
			{
				++cnt;

				if ( !bcountonly )
				{
					feature = SoftBody::eFeature::Face;
					index = i;
					mint = t;
				}
			}
		}
	}

	else
	{
		/* Use dbvt   */
		RayFromToCaster collider ( rayFrom, rayTo, mint );

		Dbvt::rayTest ( m_fdbvt.m_root, rayFrom, rayTo, collider );

		if ( collider.m_face )
		{
			mint = collider.m_mint;
			feature = SoftBody::eFeature::Face;
			index = ( i32 ) ( collider.m_face - &m_faces[0] );
			cnt = 1;
		}
	}

	for ( i32 i = 0; i < m_tetras.size(); i++ )
	{
		const SoftBody::Tetra& tet = m_tetras[i];
		i32 tetfaces[4][3] = {{0, 1, 2}, {0, 1, 3}, {1, 2, 3}, {0, 2, 3}};

		for ( i32 f = 0; f < 4; f++ )
		{
			i32 index0 = tetfaces[f][0];
			i32 index1 = tetfaces[f][1];
			i32 index2 = tetfaces[f][2];
			Vec3 v0 = tet.m_n[index0]->m_x;
			Vec3 v1 = tet.m_n[index1]->m_x;
			Vec3 v2 = tet.m_n[index2]->m_x;

			const Scalar t = RayFromToCaster::rayFromToTriangle ( rayFrom, rayTo, dir,
							 v0, v1, v2,
							 mint );

			if ( t > 0 )
			{
				++cnt;

				if ( !bcountonly )
				{
					feature = SoftBody::eFeature::Tetra;
					index = i;
					mint = t;
				}
			}
		}
	}

	return ( cnt );
}

i32 SoftBody::rayFaceTest ( const Vec3& rayFrom, const Vec3& rayTo,
		Scalar& mint, i32& index ) const
{
	i32 cnt = 0;
	{
		/* Use dbvt   */
		RayFromToCaster collider ( rayFrom, rayTo, mint );

		Dbvt::rayTest ( m_fdbvt.m_root, rayFrom, rayTo, collider );

		if ( collider.m_face )
		{
			mint = collider.m_mint;
			index = ( i32 ) ( collider.m_face - &m_faces[0] );
			cnt = 1;
		}
	}

	return ( cnt );
}

//
static inline DbvntNode* copyToDbvnt ( const DbvtNode* n )
{
	if ( n == 0 )
		return 0;

	DbvntNode* root = new DbvntNode ( n );

	if ( n->isinternal() )
	{
		DbvntNode* c0 = copyToDbvnt ( n->childs[0] );
		root->childs[0] = c0;
		DbvntNode* c1 = copyToDbvnt ( n->childs[1] );
		root->childs[1] = c1;
	}

	return root;
}

static inline void calculateNormalCone ( DbvntNode* root )
{
	if ( !root )
		return;

	if ( root->isleaf() )
	{
		const SoftBody::Face* face = ( SoftBody::Face* ) root->data;
		root->normal = face->m_normal;
		root->angle = 0;
	}

	else
	{
		Vec3 n0 ( 0, 0, 0 ), n1 ( 0, 0, 0 );
		Scalar a0 = 0, a1 = 0;

		if ( root->childs[0] )
		{
			calculateNormalCone ( root->childs[0] );
			n0 = root->childs[0]->normal;
			a0 = root->childs[0]->angle;
		}

		if ( root->childs[1] )
		{
			calculateNormalCone ( root->childs[1] );
			n1 = root->childs[1]->normal;
			a1 = root->childs[1]->angle;
		}

		root->normal = ( n0 + n1 ).safeNormalize();

		root->angle = d3Max ( a0, a1 ) + Angle ( n0, n1 ) * 0.5;
	}
}

void SoftBody::initializeFaceTree()
{
	DRX3D_PROFILE ( "SoftBody::initializeFaceTree" );
	m_fdbvt.clear();
	// create leaf nodes;
	AlignedObjectArray<DbvtNode*> leafNodes;
	leafNodes.resize ( m_faces.size() );

	for ( i32 i = 0; i < m_faces.size(); ++i )
	{
		Face& f = m_faces[i];
		ATTRIBUTE_ALIGNED16 ( DbvtVolume )
		vol = VolumeOf ( f, 0 );
		DbvtNode* node = new ( AlignedAlloc ( sizeof ( DbvtNode ), 16 ) ) DbvtNode();
		node->parent = NULL;
		node->data = &f;
		node->childs[1] = 0;
		node->volume = vol;
		leafNodes[i] = node;
		f.m_leaf = node;
	}

	AlignedObjectArray<AlignedObjectArray<i32> > adj;

	adj.resize ( m_faces.size() );
	// construct the adjacency list for triangles

	for ( i32 i = 0; i < adj.size(); ++i )
	{
		for ( i32 j = i + 1; j < adj.size(); ++j )
		{
			i32 dup = 0;

			for ( i32 k = 0; k < 3; ++k )
			{
				for ( i32 l = 0; l < 3; ++l )
				{
					if ( m_faces[i].m_n[k] == m_faces[j].m_n[l] )
					{
						++dup;
						break;
					}
				}

				if ( dup == 2 )
				{
					adj[i].push_back ( j );
					adj[j].push_back ( i );
				}
			}
		}
	}

	m_fdbvt.m_root = buildTreeBottomUp ( leafNodes, adj );

	if ( m_fdbvnt )
		delete m_fdbvnt;

	m_fdbvnt = copyToDbvnt ( m_fdbvt.m_root );

	updateFaceTree ( false, false );

	rebuildNodeTree();
}

//
void SoftBody::rebuildNodeTree()
{
	m_ndbvt.clear();
	AlignedObjectArray<DbvtNode*> leafNodes;
	leafNodes.resize ( m_nodes.size() );

	for ( i32 i = 0; i < m_nodes.size(); ++i )
	{
		Node& n = m_nodes[i];
		ATTRIBUTE_ALIGNED16 ( DbvtVolume )
		vol = DbvtVolume::FromCR ( n.m_x, 0 );
		DbvtNode* node = new ( AlignedAlloc ( sizeof ( DbvtNode ), 16 ) ) DbvtNode();
		node->parent = NULL;
		node->data = &n;
		node->childs[1] = 0;
		node->volume = vol;
		leafNodes[i] = node;
		n.m_leaf = node;
	}

	AlignedObjectArray<AlignedObjectArray<i32> > adj;

	adj.resize ( m_nodes.size() );
	AlignedObjectArray<i32> old_id;
	old_id.resize ( m_nodes.size() );

	for ( i32 i = 0; i < m_nodes.size(); ++i )
		old_id[i] = m_nodes[i].index;

	for ( i32 i = 0; i < m_nodes.size(); ++i )
		m_nodes[i].index = i;

	for ( i32 i = 0; i < m_links.size(); ++i )
	{
		Link& l = m_links[i];
		adj[l.m_n[0]->index].push_back ( l.m_n[1]->index );
		adj[l.m_n[1]->index].push_back ( l.m_n[0]->index );
	}

	m_ndbvt.m_root = buildTreeBottomUp ( leafNodes, adj );

	for ( i32 i = 0; i < m_nodes.size(); ++i )
		m_nodes[i].index = old_id[i];
}

//
Vec3 SoftBody::evaluateCom() const
{
	Vec3 com ( 0, 0, 0 );

	if ( m_pose.m_bframe )
	{
		for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			com += m_nodes[i].m_x * m_pose.m_wgh[i];
		}
	}

	return ( com );
}

bool SoftBody::checkContact ( const CollisionObject2Wrapper* colObjWrap,
		const Vec3& x,
		Scalar margin,
		SoftBody::sCti& cti ) const
{
	Vec3 nrm;
	const CollisionShape* shp = colObjWrap->getCollisionShape();
	//    const RigidBody *tmpRigid = RigidBody::upcast(colObjWrap->getCollisionObject());
	//const Transform2 &wtr = tmpRigid ? tmpRigid->getWorldTransform() : colObjWrap->getWorldTransform();
	const Transform2& wtr = colObjWrap->getWorldTransform();
	//todo: check which transform is needed here

	Scalar dst =
		m_worldInfo->m_sparsesdf.Evaluate (
			wtr.invXform ( x ),
			shp,
			nrm,
			margin );

	if ( dst < 0 )
	{
		cti.m_colObj = colObjWrap->getCollisionObject();
		cti.m_normal = wtr.getBasis() * nrm;
		cti.m_offset = -Dot ( cti.m_normal, x - cti.m_normal * dst );
		return ( true );
	}

	return ( false );
}

//
bool SoftBody::checkDeformableContact ( const CollisionObject2Wrapper* colObjWrap,
		const Vec3& x,
		Scalar margin,
		SoftBody::sCti& cti, bool predict ) const
{
	Vec3 nrm;
	const CollisionShape* shp = colObjWrap->getCollisionShape();
	const CollisionObject2* tmpCollisionObj = colObjWrap->getCollisionObject();
	// use the position x_{n+1}^* = x_n + dt * v_{n+1}^* where v_{n+1}^* = v_n + dtg for collision detect
	// but resolve contact at x_n
	Transform2 wtr = ( predict ) ? ( colObjWrap->m_preTransform2 != NULL ? tmpCollisionObj->getInterpolationWorldTransform() * ( *colObjWrap->m_preTransform2 ) : tmpCollisionObj->getInterpolationWorldTransform() )
							 : colObjWrap->getWorldTransform();
	Scalar dst =
		m_worldInfo->m_sparsesdf.Evaluate (
			wtr.invXform ( x ),
			shp,
			nrm,
			margin );

	if ( !predict )
	{
		cti.m_colObj = colObjWrap->getCollisionObject();
		cti.m_normal = wtr.getBasis() * nrm;
		cti.m_offset = dst;
	}

	if ( dst < 0 )
		return true;

	return ( false );
}

//
// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
static void getBarycentric ( const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c, Vec3& bary )
{
	Vec3 v0 = b - a, v1 = c - a, v2 = p - a;
	Scalar d00 = v0.dot ( v0 );
	Scalar d01 = v0.dot ( v1 );
	Scalar d11 = v1.dot ( v1 );
	Scalar d20 = v2.dot ( v0 );
	Scalar d21 = v2.dot ( v1 );
	Scalar denom = d00 * d11 - d01 * d01;
	// In the case of a degenerate triangle, pick a vertex.

	if ( Fabs ( denom ) < SIMD_EPSILON )
	{
		bary.setY ( Scalar ( 0.0 ) );
		bary.setZ ( Scalar ( 0.0 ) );
	}

	else
	{
		bary.setY ( ( d11 * d20 - d01 * d21 ) / denom );
		bary.setZ ( ( d00 * d21 - d01 * d20 ) / denom );
	}

	bary.setX ( Scalar ( 1 ) - bary.getY() - bary.getZ() );
}

//
bool SoftBody::checkDeformableFaceContact ( const CollisionObject2Wrapper* colObjWrap,
		Face& f,
		Vec3& contact_point,
		Vec3& bary,
		Scalar margin,
		SoftBody::sCti& cti, bool predict ) const
{
	Vec3 nrm;
	const CollisionShape* shp = colObjWrap->getCollisionShape();
	const CollisionObject2* tmpCollisionObj = colObjWrap->getCollisionObject();
	// use the position x_{n+1}^* = x_n + dt * v_{n+1}^* where v_{n+1}^* = v_n + dtg for collision detect
	// but resolve contact at x_n
	Transform2 wtr = ( predict ) ? ( colObjWrap->m_preTransform2 != NULL ? tmpCollisionObj->getInterpolationWorldTransform() * ( *colObjWrap->m_preTransform2 ) : tmpCollisionObj->getInterpolationWorldTransform() )
							 : colObjWrap->getWorldTransform();
	Scalar dst;
	GjkEpaSolver2::sResults results;

	//  #define USE_QUADRATURE 1

	// use collision quadrature point
#ifdef USE_QUADRATURE
	{
		dst = SIMD_INFINITY;
		Vec3 local_nrm;

		for ( i32 q = 0; q < m_quads.size(); ++q )
		{
			Vec3 p;

			if ( predict )
				p = BaryEval ( f.m_n[0]->m_q, f.m_n[1]->m_q, f.m_n[2]->m_q, m_quads[q] );
			else
				p = BaryEval ( f.m_n[0]->m_x, f.m_n[1]->m_x, f.m_n[2]->m_x, m_quads[q] );

			Scalar local_dst = m_worldInfo->m_sparsesdf.Evaluate (
								   wtr.invXform ( p ),
								   shp,
								   local_nrm,
								   margin );

			if ( local_dst < dst )
			{
				if ( local_dst < 0 && predict )
					return true;

				dst = local_dst;

				contact_point = p;

				bary = m_quads[q];

				nrm = local_nrm;
			}

			if ( !predict )
			{
				cti.m_colObj = colObjWrap->getCollisionObject();
				cti.m_normal = wtr.getBasis() * nrm;
				cti.m_offset = dst;
			}
		}

		return ( dst < 0 );
	}

#endif

	// collision detection using x*
	Transform2 triangle_transform;
	triangle_transform.setIdentity();
	triangle_transform.setOrigin ( f.m_n[0]->m_q );
	TriangleShape triangle ( Vec3 ( 0, 0, 0 ), f.m_n[1]->m_q - f.m_n[0]->m_q, f.m_n[2]->m_q - f.m_n[0]->m_q );
	Vec3 guess ( 0, 0, 0 );
	const ConvexShape* csh = static_cast<const ConvexShape*> ( shp );
	GjkEpaSolver2::SignedDistance ( &triangle, triangle_transform, csh, wtr, guess, results );
	dst = results.distance - 2.0 * csh->getMargin() - margin;  // margin padding so that the distance = the actual distance between face and rigid - margin of rigid - margin of deformable

	if ( dst >= 0 )
		return false;

	// Use consistent barycenter to recalculate distance.
	if ( this->m_cacheBarycenter )
	{
		if ( f.m_pcontact[3] != 0 )
		{
			for ( i32 i = 0; i < 3; ++i )
				bary[i] = f.m_pcontact[i];

			contact_point = BaryEval ( f.m_n[0]->m_x, f.m_n[1]->m_x, f.m_n[2]->m_x, bary );

			const ConvexShape* csh = static_cast<const ConvexShape*> ( shp );

			GjkEpaSolver2::SignedDistance ( contact_point, margin, csh, wtr, results );

			cti.m_colObj = colObjWrap->getCollisionObject();

			dst = results.distance;

			cti.m_normal = results.normal;

			cti.m_offset = dst;

			//point-convex CD
			wtr = colObjWrap->getWorldTransform();

			TriangleShape triangle2 ( Vec3 ( 0, 0, 0 ), f.m_n[1]->m_x - f.m_n[0]->m_x, f.m_n[2]->m_x - f.m_n[0]->m_x );

			triangle_transform.setOrigin ( f.m_n[0]->m_x );

			GjkEpaSolver2::SignedDistance ( &triangle2, triangle_transform, csh, wtr, guess, results );

			dst = results.distance - csh->getMargin() - margin;

			return true;
		}
	}

	// Use triangle-convex CD.
	wtr = colObjWrap->getWorldTransform();

	TriangleShape triangle2 ( Vec3 ( 0, 0, 0 ), f.m_n[1]->m_x - f.m_n[0]->m_x, f.m_n[2]->m_x - f.m_n[0]->m_x );

	triangle_transform.setOrigin ( f.m_n[0]->m_x );

	GjkEpaSolver2::SignedDistance ( &triangle2, triangle_transform, csh, wtr, guess, results );

	contact_point = results.witnesses[0];

	getBarycentric ( contact_point, f.m_n[0]->m_x, f.m_n[1]->m_x, f.m_n[2]->m_x, bary );

	for ( i32 i = 0; i < 3; ++i )
		f.m_pcontact[i] = bary[i];

	dst = results.distance - csh->getMargin() - margin;

	cti.m_colObj = colObjWrap->getCollisionObject();

	cti.m_normal = results.normal;

	cti.m_offset = dst;

	return true;
}

void SoftBody::updateNormals()
{
	const Vec3 zv ( 0, 0, 0 );
	i32 i, ni;

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		m_nodes[i].m_n = zv;
	}

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		SoftBody::Face& f = m_faces[i];
		const Vec3 n = Cross ( f.m_n[1]->m_x - f.m_n[0]->m_x,
					   f.m_n[2]->m_x - f.m_n[0]->m_x );
		f.m_normal = n;
		f.m_normal.safeNormalize();
		f.m_n[0]->m_n += n;
		f.m_n[1]->m_n += n;
		f.m_n[2]->m_n += n;
	}

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		Scalar len = m_nodes[i].m_n.length();

		if ( len > SIMD_EPSILON )
			m_nodes[i].m_n /= len;
	}
}

//
void SoftBody::updateBounds()
{
	/*if( m_acceleratedSoftBody )
	{
	    // If we have an accelerated softbody we need to obtain the bounds correctly
	    // For now (slightly hackily) just have a very large AABB
	    // TODO: Write get bounds kernel
	    // If that is updating in place, atomic collisions might be low (when the cloth isn't perfectly aligned to an axis) and we could
	    // probably do a test and exchange reasonably efficiently.

	    m_bounds[0] = Vec3(-1000, -1000, -1000);
	    m_bounds[1] = Vec3(1000, 1000, 1000);

	} else {*/
	//    if (m_ndbvt.m_root)
	//    {
	//        const Vec3& mins = m_ndbvt.m_root->volume.Mins();
	//        const Vec3& maxs = m_ndbvt.m_root->volume.Maxs();
	//        const Scalar csm = getCollisionShape()->getMargin();
	//        const Vec3 mrg = Vec3(csm,
	//                                        csm,
	//                                        csm) *
	//                              1;  // ??? to investigate...
	//        m_bounds[0] = mins - mrg;
	//        m_bounds[1] = maxs + mrg;
	//        if (0 != getBroadphaseHandle())
	//        {
	//            m_worldInfo->m_broadphase->setAabb(getBroadphaseHandle(),
	//                                               m_bounds[0],
	//                                               m_bounds[1],
	//                                               m_worldInfo->m_dispatcher);
	//        }
	//    }
	//    else
	//    {
	//        m_bounds[0] =
	//            m_bounds[1] = Vec3(0, 0, 0);
	//    }
	if ( m_nodes.size() )
	{
		Vec3 mins = m_nodes[0].m_x;
		Vec3 maxs = m_nodes[0].m_x;

		for ( i32 i = 1; i < m_nodes.size(); ++i )
		{
			for ( i32 d = 0; d < 3; ++d )
			{
				if ( m_nodes[i].m_x[d] > maxs[d] )
					maxs[d] = m_nodes[i].m_x[d];

				if ( m_nodes[i].m_x[d] < mins[d] )
					mins[d] = m_nodes[i].m_x[d];
			}
		}

		const Scalar csm = getCollisionShape()->getMargin();

		const Vec3 mrg = Vec3 ( csm,
						 csm,
						 csm );

		m_bounds[0] = mins - mrg;

		m_bounds[1] = maxs + mrg;

		if ( 0 != getBroadphaseHandle() )
		{
			m_worldInfo->m_broadphase->setAabb ( getBroadphaseHandle(),
					m_bounds[0],
					m_bounds[1],
					m_worldInfo->m_dispatcher );
		}
	}

	else
	{
		m_bounds[0] =
			m_bounds[1] = Vec3 ( 0, 0, 0 );
	}
}

//
void SoftBody::updatePose()
{
	if ( m_pose.m_bframe )
	{
		SoftBody::Pose& pose = m_pose;
		const Vec3 com = evaluateCom();
		/* Com          */
		pose.m_com = com;
		/* Rotation     */
		Matrix3x3 Apq;
		const Scalar eps = SIMD_EPSILON;
		Apq[0] = Apq[1] = Apq[2] = Vec3 ( 0, 0, 0 );
		Apq[0].setX ( eps );
		Apq[1].setY ( eps * 2 );
		Apq[2].setZ ( eps * 3 );

		for ( i32 i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			const Vec3 a = pose.m_wgh[i] * ( m_nodes[i].m_x - com );
			const Vec3& b = pose.m_pos[i];
			Apq[0] += a.x() * b;
			Apq[1] += a.y() * b;
			Apq[2] += a.z() * b;
		}

		Matrix3x3 r, s;

		PolarDecompose ( Apq, r, s );
		pose.m_rot = r;
		pose.m_scl = pose.m_aqq * r.transpose() * Apq;

		if ( m_cfg.maxvolume > 1 )
		{
			const Scalar idet = Clamp<Scalar> ( 1 / pose.m_scl.determinant(),
								1, m_cfg.maxvolume );
			pose.m_scl = Mul ( pose.m_scl, idet );
		}
	}
}

//
void SoftBody::updateArea ( bool averageArea )
{
	i32 i, ni;

	/* Face area        */

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		Face& f = m_faces[i];
		f.m_ra = AreaOf ( f.m_n[0]->m_x, f.m_n[1]->m_x, f.m_n[2]->m_x );
	}

	/* Node area        */

	if ( averageArea )
	{
		AlignedObjectArray<i32> counts;
		counts.resize ( m_nodes.size(), 0 );

		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			m_nodes[i].m_area = 0;
		}

		for ( i = 0, ni = m_faces.size(); i < ni; ++i )
		{
			SoftBody::Face& f = m_faces[i];

			for ( i32 j = 0; j < 3; ++j )
			{
				i32k index = ( i32 ) ( f.m_n[j] - &m_nodes[0] );
				counts[index]++;
				f.m_n[j]->m_area += Fabs ( f.m_ra );
			}
		}

		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			if ( counts[i] > 0 )
				m_nodes[i].m_area /= ( Scalar ) counts[i];
			else
				m_nodes[i].m_area = 0;
		}
	}

	else
	{
		// initialize node area as zero
		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			m_nodes[i].m_area = 0;
		}

		for ( i = 0, ni = m_faces.size(); i < ni; ++i )
		{
			SoftBody::Face& f = m_faces[i];

			for ( i32 j = 0; j < 3; ++j )
			{
				f.m_n[j]->m_area += f.m_ra;
			}
		}

		for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
		{
			m_nodes[i].m_area *= 0.3333333f;
		}
	}
}

void SoftBody::updateLinkConstants()
{
	i32 i, ni;

	/* Links        */

	for ( i = 0, ni = m_links.size(); i < ni; ++i )
	{
		Link& l = m_links[i];
		Material& m = *l.m_material;
		l.m_c0 = ( l.m_n[0]->m_im + l.m_n[1]->m_im ) / m.m_kLST;
	}
}

void SoftBody::updateConstants()
{
	resetLinkRestLengths();
	updateLinkConstants();
	updateArea();
}

//
void SoftBody::initializeClusters()
{
	i32 i;

	for ( i = 0; i < m_clusters.size(); ++i )
	{
		Cluster& c = *m_clusters[i];
		c.m_imass = 0;
		c.m_masses.resize ( c.m_nodes.size() );

		for ( i32 j = 0; j < c.m_nodes.size(); ++j )
		{
			if ( c.m_nodes[j]->m_im == 0 )
			{
				c.m_containsAnchor = true;
				c.m_masses[j] = DRX3D_LARGE_FLOAT;
			}

			else
			{
				c.m_masses[j] = Scalar ( 1. ) / c.m_nodes[j]->m_im;
			}

			c.m_imass += c.m_masses[j];
		}

		c.m_imass = Scalar ( 1. ) / c.m_imass;

		c.m_com = SoftBody::clusterCom ( &c );
		c.m_lv = Vec3 ( 0, 0, 0 );
		c.m_av = Vec3 ( 0, 0, 0 );
		c.m_leaf = 0;
		/* Inertia  */
		Matrix3x3& ii = c.m_locii;
		ii[0] = ii[1] = ii[2] = Vec3 ( 0, 0, 0 );
		{
			i32 i, ni;

			for ( i = 0, ni = c.m_nodes.size(); i < ni; ++i )
			{
				const Vec3 k = c.m_nodes[i]->m_x - c.m_com;
				const Vec3 q = k * k;
				const Scalar m = c.m_masses[i];
				ii[0][0] += m * ( q[1] + q[2] );
				ii[1][1] += m * ( q[0] + q[2] );
				ii[2][2] += m * ( q[0] + q[1] );
				ii[0][1] -= m * k[0] * k[1];
				ii[0][2] -= m * k[0] * k[2];
				ii[1][2] -= m * k[1] * k[2];
			}
		}

		ii[1][0] = ii[0][1];
		ii[2][0] = ii[0][2];
		ii[2][1] = ii[1][2];

		ii = ii.inverse();

		/* Frame    */
		c.m_framexform.setIdentity();
		c.m_framexform.setOrigin ( c.m_com );
		c.m_framerefs.resize ( c.m_nodes.size() );
		{
			i32 i;

			for ( i = 0; i < c.m_framerefs.size(); ++i )
			{
				c.m_framerefs[i] = c.m_nodes[i]->m_x - c.m_com;
			}
		}
	}
}

//
void SoftBody::updateClusters()
{
	DRX3D_PROFILE ( "UpdateClusters" );
	i32 i;

	for ( i = 0; i < m_clusters.size(); ++i )
	{
		SoftBody::Cluster& c = *m_clusters[i];
		i32k n = c.m_nodes.size();
		//const Scalar          invn=1/(Scalar)n;

		if ( n )
		{
			/* Frame                */
			const Scalar eps = Scalar ( 0.0001 );
			Matrix3x3 m, r, s;
			m[0] = m[1] = m[2] = Vec3 ( 0, 0, 0 );
			m[0][0] = eps * 1;
			m[1][1] = eps * 2;
			m[2][2] = eps * 3;
			c.m_com = clusterCom ( &c );

			for ( i32 i = 0; i < c.m_nodes.size(); ++i )
			{
				const Vec3 a = c.m_nodes[i]->m_x - c.m_com;
				const Vec3& b = c.m_framerefs[i];
				m[0] += a[0] * b;
				m[1] += a[1] * b;
				m[2] += a[2] * b;
			}

			PolarDecompose ( m, r, s );

			c.m_framexform.setOrigin ( c.m_com );
			c.m_framexform.setBasis ( r );
			/* Inertia          */
#if 1 /* Constant   */
			c.m_invwi = c.m_framexform.getBasis() * c.m_locii * c.m_framexform.getBasis().transpose();
#else
#if 0 /* Sphere */
			const Scalar    rk = ( 2 * c.m_extents.length2() ) / ( 5 * c.m_imass );
			const Vec3  inertia ( rk, rk, rk );
			const Vec3  iin ( Fabs ( inertia[0] ) > SIMD_EPSILON ? 1 / inertia[0] : 0,
							  Fabs ( inertia[1] ) > SIMD_EPSILON ? 1 / inertia[1] : 0,
							  Fabs ( inertia[2] ) > SIMD_EPSILON ? 1 / inertia[2] : 0 );

			c.m_invwi = c.m_xform.getBasis().scaled ( iin ) * c.m_xform.getBasis().transpose();
#else /* Actual */
			c.m_invwi[0] = c.m_invwi[1] = c.m_invwi[2] = Vec3 ( 0, 0, 0 );

			for ( i32 i = 0; i < n; ++i )
			{
				const Vec3 k = c.m_nodes[i]->m_x - c.m_com;
				const Vec3 q = k * k;
				const Scalar m = 1 / c.m_nodes[i]->m_im;
				c.m_invwi[0][0] += m * ( q[1] + q[2] );
				c.m_invwi[1][1] += m * ( q[0] + q[2] );
				c.m_invwi[2][2] += m * ( q[0] + q[1] );
				c.m_invwi[0][1] -= m * k[0] * k[1];
				c.m_invwi[0][2] -= m * k[0] * k[2];
				c.m_invwi[1][2] -= m * k[1] * k[2];
			}

			c.m_invwi[1][0] = c.m_invwi[0][1];

			c.m_invwi[2][0] = c.m_invwi[0][2];
			c.m_invwi[2][1] = c.m_invwi[1][2];
			c.m_invwi = c.m_invwi.inverse();
#endif
#endif
			/* Velocities           */
			c.m_lv = Vec3 ( 0, 0, 0 );
			c.m_av = Vec3 ( 0, 0, 0 );
			{
				i32 i;

				for ( i = 0; i < n; ++i )
				{
					const Vec3 v = c.m_nodes[i]->m_v * c.m_masses[i];
					c.m_lv += v;
					c.m_av += Cross ( c.m_nodes[i]->m_x - c.m_com, v );
				}
			}

			c.m_lv = c.m_imass * c.m_lv * ( 1 - c.m_ldamping );
			c.m_av = c.m_invwi * c.m_av * ( 1 - c.m_adamping );
			c.m_vimpulses[0] =
				c.m_vimpulses[1] = Vec3 ( 0, 0, 0 );
			c.m_dimpulses[0] =
				c.m_dimpulses[1] = Vec3 ( 0, 0, 0 );
			c.m_nvimpulses = 0;
			c.m_ndimpulses = 0;
			/* Matching             */

			if ( c.m_matching > 0 )
			{
				for ( i32 j = 0; j < c.m_nodes.size(); ++j )
				{
					Node& n = *c.m_nodes[j];
					const Vec3 x = c.m_framexform * c.m_framerefs[j];
					n.m_x = Lerp ( n.m_x, x, c.m_matching );
				}
			}

			/* Dbvt                 */

			if ( c.m_collide )
			{
				Vec3 mi = c.m_nodes[0]->m_x;
				Vec3 mx = mi;

				for ( i32 j = 1; j < n; ++j )
				{
					mi.setMin ( c.m_nodes[j]->m_x );
					mx.setMax ( c.m_nodes[j]->m_x );
				}

				ATTRIBUTE_ALIGNED16 ( DbvtVolume )

				bounds = DbvtVolume::FromMM ( mi, mx );

				if ( c.m_leaf )
					m_cdbvt.update ( c.m_leaf, bounds, c.m_lv * m_sst.sdt * 3, m_sst.radmrg );
				else
					c.m_leaf = m_cdbvt.insert ( bounds, &c );
			}
		}
	}
}

//
void SoftBody::cleanupClusters()
{
	for ( i32 i = 0; i < m_joints.size(); ++i )
	{
		m_joints[i]->Terminate ( m_sst.sdt );

		if ( m_joints[i]->m_delete )
		{
			AlignedFree ( m_joints[i] );
			m_joints.remove ( m_joints[i--] );
		}
	}
}

//
void SoftBody::prepareClusters ( i32 iterations )
{
	for ( i32 i = 0; i < m_joints.size(); ++i )
	{
		m_joints[i]->Prepare ( m_sst.sdt, iterations );
	}
}

//
void SoftBody::solveClusters ( Scalar sor )
{
	for ( i32 i = 0, ni = m_joints.size(); i < ni; ++i )
	{
		m_joints[i]->Solve ( m_sst.sdt, sor );
	}
}

//
void SoftBody::applyClusters ( bool drift )
{
	DRX3D_PROFILE ( "ApplyClusters" );
	//  const Scalar                    f0=m_sst.sdt;
	//const Scalar                  f1=f0/2;
	AlignedObjectArray<Vec3> deltas;
	AlignedObjectArray<Scalar> weights;
	deltas.resize ( m_nodes.size(), Vec3 ( 0, 0, 0 ) );
	weights.resize ( m_nodes.size(), 0 );
	i32 i;

	if ( drift )
	{
		for ( i = 0; i < m_clusters.size(); ++i )
		{
			Cluster& c = *m_clusters[i];

			if ( c.m_ndimpulses )
			{
				c.m_dimpulses[0] /= ( Scalar ) c.m_ndimpulses;
				c.m_dimpulses[1] /= ( Scalar ) c.m_ndimpulses;
			}
		}
	}

	for ( i = 0; i < m_clusters.size(); ++i )
	{
		Cluster& c = *m_clusters[i];

		if ( 0 < ( drift ? c.m_ndimpulses : c.m_nvimpulses ) )
		{
			const Vec3 v = ( drift ? c.m_dimpulses[0] : c.m_vimpulses[0] ) * m_sst.sdt;
			const Vec3 w = ( drift ? c.m_dimpulses[1] : c.m_vimpulses[1] ) * m_sst.sdt;

			for ( i32 j = 0; j < c.m_nodes.size(); ++j )
			{
				i32k idx = i32 ( c.m_nodes[j] - &m_nodes[0] );
				const Vec3& x = c.m_nodes[j]->m_x;
				const Scalar q = c.m_masses[j];
				deltas[idx] += ( v + Cross ( w, x - c.m_com ) ) * q;
				weights[idx] += q;
			}
		}
	}

	for ( i = 0; i < deltas.size(); ++i )
	{
		if ( weights[i] > 0 )
		{
			m_nodes[i].m_x += deltas[i] / weights[i];
		}
	}
}

//
void SoftBody::dampClusters()
{
	i32 i;

	for ( i = 0; i < m_clusters.size(); ++i )
	{
		Cluster& c = *m_clusters[i];

		if ( c.m_ndamping > 0 )
		{
			for ( i32 j = 0; j < c.m_nodes.size(); ++j )
			{
				Node& n = *c.m_nodes[j];

				if ( n.m_im > 0 )
				{
					const Vec3 vx = c.m_lv + Cross ( c.m_av, c.m_nodes[j]->m_q - c.m_com );

					if ( vx.length2() <= n.m_v.length2() )
					{
						n.m_v += c.m_ndamping * ( vx - n.m_v );
					}
				}
			}
		}
	}
}

void SoftBody::setSpringStiffness ( Scalar k )
{
	for ( i32 i = 0; i < m_links.size(); ++i )
	{
		m_links[i].Feature::m_material->m_kLST = k;
	}

	m_repulsionStiffness = k;
}

void SoftBody::setGravityFactor ( Scalar gravFactor )
{
	m_gravityFactor = gravFactor;
}

void SoftBody::setCacheBarycenter ( bool cacheBarycenter )
{
	m_cacheBarycenter = cacheBarycenter;
}

void SoftBody::initializeDmInverse()
{
	Scalar unit_simplex_measure = 1. / 6.;

	for ( i32 i = 0; i < m_tetras.size(); ++i )
	{
		Tetra& t = m_tetras[i];
		Vec3 c1 = t.m_n[1]->m_x - t.m_n[0]->m_x;
		Vec3 c2 = t.m_n[2]->m_x - t.m_n[0]->m_x;
		Vec3 c3 = t.m_n[3]->m_x - t.m_n[0]->m_x;
		Matrix3x3 Dm ( c1.getX(), c2.getX(), c3.getX(),
					   c1.getY(), c2.getY(), c3.getY(),
					   c1.getZ(), c2.getZ(), c3.getZ() );
		t.m_element_measure = Dm.determinant() * unit_simplex_measure;
		t.m_Dm_inverse = Dm.inverse();

		// calculate the first three columns of P^{-1}
		Vec3 a = t.m_n[0]->m_x;
		Vec3 b = t.m_n[1]->m_x;
		Vec3 c = t.m_n[2]->m_x;
		Vec3 d = t.m_n[3]->m_x;

		Scalar det = 1 / ( a[0] * b[1] * c[2] - a[0] * b[1] * d[2] - a[0] * b[2] * c[1] + a[0] * b[2] * d[1] + a[0] * c[1] * d[2] - a[0] * c[2] * d[1] + a[1] * ( -b[0] * c[2] + b[0] * d[2] + b[2] * c[0] - b[2] * d[0] - c[0] * d[2] + c[2] * d[0] ) + a[2] * ( b[0] * c[1] - b[0] * d[1] + b[1] * ( d[0] - c[0] ) + c[0] * d[1] - c[1] * d[0] ) - b[0] * c[1] * d[2] + b[0] * c[2] * d[1] + b[1] * c[0] * d[2] - b[1] * c[2] * d[0] - b[2] * c[0] * d[1] + b[2] * c[1] * d[0] );

		Scalar P11 = -b[2] * c[1] + d[2] * c[1] + b[1] * c[2] + b[2] * d[1] - c[2] * d[1] - b[1] * d[2];
		Scalar P12 = b[2] * c[0] - d[2] * c[0] - b[0] * c[2] - b[2] * d[0] + c[2] * d[0] + b[0] * d[2];
		Scalar P13 = -b[1] * c[0] + d[1] * c[0] + b[0] * c[1] + b[1] * d[0] - c[1] * d[0] - b[0] * d[1];
		Scalar P21 = a[2] * c[1] - d[2] * c[1] - a[1] * c[2] - a[2] * d[1] + c[2] * d[1] + a[1] * d[2];
		Scalar P22 = -a[2] * c[0] + d[2] * c[0] + a[0] * c[2] + a[2] * d[0] - c[2] * d[0] - a[0] * d[2];
		Scalar P23 = a[1] * c[0] - d[1] * c[0] - a[0] * c[1] - a[1] * d[0] + c[1] * d[0] + a[0] * d[1];
		Scalar P31 = -a[2] * b[1] + d[2] * b[1] + a[1] * b[2] + a[2] * d[1] - b[2] * d[1] - a[1] * d[2];
		Scalar P32 = a[2] * b[0] - d[2] * b[0] - a[0] * b[2] - a[2] * d[0] + b[2] * d[0] + a[0] * d[2];
		Scalar P33 = -a[1] * b[0] + d[1] * b[0] + a[0] * b[1] + a[1] * d[0] - b[1] * d[0] - a[0] * d[1];
		Scalar P41 = a[2] * b[1] - c[2] * b[1] - a[1] * b[2] - a[2] * c[1] + b[2] * c[1] + a[1] * c[2];
		Scalar P42 = -a[2] * b[0] + c[2] * b[0] + a[0] * b[2] + a[2] * c[0] - b[2] * c[0] - a[0] * c[2];
		Scalar P43 = a[1] * b[0] - c[1] * b[0] - a[0] * b[1] - a[1] * c[0] + b[1] * c[0] + a[0] * c[1];

		Vec4 p1 ( P11 * det, P21 * det, P31 * det, P41 * det );
		Vec4 p2 ( P12 * det, P22 * det, P32 * det, P42 * det );
		Vec4 p3 ( P13 * det, P23 * det, P33 * det, P43 * det );

		t.m_P_inv[0] = p1;
		t.m_P_inv[1] = p2;
		t.m_P_inv[2] = p3;
	}
}

static Scalar Dot4 ( const Vec4& a, const Vec4& b )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

void SoftBody::updateDeformation()
{
	Quat q;

	for ( i32 i = 0; i < m_tetras.size(); ++i )
	{
		SoftBody::Tetra& t = m_tetras[i];
		Vec3 c1 = t.m_n[1]->m_q - t.m_n[0]->m_q;
		Vec3 c2 = t.m_n[2]->m_q - t.m_n[0]->m_q;
		Vec3 c3 = t.m_n[3]->m_q - t.m_n[0]->m_q;
		Matrix3x3 Ds ( c1.getX(), c2.getX(), c3.getX(),
					   c1.getY(), c2.getY(), c3.getY(),
					   c1.getZ(), c2.getZ(), c3.getZ() );
		t.m_F = Ds * t.m_Dm_inverse;

		SoftBody::TetraScratch& s = m_tetraScratches[i];
		s.m_F = t.m_F;
		s.m_J = t.m_F.determinant();
		Matrix3x3 C = t.m_F.transpose() * t.m_F;
		s.m_trace = C[0].getX() + C[1].getY() + C[2].getZ();
		s.m_cofF = t.m_F.adjoint().transpose();

		Vec3 a = t.m_n[0]->m_q;
		Vec3 b = t.m_n[1]->m_q;
		Vec3 c = t.m_n[2]->m_q;
		Vec3 d = t.m_n[3]->m_q;
		Vec4 q1 ( a[0], b[0], c[0], d[0] );
		Vec4 q2 ( a[1], b[1], c[1], d[1] );
		Vec4 q3 ( a[2], b[2], c[2], d[2] );
		Matrix3x3 B ( Dot4 ( q1, t.m_P_inv[0] ), Dot4 ( q1, t.m_P_inv[1] ), Dot4 ( q1, t.m_P_inv[2] ),
					  Dot4 ( q2, t.m_P_inv[0] ), Dot4 ( q2, t.m_P_inv[1] ), Dot4 ( q2, t.m_P_inv[2] ),
					  Dot4 ( q3, t.m_P_inv[0] ), Dot4 ( q3, t.m_P_inv[1] ), Dot4 ( q3, t.m_P_inv[2] ) );
		q.setRotation ( Vec3 ( 0, 0, 1 ), 0 );
		B.extractRotation ( q, 0.01 );  // precision of the rotation is not very important for visual correctness.
		Matrix3x3 Q ( q );
		s.m_corotation = Q;
	}
}

void SoftBody::advanceDeformation()
{
	updateDeformation();

	for ( i32 i = 0; i < m_tetras.size(); ++i )
	{
		m_tetraScratchesTn[i] = m_tetraScratches[i];
	}
}

//
void SoftBody::Joint::Prepare ( Scalar dt, i32 )
{
	m_bodies[0].activate();
	m_bodies[1].activate();
}

//
void SoftBody::LJoint::Prepare ( Scalar dt, i32 iterations )
{
	static const Scalar maxdrift = 4;
	Joint::Prepare ( dt, iterations );
	m_rpos[0] = m_bodies[0].xform() * m_refs[0];
	m_rpos[1] = m_bodies[1].xform() * m_refs[1];
	m_drift = Clamp ( m_rpos[0] - m_rpos[1], maxdrift ) * m_erp / dt;
	m_rpos[0] -= m_bodies[0].xform().getOrigin();
	m_rpos[1] -= m_bodies[1].xform().getOrigin();
	m_massmatrix = ImpulseMatrix ( m_bodies[0].invMass(), m_bodies[0].invWorldInertia(), m_rpos[0],
				   m_bodies[1].invMass(), m_bodies[1].invWorldInertia(), m_rpos[1] );

	if ( m_split > 0 )
	{
		m_sdrift = m_massmatrix * ( m_drift * m_split );
		m_drift *= 1 - m_split;
	}

	m_drift /= ( Scalar ) iterations;
}

//
void SoftBody::LJoint::Solve ( Scalar dt, Scalar sor )
{
	const Vec3 va = m_bodies[0].velocity ( m_rpos[0] );
	const Vec3 vb = m_bodies[1].velocity ( m_rpos[1] );
	const Vec3 vr = va - vb;
	SoftBody::Impulse impulse;
	impulse.m_asVelocity = 1;
	impulse.m_velocity = m_massmatrix * ( m_drift + vr * m_cfm ) * sor;
	m_bodies[0].applyImpulse ( -impulse, m_rpos[0] );
	m_bodies[1].applyImpulse ( impulse, m_rpos[1] );
}

//
void SoftBody::LJoint::Terminate ( Scalar dt )
{
	if ( m_split > 0 )
	{
		m_bodies[0].applyDImpulse ( -m_sdrift, m_rpos[0] );
		m_bodies[1].applyDImpulse ( m_sdrift, m_rpos[1] );
	}
}

//
void SoftBody::AJoint::Prepare ( Scalar dt, i32 iterations )
{
	static const Scalar maxdrift = SIMD_PI / 16;
	m_icontrol->Prepare ( this );
	Joint::Prepare ( dt, iterations );
	m_axis[0] = m_bodies[0].xform().getBasis() * m_refs[0];
	m_axis[1] = m_bodies[1].xform().getBasis() * m_refs[1];
	m_drift = NormalizeAny ( Cross ( m_axis[1], m_axis[0] ) );
	m_drift *= d3Min ( maxdrift, Acos ( Clamp<Scalar> ( Dot ( m_axis[0], m_axis[1] ), -1, + 1 ) ) );
	m_drift *= m_erp / dt;
	m_massmatrix = AngularImpulseMatrix ( m_bodies[0].invWorldInertia(), m_bodies[1].invWorldInertia() );

	if ( m_split > 0 )
	{
		m_sdrift = m_massmatrix * ( m_drift * m_split );
		m_drift *= 1 - m_split;
	}

	m_drift /= ( Scalar ) iterations;
}

//
void SoftBody::AJoint::Solve ( Scalar dt, Scalar sor )
{
	const Vec3 va = m_bodies[0].angularVelocity();
	const Vec3 vb = m_bodies[1].angularVelocity();
	const Vec3 vr = va - vb;
	const Scalar sp = Dot ( vr, m_axis[0] );
	const Vec3 vc = vr - m_axis[0] * m_icontrol->Speed ( this, sp );
	SoftBody::Impulse impulse;
	impulse.m_asVelocity = 1;
	impulse.m_velocity = m_massmatrix * ( m_drift + vc * m_cfm ) * sor;
	m_bodies[0].applyAImpulse ( -impulse );
	m_bodies[1].applyAImpulse ( impulse );
}

//
void SoftBody::AJoint::Terminate ( Scalar dt )
{
	if ( m_split > 0 )
	{
		m_bodies[0].applyDAImpulse ( -m_sdrift );
		m_bodies[1].applyDAImpulse ( m_sdrift );
	}
}

//
void SoftBody::CJoint::Prepare ( Scalar dt, i32 iterations )
{
	Joint::Prepare ( dt, iterations );
	const bool dodrift = ( m_life == 0 );
	m_delete = ( ++m_life ) > m_maxlife;

	if ( dodrift )
	{
		m_drift = m_drift * m_erp / dt;

		if ( m_split > 0 )
		{
			m_sdrift = m_massmatrix * ( m_drift * m_split );
			m_drift *= 1 - m_split;
		}

		m_drift /= ( Scalar ) iterations;
	}

	else
	{
		m_drift = m_sdrift = Vec3 ( 0, 0, 0 );
	}
}

//
void SoftBody::CJoint::Solve ( Scalar dt, Scalar sor )
{
	const Vec3 va = m_bodies[0].velocity ( m_rpos[0] );
	const Vec3 vb = m_bodies[1].velocity ( m_rpos[1] );
	const Vec3 vrel = va - vb;
	const Scalar rvac = Dot ( vrel, m_normal );
	SoftBody::Impulse impulse;
	impulse.m_asVelocity = 1;
	impulse.m_velocity = m_drift;

	if ( rvac < 0 )
	{
		const Vec3 iv = m_normal * rvac;
		const Vec3 fv = vrel - iv;
		impulse.m_velocity += iv + fv * m_friction;
	}

	impulse.m_velocity = m_massmatrix * impulse.m_velocity * sor;

	if ( m_bodies[0].m_soft == m_bodies[1].m_soft )
	{
		if ( ( impulse.m_velocity.getX() == impulse.m_velocity.getX() ) && ( impulse.m_velocity.getY() == impulse.m_velocity.getY() ) &&
			 ( impulse.m_velocity.getZ() == impulse.m_velocity.getZ() ) )
		{
			if ( impulse.m_asVelocity )
			{
				if ( impulse.m_velocity.length() < m_bodies[0].m_soft->m_maxSelfCollisionImpulse )
				{
				}
				else
				{
					m_bodies[0].applyImpulse ( -impulse * m_bodies[0].m_soft->m_selfCollisionImpulseFactor, m_rpos[0] );
					m_bodies[1].applyImpulse ( impulse * m_bodies[0].m_soft->m_selfCollisionImpulseFactor, m_rpos[1] );
				}
			}
		}
	}

	else
	{
		m_bodies[0].applyImpulse ( -impulse, m_rpos[0] );
		m_bodies[1].applyImpulse ( impulse, m_rpos[1] );
	}
}

//
void SoftBody::CJoint::Terminate ( Scalar dt )
{
	if ( m_split > 0 )
	{
		m_bodies[0].applyDImpulse ( -m_sdrift, m_rpos[0] );
		m_bodies[1].applyDImpulse ( m_sdrift, m_rpos[1] );
	}
}

//
void SoftBody::applyForces()
{
	DRX3D_PROFILE ( "SoftBody applyForces" );
	//  const Scalar                    dt =            m_sst.sdt;
	const Scalar kLF = m_cfg.kLF;
	const Scalar kDG = m_cfg.kDG;
	const Scalar kPR = m_cfg.kPR;
	const Scalar kVC = m_cfg.kVC;
	const bool as_lift = kLF > 0;
	const bool as_drag = kDG > 0;
	const bool as_pressure = kPR != 0;
	const bool as_volume = kVC > 0;
	const bool as_aero = as_lift ||
			as_drag;
	//const bool                        as_vaero =      as_aero &&
	//                                              (m_cfg.aeromodel < SoftBody::eAeroModel::F_TwoSided);
	//const bool                        as_faero =      as_aero &&
	//                                              (m_cfg.aeromodel >= SoftBody::eAeroModel::F_TwoSided);
	const bool use_medium = as_aero;
	const bool use_volume = as_pressure ||
			as_volume;
	Scalar volume = 0;
	Scalar ivolumetp = 0;
	Scalar dvolumetv = 0;
	SoftBody::sMedium medium;

	if ( use_volume )
	{
		volume = getVolume();
		ivolumetp = 1 / Fabs ( volume ) * kPR;
		dvolumetv = ( m_pose.m_volume - volume ) * kVC;
	}

	/* Per vertex forces            */
	i32 i, ni;

	for ( i = 0, ni = m_nodes.size(); i < ni; ++i )
	{
		SoftBody::Node& n = m_nodes[i];

		if ( n.m_im > 0 )
		{
			if ( use_medium )
			{
				/* Aerodynamics         */
				addAeroForceToNode ( m_windVelocity, i );
			}

			/* Pressure             */

			if ( as_pressure )
			{
				n.m_f += n.m_n * ( n.m_area * ivolumetp );
			}

			/* Volume               */

			if ( as_volume )
			{
				n.m_f += n.m_n * ( n.m_area * dvolumetv );
			}
		}
	}

	/* Per face forces              */

	for ( i = 0, ni = m_faces.size(); i < ni; ++i )
	{
		//  SoftBody::Face& f=m_faces[i];

		/* Aerodynamics         */
		addAeroForceToFace ( m_windVelocity, i );
	}
}

//
void SoftBody::setMaxStress ( Scalar maxStress )
{
	m_cfg.m_maxStress = maxStress;
}

//
void SoftBody::interpolateRenderMesh()
{
	if ( m_z.size() > 0 )
	{
		for ( i32 i = 0; i < m_renderNodes.size(); ++i )
		{
			const Node* p0 = m_renderNodesParents[i][0];
			const Node* p1 = m_renderNodesParents[i][1];
			const Node* p2 = m_renderNodesParents[i][2];
			Vec3 normal = Cross ( p1->m_x - p0->m_x, p2->m_x - p0->m_x );
			Vec3 unit_normal = normal.normalized();
			RenderNode& n = m_renderNodes[i];
			n.m_x.setZero();

			for ( i32 j = 0; j < 3; ++j )
			{
				n.m_x += m_renderNodesParents[i][j]->m_x * m_renderNodesInterpolationWeights[i][j];
			}

			n.m_x += m_z[i] * unit_normal;
		}
	}

	else
	{
		for ( i32 i = 0; i < m_renderNodes.size(); ++i )
		{
			RenderNode& n = m_renderNodes[i];
			n.m_x.setZero();

			for ( i32 j = 0; j < 4; ++j )
			{
				if ( m_renderNodesParents[i].size() )
				{
					n.m_x += m_renderNodesParents[i][j]->m_x * m_renderNodesInterpolationWeights[i][j];
				}
			}
		}
	}
}

void SoftBody::setCollisionQuadrature ( i32 N )
{
	for ( i32 i = 0; i <= N; ++i )
	{
		for ( i32 j = 0; i + j <= N; ++j )
		{
			m_quads.push_back ( Vec3 ( Scalar ( i ) / Scalar ( N ), Scalar ( j ) / Scalar ( N ), Scalar ( N - i - j ) / Scalar ( N ) ) );
		}
	}
}

//
void SoftBody::PSolve_Anchors ( SoftBody* psb, Scalar kst, Scalar ti )
{
	DRX3D_PROFILE ( "PSolve_Anchors" );
	const Scalar kAHR = psb->m_cfg.kAHR * kst;
	const Scalar dt = psb->m_sst.sdt;

	for ( i32 i = 0, ni = psb->m_anchors.size(); i < ni; ++i )
	{
		const Anchor& a = psb->m_anchors[i];
		const Transform2& t = a.m_body->getWorldTransform();
		Node& n = *a.m_node;
		const Vec3 wa = t * a.m_local;
		const Vec3 va = a.m_body->getVelocityInLocalPoint ( a.m_c1 ) * dt;
		const Vec3 vb = n.m_x - n.m_q;
		const Vec3 vr = ( va - vb ) + ( wa - n.m_x ) * kAHR;
		const Vec3 impulse = a.m_c0 * vr * a.m_influence;
		n.m_x += impulse * a.m_c2;
		a.m_body->applyImpulse ( -impulse, a.m_c1 );
	}
}

//
void SoftBody::PSolve_RContacts ( SoftBody* psb, Scalar kst, Scalar ti )
{
	DRX3D_PROFILE ( "PSolve_RContacts" );
	const Scalar dt = psb->m_sst.sdt;
	const Scalar mrg = psb->getCollisionShape()->getMargin();
	MultiBodyJacobianData jacobianData;

	for ( i32 i = 0, ni = psb->m_rcontacts.size(); i < ni; ++i )
	{
		const RContact& c = psb->m_rcontacts[i];
		const sCti& cti = c.m_cti;

		if ( cti.m_colObj->hasContactResponse() )
		{
			Vec3 va ( 0, 0, 0 );
			RigidBody* rigidCol = 0;
			MultiBodyLinkCollider* multibodyLinkCol = 0;
			Scalar* deltaV = NULL;

			if ( cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY )
			{
				rigidCol = ( RigidBody* ) RigidBody::upcast ( cti.m_colObj );
				va = rigidCol ? rigidCol->getVelocityInLocalPoint ( c.m_c1 ) * dt : Vec3 ( 0, 0, 0 );
			}

			else
				if ( cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK )
				{
					multibodyLinkCol = ( MultiBodyLinkCollider* ) MultiBodyLinkCollider::upcast ( cti.m_colObj );

					if ( multibodyLinkCol )
					{
						i32k ndof = multibodyLinkCol->m_multiBody->getNumDofs() + 6;
						jacobianData.m_jacobians.resize ( ndof );
						jacobianData.m_deltaVelocitiesUnitImpulse.resize ( ndof );
						Scalar* jac = &jacobianData.m_jacobians[0];

						multibodyLinkCol->m_multiBody->fillContactJacobianMultiDof ( multibodyLinkCol->m_link, c.m_node->m_x, cti.m_normal, jac, jacobianData.scratch_r, jacobianData.scratch_v, jacobianData.scratch_m );
						deltaV = &jacobianData.m_deltaVelocitiesUnitImpulse[0];
						multibodyLinkCol->m_multiBody->calcAccelerationDeltasMultiDof ( &jacobianData.m_jacobians[0], deltaV, jacobianData.scratch_r, jacobianData.scratch_v );

						Scalar vel = 0.0;

						for ( i32 j = 0; j < ndof; ++j )
						{
							vel += multibodyLinkCol->m_multiBody->getVelocityVector() [j] * jac[j];
						}

						va = cti.m_normal * vel * dt;
					}
				}

			const Vec3 vb = c.m_node->m_x - c.m_node->m_q;

			const Vec3 vr = vb - va;

			const Scalar dn = Dot ( vr, cti.m_normal );

			if ( dn <= SIMD_EPSILON )
			{
				const Scalar dp = d3Min ( ( Dot ( c.m_node->m_x, cti.m_normal ) + cti.m_offset ), mrg );
				const Vec3 fv = vr - ( cti.m_normal * dn );
				// c0 is the impulse matrix, c3 is 1 - the friction coefficient or 0, c4 is the contact hardness coefficient
				const Vec3 impulse = c.m_c0 * ( ( vr - ( fv * c.m_c3 ) + ( cti.m_normal * ( dp * c.m_c4 ) ) ) * kst );
				c.m_node->m_x -= impulse * c.m_c2;

				if ( cti.m_colObj->getInternalType() == CollisionObject2::CO_RIGID_BODY )
				{
					if ( rigidCol )
						rigidCol->applyImpulse ( impulse, c.m_c1 );
				}

				else
					if ( cti.m_colObj->getInternalType() == CollisionObject2::CO_FEATHERSTONE_LINK )
					{
						if ( multibodyLinkCol )
						{
							double multiplier = 0.5;
							multibodyLinkCol->m_multiBody->applyDeltaVeeMultiDof ( deltaV, -impulse.length() * multiplier );
						}
					}
			}
		}
	}
}

//
void SoftBody::PSolve_SContacts ( SoftBody* psb, Scalar, Scalar ti )
{
	DRX3D_PROFILE ( "PSolve_SContacts" );

	for ( i32 i = 0, ni = psb->m_scontacts.size(); i < ni; ++i )
	{
		const SContact& c = psb->m_scontacts[i];
		const Vec3& nr = c.m_normal;
		Node& n = *c.m_node;
		Face& f = *c.m_face;
		const Vec3 p = BaryEval ( f.m_n[0]->m_x,
					   f.m_n[1]->m_x,
					   f.m_n[2]->m_x,
					   c.m_weights );
		const Vec3 q = BaryEval ( f.m_n[0]->m_q,
					   f.m_n[1]->m_q,
					   f.m_n[2]->m_q,
					   c.m_weights );
		const Vec3 vr = ( n.m_x - n.m_q ) - ( p - q );
		Vec3 corr ( 0, 0, 0 );
		Scalar dot = Dot ( vr, nr );

		if ( dot < 0 )
		{
			const Scalar j = c.m_margin - ( Dot ( nr, n.m_x ) - Dot ( nr, p ) );
			corr += c.m_normal * j;
		}

		corr -= ProjectOnPlane ( vr, nr ) * c.m_friction;

		n.m_x += corr * c.m_cfm[0];
		f.m_n[0]->m_x -= corr * ( c.m_cfm[1] * c.m_weights.x() );
		f.m_n[1]->m_x -= corr * ( c.m_cfm[1] * c.m_weights.y() );
		f.m_n[2]->m_x -= corr * ( c.m_cfm[1] * c.m_weights.z() );
	}
}

//
void SoftBody::PSolve_Links ( SoftBody* psb, Scalar kst, Scalar ti )
{
	DRX3D_PROFILE ( "PSolve_Links" );

	for ( i32 i = 0, ni = psb->m_links.size(); i < ni; ++i )
	{
		Link& l = psb->m_links[i];

		if ( l.m_c0 > 0 )
		{
			Node& a = *l.m_n[0];
			Node& b = *l.m_n[1];
			const Vec3 del = b.m_x - a.m_x;
			const Scalar len = del.length2();

			if ( l.m_c1 + len > SIMD_EPSILON )
			{
				const Scalar k = ( ( l.m_c1 - len ) / ( l.m_c0 * ( l.m_c1 + len ) ) ) * kst;
				a.m_x -= del * ( k * a.m_im );
				b.m_x += del * ( k * b.m_im );
			}
		}
	}
}

//
void SoftBody::VSolve_Links ( SoftBody* psb, Scalar kst )
{
	DRX3D_PROFILE ( "VSolve_Links" );

	for ( i32 i = 0, ni = psb->m_links.size(); i < ni; ++i )
	{
		Link& l = psb->m_links[i];
		Node** n = l.m_n;
		const Scalar j = -Dot ( l.m_c3, n[0]->m_v - n[1]->m_v ) * l.m_c2 * kst;
		n[0]->m_v += l.m_c3 * ( j * n[0]->m_im );
		n[1]->m_v -= l.m_c3 * ( j * n[1]->m_im );
	}
}

//
SoftBody::psolver_t SoftBody::getSolver ( ePSolver::_ solver )
{
	switch ( solver )
	{

		case ePSolver::Anchors:
			return ( &SoftBody::PSolve_Anchors );

		case ePSolver::Linear:
			return ( &SoftBody::PSolve_Links );

		case ePSolver::RContacts:
			return ( &SoftBody::PSolve_RContacts );

		case ePSolver::SContacts:
			return ( &SoftBody::PSolve_SContacts );

		default:
			{
			}
	}

	return ( 0 );
}

//
SoftBody::vsolver_t SoftBody::getSolver ( eVSolver::_ solver )
{
	switch ( solver )
	{

		case eVSolver::Linear:
			return ( &SoftBody::VSolve_Links );

		default:
			{
			}
	}

	return ( 0 );
}

void SoftBody::setSelfCollision ( bool useSelfCollision )
{
	m_useSelfCollision = useSelfCollision;
}

bool SoftBody::useSelfCollision()
{
	return m_useSelfCollision;
}

//
void SoftBody::defaultCollisionHandler ( const CollisionObject2Wrapper* pcoWrap )
{
	switch ( m_cfg.collisions & fCollision::RVSmask )
	{

		case fCollision::SDF_RS:
			{
				SoftColliders::CollideSDF_RS docollide;
				RigidBody* prb1 = ( RigidBody* ) RigidBody::upcast ( pcoWrap->getCollisionObject() );
				Transform2 wtr = pcoWrap->getWorldTransform();

				const Transform2 ctr = pcoWrap->getWorldTransform();
				const Scalar timemargin = ( wtr.getOrigin() - ctr.getOrigin() ).length();
				const Scalar basemargin = getCollisionShape()->getMargin();
				Vec3 mins;
				Vec3 maxs;
				ATTRIBUTE_ALIGNED16 ( DbvtVolume )
				volume;
				pcoWrap->getCollisionShape()->getAabb ( pcoWrap->getWorldTransform(),
						mins,
						maxs );
				volume = DbvtVolume::FromMM ( mins, maxs );
				volume.Expand ( Vec3 ( basemargin, basemargin, basemargin ) );
				docollide.psb = this;
				docollide.m_colObj1Wrap = pcoWrap;
				docollide.m_rigidBody = prb1;

				docollide.dynmargin = basemargin + timemargin;
				docollide.stamargin = basemargin;
				m_ndbvt.collideTV ( m_ndbvt.m_root, volume, docollide );
			}

			break;

		case fCollision::CL_RS:
			{
				SoftColliders::CollideCL_RS collider;
				collider.ProcessColObj ( this, pcoWrap );
			}

			break;

		case fCollision::SDF_RD:
			{
				RigidBody* prb1 = ( RigidBody* ) RigidBody::upcast ( pcoWrap->getCollisionObject() );

				if ( this->isActive() )
				{
					const Transform2 wtr = pcoWrap->getWorldTransform();
					const Scalar timemargin = 0;
					const Scalar basemargin = getCollisionShape()->getMargin();
					Vec3 mins;
					Vec3 maxs;
					ATTRIBUTE_ALIGNED16 ( DbvtVolume )
					volume;
					pcoWrap->getCollisionShape()->getAabb ( wtr,
							mins,
							maxs );
					volume = DbvtVolume::FromMM ( mins, maxs );
					volume.Expand ( Vec3 ( basemargin, basemargin, basemargin ) );

					if ( m_cfg.collisions & fCollision::SDF_RDN )
					{
						SoftColliders::CollideSDF_RD docollideNode;
						docollideNode.psb = this;
						docollideNode.m_colObj1Wrap = pcoWrap;
						docollideNode.m_rigidBody = prb1;
						docollideNode.dynmargin = basemargin + timemargin;
						docollideNode.stamargin = basemargin;
						m_ndbvt.collideTV ( m_ndbvt.m_root, volume, docollideNode );
					}

					if ( ( ( pcoWrap->getCollisionObject()->getInternalType() == CO_RIGID_BODY ) && ( m_cfg.collisions & fCollision::SDF_RDF ) ) || ( ( pcoWrap->getCollisionObject()->getInternalType() == CO_FEATHERSTONE_LINK ) && ( m_cfg.collisions & fCollision::SDF_MDF ) ) )
					{
						SoftColliders::CollideSDF_RDF docollideFace;
						docollideFace.psb = this;
						docollideFace.m_colObj1Wrap = pcoWrap;
						docollideFace.m_rigidBody = prb1;
						docollideFace.dynmargin = basemargin + timemargin;
						docollideFace.stamargin = basemargin;
						m_fdbvt.collideTV ( m_fdbvt.m_root, volume, docollideFace );
					}
				}
			}

			break;
	}
}

//
void SoftBody::defaultCollisionHandler ( SoftBody* psb )
{
	DRX3D_PROFILE ( "Deformable Collision" );
	i32k cf = m_cfg.collisions & psb->m_cfg.collisions;

	switch ( cf & fCollision::SVSmask )
	{

		case fCollision::CL_SS:
			{
				//support self-collision if CL_SELF flag set
				if ( this != psb || psb->m_cfg.collisions & fCollision::CL_SELF )
				{
					SoftColliders::CollideCL_SS docollide;
					docollide.ProcessSoftSoft ( this, psb );
				}
			}

			break;

		case fCollision::VF_SS:
			{
				//only self-collision for Cluster, not Vertex-Face yet
				if ( this != psb )
				{
					SoftColliders::CollideVF_SS docollide;
					/* common                   */
					docollide.mrg = getCollisionShape()->getMargin() +
									psb->getCollisionShape()->getMargin();
					/* psb0 nodes vs psb1 faces */
					docollide.psb[0] = this;
					docollide.psb[1] = psb;
					docollide.psb[0]->m_ndbvt.collideTT ( docollide.psb[0]->m_ndbvt.m_root,
							docollide.psb[1]->m_fdbvt.m_root,
							docollide );
					/* psb1 nodes vs psb0 faces */
					docollide.psb[0] = psb;
					docollide.psb[1] = this;
					docollide.psb[0]->m_ndbvt.collideTT ( docollide.psb[0]->m_ndbvt.m_root,
							docollide.psb[1]->m_fdbvt.m_root,
							docollide );
				}
			}

			break;

		case fCollision::VF_DD:
			{
				if ( !psb->m_softSoftCollision )
					return;

				if ( psb->isActive() || this->isActive() )
				{
					if ( this != psb )
					{
						SoftColliders::CollideVF_DD docollide;
						/* common                    */
						docollide.mrg = getCollisionShape()->getMargin() +
										psb->getCollisionShape()->getMargin();
						/* psb0 nodes vs psb1 faces    */

						if ( psb->m_tetras.size() > 0 )
							docollide.useFaceNormal = true;
						else
							docollide.useFaceNormal = false;

						docollide.psb[0] = this;

						docollide.psb[1] = psb;

						docollide.psb[0]->m_ndbvt.collideTT ( docollide.psb[0]->m_ndbvt.m_root,
								docollide.psb[1]->m_fdbvt.m_root,
								docollide );

						/* psb1 nodes vs psb0 faces    */
						if ( this->m_tetras.size() > 0 )
							docollide.useFaceNormal = true;
						else
							docollide.useFaceNormal = false;

						docollide.psb[0] = psb;

						docollide.psb[1] = this;

						docollide.psb[0]->m_ndbvt.collideTT ( docollide.psb[0]->m_ndbvt.m_root,
								docollide.psb[1]->m_fdbvt.m_root,
								docollide );
					}

					else
					{
						if ( psb->useSelfCollision() )
						{
							SoftColliders::CollideFF_DD docollide;
							docollide.mrg = 2 * getCollisionShape()->getMargin();
							docollide.psb[0] = this;
							docollide.psb[1] = psb;

							if ( this->m_tetras.size() > 0 )
								docollide.useFaceNormal = true;
							else
								docollide.useFaceNormal = false;

							/* psb0 faces vs psb0 faces    */
							calculateNormalCone ( this->m_fdbvnt );

							this->m_fdbvt.selfCollideT ( m_fdbvnt, docollide );
						}
					}
				}
			}

			break;

		default:
			{
			}
	}
}

void SoftBody::geometricCollisionHandler ( SoftBody* psb )
{
	if ( psb->isActive() || this->isActive() )
	{
		if ( this != psb )
		{
			SoftColliders::CollideCCD docollide;
			/* common                    */
			docollide.mrg = SAFE_EPSILON;  // for rounding error instead of actual margin
			docollide.dt = psb->m_sst.sdt;
			/* psb0 nodes vs psb1 faces    */

			if ( psb->m_tetras.size() > 0 )
				docollide.useFaceNormal = true;
			else
				docollide.useFaceNormal = false;

			docollide.psb[0] = this;

			docollide.psb[1] = psb;

			docollide.psb[0]->m_ndbvt.collideTT ( docollide.psb[0]->m_ndbvt.m_root,
					docollide.psb[1]->m_fdbvt.m_root,
					docollide );

			/* psb1 nodes vs psb0 faces    */
			if ( this->m_tetras.size() > 0 )
				docollide.useFaceNormal = true;
			else
				docollide.useFaceNormal = false;

			docollide.psb[0] = psb;

			docollide.psb[1] = this;

			docollide.psb[0]->m_ndbvt.collideTT ( docollide.psb[0]->m_ndbvt.m_root,
					docollide.psb[1]->m_fdbvt.m_root,
					docollide );
		}

		else
		{
			if ( psb->useSelfCollision() )
			{
				SoftColliders::CollideCCD docollide;
				docollide.mrg = SAFE_EPSILON;
				docollide.psb[0] = this;
				docollide.psb[1] = psb;
				docollide.dt = psb->m_sst.sdt;

				if ( this->m_tetras.size() > 0 )
					docollide.useFaceNormal = true;
				else
					docollide.useFaceNormal = false;

				/* psb0 faces vs psb0 faces    */
				calculateNormalCone ( this->m_fdbvnt );  // should compute this outside of this scope

				this->m_fdbvt.selfCollideT ( m_fdbvnt, docollide );
			}
		}
	}
}

void SoftBody::setWindVelocity ( const Vec3& velocity )
{
	m_windVelocity = velocity;
}

const Vec3& SoftBody::getWindVelocity()
{
	return m_windVelocity;
}

i32 SoftBody::calculateSerializeBufferSize() const
{
	i32 sz = sizeof ( SoftBodyData );
	return sz;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
tukk SoftBody::serialize ( uk dataBuffer, class Serializer* serializer ) const
{
	SoftBodyData* sbd = ( SoftBodyData* ) dataBuffer;

	CollisionObject2::serialize ( &sbd->m_collisionObjectData, serializer );

	HashMap<HashPtr, i32> m_nodeIndexMap;

	sbd->m_numMaterials = m_materials.size();
	sbd->m_materials = sbd->m_numMaterials ? ( SoftBodyMaterialData** ) serializer->getUniquePointer ( ( uk ) & m_materials ) : 0;

	if ( sbd->m_materials )
	{
		i32 sz = sizeof ( SoftBodyMaterialData* );
		i32 numElem = sbd->m_numMaterials;
		Chunk* chunk = serializer->allocate ( sz, numElem );
		//SoftBodyMaterialData** memPtr = chunk->m_oldPtr;
		SoftBodyMaterialData** memPtr = ( SoftBodyMaterialData** ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			SoftBody::Material* mat = m_materials[i];
			*memPtr = mat ? ( SoftBodyMaterialData* ) serializer->getUniquePointer ( ( uk ) mat ) : 0;

			if ( !serializer->findPointer ( mat ) )
			{
				//serialize it here
				Chunk* chunk = serializer->allocate ( sizeof ( SoftBodyMaterialData ), 1 );
				SoftBodyMaterialData* memPtr = ( SoftBodyMaterialData* ) chunk->m_oldPtr;
				memPtr->m_flags = mat->m_flags;
				memPtr->m_angularStiffness = mat->m_kAST;
				memPtr->m_linearStiffness = mat->m_kLST;
				memPtr->m_volumeStiffness = mat->m_kVST;
				serializer->finalizeChunk ( chunk, "SoftBodyMaterialData", DRX3D_SBMATERIAL_CODE, mat );
			}
		}

		serializer->finalizeChunk ( chunk, "SoftBodyMaterialData", DRX3D_ARRAY_CODE, ( uk ) &m_materials );
	}

	sbd->m_numNodes = m_nodes.size();

	sbd->m_nodes = sbd->m_numNodes ? ( SoftBodyNodeData* ) serializer->getUniquePointer ( ( uk ) & m_nodes ) : 0;

	if ( sbd->m_nodes )
	{
		i32 sz = sizeof ( SoftBodyNodeData );
		i32 numElem = sbd->m_numNodes;
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftBodyNodeData* memPtr = ( SoftBodyNodeData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			m_nodes[i].m_f.serializeFloat ( memPtr->m_accumulatedForce );
			memPtr->m_area = m_nodes[i].m_area;
			memPtr->m_attach = m_nodes[i].m_battach;
			memPtr->m_inverseMass = m_nodes[i].m_im;
			memPtr->m_material = m_nodes[i].m_material ? ( SoftBodyMaterialData* ) serializer->getUniquePointer ( ( uk ) m_nodes[i].m_material ) : 0;
			m_nodes[i].m_n.serializeFloat ( memPtr->m_normal );
			m_nodes[i].m_x.serializeFloat ( memPtr->m_position );
			m_nodes[i].m_q.serializeFloat ( memPtr->m_previousPosition );
			m_nodes[i].m_v.serializeFloat ( memPtr->m_velocity );
			m_nodeIndexMap.insert ( &m_nodes[i], i );
		}

		serializer->finalizeChunk ( chunk, "SoftBodyNodeData", DRX3D_SBNODE_CODE, ( uk ) &m_nodes );
	}

	sbd->m_numLinks = m_links.size();

	sbd->m_links = sbd->m_numLinks ? ( SoftBodyLinkData* ) serializer->getUniquePointer ( ( uk ) & m_links[0] ) : 0;

	if ( sbd->m_links )
	{
		i32 sz = sizeof ( SoftBodyLinkData );
		i32 numElem = sbd->m_numLinks;
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftBodyLinkData* memPtr = ( SoftBodyLinkData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			memPtr->m_bbending = m_links[i].m_bbending;
			memPtr->m_material = m_links[i].m_material ? ( SoftBodyMaterialData* ) serializer->getUniquePointer ( ( uk ) m_links[i].m_material ) : 0;
			memPtr->m_nodeIndices[0] = m_links[i].m_n[0] ? m_links[i].m_n[0] - &m_nodes[0] : -1;
			memPtr->m_nodeIndices[1] = m_links[i].m_n[1] ? m_links[i].m_n[1] - &m_nodes[0] : -1;
			Assert ( memPtr->m_nodeIndices[0] < m_nodes.size() );
			Assert ( memPtr->m_nodeIndices[1] < m_nodes.size() );
			memPtr->m_restLength = m_links[i].m_rl;
		}

		serializer->finalizeChunk ( chunk, "SoftBodyLinkData", DRX3D_ARRAY_CODE, ( uk ) &m_links[0] );
	}

	sbd->m_numFaces = m_faces.size();

	sbd->m_faces = sbd->m_numFaces ? ( SoftBodyFaceData* ) serializer->getUniquePointer ( ( uk ) & m_faces[0] ) : 0;

	if ( sbd->m_faces )
	{
		i32 sz = sizeof ( SoftBodyFaceData );
		i32 numElem = sbd->m_numFaces;
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftBodyFaceData* memPtr = ( SoftBodyFaceData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			memPtr->m_material = m_faces[i].m_material ? ( SoftBodyMaterialData* ) serializer->getUniquePointer ( ( uk ) m_faces[i].m_material ) : 0;
			m_faces[i].m_normal.serializeFloat ( memPtr->m_normal );

			for ( i32 j = 0; j < 3; j++ )
			{
				memPtr->m_nodeIndices[j] = m_faces[i].m_n[j] ? m_faces[i].m_n[j] - &m_nodes[0] : -1;
			}

			memPtr->m_restArea = m_faces[i].m_ra;
		}

		serializer->finalizeChunk ( chunk, "SoftBodyFaceData", DRX3D_ARRAY_CODE, ( uk ) &m_faces[0] );
	}

	sbd->m_numTetrahedra = m_tetras.size();

	sbd->m_tetrahedra = sbd->m_numTetrahedra ? ( SoftBodyTetraData* ) serializer->getUniquePointer ( ( uk ) & m_tetras[0] ) : 0;

	if ( sbd->m_tetrahedra )
	{
		i32 sz = sizeof ( SoftBodyTetraData );
		i32 numElem = sbd->m_numTetrahedra;
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftBodyTetraData* memPtr = ( SoftBodyTetraData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			for ( i32 j = 0; j < 4; j++ )
			{
				m_tetras[i].m_c0[j].serializeFloat ( memPtr->m_c0[j] );
				memPtr->m_nodeIndices[j] = m_tetras[i].m_n[j] ? m_tetras[i].m_n[j] - &m_nodes[0] : -1;
			}

			memPtr->m_c1 = m_tetras[i].m_c1;

			memPtr->m_c2 = m_tetras[i].m_c2;
			memPtr->m_material = m_tetras[i].m_material ? ( SoftBodyMaterialData* ) serializer->getUniquePointer ( ( uk ) m_tetras[i].m_material ) : 0;
			memPtr->m_restVolume = m_tetras[i].m_rv;
		}

		serializer->finalizeChunk ( chunk, "SoftBodyTetraData", DRX3D_ARRAY_CODE, ( uk ) &m_tetras[0] );
	}

	sbd->m_numAnchors = m_anchors.size();

	sbd->m_anchors = sbd->m_numAnchors ? ( SoftRigidAnchorData* ) serializer->getUniquePointer ( ( uk ) & m_anchors[0] ) : 0;

	if ( sbd->m_anchors )
	{
		i32 sz = sizeof ( SoftRigidAnchorData );
		i32 numElem = sbd->m_numAnchors;
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftRigidAnchorData* memPtr = ( SoftRigidAnchorData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			m_anchors[i].m_c0.serializeFloat ( memPtr->m_c0 );
			m_anchors[i].m_c1.serializeFloat ( memPtr->m_c1 );
			memPtr->m_c2 = m_anchors[i].m_c2;
			m_anchors[i].m_local.serializeFloat ( memPtr->m_localFrame );
			memPtr->m_nodeIndex = m_anchors[i].m_node ? m_anchors[i].m_node - &m_nodes[0] : -1;

			memPtr->m_rigidBody = m_anchors[i].m_body ? ( RigidBodyData* ) serializer->getUniquePointer ( ( uk ) m_anchors[i].m_body ) : 0;
			Assert ( memPtr->m_nodeIndex < m_nodes.size() );
		}

		serializer->finalizeChunk ( chunk, "SoftRigidAnchorData", DRX3D_ARRAY_CODE, ( uk ) &m_anchors[0] );
	}

	sbd->m_config.m_dynamicFriction = m_cfg.kDF;

	sbd->m_config.m_baumgarte = m_cfg.kVCF;
	sbd->m_config.m_pressure = m_cfg.kPR;
	sbd->m_config.m_aeroModel = this->m_cfg.aeromodel;
	sbd->m_config.m_lift = m_cfg.kLF;
	sbd->m_config.m_drag = m_cfg.kDG;
	sbd->m_config.m_positionIterations = m_cfg.piterations;
	sbd->m_config.m_driftIterations = m_cfg.diterations;
	sbd->m_config.m_clusterIterations = m_cfg.citerations;
	sbd->m_config.m_velocityIterations = m_cfg.viterations;
	sbd->m_config.m_maxVolume = m_cfg.maxvolume;
	sbd->m_config.m_damping = m_cfg.kDP;
	sbd->m_config.m_poseMatch = m_cfg.kMT;
	sbd->m_config.m_collisionFlags = m_cfg.collisions;
	sbd->m_config.m_volume = m_cfg.kVC;
	sbd->m_config.m_rigidContactHardness = m_cfg.kCHR;
	sbd->m_config.m_kineticContactHardness = m_cfg.kKHR;
	sbd->m_config.m_softContactHardness = m_cfg.kSHR;
	sbd->m_config.m_anchorHardness = m_cfg.kAHR;
	sbd->m_config.m_timeScale = m_cfg.timescale;
	sbd->m_config.m_maxVolume = m_cfg.maxvolume;
	sbd->m_config.m_softRigidClusterHardness = m_cfg.kSRHR_CL;
	sbd->m_config.m_softKineticClusterHardness = m_cfg.kSKHR_CL;
	sbd->m_config.m_softSoftClusterHardness = m_cfg.kSSHR_CL;
	sbd->m_config.m_softRigidClusterImpulseSplit = m_cfg.kSR_SPLT_CL;
	sbd->m_config.m_softKineticClusterImpulseSplit = m_cfg.kSK_SPLT_CL;
	sbd->m_config.m_softSoftClusterImpulseSplit = m_cfg.kSS_SPLT_CL;

	//pose for shape matching
	{
		sbd->m_pose = ( SoftBodyPoseData* ) serializer->getUniquePointer ( ( uk ) & m_pose );

		i32 sz = sizeof ( SoftBodyPoseData );
		Chunk* chunk = serializer->allocate ( sz, 1 );
		SoftBodyPoseData* memPtr = ( SoftBodyPoseData* ) chunk->m_oldPtr;

		m_pose.m_aqq.serializeFloat ( memPtr->m_aqq );
		memPtr->m_bframe = m_pose.m_bframe;
		memPtr->m_bvolume = m_pose.m_bvolume;
		m_pose.m_com.serializeFloat ( memPtr->m_com );

		memPtr->m_numPositions = m_pose.m_pos.size();
		memPtr->m_positions = memPtr->m_numPositions ? ( Vec3FloatData* ) serializer->getUniquePointer ( ( uk ) & m_pose.m_pos[0] ) : 0;

		if ( memPtr->m_numPositions )
		{
			i32 numElem = memPtr->m_numPositions;
			i32 sz = sizeof ( Vec3Data );
			Chunk* chunk = serializer->allocate ( sz, numElem );
			Vec3FloatData* memPtr = ( Vec3FloatData* ) chunk->m_oldPtr;

			for ( i32 i = 0; i < numElem; i++, memPtr++ )
			{
				m_pose.m_pos[i].serializeFloat ( *memPtr );
			}

			serializer->finalizeChunk ( chunk, "Vec3FloatData", DRX3D_ARRAY_CODE, ( uk ) &m_pose.m_pos[0] );
		}

		memPtr->m_restVolume = m_pose.m_volume;

		m_pose.m_rot.serializeFloat ( memPtr->m_rot );
		m_pose.m_scl.serializeFloat ( memPtr->m_scale );

		memPtr->m_numWeigts = m_pose.m_wgh.size();
		memPtr->m_weights = memPtr->m_numWeigts ? ( float* ) serializer->getUniquePointer ( ( uk ) & m_pose.m_wgh[0] ) : 0;

		if ( memPtr->m_numWeigts )
		{
			i32 numElem = memPtr->m_numWeigts;
			i32 sz = sizeof ( float );
			Chunk* chunk = serializer->allocate ( sz, numElem );
			float* memPtr = ( float* ) chunk->m_oldPtr;

			for ( i32 i = 0; i < numElem; i++, memPtr++ )
			{
				*memPtr = m_pose.m_wgh[i];
			}

			serializer->finalizeChunk ( chunk, "float", DRX3D_ARRAY_CODE, ( uk ) &m_pose.m_wgh[0] );
		}

		serializer->finalizeChunk ( chunk, "SoftBodyPoseData", DRX3D_ARRAY_CODE, ( uk ) &m_pose );
	}

	//clusters for convex-cluster collision detection

	sbd->m_numClusters = m_clusters.size();
	sbd->m_clusters = sbd->m_numClusters ? ( SoftBodyClusterData* ) serializer->getUniquePointer ( ( uk ) m_clusters[0] ) : 0;

	if ( sbd->m_numClusters )
	{
		i32 numElem = sbd->m_numClusters;
		i32 sz = sizeof ( SoftBodyClusterData );
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftBodyClusterData* memPtr = ( SoftBodyClusterData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			memPtr->m_adamping = m_clusters[i]->m_adamping;
			m_clusters[i]->m_av.serializeFloat ( memPtr->m_av );
			memPtr->m_clusterIndex = m_clusters[i]->m_clusterIndex;
			memPtr->m_collide = m_clusters[i]->m_collide;
			m_clusters[i]->m_com.serializeFloat ( memPtr->m_com );
			memPtr->m_containsAnchor = m_clusters[i]->m_containsAnchor;
			m_clusters[i]->m_dimpulses[0].serializeFloat ( memPtr->m_dimpulses[0] );
			m_clusters[i]->m_dimpulses[1].serializeFloat ( memPtr->m_dimpulses[1] );
			m_clusters[i]->m_framexform.serializeFloat ( memPtr->m_framexform );
			memPtr->m_idmass = m_clusters[i]->m_idmass;
			memPtr->m_imass = m_clusters[i]->m_imass;
			m_clusters[i]->m_invwi.serializeFloat ( memPtr->m_invwi );
			memPtr->m_ldamping = m_clusters[i]->m_ldamping;
			m_clusters[i]->m_locii.serializeFloat ( memPtr->m_locii );
			m_clusters[i]->m_lv.serializeFloat ( memPtr->m_lv );
			memPtr->m_matching = m_clusters[i]->m_matching;
			memPtr->m_maxSelfCollisionImpulse = m_clusters[i]->m_maxSelfCollisionImpulse;
			memPtr->m_ndamping = m_clusters[i]->m_ndamping;
			memPtr->m_ldamping = m_clusters[i]->m_ldamping;
			memPtr->m_adamping = m_clusters[i]->m_adamping;
			memPtr->m_selfCollisionImpulseFactor = m_clusters[i]->m_selfCollisionImpulseFactor;

			memPtr->m_numFrameRefs = m_clusters[i]->m_framerefs.size();
			memPtr->m_numMasses = m_clusters[i]->m_masses.size();
			memPtr->m_numNodes = m_clusters[i]->m_nodes.size();

			memPtr->m_nvimpulses = m_clusters[i]->m_nvimpulses;
			m_clusters[i]->m_vimpulses[0].serializeFloat ( memPtr->m_vimpulses[0] );
			m_clusters[i]->m_vimpulses[1].serializeFloat ( memPtr->m_vimpulses[1] );
			memPtr->m_ndimpulses = m_clusters[i]->m_ndimpulses;

			memPtr->m_framerefs = memPtr->m_numFrameRefs ? ( Vec3FloatData* ) serializer->getUniquePointer ( ( uk ) & m_clusters[i]->m_framerefs[0] ) : 0;

			if ( memPtr->m_framerefs )
			{
				i32 numElem = memPtr->m_numFrameRefs;
				i32 sz = sizeof ( Vec3FloatData );
				Chunk* chunk = serializer->allocate ( sz, numElem );
				Vec3FloatData* memPtr = ( Vec3FloatData* ) chunk->m_oldPtr;

				for ( i32 j = 0; j < numElem; j++, memPtr++ )
				{
					m_clusters[i]->m_framerefs[j].serializeFloat ( *memPtr );
				}

				serializer->finalizeChunk ( chunk, "Vec3FloatData", DRX3D_ARRAY_CODE, ( uk ) &m_clusters[i]->m_framerefs[0] );
			}

			memPtr->m_masses = memPtr->m_numMasses ? ( float* ) serializer->getUniquePointer ( ( uk ) & m_clusters[i]->m_masses[0] ) : 0;

			if ( memPtr->m_masses )
			{
				i32 numElem = memPtr->m_numMasses;
				i32 sz = sizeof ( float );
				Chunk* chunk = serializer->allocate ( sz, numElem );
				float* memPtr = ( float* ) chunk->m_oldPtr;

				for ( i32 j = 0; j < numElem; j++, memPtr++ )
				{
					*memPtr = m_clusters[i]->m_masses[j];
				}

				serializer->finalizeChunk ( chunk, "float", DRX3D_ARRAY_CODE, ( uk ) &m_clusters[i]->m_masses[0] );
			}

			memPtr->m_nodeIndices = memPtr->m_numNodes ? ( i32* ) serializer->getUniquePointer ( ( uk ) & m_clusters[i]->m_nodes ) : 0;

			if ( memPtr->m_nodeIndices )
			{
				i32 numElem = memPtr->m_numMasses;
				i32 sz = sizeof ( i32 );
				Chunk* chunk = serializer->allocate ( sz, numElem );
				i32* memPtr = ( i32* ) chunk->m_oldPtr;

				for ( i32 j = 0; j < numElem; j++, memPtr++ )
				{
					i32* indexPtr = m_nodeIndexMap.find ( m_clusters[i]->m_nodes[j] );
					Assert ( indexPtr );
					*memPtr = *indexPtr;
				}

				serializer->finalizeChunk ( chunk, "i32", DRX3D_ARRAY_CODE, ( uk ) &m_clusters[i]->m_nodes );
			}
		}

		serializer->finalizeChunk ( chunk, "SoftBodyClusterData", DRX3D_ARRAY_CODE, ( uk ) m_clusters[0] );
	}

	sbd->m_numJoints = m_joints.size();

	sbd->m_joints = m_joints.size() ? ( SoftBodyJointData* ) serializer->getUniquePointer ( ( uk ) & m_joints[0] ) : 0;

	if ( sbd->m_joints )
	{
		i32 sz = sizeof ( SoftBodyJointData );
		i32 numElem = m_joints.size();
		Chunk* chunk = serializer->allocate ( sz, numElem );
		SoftBodyJointData* memPtr = ( SoftBodyJointData* ) chunk->m_oldPtr;

		for ( i32 i = 0; i < numElem; i++, memPtr++ )
		{
			memPtr->m_jointType = ( i32 ) m_joints[i]->Type();
			m_joints[i]->m_refs[0].serializeFloat ( memPtr->m_refs[0] );
			m_joints[i]->m_refs[1].serializeFloat ( memPtr->m_refs[1] );
			memPtr->m_cfm = m_joints[i]->m_cfm;
			memPtr->m_erp = float ( m_joints[i]->m_erp );
			memPtr->m_split = float ( m_joints[i]->m_split );
			memPtr->m_delete = m_joints[i]->m_delete;

			for ( i32 j = 0; j < 4; j++ )
			{
				memPtr->m_relPosition[0].m_floats[j] = 0.f;
				memPtr->m_relPosition[1].m_floats[j] = 0.f;
			}

			memPtr->m_bodyA = 0;

			memPtr->m_bodyB = 0;

			if ( m_joints[i]->m_bodies[0].m_soft )
			{
				memPtr->m_bodyAtype = DRX3D_JOINT_SOFT_BODY_CLUSTER;
				memPtr->m_bodyA = serializer->getUniquePointer ( ( uk ) m_joints[i]->m_bodies[0].m_soft );
			}

			if ( m_joints[i]->m_bodies[0].m_collisionObject )
			{
				memPtr->m_bodyAtype = DRX3D_JOINT_COLLISION_OBJECT;
				memPtr->m_bodyA = serializer->getUniquePointer ( ( uk ) m_joints[i]->m_bodies[0].m_collisionObject );
			}

			if ( m_joints[i]->m_bodies[0].m_rigid )
			{
				memPtr->m_bodyAtype = DRX3D_JOINT_RIGID_BODY;
				memPtr->m_bodyA = serializer->getUniquePointer ( ( uk ) m_joints[i]->m_bodies[0].m_rigid );
			}

			if ( m_joints[i]->m_bodies[1].m_soft )
			{
				memPtr->m_bodyBtype = DRX3D_JOINT_SOFT_BODY_CLUSTER;
				memPtr->m_bodyB = serializer->getUniquePointer ( ( uk ) m_joints[i]->m_bodies[1].m_soft );
			}

			if ( m_joints[i]->m_bodies[1].m_collisionObject )
			{
				memPtr->m_bodyBtype = DRX3D_JOINT_COLLISION_OBJECT;
				memPtr->m_bodyB = serializer->getUniquePointer ( ( uk ) m_joints[i]->m_bodies[1].m_collisionObject );
			}

			if ( m_joints[i]->m_bodies[1].m_rigid )
			{
				memPtr->m_bodyBtype = DRX3D_JOINT_RIGID_BODY;
				memPtr->m_bodyB = serializer->getUniquePointer ( ( uk ) m_joints[i]->m_bodies[1].m_rigid );
			}
		}

		serializer->finalizeChunk ( chunk, "SoftBodyJointData", DRX3D_ARRAY_CODE, ( uk ) &m_joints[0] );
	}

	return SoftBodyDataName;
}

void SoftBody::updateDeactivation ( Scalar timeStep )
{
	if ( ( getActivationState() == ISLAND_SLEEPING ) || ( getActivationState() == DISABLE_DEACTIVATION ) )
		return;

	if ( m_maxSpeedSquared < m_sleepingThreshold * m_sleepingThreshold )
	{
		m_deactivationTime += timeStep;
	}

	else
	{
		m_deactivationTime = Scalar ( 0. );
		setActivationState ( 0 );
	}
}

void SoftBody::setZeroVelocity()
{
	for ( i32 i = 0; i < m_nodes.size(); ++i )
	{
		m_nodes[i].m_v.setZero();
	}
}

bool SoftBody::wantsSleeping()
{
	if ( getActivationState() == DISABLE_DEACTIVATION )
		return false;

	//disable deactivation
	if ( gDisableDeactivation || ( gDeactivationTime == Scalar ( 0. ) ) )
		return false;

	if ( ( getActivationState() == ISLAND_SLEEPING ) || ( getActivationState() == WANTS_DEACTIVATION ) )
		return true;

	if ( m_deactivationTime > gDeactivationTime )
	{
		return true;
	}

	return false;
}
