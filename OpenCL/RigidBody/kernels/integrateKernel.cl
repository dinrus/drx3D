#include <drx3D/Physics/Collision/NarrowPhase/shared/b3RigidBodyData.h>
#include <drx3D/Physics/Dynamics/shared/b3IntegrateTransforms.h>

__kernel void 
  integrateTransformsKernel( __global b3RigidBodyData_t* bodies,const int numNodes, float timeStep, float angularDamping, float4 gravityAcceleration)
{
	int nodeID = get_global_id(0);
	
	if( nodeID < numNodes)
	{
		integrateSingleTransform(bodies,nodeID, timeStep, angularDamping,gravityAcceleration);
	}
}
