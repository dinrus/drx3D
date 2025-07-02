#include "../DeformableMultibody.h"
///BulletDynamicsCommon.h is the main drx3D include file, contains most common include files.
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/DeformableMultiBodyDynamicsWorld.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <drx3D/Physics/SoftBody/DeformableBodySolver.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraintSolver.h>
#include <stdio.h>  //printf debugging
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include <drx3D/Common/ResourcePath.h>
#include "../../SoftDemo/BunnyMesh.h"
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointFeedback.h>

#include <drx3D/Common/Interfaces/CommonDeformableBodyBase.h>
#include <drx3D/Common/ResourcePath.h>
///The DeformableMultibody demo deformable bodies self-collision
static bool g_floatingBase = true;
static float friction = 1.;
class DeformableMultibody : public CommonDeformableBodyBase
{
public:
	DeformableMultibody(struct GUIHelperInterface* helper)
    :CommonDeformableBodyBase(helper)
	{
	}
    
	virtual ~DeformableMultibody()
	{
	}
    
	void initPhysics();

	void exitPhysics();

	void resetCamera()
	{
        float dist = 30;
        float pitch = -30;
        float yaw = 100;
        float targetPos[3] = {0, -10, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
    
    virtual void stepSimulation(float deltaTime);
    
    MultiBody* createFeatherstoneMultiBody_testMultiDof(class MultiBodyDynamicsWorld* world, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical = false, bool floating = false);
    
    void addColliders_testMultiDof(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents);
    
    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
        DeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();
        
        for (i32 i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
        {
            SoftBody* psb = (SoftBody*)deformableWorld->getSoftBodyArray()[i];
            {
                SoftBodyHelpers::DrawFrame(psb, deformableWorld->getDebugDrawer());
				SoftBodyHelpers::Draw(psb, deformableWorld->getDebugDrawer(), fDrawFlags::Faces);// deformableWorld->getDrawFlags());
            }
        }
    }
};

void DeformableMultibody::initPhysics()
{
	m_guiHelper->setUpAxis(1);

	///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new DbvtBroadphase();
    DeformableBodySolver* deformableBodySolver = new DeformableBodySolver();
    DeformableMultiBodyConstraintSolver* sol;
    sol = new DeformableMultiBodyConstraintSolver;
    sol->setDeformableSolver(deformableBodySolver);
	m_solver = sol;
    
    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, deformableBodySolver);
    Vec3 gravity = Vec3(0, -10, 0);
	m_dynamicsWorld->setGravity(gravity);
    getDeformableDynamicsWorld()->getWorldInfo().m_gravity = gravity;
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.setDefaultVoxelsz(0.25);
	getDeformableDynamicsWorld()->getWorldInfo().m_sparsesdf.Reset();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    {
        ///create a ground
        CollisionShape* groundShape = new BoxShape(Vec3(Scalar(150.), Scalar(25.), Scalar(150.)));

        m_collisionShapes.push_back(groundShape);

        Transform2 groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(Vec3(0, -40, 0));
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
        body->setFriction(0.5);

        //add the ground to the dynamics world
        m_dynamicsWorld->addRigidBody(body,1,1+2);
    }

    {
        bool damping = true;
        bool gyro = false;
        i32 numLinks = 4;
        bool spherical = false;  //set it ot false -to use 1DoF hinges instead of 3DoF sphericals
        bool canSleep = false;
        bool selfCollide = true;
        Vec3 linkHalfExtents(.4, 1, .4);
        Vec3 baseHalfExtents(.4, 1, .4);
        
        MultiBody* mbC = createFeatherstoneMultiBody_testMultiDof(m_dynamicsWorld, numLinks, Vec3(0.f, 10.f,0.f), linkHalfExtents, baseHalfExtents, spherical, g_floatingBase);
        
        mbC->setCanSleep(canSleep);
        mbC->setHasSelfCollision(selfCollide);
        mbC->setUseGyroTerm(gyro);
        //
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

        if (numLinks > 0)
        {
            Scalar q0 = 0.f * SIMD_PI / 180.f;
            if (!spherical)
            {
                mbC->setJointPosMultiDof(0, &q0);
            }
            else
            {
                Quat quat0(Vec3(1, 1, 0).normalized(), q0);
                quat0.normalize();
                mbC->setJointPosMultiDof(0, quat0);
            }
        }
        ///
        addColliders_testMultiDof(mbC, m_dynamicsWorld, baseHalfExtents, linkHalfExtents);
    }
    
    // create a patch of cloth
    {
        Scalar h = 0;
        const Scalar s = 4;
        SoftBody* psb = SoftBodyHelpers::CreatePatch(getDeformableDynamicsWorld()->getWorldInfo(), Vec3(-s, h, -s),
                                                         Vec3(+s, h, -s),
                                                         Vec3(-s, h, +s),
                                                         Vec3(+s, h, +s),
                                                         20,20,
//                                                         3,3,
                                                         1 + 2 + 4 + 8, true);

        psb->getCollisionShape()->setMargin(0.025);
        psb->generateBendingConstraints(2);
        psb->setTotalMass(1);
        psb->m_cfg.kKHR = 1; // collision hardness with kinematic objects
        psb->m_cfg.kCHR = 1; // collision hardness with rigid body
        psb->m_cfg.kDF = 2;
        psb->m_cfg.collisions = SoftBody::fCollision::SDF_RD;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_RDF;
        psb->m_cfg.collisions |= SoftBody::fCollision::SDF_MDF;
        psb->setCollisionFlags(0);
        getDeformableDynamicsWorld()->addSoftBody(psb);

        DeformableMassSpringForce* mass_spring = new DeformableMassSpringForce(30, 1, true);
        getDeformableDynamicsWorld()->addForce(psb, mass_spring);
        m_forces.push_back(mass_spring);
        
        DeformableGravityForce* gravity_force =  new DeformableGravityForce(gravity);
        getDeformableDynamicsWorld()->addForce(psb, gravity_force);
        m_forces.push_back(gravity_force);
    }
    getDeformableDynamicsWorld()->setImplicit(false);
	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void DeformableMultibody::exitPhysics()
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

void DeformableMultibody::stepSimulation(float deltaTime)
{
//    getDeformableDynamicsWorld()->getMultiBodyDynamicsWorld()->stepSimulation(deltaTime);
    m_dynamicsWorld->stepSimulation(deltaTime, 5, 1./250.);
}


MultiBody* DeformableMultibody::createFeatherstoneMultiBody_testMultiDof(MultiBodyDynamicsWorld* pWorld, i32 numLinks, const Vec3& basePosition, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents, bool spherical, bool floating)
{
    //init the base
    Vec3 baseInertiaDiag(0.f, 0.f, 0.f);
    float baseMass = .1f;
    
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
    Vec3 vel(0, 0, 0);
    //    pMultiBody->setBaseVel(vel);
    
    //init the links
    Vec3 hingeJointAxis(1, 0, 0);
    float linkMass = .1f;
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
    
    ///
    pWorld->addMultiBody(pMultiBody);
    ///
    return pMultiBody;
}

void DeformableMultibody::addColliders_testMultiDof(MultiBody* pMultiBody, MultiBodyDynamicsWorld* pWorld, const Vec3& baseHalfExtents, const Vec3& linkHalfExtents)
{
    AlignedObjectArray<Quat> world_to_local;
    world_to_local.resize(pMultiBody->getNumLinks() + 1);
    
    AlignedObjectArray<Vec3> local_origin;
    local_origin.resize(pMultiBody->getNumLinks() + 1);
    world_to_local[0] = pMultiBody->getWorldToBaseRot();
    local_origin[0] = pMultiBody->getBasePos();
    
    {

        Scalar quat[4] = {-world_to_local[0].x(), -world_to_local[0].y(), -world_to_local[0].z(), world_to_local[0].w()};
        
        CollisionShape* box = new BoxShape(baseHalfExtents);
        box->setMargin(0.01);
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
class CommonExampleInterface* DeformableMultibodyCreateFunc(struct CommonExampleOptions& options)
{
	return new DeformableMultibody(options.m_guiHelper);
}


