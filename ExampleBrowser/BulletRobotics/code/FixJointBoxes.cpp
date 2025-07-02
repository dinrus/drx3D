#include "../FixJointBoxes.h"

#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/SharedMemory/PhysicsServerSharedMemory.h>
#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <string>
#include <vector>
#include "../../RobotSimulator/RobotSimulatorClientAPI.h"

static Scalar numSolverIterations = 1000;
static Scalar solverId = 0;

class FixJointBoxes : public CommonExampleInterface
{
	GUIHelperInterface* m_guiHelper;
	RobotSimulatorClientAPI m_robotSim;
	i32 m_options;
	RobotSimulatorSetPhysicsEngineParameters physicsArgs;
	i32 solver;

	const size_t numCubes;
	std::vector<i32> cubeIds;

public:
	FixJointBoxes(GUIHelperInterface* helper, i32 options)
		: m_guiHelper(helper),
		  m_options(options),
		  numCubes(30),
		  cubeIds(numCubes, 0),
		  solver(solverId)
	{
	}

	virtual ~FixJointBoxes()
	{
	}

	virtual void physicsDebugDraw(i32 debugDrawMode)
	{
		m_robotSim.debugDraw(debugDrawMode);
	}
	virtual void initPhysics()
	{
		i32 mode = eCONNECT_EXISTING_EXAMPLE_BROWSER;
		m_robotSim.setGuiHelper(m_guiHelper);
		bool connected = m_robotSim.connect(mode);

		drx3DPrintf("robotSim connected = %d", connected);

		m_robotSim.configureDebugVisualizer(COV_ENABLE_RGB_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_DEPTH_BUFFER_PREVIEW, 0);
		m_robotSim.configureDebugVisualizer(COV_ENABLE_SEGMENTATION_MARK_PREVIEW, 0);

		{
			RobotSimulatorLoadUrdfFileArgs args;
			RobotSimulatorChangeDynamicsArgs dynamicsArgs;

			for (i32 i = 0; i < numCubes; i++)
			{
				args.m_forceOverrideFixedBase = (i == 0);
				args.m_startPosition.setVal(0, i * 0.05, 1);
				cubeIds[i] = m_robotSim.loadURDF("cube_small.urdf", args);

				b3RobotJointInfo jointInfo;

				jointInfo.m_parentFrame[1] = -0.025;
				jointInfo.m_childFrame[1] = 0.025;

				if (i > 0)
				{
					m_robotSim.createConstraint(cubeIds[i], -1, cubeIds[i - 1], -1, &jointInfo);
					m_robotSim.setCollisionFilterGroupMask(cubeIds[i], -1, 0, 0);
				}

				m_robotSim.loadURDF("plane.urdf");
			}
		}

		{
			SliderParams slider("Прямой разрешитель", &solverId);
			slider.m_minVal = 0;
			slider.m_maxVal = 1;
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		}
		{
			SliderParams slider("numSolverIterations", &numSolverIterations);
			slider.m_minVal = 50;
			slider.m_maxVal = 1e4;
			m_guiHelper->getParameterInterface()->registerSliderFloatParameter(slider);
		}

		physicsArgs.m_defaultGlobalCFM = 1e-6;
		m_robotSim.setPhysicsEngineParameter(physicsArgs);

		m_robotSim.setGravity(Vec3(0, 0, -10));
		m_robotSim.setNumSolverIterations((i32)numSolverIterations);
	}

	virtual void exitPhysics()
	{
		m_robotSim.disconnect();
	}

	void resetCubePosition()
	{
		for (i32 i = 0; i < numCubes; i++)
		{
			Vec3 pos(0, i * (Scalar)0.05, 1);
			Quat quar(0, 0, 0, 1);
			m_robotSim.resetBasePositionAndOrientation(cubeIds[i], pos, quar);
		}
	}
	virtual void stepSimulation(float deltaTime)
	{
		i32 newSolver = (i32)(solverId + 0.5);
		if (newSolver != solver)
		{
			printf("Переключение разрешителя, новый %d, старый %d\n", newSolver, solver);
			solver = newSolver;
			resetCubePosition();
			if (solver)
			{
				physicsArgs.m_constraintSolverType = eConstraintSolverLCP_DANTZIG;
			}
			else
			{
				physicsArgs.m_constraintSolverType = eConstraintSolverLCP_SI;
			}

			m_robotSim.setPhysicsEngineParameter(physicsArgs);
		}
		m_robotSim.setNumSolverIterations((i32)numSolverIterations);
		m_robotSim.stepSimulation();
	}
	virtual void renderScene()
	{
		m_robotSim.renderScene();
	}

	virtual bool mouseMoveCallback(float x, float y)
	{
		return m_robotSim.mouseMoveCallback(x, y);
	}
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return m_robotSim.mouseButtonCallback(button, state, x, y);
	}
	virtual bool keyboardCallback(i32 key, i32 state)
	{
		return false;
	}

	virtual void resetCamera()
	{
		float dist = 1;
		float pitch = -20;
		float yaw = -30;
		float targetPos[3] = {0, 0.2, 0.5};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

class CommonExampleInterface* FixJointBoxesCreateFunc(struct CommonExampleOptions& options)
{
	return new FixJointBoxes(options.m_guiHelper, options.m_option);
}
