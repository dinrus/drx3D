#include "../DeformableRigid.h"
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

///The DeformableRigid shows contact between deformable objects and rigid objects.
class DeformableRigid : public CommonDeformableBodyBase
{
public:
	DeformableRigid(struct GUIHelperInterface* helper)
    :CommonDeformableBodyBase(helper)
	{
	}
	virtual ~DeformableRigid()
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
        
//
//        CollisionShape* boxShape = new BoxShape(Vec3(1, 1, 1));
//        boxShape->setMargin(1e-3);
//        if (0)
//        {
//        Vec3 p(0.99,1.01,0.99);
//        for (i32 i = 0; i < 40; ++i)
//        {
//            p[1] -= 0.001;
//            Scalar margin(.000001);
//            Transform2 trans;
//            trans.setIdentity();
//            btGjkEpaSolver2::sResults results;
//            const ConvexShape* csh = static_cast<const ConvexShape*>(boxShape);
//            Scalar d = btGjkEpaSolver2::SignedDistance(p, margin, csh, trans, results);
//            printf("d = %f\n", d);
//            printf("----\n");
//        }
//        }
//
//        Vec3 p(.991,1.01,.99);
//        for (i32 i = 0; i < 40; ++i)
//        {
//            p[1] -= 0.001;
//            Scalar margin(.006);
//            Transform2 trans;
//            trans.setIdentity();
//            Scalar dst;
//            btGjkEpaSolver2::sResults results;
//            Transform2 point_transform;
//            point_transform.setIdentity();
//            point_transform.setOrigin(p);
//            SphereShape sphere(margin);
//            Vec3 guess(0,0,0);
//            const ConvexShape* csh = static_cast<const ConvexShape*>(boxShape);
//            btGjkEpaSolver2::SignedDistance(&sphere, point_transform, csh, trans, guess, results);
//            dst = results.distance-csh->getMargin();
//            dst -= margin;
//            printf("d = %f\n", dst);
//             printf("----\n");
//        }
    }
    
    void Ctor_RbUpStack(i32 count)
    {
        float mass = .2;
        
        CompoundShape* cylinderCompound = new CompoundShape;
        CollisionShape* cylinderShape = new CylinderShapeX(Vec3(2, .5, .5));
        CollisionShape* boxShape = new BoxShape(Vec3(2, .5, .5));
        Transform2 localTransform;
        localTransform.setIdentity();
        cylinderCompound->addChildShape(localTransform, boxShape);
        Quat orn(SIMD_HALF_PI, 0, 0);
        localTransform.setRotation(orn);
        //    localTransform.setOrigin(Vec3(1,1,1));
        cylinderCompound->addChildShape(localTransform, cylinderShape);
        
        CollisionShape* shape[] = {
            new BoxShape(Vec3(1, 1, 1)),
            new SphereShape(0.75),
            cylinderCompound
        };
//        static i32k nshapes = sizeof(shape) / sizeof(shape[0]);
//        for (i32 i = 0; i < count; ++i)
//        {
//            Transform2 startTransform;
//            startTransform.setIdentity();
//            startTransform.setOrigin(Vec3(0, 2+ 2 * i, 0));
//            startTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.));
//            createRigidBody(mass, startTransform, shape[i % nshapes]);
//        }
        Transform2 startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(Vec3(1, 1.5, 1));
        createRigidBody(mass, startTransform, shape[0]);
        startTransform.setOrigin(Vec3(1, 1.5, -1));
        createRigidBody(mass, startTransform, shape[0]);
        startTransform.setOrigin(Vec3(-1, 1.5, 1));
        createRigidBody(mass, startTransform, shape[0]);
        startTransform.setOrigin(Vec3(-1, 1.5, -1));
        createRigidBody(mass, startTransform, shape[0]);
        startTransform.setOrigin(Vec3(0, 3.5, 0));
        createRigidBody(mass, startTransform, shape[0]);
    }
    
    virtual const DeformableMultiBodyDynamicsWorld* getDeformableDynamicsWorld() const
    {
        ///just make it a SoftRigidDynamicsWorld please
        ///or we will add type checking
        return (DeformableMultiBodyDynamicsWorld*)m_dynamicsWorld;
    }
    
    virtual DeformableMultiBodyDynamicsWorld* getDeformableDynamicsWorld()
    {
        ///just make it a SoftRigidDynamicsWorld please
        ///or we will add type checking
        return (DeformableMultiBodyDynamicsWorld*)m_dynamicsWorld;
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
				SoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), fDrawFlags::Faces);// deformableWorld->getDrawFlags());
            }
        }
    }
};

void DeformableRigid::initPhysics()
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
    Vec3 gravity = Vec3(0, -10, 0);
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
        groundTransform.setOrigin(Vec3(0, -42, 0));
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
    if(1)
    {
        bool onGround = false;
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

        if (onGround)
            psb = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, 0, -s),
                                                 Vec3(+s, 0, -s),
                                                 Vec3(-s, 0, +s),
                                                 Vec3(+s, 0, +s),
//                                                 20,20,
                                                 2,2,
                                                 0, true);
        
        psb->getCollisionShape()->setMargin(0.05);
        psb->generateBendingConstraints(2);
        psb->setTotalMass(1);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = 2;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        getDeformableDynamicsWorld()->addSoftBody(psb);
        
        DeformableMassSpringForce* mass_spring = new DeformableMassSpringForce(15,0.5, true);
        getDeformableDynamicsWorld()->addForce(psb, mass_spring);
        m_forces.push_back(mass_spring);
        
        DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        // add a few rigid bodies
    }
    Ctor_RbUpStack(10);
    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void DeformableRigid::exitPhysics()
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



class CommonExampleInterface* DeformableRigidCreateFunc(struct CommonExampleOptions& options)
{
	return new DeformableRigid(options.m_guiHelper);
}


