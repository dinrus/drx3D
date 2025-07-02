#include "../ClothFriction.h"
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

///The ClothFriction shows the use of deformable friction.
class ClothFriction : public CommonDeformableBodyBase
{
    DeformableBodySolver* m_deformableBodySolver;
public:
    ClothFriction(struct GUIHelperInterface* helper)
    : CommonDeformableBodyBase(helper),
    m_deformableBodySolver(0)
    {
    }
    virtual ~ClothFriction()
    {
        
    }
    void initPhysics();
    
    void exitPhysics();
    
    void resetCamera()
    {
        float dist = 12;
        float pitch = -50;
        float yaw = 120;
        float targetPos[3] = {0, -3, 0};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
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
            SoftBody* psb = (SoftBody*)deformableWorld->getSoftBodyArray()[i];
            {
                //SoftBodyHelpers::DrawFrame(psb, deformableWorld->getDebugDrawer());
				SoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), fDrawFlags::Faces);// deformableWorld->getDrawFlags());
            }
        }

    }
};

void ClothFriction::initPhysics()
{
    m_guiHelper->setUpAxis(1);
    
    ///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();
    
    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
    
    m_broadphase = new DbvtBroadphase();
    m_deformableBodySolver = new DeformableBodySolver();
    
    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(m_deformableBodySolver);
    m_solver = sol;
    
    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, m_deformableBodySolver);
    Vec3 gravity = Vec3(0, -10, 0);
    m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.Reset();
    
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
    
    {
        ///create a ground
        CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150), Scalar(25.), Scalar(150)));
        
        m_collisionShapes.push_back(groundShape);
        
        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, -32, 0));
        groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.1));
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
        body->setFriction(3);
        
        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body);
    }
    
    // create a piece of cloth
    {
        Scalar s = 4;
        Scalar h = 0;
        
        SoftBody* psb = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, h, -s),
                                                         Vec3(+s, h, -s),
                                                         Vec3(-s, h, +s),
                                                         Vec3(+s, h, +s),
                                                         10,10,
                                                         0, true);
        
        psb->getCollisionShape()->setMargin(0.06);
        psb->generateBendingConstraints(2);
        psb->setTotalMass(1);
        psb->setSpringStiffness(100);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = 3;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        psb->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
        getDeformableDynamicsWorld()->addSoftBody(psb);
        
        DeformableMassSpringForce* mass_spring = new DeformableMassSpringForce(10,1, true);
        getDeformableDynamicsWorld()->addForce(psb, mass_spring);
        m_forces.push_back(mass_spring);
        
        DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        
        
        h = .5;
        s = 2;
        SoftBody* psb2 = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, h, -s),
                                                          Vec3(+s, h, -s),
                                                          Vec3(-s, h, +s),
                                                          Vec3(+s, h, +s),
                                                          5,5,
                                                          0, true);
        psb2->getCollisionShape()->setMargin(0.06);
        psb2->generateBendingConstraints(2);
        psb2->setTotalMass(1);
        psb2->setSpringStiffness(100);
        psb2->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb2->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb2->m_cfg.kDF = 1;
        psb2->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb2->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        psb2->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
        psb->translate(Vec3(0,0,0));
        getDeformableDynamicsWorld()->addSoftBody(psb2);
        
        DeformableMassSpringForce* mass_spring2 = new DeformableMassSpringForce(10,.1, true);
        getDeformableDynamicsWorld()->addForce(psb2, mass_spring2);
        m_forces.push_back(mass_spring2);
        
        DeformableGravityForce* gravity_force2 =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb2, gravity_force2);
        m_forces.push_back(gravity_force2);
    }
    getDeformableDynamicsWorld()->setImplicit(false);
    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void ClothFriction::exitPhysics()
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
    
    delete m_deformableBodySolver;

    delete m_broadphase;
    
    delete m_dispatcher;
    
    delete m_collisionConfiguration;
}

class CommonExampleInterface* ClothFrictionCreateFunc(struct CommonExampleOptions& options)
{
    return new ClothFriction(options.m_guiHelper);
}


