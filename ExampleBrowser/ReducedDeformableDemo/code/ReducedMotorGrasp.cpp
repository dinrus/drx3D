#include "../ReducedMotorGrasp.h"
#include <drx/Core/Core.h>
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBody.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodyHelpers.h>
#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <stdio.h>  //printf debugging

#include <drx3D/Common/Interfaces/CommonDeformableBodyBase.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Importers/URDF/UrdfImporter.h>
#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/Common/b3FileUtils.h>

///The ReducedMotorGrasp shows grasping a volumetric deformable objects with multibody gripper with moter constraints.
static Scalar sGripperVerticalVelocity = 0.f;
static Scalar sGripperClosingTargetVelocity = 0.f;
static Scalar damping_alpha = 0.0;
static Scalar damping_beta = 0.0001;
static i32 num_modes = 20;
static float friction = 1.;
struct TetraCube
{
#include "../../SoftDemo/cube.inl"
};

struct TetraBunny
{
#include "../../SoftDemo/bunny.inl"
};

static bool supportsJointMotor(MultiBody* mb, i32 mbLinkIndex)
{
    bool canHaveMotor = (mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::eRevolute
                         || mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::ePrismatic);
    return canHaveMotor;
}

class ReducedMotorGrasp : public CommonDeformableBodyBase
{
    AlignedObjectArray<DeformableLagrangianForce*> m_forces;
public:
	ReducedMotorGrasp(struct GUIHelperInterface* helper)
    :CommonDeformableBodyBase(helper)
	{
	}
	virtual ~ReducedMotorGrasp()
	{
	}
	void initPhysics();

	void exitPhysics();

    void Ctor_RbUpStack()
    {
        float mass = 8;
        CollisionShape* shape = new BoxShape(Vec3(2, 0.25, 0.5));
        Transform2 startTransform;
        startTransform.setIdentity();

        startTransform.setOrigin(Vec3(0,0.25,0));
        RigidBody* rb1 = createRigidBody(mass, startTransform, shape);
        rb1->setLinearVelocity(Vec3(0, 0, 0));
        rb1->setFriction(0.7);
    }

	void resetCamera()
	{
        // float dist = 0.3;
        // float pitch = -45;
        // float yaw = 100;
        // float targetPos[3] = {0, -0.1, 0};
        float dist = 0.4;
        float pitch = -25;
        float yaw = 90;
        float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

    MultiBody* createFeatherstoneMultiBody(MultiBodyDynamicsWorld* pWorld,const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool floating);

    void addColliders(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents);

    MultiBody* createFeatherstoneMultiBody_testMultiDof(MultiBodyDynamicsWorld* pWorld, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical, bool floating);

    void stepSimulation(float deltaTime)
    {
        double fingerTargetVelocities[2] = {sGripperVerticalVelocity, sGripperClosingTargetVelocity};
        i32 num_multiBody = getDeformableDynamicsWorld()->getNumMultibodies();
        for (i32 i = 0; i < num_multiBody; ++i)
        {
            MultiBody* mb = getDeformableDynamicsWorld()->MultiBodyDynamicsWorld::getMultiBody(i);
            mb->setBaseVel(Vec3(0,sGripperVerticalVelocity, 0));
            i32 dofIndex = 6;  //skip the 3 linear + 3 angular degree of freedom entries of the base
            for (i32 link = 0; link < mb->getNumLinks(); link++)
            {
                if (supportsJointMotor(mb, link))
                {
                    MultiBodyJointMotor* motor = (MultiBodyJointMotor*)mb->getLink(link).m_userPtr;
                    if (motor)
                    {
                        if (dofIndex == 6)
                        {
                            motor->setVelocityTarget(-fingerTargetVelocities[1], 1);
                            motor->setMaxAppliedImpulse(10);
                        }
                        if (dofIndex == 7)
                        {
                            motor->setVelocityTarget(fingerTargetVelocities[1], 1);
                            motor->setMaxAppliedImpulse(10);
                        }
                        motor->setMaxAppliedImpulse(25);
                    }
                }
                dofIndex += mb->getLink(link).m_dofCount;
            }
        }

        //use a smaller internal timestep, there are stability issues
        float internalTimeStep = 1. / 240.f;
        m_dynamicsWorld->stepSimulation(deltaTime, 4, internalTimeStep);
        // float internalTimeStep = 1. / 60.f;
        // m_dynamicsWorld->stepSimulation(deltaTime, 1, internalTimeStep);
    }

    void createGrip()
    {
        i32 count = 2;
        float mass = 2;
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

    virtual const DeformableMultiBodyDynamicsWorld* getDeformableDynamicsWorld() const
    {
        return (DeformableMultiBodyDynamicsWorld*)m_dynamicsWorld;
    }

    virtual DeformableMultiBodyDynamicsWorld* getDeformableDynamicsWorld()
    {
        return (DeformableMultiBodyDynamicsWorld*)m_dynamicsWorld;
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
				SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), fDrawFlags::Faces);// deformableWorld->getDrawFlags());
            }
            // for (i32 p = 0; p < rsb->m_contactNodesList.size(); ++p)
            // {
            //     i32 index = rsb->m_contactNodesList[p];
            //     deformableWorld->getDebugDrawer()->drawSphere(rsb->m_nodes[index].m_x, 0.2, Vec3(0, 1, 0));
            // }
        }
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


void ReducedMotorGrasp::initPhysics()
{
	m_guiHelper->setUpAxis(1);

    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();
    ReducedDeformableBodySolver* reducedSoftBodySolver = new ReducedDeformableBodySolver();
    // Vec3 gravity = Vec3(0, 0, 0);
    Vec3 gravity = Vec3(0, -9.81, 0);
    reducedSoftBodySolver->setGravity(gravity);

	DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(reducedSoftBodySolver);
	m_solver = sol;

	m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, reducedSoftBodySolver);
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
    // getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.1;
    // getDeformableDynamicsWorld()->getSolverInfo().m_deformable_cfm = 0;
    // getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 150;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
    m_maxPickingForce = 0.001;
    // build a gripper
    {
        bool damping = true;
        bool gyro = false;
        bool canSleep = false;
        bool selfCollide = true;
        i32 numLinks = 2;
        // Vec3 linkHalfExtents(0.02, 0.018, .003);
        // Vec3 baseHalfExtents(0.02, 0.002, .002);
        Vec3 linkHalfExtents(0.03, 0.04, 0.006);
        Vec3 baseHalfExtents(0.02, 0.015, 0.015);
        Vec3 basePosition(0, 0.3, 0);
        // btMultiBody* mbC = createFeatherstoneMultiBody(getDeformableDynamicsWorld(), Vec3(0.f, 0.05f,0.f), baseHalfExtents, linkHalfExtents, false);
        MultiBody* mbC = createFeatherstoneMultiBody(getDeformableDynamicsWorld(), basePosition, baseHalfExtents, linkHalfExtents, false);

        mbC->setCanSleep(canSleep);
        mbC->setHasSelfCollision(selfCollide);
        mbC->setUseGyroTerm(gyro);

        for (i32 i = 0; i < numLinks; i++)
        {
            i32 mbLinkIndex = i;
            double maxMotorImpulse = 1;

            if (supportsJointMotor(mbC, mbLinkIndex))
            {
                i32 dof = 0;
                Scalar desiredVelocity = 0.f;
                MultiBodyJointMotor* motor = new MultiBodyJointMotor(mbC, mbLinkIndex, dof, desiredVelocity, maxMotorImpulse);
                motor->setPositionTarget(0, 0);
                motor->setVelocityTarget(0, 1);
                mbC->getLink(mbLinkIndex).m_userPtr = motor;
                getDeformableDynamicsWorld()->addMultiBodyConstraint(motor);
                motor->finalizeMultiDof();
            }
        }

        if (!damping)
        {
            mbC->setLinearDamping(0.0f);
            mbC->setAngularDamping(0.0f);
        }
        else
        {
            mbC->setLinearDamping(0.04f);
            mbC->setAngularDamping(0.04f);
        }
        Scalar q0 = 0.f * SIMD_PI / 180.f;
        if (numLinks > 0)
            mbC->setJointPosMultiDof(0, &q0);
        addColliders(mbC, getDeformableDynamicsWorld(), baseHalfExtents, linkHalfExtents);
    }

    //create a ground
    {
        CollisionShape* groundShape = new BoxShape(Vec3(Scalar(10.), Scalar(5.), Scalar(10.)));
        groundShape->setMargin(0.001);
        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, -5.1, 0));
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
        m_dynamicsWorld->addRigidBody(body,1,1+2);
    }

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
        rsb->getCollisionShape()->setMargin(0.001);

        rsb->setStiffnessScale(100);
        rsb->setDamping(damping_alpha, damping_beta);

        rsb->scale(Vec3(0.075, 0.075, 0.075));
        rsb->setTotalMass(1);

        Transform2 init_transform;
        init_transform.setIdentity();
        init_transform.setOrigin(Vec3(0, 0.1, 0));
        // init_transform.setRotation(Quat(SIMD_PI / 2.0, 0, SIMD_PI / 2.0));
        rsb->transform(init_transform);

        // rsb->setRigidVelocity(Vec3(0, 1, 0));

        rsb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        rsb->m_cfg.kCHR = 1; // collision hardness with rigid body
        rsb->m_cfg.kDF = 0;
        rsb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        rsb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDN;
        rsb->m_sleepingThreshold = 0;
        SoftBodyHelpers::generateBoundaryFaces(rsb);
    }

    // Ctor_RbUpStack();

    getDeformableDynamicsWorld()->setImplicit(false);
    getDeformableDynamicsWorld()->setLineSearch(false);
    getDeformableDynamicsWorld()->setUseProjection(false);
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_erp = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_cfm = 0.2;
    getDeformableDynamicsWorld()->getSolverInfo().m_friction = 1;
    getDeformableDynamicsWorld()->getSolverInfo().m_deformable_maxErrorReduction = Scalar(200);
    getDeformableDynamicsWorld()->getSolverInfo().m_leastSquaresResidualThreshold = 1e-6;
    getDeformableDynamicsWorld()->getSolverInfo().m_splitImpulse = false;
    getDeformableDynamicsWorld()->getSolverInfo().m_numIterations = 200;
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);

    {
        SliderParams slider("Moving velocity", &sGripperVerticalVelocity);
        // slider.m_minVal = -.02;
        // slider.m_maxVal = .02;
        slider.m_minVal = -.2;
        slider.m_maxVal = .2;
        m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }

    {
        SliderParams slider("Closing velocity", &sGripperClosingTargetVelocity);
        slider.m_minVal = -1;
        slider.m_maxVal = 1;
        m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }

}

void ReducedMotorGrasp::exitPhysics()
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

MultiBody* ReducedMotorGrasp::createFeatherstoneMultiBody(MultiBodyDynamicsWorld* pWorld, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool floating)
{
    //init the base
    Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
    float baseMass = 55;
    float linkMass = 55;
    i32 numLinks = 2;

    if (baseMass)
    {
        CollisionShape* pTempBox = new BoxShape(Vec3(baseHalfExtents[0], baseHalfExtents[1], baseHalfExtents[2]));
        pTempBox->calculateLocalInertia(baseMass, baseInertiaDiag);
        delete pTempBox;
    }

    bool canSleep = false;
    MultiBody* pMultiBody = new MultiBody(numLinks, baseMass, baseInertiaDiag, !floating, canSleep);

    Quat baseOriQuat(0.f, 0.f, 0.f, 1.f);
    pMultiBody->setBasePos(basePosition);
    pMultiBody->setWorldToBaseRot(baseOriQuat);

    //init the links
    Vec3 hingeJointAxis(1, 0, 0);

    Vec3 linkInertiaDiag(0.f, 0.f, 0.f);

    CollisionShape* pTempBox = new BoxShape(Vec3(linkHalfExtents[0], linkHalfExtents[1], linkHalfExtents[2]));
    pTempBox->calculateLocalInertia(linkMass, linkInertiaDiag);
    delete pTempBox;

    //y-axis assumed up
    AlignedObjectArray<Vec3> parentComToCurrentCom;
    parentComToCurrentCom.push_back(Vec3(0, -linkHalfExtents[1] * 2.f, -baseHalfExtents[2] * 2.f));
    parentComToCurrentCom.push_back(Vec3(0, -linkHalfExtents[1] * 2.f, +baseHalfExtents[2] * 2.f));//par body's COM to cur body's COM offset


    Vec3 currentPivotToCurrentCom(0, -linkHalfExtents[1]*2.f, 0);                         //cur body's COM to cur body's PIV offset

    AlignedObjectArray<Vec3> parentComToCurrentPivot;
    parentComToCurrentPivot.push_back(Vec3(parentComToCurrentCom[0] - currentPivotToCurrentCom));
    parentComToCurrentPivot.push_back(Vec3(parentComToCurrentCom[1] - currentPivotToCurrentCom));//par body's COM to cur body's PIV offset

    //////
    Scalar q0 = 0.f * SIMD_PI / 180.f;
    Quat quat0(Vec3(0, 1, 0).normalized(), q0);
    quat0.normalize();
    /////

    for (i32 i = 0; i < numLinks; ++i)
    {
        pMultiBody->setupRevolute(i, linkMass, linkInertiaDiag, - 1, Quat(0.f, 0.f, 0.f, 1.f), hingeJointAxis, parentComToCurrentPivot[i], currentPivotToCurrentCom, true);
    }
    pMultiBody->finalizeMultiDof();
    ///
    pWorld->addMultiBody(pMultiBody);
    ///
    return pMultiBody;
}

void ReducedMotorGrasp::addColliders(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents)
{
    AlignedObjectArray<Quat> world_to_local;
    world_to_local.resize(pMultiBody->getNumLinks() + 1);

    AlignedObjectArray<Vec3> local_origin;
    local_origin.resize(pMultiBody->getNumLinks() + 1);
    world_to_local[0] = pMultiBody->getWorldToBaseRot();
    local_origin[0] = pMultiBody->getBasePos();

    {
        Scalar quat[4] = {-world_to_local[0].x(), -world_to_local[0].y(), -world_to_local[0].z(), world_to_local[0].w()};

        if (1)
        {
            CollisionShape* box = new BoxShape(baseHalfExtents);
            box->setMargin(0.001);
            MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, -1);
            col->setCollisionShape(box);

            Transform2 tr;
            tr.setIdentity();
            tr.setOrigin(local_origin[0]);
            tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
            col->setWorldTransform(tr);

            pWorld->addCollisionObject(col, 2, 1 + 2);

            col->setFriction(friction);
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

        Scalar quat[4] = {-world_to_local[i + 1].x(), -world_to_local[i + 1].y(), -world_to_local[i + 1].z(), world_to_local[i + 1].w()};

        CollisionShape* box = new BoxShape(linkHalfExtents);
        box->setMargin(0.001);
        MultiBodyLinkCollider* col = new MultiBodyLinkCollider(pMultiBody, i);

        col->setCollisionShape(box);
        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(posr);
        tr.setRotation(Quat(quat[0], quat[1], quat[2], quat[3]));
        col->setWorldTransform(tr);
        col->setFriction(friction);
        pWorld->addCollisionObject(col, 2, 1 + 2);

        pMultiBody->getLink(i).m_collider = col;
    }
}

class CommonExampleInterface* ReducedMotorGraspCreateFunc(struct CommonExampleOptions& options)
{
	return new ReducedMotorGrasp(options.m_guiHelper);
}


