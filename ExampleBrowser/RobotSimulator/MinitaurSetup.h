#ifndef MINITAUR_SIMULATION_SETUP_H
#define MINITAUR_SIMULATION_SETUP_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Quat.h>

class MinitaurSetup
{
	struct MinitaurSetupInternalData* m_data;
	void resetPose(class RobotSimulatorClientAPI_NoGUI* sim);

public:
	MinitaurSetup();
	virtual ~MinitaurSetup();

	i32 setupMinitaur(class RobotSimulatorClientAPI_NoGUI* sim, const class Vec3& startPos = Vec3(0, 0, 0), const class Quat& startOrn = Quat(0, 0, 0, 1));

	void setDesiredMotorAngle(class RobotSimulatorClientAPI_NoGUI* sim, tukk motorName, double desiredAngle, double maxTorque = 3, double kp = 0.1, double kd = 0.9);
};
#endif  //MINITAUR_SIMULATION_SETUP_H
