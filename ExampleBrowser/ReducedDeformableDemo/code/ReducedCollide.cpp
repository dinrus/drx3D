#include "../ReducedCollide.h"
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

///The BasicTest shows the contact between volumetric deformable objects and rigid objects.
// static Scalar E = 50;
// static Scalar nu = 0.3;
static Scalar damping_alpha = 0.0;
static Scalar damping_beta = 0.0;
static Scalar COLLIDING_VELOCITY = 4;
static i32 num_modes = 20;

class ReducedCollide : public CommonDeformableBodyBase
{
public:
    ReducedCollide(struct GUIHelperInterface* helper)
        : CommonDeformableBodyBase(helper)
    {
    }
    virtual ~ReducedCollide()
    {
    }
    void initPhysics();

    void exitPhysics();

	MultiBody* createFeatherstoneMultiBody_testMultiDof(class MultiBodyDynamicsWorld* world, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical = false, bool floating = false);
	void addColliders_testMultiDof(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents);

    // TODO: disable pick force, non-interactive for now.
    bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) {
        return false;
    }

    void resetCamera()
    {
        // float dist = 20;
        // float pitch = -10;
        float dist = 10;
        float pitch = -5;
        float yaw = 90;
        float targetPos[3] = {0, 0, 0};

        // float dist = 5;
		// float pitch = -35;
		// float yaw = 50;
		// float targetPos[3] = {-3, 2.8, -2.5};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }

    void Ctor_RbUpStack()
    {
        float mass = 10;

        CollisionShape* shape = new BoxShape(Vec3(0.5, 0.5, 0.5));
        // CollisionShape* shape = new BoxShape(Vec3(1, 1, 1));
        Vec3 localInertia(0, 0, 0);
		if (mass != 0.f)
			shape->calculateLocalInertia(mass, localInertia);

        Transform2 startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(Vec3(0,-2,0));
        // startTransform.setRotation(Quat(Vec3(1, 0, 1), SIMD_PI / 3.0));
        DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);

		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		m_dynamicsWorld->addRigidBody(body, 1, 1+2);

        body->setActivationState(DISABLE_DEACTIVATION);
        body->setLinearVelocity(Vec3(0, COLLIDING_VELOCITY, 0));
        // body->setFriction(1);
    }

    void rigidBar()
    {
        float mass = 10;

        CollisionShape* shape = new BoxShape(Vec3(0.5, 0.25, 2));
        Vec3 localInertia(0, 0, 0);
		if (mass != 0.f)
			shape->calculateLocalInertia(mass, localInertia);

        Transform2 startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(Vec3(0,10,0));
        // startTransform.setRotation(Quat(Vec3(1, 0, 1), SIMD_PI / 3.0));
        DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);

		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		m_dynamicsWorld->addRigidBody(body, 1, 1+2);

        body->setActivationState(DISABLE_DEACTIVATION);
        body->setLinearVelocity(Vec3(0, 0, 0));
        // body->setFriction(0);
    }

    void createGround()
    {
        // float mass = 55;
        float mass = 0;

        CollisionShape* shape = new BoxShape(Vec3(10, 2, 10));
        Vec3 localInertia(0, 0, 0);
		if (mass != 0.f)
			shape->calculateLocalInertia(mass, localInertia);

        Transform2 startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(Vec3(0,-2,0));
        // startTransform.setRotation(Quat(Vec3(1, 0, 1), SIMD_PI / 3.0));
        DefaultMotionState* myMotionState = new DefaultMotionState(startTransform);

		RigidBody::RigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
		RigidBody* body = new RigidBody(rbInfo);

		m_dynamicsWorld->addRigidBody(body, 1, 1+2);

        body->setActivationState(DISABLE_DEACTIVATION);
        body->setLinearVelocity(Vec3(0, 0, 0));
        // body->setFriction(1);
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
                SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
            }

            for (i32 p = 0; p < rsb->m_contactNodesList.size(); ++p)
            {
                i32 index = rsb->m_contactNodesList[p];
                deformableWorld->getDebugDrawer()->drawSphere(rsb->m_nodes[index].m_x, 0.2, Vec3(0, 1, 0));
            }
        }
    }
};

void ReducedCollide::initPhysics()
{
    m_guiHelper->setUpAxis(1);

    ///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

    m_broadphase = new DbvtBroadphase();
    ReducedDeformableBodySolver* reducedSoftBodySolver = new ReducedDeformableBodySolver();
    Vec3 gravity = Vec3(0, 0, 0);
    reducedSoftBodySolver->setGravity(gravity);

    DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(reducedSoftBodySolver);
    m_solver = sol;

    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, reducedSoftBodySolver);
    m_dynamicsWorld->setGravity(gravity);
	m_dynamicsWorld->getSolverInfo().m_globalCfm = 1e-3;
    m_dynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_RANDMIZE_ORDER;
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    // create volumetric reduced deformable body
    {
        STxt file_path(drx::GetHomeDir() + "dinrus/dev/drx3D/data/reduced_cube/");
        STxt vtk_file("cube_mesh.vtk");
        ReducedDeformableBody* rsb = ReducedDeformableBodyHelpers::createReducedDeformableObject(
                                            getDeformableDynamicsWorld()->getWorldInfo(),
                                            file_path,
                                            vtk_file,
                                            num_modes,
                                            false);

        getDeformableDynamicsWorld()->addSoftBody(rsb);
        rsb->getCollisionShape()->setMargin(0.1);
        // rsb->scale(Vec3(0.5, 0.5, 0.5));

        rsb->setStiffnessScale(100);
        rsb->setDamping(damping_alpha, damping_beta);

        rsb->setTotalMass(15);

        Transform2 init_transform;
        init_transform.setIdentity();
        init_transform.setOrigin(Vec3(0, 4, 0));
        // init_transform.setRotation(Quat(0, SIMD_PI / 2.0, SIMD_PI / 2.0));
        rsb->transformTo(init_transform);

        rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
        rsb->m_cfg.kDF = 0;
        rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        rsb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(rsb);

        rsb->setRigidVelocity(Vec3(0, -COLLIDING_VELOCITY, 0));
        // rsb->setRigidAngularVelocity(Vec3(1, 0, 0));
        drx3DPrintf("total mass: %e", rsb->getTotalMass());
    }
    // rigidBar();

    // add a few rigid bodies
    Ctor_RbUpStack();

    // create ground
    // createGround();

    // create multibody
    // {
    //     bool damping = false;
    //     bool gyro = true;
    //     i32 numLinks = 0;
    //     bool spherical = true;  //set it ot false -to use 1DoF hinges instead of 3DoF sphericals
    //     bool multibodyOnly = true;
    //     bool canSleep = false;
    //     bool selfCollide = true;
    //     bool multibodyConstraint = false;
    //     Vec3 linkHalfExtents(0.05, 0.37, 0.1);
    //     Vec3 baseHalfExtents(1, 1, 1);
    //     // Vec3 baseHalfExtents(2.5, 0.5, 2.5);
    //     // Vec3 baseHalfExtents(0.05, 0.37, 0.1);

    //     bool g_floatingBase = true;
    //     // btMultiBody* mbC = createFeatherstoneMultiBody_testMultiDof(m_dynamicsWorld, numLinks, Vec3(0, 4, 0), linkHalfExtents, baseHalfExtents, spherical, g_floatingBase);
    //     btMultiBody* mbC = createFeatherstoneMultiBody_testMultiDof(m_dynamicsWorld, numLinks, Vec3(0.f, 4.f, 0.f), baseHalfExtents, linkHalfExtents, spherical, g_floatingBase);
    //     //mbC->forceMultiDof();							//if !spherical, you can comment this line to check the 1DoF algorithm

    //     mbC->setCanSleep(canSleep);
    //     mbC->setHasSelfCollision(selfCollide);
    //     mbC->setUseGyroTerm(gyro);
    //     //
    //     if (!damping)
    //     {
    //         mbC->setLinearDamping(0.f);
    //         mbC->setAngularDamping(0.f);
    //     }
    //     else
    //     {
    //         mbC->setLinearDamping(0.1f);
    //         mbC->setAngularDamping(0.9f);
    //     }
    //     //
    //     //////////////////////////////////////////////
    //     // if (numLinks > 0)
    //     // {
    //     //     Scalar q0 = 45.f * SIMD_PI / 180.f;
    //     //     if (!spherical)
    //     //     {
    //     //         mbC->setJointPosMultiDof(0, &q0);
    //     //     }
    //     //     else
    //     //     {
    //     //         Quat quat0(Vec3(1, 1, 0).normalized(), q0);
    //     //         quat0.normalize();
    //     //         mbC->setJointPosMultiDof(0, quat0);
    //     //     }
    //     // }
    //     ///
    //     addColliders_testMultiDof(mbC, m_dynamicsWorld, baseHalfExtents, linkHalfExtents);
    // }

    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(false);
    getDeformableDynamicsWorld()->getSolverInfo().m_friction = 1;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_cfm = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-3;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = false;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 100;
    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);

    // {
    //     SliderParams slider("Young's Modulus", &E);
    //     slider.m_minVal = 0;
    //     slider.m_maxVal = 2000;
    //     if (m_guiHelper->getParameterInterface())
    //         m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    // }
    // {
    //     SliderParams slider("Poisson Ratio", &nu);
    //     slider.m_minVal = 0.05;
    //     slider.m_maxVal = 0.49;
    //     if (m_guiHelper->getParameterInterface())
    //         m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    // }
    // {
    //     SliderParams slider("Mass Damping", &damping_alpha);
    //     slider.m_minVal = 0;
    //     slider.m_maxVal = 1;
    //     if (m_guiHelper->getParameterInterface())
    //         m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    // }
    // {
    //     SliderParams slider("Stiffness Damping", &damping_beta);
    //     slider.m_minVal = 0;
    //     slider.m_maxVal = 0.1;
    //     if (m_guiHelper->getParameterInterface())
    //         m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    // }
}

void ReducedCollide::exitPhysics()
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

MultiBody* ReducedCollide::createFeatherstoneMultiBody_testMultiDof(MultiBodyDynamicsWorld* pWorld, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical, bool floating)
{
	//init the base
	Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
	float baseMass = 10;

	if (baseMass)
	{
		CollisionShape* pTempBox = new BoxShape(Vec3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));
		pTempBox->calculateLocalInertia(baseMass, baseInertiaDiag);
		delete pTempBox;
	}

	bool canSleep = false;

	MultiBody* pMultiBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);

	Quat baseOriQuat(0.f, 0.f, 0.f, 1.f);
	// Quat baseOriQuat(Vec3(0, 0, 1), -SIMD_PI / 6.0);
	pMultiBody->setBasePos(basePosition);
	pMultiBody->setWorldToBaseRot(baseOriQuat);
	Vec3 vel(0, 0, 0);

	//init the links
	Vec3 hingeJointAxis(1, 0, 0);
	float linkMass = 1.f;
	Vec3 linkInertiaDiag(0.f, 0.f, 0.f);

	CollisionShape* pTempBox = new BoxShape(Vec3(linkHalfExtents[0], linkHalfExtents[1], linkHalfExtents[2]));
	pTempBox->calculateLocalInertia(linkMass, linkInertiaDiag);
	delete pTempBox;

	//y-axis assumed up
	Vec3 parentComToCurrentCom(0, -linkHalfExtents[1] * 2.f, 0);                      //par body's COM to cur body's COM offset
	Vec3 currentPivotToCurrentCom(0, -linkHalfExtents[1], 0);                         //cur body's COM to cur body's PIV offset
	Vec3 parentComToCurrentPivot = parentComToCurrentCom - currentPivotToCurrentCom;  //par body's COM to cur body's PIV offset

	//////
	Scalar q0 = 0.f * SIMD_PI / 180.f;
	Quat quat0(Vec3(0, 1, 0).normalized(), q0);
	quat0.normalize();
	/////

	for (i32 i = 0; i < numLinks; ++i)
	{
		if (!spherical)
			pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f), hingeJointAxis, parentComToCurrentPivot, currentPivotToCurrentCom, true);
		else
			//pMultiBody->setupPlanar(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f)/*quat0*/, Vec3(1, 0, 0), parentComToCurrentPivot*2, false);
			pMultiBody->setupSpherical(i, linkMass, linkInertiaDiag, i - 1, Quat(0.f, 0.f, 0.f, 1.f), parentComToCurrentPivot, currentPivotToCurrentCom, true);
	}

	pMultiBody->finalizeMultiDof();
    pMultiBody->setBaseVel(vel);

	///
	pWorld->addMultiBody(pMultiBody);
	///
	return pMultiBody;
}

void ReducedCollide::addColliders_testMultiDof(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents)
{
	AlignedObjectArray<Quat> world_to_local;
	world_to_local.resize(pMultiBody->getNumLinks() + 1);

	AlignedObjectArray<Vec3> local_origin;
	local_origin.resize(pMultiBody->getNumLinks() + 1);
	world_to_local[0] = pMultiBody->getWorldToBaseRot();
	local_origin[0] = pMultiBody->getBasePos();

	{
		//	float pos[4]={local_origin[0].x(),local_origin[0].y(),local_origin[0].z(),1};
		Scalar quat[4] = {-world_to_local[0].x(), -world_to_local[0].y(), -world_to_local[0].z(), world_to_local[0].w()};

		if (1)
		{
			CollisionShape* box = new BoxShape(baseHalfExtents);
			MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, -1);
			col->setCollisionShape(box);

			Transform2 tr;
			tr.setIdentity();
			tr.setOrigin(local_origin[0]);
			tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
			col->setWorldTransform(tr);

			pWorld->addCollisionObject(col, 2, 1 + 2);

			col->setFriction(1);
			pMultiBody->setBaseCollider(col);
		}
	}

	for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
	{
		i32k parent = pMultiBody->getParent(i);
		world_to_local[i + 1] = pMultiBody->getParentToLocalRot(i) * world_to_local[parent + 1];
		local_origin[i + 1] = local_origin[parent + 1] + (quatRotate(world_to_local[i + 1].inverse(), pMultiBody->getRVector(i)));
	}

	for (i32 i = 0; i < pMultiBody->getNumLinks(); ++i)
	{
		Vec3 posr = local_origin[i + 1];
		//	float pos[4]={posr.x(),posr.y(),posr.z(),1};

		Scalar quat[4] = {-world_to_local[i + 1].x(), -world_to_local[i + 1].y(), -world_to_local[i + 1].z(), world_to_local[i + 1].w()};

		CollisionShape* box = new BoxShape(linkHalfExtents);
		MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, i);

		col->setCollisionShape(box);
		Transform2 tr;
		tr.setIdentity();
		tr.setOrigin(posr);
		tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
		col->setWorldTransform(tr);
		col->setFriction(1);
		pWorld->addCollisionObject(col, 2, 1 + 2);

		pMultiBody->getLink(i).m_collider = col;
	}
}



class CommonExampleInterface* ReducedCollideCreateFunc(struct CommonExampleOptions& options)
{
    return new ReducedCollide(options.m_guiHelper);
}


