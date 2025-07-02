
#include <drxtypes.h>

//Number of random voronoi points to generate for shattering
#define VORONOIPOINTS 100

//maximum number of objects (and allow user to shoot additional boxes)
#define MAX_PROXIES (2048)
#define BREAKING_THRESHOLD 3
#define CONVEX_MARGIN 0.04
static i32 useMpr = 0;

#include "../VoronoiFractureDemo.h"

///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <stdio.h>  //printf debugging

static bool useGenericConstraint = false;

#include "../ConvexConvexMprAlgorithm.h"

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/ConvexHullComputer.h>
#include <drx3D/Maths/Linear/Quat.h>
#include <set>
#include <time.h>

class BroadphaseInterface;
class CollisionShape;
class OverlappingPairCache;
class CollisionDispatcher;
class ConstraintSolver;
struct CollisionAlgorithmCreateFunc;
class DefaultCollisionConfiguration;

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class VoronoiFractureDemo : public CommonRigidBodyBase
{
	//keep the collision shapes, for deletion/cleanup
	AlignedObjectArray<CollisionShape*> m_collisionShapes;

	BroadphaseInterface* m_broadphase;

	CollisionDispatcher* m_dispatcher;

	ConstraintSolver* m_solver;

	DefaultCollisionConfiguration* m_collisionConfiguration;

	Clock m_perfmTimer;

public:
	VoronoiFractureDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
		srand((unsigned)time(NULL));  // Seed it...
	}
	virtual ~VoronoiFractureDemo()
	{
		Assert(m_dynamicsWorld == 0);
	}

	void initPhysics();

	void exitPhysics();

	//virtual void renderme();

	void getVerticesInsidePlanes(const AlignedObjectArray<Vec3>& planes, AlignedObjectArray<Vec3>& verticesOut, std::set<i32>& planeIndicesOut);
	void voronoiBBShatter(const AlignedObjectArray<Vec3>& points, const Vec3& bbmin, const Vec3& bbmax, const Quat& bbq, const Vec3& bbt, Scalar matDensity);
	void voronoiConvexHullShatter(const AlignedObjectArray<Vec3>& points, const AlignedObjectArray<Vec3>& verts, const Quat& bbq, const Vec3& bbt, Scalar matDensity);

	//virtual void clientMoveAndDisplay();

	//virtual void displayCallback();
	//virtual void	clientResetScene();

	//virtual void keyboardCallback(u8 key, i32 x, i32 y);

	void attachFixedConstraints();

	virtual void resetCamera()
	{
		float dist = 18;
		float pitch = -30;
		float yaw = 129;
		float targetPos[3] = {-1.5, 4.7, -2};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

void VoronoiFractureDemo::attachFixedConstraints()
{
	AlignedObjectArray<RigidBody*> bodies;

	i32 numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();

	for (i32 i = 0; i < numManifolds; i++)
	{
		PersistentManifold* manifold = m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		if (!manifold->getNumContacts())
			continue;

		Scalar minDist = 1e30f;
		i32 minIndex = -1;
		for (i32 v = 0; v < manifold->getNumContacts(); v++)
		{
			if (minDist > manifold->getContactPoint(v).getDistance())
			{
				minDist = manifold->getContactPoint(v).getDistance();
				minIndex = v;
			}
		}
		if (minDist > 0.)
			continue;

		CollisionObject2* colObj0 = (CollisionObject2*)manifold->getBody0();
		CollisionObject2* colObj1 = (CollisionObject2*)manifold->getBody1();
		//	i32 tag0 = (colObj0)->getIslandTag();
		//		i32 tag1 = (colObj1)->getIslandTag();
		RigidBody* body0 = RigidBody::upcast(colObj0);
		RigidBody* body1 = RigidBody::upcast(colObj1);
		if (bodies.findLinearSearch(body0) == bodies.size())
			bodies.push_back(body0);
		if (bodies.findLinearSearch(body1) == bodies.size())
			bodies.push_back(body1);

		if (body0 && body1)
		{
			if (!colObj0->isStaticOrKinematicObject() && !colObj1->isStaticOrKinematicObject())
			{
				if (body0->checkCollideWithOverride(body1))
				{
					{
						Transform2 trA, trB;
						trA.setIdentity();
						trB.setIdentity();
						Vec3 contactPosWorld = manifold->getContactPoint(minIndex).m_positionWorldOnA;
						Transform2 globalFrame;
						globalFrame.setIdentity();
						globalFrame.setOrigin(contactPosWorld);

						trA = body0->getWorldTransform().inverse() * globalFrame;
						trB = body1->getWorldTransform().inverse() * globalFrame;
						float totalMass = 1.f / body0->getInvMass() + 1.f / body1->getInvMass();

						if (useGenericConstraint)
						{
							Generic6DofConstraint* dof6 = new Generic6DofConstraint(*body0, *body1, trA, trB, true);
							dof6->setOverrideNumSolverIterations(30);

							dof6->setBreakingImpulseThreshold(BREAKING_THRESHOLD * totalMass);

							for (i32 i = 0; i < 6; i++)
								dof6->setLimit(i, 0, 0);
							m_dynamicsWorld->addConstraint(dof6, true);
						}
						else
						{
							FixedConstraint* fixed = new FixedConstraint(*body0, *body1, trA, trB);
							fixed->setBreakingImpulseThreshold(BREAKING_THRESHOLD * totalMass);
							fixed->setOverrideNumSolverIterations(30);
							m_dynamicsWorld->addConstraint(fixed, true);
						}
					}
				}
			}
		}
	}

	for (i32 i = 0; i < bodies.size(); i++)
	{
		m_dynamicsWorld->removeRigidBody(bodies[i]);
		m_dynamicsWorld->addRigidBody(bodies[i]);
	}
}
/*
void VoronoiFractureDemo::keyboardCallback(u8 key, i32 x, i32 y)
{
	if (key == 'g')
	{
		attachFixedConstraints();
	}else
	{
		PlatformDemoApplication::keyboardCallback(key,x,y);
	}
}
*/

void VoronoiFractureDemo::getVerticesInsidePlanes(const AlignedObjectArray<Vec3>& planes, AlignedObjectArray<Vec3>& verticesOut, std::set<i32>& planeIndicesOut)
{
	// Based on btGeometryUtil.cpp (Gino van den Bergen / Erwin Coumans)
	verticesOut.resize(0);
	planeIndicesOut.clear();
	i32k numPlanes = planes.size();
	i32 i, j, k, l;
	for (i = 0; i < numPlanes; i++)
	{
		const Vec3& N1 = planes[i];
		for (j = i + 1; j < numPlanes; j++)
		{
			const Vec3& N2 = planes[j];
			Vec3 n1n2 = N1.cross(N2);
			if (n1n2.length2() > Scalar(0.0001))
			{
				for (k = j + 1; k < numPlanes; k++)
				{
					const Vec3& N3 = planes[k];
					Vec3 n2n3 = N2.cross(N3);
					Vec3 n3n1 = N3.cross(N1);
					if ((n2n3.length2() > Scalar(0.0001)) && (n3n1.length2() > Scalar(0.0001)))
					{
						Scalar quotient = (N1.dot(n2n3));
						if (Fabs(quotient) > Scalar(0.0001))
						{
							Vec3 potentialVertex = (n2n3 * N1[3] + n3n1 * N2[3] + n1n2 * N3[3]) * (Scalar(-1.) / quotient);
							for (l = 0; l < numPlanes; l++)
							{
								const Vec3& NP = planes[l];
								if (Scalar(NP.dot(potentialVertex)) + Scalar(NP[3]) > Scalar(0.000001))
									break;
							}
							if (l == numPlanes)
							{
								// vertex (three plane intersection) inside all planes
								verticesOut.push_back(potentialVertex);
								planeIndicesOut.insert(i);
								planeIndicesOut.insert(j);
								planeIndicesOut.insert(k);
							}
						}
					}
				}
			}
		}
	}
}

static Vec3 curVoronoiPoint;

struct pointCmp
{
	bool operator()(const Vec3& p1, const Vec3& p2) const
	{
		float v1 = (p1 - curVoronoiPoint).length2();
		float v2 = (p2 - curVoronoiPoint).length2();
		bool result0 = v1 < v2;
		//bool result1 = ((Scalar)(p1-curVoronoiPoint).length2()) < ((Scalar)(p2-curVoronoiPoint).length2());
		//apparently result0 is not always result1, because extended precision used in registered is different from precision when values are stored in memory
		return result0;
	}
};

void VoronoiFractureDemo::voronoiBBShatter(const AlignedObjectArray<Vec3>& points, const Vec3& bbmin, const Vec3& bbmax, const Quat& bbq, const Vec3& bbt, Scalar matDensity)
{
	// points define voronoi cells in world space (avoid duplicates)
	// bbmin & bbmax = bounding box min and max in local space
	// bbq & bbt = bounding box quaternion rotation and translation
	// matDensity = Material density for voronoi shard mass calculation
	Vec3 bbvx = quatRotate(bbq, Vec3(1.0, 0.0, 0.0));
	Vec3 bbvy = quatRotate(bbq, Vec3(0.0, 1.0, 0.0));
	Vec3 bbvz = quatRotate(bbq, Vec3(0.0, 0.0, 1.0));
	Quat bbiq = bbq.inverse();
	ConvexHullComputer* convexHC = new ConvexHullComputer();
	AlignedObjectArray<Vec3> vertices;
	Vec3 rbb, nrbb;
	Scalar nlength, maxDistance, distance;
	AlignedObjectArray<Vec3> sortedVoronoiPoints;
	sortedVoronoiPoints.copyFromArray(points);
	Vec3 normal, plane;
	AlignedObjectArray<Vec3> planes;
	std::set<i32> planeIndices;
	std::set<i32>::iterator planeIndicesIter;
	i32 numplaneIndices;
	i32 cellnum = 0;
	i32 i, j, k;

	i32 numpoints = points.size();
	for (i = 0; i < numpoints; i++)
	{
		curVoronoiPoint = points[i];
		Vec3 icp = quatRotate(bbiq, curVoronoiPoint - bbt);
		rbb = icp - bbmax;
		nrbb = bbmin - icp;
		planes.resize(6);
		planes[0] = bbvx;
		planes[0][3] = rbb.x();
		planes[1] = bbvy;
		planes[1][3] = rbb.y();
		planes[2] = bbvz;
		planes[2][3] = rbb.z();
		planes[3] = -bbvx;
		planes[3][3] = nrbb.x();
		planes[4] = -bbvy;
		planes[4][3] = nrbb.y();
		planes[5] = -bbvz;
		planes[5][3] = nrbb.z();
		maxDistance = SIMD_INFINITY;
		sortedVoronoiPoints.heapSort(pointCmp());
		for (j = 1; j < numpoints; j++)
		{
			normal = sortedVoronoiPoints[j] - curVoronoiPoint;
			nlength = normal.length();
			if (nlength > maxDistance)
				break;
			plane = normal.normalized();
			plane[3] = -nlength / Scalar(2.);
			planes.push_back(plane);
			getVerticesInsidePlanes(planes, vertices, planeIndices);
			if (vertices.size() == 0)
				break;
			numplaneIndices = planeIndices.size();
			if (numplaneIndices != planes.size())
			{
				planeIndicesIter = planeIndices.begin();
				for (k = 0; k < numplaneIndices; k++)
				{
					if (k != *planeIndicesIter)
						planes[k] = planes[*planeIndicesIter];
					planeIndicesIter++;
				}
				planes.resize(numplaneIndices);
			}
			maxDistance = vertices[0].length();
			for (k = 1; k < vertices.size(); k++)
			{
				distance = vertices[k].length();
				if (maxDistance < distance)
					maxDistance = distance;
			}
			maxDistance *= Scalar(2.);
		}
		if (vertices.size() == 0)
			continue;

		// Clean-up voronoi convex shard vertices and generate edges & faces
		convexHC->compute(&vertices[0].getX(), sizeof(Vec3), vertices.size(), CONVEX_MARGIN, 0.0);

		// At this point we have a complete 3D voronoi shard mesh contained in convexHC

		// Calculate volume and center of mass (Stan Melax volume integration)
		i32 numFaces = convexHC->faces.size();
		i32 v0, v1, v2;  // Triangle vertices
		Scalar volume = Scalar(0.);
		Vec3 com(0., 0., 0.);
		for (j = 0; j < numFaces; j++)
		{
			const ConvexHullComputer::Edge* edge = &convexHC->edges[convexHC->faces[j]];
			v0 = edge->getSourceVertex();
			v1 = edge->getTargetVertex();
			edge = edge->getNextEdgeOfFace();
			v2 = edge->getTargetVertex();
			while (v2 != v0)
			{
				// Counter-clockwise triangulated voronoi shard mesh faces (v0-v1-v2) and edges here...
				Scalar vol = convexHC->vertices[v0].triple(convexHC->vertices[v1], convexHC->vertices[v2]);
				volume += vol;
				com += vol * (convexHC->vertices[v0] + convexHC->vertices[v1] + convexHC->vertices[v2]);
				edge = edge->getNextEdgeOfFace();
				v1 = v2;
				v2 = edge->getTargetVertex();
			}
		}
		com /= volume * Scalar(4.);
		volume /= Scalar(6.);

		// Shift all vertices relative to center of mass
		i32 numVerts = convexHC->vertices.size();
		for (j = 0; j < numVerts; j++)
		{
			convexHC->vertices[j] -= com;
		}

		// Note:
		// At this point convex hulls contained in convexHC should be accurate (line up flush with other pieces, no cracks),
		// ...however drx3D Physics rigid bodies demo visualizations appear to produce some visible cracks.
		// Use the mesh in convexHC for visual display or to perform boolean operations with.

		// Create drx3D Physics rigid body shards
		CollisionShape* shardShape = new ConvexHullShape(&(convexHC->vertices[0].getX()), convexHC->vertices.size());
		shardShape->setMargin(CONVEX_MARGIN);  // for this demo; note convexHC has optional margin parameter for this
		m_collisionShapes.push_back(shardShape);
		Transform2 shardTransform;
		shardTransform.setIdentity();
		shardTransform.setOrigin(curVoronoiPoint + com);  // Shard's adjusted location
		DefaultMotionState* shardMotionState = new DefaultMotionState(shardTransform);
		Scalar shardMass(volume * matDensity);
		Vec3 shardInertia(0., 0., 0.);
		shardShape->calculateLocalInertia(shardMass, shardInertia);
		RigidBody::RigidBodyConstructionInfo shardRBInfo(shardMass, shardMotionState, shardShape, shardInertia);
		RigidBody* shardBody = new RigidBody(shardRBInfo);
		m_dynamicsWorld->addRigidBody(shardBody);

		cellnum++;
	}
	printf("Generated %d voronoi RigidBody shards\n", cellnum);
}

void VoronoiFractureDemo::voronoiConvexHullShatter(const AlignedObjectArray<Vec3>& points, const AlignedObjectArray<Vec3>& verts, const Quat& bbq, const Vec3& bbt, Scalar matDensity)
{
	// points define voronoi cells in world space (avoid duplicates)
	// verts = source (convex hull) mesh vertices in local space
	// bbq & bbt = source (convex hull) mesh quaternion rotation and translation
	// matDensity = Material density for voronoi shard mass calculation
	ConvexHullComputer* convexHC = new ConvexHullComputer();
	AlignedObjectArray<Vec3> vertices, chverts;
	Vec3 rbb, nrbb;
	Scalar nlength, maxDistance, distance;
	AlignedObjectArray<Vec3> sortedVoronoiPoints;
	sortedVoronoiPoints.copyFromArray(points);
	Vec3 normal, plane;
	AlignedObjectArray<Vec3> planes, convexPlanes;
	std::set<i32> planeIndices;
	std::set<i32>::iterator planeIndicesIter;
	i32 numplaneIndices;
	i32 cellnum = 0;
	i32 i, j, k;

	// Convert verts to world space and get convexPlanes
	i32 numverts = verts.size();
	chverts.resize(verts.size());
	for (i = 0; i < numverts; i++)
	{
		chverts[i] = quatRotate(bbq, verts[i]) + bbt;
	}
	//GeometryUtil::getPlaneEquationsFromVertices(chverts, convexPlanes);
	// Using convexHullComputer faster than getPlaneEquationsFromVertices for large meshes...
	convexHC->compute(&chverts[0].getX(), sizeof(Vec3), numverts, 0.0, 0.0);
	i32 numFaces = convexHC->faces.size();
	i32 v0, v1, v2;  // vertices
	for (i = 0; i < numFaces; i++)
	{
		const ConvexHullComputer::Edge* edge = &convexHC->edges[convexHC->faces[i]];
		v0 = edge->getSourceVertex();
		v1 = edge->getTargetVertex();
		edge = edge->getNextEdgeOfFace();
		v2 = edge->getTargetVertex();
		plane = (convexHC->vertices[v1] - convexHC->vertices[v0]).cross(convexHC->vertices[v2] - convexHC->vertices[v0]).normalize();
		plane[3] = -plane.dot(convexHC->vertices[v0]);
		convexPlanes.push_back(plane);
	}
	i32k numconvexPlanes = convexPlanes.size();

	i32 numpoints = points.size();
	for (i = 0; i < numpoints; i++)
	{
		curVoronoiPoint = points[i];
		planes.copyFromArray(convexPlanes);
		for (j = 0; j < numconvexPlanes; j++)
		{
			planes[j][3] += planes[j].dot(curVoronoiPoint);
		}
		maxDistance = SIMD_INFINITY;
		sortedVoronoiPoints.heapSort(pointCmp());
		for (j = 1; j < numpoints; j++)
		{
			normal = sortedVoronoiPoints[j] - curVoronoiPoint;
			nlength = normal.length();
			if (nlength > maxDistance)
				break;
			plane = normal.normalized();
			plane[3] = -nlength / Scalar(2.);
			planes.push_back(plane);
			getVerticesInsidePlanes(planes, vertices, planeIndices);
			if (vertices.size() == 0)
				break;
			numplaneIndices = planeIndices.size();
			if (numplaneIndices != planes.size())
			{
				planeIndicesIter = planeIndices.begin();
				for (k = 0; k < numplaneIndices; k++)
				{
					if (k != *planeIndicesIter)
						planes[k] = planes[*planeIndicesIter];
					planeIndicesIter++;
				}
				planes.resize(numplaneIndices);
			}
			maxDistance = vertices[0].length();
			for (k = 1; k < vertices.size(); k++)
			{
				distance = vertices[k].length();
				if (maxDistance < distance)
					maxDistance = distance;
			}
			maxDistance *= Scalar(2.);
		}
		if (vertices.size() == 0)
			continue;

		// Clean-up voronoi convex shard vertices and generate edges & faces
		convexHC->compute(&vertices[0].getX(), sizeof(Vec3), vertices.size(), 0.0, 0.0);

		// At this point we have a complete 3D voronoi shard mesh contained in convexHC

		// Calculate volume and center of mass (Stan Melax volume integration)
		numFaces = convexHC->faces.size();
		Scalar volume = Scalar(0.);
		Vec3 com(0., 0., 0.);
		for (j = 0; j < numFaces; j++)
		{
			const ConvexHullComputer::Edge* edge = &convexHC->edges[convexHC->faces[j]];
			v0 = edge->getSourceVertex();
			v1 = edge->getTargetVertex();
			edge = edge->getNextEdgeOfFace();
			v2 = edge->getTargetVertex();
			while (v2 != v0)
			{
				// Counter-clockwise triangulated voronoi shard mesh faces (v0-v1-v2) and edges here...
				Scalar vol = convexHC->vertices[v0].triple(convexHC->vertices[v1], convexHC->vertices[v2]);
				volume += vol;
				com += vol * (convexHC->vertices[v0] + convexHC->vertices[v1] + convexHC->vertices[v2]);
				edge = edge->getNextEdgeOfFace();
				v1 = v2;
				v2 = edge->getTargetVertex();
			}
		}
		com /= volume * Scalar(4.);
		volume /= Scalar(6.);

		// Shift all vertices relative to center of mass
		i32 numVerts = convexHC->vertices.size();
		for (j = 0; j < numVerts; j++)
		{
			convexHC->vertices[j] -= com;
		}

		// Note:
		// At this point convex hulls contained in convexHC should be accurate (line up flush with other pieces, no cracks),
		// ...however drx3D Physics rigid bodies demo visualizations appear to produce some visible cracks.
		// Use the mesh in convexHC for visual display or to perform boolean operations with.

		// Create drx3D Physics rigid body shards
		CollisionShape* shardShape = new ConvexHullShape(&(convexHC->vertices[0].getX()), convexHC->vertices.size());
		shardShape->setMargin(CONVEX_MARGIN);  // for this demo; note convexHC has optional margin parameter for this
		m_collisionShapes.push_back(shardShape);
		Transform2 shardTransform;
		shardTransform.setIdentity();
		shardTransform.setOrigin(curVoronoiPoint + com);  // Shard's adjusted location
		DefaultMotionState* shardMotionState = new DefaultMotionState(shardTransform);
		Scalar shardMass(volume * matDensity);
		Vec3 shardInertia(0., 0., 0.);
		shardShape->calculateLocalInertia(shardMass, shardInertia);
		RigidBody::RigidBodyConstructionInfo shardRBInfo(shardMass, shardMotionState, shardShape, shardInertia);
		RigidBody* shardBody = new RigidBody(shardRBInfo);
		m_dynamicsWorld->addRigidBody(shardBody);

		cellnum++;
	}
	printf("Generated %d voronoi RigidBody shards\n", cellnum);
}

/*
void VoronoiFractureDemo::clientMoveAndDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	//simple dynamics world doesn't handle fixed-time-stepping
	float ms = getDeltaTimeMicroseconds();
	
	///step the simulation
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(1. / 60., 0);// ms / 1000000.f);
		//optional but useful: debug drawing
		m_dynamicsWorld->debugDrawWorld();
	}
		
	renderme(); 
	
	glFlush();

	swapBuffers();
}
*/
/*
void VoronoiFractureDemo::displayCallback(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	
	renderme();

	//optional but useful: debug drawing to detect problems
	if (m_dynamicsWorld)
		m_dynamicsWorld->debugDrawWorld();

	glFlush();
	swapBuffers();
}
*/
/*
void VoronoiFractureDemo::renderme()
{
	DemoApplication::renderme();
	char buf[124];

	i32 lineWidth = 200;
	i32 xStart = m_glutScreenWidth - lineWidth;

	if (useMpr)
	{
		sprintf(buf, "Using GJK+MPR");
	}
	else
	{
		sprintf(buf, "Using GJK+EPA");
	}
	GLDebugDrawString(xStart, 20, buf);

}
*/

void VoronoiFractureDemo::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	srand(13);
	useGenericConstraint = !useGenericConstraint;
	printf("useGenericConstraint = %d\n", useGenericConstraint);

	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new DefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	useMpr = 1 - useMpr;

	if (useMpr)
	{
		printf("using GJK+MPR convex-convex collision detection\n");
		ConvexConvexMprAlgorithm::CreateFunc* cf = new ConvexConvexMprAlgorithm::CreateFunc;
		m_dispatcher->registerCollisionCreateFunc(CONVEX_HULL_SHAPE_PROXYTYPE, CONVEX_HULL_SHAPE_PROXYTYPE, cf);
		m_dispatcher->registerCollisionCreateFunc(CONVEX_HULL_SHAPE_PROXYTYPE, BOX_SHAPE_PROXYTYPE, cf);
		m_dispatcher->registerCollisionCreateFunc(BOX_SHAPE_PROXYTYPE, CONVEX_HULL_SHAPE_PROXYTYPE, cf);
	}
	else
	{
		printf("using default (GJK+EPA) convex-convex collision detection\n");
	}

	m_broadphase = new DbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	SequentialImpulseConstraintSolver* sol = new SequentialImpulseConstraintSolver;
	m_solver = sol;

	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_dynamicsWorld->getSolverInfo().m_splitImpulse = true;

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));

	///create a few basic rigid bodies
	CollisionShape* groundShape = new BoxShape(Vec3(Scalar(50.), Scalar(50.), Scalar(50.)));
	//	CollisionShape* groundShape = new StaticPlaneShape(Vec3(0,1,0),50);

	m_collisionShapes.push_back(groundShape);

	Transform2 groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 0));

	//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
	{
		Scalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	{
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(10.), Scalar(8.), Scalar(1.)));
		Scalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		Vec3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);
		groundTransform.setOrigin(Vec3(0, 0, 0));
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		DefaultMotionState* myMotionState = new DefaultMotionState(groundTransform);
		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}

	// ==> Voronoi Shatter Basic Demo: Random Cuboid

	// Random size cuboid (defined by bounding box max and min)
	Vec3 bbmax(Scalar(rand() / Scalar(RAND_MAX)) * 12. + 0.5, Scalar(rand() / Scalar(RAND_MAX)) * 1. + 0.5, Scalar(rand() / Scalar(RAND_MAX)) * 1. + 0.5);
	Vec3 bbmin = -bbmax;
	// Place it 10 units above ground
	Vec3 bbt(0, 15, 0);
	// Use an arbitrary material density for shards (should be consitent/relative with/to rest of RBs in world)
	Scalar matDensity = 1;
	// Using random rotation
	Quat bbq(Scalar(rand() / Scalar(RAND_MAX)) * 2. - 1., Scalar(rand() / Scalar(RAND_MAX)) * 2. - 1., Scalar(rand() / Scalar(RAND_MAX)) * 2. - 1., Scalar(rand() / Scalar(RAND_MAX)) * 2. - 1.);
	bbq.normalize();
	// Generate random points for voronoi cells
	AlignedObjectArray<Vec3> points;
	Vec3 point;
	Vec3 diff = bbmax - bbmin;
	for (i32 i = 0; i < VORONOIPOINTS; i++)
	{
		// Place points within box area (points are in world coordinates)
		point = quatRotate(bbq, Vec3(Scalar(rand() / Scalar(RAND_MAX)) * diff.x() - diff.x() / 2., Scalar(rand() / Scalar(RAND_MAX)) * diff.y() - diff.y() / 2., Scalar(rand() / Scalar(RAND_MAX)) * diff.z() - diff.z() / 2.)) + bbt;
		points.push_back(point);
	}
	m_perfmTimer.reset();
	voronoiBBShatter(points, bbmin, bbmax, bbq, bbt, matDensity);
	printf("Total Time: %f seconds\n", m_perfmTimer.getTimeMilliseconds() / 1000.);

	for (i32 i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		obj->getCollisionShape()->setMargin(CONVEX_MARGIN + 0.01);
	}
	m_dynamicsWorld->performDiscreteCollisionDetection();

	for (i32 i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		obj->getCollisionShape()->setMargin(CONVEX_MARGIN);
	}

	attachFixedConstraints();

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void VoronoiFractureDemo::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization

	i32 i;
	//remove all constraints
	for (i = m_dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
	{
		TypedConstraint* constraint = m_dynamicsWorld->getConstraint(i);
		m_dynamicsWorld->removeConstraint(constraint);
		delete constraint;
	}

	//remove the rigidbodies from the dynamics world and delete them

	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		RigidBody* body = RigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (i32 j = 0; j < m_collisionShapes.size(); j++)
	{
		CollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_dynamicsWorld;
	m_dynamicsWorld = 0;

	delete m_solver;
	m_solver = 0;

	delete m_broadphase;
	m_broadphase = 0;

	delete m_dispatcher;
	m_dispatcher = 0;

	delete m_collisionConfiguration;
	m_collisionConfiguration = 0;
}

/*
static DemoApplication* Create()
	{
		VoronoiFractureDemo* demo = new VoronoiFractureDemo;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}

*/

CommonExampleInterface* VoronoiFractureCreateFunc(struct CommonExampleOptions& options)
{
	return new VoronoiFractureDemo(options.m_guiHelper);
}
