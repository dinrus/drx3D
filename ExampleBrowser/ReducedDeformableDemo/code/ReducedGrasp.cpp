#include "../ReducedGrasp.h"
#include <drx/Core/Core.h>
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBody.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodyHelpers.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <stdio.h>  //printf debugging
#include <drx3D/Common/Interfaces/CommonDeformableBodyBase.h>
#include <drx3D/Common/ResourcePath.h>

///The BasicTest shows the contact between volumetric deformable objects and rigid objects.
// static Scalar E = 50;
// static Scalar nu = 0.3;
static Scalar damping_alpha = 0.0;
static Scalar damping_beta = 0.0001;
static i32 num_modes = 20;

class ReducedGrasp : public CommonDeformableBodyBase
{
public:
    ReducedGrasp(struct GUIHelperInterface* helper)
        : CommonDeformableBodyBase(helper)
    {
    }
    virtual ~ReducedGrasp()
    {
    }
    void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
        float dist = 10;
        float pitch = -10;
        float yaw = 90;

        // float dist = 25;
        // float pitch = -30;
        // float yaw = 100;
        float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

    void stepSimulation(float deltaTime)
    {
        //use a smaller internal timestep, there are stability issues
        float internalTimeStep = 1. / 240.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
        // float internalTimeStep = 1. / 60.f;
        // m_dynamicsWorld->stepSimulation(deltaTime, 1, internalTimeStep);
    }

    void createGrip()
    {
        i32 count = 2;
        float mass = 1e6;
        CollisionShape* shape = new BoxShape(Vec3(1, 1, 0.25));
        {
            Transform2 startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(Vec3(0,1,0));
            startTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.));
            createRigidBody(mass, startTransform, shape);
        }
        {
            Transform2 startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(Vec3(0,1,-4));
            startTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.));
            createRigidBody(mass, startTransform, shape);
        }

    }

    void Ctor_RbUpStack()
    {
        float mass = 8;
        CollisionShape* shape = new BoxShape(Vec3(0.25, 2, 0.5));
        Transform2 startTransform;
        startTransform.setIdentity();

        startTransform.setOrigin(Vec3(0,9.5,0));
        RigidBody* rb1 = createRigidBody(mass, startTransform, shape);
        rb1->setLinearVelocity(Vec3(0, 0, 0));
        rb1->setFriction(0.7);
    }

    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
        DeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();

        for (i32 i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
        {
            // btReducedDeformableBody* rsb = static_cast<btReducedDeformableBody*>(deformableWorld->getSoftBodyArray()[i]);
            // {
            //     SoftBodyHelpers::DrawFrame(rsb, deformableWorld->getDebugDrawer());
            //     SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
            // }

            // for (i32 p = 0; p < rsb->m_nodeRigidContacts.size(); ++p)
            // {
            //     deformableWorld->getDebugDrawer()->drawSphere(rsb->m_nodes[rsb->m_contactNodesList[p]].m_x, 0.1, Vec3(0, 1, 0));
            // }

            SoftBody* psb = static_cast<SoftBody*>(deformableWorld->getSoftBodyArray()[i]);
            {
                SoftBodyHelpers::DrawFrame(psb, deformableWorld->getDebugDrawer());
                SoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
            }
        }
    }

    static void GripperDynamics(Scalar time, DeformableMultiBodyDynamicsWorld* world);
};

void ReducedGrasp::GripperDynamics(Scalar time, DeformableMultiBodyDynamicsWorld* world)
{
    AlignedObjectArray<RigidBody*>& rbs = world->getNonStaticRigidBodies();
    if (rbs.size()<2)
        return;
    RigidBody* rb0 = rbs[0];
    // Scalar pressTime = 0.9;
    // Scalar pressTime = 0.96;
    Scalar pressTime = 1.26;
    Scalar liftTime = 2.5;
    Scalar shiftTime = 6;
    Scalar holdTime = 7;
    Scalar dropTime = 10;
    // Scalar holdTime = 500;
    // Scalar dropTime = 1000;
    Transform2 rbTransform;
    rbTransform.setIdentity();
    Vec3 translation;
    Vec3 velocity;

    Vec3 initialTranslationLeft = Vec3(0,1,0);            // inner face has z=2
    Vec3 initialTranslationRight = Vec3(0,1,-4);          // inner face has z=-2
    Vec3 pinchVelocityLeft = Vec3(0,0,-1);
    Vec3 pinchVelocityRight = Vec3(0,0,1);
    Vec3 liftVelocity = Vec3(0,4,0);
    Vec3 shiftVelocity = Vec3(0,0,2);
    Vec3 holdVelocity = Vec3(0,0,0);
    Vec3 openVelocityLeft = Vec3(0,0,4);
    Vec3 openVelocityRight = Vec3(0,0,-4);

    if (time < pressTime)
    {
        velocity = pinchVelocityLeft;
        translation = initialTranslationLeft + pinchVelocityLeft * time;
    }
    // else
    // {
    //     velocity = Vec3(0, 0, 0);
    //     translation = initialTranslationLeft + pinchVelocityLeft * pressTime;
    // }
    else if (time < liftTime)
    {
        velocity = liftVelocity;
        translation = initialTranslationLeft + pinchVelocityLeft * pressTime + liftVelocity * (time - pressTime);
    }
    else if (time < shiftTime)
    {
        velocity = shiftVelocity;
        translation = initialTranslationLeft + pinchVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (time - liftTime);
    }
    else if (time < holdTime)
    {
        velocity = Vec3(0,0,0);
        translation = initialTranslationLeft + pinchVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (time - shiftTime);
    }
    else if (time < dropTime)
    {
        velocity = openVelocityLeft;
        translation = initialTranslationLeft + pinchVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityLeft * (time - holdTime);
    }
    else
    {
        velocity = holdVelocity;
        translation = initialTranslationLeft + pinchVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityLeft * (dropTime - holdTime);
    }
    rbTransform.setOrigin(translation);
    rbTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0));
    rb0->setCenterOfMassTransform(rbTransform);
    rb0->setAngularVelocity(Vec3(0,0,0));
    rb0->setLinearVelocity(velocity);

    RigidBody* rb1 = rbs[1];
    if (time < pressTime)
    {
        velocity = pinchVelocityRight;
        translation = initialTranslationRight + pinchVelocityRight * time;
    }
    // else
    // {
    //     velocity = Vec3(0, 0, 0);
    //     translation = initialTranslationRight + pinchVelocityRight * pressTime;
    // }
    else if (time < liftTime)
    {
        velocity = liftVelocity;
        translation = initialTranslationRight + pinchVelocityRight * pressTime + liftVelocity * (time - pressTime);
    }
    else if (time < shiftTime)
    {
        velocity = shiftVelocity;
        translation = initialTranslationRight + pinchVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (time - liftTime);
    }
    else if (time < holdTime)
    {
        velocity = Vec3(0,0,0);
        translation = initialTranslationRight + pinchVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (time - shiftTime);
    }
    else if (time < dropTime)
    {
        velocity = openVelocityRight;
        translation = initialTranslationRight + pinchVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityRight * (time - holdTime);
    }
    else
    {
        velocity = holdVelocity;
        translation = initialTranslationRight + pinchVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityRight * (dropTime - holdTime);
    }
    rbTransform.setOrigin(translation);
    rbTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0));
    rb1->setCenterOfMassTransform(rbTransform);
    rb1->setAngularVelocity(Vec3(0,0,0));
    rb1->setLinearVelocity(velocity);

    rb0->setFriction(20);
    rb1->setFriction(20);
}

void ReducedGrasp::initPhysics()
{
    m_guiHelper->setUpAxis(1);

    ///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

    m_broadphase = new DbvtBroadphase();
    // btDeformableBodySolver* solver = new btDeformableBodySolver();
    ReducedDeformableBodySolver* solver = new ReducedDeformableBodySolver();

    DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(solver);
    m_solver = sol;

    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, solver);
    Vec3 gravity = Vec3(0, -10, 0);
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
    getDeformableDynamicsWorld()->setSolverCallback(GripperDynamics);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    // create volumetric reduced deformable body
    {
        STxt file_path(drx::GetHomeDir() + "dinrus/dev/drx3D/data/reduced_cube/");
        STxt vtk_file("cube_mesh.vtk");
        ReducedDeformableBody* rsb = ReducedDeformableBodyHelpers::createReducedDeformableObject(
                                            getDeformableDynamicsWorld()->getWorldInfo(),
                                            file_path,
                                            vtk_file,
                                            num_modes,
                                            false);

        getDeformableDynamicsWorld()->addSoftBody(rsb);
        rsb->getCollisionShape()->setMargin(0.015);

        Transform2 init_transform;
        init_transform.setIdentity();
        init_transform.setOrigin(Vec3(0, 1, -2));
        // init_transform.setRotation(Quat(0, SIMD_PI / 2.0, SIMD_PI / 2.0));
        // init_transform.setRotation(Quat(Vec3(0, 1, 0), SIMD_PI / 2.0));
        rsb->transform(init_transform);

        rsb->setStiffnessScale(100);
        rsb->setDamping(damping_alpha, damping_beta);

        rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
        rsb->m_cfg.kDF = 0;
        rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        rsb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(rsb);
    }

    // create full deformable cube
    {
        // STxt filepath("../../../examples/SoftDemo/cube/");
        // STxt filename = filepath + "mesh.vtk";
        // SoftBody* psb = SoftBodyHelpers::CreateFromVtkFile(getDeformableDynamicsWorld()->getWorldInfo(), filename.c_str());

        // // psb->scale(Vec3(2, 2, 2));
        // psb->translate(Vec3(0, 1, -2));
        // psb->getCollisionShape()->setMargin(0.05);
        // psb->setTotalMass(28.6);
        // psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        // psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        // psb->m_cfg.kDF = .5;
        // psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        // psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        // getDeformableDynamicsWorld()->addSoftBody(psb);
        // SoftBodyHelpers::generateBoundaryFaces(psb);

        // btDeformableGravityForce* gravity_force =  new btDeformableGravityForce(gravity);
        // getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        // m_forces.push_back(gravity_force);

        // Scalar E = 10000;
        // Scalar nu = 0.3;
        // Scalar lambda = E * nu / ((1 + nu) * (1 - 2 * nu));
        // Scalar mu = E / (2 * (1 + nu));
        // btDeformableNeoHookeanForce* neohookean = new btDeformableNeoHookeanForce(lambda, mu, 0.02);
        // // neohookean->setPoissonRatio(0.3);
        // // neohookean->setYoungsModulus(25);
        // neohookean->setDamping(0.01);
        // psb->m_cfg.drag = 0.001;
        // getDeformableDynamicsWorld()->addForce(psb, neohookean);
        // m_forces.push_back(neohookean);
    }

    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(false);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_cfm = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_friction = 1;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = false;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;

    // grippers
    createGrip();

    // rigid block
    // Ctor_RbUpStack();

    // {
    //     float mass = 10;
    //     CollisionShape* shape = new BoxShape(Vec3(0.25, 2, 0.5));
    //     Transform2 startTransform;
    //     startTransform.setIdentity();
    //     startTransform.setOrigin(Vec3(0,4,0));
    //     RigidBody* rb1 = createRigidBody(mass, startTransform, shape);
    //     rb1->setLinearVelocity(Vec3(0, 0, 0));
    // }

    //create a ground
    {
        CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150.), Scalar(25.), Scalar(150.)));

        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, -25, 0));
        groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0));
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
        body->setFriction(0.5);

        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body);
    }

    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void ReducedGrasp::exitPhysics()
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



class CommonExampleInterface* ReducedGraspCreateFunc(struct CommonExampleOptions& options)
{
    return new ReducedGrasp(options.m_guiHelper);
}


