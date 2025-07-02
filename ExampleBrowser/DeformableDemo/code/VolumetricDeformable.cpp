#include "../VolumetricDeformable.h"
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <stdio.h>  //printf debugging
#include <drx3D/Common/Interfaces/CommonDeformableBodyBase.h>
#include <drx3D/Common/ResourcePath.h>

///The VolumetricDeformable shows the contact between volumetric deformable objects and rigid objects.
static Scalar E = 50;
static Scalar nu = 0.3;
static Scalar damping_alpha = 0.1;
static Scalar damping_beta = 0.01;

struct TetraCube
{
#include "../../SoftDemo/cube.inl"
};

class VolumetricDeformable : public CommonDeformableBodyBase
{
	DeformableLinearElasticityForce* m_linearElasticity;

public:
	VolumetricDeformable(struct GUIHelperInterface* helper)
		: CommonDeformableBodyBase(helper)
	{
        m_linearElasticity = 0;
		m_pickingForceElasticStiffness = 100;
		m_pickingForceDampingStiffness = 0;
		m_maxPickingForce = 1e10; // allow large picking force with implicit scheme.
	}
	virtual ~VolumetricDeformable()
	{
	}
	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
        float dist = 20;
        float pitch = -45;
        float yaw = 100;
        float targetPos[3] = {0, 3, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
    
    void stepSimulation(float deltaTime)
    {
		m_linearElasticity->setPoissonRatio(nu);
		m_linearElasticity->setYoungsModulus(E);
		m_linearElasticity->setDamping(damping_alpha, damping_beta);
        //use a smaller internal timestep, there are stability issues
        float internalTimeStep = 1. / 240;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
    }
    
    void createStaticBox(const Vec3& halfEdge, const Vec3& translation)
    {
        CollisionShape* box = new BoxShape(halfEdge);
        m_collisionShapes.push_back(box);
        
        Transform2 Transform;
        Transform.setIdentity();
        Transform.setOrigin(translation);
        Transform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.0));
        //We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
        Scalar mass(0.);
        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);
        Vec3 localInertia(0, 0, 0);
        if (isDynamic)
            box->calculateLocalInertia(mass, localInertia);
        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        DefaultMotionState* myMotionState = new DefaultMotionState(Transform);
        RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, box, localInertia);
        RigidBody* body = new RigidBody(rbInfo);
        body->setFriction(0.5);
        
        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body);
    }
    
    void Ctor_RbUpStack(i32 count)
    {
        float mass = 2;
        
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
        };
        static i32k nshapes = sizeof(shape) / sizeof(shape[0]);
        for (i32 i = 0; i < count; ++i)
        {
            Transform2 startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin(Vec3(i, 10 + 2 * i, i-1));
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

void VolumetricDeformable::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();
    DeformableBodySolver* deformableBodySolver = new DeformableBodySolver();

	DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(deformableBodySolver);
	m_solver = sol;

	m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, deformableBodySolver);
    Vec3 gravity = Vec3(0, -100, 0);
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.Reset();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    {
        ///create a ground
        CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150.), Scalar(50.), Scalar(150.)));
        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, -50, 0));
        groundTransform.setRotation(Quat(Vec3(1, 0, 0), SIMD_PI * 0.0));
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
    
    createStaticBox(Vec3(1, 5, 5), Vec3(-5,0,0));
    createStaticBox(Vec3(1, 5, 5), Vec3(5,0,0));
    createStaticBox(Vec3(5, 5, 1), Vec3(0,0,5));
    createStaticBox(Vec3(5, 5, 1), Vec3(0,0,-5));
    
    // create volumetric soft body
    {
        SoftBody* psb = SoftBodyHelpers::CreateFromTetGenData(getDeformableDynamicsWorld()->getWorldInfo(),
                                                                  TetraCube::getElements(),
                                                                  0,
                                                                  TetraCube::getNodes(),
                                                                  false, true, true);
        getDeformableDynamicsWorld()->addSoftBody(psb);
        psb->scale(Vec3(2, 2, 2));
        psb->translate(Vec3(0, 5, 0));
        psb->getCollisionShape()->setMargin(0.1);
        psb->setTotalMass(0.5);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
		psb->m_cfg.kDF = 2;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
		psb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(psb);
        DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
        
        DeformableLinearElasticityForce* linearElasticity = new DeformableLinearElasticityForce(100,100,0.01);
		m_linearElasticity = linearElasticity;
        getDeformableDynamicsWorld()->addForce(psb, linearElasticity);
        m_forces.push_back(linearElasticity);
    }
    getDeformableDynamicsWorld()->setImplicit(true);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(true);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.3;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = true;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;
    // add a few rigid bodies
    Ctor_RbUpStack(4);
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
	
	{
		SliderParams slider("Young's Modulus", &E);
		slider.m_minVal = 0;
		slider.m_maxVal = 2000;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("Poisson Ratio", &nu);
		slider.m_minVal = 0.05;
		slider.m_maxVal = 0.49;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("Mass Damping", &damping_alpha);
		slider.m_minVal = 0;
		slider.m_maxVal = 1;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
    {
        SliderParams slider("Stiffness Damping", &damping_beta);
        slider.m_minVal = 0;
        slider.m_maxVal = 0.1;
        if (m_guiHelper->getParameterInterface())
            m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }
}

void VolumetricDeformable::exitPhysics()
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



class CommonExampleInterface* VolumetricDeformableCreateFunc(struct CommonExampleOptions& options)
{
	return new VolumetricDeformable(options.m_guiHelper);
}


