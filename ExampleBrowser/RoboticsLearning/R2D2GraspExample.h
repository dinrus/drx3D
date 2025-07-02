#ifndef R2D2_GRASP_EXAMPLE_H
#define R2D2_GRASP_EXAMPLE_H

enum RobotLearningExampleOptions
{
	eROBOTIC_LEARN_GRASP = 1,
	eROBOTIC_LEARN_COMPLIANT_CONTACT = 2,
	eROBOTIC_LEARN_ROLLING_FRICTION = 4,
};

class CommonExampleInterface* R2D2GraspExampleCreateFunc(struct CommonExampleOptions& options);

#endif  //R2D2_GRASP_EXAMPLE_H
