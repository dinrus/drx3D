#include "../LargeDeformation.h"
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

///The LargeDeformation shows the contact between volumetric deformable objects and rigid objects.
static Scalar E = 50;
static Scalar nu = 0.3;
static Scalar damping_alpha = 0.1;
static Scalar damping_beta = 0.01;

struct TetraCube
{
#include "../../SoftDemo/cube.inl"
};

class LargeDeformation : public CommonDeformableBodyBase
{
	DeformableLinearElasticityForce* m_linearElasticity;

public:
	LargeDeformation(struct GUIHelperInterface* helper)
		: CommonDeformableBodyBase(helper)
	{
        m_linearElasticity = 0;
	}
	virtual ~LargeDeformation()
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
        float internalTimeStep = 1. / 60.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 1, internalTimeStep);
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

void LargeDeformation::initPhysics()
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
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

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
		psb->m_cfg.kDF = 0.5;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
		psb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(psb);
		for (i32 i = 0; i < psb->m_nodes.size(); ++i)
		{
			for (i32 j = 0; j < 3; ++j)
				psb->m_nodes[i].m_x[j] = ((double) 2*rand() / (RAND_MAX))-1.0;
			psb->m_nodes[i].m_x[1]+=8;
		}
        
        DeformableLinearElasticityForce* linearElasticity = new DeformableLinearElasticityForce(100,100,0.01);
		m_linearElasticity = linearElasticity;
        getDeformableDynamicsWorld()->addForce(psb, linearElasticity);
        m_forces.push_back(linearElasticity);
    }
    getDeformableDynamicsWorld()->setImplicit(true);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(true);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.1;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(20);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = true;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;
    // add a few rigid bodies
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
//	{
//		SliderParams slider("Young's Modulus", &E);
//		slider.m_minVal = 0;
//		slider.m_maxVal = 200;
//		if (m_guiHelper->getParameterInterface())
//			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
//	}
//	{
//		SliderParams slider("Poisson Ratio", &nu);
//		slider.m_minVal = 0.05;
//		slider.m_maxVal = 0.40;
//		if (m_guiHelper->getParameterInterface())
//			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
//	}
//	{
//		SliderParams slider("Mass Damping", &damping_alpha);
//		slider.m_minVal = 0.001;
//		slider.m_maxVal = 0.01;
//		if (m_guiHelper->getParameterInterface())
//			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
//	}
//    {
//        SliderParams slider("Stiffness Damping", &damping_beta);
//        slider.m_minVal = 0.001;
//        slider.m_maxVal = 0.01;
//        if (m_guiHelper->getParameterInterface())
//            m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
//    }
}

void LargeDeformation::exitPhysics()
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



class CommonExampleInterface* LargeDeformationCreateFunc(struct CommonExampleOptions& options)
{
	return new LargeDeformation(options.m_guiHelper);
}


