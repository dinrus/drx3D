#include "../LoadDeformed.h"
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <stdio.h>  //printf debugging
#include <drx3D/Common/Interfaces/CommonDeformableBodyBase.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>

struct CustomSoftBodyHelper : public SoftBodyHelpers
{
	static STxt loadDeformableState(AlignedObjectArray<Vec3>& qs, AlignedObjectArray<Vec3>& vs, tukk filename, CommonFileIOInterface* fileIO);

};

static inline bool isSpace(const char c)
{
	return (c == ' ') || (c == '\t');
}
static inline bool isNewLine(const char c)
{
	return (c == '\r') || (c == '\n') || (c == '\0');
}
static inline float parseFloat(tukk & token)
{
	token += strspn(token, " \t");
	float f = (float)atof(token);
	token += strcspn(token, " \t\r");
	return f;
}
static inline void parseFloat3(
	float& x, float& y, float& z,
	tukk & token)
{
	x = parseFloat(token);
	y = parseFloat(token);
	z = parseFloat(token);
}


STxt CustomSoftBodyHelper::loadDeformableState(AlignedObjectArray<Vec3>& qs, AlignedObjectArray<Vec3>& vs, tukk filename, CommonFileIOInterface* fileIO)
{
	{
		qs.clear();
		vs.clear();
		STxt tmp = filename;
		std::stringstream err;
#ifdef USE_STREAM
		std::ifstream ifs(filename);
		if (!ifs)
		{
			err << "Cannot open file [" << filename << "]" << std::endl;
			return err.str();
		}
#else
		i32 fileHandle = fileIO->fileOpen(filename, "r");
		if (fileHandle < 0)
		{
			err << "Cannot open file [" << filename << "]" << std::endl;
			return err.str();
		}
#endif

		STxt name;

		i32 maxchars = 8192;              // Alloc enough size.
		std::vector<char> buf(maxchars);  // Alloc enough size.
		STxt linebuf;
		linebuf.reserve(maxchars);

#ifdef USE_STREAM
		while (ifs.peek() != -1)
#else
		tuk line = 0;
		do
#endif
		{
			linebuf.resize(0);
#ifdef USE_STREAM
			safeGetline(ifs, linebuf);
#else
			char tmpBuf[1024];
			line = fileIO->readLine(fileHandle, tmpBuf, 1024);
			if (line)
			{
				linebuf = line;
			}
#endif
			// Trim newline '\r\n' or '\r'
			if (linebuf.size() > 0)
			{
				if (linebuf[linebuf.size() - 1] == '\n') linebuf.erase(linebuf.size() - 1);
			}
			if (linebuf.size() > 0)
			{
				if (linebuf[linebuf.size() - 1] == '\n') linebuf.erase(linebuf.size() - 1);
			}

			// Skip if empty line.
			if (linebuf.empty())
			{
				continue;
			}

			// Skip leading space.
			tukk token = linebuf.c_str();
			token += strspn(token, " \t");

			Assert(token);
			if (token[0] == '\0') continue;  // empty line

			if (token[0] == '#') continue;  // comment line

			// q
			if (token[0] == 'q' && isSpace((token[1])))
			{
				token += 2;
				float x, y, z;
				parseFloat3(x, y, z, token);
				qs.push_back(Vec3(x, y, z));
				continue;
			}

			// v
			if (token[0] == 'v' && isSpace((token[1])))
			{
				token += 3;
				float x, y, z;
				parseFloat3(x, y, z, token);
				vs.push_back(Vec3(x, y, z));
				continue;
			}

			// Ignore unknown command.
		}
#ifndef USE_STREAM
		while (line)
			;
#endif

		if (fileHandle >= 0)
		{
			fileIO->fileClose(fileHandle);
		}
		return err.str();
	}
}


class LoadDeformed : public CommonDeformableBodyBase
{
	i32 steps;
	SoftBody* psb;
	char filename;
	i32 reset_frame;
	float sim_time;

public:
	LoadDeformed(struct GUIHelperInterface* helper)
		: CommonDeformableBodyBase(helper)
	{
		steps = 0;
		psb = nullptr;
		reset_frame = 0;
		sim_time = 0;
	}
	virtual ~LoadDeformed()
	{
	}
	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
		float dist = 2;
		float pitch = -45;
		float yaw = 100;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	void stepSimulation(float deltaTime)
	{
		steps++;
		sim_time += deltaTime;
		////        i32 seconds = 1/deltaTime;
		if (0)
		{
			//        if (reset_frame==0 && steps<100){
			////            printf("steps %d, seconds %d, steps/seconds %d\n", steps,seconds,steps/seconds);
			char filename[100];
			sprintf(filename, "%s_%d_%d.txt", "states", reset_frame, steps);
			SoftBodyHelpers::writeState(filename, psb);
		}
		if (sim_time + reset_frame * 0.05 >= 5) exit(0);
		float internalTimeStep = 1. / 240.f;
		//        float internalTimeStep = 0.1f;
		m_dynamicsWorld->stepSimulation(deltaTime, deltaTime / internalTimeStep, internalTimeStep);
	}

	void addCloth(const Vec3& origin);

	virtual void renderScene()
	{
		CommonDeformableBodyBase::renderScene();
		DeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();

		for (i32 i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
		{
			SoftBody* psb = (SoftBody*)deformableWorld->getSoftBodyArray()[i];
			{
				SoftBodyHelpers::DrawFrame(psb, deformableWorld->getDebugDrawer());
				SoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
			}
		}
	}
};

void LoadDeformed::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();
	DeformableBodySolver* deformableBodySolver = new DeformableBodySolver();

	DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
	sol->setDeformableSolver(deformableBodySolver);
	m_solver = sol;

	m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, deformableBodySolver);
	Vec3 gravity = Vec3(0, -9.8, 0);
	m_dynamicsWorld->setGravity(gravity);
	getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	{
		///create a ground
		CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150.), Scalar(2.5), Scalar(150.)));
		groundShape->setMargin(0.02);
		m_collisionShapes.push_back(groundShape);

		Transform2 groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(Vec3(0, -3.5, 0));
		groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0));
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
		body->setFriction(4);

		//add the ground to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
	}
	addCloth(Vec3(0, 1, 0));
	getDeformableDynamicsWorld()->setImplicit(false);
	getDeformableDynamicsWorld()->setLineSearch(false);
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void LoadDeformed::addCloth(const Vec3& origin)
// create a piece of cloth
{
	const Scalar s = 0.6;
	const Scalar h = 0;

	psb = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, h, -2 * s),
										 Vec3(+s, h, -2 * s),
										 Vec3(-s, h, +2 * s),
										 Vec3(+s, h, +2 * s),
										 15, 30,
										 0, true, 0.0);

	psb->getCollisionShape()->setMargin(0.02);
	psb->generateBendingConstraints(2);
	psb->setTotalMass(.5);
	psb->m_cfg.kKHR = 1;  // collision hardness with kinematic objects
	psb->m_cfg.kCHR = 1;  // collision hardness with rigid body
	psb->m_cfg.kDF = 0.1;
	psb->rotate(Quat(0, SIMD_PI / 2, 0));
	Transform2 clothTransform;
	clothTransform.setIdentity();
	clothTransform.setOrigin(Vec3(0, 0.2, 0) + origin);
	psb->transform(clothTransform);

	DefaultFileIO fileio;
	char absolute_path[1024];
	char filename[100];
	sprintf(filename, "/Users/fuchuyuan/Documents/mybullet/build_cmake/examples/ExampleBrowser/states_0_%d.txt", reset_frame);
	fileio.findResourcePath(filename, absolute_path, 1024);
	AlignedObjectArray<Vec3> qs;
	AlignedObjectArray<Vec3> vs;
	CustomSoftBodyHelper::loadDeformableState(qs, vs, absolute_path, &fileio);
	if (reset_frame > 0)
		psb->updateState(qs, vs);
	psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
	psb->m_cfg.collisions |= SoftBody::fCollision::SDF_MDF;
	psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
	//    psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
	psb->setCollisionFlags(0);
	psb->setCacheBarycenter(true);
	getDeformableDynamicsWorld()->addSoftBody(psb);
	psb->setSelfCollision(false);

	DeformableMassSpringForce* mass_spring = new DeformableMassSpringForce(2, 0.2, true);
	psb->setSpringStiffness(4);
	getDeformableDynamicsWorld()->addForce(psb, mass_spring);
	m_forces.push_back(mass_spring);
	Vec3 gravity = Vec3(0, -9.8, 0);
	DeformableGravityForce* gravity_force = new DeformableGravityForce(gravity);
	getDeformableDynamicsWorld()->addForce(psb, gravity_force);
	//    getDeformableDynamicsWorld()->setUseProjection(true);
	m_forces.push_back(gravity_force);
}

void LoadDeformed::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization
	removePickingConstraint();
	//remove the rigidbodies from the dynamics world and delete them
	i32 i;
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
	// delete forces
	for (i32 j = 0; j < m_forces.size(); j++)
	{
		DeformableLagrangianForce* force = m_forces[j];
		delete force;
	}
	m_forces.clear();

	//delete collision shapes
	for (i32 j = 0; j < m_collisionShapes.size(); j++)
	{
		CollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_dynamicsWorld;

	delete m_solver;

	delete m_broadphase;

	delete m_dispatcher;

	delete m_collisionConfiguration;
}

class CommonExampleInterface* LoadDeformedCreateFunc(struct CommonExampleOptions& options)
{
	return new LoadDeformed(options.m_guiHelper);
}
