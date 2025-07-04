
#include <drx3D/Importers/URDF/ImportURDFSetup.h>
#include <drx3D/Physics/Dynamics/ConstraintSolver/Generic6DofSpring2Constraint.h>
//#define TEST_MULTIBODY_SERIALIZATION 1

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Common/b3FileUtils.h>

#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyJointMotor.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyDynamicsWorld.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/Common/ResourcePath.h>

#include <drx3D/Importers/URDF/UrdfImporter.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>

//#include "urdf_samples.h"

#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>
#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>

class ImportUrdfSetup : public CommonMultiBodyBase
{
	char m_fileName[1024];

	struct ImportUrdfInternalData* m_data;
	bool m_useMultiBody;
	AlignedObjectArray<STxt*> m_nameMemory;
	Scalar m_grav;
	i32 m_upAxis;

public:
	ImportUrdfSetup(struct GUIHelperInterface* helper, i32 option, tukk fileName);
	virtual ~ImportUrdfSetup();

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

AlignedObjectArray<STxt> gFileNameArray;

#define MAX_NUM_MOTORS 1024

struct ImportUrdfInternalData
{
	ImportUrdfInternalData()
		: m_numMotors(0),
		  m_mb(0)
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
	MultiBody* m_mb;
	RigidBody* m_rb;
};

ImportUrdfSetup::ImportUrdfSetup(struct GUIHelperInterface* helper, i32 option, tukk fileName)
	: CommonMultiBodyBase(helper),
	  m_grav(-10),
	  m_upAxis(2)
{
	m_data = new ImportUrdfInternalData;

	if (option == 1)
	{
		m_useMultiBody = true;
	}
	else
	{
		m_useMultiBody = false;
	}

	static i32 count = 0;
	if (fileName)
	{
		setFileName(fileName);
	}
	else
	{
		gFileNameArray.clear();

		//load additional urdf file names from file

		FILE* f = fopen("urdf_files.txt", "r");
		if (f)
		{
			i32 result;
			//warning: we don't avoid string buffer overflow in this basic example in fscanf
			char fileName[1024];
			do
			{
				result = fscanf(f, "%s", fileName);
				drx3DPrintf("urdf_files.txt entry %s", fileName);
				if (result == 1)
				{
					gFileNameArray.push_back(fileName);
				}
			} while (result == 1);

			fclose(f);
		}

		if (gFileNameArray.size() == 0)
		{
			gFileNameArray.push_back("r2d2.urdf");
		}

		i32 numFileNames = gFileNameArray.size();

		if (count >= numFileNames)
		{
			count = 0;
		}
		sprintf(m_fileName, "%s", gFileNameArray[count++].c_str());
	}
}

ImportUrdfSetup::~ImportUrdfSetup()
{
	for (i32 i = 0; i < m_nameMemory.size(); i++)
	{
		delete m_nameMemory[i];
	}
	m_nameMemory.clear();
	delete m_data;
}

void ImportUrdfSetup::setFileName(tukk urdfFileName)
{
	memcpy(m_fileName, urdfFileName, strlen(urdfFileName) + 1);
}

void ImportUrdfSetup::initPhysics()
{
	m_guiHelper->setUpAxis(m_upAxis);

	this->createEmptyDynamicsWorld();
	//m_dynamicsWorld->getSolverInfo().m_numIterations = 100;
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(
		IDebugDraw::DBG_DrawConstraints + IDebugDraw::DBG_DrawContactPoints + IDebugDraw::DBG_DrawAabb);  //+btIDebugDraw::DBG_DrawConstraintLimits);

	if (m_guiHelper->getParameterInterface())
	{
		SliderParams slider("Gravity", &m_grav);
		slider.m_minVal = -10;
		slider.m_maxVal = 10;
		m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
	}

	i32 flags = 0;
	double globalScaling = 1;

	URDFImporter u2b(m_guiHelper, 0, 0, globalScaling, flags);

	bool loadOk = u2b.loadURDF(m_fileName);

#ifdef TEST_MULTIBODY_SERIALIZATION
	//test to serialize a multibody to disk or shared memory, with base, link and joint names
	Serializer* s = new btDefaultSerializer;
#endif  //TEST_MULTIBODY_SERIALIZATION

	if (loadOk)
	{
		//printTree(u2b,u2b.getRootLinkIndex());

		//u2b.printTree();

		Transform2 identityTrans;
		identityTrans.setIdentity();

		{
			//todo: move these internal API called inside the 'ConvertURDF2Bullet' call, hidden from the user
			//i32 rootLinkIndex = u2b.getRootLinkIndex();
			//drx3DPrintf("urdf root link index = %d\n",rootLinkIndex);
			MyMultiBodyCreator creation(m_guiHelper);

			ConvertURDF2Bullet(u2b, creation, identityTrans, m_dynamicsWorld, m_useMultiBody, u2b.getPathPrefix());
			m_data->m_rb = creation.getRigidBody();
			m_data->m_mb = creation.getBulletMultiBody();
			MultiBody* mb = m_data->m_mb;
			for (i32 i = 0; i < u2b.getNumAllocatedCollisionShapes(); i++)
			{
				m_collisionShapes.push_back(u2b.getAllocatedCollisionShape(i));
			}

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
			else
			{
				if (1)
				{
					//create motors for each generic joint
					i32 num6Dof = creation.getNum6DofConstraints();
					for (i32 i = 0; i < num6Dof; i++)
					{
						Generic6DofSpring2Constraint* c = creation.get6DofConstraint(i);
						if (c->getUserConstraintPtr())
						{
							GenericConstraintUserInfo* jointInfo = (GenericConstraintUserInfo*)c->getUserConstraintPtr();
							if ((jointInfo->m_urdfJointType == URDFRevoluteJoint) ||
								(jointInfo->m_urdfJointType == URDFPrismaticJoint) ||
								(jointInfo->m_urdfJointType == URDFContinuousJoint))
							{
								i32 urdfLinkIndex = jointInfo->m_urdfIndex;
								STxt jointName = u2b.getJointName(urdfLinkIndex);
								char motorName[1024];
								sprintf(motorName, "%s q'", jointName.c_str());
								Scalar* motorVel = &m_data->m_motorTargetVelocities[m_data->m_numMotors];

								*motorVel = 0.f;
								SliderParams slider(motorName, motorVel);
								slider.m_minVal = -4;
								slider.m_maxVal = 4;
								m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
								m_data->m_generic6DofJointMotors[m_data->m_numMotors] = c;
								bool motorOn = true;
								c->enableMotor(jointInfo->m_jointAxisIndex, motorOn);
								c->setMaxMotorForce(jointInfo->m_jointAxisIndex, 10000);
								c->setTargetVelocity(jointInfo->m_jointAxisIndex, 0);

								m_data->m_numMotors++;
							}
						}
					}
				}
			}
		}

		//the btMultiBody support is work-in-progress :-)

		for (i32 i = 0; i < m_dynamicsWorld->getNumMultiBodyConstraints(); i++)
		{
			m_dynamicsWorld->getMultiBodyConstraint(i)->finalizeMultiDof();
		}

		bool createGround = true;
		if (createGround)
		{
			Vec3 groundHalfExtents(20, 20, 20);
			groundHalfExtents[m_upAxis] = 1.f;
			BoxShape* box = new BoxShape(groundHalfExtents);
			m_collisionShapes.push_back(box);
			box->initializePolyhedralFeatures();

			m_guiHelper->createCollisionShapeGraphicsObject(box);
			Transform2 start;
			start.setIdentity();
			Vec3 groundOrigin(0, 0, 0);
			groundOrigin[m_upAxis] = -2.5;
			start.setOrigin(groundOrigin);
			RigidBody* body = createRigidBody(0, start, box);
			//m_dynamicsWorld->removeRigidBody(body);
			// m_dynamicsWorld->addRigidBody(body,2,1);
			Vec3 color(0.5, 0.5, 0.5);
			m_guiHelper->createRigidBodyGraphicsObject(body, color);
		}
	}

#ifdef TEST_MULTIBODY_SERIALIZATION
	m_dynamicsWorld->serialize(s);
	ResourcePath p;
	char resourcePath[1024];
	if (p.findResourcePath("r2d2_multibody.bullet", resourcePath, 1024))
	{
		FILE* f = fopen(resourcePath, "wb");
		fwrite(s->getBufferPointer(), s->getCurrentBufferSize(), 1, f);
		fclose(f);
	}
#endif  //TEST_MULTIBODY_SERIALIZATION
}

void ImportUrdfSetup::stepSimulation(float deltaTime)
{
	if (m_dynamicsWorld)
	{
		Vec3 gravity(0, 0, 0);
		gravity[m_upAxis] = m_grav;
		m_dynamicsWorld->setGravity(gravity);

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

class CommonExampleInterface* ImportURDFCreateFunc(struct CommonExampleOptions& options)
{
	return new ImportUrdfSetup(options.m_guiHelper, options.m_option, options.m_fileName);
}
