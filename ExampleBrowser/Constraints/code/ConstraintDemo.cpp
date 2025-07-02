#include "../ConstraintDemo.h"
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <stdio.h>  //printf debugging
#include <cmath>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

///AllConstraintDemo shows how to create a constraint, like Hinge or btGenericD6constraint
class AllConstraintDemo : public CommonRigidBodyBase
{
    //keep track of variables to delete memory at the end

    void setupEmptyDynamicsWorld();

public:
    AllConstraintDemo(struct GUIHelperInterface* helper);

    virtual ~AllConstraintDemo();

    virtual void initPhysics();

    virtual void exitPhysics();

    virtual void resetCamera()
    {
        float dist = 27;
        float pitch = -30;
        float yaw = 720;
        float targetPos[3] = {2, 0, -10};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }

    virtual bool keyboardCallback(i32 key, i32 state);

    // for cone-twist motor driving
    float m_Time;
    class ConeTwistConstraint* m_ctc;
};

#define ENABLE_ALL_DEMOS 1

#define CUBE_HALF_EXTENTS 1.f

#define SIMD_PI_2 ((SIMD_PI)*0.5f)
#define SIMD_PI_4 ((SIMD_PI)*0.25f)

Transform2 sliderTransform;
Vec3 lowerSliderLimit = Vec3(-10, 0, 0);
Vec3 hiSliderLimit = Vec3(10, 0, 0);

RigidBody* d6body0 = 0;

HingeConstraint* spDoorHinge = NULL;
HingeConstraint* spHingeDynAB = NULL;
Generic6DofConstraint* spSlider6Dof = NULL;

static bool s_bTestConeTwistMotor = false;

void AllConstraintDemo::setupEmptyDynamicsWorld()
{
    m_collisionConfiguration = new DefaultCollisionConfiguration();
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
    m_broadphase = new DbvtBroadphase();
    m_solver = new SequentialImpulseConstraintSolver();
    m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
}

void AllConstraintDemo::initPhysics()
{
    m_guiHelper->setUpAxis(1);

    m_Time = 0;

    setupEmptyDynamicsWorld();

    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    //CollisionShape* groundShape = new BoxShape(Vec3(Scalar(50.),Scalar(40.),Scalar(50.)));
    CollisionShape* groundShape = new StaticPlaneShape(Vec3(0, 1, 0), 40);

    m_collisionShapes.push_back(groundShape);
    Transform2 groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(Vec3(0, -56, 0));
    RigidBody* groundBody;
    groundBody = createRigidBody(0, groundTransform, groundShape);

    CollisionShape* shape = new BoxShape(Vec3(CUBE_HALF_EXTENTS, CUBE_HALF_EXTENTS, CUBE_HALF_EXTENTS));
    m_collisionShapes.push_back(shape);
    Transform2 trans;
    trans.setIdentity();
    trans.setOrigin(Vec3(0, 20, 0));

    float mass = 1.f;

#if ENABLE_ALL_DEMOS
    ///gear constraint demo

#define THETA SIMD_PI / 4.f
#define L_1 (2 - std::tan(THETA))
#define L_2 (1 / std::cos(THETA))
#define RATIO L_2 / L_1

    RigidBody* bodyA = 0;
    RigidBody* bodyB = 0;

    {
        CollisionShape* cylA = new CylinderShape(Vec3(0.2, 0.25, 0.2));
        CollisionShape* cylB = new CylinderShape(Vec3(L_1, 0.025, L_1));
        CompoundShape* cyl0 = new CompoundShape();
        cyl0->addChildShape(Transform2::getIdentity(), cylA);
        cyl0->addChildShape(Transform2::getIdentity(), cylB);

        Scalar mass = 6.28;
        Vec3 localInertia;
        cyl0->calculateLocalInertia(mass, localInertia);
        RigidBody::RigidBodyConstructionInfo ci(mass, 0, cyl0, localInertia);
        ci.m_startWorldTransform.setOrigin(Vec3(-8, 1, -8));

        RigidBody* body = new RigidBody(ci);  //1,0,cyl0,localInertia);
        m_dynamicsWorld->addRigidBody(body);
        body->setLinearFactor(Vec3(0, 0, 0));
        body->setAngularFactor(Vec3(0, 1, 0));
        bodyA = body;
    }

    {
        CollisionShape* cylA = new CylinderShape(Vec3(0.2, 0.26, 0.2));
        CollisionShape* cylB = new CylinderShape(Vec3(L_2, 0.025, L_2));
        CompoundShape* cyl0 = new CompoundShape();
        cyl0->addChildShape(Transform2::getIdentity(), cylA);
        cyl0->addChildShape(Transform2::getIdentity(), cylB);

        Scalar mass = 6.28;
        Vec3 localInertia;
        cyl0->calculateLocalInertia(mass, localInertia);
        RigidBody::RigidBodyConstructionInfo ci(mass, 0, cyl0, localInertia);
        ci.m_startWorldTransform.setOrigin(Vec3(-10, 2, -8));

        Quat orn(Vec3(0, 0, 1), -THETA);
        ci.m_startWorldTransform.setRotation(orn);

        RigidBody* body = new RigidBody(ci);  //1,0,cyl0,localInertia);
        body->setLinearFactor(Vec3(0, 0, 0));
        HingeConstraint* hinge = new HingeConstraint(*body, Vec3(0, 0, 0), Vec3(0, 1, 0), true);
        m_dynamicsWorld->addConstraint(hinge);
        bodyB = body;
        body->setAngularVelocity(Vec3(0, 3, 0));

        m_dynamicsWorld->addRigidBody(body);
    }

    Vec3 axisA(0, 1, 0);
    Vec3 axisB(0, 1, 0);
    Quat orn(Vec3(0, 0, 1), -THETA);
    Matrix3x3 mat(orn);
    axisB = mat.getRow(1);

    GearConstraint* gear = new GearConstraint(*bodyA, *bodyB, axisA, axisB, RATIO);
    m_dynamicsWorld->addConstraint(gear, true);

#endif

#if ENABLE_ALL_DEMOS
    //point to point constraint with a breaking threshold
    {
        trans.setIdentity();
        trans.setOrigin(Vec3(1, 30, -5));
        createRigidBody(mass, trans, shape);
        trans.setOrigin(Vec3(0, 0, -5));

        RigidBody* body0 = createRigidBody(mass, trans, shape);
        trans.setOrigin(Vec3(2 * CUBE_HALF_EXTENTS, 20, 0));
        mass = 1.f;
        //  RigidBody* body1 = 0;//createRigidBody( mass,trans,shape);
        Vec3 pivotInA(CUBE_HALF_EXTENTS, CUBE_HALF_EXTENTS, 0);
        TypedConstraint* p2p = new Point2PointConstraint(*body0, pivotInA);
        m_dynamicsWorld->addConstraint(p2p);
        p2p->setBreakingImpulseThreshold(10.2);
        p2p->setDbgDrawSize(Scalar(5.f));
    }
#endif

#if ENABLE_ALL_DEMOS
    //point to point constraint (ball socket)
    {
        RigidBody* body0 = createRigidBody(mass, trans, shape);
        trans.setOrigin(Vec3(2 * CUBE_HALF_EXTENTS, 20, 0));

        mass = 1.f;
        //      RigidBody* body1 = 0;//createRigidBody( mass,trans,shape);
        //      RigidBody* body1 = createRigidBody( 0.0,trans,0);
        //body1->setActivationState(DISABLE_DEACTIVATION);
        //body1->setDamping(0.3,0.3);

        Vec3 pivotInA(CUBE_HALF_EXTENTS, -CUBE_HALF_EXTENTS, -CUBE_HALF_EXTENTS);
        Vec3 axisInA(0, 0, 1);

        //  Vec3 pivotInB = body1 ? body1->getCenterOfMassTransform().inverse()(body0->getCenterOfMassTransform()(pivotInA)) : pivotInA;
        //      Vec3 axisInB = body1?
        //          (body1->getCenterOfMassTransform().getBasis().inverse()*(body1->getCenterOfMassTransform().getBasis() * axisInA)) :
        body0->getCenterOfMassTransform().getBasis() * axisInA;

#define P2P
#ifdef P2P
        TypedConstraint* p2p = new Point2PointConstraint(*body0, pivotInA);
        //TypedConstraint* p2p = new Point2PointConstraint(*body0,*body1,pivotInA,pivotInB);
        //TypedConstraint* hinge = new btHingeConstraint(*body0,*body1,pivotInA,pivotInB,axisInA,axisInB);
        m_dynamicsWorld->addConstraint(p2p);
        p2p->setDbgDrawSize(Scalar(5.f));
#else
        btHingeConstraint* hinge = new btHingeConstraint(*body0, pivotInA, axisInA);

        //use zero targetVelocity and a small maxMotorImpulse to simulate joint friction
        //float targetVelocity = 0.f;
        //float maxMotorImpulse = 0.01;
        float targetVelocity = 1.f;
        float maxMotorImpulse = 1.0f;
        hinge->enableAngularMotor(true, targetVelocity, maxMotorImpulse);
        m_dynamicsWorld->addConstraint(hinge);
        hinge->setDbgDrawSize(Scalar(5.f));
#endif  //P2P
    }
#endif

#if ENABLE_ALL_DEMOS
    {
        Transform2 trans;
        trans.setIdentity();
        Vec3 worldPos(-20, 0, 30);
        trans.setOrigin(worldPos);

        Transform2 frameInA, frameInB;
        frameInA = Transform2::getIdentity();
        frameInB = Transform2::getIdentity();

        RigidBody* pRbA1 = createRigidBody(mass, trans, shape);
        //  RigidBody* pRbA1 = createRigidBody(0.f, trans, shape);
        pRbA1->setActivationState(DISABLE_DEACTIVATION);

        // add dynamic rigid body B1
        worldPos.setVal(-30, 0, 30);
        trans.setOrigin(worldPos);
        RigidBody* pRbB1 = createRigidBody(mass, trans, shape);
        //  RigidBody* pRbB1 = createRigidBody(0.f, trans, shape);
        pRbB1->setActivationState(DISABLE_DEACTIVATION);

        // create slider constraint between A1 and B1 and add it to world

        SliderConstraint* spSlider1 = new SliderConstraint(*pRbA1, *pRbB1, frameInA, frameInB, true);
        //  spSlider1 = new SliderConstraint(*pRbA1, *pRbB1, frameInA, frameInB, false);
        spSlider1->setLowerLinLimit(-15.0F);
        spSlider1->setUpperLinLimit(-5.0F);
        //  spSlider1->setLowerLinLimit(5.0F);
        //  spSlider1->setUpperLinLimit(15.0F);
        //  spSlider1->setLowerLinLimit(-10.0F);
        //  spSlider1->setUpperLinLimit(-10.0F);

        spSlider1->setLowerAngLimit(-SIMD_PI / 3.0F);
        spSlider1->setUpperAngLimit(SIMD_PI / 3.0F);

        m_dynamicsWorld->addConstraint(spSlider1, true);
        spSlider1->setDbgDrawSize(Scalar(5.f));
    }
#endif

#if ENABLE_ALL_DEMOS
    //create a slider, using the generic D6 constraint
    {
        mass = 1.f;
        Vec3 sliderWorldPos(0, 10, 0);
        Vec3 sliderAxis(1, 0, 0);
        Scalar angle = 0.f;  //SIMD_RADS_PER_DEG * 10.f;
        Matrix3x3 sliderOrientation(Quat(sliderAxis, angle));
        trans.setIdentity();
        trans.setOrigin(sliderWorldPos);
        //trans.setBasis(sliderOrientation);
        sliderTransform = trans;

        d6body0 = createRigidBody(mass, trans, shape);
        d6body0->setActivationState(DISABLE_DEACTIVATION);
        RigidBody* fixedBody1 = createRigidBody(0, trans, 0);
        m_dynamicsWorld->addRigidBody(fixedBody1);

        Transform2 frameInA, frameInB;
        frameInA = Transform2::getIdentity();
        frameInB = Transform2::getIdentity();
        frameInA.setOrigin(Vec3(0., 5., 0.));
        frameInB.setOrigin(Vec3(0., 5., 0.));

        //      bool useLinearReferenceFrameA = false;//use fixed frame B for linear llimits
        bool useLinearReferenceFrameA = true;  //use fixed frame A for linear llimits
        spSlider6Dof = new Generic6DofConstraint(*fixedBody1, *d6body0, frameInA, frameInB, useLinearReferenceFrameA);
        spSlider6Dof->setLinearLowerLimit(lowerSliderLimit);
        spSlider6Dof->setLinearUpperLimit(hiSliderLimit);

        //range should be small, otherwise singularities will 'explode' the constraint
        //      spSlider6Dof->setAngularLowerLimit(Vec3(-1.5,0,0));
        //      spSlider6Dof->setAngularUpperLimit(Vec3(1.5,0,0));
        //      spSlider6Dof->setAngularLowerLimit(Vec3(0,0,0));
        //      spSlider6Dof->setAngularUpperLimit(Vec3(0,0,0));
        spSlider6Dof->setAngularLowerLimit(Vec3(-SIMD_PI, 0, 0));
        spSlider6Dof->setAngularUpperLimit(Vec3(1.5, 0, 0));

        spSlider6Dof->getTranslationalLimitMotor()->m_enableMotor[0] = true;
        spSlider6Dof->getTranslationalLimitMotor()->m_targetVelocity[0] = -5.0f;
        spSlider6Dof->getTranslationalLimitMotor()->m_maxMotorForce[0] = 6.0f;

        m_dynamicsWorld->addConstraint(spSlider6Dof);
        spSlider6Dof->setDbgDrawSize(Scalar(5.f));
    }
#endif
#if ENABLE_ALL_DEMOS
    {  // create a door using hinge constraint attached to the world
        CollisionShape* pDoorShape = new BoxShape(Vec3(2.0f, 5.0f, 0.2f));
        m_collisionShapes.push_back(pDoorShape);
        Transform2 doorTrans;
        doorTrans.setIdentity();
        doorTrans.setOrigin(Vec3(-5.0f, -2.0f, 0.0f));
        RigidBody* pDoorBody = createRigidBody(1.0, doorTrans, pDoorShape);
        pDoorBody->setActivationState(DISABLE_DEACTIVATION);
        const Vec3 PivotA(10.f + 2.1f, -2.0f, 0.0f);  // right next to the door slightly outside
        Vec3 AxisA(0.0f, 1.0f, 0.0f);                 // pointing upwards, aka Y-axis

        spDoorHinge = new HingeConstraint(*pDoorBody, PivotA, AxisA);

        //      spDoorHinge->setLimit( 0.0f, SIMD_PI_2 );
        // test problem values
        //      spDoorHinge->setLimit( -SIMD_PI, SIMD_PI*0.8f);

        //      spDoorHinge->setLimit( 1.f, -1.f);
        //      spDoorHinge->setLimit( -SIMD_PI*0.8f, SIMD_PI);
        //      spDoorHinge->setLimit( -SIMD_PI*0.8f, SIMD_PI, 0.9f, 0.3f, 0.0f);
        //      spDoorHinge->setLimit( -SIMD_PI*0.8f, SIMD_PI, 0.9f, 0.01f, 0.0f); // "sticky limits"
        spDoorHinge->setLimit(-SIMD_PI * 0.25f, SIMD_PI * 0.25f);
        //      spDoorHinge->setLimit( 0.0f, 0.0f );
        m_dynamicsWorld->addConstraint(spDoorHinge);
        spDoorHinge->setDbgDrawSize(Scalar(5.f));

        //doorTrans.setOrigin(Vec3(-5.0f, 2.0f, 0.0f));
        //RigidBody* pDropBody = createRigidBody( 10.0, doorTrans, shape);
    }
#endif
#if ENABLE_ALL_DEMOS
    {  // create a generic 6DOF constraint

        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(10.), Scalar(6.), Scalar(0.)));
        tr.getBasis().setEulerZYX(0, 0, 0);
        //      RigidBody* pBodyA = createRigidBody( mass, tr, shape);
        RigidBody* pBodyA = createRigidBody(0.0, tr, shape);
        //      RigidBody* pBodyA = createRigidBody( 0.0, tr, 0);
        pBodyA->setActivationState(DISABLE_DEACTIVATION);

        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(0.), Scalar(6.), Scalar(0.)));
        tr.getBasis().setEulerZYX(0, 0, 0);
        RigidBody* pBodyB = createRigidBody(mass, tr, shape);
        //      RigidBody* pBodyB = createRigidBody(0.f, tr, shape);
        pBodyB->setActivationState(DISABLE_DEACTIVATION);

        Transform2 frameInA, frameInB;
        frameInA = Transform2::getIdentity();
        frameInA.setOrigin(Vec3(Scalar(-5.), Scalar(0.), Scalar(0.)));
        frameInB = Transform2::getIdentity();
        frameInB.setOrigin(Vec3(Scalar(5.), Scalar(0.), Scalar(0.)));

        Generic6DofConstraint* pGen6DOF = new Generic6DofConstraint(*pBodyA, *pBodyB, frameInA, frameInB, true);
        //      btGeneric6DofConstraint* pGen6DOF = new btGeneric6DofConstraint(*pBodyA, *pBodyB, frameInA, frameInB, false);
        pGen6DOF->setLinearLowerLimit(Vec3(-10., -2., -1.));
        pGen6DOF->setLinearUpperLimit(Vec3(10., 2., 1.));
        //      pGen6DOF->setLinearLowerLimit(Vec3(-10., 0., 0.));
        //      pGen6DOF->setLinearUpperLimit(Vec3(10., 0., 0.));
        //      pGen6DOF->setLinearLowerLimit(Vec3(0., 0., 0.));
        //      pGen6DOF->setLinearUpperLimit(Vec3(0., 0., 0.));

        //      pGen6DOF->getTranslationalLimitMotor()->m_enableMotor[0] = true;
        //      pGen6DOF->getTranslationalLimitMotor()->m_targetVelocity[0] = 5.0f;
        //      pGen6DOF->getTranslationalLimitMotor()->m_maxMotorForce[0] = 6.0f;

        //      pGen6DOF->setAngularLowerLimit(Vec3(0., SIMD_HALF_PI*0.9, 0.));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0., -SIMD_HALF_PI*0.9, 0.));
        //      pGen6DOF->setAngularLowerLimit(Vec3(0., 0., -SIMD_HALF_PI));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0., 0., SIMD_HALF_PI));

        pGen6DOF->setAngularLowerLimit(Vec3(-SIMD_HALF_PI * 0.5f, -0.75, -SIMD_HALF_PI * 0.8f));
        pGen6DOF->setAngularUpperLimit(Vec3(SIMD_HALF_PI * 0.5f, 0.75, SIMD_HALF_PI * 0.8f));
        //      pGen6DOF->setAngularLowerLimit(Vec3(0.f, -0.75, SIMD_HALF_PI * 0.8f));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0.f, 0.75, -SIMD_HALF_PI * 0.8f));
        //      pGen6DOF->setAngularLowerLimit(Vec3(0.f, -SIMD_HALF_PI * 0.8f, SIMD_HALF_PI * 1.98f));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0.f, SIMD_HALF_PI * 0.8f,  -SIMD_HALF_PI * 1.98f));

        //      pGen6DOF->setAngularLowerLimit(Vec3(-0.75,-0.5, -0.5));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0.75,0.5, 0.5));
        //      pGen6DOF->setAngularLowerLimit(Vec3(-0.75,0., 0.));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0.75,0., 0.));
        //      pGen6DOF->setAngularLowerLimit(Vec3(0., -0.7,0.));
        //      pGen6DOF->setAngularUpperLimit(Vec3(0., 0.7, 0.));
        //      pGen6DOF->setAngularLowerLimit(Vec3(-1., 0.,0.));
        //      pGen6DOF->setAngularUpperLimit(Vec3(1., 0., 0.));

        m_dynamicsWorld->addConstraint(pGen6DOF, true);
        pGen6DOF->setDbgDrawSize(Scalar(5.f));
    }
#endif
#if ENABLE_ALL_DEMOS
    {  // create a ConeTwist constraint

        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-10.), Scalar(5.), Scalar(0.)));
        tr.getBasis().setEulerZYX(0, 0, 0);
        RigidBody* pBodyA = createRigidBody(1.0, tr, shape);
        //      RigidBody* pBodyA = createRigidBody( 0.0, tr, shape);
        pBodyA->setActivationState(DISABLE_DEACTIVATION);

        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-10.), Scalar(-5.), Scalar(0.)));
        tr.getBasis().setEulerZYX(0, 0, 0);
        RigidBody* pBodyB = createRigidBody(0.0, tr, shape);
        //      RigidBody* pBodyB = createRigidBody(1.0, tr, shape);

        Transform2 frameInA, frameInB;
        frameInA = Transform2::getIdentity();
        frameInA.getBasis().setEulerZYX(0, 0, SIMD_PI_2);
        frameInA.setOrigin(Vec3(Scalar(0.), Scalar(-5.), Scalar(0.)));
        frameInB = Transform2::getIdentity();
        frameInB.getBasis().setEulerZYX(0, 0, SIMD_PI_2);
        frameInB.setOrigin(Vec3(Scalar(0.), Scalar(5.), Scalar(0.)));

        m_ctc = new ConeTwistConstraint(*pBodyA, *pBodyB, frameInA, frameInB);
        //      m_ctc->setLimit(Scalar(SIMD_PI_4), Scalar(SIMD_PI_4), Scalar(SIMD_PI) * 0.8f);
        //      m_ctc->setLimit(Scalar(SIMD_PI_4*0.6f), Scalar(SIMD_PI_4), Scalar(SIMD_PI) * 0.8f, 1.0f); // soft limit == hard limit
        m_ctc->setLimit(Scalar(SIMD_PI_4 * 0.6f), Scalar(SIMD_PI_4), Scalar(SIMD_PI) * 0.8f, 0.5f);
        m_dynamicsWorld->addConstraint(m_ctc, true);
        m_ctc->setDbgDrawSize(Scalar(5.f));
        // s_bTestConeTwistMotor = true; // use only with old solver for now
        s_bTestConeTwistMotor = false;
    }
#endif
#if ENABLE_ALL_DEMOS
    {  // Hinge connected to the world, with motor (to hinge motor with new and old constraint solver)
        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(0.), Scalar(0.), Scalar(0.)));
        RigidBody* pBody = createRigidBody(1.0, tr, shape);
        pBody->setActivationState(DISABLE_DEACTIVATION);
        const Vec3 PivotA(10.0f, 0.0f, 0.0f);
        Vec3 AxisA(0.0f, 0.0f, 1.0f);

        HingeConstraint* pHinge = new HingeConstraint(*pBody, PivotA, AxisA);
        //      pHinge->enableAngularMotor(true, -1.0, 0.165); // use for the old solver
        pHinge->enableAngularMotor(true, -1.0f, 1.65f);  // use for the new SIMD solver
        m_dynamicsWorld->addConstraint(pHinge);
        pHinge->setDbgDrawSize(Scalar(5.f));
    }
#endif

#if ENABLE_ALL_DEMOS
    {
        // create a universal joint using generic 6DOF constraint
        // create two rigid bodies
        // static bodyA (parent) on top:
        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(20.), Scalar(4.), Scalar(0.)));
        RigidBody* pBodyA = createRigidBody(0.0, tr, shape);
        pBodyA->setActivationState(DISABLE_DEACTIVATION);
        // dynamic bodyB (child) below it :
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(20.), Scalar(0.), Scalar(0.)));
        RigidBody* pBodyB = createRigidBody(1.0, tr, shape);
        pBodyB->setActivationState(DISABLE_DEACTIVATION);
        // add some (arbitrary) data to build constraint frames
        Vec3 parentAxis(1.f, 0.f, 0.f);
        Vec3 childAxis(0.f, 0.f, 1.f);
        Vec3 anchor(20.f, 2.f, 0.f);

        UniversalConstraint* pUniv = new UniversalConstraint(*pBodyA, *pBodyB, anchor, parentAxis, childAxis);
        pUniv->setLowerLimit(-SIMD_HALF_PI * 0.5f, -SIMD_HALF_PI * 0.5f);
        pUniv->setUpperLimit(SIMD_HALF_PI * 0.5f, SIMD_HALF_PI * 0.5f);
        // add constraint to world
        m_dynamicsWorld->addConstraint(pUniv, true);
        // draw constraint frames and limits for debugging
        pUniv->setDbgDrawSize(Scalar(5.f));
    }
#endif

#if ENABLE_ALL_DEMOS
    {  // create a generic 6DOF constraint with springs

        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-20.), Scalar(16.), Scalar(0.)));
        tr.getBasis().setEulerZYX(0, 0, 0);
        RigidBody* pBodyA = createRigidBody(0.0, tr, shape);
        pBodyA->setActivationState(DISABLE_DEACTIVATION);

        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-10.), Scalar(16.), Scalar(0.)));
        tr.getBasis().setEulerZYX(0, 0, 0);
        RigidBody* pBodyB = createRigidBody(1.0, tr, shape);
        pBodyB->setActivationState(DISABLE_DEACTIVATION);

        Transform2 frameInA, frameInB;
        frameInA = Transform2::getIdentity();
        frameInA.setOrigin(Vec3(Scalar(10.), Scalar(0.), Scalar(0.)));
        frameInB = Transform2::getIdentity();
        frameInB.setOrigin(Vec3(Scalar(0.), Scalar(0.), Scalar(0.)));

        Generic6DofSpringConstraint* pGen6DOFSpring = new Generic6DofSpringConstraint(*pBodyA, *pBodyB, frameInA, frameInB, true);
        pGen6DOFSpring->setLinearUpperLimit(Vec3(5., 0., 0.));
        pGen6DOFSpring->setLinearLowerLimit(Vec3(-5., 0., 0.));

        pGen6DOFSpring->setAngularLowerLimit(Vec3(0.f, 0.f, -1.5f));
        pGen6DOFSpring->setAngularUpperLimit(Vec3(0.f, 0.f, 1.5f));

        m_dynamicsWorld->addConstraint(pGen6DOFSpring, true);
        pGen6DOFSpring->setDbgDrawSize(Scalar(5.f));

        pGen6DOFSpring->enableSpring(0, true);
        pGen6DOFSpring->setStiffness(0, 39.478f);
        pGen6DOFSpring->setDamping(0, 0.5f);
        pGen6DOFSpring->enableSpring(5, true);
        pGen6DOFSpring->setStiffness(5, 39.478f);
        pGen6DOFSpring->setDamping(0, 0.3f);
        pGen6DOFSpring->setEquilibriumPoint();
    }
#endif
#if ENABLE_ALL_DEMOS
    {
        // create a Hinge2 joint
        // create two rigid bodies
        // static bodyA (parent) on top:
        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-20.), Scalar(4.), Scalar(0.)));
        RigidBody* pBodyA = createRigidBody(0.0, tr, shape);
        pBodyA->setActivationState(DISABLE_DEACTIVATION);
        // dynamic bodyB (child) below it :
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-20.), Scalar(0.), Scalar(0.)));
        RigidBody* pBodyB = createRigidBody(1.0, tr, shape);
        pBodyB->setActivationState(DISABLE_DEACTIVATION);
        // add some data to build constraint frames
        Vec3 parentAxis(0.f, 1.f, 0.f);
        Vec3 childAxis(1.f, 0.f, 0.f);
        Vec3 anchor(-20.f, 0.f, 0.f);
        Hinge2Constraint* pHinge2 = new Hinge2Constraint(*pBodyA, *pBodyB, anchor, parentAxis, childAxis);
        pHinge2->setLowerLimit(-SIMD_HALF_PI * 0.5f);
        pHinge2->setUpperLimit(SIMD_HALF_PI * 0.5f);
        // add constraint to world
        m_dynamicsWorld->addConstraint(pHinge2, true);
        // draw constraint frames and limits for debugging
        pHinge2->setDbgDrawSize(Scalar(5.f));
    }
#endif
#if ENABLE_ALL_DEMOS
    {
        // create a Hinge joint between two dynamic bodies
        // create two rigid bodies
        // static bodyA (parent) on top:
        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-20.), Scalar(-2.), Scalar(0.)));
        RigidBody* pBodyA = createRigidBody(1.0f, tr, shape);
        pBodyA->setActivationState(DISABLE_DEACTIVATION);
        // dynamic bodyB:
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(-30.), Scalar(-2.), Scalar(0.)));
        RigidBody* pBodyB = createRigidBody(10.0, tr, shape);
        pBodyB->setActivationState(DISABLE_DEACTIVATION);
        // add some data to build constraint frames
        Vec3 axisA(0.f, 1.f, 0.f);
        Vec3 axisB(0.f, 1.f, 0.f);
        Vec3 pivotA(-5.f, 0.f, 0.f);
        Vec3 pivotB(5.f, 0.f, 0.f);
        spHingeDynAB = new HingeConstraint(*pBodyA, *pBodyB, pivotA, pivotB, axisA, axisB);
        spHingeDynAB->setLimit(-SIMD_HALF_PI * 0.5f, SIMD_HALF_PI * 0.5f);
        // add constraint to world
        m_dynamicsWorld->addConstraint(spHingeDynAB, true);
        // draw constraint frames and limits for debugging
        spHingeDynAB->setDbgDrawSize(Scalar(5.f));
    }
#endif

#if ENABLE_ALL_DEMOS
    {  // 6DOF connected to the world, with motor
        Transform2 tr;
        tr.setIdentity();
        tr.setOrigin(Vec3(Scalar(10.), Scalar(-15.), Scalar(0.)));
        RigidBody* pBody = createRigidBody(1.0, tr, shape);
        pBody->setActivationState(DISABLE_DEACTIVATION);
        Transform2 frameB;
        frameB.setIdentity();
        Generic6DofConstraint* pGen6Dof = new Generic6DofConstraint(*pBody, frameB, false);
        m_dynamicsWorld->addConstraint(pGen6Dof);
        pGen6Dof->setDbgDrawSize(Scalar(5.f));

        pGen6Dof->setAngularLowerLimit(Vec3(0, 0, 0));
        pGen6Dof->setAngularUpperLimit(Vec3(0, 0, 0));
        pGen6Dof->setLinearLowerLimit(Vec3(-10., 0, 0));
        pGen6Dof->setLinearUpperLimit(Vec3(10., 0, 0));

        pGen6Dof->getTranslationalLimitMotor()->m_enableMotor[0] = true;
        pGen6Dof->getTranslationalLimitMotor()->m_targetVelocity[0] = 5.0f;
        pGen6Dof->getTranslationalLimitMotor()->m_maxMotorForce[0] = 6.0f;
    }
#endif

    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void AllConstraintDemo::exitPhysics()
{
    i32 i;

    //removed/delete constraints
    for (i = m_dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
    {
        TypedConstraint* constraint = m_dynamicsWorld->getConstraint(i);
        m_dynamicsWorld->removeConstraint(constraint);
        delete constraint;
    }
    m_ctc = NULL;

    //remove the rigidbodies from the dynamics world and delete them
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

    //delete collision shapes
    for (i32 j = 0; j < m_collisionShapes.size(); j++)
    {
        CollisionShape* shape = m_collisionShapes[j];
        delete shape;
    }

    m_collisionShapes.clear();

    //delete dynamics world
    delete m_dynamicsWorld;
    m_dynamicsWorld = 0;

    //delete solver
    delete m_solver;
    m_solver = 0;

    //delete broadphase
    delete m_broadphase;
    m_broadphase = 0;

    //delete dispatcher
    delete m_dispatcher;

    delete m_collisionConfiguration;
}

AllConstraintDemo::AllConstraintDemo(struct GUIHelperInterface* helper)
    : CommonRigidBodyBase(helper)
{
}

AllConstraintDemo::~AllConstraintDemo()
{
    //cleanup in the reverse order of creation/initialization

    Assert(m_dynamicsWorld == 0);
}

#if 0
void AllConstraintDemo::clientMoveAndDisplay()
{

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float dt = float(getDeltaTimeMicroseconds()) * 0.000001f;
    //printf("dt = %f: ",dt);

    // drive cone-twist motor
    m_Time += 0.03f;
    if (s_bTestConeTwistMotor)
    {  // this works only for obsolete constraint solver for now
        // build cone target
        Scalar t = 1.25f*m_Time;
        Vec3 axis(0,sin(t),cos(t));
        axis.normalize();
        Quat q1(axis, 0.75f*SIMD_PI);

        // build twist target
        //Quat q2(0,0,0);
        //Quat q2(Vehictor3(1,0,0), -0.3*sin(m_Time));
        Quat q2(Vec3(1,0,0), -1.49f*Sin(1.5f*m_Time));

        // compose cone + twist and set target
        q1 = q1 * q2;
        m_ctc->enableMotor(true);
        m_ctc->setMotorTargetInConstraintSpace(q1);
    }

    {
        static bool once = true;
        if ( m_dynamicsWorld->getDebugDrawer() && once)
        {
            m_dynamicsWorld->getDebugDrawer()->setDebugMode(IDebugDraw::DBG_DrawConstraints+btIDebugDraw::DBG_DrawConstraintLimits);
            once=false;
        }
    }


    {
        //during idle mode, just run 1 simulation step maximum
        i32 maxSimSubSteps = m_idle ? 1 : 1;
        if (m_idle)
            dt = 1.0f/420.f;

        i32 numSimSteps = m_dynamicsWorld->stepSimulation(dt,maxSimSubSteps);

        //optional but useful: debug drawing
        m_dynamicsWorld->debugDrawWorld();

        bool verbose = false;
        if (verbose)
        {
            if (!numSimSteps)
                printf("Interpolated transforms\n");
            else
            {
                if (numSimSteps > maxSimSubSteps)
                {
                    //detect dropping frames
                    printf("Пропущено (%i) этапов симуляции из %i\n",numSimSteps - maxSimSubSteps,numSimSteps);
                } else
                {
                    printf("Симулировано (%i) этапов\n",numSimSteps);
                }
            }
        }
    }
    renderme();

//  drawLimit();

    glFlush();
    swapBuffers();
}




void AllConstraintDemo::displayCallback(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_dynamicsWorld)
        m_dynamicsWorld->debugDrawWorld();

//  drawLimit();

    renderme();

    glFlush();
    swapBuffers();
}
#endif

bool AllConstraintDemo::keyboardCallback(i32 key, i32 state)
{
    bool handled = false;

    switch (key)
    {
        case 'O':
        {
            bool offectOnOff;
            if (spDoorHinge)
            {
                offectOnOff = spDoorHinge->getUseFrameOffset();
                offectOnOff = !offectOnOff;
                spDoorHinge->setUseFrameOffset(offectOnOff);
                printf("DoorHinge %s смещение кадра\n", offectOnOff ? "использует" : "не использует");
            }
            if (spHingeDynAB)
            {
                offectOnOff = spHingeDynAB->getUseFrameOffset();
                offectOnOff = !offectOnOff;
                spHingeDynAB->setUseFrameOffset(offectOnOff);
                printf("HingeDynAB %s смещение кадра\n", offectOnOff ? "использует" : "не использует");
            }
            if (spSlider6Dof)
            {
                offectOnOff = spSlider6Dof->getUseFrameOffset();
                offectOnOff = !offectOnOff;
                spSlider6Dof->setUseFrameOffset(offectOnOff);
                printf("Slider6Dof %s смещение кадра\n", offectOnOff ? "использует" : "не использует");
            }
        }
            handled = true;
            break;
        default:
        {
        }
        break;
    }
    return handled;
}

class CommonExampleInterface* AllConstraintCreateFunc(struct CommonExampleOptions& options)
{
    return new AllConstraintDemo(options.m_guiHelper);
}
