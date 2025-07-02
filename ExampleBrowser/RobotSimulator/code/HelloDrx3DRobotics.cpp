#include "../RobotSimulatorClientAPI_NoGUI.h"

i32 main(i32 argc, tuk argv[])
{
	RobotSimulatorClientAPI_NoGUI* sim = new RobotSimulatorClientAPI_NoGUI();

	bool isConnected = sim->connect(eCONNECT_SHARED_MEMORY);

	if (!isConnected)
	{
		printf("Using Direct mode\n");
		isConnected = sim->connect(eCONNECT_DIRECT);
	}
	else
	{
		printf("Using shared memory\n");
	}

	//remove all existing objects (if any)
	sim->resetSimulation();
	sim->setGravity(Vec3(0, 0, -9.8));
	sim->setNumSolverIterations(100);
	RobotSimulatorSetPhysicsEngineParameters args;
	sim->getPhysicsEngineParameters(args);

	i32 planeUid = sim->loadURDF("plane.urdf");
	printf("planeUid = %d\n", planeUid);

	i32 r2d2Uid = sim->loadURDF("r2d2.urdf");
	printf("r2d2 #joints = %d\n", sim->getNumJoints(r2d2Uid));

	Vec3 basePosition = Vec3(0, 0, 0.5);
	Quat baseOrientation = Quat(0, 0, 0, 1);

	sim->resetBasePositionAndOrientation(r2d2Uid, basePosition, baseOrientation);

	while (sim->isConnected())
	{
		Vec3 basePos;
		Quat baseOrn;
		sim->getBasePositionAndOrientation(r2d2Uid, basePos, baseOrn);
		printf("r2d2 basePosition = [%f,%f,%f]\n", basePos[0], basePos[1], basePos[2]);

		sim->stepSimulation();
	}
	delete sim;
}
