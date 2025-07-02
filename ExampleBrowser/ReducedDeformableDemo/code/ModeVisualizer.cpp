#include "../ModeVisualizer.h"
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


static i32 num_modes = 20;
static Scalar visualize_mode = 0;
static Scalar frequency_scale = 1;

class ModeVisualizer : public CommonDeformableBodyBase
{
    Scalar sim_time;

    // get deformed shape
    void getDeformedShape(ReducedDeformableBody* rsb, i32k mode_n, const Scalar time_term = 1)
    {
      for (i32 i = 0; i < rsb->m_nodes.size(); ++i)
        for (i32 k = 0; k < 3; ++k)
          rsb->m_nodes[i].m_x[k] = rsb->m_x0[i][k] + rsb->m_modes[mode_n][3 * i + k] * time_term;
    }

    Vec3 computeMassWeightedColumnSum(ReducedDeformableBody* rsb, i32k mode_n)
    {
        Vec3 sum(0, 0, 0);
        for (i32 i = 0; i < rsb->m_nodes.size(); ++i)
        {
            for (i32 k = 0; k < 3; ++k)
            {
                sum[k] += rsb->m_nodalMass[i] * rsb->m_modes[mode_n][3 * i + k];
            }
        }
        return sum;
    }

public:
    ModeVisualizer(struct GUIHelperInterface* helper)
        : CommonDeformableBodyBase(helper)
    {
        sim_time = 0;
    }
    virtual ~ModeVisualizer()
    {
    }
    void initPhysics();

    void exitPhysics();

    // disable pick force. non-interactive example.
    bool pickBody(const Vec3& rayFromWorld, const Vec3& rayToWorld) {
        return false;
    } 

    void resetCamera()
    {
        float dist = 10;
        float pitch = 0;
        float yaw = 90;
        float targetPos[3] = {0, 3, 0};
        m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
    }
    
    void stepSimulation(float deltaTime)
    {
      ReducedDeformableBody* rsb = static_cast<ReducedDeformableBody*>(static_cast<DeformableMultiBodyDynamicsWorld*>(m_dynamicsWorld)->getSoftBodyArray()[0]);

      sim_time += deltaTime;
      i32 n_mode = floor(visualize_mode);
      Scalar scale = sin(sqrt(rsb->m_eigenvalues[n_mode]) * sim_time / frequency_scale);
      getDeformedShape(rsb, n_mode, scale);
    //   Vec3 mass_weighted_column_sum = computeMassWeightedColumnSum(rsb, visualize_mode);
    //   std::cout << "mode=" << i32(visualize_mode) << "\t" << mass_weighted_column_sum[0] << "\t"
    //                                                       << mass_weighted_column_sum[1] << "\t"
    //                                                       << mass_weighted_column_sum[2] << "\n";
    }
    
    virtual void renderScene()
    {
        CommonDeformableBodyBase::renderScene();
        DeformableMultiBodyDynamicsWorld* deformableWorld = getDeformableDynamicsWorld();
        
        for (i32 i = 0; i < deformableWorld->getSoftBodyArray().size(); i++)
        {
            SoftBody* rsb = (SoftBody*)deformableWorld->getSoftBodyArray()[i];
            {
                SoftBodyHelpers::DrawFrame(rsb, deformableWorld->getDebugDrawer());
                SoftBodyHelpers::Draw(rsb, deformableWorld->getDebugDrawer(), deformableWorld->getDrawFlags());
            }
        }
    }
};

void ModeVisualizer::initPhysics()
{
    m_guiHelper->setUpAxis(1);

    ///collision configuration contains default setup for memory, collision setup
    m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);

    m_broadphase = new DbvtBroadphase();
    ReducedDeformableBodySolver* reducedSoftBodySolver = new ReducedDeformableBodySolver();

    DeformableMultiBodyConstraintSolver* sol = new DeformableMultiBodyConstraintSolver();
    sol->setDeformableSolver(reducedSoftBodySolver);
    m_solver = sol;

    m_dynamicsWorld = new DeformableMultiBodyDynamicsWorld(m_dispatcher, m_broadphase, sol, m_collisionConfiguration, reducedSoftBodySolver);
    m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

    // create volumetric soft body
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

      Transform2 init_transform;
      init_transform.setIdentity();
      init_transform.setOrigin(Vec3(0, 2, 0));
    //   init_transform.setRotation(Quat(Vec3(0, 1, 0), SIMD_PI / 2.0));
      rsb->transform(init_transform);
      SoftBodyHelpers::generateBoundaryFaces(rsb);
    }
    getDeformableDynamicsWorld()->setImplicit(false);
    m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
    
    {
      SliderParams slider("Visualize Mode", &visualize_mode);
      slider.m_minVal = 0;
      slider.m_maxVal = num_modes - 1;
      if (m_guiHelper->getParameterInterface())
          m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }
    {
      SliderParams slider("Frequency Reduction", &frequency_scale);
      slider.m_minVal = 1;
      slider.m_maxVal = 1e3;
      if (m_guiHelper->getParameterInterface())
          m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
    }
}

void ModeVisualizer::exitPhysics()
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



class CommonExampleInterface* ReducedModeVisualizerCreateFunc(struct CommonExampleOptions& options)
{
    return new ModeVisualizer(options.m_guiHelper);
}


