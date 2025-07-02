#include "../SplitImpulse.h"
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <stdio.h>  //printf debugging
#include <drx3D/Common/Interfaces/CommonDeformableBodyBase.h>
#include <drx3D/Common/ResourcePath.h>

///The SplitImpulse shows the effect of split impulse in deformable rigid contact.
class SplitImpulse : public CommonDeformableBodyBase
{
public:
	SplitImpulse(struct GUIHelperInterface* helper)
		: CommonDeformableBodyBase(helper)
	{
	}
	virtual ~SplitImpulse()
	{
	}
	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
        float dist = 20;
        float pitch = -45;
        float yaw = 100;
        float targetPos[3] = {0, -3, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
    
    void stepSimulation(float deltaTime)
    {
        //use a smaller internal timestep, there are stability issues
        float internalTimeStep = 1. / 240.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
    }
    
    void Ctor_RbUpStack(i32 count)
    {
        float mass = 0.2;
        
        CollisionShape* shape[] = {
            new BoxShape(Vec3(1, 1, 1)),
        };
        Transform2 startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(Vec3(0, 0.7, 0));
        createRigidBody(mass, startTransform, shape[0]);
    }
    
    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
        DeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();
        
        for (i32 i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
        {
            SoftBody* psb = (SoftBody*)deformableWorld->getSoftBodyArray()[i];
            //if (softWorld->getDebugDrawer() && !(softWorld->getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
            {
                SoftBodyHelpers::DrawFrame(psb, deformableWorld->getDebugDrawer());
                SoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
            }
        }
    }
};

void SplitImpulse::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();
    DeformableBodySolver* deformableBodySolver = new DeformableBodySolver();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(deformableBodySolver);
	m_solver = sol;

	m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, deformableBodySolver);
//    deformableBodySolver->setWorld(getDeformableDynamicsWorld());
	//	m_dynamicsWorld->getSolverInfo().m_singleAxisDeformableThreshold = 0.f;//faster but lower quality
    Vec3 gravity = Vec3(0, -50, 0);
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.Reset();
    
//    getDeformableDynamicsWorld()->before_solver_callbacks.push_back(dynamics);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    {
        ///create a ground
        CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150.), Scalar(25.), Scalar(150.)));

        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, -32, 0));
        groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.));
        //We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
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
        body->setFriction(1);

        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body);
    }
    
    // create a piece of cloth
    {
        const Scalar s = 4;
        const Scalar h = 0;
        
        SoftBody* psb = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, h, -s),
                                                         Vec3(+s, h, -s),
                                                         Vec3(-s, h, +s),
                                                         Vec3(+s, h, +s),
//                                                         3,3,
                                                          20,20,
                                                          1 + 2 + 4 + 8, true);
//                                                          0, true);

        
        psb->getCollisionShape()->setMargin(0.015);
        psb->generateBendingConstraints(2);
        psb->setTotalMass(1);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = 1;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        getDeformableDynamicsWorld()->addSoftBody(psb);
        
        DeformableMassSpringForce* mass_spring = new DeformableMassSpringForce(30,1, true);
        getDeformableDynamicsWorld()->addForce(psb, mass_spring);
        m_forces.push_back(mass_spring);
        
        DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        // add a few rigid bodies
        Ctor_RbUpStack(1);
    }
    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void SplitImpulse::exitPhysics()
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



class CommonExampleInterface* SplitImpulseCreateFunc(struct CommonExampleOptions& options)
{
	return new SplitImpulse(options.m_guiHelper);
}


