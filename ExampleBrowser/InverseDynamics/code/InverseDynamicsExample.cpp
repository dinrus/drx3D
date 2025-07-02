#include "../InverseDynamicsExample.h"

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Importers/URDF/UrdfImporter.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>
#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>

#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>

#include <drx3D/DynamicsCommon.h>

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include  <drx3D/Physics/Dynamics/Inverse/MultiBodyTreeCreator.h>

#include "../../RenderingExamples/TimeSeriesCanvas.h"

#include <vector>

// the UI interface makes it easier to use static variables & free functions
// as parameters and callbacks
static Scalar kp = 10 * 10;
static Scalar kd = 2 * 10;
static bool useInverseModel = true;
static std::vector<Scalar> qd;
static std::vector<STxt> qd_name;
static std::vector<STxt> q_name;

static Vec4 sJointCurveColors[8] =
	{
		Vec4(1, 0.3, 0.3, 1),
		Vec4(0.3, 1, 0.3, 1),
		Vec4(0.3, 0.3, 1, 1),
		Vec4(0.3, 1, 1, 1),
		Vec4(1, 0.3, 1, 1),
		Vec4(1, 1, 0.3, 1),
		Vec4(1, 0.7, 0.7, 1),
		Vec4(0.7, 1, 1, 1),

};

void toggleUseInverseModel(i32 buttonId, bool buttonState, uk userPointer)
{
	useInverseModel = !useInverseModel;
	// todo(thomas) is there a way to get a toggle button with changing text?
	drx3DPrintf("инверсная млдель %s", useInverseModel ? "включена" : "отключена");
}

class drx3d_inverseExample : public CommonMultiBodyBase
{
	drx3d_inverseExampleOptions m_option;
	MultiBody* m_multiBody;
	drx3d_inverse::MultiBodyTree* m_inverseModel;
	TimeSeriesCanvas* m_timeSeriesCanvas;

public:
	drx3d_inverseExample(struct GUIHelperInterface* helper, drx3d_inverseExampleOptions option);
	virtual ~drx3d_inverseExample();

	virtual void initPhysics();
	virtual void stepSimulation(float deltaTime);

	void setFileName(tukk urdfFileName);

	virtual void resetCamera()
	{
		float dist = 1.5;
		float pitch = -10;
		float yaw = -80;
		float targetPos[3] = {0, 0, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

drx3d_inverseExample::drx3d_inverseExample(struct GUIHelperInterface* helper, drx3d_inverseExampleOptions option)
	: CommonMultiBodyBase(helper),
	  m_option(option),
	  m_multiBody(0),
	  m_inverseModel(0),
	  m_timeSeriesCanvas(0)
{
}

drx3d_inverseExample::~drx3d_inverseExample()
{
	delete m_inverseModel;
	delete m_timeSeriesCanvas;
}

//todo(erwincoumans) Quick hack, reference to InvertedPendulumPDControl implementation. Will create a separate header/source file for this.
MultiBody* createInvertedPendulumMultiBody(MultiBodyDynamicsWorld* world, GUIHelperInterface* guiHelper, const Transform2& baseWorldTrans, bool fixedBase);

void drx3d_inverseExample::initPhysics()
{
	//roboticists like Z up
	i32 upAxis = 2;
	m_guiHelper->setUpAxis(upAxis);

	createEmptyDynamicsWorld();
	Vec3 gravity(0, 0, 0);
	// gravity[upAxis]=-9.8;
	m_dynamicsWorld->setGravity(gravity);

	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);

	{
		SliderParams slider("Kp", &kp);
		slider.m_minVal = 0;
		slider.m_maxVal = 2000;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}
	{
		SliderParams slider("Kd", &kd);
		slider.m_minVal = 0;
		slider.m_maxVal = 50;
		if (m_guiHelper->getParameterInterface())
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	if (m_option == DRX3D_ID_PROGRAMMATICALLY)
	{
		ButtonParams button("toggle inverse model", 0, true);
		button.m_callback = toggleUseInverseModel;
		m_guiHelper->getParameterInterface()->registerButtonParameter(button);
	}

	switch (m_option)
	{
		case DRX3D_ID_LOAD_URDF:
		{
			URDFImporter u2b(m_guiHelper, 0, 0, 1, 0);
			bool loadOk = u2b.loadURDF("kuka_iiwa/model.urdf");  // lwr / kuka.urdf");
			if (loadOk)
			{
				i32 rootLinkIndex = u2b.getRootLinkIndex();
				drx3DPrintf("urdf root link index = %d\n", rootLinkIndex);
				MyMultiBodyCreator creation(m_guiHelper);
				Transform2 identityTrans;
				identityTrans.setIdentity();
				ConvertURDF2Bullet(u2b, creation, identityTrans, m_dynamicsWorld, true, u2b.getPathPrefix());
				for (i32 i = 0; i < u2b.getNumAllocatedCollisionShapes(); i++)
				{
					m_collisionShapes.push_back(u2b.getAllocatedCollisionShape(i));
				}
				m_multiBody = creation.getBulletMultiBody();
				if (m_multiBody)
				{
					//kuka without joint control/constraints will gain energy explode soon due to timestep/integrator
					//temporarily set some extreme damping factors until we have some joint control or constraints
					m_multiBody->setAngularDamping(0 * 0.99);
					m_multiBody->setLinearDamping(0 * 0.99);
					drx3DPrintf("Root link name = %s", u2b.getLinkName(u2b.getRootLinkIndex()).c_str());
				}
			}
			break;
		}
		case DRX3D_ID_PROGRAMMATICALLY:
		{
			Transform2 baseWorldTrans;
			baseWorldTrans.setIdentity();
			m_multiBody = createInvertedPendulumMultiBody(m_dynamicsWorld, m_guiHelper, baseWorldTrans, false);
			break;
		}
		default:
		{
			drx3DError("Unknown option in InverseExample::initPhysics");
			drx3DAssert(0);
		}
	};

	if (m_multiBody)
	{
		{
			if (m_guiHelper->getAppInterface() && m_guiHelper->getParameterInterface())
			{
				m_timeSeriesCanvas = new TimeSeriesCanvas(m_guiHelper->getAppInterface()->m_2dCanvasInterface, 512, 230, "Joint Space Trajectory");
				m_timeSeriesCanvas->setupTimeSeries(3, 100, 0);
			}
		}

		// construct inverse model
		drx3d_inverse::MultiBodyTreeCreator id_creator;
		if (-1 == id_creator.createFromBtMultiBody(m_multiBody, false))
		{
			drx3DError("error creating tree\n");
		}
		else
		{
			m_inverseModel = drx3d_inverse::CreateMultiBodyTree(id_creator);
		}
		// add joint target controls
		qd.resize(m_multiBody->getNumDofs());

		qd_name.resize(m_multiBody->getNumDofs());
		q_name.resize(m_multiBody->getNumDofs());

		if (m_timeSeriesCanvas && m_guiHelper->getParameterInterface())
		{
			for (std::size_t dof = 0; dof < qd.size(); dof++)
			{
				qd[dof] = 0;
				char tmp[25];
				sprintf(tmp, "q_desired[%lu]", dof);
				qd_name[dof] = tmp;
				SliderParams slider(qd_name[dof].c_str(), &qd[dof]);
				slider.m_minVal = -3.14;
				slider.m_maxVal = 3.14;

				sprintf(tmp, "q[%lu]", dof);
				q_name[dof] = tmp;
				m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
				Vec4 color = sJointCurveColors[dof & 7];
				m_timeSeriesCanvas->addDataSource(q_name[dof].c_str(), color[0] * 255, color[1] * 255, color[2] * 255);
			}
		}
	}

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void drx3d_inverseExample::stepSimulation(float deltaTime)
{
	if (m_multiBody)
	{
		i32k num_dofs = m_multiBody->getNumDofs();
		drx3d_inverse::vecx nu(num_dofs), qdot(num_dofs), q(num_dofs), joint_force(num_dofs);
		drx3d_inverse::vecx pd_control(num_dofs);

		// compute joint forces from one of two control laws:
		// 1) "computed torque" control, which gives perfect, decoupled,
		//    linear second order error dynamics per dof in case of a
		//    perfect model and (and negligible time discretization effects)
		// 2) decoupled PD control per joint, without a model
		for (i32 dof = 0; dof < num_dofs; dof++)
		{
			q(dof) = m_multiBody->getJointPos(dof);
			qdot(dof) = m_multiBody->getJointVel(dof);

			const Scalar qd_dot = 0;
			const Scalar qd_ddot = 0;
			if (m_timeSeriesCanvas)
				m_timeSeriesCanvas->insertDataAtCurrentTime(q[dof], dof, true);

			// pd_control is either desired joint torque for pd control,
			// or the feedback contribution to nu
			pd_control(dof) = kd * (qd_dot - qdot(dof)) + kp * (qd[dof] - q(dof));
			// nu is the desired joint acceleration for computed torque control
			nu(dof) = qd_ddot + pd_control(dof);
		}
		if (useInverseModel)
		{
			// calculate joint forces corresponding to desired accelerations nu
			if (m_multiBody->hasFixedBase())
			{
				if (-1 != m_inverseModel->calculatedrx3d_inverse(q, qdot, nu, &joint_force))
				{
					//joint_force(dof) += damping*dot_q(dof);
					// use inverse model: apply joint force corresponding to
					// desired acceleration nu

					for (i32 dof = 0; dof < num_dofs; dof++)
					{
						m_multiBody->addJointTorque(dof, joint_force(dof));
					}
				}
			}
			else
			{
				//the inverse dynamics model represents the 6 DOFs of the base, unlike btMultiBody.
				//append some dummy values to represent the 6 DOFs of the base
				drx3d_inverse::vecx nu6(num_dofs + 6), qdot6(num_dofs + 6), q6(num_dofs + 6), joint_force6(num_dofs + 6);
				for (i32 i = 0; i < num_dofs; i++)
				{
					nu6[6 + i] = nu[i];
					qdot6[6 + i] = qdot[i];
					q6[6 + i] = q[i];
					joint_force6[6 + i] = joint_force[i];
				}
				if (-1 != m_inverseModel->calculatedrx3d_inverse(q6, qdot6, nu6, &joint_force6))
				{
					//joint_force(dof) += damping*dot_q(dof);
					// use inverse model: apply joint force corresponding to
					// desired acceleration nu

					for (i32 dof = 0; dof < num_dofs; dof++)
					{
						m_multiBody->addJointTorque(dof, joint_force6(dof + 6));
					}
				}
			}
		}
		else
		{
			for (i32 dof = 0; dof < num_dofs; dof++)
			{
				// no model: just apply PD control law
				m_multiBody->addJointTorque(dof, pd_control(dof));
			}
		}
	}

	if (m_timeSeriesCanvas)
		m_timeSeriesCanvas->nextTick();

	//todo: joint damping for btMultiBody, tune parameters

	// step the simulation
	if (m_dynamicsWorld)
	{
		// todo(thomas) check that this is correct:
		// want to advance by 10ms, with 1ms timesteps
		m_dynamicsWorld->stepSimulation(1e-3, 0);  //,1e-3);
		AlignedObjectArray<Quat> scratch_q;
		AlignedObjectArray<Vec3> scratch_m;
		m_multiBody->forwardKinematics(scratch_q, scratch_m);
#if 0
		for (i32 i = 0; i < m_multiBody->getNumLinks(); i++)
		{
			//Vec3 pos = m_multiBody->getLink(i).m_cachedWorldTransform.getOrigin();
			Transform2 tr = m_multiBody->getLink(i).m_cachedWorldTransform;
			Vec3 pos = tr.getOrigin() - quatRotate(tr.getRotation(), m_multiBody->getLink(i).m_dVector);
			Vec3 localAxis = m_multiBody->getLink(i).m_axes[0].m_topVec;
			//printf("link %d: %f,%f,%f, local axis:%f,%f,%f\n", i, pos.x(), pos.y(), pos.z(), localAxis.x(), localAxis.y(), localAxis.z());

			


		}
#endif
	}
}

CommonExampleInterface* drx3d_inverseExampleCreateFunc(CommonExampleOptions& options)
{
	return new drx3d_inverseExample(options.m_guiHelper, drx3d_inverseExampleOptions(options.m_option));
}

D3_STANDALONE_EXAMPLE(drx3d_inverseExampleCreateFunc)
