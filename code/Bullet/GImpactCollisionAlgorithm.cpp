#include <drx3D/Physics/Collision/Dispatch/ManifoldResult.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Gimpact/GImpactCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Gimpact/ContactProcessing.h>
#include <drx3D/Maths/Linear/Quickprof.h>

//! Class for accessing the plane equation

class PlaneShape : public StaticPlaneShape
{

	public:
		PlaneShape ( const Vec3& v, float f )
				: StaticPlaneShape ( v, f )
		{
		}

		void get_plane_equation ( Vec4& equation )
		{
			equation[0] = m_planeNormal[0];
			equation[1] = m_planeNormal[1];
			equation[2] = m_planeNormal[2];
			equation[3] = m_planeConstant;
		}

		void get_plane_equation_transformed ( const Transform2& trans, Vec4& equation ) const
		{
			const Vec3 normal = trans.getBasis() * m_planeNormal;
			equation[0] = normal[0];
			equation[1] = normal[1];
			equation[2] = normal[2];
			equation[3] = normal.dot ( trans * ( m_planeConstant * m_planeNormal ) );
		}
};

//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef TRI_COLLISION_PROFILING

Clock g_triangle_clock;

float g_accum_triangle_collision_time = 0;
i32 g_count_triangle_collision = 0;

void drx3d_begin_gim02_tri_time()
{
	g_triangle_clock.reset();
}

void drx3d_end_gim02_tri_time()
{
	g_accum_triangle_collision_time += g_triangle_clock.getTimeMicroseconds();
	g_count_triangle_collision++;
}

#endif  //TRI_COLLISION_PROFILING
//! Retrieving shapes shapes
/*!
Declared here due of insuficent space on Pool allocators
*/
//!@{

class GIM_ShapeRetriever
{

	public:
		const GImpactShapeInterface* m_gim_shape;
		TriangleShapeEx m_trishape;
		TetrahedronShapeEx m_tetrashape;

	public:

		class ChildShapeRetriever
		{

			public:
				GIM_ShapeRetriever* m_parent;
				virtual const CollisionShape* getChildShape ( i32 index )
				{
					return m_parent->m_gim_shape->getChildShape ( index );
				}

				virtual ~ChildShapeRetriever() {}
		};

	class TriangleShapeRetriever : public ChildShapeRetriever
		{

			public:
				virtual CollisionShape* getChildShape ( i32 index )
				{
					m_parent->m_gim_shape->getBulletTriangle ( index, m_parent->m_trishape );
					return &m_parent->m_trishape;
				}

				virtual ~TriangleShapeRetriever() {}
		};

	class TetraShapeRetriever : public ChildShapeRetriever
		{

			public:
				virtual CollisionShape* getChildShape ( i32 index )
				{
					m_parent->m_gim_shape->getBulletTetrahedron ( index, m_parent->m_tetrashape );
					return &m_parent->m_tetrashape;
				}
		};

	public:
		ChildShapeRetriever m_child_retriever;
		TriangleShapeRetriever m_tri_retriever;
		TetraShapeRetriever m_tetra_retriever;
		ChildShapeRetriever* m_current_retriever;

		GIM_ShapeRetriever ( const GImpactShapeInterface* gim_shape )
		{
			m_gim_shape = gim_shape;
			//select retriever

			if ( m_gim_shape->needsRetrieveTriangles() )
			{
				m_current_retriever = &m_tri_retriever;
			}

			else
				if ( m_gim_shape->needsRetrieveTetrahedrons() )
				{
					m_current_retriever = &m_tetra_retriever;
				}

				else
				{
					m_current_retriever = &m_child_retriever;
				}

			m_current_retriever->m_parent = this;
		}

		const CollisionShape* getChildShape ( i32 index )
		{
			return m_current_retriever->getChildShape ( index );
		}
};

//!@}

#ifdef TRI_COLLISION_PROFILING

//! Gets the average time in miliseconds of tree collisions
float GImpactCollisionAlgorithm::getAverageTreeCollisionTime()
{
	return GImpactBoxSet::getAverageTreeCollisionTime();
}

//! Gets the average time in miliseconds of triangle collisions
float GImpactCollisionAlgorithm::getAverageTriangleCollisionTime()
{
	if ( g_count_triangle_collision == 0 )
		return 0;

	float avgtime = g_accum_triangle_collision_time;

	avgtime /= ( float ) g_count_triangle_collision;

	g_accum_triangle_collision_time = 0;

	g_count_triangle_collision = 0;

	return avgtime;
}

#endif  //TRI_COLLISION_PROFILING

GImpactCollisionAlgorithm::GImpactCollisionAlgorithm ( const CollisionAlgorithmConstructionInfo& ci, const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap )
		: ActivatingCollisionAlgorithm ( ci, body0Wrap, body1Wrap )
{
	m_manifoldPtr = NULL;
	m_convex_algorithm = NULL;
}

GImpactCollisionAlgorithm::~GImpactCollisionAlgorithm()
{
	clearCache();
}

void GImpactCollisionAlgorithm::addContactPoint ( const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const Vec3& point,
		const Vec3& normal,
		Scalar distance )
{
	m_resultOut->setShapeIdentifiersA ( m_part0, m_triface0 );
	m_resultOut->setShapeIdentifiersB ( m_part1, m_triface1 );
	checkManifold ( body0Wrap, body1Wrap );
	m_resultOut->addContactPoint ( normal, point, distance );
}

void GImpactCollisionAlgorithm::shape_vs_shape_collision (
	const CollisionObject2Wrapper* body0Wrap,
	const CollisionObject2Wrapper* body1Wrap,
	const CollisionShape* shape0,
	const CollisionShape* shape1 )
{
	{
		CollisionAlgorithm* algor = newAlgorithm ( body0Wrap, body1Wrap );
		// post :	checkManifold is called

		m_resultOut->setShapeIdentifiersA ( m_part0, m_triface0 );
		m_resultOut->setShapeIdentifiersB ( m_part1, m_triface1 );

		algor->processCollision ( body0Wrap, body1Wrap, *m_dispatchInfo, m_resultOut );

		algor->~CollisionAlgorithm();
		m_dispatcher->freeCollisionAlgorithm ( algor );
	}
}

void GImpactCollisionAlgorithm::convex_vs_convex_collision (
	const CollisionObject2Wrapper* body0Wrap,
	const CollisionObject2Wrapper* body1Wrap,
	const CollisionShape* shape0,
	const CollisionShape* shape1 )
{
	m_resultOut->setShapeIdentifiersA ( m_part0, m_triface0 );
	m_resultOut->setShapeIdentifiersB ( m_part1, m_triface1 );

	CollisionObject2Wrapper ob0 ( body0Wrap, shape0, body0Wrap->getCollisionObject(), body0Wrap->getWorldTransform(), m_part0, m_triface0 );
	CollisionObject2Wrapper ob1 ( body1Wrap, shape1, body1Wrap->getCollisionObject(), body1Wrap->getWorldTransform(), m_part1, m_triface1 );
	checkConvexAlgorithm ( &ob0, &ob1 );
	m_convex_algorithm->processCollision ( &ob0, &ob1, *m_dispatchInfo, m_resultOut );
}

void GImpactCollisionAlgorithm::gimpact_vs_gimpact_find_pairs (
	const Transform2& trans0,
	const Transform2& trans1,
	const GImpactShapeInterface* shape0,
	const GImpactShapeInterface* shape1, PairSet& pairset )
{
	if ( shape0->hasBoxSet() && shape1->hasBoxSet() )
	{
		GImpactBoxSet::find_collision ( shape0->getBoxSet(), trans0, shape1->getBoxSet(), trans1, pairset );
	}

	else
	{
		AABB boxshape0;
		AABB boxshape1;
		i32 i = shape0->getNumChildShapes();

		while ( i-- )
		{
			shape0->getChildAabb ( i, trans0, boxshape0.m_min, boxshape0.m_max );

			i32 j = shape1->getNumChildShapes();

			while ( j-- )
			{
				shape1->getChildAabb ( i, trans1, boxshape1.m_min, boxshape1.m_max );

				if ( boxshape1.has_collision ( boxshape0 ) )
				{
					pairset.push_pair ( i, j );
				}
			}
		}
	}
}

void GImpactCollisionAlgorithm::gimpact_vs_shape_find_pairs (
	const Transform2& trans0,
	const Transform2& trans1,
	const GImpactShapeInterface* shape0,
	const CollisionShape* shape1,
	AlignedObjectArray<i32>& collided_primitives )
{
	AABB boxshape;

	if ( shape0->hasBoxSet() )
	{
		Transform2 trans1to0 = trans0.inverse();
		trans1to0 *= trans1;

		shape1->getAabb ( trans1to0, boxshape.m_min, boxshape.m_max );

		shape0->getBoxSet()->boxQuery ( boxshape, collided_primitives );
	}

	else
	{
		shape1->getAabb ( trans1, boxshape.m_min, boxshape.m_max );

		AABB boxshape0;
		i32 i = shape0->getNumChildShapes();

		while ( i-- )
		{
			shape0->getChildAabb ( i, trans0, boxshape0.m_min, boxshape0.m_max );

			if ( boxshape.has_collision ( boxshape0 ) )
			{
				collided_primitives.push_back ( i );
			}
		}
	}
}

void GImpactCollisionAlgorithm::collide_gjk_triangles ( const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const GImpactMeshShapePart* shape0,
		const GImpactMeshShapePart* shape1,
		i32k* pairs, i32 pair_count )
{
	TriangleShapeEx tri0;
	TriangleShapeEx tri1;

	shape0->lockChildShapes();
	shape1->lockChildShapes();

	i32k* pair_pointer = pairs;

	while ( pair_count-- )
	{
		m_triface0 = * ( pair_pointer );
		m_triface1 = * ( pair_pointer + 1 );
		pair_pointer += 2;

		shape0->getBulletTriangle ( m_triface0, tri0 );
		shape1->getBulletTriangle ( m_triface1, tri1 );

		//collide two convex shapes

		if ( tri0.overlap_test_conservative ( tri1 ) )
		{
			convex_vs_convex_collision ( body0Wrap, body1Wrap, &tri0, &tri1 );
		}
	}

	shape0->unlockChildShapes();

	shape1->unlockChildShapes();
}

void GImpactCollisionAlgorithm::collide_sat_triangles ( const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const GImpactMeshShapePart* shape0,
		const GImpactMeshShapePart* shape1,
		i32k* pairs, i32 pair_count )
{
	Transform2 orgtrans0 = body0Wrap->getWorldTransform();
	Transform2 orgtrans1 = body1Wrap->getWorldTransform();

	PrimitiveTriangle ptri0;
	PrimitiveTriangle ptri1;
	GIM_TRIANGLE_CONTACT contact_data;

	shape0->lockChildShapes();
	shape1->lockChildShapes();

	i32k* pair_pointer = pairs;

	while ( pair_count-- )
	{
		m_triface0 = * ( pair_pointer );
		m_triface1 = * ( pair_pointer + 1 );
		pair_pointer += 2;

		shape0->getPrimitiveTriangle ( m_triface0, ptri0 );
		shape1->getPrimitiveTriangle ( m_triface1, ptri1 );

#ifdef TRI_COLLISION_PROFILING
		drx3d_begin_gim02_tri_time();
#endif

		ptri0.applyTransform2 ( orgtrans0 );
		ptri1.applyTransform2 ( orgtrans1 );

		//build planes
		ptri0.buildTriPlane();
		ptri1.buildTriPlane();
		// test conservative

		if ( ptri0.overlap_test_conservative ( ptri1 ) )
		{
			if ( ptri0.find_triangle_collision_clip_method ( ptri1, contact_data ) )
			{
				i32 j = contact_data.m_point_count;

				while ( j-- )
				{
					addContactPoint ( body0Wrap, body1Wrap,
									  contact_data.m_points[j],
									  contact_data.m_separating_normal,
									  -contact_data.m_penetration_depth );
				}
			}
		}

#ifdef TRI_COLLISION_PROFILING
		drx3d_end_gim02_tri_time();

#endif
	}

	shape0->unlockChildShapes();

	shape1->unlockChildShapes();
}

void GImpactCollisionAlgorithm::gimpact_vs_gimpact (
	const CollisionObject2Wrapper* body0Wrap,
	const CollisionObject2Wrapper* body1Wrap,
	const GImpactShapeInterface* shape0,
	const GImpactShapeInterface* shape1 )
{
	if ( shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE )
	{
		const GImpactMeshShape* meshshape0 = static_cast<const GImpactMeshShape*> ( shape0 );
		m_part0 = meshshape0->getMeshPartCount();

		while ( m_part0-- )
		{
			gimpact_vs_gimpact ( body0Wrap, body1Wrap, meshshape0->getMeshPart ( m_part0 ), shape1 );
		}

		return;
	}

	if ( shape1->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE )
	{
		const GImpactMeshShape* meshshape1 = static_cast<const GImpactMeshShape*> ( shape1 );
		m_part1 = meshshape1->getMeshPartCount();

		while ( m_part1-- )
		{
			gimpact_vs_gimpact ( body0Wrap, body1Wrap, shape0, meshshape1->getMeshPart ( m_part1 ) );
		}

		return;
	}

	Transform2 orgtrans0 = body0Wrap->getWorldTransform();

	Transform2 orgtrans1 = body1Wrap->getWorldTransform();

	PairSet pairset;

	gimpact_vs_gimpact_find_pairs ( orgtrans0, orgtrans1, shape0, shape1, pairset );

	if ( pairset.size() == 0 )
		return;

	if ( shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
		 shape1->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART )
	{
		const GImpactMeshShapePart* shapepart0 = static_cast<const GImpactMeshShapePart*> ( shape0 );
		const GImpactMeshShapePart* shapepart1 = static_cast<const GImpactMeshShapePart*> ( shape1 );
//specialized function
#ifdef DRX3D_TRIANGLE_COLLISION
		collide_gjk_triangles ( body0Wrap, body1Wrap, shapepart0, shapepart1, &pairset[0].m_index1, pairset.size() );
#else
		collide_sat_triangles ( body0Wrap, body1Wrap, shapepart0, shapepart1, &pairset[0].m_index1, pairset.size() );
#endif

		return;
	}

	//general function

	shape0->lockChildShapes();

	shape1->lockChildShapes();

	GIM_ShapeRetriever retriever0 ( shape0 );

	GIM_ShapeRetriever retriever1 ( shape1 );

	bool child_has_transform0 = shape0->childrenHasTransform2();

	bool child_has_transform1 = shape1->childrenHasTransform2();

	i32 i = pairset.size();

	while ( i-- )
	{
		GIM_PAIR* pair = &pairset[i];
		m_triface0 = pair->m_index1;
		m_triface1 = pair->m_index2;
		const CollisionShape* colshape0 = retriever0.getChildShape ( m_triface0 );
		const CollisionShape* colshape1 = retriever1.getChildShape ( m_triface1 );

		Transform2 tr0 = body0Wrap->getWorldTransform();
		Transform2 tr1 = body1Wrap->getWorldTransform();

		if ( child_has_transform0 )
		{
			tr0 = orgtrans0 * shape0->getChildTransform ( m_triface0 );
		}

		if ( child_has_transform1 )
		{
			tr1 = orgtrans1 * shape1->getChildTransform ( m_triface1 );
		}

		CollisionObject2Wrapper ob0 ( body0Wrap, colshape0, body0Wrap->getCollisionObject(), tr0, m_part0, m_triface0 );

		CollisionObject2Wrapper ob1 ( body1Wrap, colshape1, body1Wrap->getCollisionObject(), tr1, m_part1, m_triface1 );

		//collide two convex shapes
		convex_vs_convex_collision ( &ob0, &ob1, colshape0, colshape1 );
	}

	shape0->unlockChildShapes();

	shape1->unlockChildShapes();
}

void GImpactCollisionAlgorithm::gimpact_vs_shape ( const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const GImpactShapeInterface* shape0,
		const CollisionShape* shape1, bool swapped )
{
	if ( shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE )
	{
		const GImpactMeshShape* meshshape0 = static_cast<const GImpactMeshShape*> ( shape0 );
		i32& part = swapped ? m_part1 : m_part0;
		part = meshshape0->getMeshPartCount();

		while ( part-- )
		{
			gimpact_vs_shape ( body0Wrap,
							   body1Wrap,
							   meshshape0->getMeshPart ( part ),
							   shape1, swapped );
		}

		return;
	}

#ifdef GIMPACT_VS_PLANE_COLLISION
	if ( shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
		 shape1->getShapeType() == STATIC_PLANE_PROXYTYPE )
	{
		const GImpactMeshShapePart* shapepart = static_cast<const GImpactMeshShapePart*> ( shape0 );
		const StaticPlaneShape* planeshape = static_cast<const StaticPlaneShape*> ( shape1 );
		gimpacttrimeshpart_vs_plane_collision ( body0Wrap, body1Wrap, shapepart, planeshape, swapped );
		return;
	}

#endif

	if ( shape1->isCompound() )
	{
		const CompoundShape* compoundshape = static_cast<const CompoundShape*> ( shape1 );
		gimpact_vs_compoundshape ( body0Wrap, body1Wrap, shape0, compoundshape, swapped );
		return;
	}

	else
		if ( shape1->isConcave() )
		{
			const ConcaveShape* concaveshape = static_cast<const ConcaveShape*> ( shape1 );
			gimpact_vs_concave ( body0Wrap, body1Wrap, shape0, concaveshape, swapped );
			return;
		}

	Transform2 orgtrans0 = body0Wrap->getWorldTransform();

	Transform2 orgtrans1 = body1Wrap->getWorldTransform();

	AlignedObjectArray<i32> collided_results;

	gimpact_vs_shape_find_pairs ( orgtrans0, orgtrans1, shape0, shape1, collided_results );

	if ( collided_results.size() == 0 )
		return;

	shape0->lockChildShapes();

	GIM_ShapeRetriever retriever0 ( shape0 );

	bool child_has_transform0 = shape0->childrenHasTransform2();

	i32 i = collided_results.size();

	while ( i-- )
	{
		i32 child_index = collided_results[i];

		if ( swapped )
			m_triface1 = child_index;
		else
			m_triface0 = child_index;

		const CollisionShape* colshape0 = retriever0.getChildShape ( child_index );

		Transform2 tr0 = body0Wrap->getWorldTransform();

		if ( child_has_transform0 )
		{
			tr0 = orgtrans0 * shape0->getChildTransform ( child_index );
		}

		CollisionObject2Wrapper ob0 ( body0Wrap, colshape0, body0Wrap->getCollisionObject(), body0Wrap->getWorldTransform(), m_part0, m_triface0 );

		const CollisionObject2Wrapper* prevObj;

		if ( m_resultOut->getBody0Wrap()->getCollisionObject() == ob0.getCollisionObject() )
		{
			prevObj = m_resultOut->getBody0Wrap();
			m_resultOut->setBody0Wrap ( &ob0 );
		}

		else
		{
			prevObj = m_resultOut->getBody1Wrap();
			m_resultOut->setBody1Wrap ( &ob0 );
		}

		//collide two shapes

		if ( swapped )
		{
			shape_vs_shape_collision ( body1Wrap, &ob0, shape1, colshape0 );
		}

		else
		{
			shape_vs_shape_collision ( &ob0, body1Wrap, colshape0, shape1 );
		}

		if ( m_resultOut->getBody0Wrap()->getCollisionObject() == ob0.getCollisionObject() )
		{
			m_resultOut->setBody0Wrap ( prevObj );
		}

		else
		{
			m_resultOut->setBody1Wrap ( prevObj );
		}
	}

	shape0->unlockChildShapes();
}

void GImpactCollisionAlgorithm::gimpact_vs_compoundshape ( const CollisionObject2Wrapper* body0Wrap,
		const CollisionObject2Wrapper* body1Wrap,
		const GImpactShapeInterface* shape0,
		const CompoundShape* shape1, bool swapped )
{
	Transform2 orgtrans1 = body1Wrap->getWorldTransform();

	i32 i = shape1->getNumChildShapes();

	while ( i-- )
	{
		const CollisionShape* colshape1 = shape1->getChildShape ( i );
		Transform2 childtrans1 = orgtrans1 * shape1->getChildTransform ( i );

		CollisionObject2Wrapper ob1 ( body1Wrap, colshape1, body1Wrap->getCollisionObject(), childtrans1, -1, i );

		const CollisionObject2Wrapper* tmp = 0;

		if ( m_resultOut->getBody0Wrap()->getCollisionObject() == ob1.getCollisionObject() )
		{
			tmp = m_resultOut->getBody0Wrap();
			m_resultOut->setBody0Wrap ( &ob1 );
		}

		else
		{
			tmp = m_resultOut->getBody1Wrap();
			m_resultOut->setBody1Wrap ( &ob1 );
		}

		//collide child shape
		gimpact_vs_shape ( body0Wrap, &ob1,
						   shape0, colshape1, swapped );

		if ( m_resultOut->getBody0Wrap()->getCollisionObject() == ob1.getCollisionObject() )
		{
			m_resultOut->setBody0Wrap ( tmp );
		}

		else
		{
			m_resultOut->setBody1Wrap ( tmp );
		}
	}
}

void GImpactCollisionAlgorithm::gimpacttrimeshpart_vs_plane_collision (
	const CollisionObject2Wrapper* body0Wrap,
	const CollisionObject2Wrapper* body1Wrap,
	const GImpactMeshShapePart* shape0,
	const StaticPlaneShape* shape1, bool swapped )
{
	Transform2 orgtrans0 = body0Wrap->getWorldTransform();
	Transform2 orgtrans1 = body1Wrap->getWorldTransform();

	const PlaneShape* planeshape = static_cast<const PlaneShape*> ( shape1 );
	Vec4 plane;
	planeshape->get_plane_equation_transformed ( orgtrans1, plane );

	//test box against plane

	AABB tribox;
	shape0->getAabb ( orgtrans0, tribox.m_min, tribox.m_max );
	tribox.increment_margin ( planeshape->getMargin() );

	if ( tribox.plane_classify ( plane ) != DRX3D_CONST_COLLIDE_PLANE )
		return;

	shape0->lockChildShapes();

	Scalar margin = shape0->getMargin() + planeshape->getMargin();

	Vec3 vertex;

	i32 vi = shape0->getVertexCount();

	while ( vi-- )
	{
		shape0->getVertex ( vi, vertex );
		vertex = orgtrans0 ( vertex );

		Scalar distance = vertex.dot ( plane ) - plane[3] - margin;

		if ( distance < 0.0 )  //add contact
		{
			if ( swapped )
			{
				addContactPoint ( body1Wrap, body0Wrap,
								  vertex,
								  -plane,
								  distance );
			}

			else
			{
				addContactPoint ( body0Wrap, body1Wrap,
								  vertex,
								  plane,
								  distance );
			}
		}
	}

	shape0->unlockChildShapes();
}

class GImpactTriangleCallback : public TriangleCallback
{

	public:
		GImpactCollisionAlgorithm* algorithm;
		const CollisionObject2Wrapper* body0Wrap;
		const CollisionObject2Wrapper* body1Wrap;
		const GImpactShapeInterface* gimpactshape0;
		bool swapped;
		Scalar margin;

		virtual void processTriangle ( Vec3* triangle, i32 partId, i32 triangleIndex )
		{
			TriangleShapeEx tri1 ( triangle[0], triangle[1], triangle[2] );
			tri1.setMargin ( margin );

			if ( swapped )
			{
				algorithm->setPart0 ( partId );
				algorithm->setFace0 ( triangleIndex );
			}

			else
			{
				algorithm->setPart1 ( partId );
				algorithm->setFace1 ( triangleIndex );
			}

			CollisionObject2Wrapper ob1Wrap ( body1Wrap, &tri1, body1Wrap->getCollisionObject(), body1Wrap->getWorldTransform(), partId, triangleIndex );

			const CollisionObject2Wrapper* tmp = 0;

			if ( algorithm->internalGetResultOut()->getBody0Wrap()->getCollisionObject() == ob1Wrap.getCollisionObject() )
			{
				tmp = algorithm->internalGetResultOut()->getBody0Wrap();
				algorithm->internalGetResultOut()->setBody0Wrap ( &ob1Wrap );
			}

			else
			{
				tmp = algorithm->internalGetResultOut()->getBody1Wrap();
				algorithm->internalGetResultOut()->setBody1Wrap ( &ob1Wrap );
			}

			algorithm->gimpact_vs_shape (

				body0Wrap, &ob1Wrap, gimpactshape0, &tri1, swapped );

			if ( algorithm->internalGetResultOut()->getBody0Wrap()->getCollisionObject() == ob1Wrap.getCollisionObject() )
			{
				algorithm->internalGetResultOut()->setBody0Wrap ( tmp );
			}

			else
			{
				algorithm->internalGetResultOut()->setBody1Wrap ( tmp );
			}
		}
};

void GImpactCollisionAlgorithm::gimpact_vs_concave (
	const CollisionObject2Wrapper* body0Wrap,
	const CollisionObject2Wrapper* body1Wrap,
	const GImpactShapeInterface* shape0,
	const ConcaveShape* shape1, bool swapped )
{
	//create the callback
	GImpactTriangleCallback tricallback;
	tricallback.algorithm = this;
	tricallback.body0Wrap = body0Wrap;
	tricallback.body1Wrap = body1Wrap;
	tricallback.gimpactshape0 = shape0;
	tricallback.swapped = swapped;
	tricallback.margin = shape1->getMargin();

	//getting the trimesh AABB
	Transform2 gimpactInConcaveSpace;

	gimpactInConcaveSpace = body1Wrap->getWorldTransform().inverse() * body0Wrap->getWorldTransform();

	Vec3 minAABB, maxAABB;
	shape0->getAabb ( gimpactInConcaveSpace, minAABB, maxAABB );

	shape1->processAllTriangles ( &tricallback, minAABB, maxAABB );
}

void GImpactCollisionAlgorithm::processCollision ( const CollisionObject2Wrapper* body0Wrap, const CollisionObject2Wrapper* body1Wrap, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut )
{
	clearCache();

	m_resultOut = resultOut;
	m_dispatchInfo = &dispatchInfo;
	const GImpactShapeInterface* gimpactshape0;
	const GImpactShapeInterface* gimpactshape1;

	if ( body0Wrap->getCollisionShape()->getShapeType() == GIMPACT_SHAPE_PROXYTYPE )
	{
		gimpactshape0 = static_cast<const GImpactShapeInterface*> ( body0Wrap->getCollisionShape() );

		if ( body1Wrap->getCollisionShape()->getShapeType() == GIMPACT_SHAPE_PROXYTYPE )
		{
			gimpactshape1 = static_cast<const GImpactShapeInterface*> ( body1Wrap->getCollisionShape() );

			gimpact_vs_gimpact ( body0Wrap, body1Wrap, gimpactshape0, gimpactshape1 );
		}

		else
		{
			gimpact_vs_shape ( body0Wrap, body1Wrap, gimpactshape0, body1Wrap->getCollisionShape(), false );
		}
	}

	else
		if ( body1Wrap->getCollisionShape()->getShapeType() == GIMPACT_SHAPE_PROXYTYPE )
		{
			gimpactshape1 = static_cast<const GImpactShapeInterface*> ( body1Wrap->getCollisionShape() );

			gimpact_vs_shape ( body1Wrap, body0Wrap, gimpactshape1, body0Wrap->getCollisionShape(), true );
		}

	// Ensure that gContactProcessedCallback is called for concave shapes.

	if ( getLastManifold() )
	{
		m_resultOut->refreshContactPoints();
	}
}

Scalar GImpactCollisionAlgorithm::calculateTimeOfImpact ( CollisionObject2* body0, CollisionObject2* body1, const DispatcherInfo& dispatchInfo, ManifoldResult* resultOut )
{
	return 1.f;
}

///////////////////////////////////// REGISTERING ALGORITHM //////////////////////////////////////////////

//! Use this function for DoRegister the algorithm externally
void GImpactCollisionAlgorithm::registerAlgorithm ( CollisionDispatcher* dispatcher )
{
	static GImpactCollisionAlgorithm::CreateFunc s_gimpact_cf;

	i32 i;

	for ( i = 0; i < MAX_BROADPHASE_COLLISION_TYPES; i++ )
	{
		dispatcher->registerCollisionCreateFunc ( GIMPACT_SHAPE_PROXYTYPE, i, &s_gimpact_cf );
	}

	for ( i = 0; i < MAX_BROADPHASE_COLLISION_TYPES; i++ )
	{
		dispatcher->registerCollisionCreateFunc ( i, GIMPACT_SHAPE_PROXYTYPE, &s_gimpact_cf );
	}
}
