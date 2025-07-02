
#include "pdControlPlugin.h"
#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Plugins/PluginContext.h>
#include <stdio.h>

#include <drx3D/Maths/Linear/Scalar.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <drx3D/SharedMemory/RobotSimulatorClientAPI_NoDirect.h>
#include <drx3D/SharedMemory/RobotSimulatorClientAPI_InternalData.h>

struct MyPDControl
{
	i32 m_objectUniqueId;
	i32 m_linkIndex;
	Scalar m_desiredPosition;
	Scalar m_desiredVelocity;
	Scalar m_kd;
	Scalar m_kp;
	Scalar m_maxForce;
};

struct MyPDControlContainer
{
	i32 m_testData;
	AlignedObjectArray<MyPDControl> m_controllers;
	RobotSimulatorClientAPI_NoDirect m_api;
	MyPDControlContainer()
		: m_testData(42)
	{
	}
	virtual ~MyPDControlContainer()
	{
	}
};

DRX3D_SHARED_API i32 initPlugin_pdControlPlugin(struct PluginContext* context)
{
	MyPDControlContainer* obj = new MyPDControlContainer();
	RobotSimulatorClientAPI_InternalData data;
	data.m_physicsClientHandle = context->m_physClient;
	data.m_guiHelper = 0;
	obj->m_api.setInternalData(&data);
	context->m_userPointer = obj;

	return SHARED_MEMORY_MAGIC_NUMBER;
}

DRX3D_SHARED_API i32 preTickPluginCallback_pdControlPlugin(struct PluginContext* context)
{
	//apply pd control here, apply forces using the PD gains
	MyPDControlContainer* obj = (MyPDControlContainer*)context->m_userPointer;

	for (i32 i = 0; i < obj->m_controllers.size(); i++)
	{
		const MyPDControl& pdControl = obj->m_controllers[i];

		i32 dof1 = 0;
		b3JointSensorState actualState;
		if (obj->m_api.getJointState(pdControl.m_objectUniqueId, pdControl.m_linkIndex, &actualState))
		{
			if (pdControl.m_maxForce > 0)
			{
				//compute torque
				Scalar qActual = actualState.m_jointPosition;
				Scalar qdActual = actualState.m_jointVelocity;

				Scalar positionError = (pdControl.m_desiredPosition - qActual);
				double desiredVelocity = 0;
				Scalar velocityError = (pdControl.m_desiredVelocity - qdActual);

				Scalar force = pdControl.m_kp * positionError + pdControl.m_kd * velocityError;

				Clamp(force, -pdControl.m_maxForce, pdControl.m_maxForce);

				//apply torque
				RobotSimulatorJointMotorArgs args(CONTROL_MODE_TORQUE);
				args.m_maxTorqueValue = force;
				obj->m_api.setJointMotorControl(pdControl.m_objectUniqueId, pdControl.m_linkIndex, args);
			}
		}
	}

	return 0;
}

DRX3D_SHARED_API i32 executePluginCommand_pdControlPlugin(struct PluginContext* context, const struct b3PluginArguments* arguments)
{
	MyPDControlContainer* obj = (MyPDControlContainer*)context->m_userPointer;

	if (arguments->m_numInts == 0)
	{
		obj->m_api.syncBodies();
		return -1;
	}

	i32 numObj = obj->m_api.getNumBodies();
	//printf("numObj = %d\n", numObj);

	//protocol:
	//first i32 is the type of command
	//second i32 is body unique id
	//third i32 is link index

	if (arguments->m_numInts != 3)
		return -1;

	switch (arguments->m_ints[0])
	{
		case eSetPDControl:
		{
			if (arguments->m_numFloats < 5)
				return -1;
			MyPDControl controller;
			controller.m_desiredPosition = arguments->m_floats[0];
			controller.m_desiredVelocity = arguments->m_floats[1];
			controller.m_kd = arguments->m_floats[2];
			controller.m_kp = arguments->m_floats[3];
			controller.m_maxForce = arguments->m_floats[4];
			controller.m_objectUniqueId = arguments->m_ints[1];
			controller.m_linkIndex = arguments->m_ints[2];
			i32 foundIndex = -1;
			for (i32 i = 0; i < obj->m_controllers.size(); i++)
			{
				if (obj->m_controllers[i].m_objectUniqueId == controller.m_objectUniqueId && obj->m_controllers[i].m_linkIndex == controller.m_linkIndex)
				{
					obj->m_controllers[i] = controller;
					foundIndex = i;
				}
			}
			if (foundIndex < 0)
			{
				obj->m_controllers.push_back(controller);
			}
			break;
		}
		case eRemovePDControl:
		{
			MyPDControl controller;
			controller.m_objectUniqueId = arguments->m_ints[1];
			controller.m_linkIndex = arguments->m_ints[2];

			for (i32 i = 0; i < obj->m_controllers.size(); i++)
			{
				if (obj->m_controllers[i].m_objectUniqueId == controller.m_objectUniqueId && obj->m_controllers[i].m_linkIndex == controller.m_linkIndex)
				{
					obj->m_controllers.removeAtIndex(i);
					break;
				}
			}
			break;
		}
		default:
		{
			return -1;
		}
	}

	i32 result = 42;
	return result;
}

DRX3D_SHARED_API void exitPlugin_pdControlPlugin(struct PluginContext* context)
{
	MyPDControlContainer* obj = (MyPDControlContainer*)context->m_userPointer;
	delete obj;
	context->m_userPointer = 0;
}
