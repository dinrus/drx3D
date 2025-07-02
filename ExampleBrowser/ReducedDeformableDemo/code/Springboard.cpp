#include "../Springboard.h"
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

static Scalar damping_alpha = 0.0;
static Scalar damping_beta = 0.0001;
static i32 num_modes = 20;

class Springboard : public CommonDeformableBodyBase
{
    Scalar sim_time;
    bool first_step;

public:
    Springboard(struct GUIHelperInterface* helper)
        : CommonDeformableBodyBase(helper)
    {
        sim_time = 0;
        first_step = true;
    }
    virtual ~Springboard()
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
        float dist = 10;
        float pitch = 0;
        float yaw = 90;
        float targetPos[3] = {0, 3, 0};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }

    void Ctor_RbUpStack()
    {
        float mass = 2;
        CollisionShape* shape = new BoxShape(Vec3(1, 1, 1));
        Transform2 startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(Vec3(0,8,1));
        RigidBody* rb1 = createRigidBody(mass, startTransform, shape);
        rb1->setActivationState(DISABLE_DEACTIVATION);
    }

    void stepSimulation(float deltaTime)
    {
      float internalTimeStep = 1. / 60.f;
      m_dynamicsWorld->stepSimulation(deltaTime, 1, internalTimeStep);

      {
        sim_time += internalTimeStep;
        ReducedDeformableBody* rsb = static_cast<ReducedDeformableBody*>(getDeformableDynamicsWorld()->getSoftBodyArray()[0]);

        // std::ofstream myfile("fixed_node.txt", std::ios_base::app);
        // myfile << sim_time << "\t" << rsb->m_nodes[0].m_x[0] - rsb->m_x0[0][0] << "\t"
        //                            << rsb->m_nodes[0].m_x[1] - rsb->m_x0[0][1] << "\t"
        //                            << rsb->m_nodes[0].m_x[2] - rsb->m_x0[0][2] << "\n";
        // myfile.close();
      }
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
                SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());

                for (i32 p = 0; p < rsb->m_fixedNodes.size(); ++p)
                {
                    deformableWorld->getDebugDrawer()->drawSphere(rsb->m_nodes[rsb->m_fixedNodes[p]].m_x, 0.2, Vec3(1, 0, 0));
                    // std::cout << rsb->m_nodes[rsb->m_fixedNodes[p]].m_x[0] << "\t" << rsb->m_nodes[rsb->m_fixedNodes[p]].m_x[1] << "\t" << rsb->m_nodes[rsb->m_fixedNodes[p]].m_x[2] << "\n";
                }
                // deformableWorld->getDebugDrawer()->drawSphere(Vec3(0, 0, 0), 0.1, Vec3(1, 1, 1));
                // deformableWorld->getDebugDrawer()->drawSphere(Vec3(0, 2, 0), 0.1, Vec3(1, 1, 1));
                // deformableWorld->getDebugDrawer()->drawSphere(Vec3(0, 4, 0), 0.1, Vec3(1, 1, 1));
            }
        }
    }
};

void Springboard::initPhysics()
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
    m_dynamicsWorld->setGravity(gravity);
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    // create volumetric reduced deformable body
    {
        STxt file_path(drx::GetHomeDir() + "dinrus/dev/drx3D/data/reduced_beam/");
        STxt vtk_file("beam_mesh_origin.vtk");
        ReducedDeformableBody* rsb = ReducedDeformableBodyHelpers::createReducedDeformableObject(
                                            getDeformableDynamicsWorld()->getWorldInfo(),
                                            file_path,
                                            vtk_file,
                                            num_modes,
                                            false);

        getDeformableDynamicsWorld()->addSoftBody(rsb);
        rsb->getCollisionShape()->setMargin(0.1);

        Transform2 init_transform;
        init_transform.setIdentity();
        init_transform.setOrigin(Vec3(0, 4, 0));
        // init_transform.setRotation(Quat(Vec3(0, 1, 0), SIMD_PI / 2.0));
        rsb->transform(init_transform);

        rsb->setStiffnessScale(200);
        rsb->setDamping(damping_alpha, damping_beta);

        // set fixed nodes
        rsb->setFixedNodes(0);
        rsb->setFixedNodes(1);
        rsb->setFixedNodes(2);
        rsb->setFixedNodes(3);

        rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
        rsb->m_cfg.kDF = 0;
        rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        rsb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(rsb);

        // rsb->setVelocity(Vec3(0, -COLLIDING_VELOCITY, 0));
        // rsb->setRigidVelocity(Vec3(0, 1, 0));
        // rsb->setRigidAngularVelocity(Vec3(1, 0, 0));
    }
    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(false);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.3;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_cfm = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = false;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;
    // add a few rigid bodies
    Ctor_RbUpStack();

    // create a static rigid box as the ground
    {
        // BoxShape* groundShape = createBoxShape(Vec3(Scalar(50), Scalar(50), Scalar(50)));
        BoxShape* groundShape = createBoxShape(Vec3(Scalar(10), Scalar(2), Scalar(10)));
        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, 0, 0));
        {
            Scalar mass(0.);
            createRigidBody(mass, groundTransform, groundShape, Vec4(0,0,0,0));
        }
    }

    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void Springboard::exitPhysics()
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



class CommonExampleInterface* ReducedSpringboardCreateFunc(struct CommonExampleOptions& options)
{
    return new Springboard(options.m_guiHelper);
}


