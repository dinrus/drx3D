
#include "../ImportSDFSetup.h"
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Common/b3FileUtils.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Importers/URDF/UrdfImporter.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>
#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>
#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>

class ImportSDFSetup : public CommonMultiBodyBase
{
	char m_fileName[1024];

	struct ImportSDFInternalData* m_data;
	bool m_useMultiBody;

	//todo(erwincoumans) we need a name memory for each model
	AlignedObjectArray<STxt*> m_nameMemory;

public:
	ImportSDFSetup(struct GUIHelperInterface* helper, i32 option, tukk fileName);
	virtual ~ImportSDFSetup();

	virtual void initPhysics();
	virtual void stepSimulation(float deltaTime);

	void setFileName(tukk urdfFileName);

	virtual void resetCamera()
	{
		float dist = 3.5;
		float pitch = -28;
		float yaw = -136;
		float targetPos[3] = {0.47, 0, -0.64};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

static AlignedObjectArray<STxt> gFileNameArray;

#define MAX_NUM_MOTORS 1024

struct ImportSDFInternalData
{
	ImportSDFInternalData()
		: m_numMotors(0)
	{
		for (i32 i = 0; i < MAX_NUM_MOTORS; i++)
		{
			m_jointMotors[i] = 0;
			m_generic6DofJointMotors[i] = 0;
		}
	}

	Scalar m_motorTargetVelocities[MAX_NUM_MOTORS];
	MultiBodyJointMotor* m_jointMotors[MAX_NUM_MOTORS];
	Generic6DofSpring2Constraint* m_generic6DofJointMotors[MAX_NUM_MOTORS];
	i32 m_numMotors;
};

ImportSDFSetup::ImportSDFSetup(struct GUIHelperInterface* helper, i32 option, tukk fileName)
	: CommonMultiBodyBase(helper)
{
	m_data = new ImportSDFInternalData;
	(void)option;

	//	if (option==1)
	//	{
	m_useMultiBody = true;
	//
	//	} else
	//	{
	//		m_useMultiBody = false;
	//	}

	static i32 count = 0;
	if (fileName)
	{
		setFileName(fileName);
	}
	else
	{
		gFileNameArray.clear();

		//load additional urdf file names from file

		FILE* f = fopen("sdf_files.txt", "r");
		if (f)
		{
			i32 result;
			//warning: we don't avoid string buffer overflow in this basic example in fscanf
			char fileName[1024];
			do
			{
				result = fscanf(f, "%s", fileName);
				drx3DPrintf("sdf_files.txt entry %s", fileName);
				if (result == 1)
				{
					gFileNameArray.push_back(fileName);
				}
			} while (result == 1);

			fclose(f);
		}

		if (gFileNameArray.size() == 0)
		{
			gFileNameArray.push_back("two_cubes.sdf");
		}

		i32 numFileNames = gFileNameArray.size();

		if (count >= numFileNames)
		{
			count = 0;
		}
		sprintf(m_fileName, "%s", gFileNameArray[count++].c_str());
	}
}

ImportSDFSetup::~ImportSDFSetup()
{
	for (i32 i = 0; i < m_nameMemory.size(); i++)
	{
		delete m_nameMemory[i];
	}
	m_nameMemory.clear();
	delete m_data;
}

void ImportSDFSetup::setFileName(tukk urdfFileName)
{
	memcpy(m_fileName, urdfFileName, strlen(urdfFileName) + 1);
}

void ImportSDFSetup::initPhysics()
{
	i32 upAxis = 2;
	m_guiHelper->setUpAxis(upAxis);

	this->createEmptyDynamicsWorld();
	//m_dynamicsWorld->getSolverInfo().m_numIterations = 100;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(
		IDebugDraw::DBG_DrawConstraints + IDebugDraw::DBG_DrawContactPoints + IDebugDraw::DBG_DrawAabb);  //+btIDebugDraw::DBG_DrawConstraintLimits);

	Vec3 gravity(0, 0, 0);
	gravity[upAxis] = -9.8;

	m_dynamicsWorld->setGravity(gravity);

	URDFImporter u2b(m_guiHelper, 0, 0, 1, 0);

	bool loadOk = u2b.loadSDF(m_fileName);

	if (loadOk)
	{
		//printTree(u2b,u2b.getRootLinkIndex());

		//u2b.printTree();

		Transform2 rootTrans;
		rootTrans.setIdentity();

		for (i32 m = 0; m < u2b.getNumModels(); m++)
		{
			u2b.activateModel(m);

			MultiBody* mb = 0;

			//todo: move these internal API called inside the 'ConvertURDF2Bullet' call, hidden from the user
			//i32 rootLinkIndex = u2b.getRootLinkIndex();
			//drx3DPrintf("urdf root link index = %d\n",rootLinkIndex);
			MyMultiBodyCreator creation(m_guiHelper);

			u2b.getRootTransformInWorld(rootTrans);
			ConvertURDF2Bullet(u2b, creation, rootTrans, m_dynamicsWorld, m_useMultiBody, u2b.getPathPrefix(), CUF_USE_SDF);
			mb = creation.getBulletMultiBody();

			if (m_useMultiBody && mb)
			{
				STxt* name = new STxt(u2b.getLinkName(u2b.getRootLinkIndex()));
				m_nameMemory.push_back(name);
#ifdef TEST_MULTIBODY_SERIALIZATION
				s->registerNameForPointer(name->c_str(), name->c_str());
#endif  //TEST_MULTIBODY_SERIALIZATION
				mb->setBaseName(name->c_str());
				//create motors for each btMultiBody joint
				i32 numLinks = mb->getNumLinks();
				for (i32 i = 0; i < numLinks; i++)
				{
					i32 mbLinkIndex = i;
					i32 urdfLinkIndex = creation.m_mb2urdfLink[mbLinkIndex];

					STxt* jointName = new STxt(u2b.getJointName(urdfLinkIndex));
					STxt* linkName = new STxt(u2b.getLinkName(urdfLinkIndex).c_str());
#ifdef TEST_MULTIBODY_SERIALIZATION
					s->registerNameForPointer(jointName->c_str(), jointName->c_str());
					s->registerNameForPointer(linkName->c_str(), linkName->c_str());
#endif  //TEST_MULTIBODY_SERIALIZATION
					m_nameMemory.push_back(jointName);
					m_nameMemory.push_back(linkName);

					mb->getLink(i).m_linkName = linkName->c_str();
					mb->getLink(i).m_jointName = jointName->c_str();

					if (mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::eRevolute || mb->getLink(mbLinkIndex).m_jointType == MultibodyLink::ePrismatic)
					{
						if (m_data->m_numMotors < MAX_NUM_MOTORS)
						{
							char motorName[1024];
							sprintf(motorName, "%s q'", jointName->c_str());
							Scalar* motorVel = &m_data->m_motorTargetVelocities[m_data->m_numMotors];
							*motorVel = 0.f;
							SliderParams slider(motorName, motorVel);
							slider.m_minVal = -4;
							slider.m_maxVal = 4;
							m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
							float maxMotorImpulse = 10.1f;
							MultiBodyJointMotor* motor = new MultiBodyJointMotor(mb, mbLinkIndex, 0, 0, maxMotorImpulse);
							//motor->setMaxAppliedImpulse(0);
							m_data->m_jointMotors[m_data->m_numMotors] = motor;
							m_dynamicsWorld->addMultiBodyConstraint(motor);
							m_data->m_numMotors++;
						}
					}
				}
			}
		}

		for (i32 i = 0; i < m_dynamicsWorld->getNumMultiBodyConstraints(); i++)
		{
			m_dynamicsWorld->getMultiBodyConstraint(i)->finalizeMultiDof();
		}

		bool createGround = true;
		if (createGround)
		{
			Vec3 groundHalfExtents(20, 20, 20);
			groundHalfExtents[upAxis] = 1.f;
			BoxShape* box = new BoxShape(groundHalfExtents);
			box->initializePolyhedralFeatures();

			m_guiHelper->createCollisionShapeGraphicsObject(box);
			Transform2 start;
			start.setIdentity();
			Vec3 groundOrigin(0, 0, 0);
			groundOrigin[upAxis] = -2.5;
			start.setOrigin(groundOrigin);
			RigidBody* body = createRigidBody(0, start, box);
			//m_dynamicsWorld->removeRigidBody(body);
			// m_dynamicsWorld->addRigidBody(body,2,1);
			Vec3 color(0.5, 0.5, 0.5);
			m_guiHelper->createRigidBodyGraphicsObject(body, color);
		}

		///this extra stepSimulation call makes sure that all the btMultibody transforms are properly propagates.
		m_dynamicsWorld->stepSimulation(1. / 240., 0);  // 1., 10, 1. / 240.);
	}
}

void ImportSDFSetup::stepSimulation(float deltaTime)
{
	if (m_dynamicsWorld)
	{
		for (i32 i = 0; i < m_data->m_numMotors; i++)
		{
			if (m_data->m_jointMotors[i])
			{
				m_data->m_jointMotors[i]->setVelocityTarget(m_data->m_motorTargetVelocities[i]);
			}
			if (m_data->m_generic6DofJointMotors[i])
			{
				GenericConstraintUserInfo* jointInfo = (GenericConstraintUserInfo*)m_data->m_generic6DofJointMotors[i]->getUserConstraintPtr();
				m_data->m_generic6DofJointMotors[i]->setTargetVelocity(jointInfo->m_jointAxisIndex, m_data->m_motorTargetVelocities[i]);
				//jointInfo->
			}
		}

		//the maximal coordinates/iterative MLCP solver requires a smallish timestep to converge
		m_dynamicsWorld->stepSimulation(deltaTime, 10, 1. / 240.);
	}
}

class CommonExampleInterface* ImportSDFCreateFunc(struct CommonExampleOptions& options)
{
	return new ImportSDFSetup(options.m_guiHelper, options.m_option, options.m_fileName);
}
