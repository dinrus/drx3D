#include "../Pinch.h"
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

///The Pinch shows the frictional contact between kinematic rigid objects with deformable objects

struct TetraCube
{
#include "../../SoftDemo/cube.inl"
};

class Pinch : public CommonDeformableBodyBase
{
public:
	Pinch(struct GUIHelperInterface* helper)
		: CommonDeformableBodyBase(helper)
	{
	}
	virtual ~Pinch()
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

void dynamics(Scalar time, DeformableMultiBodyDynamicsWorld* world)
{
    AlignedObjectArray<RigidBody*>& rbs = world->getNonStaticRigidBodies();
    if (rbs.size()<2)
        return;
    RigidBody* rb0 = rbs[0];
    Scalar pressTime = 0.9;
    Scalar liftTime = 2.5;
    Scalar shiftTime = 3.5;
    Scalar holdTime = 4.5*1000;
    Scalar dropTime = 5.3*1000;
    Transform2 rbTransform;
    rbTransform.setIdentity();
    Vec3 translation;
    Vec3 velocity;
    
    Vec3 initialTranslationLeft = Vec3(0.5,3,4);
    Vec3 initialTranslationRight = Vec3(0.5,3,-4);
    Vec3 pinchVelocityLeft = Vec3(0,0,-2);
    Vec3 pinchVelocityRight = Vec3(0,0,2);
    Vec3 liftVelocity = Vec3(0,5,0);
    Vec3 shiftVelocity = Vec3(0,0,5);
    Vec3 holdVelocity = Vec3(0,0,0);
    Vec3 openVelocityLeft = Vec3(0,0,4);
    Vec3 openVelocityRight = Vec3(0,0,-4);
    
    if (time < pressTime)
    {
        velocity = pinchVelocityLeft;
        translation = initialTranslationLeft + pinchVelocityLeft * time;
    }
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

void Pinch::initPhysics()
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
    getDeformableDynamicsWorld()->setSolverCallback(dynamics);
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
        Scalar verts[24] = {0.f, 0.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f, 1.f,
            1.f, 1.f, 0.f,
            0.f, 1.f, 1.f,
            1.f, 0.f, 1.f,
            1.f, 1.f, 1.f
        };
        i32 triangles[60] = {0, 6, 3,
            0,1,6,
            7,5,3,
            7,3,6,
            4,7,6,
            4,6,1,
            7,2,5,
            7,4,2,
            0,3,2,
            2,3,5,
            0,2,4,
            0,4,1,
            0,6,5,
            0,6,4,
            3,4,2,
            3,4,7,
            2,7,3,
            2,7,1,
            4,5,0,
            4,5,6,
        };
//       SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(getDeformableDynamicsWorld()->getWorldInfo(), &verts[0], &triangles[0], 20);
////
        SoftBody* psb = SoftBodyHelpers::CreateFromTetGenData(getDeformableDynamicsWorld()->getWorldInfo(),
                                                                  TetraCube::getElements(),
                                                                  0,
                                                                  TetraCube::getNodes(),
                                                                  false, true, true);
        
        psb->scale(Vec3(2, 2, 2));
        psb->translate(Vec3(0, 4, 0));
        psb->getCollisionShape()->setMargin(0.01);
        psb->setTotalMass(1);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = .5;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        getDeformableDynamicsWorld()->addSoftBody(psb);
        SoftBodyHelpers::generateBoundaryFaces(psb);
        
        DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        
        DeformableNeoHookeanForce* neohookean = new DeformableNeoHookeanForce(8,3, 0.02);
        neohookean->setPoissonRatio(0.3);
        neohookean->setYoungsModulus(25);
        neohookean->setDamping(0.01);
        psb->m_cfg.drag = 0.001;
        getDeformableDynamicsWorld()->addForce(psb, neohookean);
        m_forces.push_back(neohookean);
        // add a grippers
        createGrip();
    }
    getDeformableDynamicsWorld()->setImplicit(false);
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void Pinch::exitPhysics()
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



class CommonExampleInterface* PinchCreateFunc(struct CommonExampleOptions& options)
{
	return new Pinch(options.m_guiHelper);
}


