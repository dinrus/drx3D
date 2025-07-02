#ifndef KUKA_GRASP_EXAMPLE_H
#define KUKA_GRASP_EXAMPLE_H

enum KukaGraspExampleOptions
{
	eKUKA_GRASP_DLS_IK = 1,
};

class CommonExampleInterface* KukaGraspExampleCreateFunc(struct CommonExampleOptions& options);

#endif  //KUKA_GRASP_EXAMPLE_H
