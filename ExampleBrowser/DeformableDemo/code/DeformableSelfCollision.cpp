#include "../DeformableSelfCollision.h"
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

///The DeformableSelfCollision shows deformable self collisions
class DeformableSelfCollision : public CommonDeformableBodyBase
{
public:
    DeformableSelfCollision(struct GUIHelperInterface* helper)
    : CommonDeformableBodyBase(helper)
    {
        m_maxPickingForce = 0.004;
    }
    virtual ~DeformableSelfCollision()
    {
    }
    void initPhysics();
    
    void exitPhysics();
    
    void resetCamera()
    {
        float dist = 2.0;
        float pitch = -8;
        float yaw = 100;
        float targetPos[3] = {0, -1.0, 0};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }
    
    void stepSimulation(float deltaTime)
    {
        float internalTimeStep = 1. / 240.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
    }
    
    void addCloth(const Vec3& origin);
    
    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
    }
};

void DeformableSelfCollision::initPhysics()
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
    //    m_dynamicsWorld->getSolverInfo().m_singleAxisDeformableThreshold = 0.f;//faster but lower quality
    Vec3 gravity = Vec3(0, -9.8, 0);
    m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
    getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
    
    //    getDeformableDynamicsWorld()->before_solver_callbacks.push_back(dynamics);
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
        body->setFriction(4);
        
        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body);
    }
    addCloth(Vec3(0, -0.2, 0));
    addCloth(Vec3(0, -0.1, 0));
    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}
void DeformableSelfCollision::addCloth(const Vec3& origin)
// create a piece of cloth
{
    const Scalar s = 0.6;
    const Scalar h = 0;
    
    SoftBody* psb = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, h, -2*s),
                                                     Vec3(+s, h, -2*s),
                                                     Vec3(-s, h, +2*s),
                                                     Vec3(+s, h, +2*s),
                                                     15,30,
//                                                     4,4,
                                                     0, true, 0.0);

    
    psb->getCollisionShape()->setMargin(0.02);
    psb->generateBendingConstraints(2);
    psb->setTotalMass(.5);
    psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
    psb->m_cfg.kCHR = 1; // collision hardness with rigid body
    psb->m_cfg.kDF = 0.1;
//    psb->rotate(Quat(0, SIMD_PI / 2, 0));
    Transform2 clothTransform;
    clothTransform.setIdentity();
    clothTransform.setOrigin(Vec3(0,0.2,0)+origin);
    psb->transform(clothTransform);
    psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
    psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
    psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
    psb->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
    getDeformableDynamicsWorld()->addSoftBody(psb);
    psb->setSelfCollision(true);
    
    DeformableMassSpringForce* mass_spring = new DeformableMassSpringForce(2,0.2, true);
    psb->setSpringStiffness(4);
    getDeformableDynamicsWorld()->addForce(psb, mass_spring);
    m_forces.push_back(mass_spring);
    Vec3 gravity = Vec3(0, -9.8, 0);
    DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
    getDeformableDynamicsWorld()->addForce(psb, gravity_force);
    getDeformableDynamicsWorld()->setUseProjection(true);
    m_forces.push_back(gravity_force);
}

void DeformableSelfCollision::exitPhysics()
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



class CommonExampleInterface* DeformableSelfCollisionCreateFunc(struct CommonExampleOptions& options)
{
    return new DeformableSelfCollision(options.m_guiHelper);
}


