#include "../FrictionSlope.h"
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
static Scalar damping_beta = 0.001;
static Scalar COLLIDING_VELOCITY = 0;
static i32 num_modes = 20;

class FrictionSlope : public CommonDeformableBodyBase
{
public:
    FrictionSlope(struct GUIHelperInterface* helper)
        : CommonDeformableBodyBase(helper)
    {}
    virtual ~FrictionSlope()
    {
    }
    void initPhysics();

    void exitPhysics();

    // TODO: disable pick force, non-interactive for now.
    bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) {
        return false;
    }

    void resetCamera()
    {
        float dist = 20;
        float pitch = -20;
        float yaw = 90;
        float targetPos[3] = {0, 0, 0.5};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }

    void Ctor_RbUpStack()
    {
        float mass = 1;
        CollisionShape* shape = new BoxShape(Vec3(1, 1, 1));
        // CollisionShape* shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
        // CollisionShape* shape = new BoxShape(Vec3(0.5, 0.25, 2));
        Transform2 startTransform;
        startTransform.setIdentity();

        startTransform.setOrigin(Vec3(0,4,0));
        RigidBody* rb1 = createRigidBody(mass, startTransform, shape);
        rb1->setLinearVelocity(Vec3(0, 0, 0));
    }

    void createGround()
    {
        BoxShape* groundShape = createBoxShape(Vec3(Scalar(10), Scalar(2), Scalar(10)));
        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        // groundTransform.setRotation(Quat(Vec3(0, 0, 1), SIMD_PI / 6.0));
        groundTransform.setOrigin(Vec3(0, 0, 0));
        Scalar mass(1e6);
        RigidBody* ground = createRigidBody(mass, groundTransform, groundShape, Vec4(0,0,0,0));
        // ground->setFriction(1);
    }

    void stepSimulation(float deltaTime)
    {
      float internalTimeStep = 1. / 60.f;
      m_dynamicsWorld->stepSimulation(deltaTime, 1, internalTimeStep);
    }

    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
        DeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();

        for (i32 i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
        {
            ReducedDeformableBody* rsb = static_cast<ReducedDeformableBody*>(deformableWorld->getSoftBodyArray()[i]);
            {
                SoftBodyHelpers::DrawFrame(rsb, deformableWorld->getDebugDrawer());
                // SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), flag);
                SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());

                // for (i32 p = 0; p < rsb->m_fixedNodes.size(); ++p)
                // {
                //     deformableWorld->getDebugDrawer()->drawSphere(rsb->m_nodes[rsb->m_fixedNodes[p]].m_x, 0.2, Vec3(1, 0, 0));
                // }
                // for (i32 p = 0; p < rsb->m_nodeRigidContacts.size(); ++p)
                // {
                //     deformableWorld->getDebugDrawer()->drawSphere(rsb->m_nodes[rsb->m_contactNodesList[p]].m_x, 0.2, Vec3(0, 1, 0));
                // }
            }
        }
    }
};

namespace FrictionSlopeHelper
{
    void groundMotion(Scalar time, DeformableMultiBodyDynamicsWorld* world)
    {
        AlignedObjectArray<RigidBody*>& rbs = world->getNonStaticRigidBodies();

        RigidBody* ground = rbs[0];
        Assert(ground->getMass() > 1e5);

        Scalar start_time = 2;
        Scalar end_time = 8;
        Scalar start_angle = 0;
        Scalar end_angle = SIMD_PI / 6;
        Scalar current_angle = 0;
        Scalar turn_speed = (end_angle - start_angle) / (end_time - start_time);

        if (time >= start_time)
        {
            current_angle = (time - start_time) * turn_speed;
            if (time > end_time)
            {
                current_angle = end_angle;
                turn_speed = 0;
            }
        }
        else
        {
            current_angle = start_angle;
            turn_speed = 0;
        }

        Transform2 groundTransform;
        groundTransform.setIdentity();
        // groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI / 6.0));
        groundTransform.setRotation(Quat(Vec3(0, 0, 1), current_angle));

        ground->setCenterOfMassTransform(groundTransform);
        ground->setLinearVelocity(Vec3(0, 0, 0));
        ground->setAngularVelocity(Vec3(0, 0, 0));
    }
};

void FrictionSlope::initPhysics()
{
    m_guiHelper->setUpAxis(1);

    ///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

    m_broadphase = new DbvtBroadphase();
    ReducedDeformableBodySolver* reducedSoftBodySolver = new ReducedDeformableBodySolver();
    Vec3 gravity = Vec3(0, -10, 0);
    reducedSoftBodySolver->setGravity(gravity);

    DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(reducedSoftBodySolver);
    m_solver = sol;

    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, reducedSoftBodySolver);
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    // create volumetric reduced deformable body
    {
        STxt file_path("../../../data/reduced_beam/");
        STxt vtk_file("beam_mesh_origin.vtk");
        ReducedDeformableBody* rsb = ReducedDeformableBodyHelpers::createReducedDeformableObject(
                                            getDeformableDynamicsWorld()->getWorldInfo(),
                                            file_path,
                                            vtk_file,
                                            num_modes,
                                            false);

        getDeformableDynamicsWorld()->addSoftBody(rsb);
        rsb->getCollisionShape()->setMargin(0.01);

        Transform2 init_transform;
        init_transform.setIdentity();
        init_transform.setOrigin(Vec3(0, 4, 0));
        init_transform.setRotation(Quat(Vec3(0, 0, 1), SIMD_PI / 2.0));
        rsb->transform(init_transform);
        rsb->setStiffnessScale(50);
        rsb->setDamping(damping_alpha, damping_beta);

        rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
        rsb->m_cfg.kDF = 0;
        rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        rsb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(rsb);
    }

    createGround();
    // add a few rigid bodies
    // Ctor_RbUpStack();

    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(false);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_friction = 1;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = false;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;
    getDeformableDynamicsWorld()->setSolverCallback(FrictionSlopeHelper::groundMotion);
    m_dynamicsWorld->setGravity(gravity);
    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void FrictionSlope::exitPhysics()
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



class CommonExampleInterface* FrictionSlopeCreateFunc(struct CommonExampleOptions& options)
{
    return new FrictionSlope(options.m_guiHelper);
}


