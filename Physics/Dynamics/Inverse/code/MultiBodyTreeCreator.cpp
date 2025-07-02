#include <drx3D/Physics/Dynamics/Inverse/IMultiBodyTreeCreator.h>
#include <drx/Core/Core.h>
#include <drx/Core/Core.h>
namespace drx3d_inverse
{
MultiBodyTree* CreateMultiBodyTree(const IMultiBodyTreeCreator& creator)
{
	i32 num_bodies;
	i32 parent_index;
	JointType joint_type;
	vec3 body_r_parent_body_ref;
	mat33 body_R_parent_ref;
	vec3 body_axis_of_motion;
	idScalar mass;
	vec3 body_r_body_com;
	mat33 body_I_body;
	i32 user_int;
	uk user_ptr;

	MultiBodyTree* tree = new MultiBodyTree();
	if (0x0 == tree)
	{
	//	throw drx::Exc("cannot allocate tree\n");
		return 0x0;
	}

	// TODO: move to some policy argument
	tree->setAcceptInvalidMassParameters(false);

	// get number of bodies in the system
	if (-1 == creator.getNumBodies(&num_bodies))
	{
	//	throw drx::Exc("getting body indices\n");
		delete tree;
		return 0x0;
	}

	// get data for all bodies
	for (i32 index = 0; index < num_bodies; index++)
	{
		// get body parameters from user callbacks
		if (-1 ==
			creator.getBody(index, &parent_index, &joint_type, &body_r_parent_body_ref,
							&body_R_parent_ref, &body_axis_of_motion, &mass, &body_r_body_com,
							&body_I_body, &user_int, &user_ptr))
		{
			throw drx::Exc(drx::Format("getting data for body %d\n", index));
			delete tree;
			return 0x0;
		}
		// add body to system
		if (-1 ==
			tree->addBody(index, parent_index, joint_type, body_r_parent_body_ref,
						  body_R_parent_ref, body_axis_of_motion, mass, body_r_body_com,
						  body_I_body, user_int, user_ptr))
		{
			//throw drx::Exc(drx::Format("adding body %d\n", index));
			delete tree;
			return 0x0;
		}
	}
	// finalize initialization
	if (-1 == tree->finalize())
	{
		//throw drx::Exc("building system\n");
		delete tree;
		return 0x0;
	}

	return tree;
}
}  // namespace drx3d_inverse
