#include "../PinchFriction.h"
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

///The PinchFriction shows the frictional contacts among volumetric deformable objects

struct TetraCube
{
#include "../../SoftDemo/cube.inl"
};

class PinchFriction : public CommonDeformableBodyBase
{
    AlignedObjectArray<DeformableLagrangianForce*> m_forces;
public:
    PinchFriction(struct GUIHelperInterface* helper)
    : CommonDeformableBodyBase(helper)
    {
    }
    virtual ~PinchFriction()
    {
    }
    void initPhysics();
    
    void exitPhysics();
    
    void resetCamera()
    {
        float dist = 25;
        float pitch = -30;
        float yaw = 100;
        float targetPos[3] = {0, -0, 0};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }
    
    void stepSimulation(float deltaTime)
    {
        //use a smaller internal timestep, there are stability issues
        float internalTimeStep = 1. / 240.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
    }
    
    void createGrip()
    {
        i32 count = 2;
        float mass = 1e6;
        CollisionShape* shape[] = {
            new BoxShape(Vec3(3, 3, 0.5)),
        };
        static i32k nshapes = sizeof(shape) / sizeof(shape[0]);
        for (i32 i = 0; i < count; ++i)
        {
            Transform2 startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(Vec3(10, 0, 0));
            startTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.));
            createRigidBody(mass, startTransform, shape[i % nshapes]);
        }
    }
    
    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
    }
    
    virtual bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
    {
        return false;
    }
    virtual bool movePickedBody(const Vec3& rayFromWorld, const Vec3& rayToWorld)
    {
        return false;
    }
    virtual void removePickingConstraint(){}
};

void dynamics2(Scalar time, DeformableMultiBodyDynamicsWorld* world)
{
    AlignedObjectArray<RigidBody*>& rbs = world->getNonStaticRigidBodies();
    if (rbs.size()<2)
        return;
    RigidBody* rb0 = rbs[0];
    Scalar pressTime = 0.45;
    Scalar liftTime = 5;
    Scalar shiftTime = 1.75;
    Scalar holdTime = 7.5;
    Scalar dropTime = 8.3;
    Transform2 rbTransform;
    rbTransform.setIdentity();
    Vec3 translation;
    Vec3 velocity;
    
    Vec3 initialTranslationLeft = Vec3(0.5,3,4);
    Vec3 initialTranslationRight = Vec3(0.5,3,-4);
    Vec3 PinchFrictionVelocityLeft = Vec3(0,0,-2);
    Vec3 PinchFrictionVelocityRight = Vec3(0,0,2);
    Vec3 liftVelocity = Vec3(0,2,0);
    Vec3 shiftVelocity = Vec3(0,0,0);
    Vec3 holdVelocity = Vec3(0,0,0);
    Vec3 openVelocityLeft = Vec3(0,0,4);
    Vec3 openVelocityRight = Vec3(0,0,-4);
    
    if (time < pressTime)
    {
        velocity = PinchFrictionVelocityLeft;
        translation = initialTranslationLeft + PinchFrictionVelocityLeft * time;
    }
    else if (time < liftTime)
    {
        velocity = liftVelocity;
        translation = initialTranslationLeft + PinchFrictionVelocityLeft * pressTime + liftVelocity * (time - pressTime);
        
    }
    else if (time < shiftTime)
    {
        velocity = shiftVelocity;
        translation = initialTranslationLeft + PinchFrictionVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (time - liftTime);
    }
    else if (time < holdTime)
    {
        velocity = Vec3(0,0,0);
        translation = initialTranslationLeft + PinchFrictionVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (time - shiftTime);
    }
    else if (time < dropTime)
    {
        velocity = openVelocityLeft;
        translation = initialTranslationLeft + PinchFrictionVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityLeft * (time - holdTime);
    }
    else
    {
        velocity = holdVelocity;
        translation = initialTranslationLeft + PinchFrictionVelocityLeft * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityLeft * (dropTime - holdTime);
    }
    rbTransform.setOrigin(translation);
    rbTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0));
    rb0->setCenterOfMassTransform(rbTransform);
    rb0->setAngularVelocity(Vec3(0,0,0));
    rb0->setLinearVelocity(velocity);
    
    RigidBody* rb1 = rbs[1];
    if (time < pressTime)
    {
        velocity = PinchFrictionVelocityRight;
        translation = initialTranslationRight + PinchFrictionVelocityRight * time;
    }
    else if (time < liftTime)
    {
        velocity = liftVelocity;
        translation = initialTranslationRight + PinchFrictionVelocityRight * pressTime + liftVelocity * (time - pressTime);
        
    }
    else if (time < shiftTime)
    {
        velocity = shiftVelocity;
        translation = initialTranslationRight + PinchFrictionVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (time - liftTime);
    }
    else if (time < holdTime)
    {
        velocity = Vec3(0,0,0);
        translation = initialTranslationRight + PinchFrictionVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (time - shiftTime);
    }
    else if (time < dropTime)
    {
        velocity = openVelocityRight;
        translation = initialTranslationRight + PinchFrictionVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityRight * (time - holdTime);
    }
    else
    {
        velocity = holdVelocity;
        translation = initialTranslationRight + PinchFrictionVelocityRight * pressTime + liftVelocity * (liftTime-pressTime) + shiftVelocity * (shiftTime - liftTime) + holdVelocity * (holdTime - shiftTime)+ openVelocityRight * (dropTime - holdTime);
    }
    rbTransform.setOrigin(translation);
    rbTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0));
    rb1->setCenterOfMassTransform(rbTransform);
    rb1->setAngularVelocity(Vec3(0,0,0));
    rb1->setLinearVelocity(velocity);
    
    rb0->setFriction(200);
    rb1->setFriction(200);
}

void PinchFriction::initPhysics()
{
    m_guiHelper->setUpAxis(1);
    
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();
    
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
    
    m_broadphase = new DbvtBroadphase();
    DeformableBodySolver* deformableBodySolver = new DeformableBodySolver();
    
    DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(deformableBodySolver);
    m_solver = sol;
    
    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, deformableBodySolver);
    Vec3 gravity = Vec3(0, -10, 0);
    m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.Reset();
    getDeformableDynamicsWorld()->setSolverCallback(dynamics2);
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
    
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
    
    // create a soft block
    {
        SoftBody* psb = SoftBodyHelpers::CreateFromTetGenData(getDeformableDynamicsWorld()->getWorldInfo(),
                                                                  TetraCube::getElements(),
                                                                  0,
                                                                  TetraCube::getNodes(),
                                                                  false, true, true);
        
        psb->scale(Vec3(2, 2, 1));
        psb->translate(Vec3(0, 2.1, 2.2));
        psb->getCollisionShape()->setMargin(0.025);
        psb->setSpringStiffness(10);
        psb->setTotalMass(.6);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = 2;
        SoftBodyHelpers::generateBoundaryFaces(psb);
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        psb->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
        getDeformableDynamicsWorld()->addSoftBody(psb);

        DeformableGravityForce* gravity_force = new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        
        DeformableNeoHookeanForce* neohookean = new DeformableNeoHookeanForce(6,6,.003);
        getDeformableDynamicsWorld()->addForce(psb, neohookean);
        m_forces.push_back(neohookean);
    }
    
    // create a second soft block
    {
        SoftBody* psb2 = SoftBodyHelpers::CreateFromTetGenData(getDeformableDynamicsWorld()->getWorldInfo(),
                                                                  TetraCube::getElements(),
                                                                  0,
                                                                  TetraCube::getNodes(),
                                                                  false, true, true);
        
        psb2->scale(Vec3(2, 2, 1));
        psb2->translate(Vec3(0, 2.1, -2.2));
        psb2->getCollisionShape()->setMargin(0.025);
        psb2->setTotalMass(.6);
        psb2->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb2->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb2->m_cfg.kDF = 2;
        psb2->setSpringStiffness(10);
        psb2->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb2->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        psb2->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
        SoftBodyHelpers::generateBoundaryFaces(psb2);
        getDeformableDynamicsWorld()->addSoftBody(psb2);
        
        DeformableGravityForce* gravity_force = new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb2, gravity_force);
        m_forces.push_back(gravity_force);
        
        DeformableNeoHookeanForce* neohookean = new DeformableNeoHookeanForce(6,6,.003);
        getDeformableDynamicsWorld()->addForce(psb2, neohookean);
        m_forces.push_back(neohookean);
    }
    
    // create a third soft block
    {
        SoftBody* psb3 = SoftBodyHelpers::CreateFromTetGenData(getDeformableDynamicsWorld()->getWorldInfo(),
                                                                   TetraCube::getElements(),
                                                                   0,
                                                                   TetraCube::getNodes(),
                                                                   false, true, true);
        
        psb3->scale(Vec3(2, 2, 1));
        psb3->translate(Vec3(0, 2.1, 0));
        psb3->getCollisionShape()->setMargin(0.025);
        psb3->setTotalMass(.6);
        psb3->setSpringStiffness(10);
        psb3->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb3->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb3->m_cfg.kDF = 2;
        psb3->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb3->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        psb3->m_cfg.collisions |= SoftBody::fCollision::VF_DD;
        SoftBodyHelpers::generateBoundaryFaces(psb3);
        getDeformableDynamicsWorld()->addSoftBody(psb3);
        
        DeformableGravityForce* gravity_force = new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb3, gravity_force);
        m_forces.push_back(gravity_force);
        
        DeformableNeoHookeanForce* neohookean = new DeformableNeoHookeanForce(6,6,.003);
        getDeformableDynamicsWorld()->addForce(psb3, neohookean);
        m_forces.push_back(neohookean);
    }
    getDeformableDynamicsWorld()->setImplicit(false);
    // add a pair of grippers
    createGrip();
    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void PinchFriction::exitPhysics()
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



class CommonExampleInterface* PinchFrictionCreateFunc(struct CommonExampleOptions& options)
{
    return new PinchFriction(options.m_guiHelper);
}


