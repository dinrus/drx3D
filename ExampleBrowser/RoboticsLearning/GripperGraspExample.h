#ifndef GRIPPER_GRASP_EXAMPLE_H
#define GRIPPER_GRASP_EXAMPLE_H

enum GripperGraspExampleOptions
{
	eGRIPPER_GRASP = 1,
	eTWO_POINT_GRASP = 2,
	eONE_MOTOR_GRASP = 4,
	eGRASP_SOFT_BODY = 8,
	eSOFTBODY_MULTIBODY_COUPLING = 16,
	eGRASP_DEFORMABLE_CLOTH = 32,
};

class CommonExampleInterface* GripperGraspExampleCreateFunc(struct CommonExampleOptions& options);

#endif  //GRIPPER_GRASP_EXAMPLE_H
