#ifndef DRX3D_CONVERSION_H
#define DRX3D_CONVERSION_H

class MultiBody;
#include "MathUtil.h"
void btExtractJointBodyFromBullet(const btMultiBody* bulletMB, Eigen::MatrixXd& bodyDefs, Eigen::MatrixXd& jointMat);


#endif  //DRX3D_CONVERSION_H