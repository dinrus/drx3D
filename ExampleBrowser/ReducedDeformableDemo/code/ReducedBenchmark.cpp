#include "../ReducedBenchmark.h"
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
static Scalar COLLIDING_VELOCITY = 0;
static i32 num_modes = 20;
static bool run_reduced = true;

class ReducedBenchmark : public CommonDeformableBodyBase
{
    Vec3 m_gravity;
public:
    ReducedBenchmark(struct GUIHelperInterface* helper)
        : CommonDeformableBodyBase(helper)
    {
    }
    virtual ~ReducedBenchmark()
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
        // float dist = 6;
        // float pitch = -10;
        // float yaw = 90;
        // float targetPos[3] = {0, 2, 0};
        float dist = 10;
        float pitch = -30;
        float yaw = 125;
        float targetPos[3] = {0, 2, 0};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }

    void Ctor_RbUpStack(const Vec3& origin)
    {
        float mass = 10;
        CollisionShape* shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
        // CollisionShape* shape = new BoxShape(Vec3(1, 1, 1));
        Transform2 startTransform;
        startTransform.setIdentity();
        // startTransform.setOrigin(Vec3(0, 12, 0));
        // RigidBody* rb0 = createRigidBody(mass, startTransform, shape);
        // rb0->setLinearVelocity(Vec3(0, 0, 0));

        startTransform.setOrigin(origin);
        // startTransform.setRotation(Quat(Vec3(1, 0, 1), SIMD_PI / 4.0));
        RigidBody* rb1 = createRigidBody(mass, startTransform, shape);
        rb1->setActivationState(DISABLE_DEACTIVATION);
        // rb1->setLinearVelocity(Vec3(0, 0, 4));
    }

    void createDeform(const Vec3& origin, const Quat& rotation)
    {

        if (run_reduced)
        {
            STxt file_path("../../../data/reduced_torus/");
            STxt vtk_file("torus_mesh.vtk");
            ReducedDeformableBody* rsb = ReducedDeformableBodyHelpers::createReducedDeformableObject(
                                                getDeformableDynamicsWorld()->getWorldInfo(),
                                                file_path,
                                                vtk_file,
                                                num_modes,
                                                false);

            getDeformableDynamicsWorld()->addSoftBody(rsb);
            rsb->getCollisionShape()->setMargin(0.01);
            // rsb->scale(Vec3(1, 1, 0.5));

            rsb->setTotalMass(10);

            Transform2 init_transform;
            init_transform.setIdentity();
            init_transform.setOrigin(origin);
            init_transform.setRotation(rotation);
            rsb->transformTo(init_transform);

            rsb->setStiffnessScale(5);
            rsb->setDamping(damping_alpha, damping_beta);
            // rsb->scale(Vec3(0.5, 0.5, 0.5));

            rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
            rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
            rsb->m_cfg.kDF = 0;
            rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
            rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
            rsb->m_sleepingThreshold = 0;
            SoftBodyHelpers::generateBoundaryFaces(rsb);

            std::cout << "Running reduced deformable\n";
        }
        else    // create full deformable cube
        {
            STxt filepath("../../../data/reduced_torus/");
            STxt filename = filepath + "torus_mesh.vtk";
            SoftBody* psb = SoftBodyHelpers::CreateFromVtkFile(getDeformableDynamicsWorld()->getWorldInfo(), filename.c_str());

            Transform2 init_transform;
            init_transform.setIdentity();
            init_transform.setOrigin(origin);
            init_transform.setRotation(rotation);
            psb->transform(init_transform);
            psb->getCollisionShape()->setMargin(0.015);
            psb->setTotalMass(10);
            psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
            psb->m_cfg.kCHR = 1; // collision hardness with rigid body
            psb->m_cfg.kDF = .5;
            psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
            psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
            getDeformableDynamicsWorld()->addSoftBody(psb);
            SoftBodyHelpers::generateBoundaryFaces(psb);

            DeformableGravityForce* gravity_force =  new DeformableGravityForce(m_gravity);
            getDeformableDynamicsWorld()->addForce(psb, gravity_force);
            m_forces.push_back(gravity_force);

            Scalar E = 10000;
            Scalar nu = 0.3;
            Scalar lambda = E * nu / ((1 + nu) * (1 - 2 * nu));
            Scalar mu = E / (2 * (1 + nu));
            DeformableNeoHookeanForce* neohookean = new DeformableNeoHookeanForce(lambda, mu, 0.01);
            // neohookean->setPoissonRatio(0.3);
            // neohookean->setYoungsModulus(25);
            neohookean->setDamping(0.01);
            psb->m_cfg.drag = 0.001;
            getDeformableDynamicsWorld()->addForce(psb, neohookean);
            m_forces.push_back(neohookean);

            std::cout << "Running full deformable\n";
        }

        // btReducedDeformableBody* rsb = btReducedDeformableBodyHelpers::createReducedTorus(getDeformableDynamicsWorld()->getWorldInfo(), num_modes);

        // getDeformableDynamicsWorld()->addSoftBody(rsb);
        // rsb->getCollisionShape()->setMargin(0.01);
        // // rsb->scale(Vec3(1, 1, 0.5));

        // rsb->setTotalMass(10);

        // Transform2 init_transform;
        // init_transform.setIdentity();
        // init_transform.setOrigin(origin);
        // init_transform.setRotation(rotation);
        // rsb->transformTo(init_transform);

        // rsb->setStiffnessScale(5);
        // rsb->setDamping(damping_alpha, damping_beta);
        // // rsb->scale(Vec3(0.5, 0.5, 0.5));

        // rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        // rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
        // rsb->m_cfg.kDF = 0;
        // rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        // rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        // rsb->m_sleepingThreshold = 0;
        // SoftBodyHelpers::generateBoundaryFaces(rsb);
    }

    void stepSimulation(float deltaTime)
    {
      float internalTimeStep = 1. / 240.f;
      m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
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
            }
        }
    }
};

void ReducedBenchmark::initPhysics()
{
    m_guiHelper->setUpAxis(1);

    ///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

    m_broadphase = new DbvtBroadphase();

    if (run_reduced)
    {
        ReducedDeformableBodySolver* solver = new ReducedDeformableBodySolver();

        DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
        sol->setDeformableSolver(solver);
        m_solver = sol;
        m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, solver);
    }
    else
    {
        DeformableBodySolver* solver = new DeformableBodySolver();

        DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
        sol->setDeformableSolver(solver);
        m_solver = sol;
        m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, solver);
    }

    // m_dynamicsWorld = new btDeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, solver);
    Vec3 gravity = Vec3(0, -10, 0);
    m_gravity = gravity;
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    // 3x3 torus
    createDeform(Vec3(4, 4, -4), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(4, 4, 0), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(4, 4, 4), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(0, 4, -4), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(0, 4, 0), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(0, 4, 4), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(-4, 4, -4), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(-4, 4, 0), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));
    createDeform(Vec3(-4, 4, 4), Quat(SIMD_PI / 2.0, SIMD_PI / 2.0, 0));

    // create a static rigid box as the ground
    {
        // BoxShape* groundShape = createBoxShape(Vec3(Scalar(50), Scalar(50), Scalar(50)));
        BoxShape* groundShape = createBoxShape(Vec3(Scalar(20), Scalar(2), Scalar(20)));
        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        // groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI / 6.0));
        // groundTransform.setRotation(Quat(Vec3(0, 0, 1), SIMD_PI / 6.0));
        groundTransform.setOrigin(Vec3(0, 0, 0));
        // groundTransform.setOrigin(Vec3(0, 0, 6));
        // groundTransform.setOrigin(Vec3(0, -50, 0));
        {
            Scalar mass(0.);
            createRigidBody(mass, groundTransform, groundShape, Vec4(0,0,0,0));
        }
    }

    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(false);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_cfm = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_friction = 0.5;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = false;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;

    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
    m_dynamicsWorld->setGravity(gravity);
}

void ReducedBenchmark::exitPhysics()
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



class CommonExampleInterface* ReducedBenchmarkCreateFunc(struct CommonExampleOptions& options)
{
    return new ReducedBenchmark(options.m_guiHelper);
}


