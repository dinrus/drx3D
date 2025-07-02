#ifndef ET_RIGIDBODYFROMOBJ_EXAMPLE_H
#define ET_RIGIDBODYFROMOBJ_EXAMPLE_H

enum ObjToRigidBodyOptionsEnum
{
	ObjUseConvexHullForRendering = 1,
	OptimizeConvexObj = 2,
	ComputePolyhedralFeatures = 4,
};
class CommonExampleInterface* ET_RigidBodyFromObjCreateFunc(struct CommonExampleOptions& options);

#endif  //ET_RIGIDBODYFROMOBJ_EXAMPLE_H
