#include <drx3D/SharedMemory/PhysicsClientC_API.h>
#include <drx3D/SharedMemory/PhysicsClientSharedMemory.h>
#include <drx3D/Common/b3Scalar.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3Matrix3x3.h>
#include <drx3D/Common/b3Transform.h>
#include <drx3D/Common/b3TransformUtil.h>

#include <string.h>
#include <drx3D/SharedMemory/SharedMemoryCommands.h>

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadSdfCommandInit(b3PhysicsClientHandle physClient, tukk sdfFileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	return b3LoadSdfCommandInit2((b3SharedMemoryCommandHandle)command, sdfFileName);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadSdfCommandInit2(b3SharedMemoryCommandHandle commandHandle, tukk sdfFileName)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_LOAD_SDF;
	i32 len = strlen(sdfFileName);
	if (len < MAX_SDF_FILENAME_LENGTH)
	{
		strcpy(command->m_sdfArguments.m_sdfFileName, sdfFileName);
	}
	else
	{
		command->m_sdfArguments.m_sdfFileName[0] = 0;
	}
	command->m_updateFlags = SDF_ARGS_FILE_NAME;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SaveWorldCommandInit(b3PhysicsClientHandle physClient, tukk sdfFileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_SAVE_WORLD;
	i32 len = strlen(sdfFileName);
	if (len < MAX_SDF_FILENAME_LENGTH)
	{
		strcpy(command->m_sdfArguments.m_sdfFileName, sdfFileName);
	}
	else
	{
		command->m_sdfArguments.m_sdfFileName[0] = 0;
	}
	command->m_updateFlags = SDF_ARGS_FILE_NAME;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadBulletCommandInit(b3PhysicsClientHandle physClient, tukk fileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_LOAD_BULLET;
		i32 len = strlen(fileName);
		if (len < MAX_URDF_FILENAME_LENGTH)
		{
			strcpy(command->m_fileArguments.m_fileName, fileName);
		}
		else
		{
			command->m_fileArguments.m_fileName[0] = 0;
		}
		command->m_updateFlags = 0;

		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadStateCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_RESTORE_STATE;
		command->m_updateFlags = 0;
		command->m_loadStateArguments.m_fileName[0] = 0;
		command->m_loadStateArguments.m_stateId = -1;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveStateCommand(b3PhysicsClientHandle physClient, i32 stateId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_REMOVE_STATE;
		command->m_updateFlags = 0;
		command->m_loadStateArguments.m_fileName[0] = 0;
		command->m_loadStateArguments.m_stateId = stateId;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3LoadStateSetStateId(b3SharedMemoryCommandHandle commandHandle, i32 stateId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_RESTORE_STATE);
	if (command->m_type == CMD_RESTORE_STATE)
	{
		command->m_loadStateArguments.m_stateId = stateId;
		command->m_updateFlags |= CMD_LOAD_STATE_HAS_STATEID;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3LoadStateSetFileName(b3SharedMemoryCommandHandle commandHandle, tukk fileName)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_RESTORE_STATE);
	if (command->m_type == CMD_RESTORE_STATE)
	{
		i32 len = strlen(fileName);
		if (len < MAX_URDF_FILENAME_LENGTH)
		{
			strcpy(command->m_loadStateArguments.m_fileName, fileName);
		}
		else
		{
			command->m_loadStateArguments.m_fileName[0] = 0;
		}
		command->m_updateFlags |= CMD_LOAD_STATE_HAS_FILENAME;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SaveStateCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_SAVE_STATE;
		command->m_updateFlags = 0;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3GetStatusGetStateId(b3SharedMemoryStatusHandle statusHandle)
{
	i32 stateId = -1;

	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	drx3DAssert(status->m_type == CMD_SAVE_STATE_COMPLETED);
	if (status && status->m_type == CMD_SAVE_STATE_COMPLETED)
	{
		stateId = status->m_saveStateResultArgs.m_stateId;
	}
	return stateId;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SaveBulletCommandInit(b3PhysicsClientHandle physClient, tukk fileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_SAVE_BULLET;
		i32 len = strlen(fileName);
		if (len < MAX_URDF_FILENAME_LENGTH)
		{
			strcpy(command->m_fileArguments.m_fileName, fileName);
		}
		else
		{
			command->m_fileArguments.m_fileName[0] = 0;
		}
		command->m_updateFlags = 0;

		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadMJCFCommandInit(b3PhysicsClientHandle physClient, tukk fileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		return b3LoadMJCFCommandInit2((b3SharedMemoryCommandHandle)command, fileName);
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadMJCFCommandInit2(b3SharedMemoryCommandHandle commandHandle, tukk fileName)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_LOAD_MJCF;
	i32 len = strlen(fileName);
	if (len < MAX_URDF_FILENAME_LENGTH)
	{
		strcpy(command->m_mjcfArguments.m_mjcfFileName, fileName);
	}
	else
	{
		command->m_mjcfArguments.m_mjcfFileName[0] = 0;
	}
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3LoadMJCFCommandSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_MJCF);
	if (command->m_type == CMD_LOAD_MJCF)
	{
		command->m_mjcfArguments.m_flags = flags;
		command->m_updateFlags |= URDF_ARGS_HAS_CUSTOM_URDF_FLAGS;
	}
}

DRX3D_SHARED_API void b3LoadMJCFCommandSetUseMultiBody(b3SharedMemoryCommandHandle commandHandle, i32 useMultiBody)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_MJCF);
	if (command->m_type == CMD_LOAD_MJCF)
	{
		command->m_updateFlags |= URDF_ARGS_USE_MULTIBODY;
		command->m_mjcfArguments.m_useMultiBody = useMultiBody;
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadSoftBodyCommandInit(b3PhysicsClientHandle physClient, tukk fileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_LOAD_SOFT_BODY;
		i32 len = strlen(fileName);
		if (len < MAX_FILENAME_LENGTH)
		{
			strcpy(command->m_loadSoftBodyArguments.m_fileName, fileName);
		}
		else
		{
			command->m_loadSoftBodyArguments.m_fileName[0] = 0;
		}
		command->m_updateFlags = LOAD_SOFT_BODY_FILE_NAME;

		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetScale(b3SharedMemoryCommandHandle commandHandle, double scale)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_scale = scale;
	command->m_updateFlags |= LOAD_SOFT_BODY_UPDATE_SCALE;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetMass(b3SharedMemoryCommandHandle commandHandle, double mass)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_mass = mass;
	command->m_updateFlags |= LOAD_SOFT_BODY_UPDATE_MASS;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetCollisionMargin(b3SharedMemoryCommandHandle commandHandle, double collisionMargin)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_collisionMargin = collisionMargin;
	command->m_updateFlags |= LOAD_SOFT_BODY_UPDATE_COLLISION_MARGIN;
	return 0;
}


DRX3D_SHARED_API i32 b3LoadSoftBodySetStartPosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_initialPosition[0] = startPosX;
	command->m_loadSoftBodyArguments.m_initialPosition[1] = startPosY;
	command->m_loadSoftBodyArguments.m_initialPosition[2] = startPosZ;
	command->m_updateFlags |= LOAD_SOFT_BODY_INITIAL_POSITION;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetStartOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
        command->m_loadSoftBodyArguments.m_initialOrientation[0] = startOrnX;
	command->m_loadSoftBodyArguments.m_initialOrientation[1] = startOrnY;
	command->m_loadSoftBodyArguments.m_initialOrientation[2] = startOrnZ;
	command->m_loadSoftBodyArguments.m_initialOrientation[3] = startOrnW;
	command->m_updateFlags |= LOAD_SOFT_BODY_INITIAL_ORIENTATION;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyUpdateSimMesh(b3SharedMemoryCommandHandle commandHandle, tukk filename)
{
    struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
    drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
    i32 len = strlen(filename);
    if (len < MAX_FILENAME_LENGTH)
    {
        strcpy(command->m_loadSoftBodyArguments.m_simFileName, filename);
    }
    else
    {
        command->m_loadSoftBodyArguments.m_simFileName[0] = 0;
    }
    command->m_updateFlags |= LOAD_SOFT_BODY_SIM_MESH;
    return 0;

}

DRX3D_SHARED_API i32 b3LoadSoftBodyAddCorotatedForce(b3SharedMemoryCommandHandle commandHandle, double corotatedMu, double corotatedLambda)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
        command->m_loadSoftBodyArguments.m_corotatedMu = corotatedMu;
        command->m_loadSoftBodyArguments.m_corotatedLambda = corotatedLambda;
	command->m_updateFlags |= LOAD_SOFT_BODY_ADD_COROTATED_FORCE;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyAddNeoHookeanForce(b3SharedMemoryCommandHandle commandHandle, double NeoHookeanMu, double NeoHookeanLambda, double NeoHookeanDamping)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
        command->m_loadSoftBodyArguments.m_NeoHookeanMu = NeoHookeanMu;
        command->m_loadSoftBodyArguments.m_NeoHookeanLambda = NeoHookeanLambda;
        command->m_loadSoftBodyArguments.m_NeoHookeanDamping = NeoHookeanDamping;
	command->m_updateFlags |= LOAD_SOFT_BODY_ADD_NEOHOOKEAN_FORCE;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyAddMassSpringForce(b3SharedMemoryCommandHandle commandHandle, double springElasticStiffness , double springDampingStiffness)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_springElasticStiffness = springElasticStiffness;
	command->m_loadSoftBodyArguments.m_springDampingStiffness = springDampingStiffness;
	command->m_updateFlags |= LOAD_SOFT_BODY_ADD_MASS_SPRING_FORCE;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyAddGravityForce(b3SharedMemoryCommandHandle commandHandle, double gravityX, double gravityY, double gravityZ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_updateFlags |= LOAD_SOFT_BODY_ADD_GRAVITY_FORCE;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetCollisionHardness(b3SharedMemoryCommandHandle commandHandle, double collisionHardness)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
        command->m_loadSoftBodyArguments.m_collisionHardness = collisionHardness;
	command->m_updateFlags |= LOAD_SOFT_BODY_SET_COLLISION_HARDNESS;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetRepulsionStiffness(b3SharedMemoryCommandHandle commandHandle, double stiffness)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_repulsionStiffness = stiffness;
	command->m_updateFlags |= LOAD_SOFT_BODY_SET_REPULSION_STIFFNESS;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetGravityFactor(b3SharedMemoryCommandHandle commandHandle, double gravFactor)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_gravFactor = gravFactor;
	command->m_updateFlags |= LOAD_SOFT_BODY_SET_GRAVITY_FACTOR;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetSelfCollision(b3SharedMemoryCommandHandle commandHandle, i32 useSelfCollision)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
        command->m_loadSoftBodyArguments.m_useSelfCollision = useSelfCollision;
	command->m_updateFlags |= LOAD_SOFT_BODY_USE_SELF_COLLISION;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodySetFrictionCoefficient(b3SharedMemoryCommandHandle commandHandle, double frictionCoefficient)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
        command->m_loadSoftBodyArguments.m_frictionCoeff = frictionCoefficient;
	command->m_updateFlags |= LOAD_SOFT_BODY_SET_FRICTION_COEFFICIENT;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyUseBendingSprings(b3SharedMemoryCommandHandle commandHandle, i32 useBendingSprings, double bendingStiffness)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
    command->m_loadSoftBodyArguments.m_useBendingSprings = useBendingSprings;
    command->m_loadSoftBodyArguments.m_springBendingStiffness = bendingStiffness;
	command->m_updateFlags |= LOAD_SOFT_BODY_ADD_BENDING_SPRINGS;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyUseAllDirectionDampingSprings(b3SharedMemoryCommandHandle commandHandle, i32 allDirectionDamping)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
	command->m_loadSoftBodyArguments.m_dampAllDirections = allDirectionDamping;
	command->m_updateFlags |= LOAD_SOFT_BODY_SET_DAMPING_SPRING_MODE;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSoftBodyUseFaceContact(b3SharedMemoryCommandHandle commandHandle, i32 useFaceContact)
{
    struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
    drx3DAssert(command->m_type == CMD_LOAD_SOFT_BODY);
    command->m_loadSoftBodyArguments.m_useFaceContact = useFaceContact;
    command->m_updateFlags |= LOAD_SOFT_BODY_USE_FACE_CONTACT;
    return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadUrdfCommandInit(b3PhysicsClientHandle physClient, tukk urdfFileName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());

	if (cl->canSubmitCommand())
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_LOAD_URDF;
		i32 len = strlen(urdfFileName);
		if (len < MAX_URDF_FILENAME_LENGTH)
		{
			strcpy(command->m_urdfArguments.m_urdfFileName, urdfFileName);
		}
		else
		{
			command->m_urdfArguments.m_urdfFileName[0] = 0;
		}
		command->m_updateFlags = URDF_ARGS_FILE_NAME;

		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3LoadUrdfCommandInit2(b3SharedMemoryCommandHandle commandHandle, tukk urdfFileName)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);

	command->m_type = CMD_LOAD_URDF;
	i32 len = strlen(urdfFileName);
	if (len < MAX_URDF_FILENAME_LENGTH)
	{
		strcpy(command->m_urdfArguments.m_urdfFileName, urdfFileName);
	}
	else
	{
		command->m_urdfArguments.m_urdfFileName[0] = 0;
	}
	command->m_updateFlags = URDF_ARGS_FILE_NAME;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3LoadUrdfCommandSetUseMultiBody(b3SharedMemoryCommandHandle commandHandle, i32 useMultiBody)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_LOAD_URDF);
	command->m_updateFlags |= URDF_ARGS_USE_MULTIBODY;
	command->m_urdfArguments.m_useMultiBody = useMultiBody;

	return 0;
}

DRX3D_SHARED_API i32 b3LoadUrdfCommandSetGlobalScaling(b3SharedMemoryCommandHandle commandHandle, double globalScaling)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_LOAD_URDF);
	command->m_updateFlags |= URDF_ARGS_USE_GLOBAL_SCALING;
	command->m_urdfArguments.m_globalScaling = globalScaling;
	return 0;
}

DRX3D_SHARED_API i32 b3LoadSdfCommandSetUseMultiBody(b3SharedMemoryCommandHandle commandHandle, i32 useMultiBody)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_LOAD_SDF);
	command->m_updateFlags |= URDF_ARGS_USE_MULTIBODY;
	command->m_sdfArguments.m_useMultiBody = useMultiBody;

	return 0;
}

DRX3D_SHARED_API i32 b3LoadSdfCommandSetUseGlobalScaling(b3SharedMemoryCommandHandle commandHandle, double globalScaling)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_LOAD_SDF);
	command->m_updateFlags |= URDF_ARGS_USE_GLOBAL_SCALING;
	command->m_sdfArguments.m_globalScaling = globalScaling;

	return 0;
}

DRX3D_SHARED_API i32 b3LoadUrdfCommandSetUseFixedBase(b3SharedMemoryCommandHandle commandHandle, i32 useFixedBase)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_LOAD_URDF);
	if (command && (command->m_type == CMD_LOAD_URDF))
	{
		command->m_updateFlags |= URDF_ARGS_USE_FIXED_BASE;
		command->m_urdfArguments.m_useFixedBase = useFixedBase;
		return 0;
	}
	return -1;
}

DRX3D_SHARED_API i32 b3LoadUrdfCommandSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_LOAD_URDF);
	if (command && (command->m_type == CMD_LOAD_URDF))
	{
		command->m_updateFlags |= URDF_ARGS_HAS_CUSTOM_URDF_FLAGS;
		command->m_urdfArguments.m_urdfFlags = flags;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3LoadUrdfCommandSetStartPosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if (command)
	{
		drx3DAssert(command->m_type == CMD_LOAD_URDF);
		if (command->m_type == CMD_LOAD_URDF)
		{
			command->m_urdfArguments.m_initialPosition[0] = startPosX;
			command->m_urdfArguments.m_initialPosition[1] = startPosY;
			command->m_urdfArguments.m_initialPosition[2] = startPosZ;
			command->m_updateFlags |= URDF_ARGS_INITIAL_POSITION;
		}
		return 0;
	}
	return -1;
}

DRX3D_SHARED_API i32 b3LoadUrdfCommandSetStartOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if (command)
	{
		drx3DAssert(command->m_type == CMD_LOAD_URDF);
		if (command->m_type == CMD_LOAD_URDF)
		{
			command->m_urdfArguments.m_initialOrientation[0] = startOrnX;
			command->m_urdfArguments.m_initialOrientation[1] = startOrnY;
			command->m_urdfArguments.m_initialOrientation[2] = startOrnZ;
			command->m_urdfArguments.m_initialOrientation[3] = startOrnW;
			command->m_updateFlags |= URDF_ARGS_INITIAL_ORIENTATION;
		}
		return 0;
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestPhysicsParamCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusPhysicsSimulationParameters(b3SharedMemoryStatusHandle statusHandle, struct b3PhysicsSimulationParameters* params)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	drx3DAssert(status->m_type == CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS_COMPLETED);
	if (status && status->m_type == CMD_REQUEST_PHYSICS_SIMULATION_PARAMETERS_COMPLETED)
	{
		*params = status->m_simulationParameterResultArgs;
		return 1;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitPhysicsParamCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3InitPhysicsParamCommand2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitPhysicsParamCommand2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_SEND_PHYSICS_SIMULATION_PARAMETERS;
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetGravity(b3SharedMemoryCommandHandle commandHandle, double gravx, double gravy, double gravz)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_gravityAcceleration[0] = gravx;
	command->m_physSimParamArgs.m_gravityAcceleration[1] = gravy;
	command->m_physSimParamArgs.m_gravityAcceleration[2] = gravz;
	command->m_updateFlags |= SIM_PARAM_UPDATE_GRAVITY;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetRealTimeSimulation(b3SharedMemoryCommandHandle commandHandle, i32 enableRealTimeSimulation)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_useRealTimeSimulation = (enableRealTimeSimulation != 0);
	command->m_updateFlags |= SIM_PARAM_UPDATE_REAL_TIME_SIMULATION;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetInternalSimFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_internalSimFlags = flags;
	command->m_updateFlags |= SIM_PARAM_UPDATE_INTERNAL_SIMULATION_FLAGS;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetUseSplitImpulse(b3SharedMemoryCommandHandle commandHandle, i32 useSplitImpulse)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);

	command->m_physSimParamArgs.m_useSplitImpulse = useSplitImpulse;
	command->m_updateFlags |= SIM_PARAM_UPDATE_USE_SPLIT_IMPULSE;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetSplitImpulsePenetrationThreshold(b3SharedMemoryCommandHandle commandHandle, double splitImpulsePenetrationThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);

	command->m_physSimParamArgs.m_splitImpulsePenetrationThreshold = splitImpulsePenetrationThreshold;
	command->m_updateFlags |= SIM_PARAM_UPDATE_SPLIT_IMPULSE_PENETRATION_THRESHOLD;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetContactBreakingThreshold(b3SharedMemoryCommandHandle commandHandle, double contactBreakingThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);

	command->m_physSimParamArgs.m_contactBreakingThreshold = contactBreakingThreshold;
	command->m_updateFlags |= SIM_PARAM_UPDATE_CONTACT_BREAKING_THRESHOLD;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetMaxNumCommandsPer1ms(b3SharedMemoryCommandHandle commandHandle, i32 maxNumCmdPer1ms)
{
	//obsolete command
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetEnableFileCaching(b3SharedMemoryCommandHandle commandHandle, i32 enableFileCaching)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);

	command->m_physSimParamArgs.m_enableFileCaching = enableFileCaching;
	command->m_updateFlags |= SIM_PARAM_ENABLE_FILE_CACHING;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetRestitutionVelocityThreshold(b3SharedMemoryCommandHandle commandHandle, double restitutionVelocityThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);

	command->m_physSimParamArgs.m_restitutionVelocityThreshold = restitutionVelocityThreshold;
	command->m_updateFlags |= SIM_PARAM_UPDATE_RESTITUTION_VELOCITY_THRESHOLD;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetEnableConeFriction(b3SharedMemoryCommandHandle commandHandle, i32 enableConeFriction)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_enableConeFriction = enableConeFriction;
	command->m_updateFlags |= SIM_PARAM_ENABLE_CONE_FRICTION;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetDeterministicOverlappingPairs(b3SharedMemoryCommandHandle commandHandle, i32 deterministicOverlappingPairs)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_deterministicOverlappingPairs = deterministicOverlappingPairs;
	command->m_updateFlags |= SIM_PARAM_UPDATE_DETERMINISTIC_OVERLAPPING_PAIRS;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetAllowedCcdPenetration(b3SharedMemoryCommandHandle commandHandle, double allowedCcdPenetration)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_allowedCcdPenetration = allowedCcdPenetration;
	command->m_updateFlags |= SIM_PARAM_UPDATE_CCD_ALLOWED_PENETRATION;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetJointFeedbackMode(b3SharedMemoryCommandHandle commandHandle, i32 jointFeedbackMode)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_jointFeedbackMode = jointFeedbackMode;
	command->m_updateFlags |= SIM_PARAM_UPDATE_JOINT_FEEDBACK_MODE;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetNumSolverIterations(b3SharedMemoryCommandHandle commandHandle, i32 numSolverIterations)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_numSolverIterations = numSolverIterations;
	command->m_updateFlags |= SIM_PARAM_UPDATE_NUM_SOLVER_ITERATIONS;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetNumNonContactInnerIterations(b3SharedMemoryCommandHandle commandHandle, i32 numNonContactInnerIterations)
{
    struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
    drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
    command->m_physSimParamArgs.m_numNonContactInnerIterations = numNonContactInnerIterations;
	command->m_updateFlags |= SIM_PARAM_UPDATE_NUM_NONCONTACT_INNER_ITERATIONS;
    return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetWarmStartingFactor(b3SharedMemoryCommandHandle commandHandle, double warmStartingFactor)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_warmStartingFactor = warmStartingFactor;
	command->m_updateFlags |= SIM_PARAM_UPDATE_WARM_STARTING_FACTOR;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetArticulatedWarmStartingFactor(b3SharedMemoryCommandHandle commandHandle, double warmStartingFactor)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_articulatedWarmStartingFactor = warmStartingFactor;
	command->m_updateFlags |= SIM_PARAM_UPDATE_ARTICULATED_WARM_STARTING_FACTOR;
	return 0;
}
DRX3D_SHARED_API i32 b3PhysicsParamSetSolverResidualThreshold(b3SharedMemoryCommandHandle commandHandle, double solverResidualThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_solverResidualThreshold = solverResidualThreshold;
	command->m_updateFlags |= SIM_PARAM_UPDATE_SOLVER_RESIDULAL_THRESHOLD;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetContactSlop(b3SharedMemoryCommandHandle commandHandle, double contactSlop)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_contactSlop = contactSlop;
	command->m_updateFlags |= SIM_PARAM_UPDATE_CONTACT_SLOP;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetEnableSAT(b3SharedMemoryCommandHandle commandHandle, i32 enableSAT)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_enableSAT = enableSAT;
	command->m_updateFlags |= SIM_PARAM_ENABLE_SAT;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetConstraintSolverType(b3SharedMemoryCommandHandle commandHandle, i32 constraintSolverType)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_constraintSolverType = constraintSolverType;
	command->m_updateFlags |= SIM_PARAM_CONSTRAINT_SOLVER_TYPE;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetMinimumSolverIslandSize(b3SharedMemoryCommandHandle commandHandle, i32 minimumSolverIslandSize)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_minimumSolverIslandSize = minimumSolverIslandSize;
	command->m_updateFlags |= SIM_PARAM_CONSTRAINT_MIN_SOLVER_ISLAND_SIZE;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetSolverAnalytics(b3SharedMemoryCommandHandle commandHandle, i32 reportSolverAnalytics)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_reportSolverAnalytics = reportSolverAnalytics;
	command->m_updateFlags |= SIM_PARAM_REPORT_CONSTRAINT_SOLVER_ANALYTICS;
	return 0;
}



DRX3D_SHARED_API i32 b3PhysicsParamSetCollisionFilterMode(b3SharedMemoryCommandHandle commandHandle, i32 filterMode)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_physSimParamArgs.m_collisionFilterMode = filterMode;
	command->m_updateFlags |= SIM_PARAM_UPDATE_COLLISION_FILTER_MODE;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetTimeStep(b3SharedMemoryCommandHandle commandHandle, double timeStep)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_DELTA_TIME;
	command->m_physSimParamArgs.m_deltaTime = timeStep;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetNumSubSteps(b3SharedMemoryCommandHandle commandHandle, i32 numSubSteps)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_NUM_SIMULATION_SUB_STEPS;
	command->m_physSimParamArgs.m_numSimulationSubSteps = numSubSteps;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultContactERP(b3SharedMemoryCommandHandle commandHandle, double defaultContactERP)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_DEFAULT_CONTACT_ERP;
	command->m_physSimParamArgs.m_defaultContactERP = defaultContactERP;
	return 0;
}
DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultNonContactERP(b3SharedMemoryCommandHandle commandHandle, double defaultNonContactERP)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_DEFAULT_NON_CONTACT_ERP;
	command->m_physSimParamArgs.m_defaultNonContactERP = defaultNonContactERP;
	return 0;
}
DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultFrictionERP(b3SharedMemoryCommandHandle commandHandle, double frictionERP)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_DEFAULT_FRICTION_ERP;
	command->m_physSimParamArgs.m_frictionERP = frictionERP;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultGlobalCFM(b3SharedMemoryCommandHandle commandHandle, double defaultGlobalCFM)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_DEFAULT_GLOBAL_CFM;
	command->m_physSimParamArgs.m_defaultGlobalCFM = defaultGlobalCFM;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParamSetDefaultFrictionCFM(b3SharedMemoryCommandHandle commandHandle, double frictionCFM)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_DEFAULT_FRICTION_CFM;
	command->m_physSimParamArgs.m_frictionCFM = frictionCFM;
	return 0;
}

DRX3D_SHARED_API i32 b3PhysicsParameterSetSparseSdfVoxelSize(b3SharedMemoryCommandHandle commandHandle, double sparseSdfVoxelSize)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_SEND_PHYSICS_SIMULATION_PARAMETERS);
	command->m_updateFlags |= SIM_PARAM_UPDATE_SPARSE_SDF;
	command->m_physSimParamArgs.m_sparseSdfVoxelSize = sparseSdfVoxelSize;
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitStepSimulationCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3InitStepSimulationCommand2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitStepSimulationCommand2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_STEP_FORWARD_SIMULATION;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}


DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitPerformCollisionDetectionCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_PERFORM_COLLISION_DETECTION;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitResetSimulationCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3InitResetSimulationCommand2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitResetSimulationCommand2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	command->m_type = CMD_RESET_SIMULATION;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3InitResetSimulationSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	Assert(command->m_type == CMD_RESET_SIMULATION);
	if (command->m_type == CMD_RESET_SIMULATION)
	{
		command->m_updateFlags = flags;
	}
	return 0;
}


DRX3D_SHARED_API b3SharedMemoryCommandHandle b3JointControlCommandInit(b3PhysicsClientHandle physClient, i32 controlMode)
{
	return b3JointControlCommandInit2(physClient, 0, controlMode);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3JointControlCommandInit2(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 controlMode)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3JointControlCommandInit2Internal((b3SharedMemoryCommandHandle)command, bodyUniqueId, controlMode);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3JointControlCommandInit2Internal(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 controlMode)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_SEND_DESIRED_STATE;
	command->m_sendDesiredStateCommandArgument.m_controlMode = controlMode;
	command->m_sendDesiredStateCommandArgument.m_bodyUniqueId = bodyUniqueId;
	command->m_updateFlags = 0;
	for (i32 i = 0; i < MAX_DEGREE_OF_FREEDOM; i++)
	{
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[i] = 0;

	}
	for (i32 i = 0; i < 7; i++)
	{
		command->m_sendDesiredStateCommandArgument.m_desiredStateQ[i] = 0;
		command->m_sendDesiredStateCommandArgument.m_desiredStateQdot[i] = 0;
		command->m_sendDesiredStateCommandArgument.m_Kp[i] = 0;
		command->m_sendDesiredStateCommandArgument.m_Kd[i] = 0;
		command->m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[i] = 0;
		command->m_sendDesiredStateCommandArgument.m_damping[i] = 0;
	}
	command->m_sendDesiredStateCommandArgument.m_desiredStateQ[3] = 1;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3JointControlSetDesiredPosition(b3SharedMemoryCommandHandle commandHandle, i32 qIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((qIndex >= 0) && (qIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_desiredStateQ[qIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_Q;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[qIndex] |= SIM_DESIRED_STATE_HAS_Q;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDesiredPositionMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 qIndex, const double* position, i32 dofCount)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((qIndex >= 0) && ((qIndex + dofCount) < MAX_DEGREE_OF_FREEDOM) && dofCount > 0 && dofCount <= 4)
	{
		for (i32 dof = 0; dof < dofCount; dof++)
		{
			command->m_sendDesiredStateCommandArgument.m_desiredStateQ[qIndex + dof] = position[dof];
			command->m_updateFlags |= SIM_DESIRED_STATE_HAS_Q;
			command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[qIndex + dof] |= SIM_DESIRED_STATE_HAS_Q;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetKp(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_Kp[dofIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_KP;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_KP;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetKpMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* kps, i32 dofCount)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM ) && dofCount >= 0 && dofCount <= 4)
	{
		for (i32 dof = 0; dof < dofCount; dof++)
		{
			command->m_sendDesiredStateCommandArgument.m_Kp[dofIndex + dof] = kps[dof];
			command->m_updateFlags |= SIM_DESIRED_STATE_HAS_KP;
			command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex + dof] |= SIM_DESIRED_STATE_HAS_KP;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetKd(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_Kd[dofIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_KD;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_KD;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetKdMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* kds, i32 dofCount)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM ) && dofCount >= 0 && dofCount <= 4)
	{
		for (i32 dof = 0; dof < dofCount; dof++)
		{
			command->m_sendDesiredStateCommandArgument.m_Kd[dofIndex + dof] = kds[dof];
			command->m_updateFlags |= SIM_DESIRED_STATE_HAS_KD;
			command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex + dof] |= SIM_DESIRED_STATE_HAS_KD;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetMaximumVelocity(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double maximumVelocity)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_rhsClamp[dofIndex] = maximumVelocity;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_RHS_CLAMP;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_RHS_CLAMP;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDesiredVelocity(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_desiredStateQdot[dofIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_QDOT;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_QDOT;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDesiredVelocityMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, const double* velocity, i32 dofCount)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && ((dofIndex+ dofCount) < MAX_DEGREE_OF_FREEDOM) && dofCount>=0 && dofCount<=4)
	{
		for (i32 dof = 0; dof < dofCount; dof++)
		{
			command->m_sendDesiredStateCommandArgument.m_desiredStateQdot[dofIndex+dof] = velocity[dof];
			command->m_updateFlags |= SIM_DESIRED_STATE_HAS_QDOT;
			command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex+dof] |= SIM_DESIRED_STATE_HAS_QDOT;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDesiredForceTorqueMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* forces, i32 dofCount)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM ) && dofCount >= 0 && dofCount <= 4)
	{
		for (i32 dof = 0; dof < dofCount; dof++)
		{
			command->m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[dofIndex+dof] = forces[dof];
			command->m_updateFlags |= SIM_DESIRED_STATE_HAS_MAX_FORCE;
			command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex + dof] |= SIM_DESIRED_STATE_HAS_MAX_FORCE;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDamping(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_damping[dofIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_DAMPING;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_DAMPING;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDampingMultiDof(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double* damping, i32 dofCount)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM ) && dofCount >= 0 && dofCount <= 4)
	{
		for (i32 dof = 0; dof < dofCount; dof++)
		{
			command->m_sendDesiredStateCommandArgument.m_damping[dofIndex+dof] = damping[dof];
			command->m_updateFlags |= SIM_DESIRED_STATE_HAS_DAMPING;
			command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex + dof] |= SIM_DESIRED_STATE_HAS_DAMPING;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetMaximumForce(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[dofIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_MAX_FORCE;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_MAX_FORCE;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3JointControlSetDesiredForceTorque(b3SharedMemoryCommandHandle commandHandle, i32 dofIndex, double value)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if ((dofIndex >= 0) && (dofIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_sendDesiredStateCommandArgument.m_desiredStateForceTorque[dofIndex] = value;
		command->m_updateFlags |= SIM_DESIRED_STATE_HAS_MAX_FORCE;
		command->m_sendDesiredStateCommandArgument.m_hasDesiredStateFlags[dofIndex] |= SIM_DESIRED_STATE_HAS_MAX_FORCE;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestActualStateCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3RequestActualStateCommandInit2((b3SharedMemoryCommandHandle)command, bodyUniqueId);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestActualStateCommandInit2(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_REQUEST_ACTUAL_STATE;
	command->m_updateFlags = 0;
	command->m_requestActualStateInformationCommandArgument.m_bodyUniqueId = bodyUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3RequestActualStateCommandComputeLinkVelocity(b3SharedMemoryCommandHandle commandHandle, i32 computeLinkVelocity)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	Assert(command->m_type == CMD_REQUEST_ACTUAL_STATE);
	if (computeLinkVelocity && command->m_type == CMD_REQUEST_ACTUAL_STATE)
	{
		command->m_updateFlags |= ACTUAL_STATE_COMPUTE_LINKVELOCITY;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3RequestActualStateCommandComputeForwardKinematics(b3SharedMemoryCommandHandle commandHandle, i32 computeForwardKinematics)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	Assert(command->m_type == CMD_REQUEST_ACTUAL_STATE);
	if (computeForwardKinematics && command->m_type == CMD_REQUEST_ACTUAL_STATE)
	{
		command->m_updateFlags |= ACTUAL_STATE_COMPUTE_FORWARD_KINEMATICS;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3GetJointState(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32 jointIndex, b3JointSensorState* state)
{

	
	

	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	i32 bodyIndex = status->m_sendActualStateArgs.m_bodyUniqueId;
	drx3DAssert(bodyIndex >= 0);
	if (bodyIndex >= 0)
	{
		b3JointInfo info;

		bool result = b3GetJointInfo(physClient, bodyIndex, jointIndex, &info) != 0;
		if (result && status->m_sendActualStateArgs.m_stateDetails)
		{
			if ((info.m_qIndex >= 0) && (info.m_uIndex >= 0) && (info.m_qIndex < MAX_DEGREE_OF_FREEDOM) && (info.m_uIndex < MAX_DEGREE_OF_FREEDOM))
			{
				state->m_jointPosition = status->m_sendActualStateArgs.m_stateDetails->m_actualStateQ[info.m_qIndex];
				state->m_jointVelocity = status->m_sendActualStateArgs.m_stateDetails->m_actualStateQdot[info.m_uIndex];
			}
			else
			{
				state->m_jointPosition = 0;
				state->m_jointVelocity = 0;
			}
			for (i32 ii(0); ii < 6; ++ii)
			{
				state->m_jointForceTorque[ii] = status->m_sendActualStateArgs.m_stateDetails->m_jointReactionForces[6 * jointIndex + ii];
			}
			state->m_jointMotorTorque = status->m_sendActualStateArgs.m_stateDetails->m_jointMotorForce[jointIndex];
			return 1;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3GetJointStateMultiDof(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32 jointIndex, b3JointSensorState2* state)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	i32 bodyIndex = status->m_sendActualStateArgs.m_bodyUniqueId;
	drx3DAssert(bodyIndex >= 0);
	if (bodyIndex >= 0)
	{
		state->m_qDofSize = 0;
		state->m_uDofSize = 0;
		b3JointInfo info;
		bool result = b3GetJointInfo(physClient, bodyIndex, jointIndex, &info) != 0;
		if (result)
		{
			
			if ((info.m_qIndex >= 0) && (info.m_uIndex >= 0) && (info.m_qIndex < MAX_DEGREE_OF_FREEDOM) && (info.m_uIndex < MAX_DEGREE_OF_FREEDOM))
			{
				state->m_qDofSize = info.m_qSize;
				state->m_uDofSize = info.m_uSize;
				for (i32 i = 0; i < state->m_qDofSize; i++)
				{
					state->m_jointPosition[i] = status->m_sendActualStateArgs.m_stateDetails->m_actualStateQ[info.m_qIndex + i];
				}
				for (i32 i = 0; i < state->m_uDofSize; i++)
				{
					state->m_jointVelocity[i] = status->m_sendActualStateArgs.m_stateDetails->m_actualStateQdot[info.m_uIndex+i];
					state->m_jointMotorTorqueMultiDof[i] = status->m_sendActualStateArgs.m_stateDetails->m_jointMotorForceMultiDof[info.m_uIndex + i];
				}
			}
			else
			{
				state->m_jointPosition[0] = 0;
				state->m_jointVelocity[0] = 0;
			}
			for (i32 ii(0); ii < 6; ++ii)
			{
				state->m_jointReactionForceTorque[ii] = status->m_sendActualStateArgs.m_stateDetails->m_jointReactionForces[6 * jointIndex + ii];
			}
			
			return 1;
		}
	}
	return 0;
}


DRX3D_SHARED_API i32 b3GetLinkState(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32 linkIndex, b3LinkState* state)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	i32 bodyIndex = status->m_sendActualStateArgs.m_bodyUniqueId;
	drx3DAssert(bodyIndex >= 0);
	drx3DAssert(linkIndex >= 0);
	i32 numLinks = status->m_sendActualStateArgs.m_numLinks;
	drx3DAssert(linkIndex < numLinks);

	if (status->m_sendActualStateArgs.m_stateDetails != NULL && (bodyIndex >= 0) && (linkIndex >= 0) && linkIndex < numLinks)
	{
		b3Transform wlf, com, inertial;

		for (i32 i = 0; i < 3; ++i)
		{
			state->m_worldPosition[i] = status->m_sendActualStateArgs.m_stateDetails->m_linkState[7 * linkIndex + i];
			state->m_localInertialPosition[i] = status->m_sendActualStateArgs.m_stateDetails->m_linkLocalInertialFrames[7 * linkIndex + i];
			state->m_worldLinearVelocity[i] = status->m_sendActualStateArgs.m_stateDetails->m_linkWorldVelocities[6 * linkIndex + i];
			state->m_worldAngularVelocity[i] = status->m_sendActualStateArgs.m_stateDetails->m_linkWorldVelocities[6 * linkIndex + i + 3];
		}
		for (i32 i = 0; i < 4; ++i)
		{
			state->m_worldOrientation[i] = status->m_sendActualStateArgs.m_stateDetails->m_linkState[7 * linkIndex + 3 + i];
			state->m_localInertialOrientation[i] = status->m_sendActualStateArgs.m_stateDetails->m_linkLocalInertialFrames[7 * linkIndex + 3 + i];
		}
		com.setOrigin(b3MakeVector3(state->m_worldPosition[0], state->m_worldPosition[1], state->m_worldPosition[2]));
		com.setRotation(b3Quat(state->m_worldOrientation[0], state->m_worldOrientation[1], state->m_worldOrientation[2], state->m_worldOrientation[3]));
		inertial.setOrigin(b3MakeVector3(state->m_localInertialPosition[0], state->m_localInertialPosition[1], state->m_localInertialPosition[2]));
		inertial.setRotation(b3Quat(state->m_localInertialOrientation[0], state->m_localInertialOrientation[1], state->m_localInertialOrientation[2], state->m_localInertialOrientation[3]));
		wlf = com * inertial.inverse();
		for (i32 i = 0; i < 3; ++i)
		{
			state->m_worldLinkFramePosition[i] = wlf.getOrigin()[i];
		}
		b3Quat wlfOrn = wlf.getRotation();
		for (i32 i = 0; i < 4; ++i)
		{
			state->m_worldLinkFrameOrientation[i] = wlfOrn[i];
		}
		return 1;
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateCollisionShapeCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	if (cl)
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_CREATE_COLLISION_SHAPE;
		command->m_updateFlags = 0;
		command->m_createUserShapeArgs.m_numUserShapes = 0;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddSphere(b3SharedMemoryCommandHandle commandHandle, double radius)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_SPHERE;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_sphereRadius = radius;
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3ResetMeshDataCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 numVertices, const double* vertices)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	if (cl)
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_RESET_MESH_DATA;
		command->m_updateFlags = 0;
		command->m_resetMeshDataArgs.m_numVertices = numVertices;
		command->m_resetMeshDataArgs.m_bodyUniqueId = bodyUniqueId;
		command->m_resetMeshDataArgs.m_flags = 0;
		i32 totalUploadSizeInBytes = numVertices * sizeof(double) *3;
		cl->uploadBulletFileToSharedMemory((tukk)vertices, totalUploadSizeInBytes);
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}


DRX3D_SHARED_API b3SharedMemoryCommandHandle b3GetMeshDataCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	if (cl)
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_REQUEST_MESH_DATA;
		command->m_updateFlags = 0;
		command->m_requestMeshDataArgs.m_startingVertex = 0;
		command->m_requestMeshDataArgs.m_bodyUniqueId = bodyUniqueId;
		command->m_requestMeshDataArgs.m_linkIndex = linkIndex;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API void b3GetMeshDataSetCollisionShapeIndex(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*) commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_MESH_DATA);
	if (command->m_type == CMD_REQUEST_MESH_DATA)
	{
		command->m_updateFlags = D3_MESH_DATA_COLLISIONSHAPEINDEX;
		command->m_requestMeshDataArgs.m_collisionShapeIndex = shapeIndex;
	}
}

DRX3D_SHARED_API void b3GetMeshDataSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_MESH_DATA);
	if (command->m_type == CMD_REQUEST_MESH_DATA)
	{
		command->m_updateFlags = D3_MESH_DATA_FLAGS;
		command->m_requestMeshDataArgs.m_flags = flags;
	}
}

DRX3D_SHARED_API void b3GetMeshDataSimulationMesh(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_MESH_DATA);
	command->m_updateFlags |= D3_MESH_DATA_SIMULATION_MESH;
}

DRX3D_SHARED_API void b3MeshDataSimulationMeshVelocity(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_MESH_DATA || command->m_type == CMD_RESET_MESH_DATA);
	command->m_updateFlags |= D3_MESH_DATA_SIMULATION_MESH_VELOCITY;
}

DRX3D_SHARED_API void b3GetMeshData(b3PhysicsClientHandle physClient, struct b3MeshData* meshData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedMeshData(meshData);
	}
}

DRX3D_SHARED_API i32 b3CreateVisualShapeAddSphere(b3SharedMemoryCommandHandle commandHandle, double radius)
{
	return b3CreateCollisionShapeAddSphere(commandHandle, radius);
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddBox(b3SharedMemoryCommandHandle commandHandle, const double halfExtents[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_BOX;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_boxHalfExtents[0] = halfExtents[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_boxHalfExtents[1] = halfExtents[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_boxHalfExtents[2] = halfExtents[2];
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateVisualShapeAddBox(b3SharedMemoryCommandHandle commandHandle, const double halfExtents[/*3*/])
{
	return b3CreateCollisionShapeAddBox(commandHandle, halfExtents);
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddCapsule(b3SharedMemoryCommandHandle commandHandle, double radius, double height)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_CAPSULE;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasFromTo = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_capsuleRadius = radius;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_capsuleHeight = height;
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateVisualShapeAddCapsule(b3SharedMemoryCommandHandle commandHandle, double radius, double height)
{
	return b3CreateCollisionShapeAddCapsule(commandHandle, radius, height);
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddHeightfield(b3SharedMemoryCommandHandle commandHandle, tukk fileName, const double meshScale[/*3*/], double textureScaling)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_HEIGHTFIELD;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			strcpy(command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileName, fileName);
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[0] = meshScale[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[1] = meshScale[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[2] = meshScale[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_heightfieldTextureScaling = textureScaling;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numHeightfieldRows = -1;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numHeightfieldColumns = -1;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_replaceHeightfieldIndex = -1;

			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddHeightfield2(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], double textureScaling, float* heightfieldData, i32 numHeightfieldRows, i32 numHeightfieldColumns, i32 replaceHeightfieldIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_HEIGHTFIELD;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileName[0] = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[0] = meshScale[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[1] = meshScale[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[2] = meshScale[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_heightfieldTextureScaling = textureScaling;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numHeightfieldRows = numHeightfieldRows;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numHeightfieldColumns = numHeightfieldColumns;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_replaceHeightfieldIndex = replaceHeightfieldIndex;
			cl->uploadBulletFileToSharedMemory((tukk)heightfieldData, numHeightfieldRows*numHeightfieldColumns* sizeof(float));
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddCylinder(b3SharedMemoryCommandHandle commandHandle, double radius, double height)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_CYLINDER;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_capsuleRadius = radius;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_capsuleHeight = height;
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}
DRX3D_SHARED_API i32 b3CreateVisualShapeAddCylinder(b3SharedMemoryCommandHandle commandHandle, double radius, double height)
{
	return b3CreateCollisionShapeAddCylinder(commandHandle, radius, height);
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddPlane(b3SharedMemoryCommandHandle commandHandle, const double planeNormal[3], double planeConstant)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_PLANE;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_planeNormal[0] = planeNormal[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_planeNormal[1] = planeNormal[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_planeNormal[2] = planeNormal[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_planeConstant = planeConstant;
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}
DRX3D_SHARED_API i32 b3CreateVisualShapeAddPlane(b3SharedMemoryCommandHandle commandHandle, const double planeNormal[/*3*/], double planeConstant)
{
	return b3CreateCollisionShapeAddPlane(commandHandle, planeNormal, planeConstant);
}
DRX3D_SHARED_API i32 b3CreateCollisionShapeAddMesh(b3SharedMemoryCommandHandle commandHandle, tukk fileName, const double meshScale[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES && strlen(fileName) < VISUAL_SHAPE_MAX_PATH_LEN)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_MESH;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			strcpy(command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileName, fileName);
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[0] = meshScale[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[1] = meshScale[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[2] = meshScale[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileType = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numVertices = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numIndices = 0;

			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddConvexMesh(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], const double* vertices, i32 numVertices)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);

	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES && numVertices >= 0)
		{
			i32 i=0;
			if (numVertices>D3_MAX_NUM_VERTICES)
				numVertices=D3_MAX_NUM_VERTICES;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_MESH;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[0] = meshScale[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[1] = meshScale[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[2] = meshScale[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileType = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileName[0]=0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numVertices = numVertices;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numIndices = 0;

			cl->uploadBulletFileToSharedMemory((tukk)vertices, numVertices * sizeof(double)*3);
			command->m_createUserShapeArgs.m_numUserShapes++;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateCollisionShapeAddConcaveMesh(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], const double* vertices, i32 numVertices, i32k* indices, i32 numIndices)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES && numVertices >= 0 && numIndices >=0)
		{
			i32 i=0;
			if (numVertices>D3_MAX_NUM_VERTICES)
				numVertices=D3_MAX_NUM_VERTICES;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_MESH;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = GEOM_FORCE_CONCAVE_TRIMESH;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[0] = meshScale[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[1] = meshScale[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[2] = meshScale[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileType = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileName[0]=0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numVertices = numVertices;

			i32 totalUploadSizeInBytes = numVertices * sizeof(double) *3  + numIndices * sizeof(i32);
			tuk data = new char[totalUploadSizeInBytes];
			double* vertexUpload = (double*)data;
			i32* indexUpload = (i32*)(data + numVertices*sizeof(double)*3);

			for (i=0;i<numVertices;i++)
			{
				vertexUpload[i*3+0]=vertices[i*3+0];
				vertexUpload[i*3+1]=vertices[i*3+1];
				vertexUpload[i*3+2]=vertices[i*3+2];
			}
			if (numIndices>D3_MAX_NUM_INDICES)
				numIndices = D3_MAX_NUM_INDICES;

			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numIndices = numIndices;
			for (i=0;i<numIndices;i++)
			{
				indexUpload[i]=indices[i];
			}
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numNormals = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numUVs = 0;

			command->m_createUserShapeArgs.m_numUserShapes++;
			cl->uploadBulletFileToSharedMemory(data, totalUploadSizeInBytes);
			delete [] data;
			return shapeIndex;
		}
	}
	return -1;
}

DRX3D_SHARED_API i32 b3CreateVisualShapeAddMesh2(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double meshScale[/*3*/], const double* vertices, i32 numVertices, i32k* indices, i32 numIndices, const double* normals, i32 numNormals, const double* uvs, i32 numUVs)
{
	if (numUVs == 0 && numNormals == 0)
	{
		return b3CreateCollisionShapeAddConcaveMesh(physClient, commandHandle, meshScale, vertices, numVertices, indices, numIndices);
	}

	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		i32 shapeIndex = command->m_createUserShapeArgs.m_numUserShapes;
		if (shapeIndex < MAX_COMPOUND_COLLISION_SHAPES && numVertices >= 0 && numIndices >= 0)
		{
			i32 i = 0;
			if (numVertices>D3_MAX_NUM_VERTICES)
				numVertices = D3_MAX_NUM_VERTICES;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_type = GEOM_MESH;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags = GEOM_FORCE_CONCAVE_TRIMESH;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[0] = meshScale[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[1] = meshScale[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshScale[2] = meshScale[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileType = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_meshFileName[0] = 0;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numVertices = numVertices;

			i32 totalUploadSizeInBytes = numVertices * sizeof(double) * 3 + numIndices * sizeof(i32) + numNormals*sizeof(double)*3+numUVs*sizeof(double)*2;
			tuk data = new char[totalUploadSizeInBytes];
			double* vertexUpload = (double*)data;
			i32* indexUpload = (i32*)(data + numVertices * sizeof(double) * 3);
			double* normalUpload = (double*)(data + numVertices * sizeof(double) * 3 + numIndices * sizeof(i32));
			double* uvUpload = (double*)(data + numVertices * sizeof(double) * 3 + numIndices * sizeof(i32)+ numNormals * sizeof(double) * 3);
			for (i = 0; i<numVertices; i++)
			{
				vertexUpload[i * 3 + 0] = vertices[i * 3 + 0];
				vertexUpload[i * 3 + 1] = vertices[i * 3 + 1];
				vertexUpload[i * 3 + 2] = vertices[i * 3 + 2];
			}
			if (numIndices>D3_MAX_NUM_INDICES)
				numIndices = D3_MAX_NUM_INDICES;

			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numIndices = numIndices;
			for (i = 0; i<numIndices; i++)
			{
				indexUpload[i] = indices[i];
			}

			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numNormals = numNormals;
			for (i = 0; i < numNormals; i++)
			{
				normalUpload[i * 3 + 0] = normals[i * 3 + 0];
				normalUpload[i * 3 + 1] = normals[i * 3 + 1];
				normalUpload[i * 3 + 2] = normals[i * 3 + 2];
			}

			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_numUVs = numUVs;
			for (i = 0; i < numUVs; i++)
			{
				uvUpload[i * 2 + 0] = uvs[i * 2 + 0];
				uvUpload[i * 2 + 1] = uvs[i * 2 + 1];
			}
			command->m_createUserShapeArgs.m_numUserShapes++;
			cl->uploadBulletFileToSharedMemory(data, totalUploadSizeInBytes);
			delete[] data;
			return shapeIndex;
		}
	}
	return -1;
}


DRX3D_SHARED_API i32 b3CreateVisualShapeAddMesh(b3SharedMemoryCommandHandle commandHandle, tukk fileName, const double meshScale[/*3*/])
{
	return b3CreateCollisionShapeAddMesh(commandHandle, fileName, meshScale);
}

DRX3D_SHARED_API void b3CreateCollisionSetFlag(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		if (shapeIndex < command->m_createUserShapeArgs.m_numUserShapes)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_collisionFlags |= flags;
		}
	}
}
DRX3D_SHARED_API void b3CreateVisualSetFlag(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, i32 flags)
{
	b3CreateCollisionSetFlag(commandHandle, shapeIndex, flags);
}

DRX3D_SHARED_API void b3CreateCollisionShapeSetChildTransform(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double childPosition[3], const double childOrientation[4])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		if (shapeIndex < command->m_createUserShapeArgs.m_numUserShapes)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_hasChildTransform = 1;
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childPosition[0] = childPosition[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childPosition[1] = childPosition[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childPosition[2] = childPosition[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childOrientation[0] = childOrientation[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childOrientation[1] = childOrientation[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childOrientation[2] = childOrientation[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_childOrientation[3] = childOrientation[3];
		}
	}
}

DRX3D_SHARED_API void b3CreateVisualShapeSetChildTransform(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double childPosition[/*3*/], const double childOrientation[/*4*/])
{
	b3CreateCollisionShapeSetChildTransform(commandHandle, shapeIndex, childPosition, childOrientation);
}

DRX3D_SHARED_API void b3CreateVisualShapeSetRGBAColor(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double rgbaColor[/*4*/])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		if (shapeIndex < command->m_createUserShapeArgs.m_numUserShapes)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_rgbaColor[0] = rgbaColor[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_rgbaColor[1] = rgbaColor[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_rgbaColor[2] = rgbaColor[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_rgbaColor[3] = rgbaColor[3];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags |= GEOM_VISUAL_HAS_RGBA_COLOR;
		}
	}
}

DRX3D_SHARED_API void b3CreateVisualShapeSetSpecularColor(b3SharedMemoryCommandHandle commandHandle, i32 shapeIndex, const double specularColor[/*3*/])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE));
	if ((command->m_type == CMD_CREATE_COLLISION_SHAPE) || (command->m_type == CMD_CREATE_VISUAL_SHAPE))
	{
		if (shapeIndex < command->m_createUserShapeArgs.m_numUserShapes)
		{
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_specularColor[0] = specularColor[0];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_specularColor[1] = specularColor[1];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_specularColor[2] = specularColor[2];
			command->m_createUserShapeArgs.m_shapes[shapeIndex].m_visualFlags |= GEOM_VISUAL_HAS_SPECULAR_COLOR;
		}
	}
}

DRX3D_SHARED_API i32 b3GetStatusCollisionShapeUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	drx3DAssert(status->m_type == CMD_CREATE_COLLISION_SHAPE_COMPLETED);
	if (status && status->m_type == CMD_CREATE_COLLISION_SHAPE_COMPLETED)
	{
		return status->m_createUserShapeResultArgs.m_userShapeUniqueId;
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateVisualShapeCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	if (cl)
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_CREATE_VISUAL_SHAPE;
		command->m_updateFlags = 0;
		command->m_createUserShapeArgs.m_numUserShapes = 0;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3GetStatusVisualShapeUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	drx3DAssert(status->m_type == CMD_CREATE_VISUAL_SHAPE_COMPLETED);
	if (status && status->m_type == CMD_CREATE_VISUAL_SHAPE_COMPLETED)
	{
		return status->m_createUserShapeResultArgs.m_userShapeUniqueId;
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateMultiBodyCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	if (cl)
	{
		struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
		drx3DAssert(command);
		command->m_type = CMD_CREATE_MULTI_BODY;
		command->m_updateFlags = 0;
		command->m_createMultiBodyArgs.m_bodyName[0] = 0;
		command->m_createMultiBodyArgs.m_baseLinkIndex = -1;
		command->m_createMultiBodyArgs.m_numLinks = 0;
		command->m_createMultiBodyArgs.m_numBatchObjects = 0;
		return (b3SharedMemoryCommandHandle)command;
	}
	return 0;
}

//batch creation is an performance feature to create a large number of multi bodies in one command
DRX3D_SHARED_API i32 b3CreateMultiBodySetBatchPositions(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, double* batchPositions, i32 numBatchObjects)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_MULTI_BODY);
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	
	if (cl && command->m_type == CMD_CREATE_MULTI_BODY)
	{
		command->m_createMultiBodyArgs.m_numBatchObjects = numBatchObjects;
		cl->uploadBulletFileToSharedMemory((tukk)batchPositions, sizeof(double) * 3 * numBatchObjects);
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreateMultiBodyBase(b3SharedMemoryCommandHandle commandHandle, double mass, i32 collisionShapeUnique, i32 visualShapeUniqueId, const double basePosition[3], const double baseOrientation[4], const double baseInertialFramePosition[3], const double baseInertialFrameOrientation[4])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_MULTI_BODY);
	if (command->m_type == CMD_CREATE_MULTI_BODY)
	{
		i32 numLinks = command->m_createMultiBodyArgs.m_numLinks;

		if (numLinks < MAX_CREATE_MULTI_BODY_LINKS)
		{
			i32 baseLinkIndex = numLinks;
			command->m_updateFlags |= MULTI_BODY_HAS_BASE;
			command->m_createMultiBodyArgs.m_baseLinkIndex = baseLinkIndex;
			command->m_createMultiBodyArgs.m_linkPositions[baseLinkIndex * 3 + 0] = basePosition[0];
			command->m_createMultiBodyArgs.m_linkPositions[baseLinkIndex * 3 + 1] = basePosition[1];
			command->m_createMultiBodyArgs.m_linkPositions[baseLinkIndex * 3 + 2] = basePosition[2];

			command->m_createMultiBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 0] = baseOrientation[0];
			command->m_createMultiBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 1] = baseOrientation[1];
			command->m_createMultiBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 2] = baseOrientation[2];
			command->m_createMultiBodyArgs.m_linkOrientations[baseLinkIndex * 4 + 3] = baseOrientation[3];

			command->m_createMultiBodyArgs.m_linkInertias[baseLinkIndex * 3 + 0] = 0;  //unused, is computed automatically. Will add a method to explicitly set it (with a flag), similar to loadURDF etc.
			command->m_createMultiBodyArgs.m_linkInertias[baseLinkIndex * 3 + 1] = 0;
			command->m_createMultiBodyArgs.m_linkInertias[baseLinkIndex * 3 + 2] = 0;

			command->m_createMultiBodyArgs.m_linkInertialFramePositions[baseLinkIndex * 3 + 0] = baseInertialFramePosition[0];
			command->m_createMultiBodyArgs.m_linkInertialFramePositions[baseLinkIndex * 3 + 1] = baseInertialFramePosition[1];
			command->m_createMultiBodyArgs.m_linkInertialFramePositions[baseLinkIndex * 3 + 2] = baseInertialFramePosition[2];

			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[baseLinkIndex * 4 + 0] = baseInertialFrameOrientation[0];
			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[baseLinkIndex * 4 + 1] = baseInertialFrameOrientation[1];
			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[baseLinkIndex * 4 + 2] = baseInertialFrameOrientation[2];
			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[baseLinkIndex * 4 + 3] = baseInertialFrameOrientation[3];

			command->m_createMultiBodyArgs.m_linkCollisionShapeUniqueIds[baseLinkIndex] = collisionShapeUnique;
			command->m_createMultiBodyArgs.m_linkVisualShapeUniqueIds[baseLinkIndex] = visualShapeUniqueId;

			command->m_createMultiBodyArgs.m_linkMasses[baseLinkIndex] = mass;

			command->m_createMultiBodyArgs.m_linkParentIndices[baseLinkIndex] = -2;  //no parent
			command->m_createMultiBodyArgs.m_linkJointAxis[baseLinkIndex + 0] = 0;
			command->m_createMultiBodyArgs.m_linkJointAxis[baseLinkIndex + 1] = 0;
			command->m_createMultiBodyArgs.m_linkJointAxis[baseLinkIndex + 2] = 0;
			command->m_createMultiBodyArgs.m_linkJointTypes[baseLinkIndex] = -1;
			command->m_createMultiBodyArgs.m_numLinks++;
		}
		return numLinks;
	}
	return -2;
}

DRX3D_SHARED_API i32 b3CreateMultiBodyLink(b3SharedMemoryCommandHandle commandHandle, double linkMass, double linkCollisionShapeIndex,
										double linkVisualShapeIndex,
										const double linkPosition[3],
										const double linkOrientation[4],
										const double linkInertialFramePosition[3],
										const double linkInertialFrameOrientation[4],
										i32 linkParentIndex,
										i32 linkJointType,
										const double linkJointAxis[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_MULTI_BODY);
	if (command->m_type == CMD_CREATE_MULTI_BODY)
	{
		i32 numLinks = command->m_createMultiBodyArgs.m_numLinks;

		if (numLinks < MAX_CREATE_MULTI_BODY_LINKS)
		{
			i32 linkIndex = numLinks;
			command->m_updateFlags |= MULTI_BODY_HAS_BASE;
			command->m_createMultiBodyArgs.m_linkPositions[linkIndex * 3 + 0] = linkPosition[0];
			command->m_createMultiBodyArgs.m_linkPositions[linkIndex * 3 + 1] = linkPosition[1];
			command->m_createMultiBodyArgs.m_linkPositions[linkIndex * 3 + 2] = linkPosition[2];

			command->m_createMultiBodyArgs.m_linkOrientations[linkIndex * 4 + 0] = linkOrientation[0];
			command->m_createMultiBodyArgs.m_linkOrientations[linkIndex * 4 + 1] = linkOrientation[1];
			command->m_createMultiBodyArgs.m_linkOrientations[linkIndex * 4 + 2] = linkOrientation[2];
			command->m_createMultiBodyArgs.m_linkOrientations[linkIndex * 4 + 3] = linkOrientation[3];

			command->m_createMultiBodyArgs.m_linkInertias[linkIndex * 3 + 0] = linkMass;
			command->m_createMultiBodyArgs.m_linkInertias[linkIndex * 3 + 1] = linkMass;
			command->m_createMultiBodyArgs.m_linkInertias[linkIndex * 3 + 2] = linkMass;

			command->m_createMultiBodyArgs.m_linkInertialFramePositions[linkIndex * 3 + 0] = linkInertialFramePosition[0];
			command->m_createMultiBodyArgs.m_linkInertialFramePositions[linkIndex * 3 + 1] = linkInertialFramePosition[1];
			command->m_createMultiBodyArgs.m_linkInertialFramePositions[linkIndex * 3 + 2] = linkInertialFramePosition[2];

			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[linkIndex * 4 + 0] = linkInertialFrameOrientation[0];
			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[linkIndex * 4 + 1] = linkInertialFrameOrientation[1];
			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[linkIndex * 4 + 2] = linkInertialFrameOrientation[2];
			command->m_createMultiBodyArgs.m_linkInertialFrameOrientations[linkIndex * 4 + 3] = linkInertialFrameOrientation[3];

			command->m_createMultiBodyArgs.m_linkCollisionShapeUniqueIds[linkIndex] = linkCollisionShapeIndex;
			command->m_createMultiBodyArgs.m_linkVisualShapeUniqueIds[linkIndex] = linkVisualShapeIndex;

			command->m_createMultiBodyArgs.m_linkParentIndices[linkIndex] = linkParentIndex;
			command->m_createMultiBodyArgs.m_linkJointTypes[linkIndex] = linkJointType;
			command->m_createMultiBodyArgs.m_linkJointAxis[3 * linkIndex + 0] = linkJointAxis[0];
			command->m_createMultiBodyArgs.m_linkJointAxis[3 * linkIndex + 1] = linkJointAxis[1];
			command->m_createMultiBodyArgs.m_linkJointAxis[3 * linkIndex + 2] = linkJointAxis[2];

			command->m_createMultiBodyArgs.m_linkMasses[linkIndex] = linkMass;
			
			command->m_createMultiBodyArgs.m_numLinks++;
			return numLinks;
		}
	}

	return -1;
}

//useMaximalCoordinates are disabled by default, enabling them is experimental and not fully supported yet
DRX3D_SHARED_API void b3CreateMultiBodyUseMaximalCoordinates(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_MULTI_BODY);
	if (command->m_type == CMD_CREATE_MULTI_BODY)
	{
		command->m_updateFlags |= MULT_BODY_USE_MAXIMAL_COORDINATES;
	}
}

DRX3D_SHARED_API void b3CreateMultiBodySetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_MULTI_BODY);
	if (command->m_type == CMD_CREATE_MULTI_BODY)
	{
		command->m_updateFlags |= MULT_BODY_HAS_FLAGS;
		command->m_createMultiBodyArgs.m_flags = flags;
	}
}

DRX3D_SHARED_API i32 b3GetStatusMultiBodyUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	drx3DAssert(status->m_type == CMD_CREATE_MULTI_BODY_COMPLETED);
	if (status && status->m_type == CMD_CREATE_MULTI_BODY_COMPLETED)
	{
		return status->m_createMultiBodyResultArgs.m_bodyUniqueId;
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateBoxShapeCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_CREATE_BOX_COLLISION_SHAPE;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3CreateBoxCommandSetStartPosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_BOX_COLLISION_SHAPE);
	command->m_updateFlags |= BOX_SHAPE_HAS_INITIAL_POSITION;

	command->m_createBoxShapeArguments.m_initialPosition[0] = startPosX;
	command->m_createBoxShapeArguments.m_initialPosition[1] = startPosY;
	command->m_createBoxShapeArguments.m_initialPosition[2] = startPosZ;
	return 0;
}

DRX3D_SHARED_API i32 b3CreateBoxCommandSetHalfExtents(b3SharedMemoryCommandHandle commandHandle, double halfExtentsX, double halfExtentsY, double halfExtentsZ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_BOX_COLLISION_SHAPE);
	command->m_updateFlags |= BOX_SHAPE_HAS_HALF_EXTENTS;

	command->m_createBoxShapeArguments.m_halfExtentsX = halfExtentsX;
	command->m_createBoxShapeArguments.m_halfExtentsY = halfExtentsY;
	command->m_createBoxShapeArguments.m_halfExtentsZ = halfExtentsZ;

	return 0;
}

DRX3D_SHARED_API i32 b3CreateBoxCommandSetMass(b3SharedMemoryCommandHandle commandHandle, double mass)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_BOX_COLLISION_SHAPE);
	command->m_updateFlags |= BOX_SHAPE_HAS_MASS;
	command->m_createBoxShapeArguments.m_mass = mass;
	return 0;
}

DRX3D_SHARED_API i32 b3CreateBoxCommandSetCollisionShapeType(b3SharedMemoryCommandHandle commandHandle, i32 collisionShapeType)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_BOX_COLLISION_SHAPE);
	command->m_updateFlags |= BOX_SHAPE_HAS_COLLISION_SHAPE_TYPE;
	command->m_createBoxShapeArguments.m_collisionShapeType = collisionShapeType;

	return 0;
}

DRX3D_SHARED_API i32 b3CreateBoxCommandSetColorRGBA(b3SharedMemoryCommandHandle commandHandle, double red, double green, double blue, double alpha)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_BOX_COLLISION_SHAPE);
	command->m_updateFlags |= BOX_SHAPE_HAS_COLOR;
	command->m_createBoxShapeArguments.m_colorRGBA[0] = red;
	command->m_createBoxShapeArguments.m_colorRGBA[1] = green;
	command->m_createBoxShapeArguments.m_colorRGBA[2] = blue;
	command->m_createBoxShapeArguments.m_colorRGBA[3] = alpha;
	return 0;
}

DRX3D_SHARED_API i32 b3CreateBoxCommandSetStartOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_BOX_COLLISION_SHAPE);
	command->m_updateFlags |= BOX_SHAPE_HAS_INITIAL_ORIENTATION;

	command->m_createBoxShapeArguments.m_initialOrientation[0] = startOrnX;
	command->m_createBoxShapeArguments.m_initialOrientation[1] = startOrnY;
	command->m_createBoxShapeArguments.m_initialOrientation[2] = startOrnZ;
	command->m_createBoxShapeArguments.m_initialOrientation[3] = startOrnW;
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreatePoseCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3CreatePoseCommandInit2((b3SharedMemoryCommandHandle)command, bodyUniqueId);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreatePoseCommandInit2(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_INIT_POSE;
	command->m_updateFlags = 0;
	command->m_initPoseArgs.m_bodyUniqueId = bodyUniqueId;
	//a bit slow, initialing the full range to zero...
	for (i32 i = 0; i < MAX_DEGREE_OF_FREEDOM; i++)
	{
		command->m_initPoseArgs.m_hasInitialStateQ[i] = 0;
		command->m_initPoseArgs.m_hasInitialStateQdot[i] = 0;
	}
	return commandHandle;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetBasePosition(b3SharedMemoryCommandHandle commandHandle, double startPosX, double startPosY, double startPosZ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_INITIAL_POSITION;
	command->m_initPoseArgs.m_initialStateQ[0] = startPosX;
	command->m_initPoseArgs.m_initialStateQ[1] = startPosY;
	command->m_initPoseArgs.m_initialStateQ[2] = startPosZ;

	command->m_initPoseArgs.m_hasInitialStateQ[0] = 1;
	command->m_initPoseArgs.m_hasInitialStateQ[1] = 1;
	command->m_initPoseArgs.m_hasInitialStateQ[2] = 1;

	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseScaling(b3SharedMemoryCommandHandle commandHandle, double scaling[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_SCALING;
	
	command->m_initPoseArgs.m_scaling[0] = scaling[0];
	command->m_initPoseArgs.m_scaling[1] = scaling[1];
	command->m_initPoseArgs.m_scaling[2] = scaling[2];

	return 0;
}



DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseOrientation(b3SharedMemoryCommandHandle commandHandle, double startOrnX, double startOrnY, double startOrnZ, double startOrnW)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_INITIAL_ORIENTATION;
	command->m_initPoseArgs.m_initialStateQ[3] = startOrnX;
	command->m_initPoseArgs.m_initialStateQ[4] = startOrnY;
	command->m_initPoseArgs.m_initialStateQ[5] = startOrnZ;
	command->m_initPoseArgs.m_initialStateQ[6] = startOrnW;

	command->m_initPoseArgs.m_hasInitialStateQ[3] = 1;
	command->m_initPoseArgs.m_hasInitialStateQ[4] = 1;
	command->m_initPoseArgs.m_hasInitialStateQ[5] = 1;
	command->m_initPoseArgs.m_hasInitialStateQ[6] = 1;

	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseLinearVelocity(b3SharedMemoryCommandHandle commandHandle, const double linVel[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_BASE_LINEAR_VELOCITY;
	command->m_initPoseArgs.m_hasInitialStateQdot[0] = 1;
	command->m_initPoseArgs.m_hasInitialStateQdot[1] = 1;
	command->m_initPoseArgs.m_hasInitialStateQdot[2] = 1;

	command->m_initPoseArgs.m_initialStateQdot[0] = linVel[0];
	command->m_initPoseArgs.m_initialStateQdot[1] = linVel[1];
	command->m_initPoseArgs.m_initialStateQdot[2] = linVel[2];

	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetBaseAngularVelocity(b3SharedMemoryCommandHandle commandHandle, const double angVel[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_BASE_ANGULAR_VELOCITY;
	command->m_initPoseArgs.m_hasInitialStateQdot[3] = 1;
	command->m_initPoseArgs.m_hasInitialStateQdot[4] = 1;
	command->m_initPoseArgs.m_hasInitialStateQdot[5] = 1;

	command->m_initPoseArgs.m_initialStateQdot[3] = angVel[0];
	command->m_initPoseArgs.m_initialStateQdot[4] = angVel[1];
	command->m_initPoseArgs.m_initialStateQdot[5] = angVel[2];

	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointPositions(b3SharedMemoryCommandHandle commandHandle, i32 numJointPositions, const double* jointPositions)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_JOINT_STATE;
	for (i32 i = 0; i < numJointPositions; i++)
	{
		if ((i + 7) < MAX_DEGREE_OF_FREEDOM)
		{
			command->m_initPoseArgs.m_initialStateQ[i + 7] = jointPositions[i];
			command->m_initPoseArgs.m_hasInitialStateQ[i + 7] = 1;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetQ(b3SharedMemoryCommandHandle commandHandle, i32 numJointPositions, const double* q, i32k* hasQ)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_JOINT_STATE;
	for (i32 i = 0; i < numJointPositions; i++)
	{
		if ((i) < MAX_DEGREE_OF_FREEDOM)
		{
			command->m_initPoseArgs.m_initialStateQ[i] = q[i];
			command->m_initPoseArgs.m_hasInitialStateQ[i] = hasQ[i];
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointPosition(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, double jointPosition)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_JOINT_STATE;
	b3JointInfo info;
	b3GetJointInfo(physClient, command->m_initPoseArgs.m_bodyUniqueId, jointIndex, &info);
	//Assert((info.m_flags & JOINT_HAS_MOTORIZED_POWER) && info.m_qIndex >=0);
	if ((info.m_flags & JOINT_HAS_MOTORIZED_POWER) && info.m_qIndex >= 0)
	{
		command->m_initPoseArgs.m_initialStateQ[info.m_qIndex] = jointPosition;
		command->m_initPoseArgs.m_hasInitialStateQ[info.m_qIndex] = 1;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointPositionMultiDof(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, const double* jointPosition, i32 posSize)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_JOINT_STATE;
	b3JointInfo info;
	b3GetJointInfo(physClient, command->m_initPoseArgs.m_bodyUniqueId, jointIndex, &info);
	//if ((info.m_flags & JOINT_HAS_MOTORIZED_POWER) && info.m_qIndex >= 0)
	if (info.m_qIndex >= 0)
	{
		if (posSize == info.m_qSize)
		{
			for (i32 i = 0; i < posSize; i++)
			{
				command->m_initPoseArgs.m_initialStateQ[info.m_qIndex + i] = jointPosition[i];
				command->m_initPoseArgs.m_hasInitialStateQ[info.m_qIndex + i] = 1;
			}
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointVelocity(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, double jointVelocity)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_JOINT_VELOCITY;
	b3JointInfo info;
	b3GetJointInfo(physClient, command->m_initPoseArgs.m_bodyUniqueId, jointIndex, &info);
	//Assert((info.m_flags & JOINT_HAS_MOTORIZED_POWER) && info.m_uIndex >=0);
	if ((info.m_flags & JOINT_HAS_MOTORIZED_POWER) && (info.m_uIndex >= 0) && (info.m_uIndex < MAX_DEGREE_OF_FREEDOM))
	{
		command->m_initPoseArgs.m_initialStateQdot[info.m_uIndex] = jointVelocity;
		command->m_initPoseArgs.m_hasInitialStateQdot[info.m_uIndex] = 1;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointVelocityMultiDof(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, const double* jointVelocity, i32 velSize)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);
	command->m_updateFlags |= INIT_POSE_HAS_JOINT_VELOCITY;
	b3JointInfo info;
	b3GetJointInfo(physClient, command->m_initPoseArgs.m_bodyUniqueId, jointIndex, &info);
	
	//if ((info.m_flags & JOINT_HAS_MOTORIZED_POWER) && (info.m_uIndex >= 0) && (info.m_uIndex < MAX_DEGREE_OF_FREEDOM))
	if ((info.m_uIndex >= 0) && (info.m_uIndex < MAX_DEGREE_OF_FREEDOM))
	{
		if (velSize == info.m_uSize)
		{
			for (i32 i = 0; i < velSize; i++)
			{
				command->m_initPoseArgs.m_initialStateQdot[info.m_uIndex + i] = jointVelocity[i];
				command->m_initPoseArgs.m_hasInitialStateQdot[info.m_uIndex + i] = 1;
			}
		}
		
	}
	return 0;
}


DRX3D_SHARED_API i32 b3CreatePoseCommandSetJointVelocities(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, i32 numJointVelocities, const double* jointVelocities)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);

	command->m_updateFlags |= INIT_POSE_HAS_JOINT_VELOCITY;
	for (i32 i = 0; i < numJointVelocities; i++)
	{
		if ((i + 6) < MAX_DEGREE_OF_FREEDOM)
		{
			command->m_initPoseArgs.m_initialStateQdot[i + 6] = jointVelocities[i];
			command->m_initPoseArgs.m_hasInitialStateQdot[i + 6] = 1;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3CreatePoseCommandSetQdots(b3SharedMemoryCommandHandle commandHandle, i32 numJointVelocities, const double* qDots, i32k* hasQdots)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_INIT_POSE);

	command->m_updateFlags |= INIT_POSE_HAS_JOINT_VELOCITY;
	for (i32 i = 0; i < numJointVelocities; i++)
	{
		if (i < MAX_DEGREE_OF_FREEDOM)
		{
			command->m_initPoseArgs.m_initialStateQdot[i] = qDots[i];
			command->m_initPoseArgs.m_hasInitialStateQdot[i] = hasQdots[i];
		}
	}
	return 0;
}



DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateSensorCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_CREATE_SENSOR;
	command->m_updateFlags = 0;
	command->m_createSensorArguments.m_numJointSensorChanges = 0;
	command->m_createSensorArguments.m_bodyUniqueId = bodyUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3CreateSensorEnable6DofJointForceTorqueSensor(b3SharedMemoryCommandHandle commandHandle, i32 jointIndex, i32 enable)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_SENSOR);
	i32 curIndex = command->m_createSensorArguments.m_numJointSensorChanges;
	command->m_createSensorArguments.m_sensorType[curIndex] = SENSOR_FORCE_TORQUE;

	command->m_createSensorArguments.m_jointIndex[curIndex] = jointIndex;
	command->m_createSensorArguments.m_enableJointForceSensor[curIndex] = enable;
	command->m_createSensorArguments.m_numJointSensorChanges++;
	return 0;
}

DRX3D_SHARED_API i32 b3CreateSensorEnableIMUForLink(b3SharedMemoryCommandHandle commandHandle, i32 linkIndex, i32 enable)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CREATE_SENSOR);
	i32 curIndex = command->m_createSensorArguments.m_numJointSensorChanges;
	command->m_createSensorArguments.m_sensorType[curIndex] = SENSOR_IMU;
	command->m_createSensorArguments.m_linkIndex[curIndex] = linkIndex;
	command->m_createSensorArguments.m_enableSensor[curIndex] = enable;
	command->m_createSensorArguments.m_numJointSensorChanges++;
	return 0;
}

DRX3D_SHARED_API void b3DisconnectSharedMemory(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->disconnectSharedMemory();
	}
	delete cl;
}

DRX3D_SHARED_API b3SharedMemoryStatusHandle b3ProcessServerStatus(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl && cl->isConnected())
	{
		const SharedMemoryStatus* stat = cl->processServerStatus();
		return (b3SharedMemoryStatusHandle)stat;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3GetStatusType(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	//drx3DAssert(status);
	if (status)
	{
		return status->m_type;
	}
	return CMD_INVALID_STATUS;
}

DRX3D_SHARED_API i32 b3GetStatusForwardDynamicsAnalyticsData(b3SharedMemoryStatusHandle statusHandle, struct b3ForwardDynamicsAnalyticsArgs* analyticsData)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	//drx3DAssert(status);
	if (status)
	{
		*analyticsData = status->m_forwardDynamicsAnalyticsArgs;
		return status->m_forwardDynamicsAnalyticsArgs.m_numIslands;
	}
	return 0;
}


DRX3D_SHARED_API i32 b3GetStatusBodyIndices(b3SharedMemoryStatusHandle statusHandle, i32* bodyIndicesOut, i32 bodyIndicesCapacity)
{
	i32 numBodies = 0;
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);

	if (status)
	{
		switch (status->m_type)
		{
			case CMD_MJCF_LOADING_COMPLETED:
			case CMD_DRX3D_LOADING_COMPLETED:
			case CMD_SDF_LOADING_COMPLETED:
			{
				i32 i, maxBodies;
				numBodies = status->m_sdfLoadedArgs.m_numBodies;
				maxBodies = d3Min(bodyIndicesCapacity, numBodies);
				for (i = 0; i < maxBodies; i++)
				{
					bodyIndicesOut[i] = status->m_sdfLoadedArgs.m_bodyUniqueIds[i];
				}
				break;
			}
		}
	}

	return numBodies;
}

DRX3D_SHARED_API i32 b3GetStatusBodyIndex(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	i32 bodyId = -1;

	if (status)
	{
		switch (status->m_type)
		{
			case CMD_URDF_LOADING_COMPLETED:
			{
				bodyId = status->m_dataStreamArguments.m_bodyUniqueId;
				break;
			}
			case CMD_RIGID_BODY_CREATION_COMPLETED:
			{
				bodyId = status->m_rigidBodyCreateArgs.m_bodyUniqueId;
				break;
			}
			case CMD_CREATE_MULTI_BODY_COMPLETED:
			{
				bodyId = status->m_dataStreamArguments.m_bodyUniqueId;
				break;
			}
			case CMD_LOAD_SOFT_BODY_COMPLETED:
			{
				bodyId = status->m_loadSoftBodyResultArguments.m_objectUniqueId;
				break;
			}
			default:
			{
				drx3DAssert(0);
			}
		};
	}
	return bodyId;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestCollisionInfoCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_COLLISION_INFO;
	command->m_updateFlags = 0;
	command->m_requestCollisionInfoArgs.m_bodyUniqueId = bodyUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusAABB(b3SharedMemoryStatusHandle statusHandle, i32 linkIndex, double aabbMin[3], double aabbMax[3])
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	Assert(status);
	if (status == 0)
		return 0;
	const b3SendCollisionInfoArgs& args = status->m_sendCollisionInfoArgs;
	Assert(status->m_type == CMD_REQUEST_COLLISION_INFO_COMPLETED);
	if (status->m_type != CMD_REQUEST_COLLISION_INFO_COMPLETED)
		return 0;

	if (linkIndex == -1)
	{
		aabbMin[0] = args.m_rootWorldAABBMin[0];
		aabbMin[1] = args.m_rootWorldAABBMin[1];
		aabbMin[2] = args.m_rootWorldAABBMin[2];

		aabbMax[0] = args.m_rootWorldAABBMax[0];
		aabbMax[1] = args.m_rootWorldAABBMax[1];
		aabbMax[2] = args.m_rootWorldAABBMax[2];
		return 1;
	}

	if (linkIndex >= 0 && linkIndex < args.m_numLinks)
	{
		aabbMin[0] = args.m_linkWorldAABBsMin[linkIndex * 3 + 0];
		aabbMin[1] = args.m_linkWorldAABBsMin[linkIndex * 3 + 1];
		aabbMin[2] = args.m_linkWorldAABBsMin[linkIndex * 3 + 2];

		aabbMax[0] = args.m_linkWorldAABBsMax[linkIndex * 3 + 0];
		aabbMax[1] = args.m_linkWorldAABBsMax[linkIndex * 3 + 1];
		aabbMax[2] = args.m_linkWorldAABBsMax[linkIndex * 3 + 2];
		return 1;
	}

	return 0;
}

DRX3D_SHARED_API i32 b3GetStatusActualState2(b3SharedMemoryStatusHandle statusHandle,
										  i32* bodyUniqueId,
										  i32* numLinks,
										  i32* numDegreeOfFreedomQ,
										  i32* numDegreeOfFreedomU,
										  const double* rootLocalInertialFrame[],
										  const double* actualStateQ[],
										  const double* actualStateQdot[],
										  const double* jointReactionForces[],
										  const double* linkLocalInertialFrames[],
										  const double* jointMotorForces[],
										  const double* linkStates[],
										  const double* linkWorldVelocities[])
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	Assert(status);
	if (status == 0)
		return 0;

	b3GetStatusActualState(statusHandle, bodyUniqueId, numDegreeOfFreedomQ, numDegreeOfFreedomU,
						   rootLocalInertialFrame, actualStateQ, actualStateQdot, jointReactionForces);

	const SendActualStateArgs& args = status->m_sendActualStateArgs;
	if (numLinks)
	{
		*numLinks = args.m_numLinks;
	}
	if (linkLocalInertialFrames)
	{
		*linkLocalInertialFrames = args.m_stateDetails->m_linkLocalInertialFrames;
	}
	if (jointMotorForces)
	{
		*jointMotorForces = args.m_stateDetails->m_jointMotorForce;
	}
	if (linkStates)
	{
		*linkStates = args.m_stateDetails->m_linkState;
	}
	if (linkWorldVelocities)
	{
		*linkWorldVelocities = args.m_stateDetails->m_linkWorldVelocities;
	}
	return 1;
}

DRX3D_SHARED_API i32 b3GetStatusActualState(b3SharedMemoryStatusHandle statusHandle,
										 i32* bodyUniqueId,
										 i32* numDegreeOfFreedomQ,
										 i32* numDegreeOfFreedomU,
										 const double* rootLocalInertialFrame[],
										 const double* actualStateQ[],
										 const double* actualStateQdot[],
										 const double* jointReactionForces[])
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	Assert(status);
	if (status == 0)
		return 0;
	const SendActualStateArgs& args = status->m_sendActualStateArgs;
	Assert(status->m_type == CMD_ACTUAL_STATE_UPDATE_COMPLETED);
	if (status->m_type != CMD_ACTUAL_STATE_UPDATE_COMPLETED)
		return false;

	if (bodyUniqueId)
	{
		*bodyUniqueId = args.m_bodyUniqueId;
	}
	if (numDegreeOfFreedomQ)
	{
		*numDegreeOfFreedomQ = args.m_numDegreeOfFreedomQ;
	}
	if (numDegreeOfFreedomU)
	{
		*numDegreeOfFreedomU = args.m_numDegreeOfFreedomU;
	}
	if (rootLocalInertialFrame)
	{
		*rootLocalInertialFrame = args.m_rootLocalInertialFrame;
	}
	if (actualStateQ)
	{
		*actualStateQ = args.m_stateDetails->m_actualStateQ;
	}
	if (actualStateQdot)
	{
		*actualStateQdot = args.m_stateDetails->m_actualStateQdot;
	}
	if (jointReactionForces)
	{
		*jointReactionForces = args.m_stateDetails->m_jointReactionForces;
	}
	return true;
}

DRX3D_SHARED_API i32 b3CanSubmitCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		return (i32)cl->canSubmitCommand();
	}
	return false;
}

DRX3D_SHARED_API i32 b3SubmitClientCommand(b3PhysicsClientHandle physClient, const b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(command);
	drx3DAssert(cl);
	if (command && cl)
	{
		return (i32)cl->submitClientCommand(*command);
	}
	return -1;
}

#include <drx3D/Common/b3Clock.h>

DRX3D_SHARED_API b3SharedMemoryStatusHandle b3SubmitClientCommandAndWaitStatus(b3PhysicsClientHandle physClient, const b3SharedMemoryCommandHandle commandHandle)
{
	D3_PROFILE("b3SubmitClientCommandAndWaitStatus");
	b3Clock clock;
	double startTime = clock.getTimeInSeconds();

	b3SharedMemoryStatusHandle statusHandle = 0;
	drx3DAssert(commandHandle);
	drx3DAssert(physClient);
	if (physClient && commandHandle)
	{
		PhysicsClient* cl = (PhysicsClient*)physClient;

		double timeOutInSeconds = cl->getTimeOut();

		{
			D3_PROFILE("b3SubmitClientCommand");
			b3SubmitClientCommand(physClient, commandHandle);
		}
		{
			D3_PROFILE("b3ProcessServerStatus");
			while (cl->isConnected() && (statusHandle == 0) && (clock.getTimeInSeconds() - startTime < timeOutInSeconds))
			{
				clock.usleep(0);
				statusHandle = b3ProcessServerStatus(physClient);
			}
		}
		return (b3SharedMemoryStatusHandle)statusHandle;
	}

	return 0;
}

///return the total number of bodies in the simulation
DRX3D_SHARED_API i32 b3GetNumBodies(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getNumBodies();
}

DRX3D_SHARED_API i32 b3GetNumUserConstraints(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getNumUserConstraints();
}

DRX3D_SHARED_API i32 b3GetUserConstraintInfo(b3PhysicsClientHandle physClient, i32 constraintUniqueId, struct b3UserConstraint* info)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	b3UserConstraint constraintInfo1;
	drx3DAssert(physClient);
	drx3DAssert(info);
	drx3DAssert(constraintUniqueId >= 0);

	if (info == 0)
		return 0;

	if (cl->getUserConstraintInfo(constraintUniqueId, constraintInfo1))
	{
		*info = constraintInfo1;
		return 1;
	}
	return 0;
}

/// return the user constraint id, given the index in range [0 , b3GetNumUserConstraints() )
DRX3D_SHARED_API i32 b3GetUserConstraintId(b3PhysicsClientHandle physClient, i32 serialIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getUserConstraintId(serialIndex);
}

/// return the body unique id, given the index in range [0 , b3GetNumBodies() )
DRX3D_SHARED_API i32 b3GetBodyUniqueId(b3PhysicsClientHandle physClient, i32 serialIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getBodyUniqueId(serialIndex);
}

///given a body unique id, return the body information. See b3BodyInfo in SharedMemoryPublic.h
DRX3D_SHARED_API i32 b3GetBodyInfo(b3PhysicsClientHandle physClient, i32 bodyUniqueId, struct b3BodyInfo* info)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getBodyInfo(bodyUniqueId, *info);
}

DRX3D_SHARED_API i32 b3GetNumJoints(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getNumJoints(bodyUniqueId);
}

DRX3D_SHARED_API i32 b3GetNumDofs(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
        PhysicsClient* cl = (PhysicsClient*)physClient;
        return cl->getNumDofs(bodyUniqueId);
}

DRX3D_SHARED_API i32 b3ComputeDofCount(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	i32 nj = b3GetNumJoints(physClient, bodyUniqueId);
	i32 j = 0;
	i32 dofCountOrg = 0;
	for (j = 0; j < nj; j++)
	{
		struct b3JointInfo info;
		b3GetJointInfo(physClient, bodyUniqueId, j, &info);
		switch (info.m_jointType)
		{
			case eRevoluteType:
			{
				dofCountOrg += 1;
				break;
			}
			case ePrismaticType:
			{
				dofCountOrg += 1;
				break;
			}
			case eSphericalType:
			{
				return -1;
			}
			case ePlanarType:
			{
				return -2;
			}
			default:
			{
				//fixed joint has 0-dof and at the moment, we don't deal with planar, spherical etc
			}
		}
	}
	return dofCountOrg;
}

DRX3D_SHARED_API i32 b3GetJointInfo(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 jointIndex, struct b3JointInfo* info)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	return cl->getJointInfo(bodyUniqueId, jointIndex, *info);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateCustomCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_CUSTOM_COMMAND;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3CustomCommandLoadPlugin(b3SharedMemoryCommandHandle commandHandle, tukk pluginPath)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CUSTOM_COMMAND);
	if (command->m_type == CMD_CUSTOM_COMMAND)
	{
		command->m_updateFlags |= CMD_CUSTOM_COMMAND_LOAD_PLUGIN;
		command->m_customCommandArgs.m_pluginPath[0] = 0;

		i32 len = strlen(pluginPath);
		if (len < MAX_FILENAME_LENGTH)
		{
			strcpy(command->m_customCommandArgs.m_pluginPath, pluginPath);
		}
	}
}

DRX3D_SHARED_API void b3CustomCommandLoadPluginSetPostFix(b3SharedMemoryCommandHandle commandHandle, tukk postFix)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CUSTOM_COMMAND);
	if (command->m_type == CMD_CUSTOM_COMMAND)
	{
		command->m_updateFlags |= CMD_CUSTOM_COMMAND_LOAD_PLUGIN_POSTFIX;
		command->m_customCommandArgs.m_postFix[0] = 0;

		i32 len = strlen(postFix);
		if (len < MAX_FILENAME_LENGTH)
		{
			strcpy(command->m_customCommandArgs.m_postFix, postFix);
		}
	}
}

DRX3D_SHARED_API i32 b3GetStatusPluginCommandResult(b3SharedMemoryStatusHandle statusHandle)
{
	i32 statusUniqueId = -1;

	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	if (status == 0)
		return statusUniqueId;
	drx3DAssert(status->m_type == CMD_CUSTOM_COMMAND_COMPLETED);
	if (status->m_type == CMD_CUSTOM_COMMAND_COMPLETED)
	{
		statusUniqueId = status->m_customCommandResultArgs.m_executeCommandResult;
	}
	return statusUniqueId;
}

DRX3D_SHARED_API i32 b3GetStatusPluginCommandReturnData(b3PhysicsClientHandle physClient, struct b3UserDataValue* valueOut)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		return cl->getCachedReturnData(valueOut);
	}
	return false;
}

DRX3D_SHARED_API i32 b3GetStatusPluginUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	i32 statusUniqueId = -1;

	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	if (status)
	{
		drx3DAssert(status->m_type == CMD_CUSTOM_COMMAND_COMPLETED);
		if (status->m_type == CMD_CUSTOM_COMMAND_COMPLETED)
		{
			statusUniqueId = status->m_customCommandResultArgs.m_pluginUniqueId;
		}
	}
	return statusUniqueId;
}

DRX3D_SHARED_API void b3CustomCommandUnloadPlugin(b3SharedMemoryCommandHandle commandHandle, i32 pluginUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CUSTOM_COMMAND);
	if (command->m_type == CMD_CUSTOM_COMMAND)
	{
		command->m_updateFlags |= CMD_CUSTOM_COMMAND_UNLOAD_PLUGIN;
		command->m_customCommandArgs.m_pluginUniqueId = pluginUniqueId;
	}
}
DRX3D_SHARED_API void b3CustomCommandExecutePluginCommand(b3SharedMemoryCommandHandle commandHandle, i32 pluginUniqueId, tukk textArguments)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CUSTOM_COMMAND);
	if (command->m_type == CMD_CUSTOM_COMMAND)
	{
		command->m_updateFlags |= CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND;
		command->m_customCommandArgs.m_pluginUniqueId = pluginUniqueId;
		command->m_customCommandArgs.m_startingReturnBytes = 0;
		command->m_customCommandArgs.m_arguments.m_numInts = 0;
		command->m_customCommandArgs.m_arguments.m_numFloats = 0;
		command->m_customCommandArgs.m_arguments.m_text[0] = 0;

		i32 len = textArguments ? strlen(textArguments) : 0;

		if (len && len < MAX_FILENAME_LENGTH)
		{
			strcpy(command->m_customCommandArgs.m_arguments.m_text, textArguments);
		}
	}
}

DRX3D_SHARED_API void b3CustomCommandExecuteAddIntArgument(b3SharedMemoryCommandHandle commandHandle, i32 intVal)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CUSTOM_COMMAND);
	drx3DAssert(command->m_updateFlags & CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND);
	if (command->m_type == CMD_CUSTOM_COMMAND && (command->m_updateFlags & CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND))
	{
		i32 numInts = command->m_customCommandArgs.m_arguments.m_numInts;
		if (numInts < D3_MAX_PLUGIN_ARG_SIZE)
		{
			command->m_customCommandArgs.m_arguments.m_ints[numInts] = intVal;
			command->m_customCommandArgs.m_arguments.m_numInts++;
		}
	}
}

DRX3D_SHARED_API void b3CustomCommandExecuteAddFloatArgument(b3SharedMemoryCommandHandle commandHandle, float floatVal)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CUSTOM_COMMAND);
	drx3DAssert(command->m_updateFlags & CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND);
	if (command->m_type == CMD_CUSTOM_COMMAND && (command->m_updateFlags & CMD_CUSTOM_COMMAND_EXECUTE_PLUGIN_COMMAND))
	{
		i32 numFloats = command->m_customCommandArgs.m_arguments.m_numFloats;
		if (numFloats < D3_MAX_PLUGIN_ARG_SIZE)
		{
			command->m_customCommandArgs.m_arguments.m_floats[numFloats] = floatVal;
			command->m_customCommandArgs.m_arguments.m_numFloats++;
		}
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3GetDynamicsInfoCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3GetDynamicsInfoCommandInit2((b3SharedMemoryCommandHandle)command, bodyUniqueId, linkIndex);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3GetDynamicsInfoCommandInit2(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_GET_DYNAMICS_INFO;
	command->m_getDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_getDynamicsInfoArgs.m_linkIndex = linkIndex;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetDynamicsInfo(b3SharedMemoryStatusHandle statusHandle, struct b3DynamicsInfo* info)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	const b3DynamicsInfo& dynamicsInfo = status->m_dynamicsInfo;
	Assert(status->m_type == CMD_GET_DYNAMICS_INFO_COMPLETED);
	if (status->m_type != CMD_GET_DYNAMICS_INFO_COMPLETED)
		return false;

	if (info)
	{
		*info = dynamicsInfo;
		return true;
	}
	return false;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitChangeDynamicsInfo(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3InitChangeDynamicsInfo2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitChangeDynamicsInfo2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_CHANGE_DYNAMICS_INFO;
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = -1;
	command->m_changeDynamicsInfoArgs.m_linkIndex = -2;
	command->m_updateFlags = 0;
	return commandHandle;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetMass(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double mass)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	drx3DAssert(mass >= 0);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_mass = mass;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_MASS;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetAnisotropicFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, const double anisotropicFriction[])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_anisotropicFriction[0] = anisotropicFriction[0];
	command->m_changeDynamicsInfoArgs.m_anisotropicFriction[1] = anisotropicFriction[1];
	command->m_changeDynamicsInfoArgs.m_anisotropicFriction[2] = anisotropicFriction[2];

	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_ANISOTROPIC_FRICTION;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetJointLimit(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double jointLowerLimit, double jointUpperLimit)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_jointLowerLimit = jointLowerLimit;
	command->m_changeDynamicsInfoArgs.m_jointUpperLimit = jointUpperLimit;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_JOINT_LIMITS;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetJointLimitForce(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double jointLimitForce)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_jointLimitForce = jointLimitForce;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_JOINT_LIMIT_MAX_FORCE;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetSleepThreshold(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double sleepThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_sleepThreshold = sleepThreshold;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_SLEEP_THRESHOLD;
	return 0;
}



DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetDynamicType(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, i32 dynamicType)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	drx3DAssert(dynamicType == eDynamic || dynamicType == eStatic || dynamicType == eKinematic);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_dynamicType = dynamicType;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_DYNAMIC_TYPE;
	return 0;
}




DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetLocalInertiaDiagonal(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, const double localInertiaDiagonal[])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_localInertiaDiagonal[0] = localInertiaDiagonal[0];
	command->m_changeDynamicsInfoArgs.m_localInertiaDiagonal[1] = localInertiaDiagonal[1];
	command->m_changeDynamicsInfoArgs.m_localInertiaDiagonal[2] = localInertiaDiagonal[2];

	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_LOCAL_INERTIA_DIAGONAL;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetLateralFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double lateralFriction)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_lateralFriction = lateralFriction;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_LATERAL_FRICTION;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetSpinningFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double friction)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_spinningFriction = friction;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_SPINNING_FRICTION;
	return 0;
}
DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetRollingFriction(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double friction)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_rollingFriction = friction;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_ROLLING_FRICTION;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetRestitution(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double restitution)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_restitution = restitution;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_RESTITUTION;
	return 0;
}
DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetLinearDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double linearDamping)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = -1;
	command->m_changeDynamicsInfoArgs.m_linearDamping = linearDamping;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_LINEAR_DAMPING;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetAngularDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double angularDamping)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = -1;
	command->m_changeDynamicsInfoArgs.m_angularDamping = angularDamping;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_ANGULAR_DAMPING;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetJointDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double jointDamping)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_jointDamping = jointDamping;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_JOINT_DAMPING;
	return 0;
}


DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetContactStiffnessAndDamping(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double contactStiffness, double contactDamping)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_contactStiffness = contactStiffness;
	command->m_changeDynamicsInfoArgs.m_contactDamping = contactDamping;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_CONTACT_STIFFNESS_AND_DAMPING;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetFrictionAnchor(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, i32 frictionAnchor)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_frictionAnchor = frictionAnchor;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_FRICTION_ANCHOR;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetCcdSweptSphereRadius(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double ccdSweptSphereRadius)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_ccdSweptSphereRadius = ccdSweptSphereRadius;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_CCD_SWEPT_SPHERE_RADIUS;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetContactProcessingThreshold(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkIndex, double contactProcessingThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = linkIndex;
	command->m_changeDynamicsInfoArgs.m_contactProcessingThreshold = contactProcessingThreshold;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_CONTACT_PROCESSING_THRESHOLD;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetMaxJointVelocity(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double maxJointVelocity)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = -1;
	command->m_changeDynamicsInfoArgs.m_maxJointVelocity = maxJointVelocity;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_MAX_JOINT_VELOCITY;
	return 0;
}

DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetCollisionMargin(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, double collisionMargin)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = -1;
	command->m_changeDynamicsInfoArgs.m_collisionMargin = collisionMargin;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_COLLISION_MARGIN;
	return 0;
}





DRX3D_SHARED_API i32 b3ChangeDynamicsInfoSetActivationState(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 activationState)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command->m_type == CMD_CHANGE_DYNAMICS_INFO);
	command->m_changeDynamicsInfoArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_changeDynamicsInfoArgs.m_linkIndex = -1;
	command->m_changeDynamicsInfoArgs.m_activationState = activationState;
	command->m_updateFlags |= CHANGE_DYNAMICS_INFO_SET_ACTIVATION_STATE;
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitCreateSoftBodyAnchorConstraintCommand(b3PhysicsClientHandle physClient, i32 softBodyUniqueId, i32 nodeIndex, i32 bodyUniqueId, i32 linkIndex, const double bodyFramePosition[3])
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_CONSTRAINT;
	command->m_updateFlags = USER_CONSTRAINT_ADD_SOFT_BODY_ANCHOR;
	command->m_userConstraintArguments.m_parentBodyIndex = softBodyUniqueId;
	command->m_userConstraintArguments.m_parentJointIndex = nodeIndex;
	command->m_userConstraintArguments.m_childBodyIndex = bodyUniqueId;
	command->m_userConstraintArguments.m_childJointIndex = linkIndex;
	command->m_userConstraintArguments.m_childFrame[0] = bodyFramePosition[0];
	command->m_userConstraintArguments.m_childFrame[1] = bodyFramePosition[1];
	command->m_userConstraintArguments.m_childFrame[2] = bodyFramePosition[2];
	command->m_userConstraintArguments.m_childFrame[3] = 0.;
	command->m_userConstraintArguments.m_childFrame[4] = 0.;
	command->m_userConstraintArguments.m_childFrame[5] = 0.;
	command->m_userConstraintArguments.m_childFrame[6] = 1.;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitCreateUserConstraintCommand(b3PhysicsClientHandle physClient, i32 parentBodyUniqueId, i32 parentJointIndex, i32 childBodyUniqueId, i32 childJointIndex, struct b3JointInfo* info)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3InitCreateUserConstraintCommand2((b3SharedMemoryCommandHandle)command, parentBodyUniqueId, parentJointIndex, childBodyUniqueId, childJointIndex, info);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitCreateUserConstraintCommand2(b3SharedMemoryCommandHandle commandHandle, i32 parentBodyUniqueId, i32 parentJointIndex, i32 childBodyUniqueId, i32 childJointIndex, struct b3JointInfo* info)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_USER_CONSTRAINT;
	command->m_updateFlags = USER_CONSTRAINT_ADD_CONSTRAINT;

	command->m_userConstraintArguments.m_parentBodyIndex = parentBodyUniqueId;
	command->m_userConstraintArguments.m_parentJointIndex = parentJointIndex;
	command->m_userConstraintArguments.m_childBodyIndex = childBodyUniqueId;
	command->m_userConstraintArguments.m_childJointIndex = childJointIndex;
	for (i32 i = 0; i < 7; ++i)
	{
		command->m_userConstraintArguments.m_parentFrame[i] = info->m_parentFrame[i];
		command->m_userConstraintArguments.m_childFrame[i] = info->m_childFrame[i];
	}
	for (i32 i = 0; i < 3; ++i)
	{
		command->m_userConstraintArguments.m_jointAxis[i] = info->m_jointAxis[i];
	}
	command->m_userConstraintArguments.m_jointType = info->m_jointType;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitChangeUserConstraintCommand(b3PhysicsClientHandle physClient, i32 userConstraintUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_CONSTRAINT;
	command->m_updateFlags = USER_CONSTRAINT_CHANGE_CONSTRAINT;
	command->m_userConstraintArguments.m_userConstraintUniqueId = userConstraintUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetPivotInB(b3SharedMemoryCommandHandle commandHandle, const double jointChildPivot[])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);

	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);

	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_PIVOT_IN_B;

	command->m_userConstraintArguments.m_childFrame[0] = jointChildPivot[0];
	command->m_userConstraintArguments.m_childFrame[1] = jointChildPivot[1];
	command->m_userConstraintArguments.m_childFrame[2] = jointChildPivot[2];
	return 0;
}
DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetFrameInB(b3SharedMemoryCommandHandle commandHandle, const double jointChildFrameOrn[])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);
	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);

	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_FRAME_ORN_IN_B;

	command->m_userConstraintArguments.m_childFrame[3] = jointChildFrameOrn[0];
	command->m_userConstraintArguments.m_childFrame[4] = jointChildFrameOrn[1];
	command->m_userConstraintArguments.m_childFrame[5] = jointChildFrameOrn[2];
	command->m_userConstraintArguments.m_childFrame[6] = jointChildFrameOrn[3];

	return 0;
}

DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetMaxForce(b3SharedMemoryCommandHandle commandHandle, double maxAppliedForce)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);
	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);

	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_MAX_FORCE;
	command->m_userConstraintArguments.m_maxAppliedForce = maxAppliedForce;

	return 0;
}
DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetGearRatio(b3SharedMemoryCommandHandle commandHandle, double gearRatio)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);
	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);

	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_GEAR_RATIO;
	command->m_userConstraintArguments.m_gearRatio = gearRatio;

	return 0;
}

DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetGearAuxLink(b3SharedMemoryCommandHandle commandHandle, i32 gearAuxLink)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);
	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);

	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_GEAR_AUX_LINK;
	command->m_userConstraintArguments.m_gearAuxLink = gearAuxLink;

	return 0;
}

DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetRelativePositionTarget(b3SharedMemoryCommandHandle commandHandle, double relativePositionTarget)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);
	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);
	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_RELATIVE_POSITION_TARGET;
	command->m_userConstraintArguments.m_relativePositionTarget = relativePositionTarget;

	return 0;
}
DRX3D_SHARED_API i32 b3InitChangeUserConstraintSetERP(b3SharedMemoryCommandHandle commandHandle, double erp)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_CONSTRAINT);
	drx3DAssert(command->m_updateFlags & USER_CONSTRAINT_CHANGE_CONSTRAINT);

	command->m_updateFlags |= USER_CONSTRAINT_CHANGE_ERP;
	command->m_userConstraintArguments.m_erp = erp;

	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitGetUserConstraintStateCommand(b3PhysicsClientHandle physClient, i32 constraintUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_USER_CONSTRAINT;
	command->m_updateFlags = USER_CONSTRAINT_REQUEST_STATE;
	command->m_userConstraintArguments.m_userConstraintUniqueId = constraintUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusUserConstraintState(b3SharedMemoryStatusHandle statusHandle, struct b3UserConstraintState* constraintState)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status)
	{
		Assert(status->m_type == CMD_USER_CONSTRAINT_REQUEST_STATE_COMPLETED);
		if (status && status->m_type == CMD_USER_CONSTRAINT_REQUEST_STATE_COMPLETED)
		{
			i32 i = 0;
			constraintState->m_numDofs = status->m_userConstraintStateResultArgs.m_numDofs;
			for (i = 0; i < constraintState->m_numDofs; i++)
			{
				constraintState->m_appliedConstraintForces[i] = status->m_userConstraintStateResultArgs.m_appliedConstraintForces[i];
			}
			for (; i < 6; i++)
			{
				constraintState->m_appliedConstraintForces[i] = 0;
			}
			return 1;
		}
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveUserConstraintCommand(b3PhysicsClientHandle physClient, i32 userConstraintUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_USER_CONSTRAINT;
	command->m_updateFlags = USER_CONSTRAINT_REMOVE_CONSTRAINT;
	command->m_userConstraintArguments.m_userConstraintUniqueId = userConstraintUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveBodyCommand(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REMOVE_BODY;
	command->m_updateFlags = BODY_DELETE_FLAG;
	command->m_removeObjectArgs.m_numBodies = 1;
	command->m_removeObjectArgs.m_bodyUniqueIds[0] = bodyUniqueId;
	command->m_removeObjectArgs.m_numUserCollisionShapes = 0;
	command->m_removeObjectArgs.m_numUserConstraints = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveCollisionShapeCommand(b3PhysicsClientHandle physClient, i32 collisionShapeId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REMOVE_BODY;
	command->m_updateFlags = 0;
	command->m_removeObjectArgs.m_numBodies = 0;
	command->m_removeObjectArgs.m_numUserConstraints = 0;
	command->m_removeObjectArgs.m_numUserCollisionShapes = 1;
	command->m_removeObjectArgs.m_userCollisionShapes[0] = collisionShapeId;
	return (b3SharedMemoryCommandHandle)command;
}
DRX3D_SHARED_API i32 b3GetStatusUserConstraintUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	if (status)
	{
		drx3DAssert(status->m_type == CMD_USER_CONSTRAINT_COMPLETED);
		if (status && status->m_type == CMD_USER_CONSTRAINT_COMPLETED)
		{
			return status->m_userConstraintResultArgs.m_userConstraintUniqueId;
		}
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3PickBody(b3PhysicsClientHandle physClient, double rayFromWorldX,
													 double rayFromWorldY, double rayFromWorldZ,
													 double rayToWorldX, double rayToWorldY, double rayToWorldZ)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_PICK_BODY;
	command->m_pickBodyArguments.m_rayFromWorld[0] = rayFromWorldX;
	command->m_pickBodyArguments.m_rayFromWorld[1] = rayFromWorldY;
	command->m_pickBodyArguments.m_rayFromWorld[2] = rayFromWorldZ;
	command->m_pickBodyArguments.m_rayToWorld[0] = rayToWorldX;
	command->m_pickBodyArguments.m_rayToWorld[1] = rayToWorldY;
	command->m_pickBodyArguments.m_rayToWorld[2] = rayToWorldZ;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3MovePickedBody(b3PhysicsClientHandle physClient, double rayFromWorldX,
														   double rayFromWorldY, double rayFromWorldZ,
														   double rayToWorldX, double rayToWorldY,
														   double rayToWorldZ)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_MOVE_PICKED_BODY;
	command->m_pickBodyArguments.m_rayFromWorld[0] = rayFromWorldX;
	command->m_pickBodyArguments.m_rayFromWorld[1] = rayFromWorldY;
	command->m_pickBodyArguments.m_rayFromWorld[2] = rayFromWorldZ;
	command->m_pickBodyArguments.m_rayToWorld[0] = rayToWorldX;
	command->m_pickBodyArguments.m_rayToWorld[1] = rayToWorldY;
	command->m_pickBodyArguments.m_rayToWorld[2] = rayToWorldZ;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RemovePickingConstraint(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REMOVE_PICKING_CONSTRAINT_BODY;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateRaycastCommandInit(b3PhysicsClientHandle physClient, double rayFromWorldX,
																	 double rayFromWorldY, double rayFromWorldZ,
																	 double rayToWorldX, double rayToWorldY, double rayToWorldZ)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_RAY_CAST_INTERSECTIONS;
	command->m_requestRaycastIntersections.m_numCommandRays = 1;
	command->m_requestRaycastIntersections.m_numStreamingRays = 0;
	command->m_requestRaycastIntersections.m_numThreads = 1;
	command->m_requestRaycastIntersections.m_parentObjectUniqueId = -1;
	command->m_requestRaycastIntersections.m_parentLinkIndex=-1;

	command->m_requestRaycastIntersections.m_numCommandRays = 1;
	command->m_requestRaycastIntersections.m_fromToRays[0].m_rayFromPosition[0] = rayFromWorldX;
	command->m_requestRaycastIntersections.m_fromToRays[0].m_rayFromPosition[1] = rayFromWorldY;
	command->m_requestRaycastIntersections.m_fromToRays[0].m_rayFromPosition[2] = rayFromWorldZ;

	command->m_requestRaycastIntersections.m_fromToRays[0].m_rayToPosition[0] = rayToWorldX;
	command->m_requestRaycastIntersections.m_fromToRays[0].m_rayToPosition[1] = rayToWorldY;
	command->m_requestRaycastIntersections.m_fromToRays[0].m_rayToPosition[2] = rayToWorldZ;
	command->m_requestRaycastIntersections.m_reportHitNumber = -1;
	command->m_requestRaycastIntersections.m_collisionFilterMask = -1;
	command->m_requestRaycastIntersections.m_fractionEpsilon = D3_EPSILON;
	
	

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateRaycastBatchCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_RAY_CAST_INTERSECTIONS;
	command->m_updateFlags = 0;
	command->m_requestRaycastIntersections.m_numCommandRays = 0;
	command->m_requestRaycastIntersections.m_numStreamingRays = 0;
	command->m_requestRaycastIntersections.m_numThreads = 1;
	command->m_requestRaycastIntersections.m_parentObjectUniqueId = -1;
	command->m_requestRaycastIntersections.m_parentLinkIndex=-1;
	command->m_requestRaycastIntersections.m_reportHitNumber = -1;
	command->m_requestRaycastIntersections.m_collisionFilterMask = -1;
	command->m_requestRaycastIntersections.m_fractionEpsilon = D3_EPSILON;
	
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3RaycastBatchSetNumThreads(b3SharedMemoryCommandHandle commandHandle, i32 numThreads)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	command->m_requestRaycastIntersections.m_numThreads = numThreads;
}

DRX3D_SHARED_API void b3RaycastBatchAddRay(b3SharedMemoryCommandHandle commandHandle, const double rayFromWorld[3], const double rayToWorld[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	if (command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS)
	{
		i32 numRays = command->m_requestRaycastIntersections.m_numCommandRays;
		if (numRays < MAX_RAY_INTERSECTION_BATCH_SIZE)
		{
			command->m_requestRaycastIntersections.m_fromToRays[numRays].m_rayFromPosition[0] = rayFromWorld[0];
			command->m_requestRaycastIntersections.m_fromToRays[numRays].m_rayFromPosition[1] = rayFromWorld[1];
			command->m_requestRaycastIntersections.m_fromToRays[numRays].m_rayFromPosition[2] = rayFromWorld[2];

			command->m_requestRaycastIntersections.m_fromToRays[numRays].m_rayToPosition[0] = rayToWorld[0];
			command->m_requestRaycastIntersections.m_fromToRays[numRays].m_rayToPosition[1] = rayToWorld[1];
			command->m_requestRaycastIntersections.m_fromToRays[numRays].m_rayToPosition[2] = rayToWorld[2];

			command->m_requestRaycastIntersections.m_numCommandRays++;
		}
	}
}

DRX3D_SHARED_API void b3RaycastBatchAddRays(b3PhysicsClientHandle physClient, b3SharedMemoryCommandHandle commandHandle, const double* rayFromWorldArray, const double* rayToWorldArray, i32 numRays)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);

	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	drx3DAssert(numRays < MAX_RAY_INTERSECTION_BATCH_SIZE_STREAMING);
	if (command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS)
	{
		cl->uploadRaysToSharedMemory(*command, rayFromWorldArray, rayToWorldArray, numRays);
	}
}

DRX3D_SHARED_API void b3RaycastBatchSetParentObject(b3SharedMemoryCommandHandle commandHandle, i32 parentObjectUniqueId, i32 parentLinkIndex)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	command->m_requestRaycastIntersections.m_parentObjectUniqueId = parentObjectUniqueId;
	command->m_requestRaycastIntersections.m_parentLinkIndex = parentLinkIndex;
}

DRX3D_SHARED_API void b3RaycastBatchSetReportHitNumber(b3SharedMemoryCommandHandle commandHandle, i32 reportHitNumber)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	command->m_requestRaycastIntersections.m_reportHitNumber= reportHitNumber;
}
DRX3D_SHARED_API void b3RaycastBatchSetCollisionFilterMask(b3SharedMemoryCommandHandle commandHandle, i32 collisionFilterMask)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	command->m_requestRaycastIntersections.m_collisionFilterMask = collisionFilterMask;
}

DRX3D_SHARED_API void b3RaycastBatchSetFractionEpsilon(b3SharedMemoryCommandHandle commandHandle, double fractionEpsilon)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_RAY_CAST_INTERSECTIONS);
	command->m_requestRaycastIntersections.m_fractionEpsilon = fractionEpsilon;
}


DRX3D_SHARED_API void b3GetRaycastInformation(b3PhysicsClientHandle physClient, struct b3RaycastInformation* raycastInfo)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedRaycastHits(raycastInfo);
	}
}

///If you re-connected to an existing server, or server changed otherwise, sync the body info
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitSyncBodyInfoCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_SYNC_BODY_INFO;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestBodyInfoCommand(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REQUEST_BODY_INFO;
	command->m_sdfRequestInfoArgs.m_bodyUniqueId = bodyUniqueId;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitSyncUserDataCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_SYNC_USER_DATA;
	command->m_syncUserDataRequestArgs.m_numRequestedBodies = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3AddBodyToSyncUserDataRequest(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_SYNC_USER_DATA);

	command->m_syncUserDataRequestArgs.m_requestedBodyIds[command->m_syncUserDataRequestArgs.m_numRequestedBodies++] = bodyUniqueId;
}


DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitAddUserDataCommand(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key, UserDataValueType valueType, i32 valueLength, ukk valueData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(strlen(key) < MAX_USER_DATA_KEY_LENGTH);
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_ADD_USER_DATA;
	command->m_addUserDataRequestArgs.m_bodyUniqueId = bodyUniqueId;
	command->m_addUserDataRequestArgs.m_linkIndex = linkIndex;
	command->m_addUserDataRequestArgs.m_visualShapeIndex = visualShapeIndex;
	command->m_addUserDataRequestArgs.m_valueType = valueType;
	command->m_addUserDataRequestArgs.m_valueLength = valueLength;
	strcpy(command->m_addUserDataRequestArgs.m_key, key);
	cl->uploadBulletFileToSharedMemory((tukk)valueData, valueLength);

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRemoveUserDataCommand(b3PhysicsClientHandle physClient, i32 userDataId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REMOVE_USER_DATA;
	command->m_removeUserDataRequestArgs.m_userDataId = userDataId;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetUserData(b3PhysicsClientHandle physClient, i32 userDataId, struct b3UserDataValue* valueOut)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		return cl->getCachedUserData(userDataId, *valueOut);
	}
	return false;
}

DRX3D_SHARED_API i32 b3GetUserDataId(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex, i32 visualShapeIndex, tukk key)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		return cl->getCachedUserDataId(bodyUniqueId, linkIndex, visualShapeIndex, key);
	}
	return -1;
}

DRX3D_SHARED_API i32 b3GetUserDataIdFromStatus(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status)
	{
		Assert(status->m_type == CMD_ADD_USER_DATA_COMPLETED);
		return status->m_userDataResponseArgs.m_userDataId;
	}
	return -1;
}

DRX3D_SHARED_API i32 b3GetNumUserData(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		return cl->getNumUserData(bodyUniqueId);
	}
	return 0;
}

DRX3D_SHARED_API void b3GetUserDataInfo(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 userDataIndex, tukk* keyOut, i32* userDataIdOut, i32* linkIndexOut, i32* visualShapeIndexOut)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getUserDataInfo(bodyUniqueId, userDataIndex, keyOut, userDataIdOut, linkIndexOut, visualShapeIndexOut);
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestDebugLinesCommand(b3PhysicsClientHandle physClient, i32 debugMode)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REQUEST_DEBUG_LINES;
	command->m_requestDebugLinesArguments.m_debugMode = debugMode;
	command->m_requestDebugLinesArguments.m_startingLineIndex = 0;
	return (b3SharedMemoryCommandHandle)command;
}
DRX3D_SHARED_API void b3GetDebugLines(b3PhysicsClientHandle physClient, struct b3DebugLines* lines)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;

	drx3DAssert(lines);
	if (lines)
	{
		lines->m_numDebugLines = cl->getNumDebugLines();
		lines->m_linesFrom = cl->getDebugLinesFrom();
		lines->m_linesTo = cl->getDebugLinesTo();
		lines->m_linesColor = cl->getDebugLinesColor();
	}
}

/// Add/remove user-specific debug lines and debug text messages
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawAddLine3D(b3PhysicsClientHandle physClient, const double fromXYZ[3], const double toXYZ[3], const double colorRGB[3], const double lineWidth, const double lifeTime)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_HAS_LINE;  //USER_DEBUG_HAS_TEXT

	command->m_userDebugDrawArgs.m_debugLineFromXYZ[0] = fromXYZ[0];
	command->m_userDebugDrawArgs.m_debugLineFromXYZ[1] = fromXYZ[1];
	command->m_userDebugDrawArgs.m_debugLineFromXYZ[2] = fromXYZ[2];

	command->m_userDebugDrawArgs.m_debugLineToXYZ[0] = toXYZ[0];
	command->m_userDebugDrawArgs.m_debugLineToXYZ[1] = toXYZ[1];
	command->m_userDebugDrawArgs.m_debugLineToXYZ[2] = toXYZ[2];

	command->m_userDebugDrawArgs.m_debugLineColorRGB[0] = colorRGB[0];
	command->m_userDebugDrawArgs.m_debugLineColorRGB[1] = colorRGB[1];
	command->m_userDebugDrawArgs.m_debugLineColorRGB[2] = colorRGB[2];

	command->m_userDebugDrawArgs.m_lineWidth = lineWidth;
	command->m_userDebugDrawArgs.m_lifeTime = lifeTime;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	command->m_userDebugDrawArgs.m_parentLinkIndex = -1;
	command->m_userDebugDrawArgs.m_optionFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawAddPoints3D(b3PhysicsClientHandle physClient, const double positionsXYZ[/*3n*/], const double colorsRGB[/*3n*/], const double pointSize, const double lifeTime, i32k pointNum)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_HAS_POINTS;

	command->m_userDebugDrawArgs.m_debugPointNum = pointNum;
	command->m_userDebugDrawArgs.m_pointSize = pointSize;
	command->m_userDebugDrawArgs.m_lifeTime = lifeTime;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	command->m_userDebugDrawArgs.m_parentLinkIndex = -1;
	command->m_userDebugDrawArgs.m_optionFlags = 0;

	i32 totalUploadSizeInBytes = pointNum * sizeof(double) * 3 * 2;
	tuk data = new char[totalUploadSizeInBytes];
	double* pointPositionsUpload = (double*) data;
	double* pointColorsUpload = (double*)(data + pointNum * sizeof(double) * 3);
	for (i32 i = 0; i < pointNum; i++)
	{
		pointPositionsUpload[i * 3 + 0] = positionsXYZ[i * 3 + 0];
		pointPositionsUpload[i * 3 + 1] = positionsXYZ[i * 3 + 1];
		pointPositionsUpload[i * 3 + 2] = positionsXYZ[i * 3 + 2];
	}
	for (i32 i = 0; i < pointNum; i++)
	{
		pointColorsUpload[i * 3 + 0] = colorsRGB[i * 3 + 0];
		pointColorsUpload[i * 3 + 1] = colorsRGB[i * 3 + 1];
		pointColorsUpload[i * 3 + 2] = colorsRGB[i * 3 + 2];
	}
	cl->uploadBulletFileToSharedMemory(data, totalUploadSizeInBytes);
	delete[] data;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawAddText3D(b3PhysicsClientHandle physClient, tukk txt, const double positionXYZ[3], const double colorRGB[3], double textSize, double lifeTime)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_HAS_TEXT;

	i32 len = strlen(txt);
	if (len < MAX_FILENAME_LENGTH)
	{
		strcpy(command->m_userDebugDrawArgs.m_text, txt);
	}
	else
	{
		command->m_userDebugDrawArgs.m_text[0] = 0;
	}
	command->m_userDebugDrawArgs.m_textPositionXYZ[0] = positionXYZ[0];
	command->m_userDebugDrawArgs.m_textPositionXYZ[1] = positionXYZ[1];
	command->m_userDebugDrawArgs.m_textPositionXYZ[2] = positionXYZ[2];

	command->m_userDebugDrawArgs.m_textColorRGB[0] = colorRGB[0];
	command->m_userDebugDrawArgs.m_textColorRGB[1] = colorRGB[1];
	command->m_userDebugDrawArgs.m_textColorRGB[2] = colorRGB[2];

	command->m_userDebugDrawArgs.m_textSize = textSize;

	command->m_userDebugDrawArgs.m_lifeTime = lifeTime;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	command->m_userDebugDrawArgs.m_parentLinkIndex = -1;

	command->m_userDebugDrawArgs.m_optionFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3UserDebugTextSetOptionFlags(b3SharedMemoryCommandHandle commandHandle, i32 optionFlags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_DEBUG_DRAW);
	drx3DAssert(command->m_updateFlags & USER_DEBUG_HAS_TEXT);
	command->m_userDebugDrawArgs.m_optionFlags = optionFlags;
	command->m_updateFlags |= USER_DEBUG_HAS_OPTION_FLAGS;
}

DRX3D_SHARED_API void b3UserDebugTextSetOrientation(b3SharedMemoryCommandHandle commandHandle, const double orientation[4])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_DEBUG_DRAW);
	drx3DAssert(command->m_updateFlags & USER_DEBUG_HAS_TEXT);
	command->m_userDebugDrawArgs.m_textOrientation[0] = orientation[0];
	command->m_userDebugDrawArgs.m_textOrientation[1] = orientation[1];
	command->m_userDebugDrawArgs.m_textOrientation[2] = orientation[2];
	command->m_userDebugDrawArgs.m_textOrientation[3] = orientation[3];
	command->m_updateFlags |= USER_DEBUG_HAS_TEXT_ORIENTATION;
}

DRX3D_SHARED_API void b3UserDebugItemSetReplaceItemUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 replaceItemUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_DEBUG_DRAW);
	command->m_userDebugDrawArgs.m_replaceItemUniqueId = replaceItemUniqueId;
	command->m_updateFlags |= USER_DEBUG_HAS_REPLACE_ITEM_UNIQUE_ID;
}

DRX3D_SHARED_API void b3UserDebugItemSetParentObject(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId, i32 linkIndex)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_DEBUG_DRAW);

	command->m_updateFlags |= USER_DEBUG_HAS_PARENT_OBJECT;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = objectUniqueId;
	command->m_userDebugDrawArgs.m_parentLinkIndex = linkIndex;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugAddParameter(b3PhysicsClientHandle physClient, tukk txt, double rangeMin, double rangeMax, double startValue)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_ADD_PARAMETER;
	i32 len = strlen(txt);
	if (len < MAX_FILENAME_LENGTH)
	{
		strcpy(command->m_userDebugDrawArgs.m_text, txt);
	}
	else
	{
		command->m_userDebugDrawArgs.m_text[0] = 0;
	}
	
	command->m_userDebugDrawArgs.m_rangeMin = rangeMin;
	command->m_userDebugDrawArgs.m_rangeMax = rangeMax;
	command->m_userDebugDrawArgs.m_startValue = startValue;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	command->m_userDebugDrawArgs.m_optionFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugReadParameter(b3PhysicsClientHandle physClient, i32 debugItemUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_READ_PARAMETER;
	command->m_userDebugDrawArgs.m_itemUniqueId = debugItemUniqueId;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusDebugParameterValue(b3SharedMemoryStatusHandle statusHandle, double* paramValue)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status)
	{
		Assert(status->m_type == CMD_USER_DEBUG_DRAW_PARAMETER_COMPLETED);
		if (paramValue && (status->m_type == CMD_USER_DEBUG_DRAW_PARAMETER_COMPLETED))
		{
			*paramValue = status->m_userDebugDrawArgs.m_parameterValue;
			return 1;
		}
	}
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawRemove(b3PhysicsClientHandle physClient, i32 debugItemUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_REMOVE_ONE_ITEM;
	command->m_userDebugDrawArgs.m_itemUniqueId = debugItemUniqueId;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserDebugDrawRemoveAll(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_REMOVE_ALL;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUserRemoveAllParameters(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = USER_DEBUG_REMOVE_ALL_PARAMETERS;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	return (b3SharedMemoryCommandHandle)command;
}


DRX3D_SHARED_API i32 b3GetDebugItemUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	Assert(status->m_type == CMD_USER_DEBUG_DRAW_COMPLETED);
	if (status->m_type != CMD_USER_DEBUG_DRAW_COMPLETED)
		return -1;

	return status->m_userDebugDrawArgs.m_debugItemUniqueId;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitDebugDrawingCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_USER_DEBUG_DRAW;
	command->m_updateFlags = 0;
	command->m_userDebugDrawArgs.m_parentObjectUniqueId = -1;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3SetDebugObjectColor(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId, i32 linkIndex, const double objectColorRGB[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_DEBUG_DRAW);
	command->m_updateFlags |= USER_DEBUG_SET_CUSTOM_OBJECT_COLOR;
	command->m_userDebugDrawArgs.m_objectUniqueId = objectUniqueId;
	command->m_userDebugDrawArgs.m_linkIndex = linkIndex;
	command->m_userDebugDrawArgs.m_objectDebugColorRGB[0] = objectColorRGB[0];
	command->m_userDebugDrawArgs.m_objectDebugColorRGB[1] = objectColorRGB[1];
	command->m_userDebugDrawArgs.m_objectDebugColorRGB[2] = objectColorRGB[2];
}

DRX3D_SHARED_API void b3RemoveDebugObjectColor(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId, i32 linkIndex)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_USER_DEBUG_DRAW);
	command->m_updateFlags |= USER_DEBUG_REMOVE_CUSTOM_OBJECT_COLOR;
	command->m_userDebugDrawArgs.m_objectUniqueId = objectUniqueId;
	command->m_userDebugDrawArgs.m_linkIndex = linkIndex;
}

///request an image from a simulated camera, using a software renderer.
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestCameraImage(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3InitRequestCameraImage2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestCameraImage2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_REQUEST_CAMERA_IMAGE_DATA;
	command->m_requestPixelDataArguments.m_startPixelIndex = 0;
	command->m_updateFlags = 0;  //REQUEST_PIXEL_ARGS_USE_HARDWARE_OPENGL;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3RequestCameraImageSelectRenderer(b3SharedMemoryCommandHandle commandHandle, i32 renderer)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	drx3DAssert(renderer > (1 << 15));
	if (renderer > (1 << 15))
	{
		command->m_updateFlags |= renderer;
	}
}

DRX3D_SHARED_API void b3RequestCameraImageSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	if (command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA)
	{
		command->m_requestPixelDataArguments.m_flags = flags;
		command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_FLAGS;
	}
}

DRX3D_SHARED_API void b3RequestCameraImageSetCameraMatrices(b3SharedMemoryCommandHandle commandHandle, float viewMatrix[16], float projectionMatrix[16])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	for (i32 i = 0; i < 16; i++)
	{
		command->m_requestPixelDataArguments.m_projectionMatrix[i] = projectionMatrix[i];
		command->m_requestPixelDataArguments.m_viewMatrix[i] = viewMatrix[i];
	}
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES;
}

DRX3D_SHARED_API void b3RequestCameraImageSetLightDirection(b3SharedMemoryCommandHandle commandHandle, const float lightDirection[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	for (i32 i = 0; i < 3; i++)
	{
		command->m_requestPixelDataArguments.m_lightDirection[i] = lightDirection[i];
	}
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_LIGHT_DIRECTION;
}

DRX3D_SHARED_API void b3RequestCameraImageSetLightColor(b3SharedMemoryCommandHandle commandHandle, const float lightColor[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	for (i32 i = 0; i < 3; i++)
	{
		command->m_requestPixelDataArguments.m_lightColor[i] = lightColor[i];
	}
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_LIGHT_COLOR;
}

DRX3D_SHARED_API void b3RequestCameraImageSetLightDistance(b3SharedMemoryCommandHandle commandHandle, float lightDistance)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	command->m_requestPixelDataArguments.m_lightDistance = lightDistance;
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_LIGHT_DISTANCE;
}

DRX3D_SHARED_API void b3RequestCameraImageSetLightAmbientCoeff(b3SharedMemoryCommandHandle commandHandle, float lightAmbientCoeff)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	command->m_requestPixelDataArguments.m_lightAmbientCoeff = lightAmbientCoeff;
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_AMBIENT_COEFF;
}

DRX3D_SHARED_API void b3RequestCameraImageSetLightDiffuseCoeff(b3SharedMemoryCommandHandle commandHandle, float lightDiffuseCoeff)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	command->m_requestPixelDataArguments.m_lightDiffuseCoeff = lightDiffuseCoeff;
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_DIFFUSE_COEFF;
}

DRX3D_SHARED_API void b3RequestCameraImageSetLightSpecularCoeff(b3SharedMemoryCommandHandle commandHandle, float lightSpecularCoeff)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	command->m_requestPixelDataArguments.m_lightSpecularCoeff = lightSpecularCoeff;
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_SPECULAR_COEFF;
}

DRX3D_SHARED_API void b3RequestCameraImageSetShadow(b3SharedMemoryCommandHandle commandHandle, i32 hasShadow)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	command->m_requestPixelDataArguments.m_hasShadow = hasShadow;
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_SHADOW;
}

DRX3D_SHARED_API void b3RequestCameraImageSetProjectiveTextureMatrices(b3SharedMemoryCommandHandle commandHandle, float viewMatrix[16], float projectionMatrix[16])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	for (i32 i = 0; i < 16; i++)
	{
		command->m_requestPixelDataArguments.m_projectiveTextureProjectionMatrix[i] = projectionMatrix[i];
		command->m_requestPixelDataArguments.m_projectiveTextureViewMatrix[i] = viewMatrix[i];
	}
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_PROJECTIVE_TEXTURE_MATRICES;
}

DRX3D_SHARED_API void b3ComputePositionFromViewMatrix(const float viewMatrix[16], float cameraPosition[3], float cameraTargetPosition[3], float cameraUp[3])
{
	b3Matrix3x3 r(viewMatrix[0], viewMatrix[4], viewMatrix[8], viewMatrix[1], viewMatrix[5], viewMatrix[9], viewMatrix[2], viewMatrix[6], viewMatrix[10]);
	b3Vec3 p = b3MakeVector3(viewMatrix[12], viewMatrix[13], viewMatrix[14]);
	b3Transform t(r, p);
	b3Transform tinv = t.inverse();
	b3Matrix3x3 basis = tinv.getBasis();
	b3Vec3 origin = tinv.getOrigin();
	b3Vec3 s = b3MakeVector3(basis[0][0], basis[1][0], basis[2][0]);
	b3Vec3 u = b3MakeVector3(basis[0][1], basis[1][1], basis[2][1]);
	b3Vec3 f = b3MakeVector3(-basis[0][2], -basis[1][2], -basis[2][2]);
	b3Vec3 eye = origin;
	cameraPosition[0] = eye[0];
	cameraPosition[1] = eye[1];
	cameraPosition[2] = eye[2];
	b3Vec3 center = f + eye;
	cameraTargetPosition[0] = center[0];
	cameraTargetPosition[1] = center[1];
	cameraTargetPosition[2] = center[2];
	cameraUp[0] = u[0];
	cameraUp[1] = u[1];
	cameraUp[2] = u[2];
}

DRX3D_SHARED_API void b3ComputeViewMatrixFromPositions(const float cameraPosition[3], const float cameraTargetPosition[3], const float cameraUp[3], float viewMatrix[16])
{
	b3Vec3 eye = b3MakeVector3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	b3Vec3 center = b3MakeVector3(cameraTargetPosition[0], cameraTargetPosition[1], cameraTargetPosition[2]);
	b3Vec3 up = b3MakeVector3(cameraUp[0], cameraUp[1], cameraUp[2]);
	b3Vec3 f = (center - eye).normalized();
	b3Vec3 u = up.normalized();
	b3Vec3 s = (f.cross(u)).normalized();
	u = s.cross(f);

	viewMatrix[0 * 4 + 0] = s.x;
	viewMatrix[1 * 4 + 0] = s.y;
	viewMatrix[2 * 4 + 0] = s.z;

	viewMatrix[0 * 4 + 1] = u.x;
	viewMatrix[1 * 4 + 1] = u.y;
	viewMatrix[2 * 4 + 1] = u.z;

	viewMatrix[0 * 4 + 2] = -f.x;
	viewMatrix[1 * 4 + 2] = -f.y;
	viewMatrix[2 * 4 + 2] = -f.z;

	viewMatrix[0 * 4 + 3] = 0.f;
	viewMatrix[1 * 4 + 3] = 0.f;
	viewMatrix[2 * 4 + 3] = 0.f;

	viewMatrix[3 * 4 + 0] = -s.dot(eye);
	viewMatrix[3 * 4 + 1] = -u.dot(eye);
	viewMatrix[3 * 4 + 2] = f.dot(eye);
	viewMatrix[3 * 4 + 3] = 1.f;
}

DRX3D_SHARED_API void b3ComputeViewMatrixFromYawPitchRoll(const float cameraTargetPosition[3], float distance, float yaw, float pitch, float roll, i32 upAxis, float viewMatrix[16])
{
	b3Vec3 camUpVector;
	b3Vec3 camForward;
	b3Vec3 camPos;
	b3Vec3 camTargetPos = b3MakeVector3(cameraTargetPosition[0], cameraTargetPosition[1], cameraTargetPosition[2]);
	b3Vec3 eyePos = b3MakeVector3(0, 0, 0);

	b3Scalar yawRad = yaw * b3Scalar(0.01745329251994329547);      // rads per deg
	b3Scalar pitchRad = pitch * b3Scalar(0.01745329251994329547);  // rads per deg
	b3Scalar rollRad = roll * b3Scalar(0.01745329251994329547);      // rads per deg
	b3Quat eyeRot;

	i32 forwardAxis(-1);
	switch (upAxis)
	{
		case 1:
			forwardAxis = 2;
			camUpVector = b3MakeVector3(0, 1, 0);
			eyeRot.setEulerZYX(rollRad, yawRad, -pitchRad);
			break;
		case 2:
			forwardAxis = 1;
			camUpVector = b3MakeVector3(0, 0, 1);
			eyeRot.setEulerZYX(yawRad, rollRad, pitchRad);
			break;
		default:
			return;
	};

	eyePos[forwardAxis] = -distance;

	camForward = b3MakeVector3(eyePos[0], eyePos[1], eyePos[2]);
	if (camForward.length2() < D3_EPSILON)
	{
		camForward.setVal(1.f, 0.f, 0.f);
	}
	else
	{
		camForward.normalize();
	}

	eyePos = b3Matrix3x3(eyeRot) * eyePos;
	camUpVector = b3Matrix3x3(eyeRot) * camUpVector;

	camPos = eyePos;
	camPos += camTargetPos;

	float camPosf[4] = {camPos[0], camPos[1], camPos[2], 0};
	float camPosTargetf[4] = {camTargetPos[0], camTargetPos[1], camTargetPos[2], 0};
	float camUpf[4] = {camUpVector[0], camUpVector[1], camUpVector[2], 0};

	b3ComputeViewMatrixFromPositions(camPosf, camPosTargetf, camUpf, viewMatrix);
}

DRX3D_SHARED_API void b3ComputeProjectionMatrix(float left, float right, float bottom, float top, float nearVal, float farVal, float projectionMatrix[16])
{
	projectionMatrix[0 * 4 + 0] = (float(2) * nearVal) / (right - left);
	projectionMatrix[0 * 4 + 1] = float(0);
	projectionMatrix[0 * 4 + 2] = float(0);
	projectionMatrix[0 * 4 + 3] = float(0);

	projectionMatrix[1 * 4 + 0] = float(0);
	projectionMatrix[1 * 4 + 1] = (float(2) * nearVal) / (top - bottom);
	projectionMatrix[1 * 4 + 2] = float(0);
	projectionMatrix[1 * 4 + 3] = float(0);

	projectionMatrix[2 * 4 + 0] = (right + left) / (right - left);
	projectionMatrix[2 * 4 + 1] = (top + bottom) / (top - bottom);
	projectionMatrix[2 * 4 + 2] = -(farVal + nearVal) / (farVal - nearVal);
	projectionMatrix[2 * 4 + 3] = float(-1);

	projectionMatrix[3 * 4 + 0] = float(0);
	projectionMatrix[3 * 4 + 1] = float(0);
	projectionMatrix[3 * 4 + 2] = -(float(2) * farVal * nearVal) / (farVal - nearVal);
	projectionMatrix[3 * 4 + 3] = float(0);
}

DRX3D_SHARED_API void b3ComputeProjectionMatrixFOV(float fov, float aspect, float nearVal, float farVal, float projectionMatrix[16])
{
	float yScale = 1.0 / tan((D3_PI / 180.0) * fov / 2);
	float xScale = yScale / aspect;

	projectionMatrix[0 * 4 + 0] = xScale;
	projectionMatrix[0 * 4 + 1] = float(0);
	projectionMatrix[0 * 4 + 2] = float(0);
	projectionMatrix[0 * 4 + 3] = float(0);

	projectionMatrix[1 * 4 + 0] = float(0);
	projectionMatrix[1 * 4 + 1] = yScale;
	projectionMatrix[1 * 4 + 2] = float(0);
	projectionMatrix[1 * 4 + 3] = float(0);

	projectionMatrix[2 * 4 + 0] = 0;
	projectionMatrix[2 * 4 + 1] = 0;
	projectionMatrix[2 * 4 + 2] = (nearVal + farVal) / (nearVal - farVal);
	projectionMatrix[2 * 4 + 3] = float(-1);

	projectionMatrix[3 * 4 + 0] = float(0);
	projectionMatrix[3 * 4 + 1] = float(0);
	projectionMatrix[3 * 4 + 2] = (float(2) * farVal * nearVal) / (nearVal - farVal);
	projectionMatrix[3 * 4 + 3] = float(0);
}

DRX3D_SHARED_API void b3RequestCameraImageSetViewMatrix2(b3SharedMemoryCommandHandle commandHandle, const float cameraTargetPosition[3], float distance, float yaw, float pitch, float roll, i32 upAxis)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);

	b3ComputeViewMatrixFromYawPitchRoll(cameraTargetPosition, distance, yaw, pitch, roll, upAxis, command->m_requestPixelDataArguments.m_viewMatrix);
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES;
}

DRX3D_SHARED_API void b3RequestCameraImageSetViewMatrix(b3SharedMemoryCommandHandle commandHandle, const float cameraPosition[3], const float cameraTargetPosition[3], const float cameraUp[3])
{
	float viewMatrix[16];
	b3ComputeViewMatrixFromPositions(cameraPosition, cameraTargetPosition, cameraUp, viewMatrix);

	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);

	b3ComputeViewMatrixFromPositions(cameraPosition, cameraTargetPosition, cameraUp, command->m_requestPixelDataArguments.m_viewMatrix);

	command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES;
}

DRX3D_SHARED_API void b3RequestCameraImageSetProjectionMatrix(b3SharedMemoryCommandHandle commandHandle, float left, float right, float bottom, float top, float nearVal, float farVal)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);

	b3ComputeProjectionMatrix(left, right, bottom, top, nearVal, farVal, command->m_requestPixelDataArguments.m_projectionMatrix);

	command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES;
}

DRX3D_SHARED_API void b3RequestCameraImageSetFOVProjectionMatrix(b3SharedMemoryCommandHandle commandHandle, float fov, float aspect, float nearVal, float farVal)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);

	b3ComputeProjectionMatrixFOV(fov, aspect, nearVal, farVal, command->m_requestPixelDataArguments.m_projectionMatrix);

	command->m_updateFlags |= REQUEST_PIXEL_ARGS_HAS_CAMERA_MATRICES;
}

DRX3D_SHARED_API void b3RequestCameraImageSetPixelResolution(b3SharedMemoryCommandHandle commandHandle, i32 width, i32 height)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CAMERA_IMAGE_DATA);
	command->m_requestPixelDataArguments.m_pixelWidth = width;
	command->m_requestPixelDataArguments.m_pixelHeight = height;
	command->m_updateFlags |= REQUEST_PIXEL_ARGS_SET_PIXEL_WIDTH_HEIGHT;
}

DRX3D_SHARED_API void b3GetCameraImageData(b3PhysicsClientHandle physClient, struct b3CameraImageData* imageData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedCameraImage(imageData);
	}
}

///request an contact point information
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestContactPointInformation(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_CONTACT_POINT_INFORMATION;
	command->m_requestContactPointArguments.m_startingContactPointIndex = 0;
	command->m_requestContactPointArguments.m_objectAIndexFilter = -1;
	command->m_requestContactPointArguments.m_objectBIndexFilter = -1;
	command->m_requestContactPointArguments.m_linkIndexAIndexFilter = -2;
	command->m_requestContactPointArguments.m_linkIndexBIndexFilter = -2;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3SetContactFilterBodyA(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_requestContactPointArguments.m_objectAIndexFilter = bodyUniqueIdA;
}

DRX3D_SHARED_API void b3SetContactFilterLinkA(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexA)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_A_FILTER;
	command->m_requestContactPointArguments.m_linkIndexAIndexFilter = linkIndexA;
}

DRX3D_SHARED_API void b3SetContactFilterLinkB(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexB)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_LINK_INDEX_B_FILTER;
	command->m_requestContactPointArguments.m_linkIndexBIndexFilter = linkIndexB;
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterLinkA(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexA)
{
	b3SetContactFilterLinkA(commandHandle, linkIndexA);
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterLinkB(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexB)
{
	b3SetContactFilterLinkB(commandHandle, linkIndexB);
}

DRX3D_SHARED_API void b3SetContactFilterBodyB(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdB)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_requestContactPointArguments.m_objectBIndexFilter = bodyUniqueIdB;
}

///compute the closest points between two bodies
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitClosestDistanceQuery(b3PhysicsClientHandle physClient)
{
	b3SharedMemoryCommandHandle commandHandle = b3InitRequestContactPointInformation(physClient);
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags = CMD_REQUEST_CONTACT_POINT_HAS_QUERY_MODE;
	command->m_requestContactPointArguments.m_mode = CONTACT_QUERY_MODE_COMPUTE_CLOSEST_POINTS;
	return commandHandle;
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterBodyA(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA)
{
	b3SetContactFilterBodyA(commandHandle, bodyUniqueIdA);
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterBodyB(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdB)
{
	b3SetContactFilterBodyB(commandHandle, bodyUniqueIdB);
}

DRX3D_SHARED_API void b3SetClosestDistanceThreshold(b3SharedMemoryCommandHandle commandHandle, double distance)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_CLOSEST_DISTANCE_THRESHOLD;
	command->m_requestContactPointArguments.m_closestDistanceThreshold = distance;
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeA(b3SharedMemoryCommandHandle commandHandle, i32 collisionShapeA)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_A;
	command->m_requestContactPointArguments.m_collisionShapeA = collisionShapeA;
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeB(b3SharedMemoryCommandHandle commandHandle, i32 collisionShapeB)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_B;
	command->m_requestContactPointArguments.m_collisionShapeB = collisionShapeB;
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapePositionA(b3SharedMemoryCommandHandle commandHandle, const double collisionShapePositionA[/*3*/])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_POSITION_A;
	command->m_requestContactPointArguments.m_collisionShapePositionA[0] = collisionShapePositionA[0];
	command->m_requestContactPointArguments.m_collisionShapePositionA[1] = collisionShapePositionA[1];
	command->m_requestContactPointArguments.m_collisionShapePositionA[2] = collisionShapePositionA[2];
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapePositionB(b3SharedMemoryCommandHandle commandHandle, const double collisionShapePositionB[/*3*/])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_POSITION_B;
	command->m_requestContactPointArguments.m_collisionShapePositionB[0] = collisionShapePositionB[0];
	command->m_requestContactPointArguments.m_collisionShapePositionB[1] = collisionShapePositionB[1];
	command->m_requestContactPointArguments.m_collisionShapePositionB[2] = collisionShapePositionB[2];
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeOrientationA(b3SharedMemoryCommandHandle commandHandle, const double collisionShapeOrientationA[/*4*/])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_ORIENTATION_A;
	command->m_requestContactPointArguments.m_collisionShapeOrientationA[0] = collisionShapeOrientationA[0];
	command->m_requestContactPointArguments.m_collisionShapeOrientationA[1] = collisionShapeOrientationA[1];
	command->m_requestContactPointArguments.m_collisionShapeOrientationA[2] = collisionShapeOrientationA[2];
	command->m_requestContactPointArguments.m_collisionShapeOrientationA[3] = collisionShapeOrientationA[3];
}

DRX3D_SHARED_API void b3SetClosestDistanceFilterCollisionShapeOrientationB(b3SharedMemoryCommandHandle commandHandle, const double collisionShapeOrientationB[/*4*/])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_REQUEST_CONTACT_POINT_INFORMATION);
	command->m_updateFlags |= CMD_REQUEST_CONTACT_POINT_HAS_COLLISION_SHAPE_ORIENTATION_B;
	command->m_requestContactPointArguments.m_collisionShapeOrientationB[0] = collisionShapeOrientationB[0];
	command->m_requestContactPointArguments.m_collisionShapeOrientationB[1] = collisionShapeOrientationB[1];
	command->m_requestContactPointArguments.m_collisionShapeOrientationB[2] = collisionShapeOrientationB[2];
	command->m_requestContactPointArguments.m_collisionShapeOrientationB[3] = collisionShapeOrientationB[3];
}

///get all the bodies that touch a given axis aligned bounding box specified in world space (min and max coordinates)
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitAABBOverlapQuery(b3PhysicsClientHandle physClient, const double aabbMin[3], const double aabbMax[3])
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_AABB_OVERLAP;
	command->m_updateFlags = 0;
	command->m_requestOverlappingObjectsArgs.m_startingOverlappingObjectIndex = 0;
	command->m_requestOverlappingObjectsArgs.m_aabbQueryMin[0] = aabbMin[0];
	command->m_requestOverlappingObjectsArgs.m_aabbQueryMin[1] = aabbMin[1];
	command->m_requestOverlappingObjectsArgs.m_aabbQueryMin[2] = aabbMin[2];

	command->m_requestOverlappingObjectsArgs.m_aabbQueryMax[0] = aabbMax[0];
	command->m_requestOverlappingObjectsArgs.m_aabbQueryMax[1] = aabbMax[1];
	command->m_requestOverlappingObjectsArgs.m_aabbQueryMax[2] = aabbMax[2];
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3GetAABBOverlapResults(b3PhysicsClientHandle physClient, struct b3AABBOverlapData* data)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedOverlappingObjects(data);
	}
}

DRX3D_SHARED_API void b3GetContactPointInformation(b3PhysicsClientHandle physClient, struct b3ContactInformation* contactPointData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedContactPointInformation(contactPointData);
	}
}

DRX3D_SHARED_API void b3GetClosestPointInformation(b3PhysicsClientHandle physClient, struct b3ContactInformation* contactPointInfo)
{
	b3GetContactPointInformation(physClient, contactPointInfo);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestCollisionShapeInformation(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_COLLISION_SHAPE_INFO;
	command->m_requestCollisionShapeDataArguments.m_bodyUniqueId = bodyUniqueId;
	command->m_requestCollisionShapeDataArguments.m_linkIndex = linkIndex;

	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}
DRX3D_SHARED_API void b3GetCollisionShapeInformation(b3PhysicsClientHandle physClient, struct b3CollisionShapeInformation* collisionShapeInfo)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedCollisionShapeInformation(collisionShapeInfo);
	}
}

//request visual shape information
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestVisualShapeInformation(b3PhysicsClientHandle physClient, i32 bodyUniqueIdA)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_REQUEST_VISUAL_SHAPE_INFO;
	command->m_requestVisualShapeDataArguments.m_bodyUniqueId = bodyUniqueIdA;
	command->m_requestVisualShapeDataArguments.m_startingVisualShapeIndex = 0;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3GetVisualShapeInformation(b3PhysicsClientHandle physClient, struct b3VisualShapeInformation* visualShapeInfo)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedVisualShapeInformation(visualShapeInfo);
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CreateChangeTextureCommandInit(b3PhysicsClientHandle physClient, i32 textureUniqueId, i32 width, i32 height, tukk rgbPixels)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_CHANGE_TEXTURE;

	command->m_changeTextureArgs.m_textureUniqueId = textureUniqueId;
	command->m_changeTextureArgs.m_width = width;
	command->m_changeTextureArgs.m_height = height;
	i32 numPixels = width * height;
	cl->uploadBulletFileToSharedMemory(rgbPixels, numPixels * 3);
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitLoadTexture(b3PhysicsClientHandle physClient, tukk filename)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_LOAD_TEXTURE;
	i32 len = strlen(filename);
	if (len < MAX_FILENAME_LENGTH)
	{
		strcpy(command->m_loadTextureArguments.m_textureFileName, filename);
	}
	else
	{
		command->m_loadTextureArguments.m_textureFileName[0] = 0;
	}
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusTextureUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	i32 uid = -1;
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status)
	{
		Assert(status->m_type == CMD_LOAD_TEXTURE_COMPLETED);
		if (status->m_type == CMD_LOAD_TEXTURE_COMPLETED)
		{
			uid = status->m_loadTextureResultArguments.m_textureUniqueId;
		}
	}
	return uid;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUpdateVisualShape(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 jointIndex, i32 shapeIndex, i32 textureUniqueId)
{
        PhysicsClient* cl = (PhysicsClient*)physClient;
        drx3DAssert(cl);
        drx3DAssert(cl->canSubmitCommand());
        struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
        drx3DAssert(command);
        command->m_type = CMD_UPDATE_VISUAL_SHAPE;
        command->m_updateVisualShapeDataArguments.m_bodyUniqueId = bodyUniqueId;
        command->m_updateVisualShapeDataArguments.m_jointIndex = jointIndex;
        command->m_updateVisualShapeDataArguments.m_shapeIndex = shapeIndex;
        command->m_updateVisualShapeDataArguments.m_textureUniqueId = textureUniqueId;
        command->m_updateFlags = 0;

        if (textureUniqueId >= 0)
        {
                command->m_updateFlags |= CMD_UPDATE_VISUAL_SHAPE_TEXTURE;
        }
        return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitUpdateVisualShape2(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 jointIndex, i32 shapeIndex)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_UPDATE_VISUAL_SHAPE;
	command->m_updateVisualShapeDataArguments.m_bodyUniqueId = bodyUniqueId;
	command->m_updateVisualShapeDataArguments.m_jointIndex = jointIndex;
	command->m_updateVisualShapeDataArguments.m_shapeIndex = shapeIndex;
	command->m_updateVisualShapeDataArguments.m_textureUniqueId = -2;
	command->m_updateVisualShapeDataArguments.m_flags = 0;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3UpdateVisualShapeTexture(b3SharedMemoryCommandHandle commandHandle, i32 textureUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_UPDATE_VISUAL_SHAPE);

	if (command->m_type == CMD_UPDATE_VISUAL_SHAPE)
	{
		if (textureUniqueId >= -1)
		{
			command->m_updateFlags |= CMD_UPDATE_VISUAL_SHAPE_TEXTURE;
			command->m_updateVisualShapeDataArguments.m_textureUniqueId = textureUniqueId;
		}
	}
}

DRX3D_SHARED_API void b3UpdateVisualShapeRGBAColor(b3SharedMemoryCommandHandle commandHandle, const double rgbaColor[4])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_UPDATE_VISUAL_SHAPE);

	if (command->m_type == CMD_UPDATE_VISUAL_SHAPE)
	{
		command->m_updateVisualShapeDataArguments.m_rgbaColor[0] = rgbaColor[0];
		command->m_updateVisualShapeDataArguments.m_rgbaColor[1] = rgbaColor[1];
		command->m_updateVisualShapeDataArguments.m_rgbaColor[2] = rgbaColor[2];
		command->m_updateVisualShapeDataArguments.m_rgbaColor[3] = rgbaColor[3];
		command->m_updateFlags |= CMD_UPDATE_VISUAL_SHAPE_RGBA_COLOR;
	}
}

DRX3D_SHARED_API void b3UpdateVisualShapeFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_UPDATE_VISUAL_SHAPE);

	if (command->m_type == CMD_UPDATE_VISUAL_SHAPE)
	{
		command->m_updateVisualShapeDataArguments.m_flags = flags;
		command->m_updateFlags |= CMD_UPDATE_VISUAL_SHAPE_FLAGS;
	}
}



DRX3D_SHARED_API void b3UpdateVisualShapeSpecularColor(b3SharedMemoryCommandHandle commandHandle, const double specularColor[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_UPDATE_VISUAL_SHAPE);

	if (command->m_type == CMD_UPDATE_VISUAL_SHAPE)
	{
		command->m_updateVisualShapeDataArguments.m_specularColor[0] = specularColor[0];
		command->m_updateVisualShapeDataArguments.m_specularColor[1] = specularColor[1];
		command->m_updateVisualShapeDataArguments.m_specularColor[2] = specularColor[2];
		command->m_updateFlags |= CMD_UPDATE_VISUAL_SHAPE_SPECULAR_COLOR;
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3ApplyExternalForceCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_APPLY_EXTERNAL_FORCE;
	command->m_updateFlags = 0;
	command->m_externalForceArguments.m_numForcesAndTorques = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3ApplyExternalForce(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkId, const double force[3], const double position[3], i32 flag)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_APPLY_EXTERNAL_FORCE);
	i32 index = command->m_externalForceArguments.m_numForcesAndTorques;
	command->m_externalForceArguments.m_bodyUniqueIds[index] = bodyUniqueId;
	command->m_externalForceArguments.m_linkIds[index] = linkId;
	command->m_externalForceArguments.m_forceFlags[index] = EF_FORCE + flag;
	for (i32 i = 0; i < 3; ++i)
	{
		command->m_externalForceArguments.m_forcesAndTorques[index + i] = force[i];
		command->m_externalForceArguments.m_positions[index + i] = position[i];
	}

	command->m_externalForceArguments.m_numForcesAndTorques++;
}

DRX3D_SHARED_API void b3ApplyExternalTorque(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueId, i32 linkId, const double torque[3], i32 flag)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_APPLY_EXTERNAL_FORCE);
	i32 index = command->m_externalForceArguments.m_numForcesAndTorques;
	command->m_externalForceArguments.m_bodyUniqueIds[index] = bodyUniqueId;
	command->m_externalForceArguments.m_linkIds[index] = linkId;
	command->m_externalForceArguments.m_forceFlags[index] = EF_TORQUE + flag;

	for (i32 i = 0; i < 3; ++i)
	{
		command->m_externalForceArguments.m_forcesAndTorques[index + i] = torque[i];
	}
	command->m_externalForceArguments.m_numForcesAndTorques++;
}

///compute the forces to achieve an acceleration, given a state q and qdot using inverse dynamics
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3Calculatedrx3d_inverseCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId,
																				const double* jointPositionsQ, const double* jointVelocitiesQdot, const double* jointAccelerations)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_CALCULATE_INVERSE_DYNAMICS;
	command->m_updateFlags = 0;
	command->m_calculatedrx3d_inverseArguments.m_bodyUniqueId = bodyUniqueId;
	command->m_calculatedrx3d_inverseArguments.m_flags = 0;

	i32 dofCount = b3ComputeDofCount(physClient, bodyUniqueId);
	
	for (i32 i = 0; i < dofCount; i++)
	{
		command->m_calculatedrx3d_inverseArguments.m_jointPositionsQ[i] = jointPositionsQ[i];
		command->m_calculatedrx3d_inverseArguments.m_jointVelocitiesQdot[i] = jointVelocitiesQdot[i];
		command->m_calculatedrx3d_inverseArguments.m_jointAccelerations[i] = jointAccelerations[i];
	}
	command->m_calculatedrx3d_inverseArguments.m_dofCountQ = dofCount;
	command->m_calculatedrx3d_inverseArguments.m_dofCountQdot = dofCount;

	return (b3SharedMemoryCommandHandle)command;
}

///compute the forces to achieve an acceleration, given a state q and qdot using inverse dynamics
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3Calculatedrx3d_inverseCommandInit2(b3PhysicsClientHandle physClient, i32 bodyUniqueId,
	const double* jointPositionsQ, i32 dofCountQ, const double* jointVelocitiesQdot, const double* jointAccelerations, i32 dofCountQdot)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_CALCULATE_INVERSE_DYNAMICS;
	command->m_updateFlags = 0;
	command->m_calculatedrx3d_inverseArguments.m_bodyUniqueId = bodyUniqueId;
	command->m_calculatedrx3d_inverseArguments.m_flags = 0;

	command->m_calculatedrx3d_inverseArguments.m_dofCountQ = dofCountQ;
	for (i32 i = 0; i < dofCountQ; i++)
	{
		command->m_calculatedrx3d_inverseArguments.m_jointPositionsQ[i] = jointPositionsQ[i];
	}
	
	command->m_calculatedrx3d_inverseArguments.m_dofCountQdot = dofCountQdot;
	for (i32 i=0;i<dofCountQdot;i++)
	{
		command->m_calculatedrx3d_inverseArguments.m_jointVelocitiesQdot[i] = jointVelocitiesQdot[i];
		command->m_calculatedrx3d_inverseArguments.m_jointAccelerations[i] = jointAccelerations[i];
	}
	
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3Calculatedrx3d_inverseSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*) commandHandle;
	command->m_calculatedrx3d_inverseArguments.m_flags = flags;
}

DRX3D_SHARED_API i32 b3GetStatusdrx3d_inverseJointForces(b3SharedMemoryStatusHandle statusHandle,
														i32* bodyUniqueId,
														i32* dofCount,
														double* jointForces)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status == 0)
		return false;

	Assert(status->m_type == CMD_CALCULATED_INVERSE_DYNAMICS_COMPLETED);
	if (status->m_type != CMD_CALCULATED_INVERSE_DYNAMICS_COMPLETED)
		return false;

	if (dofCount)
	{
		*dofCount = status->m_inverseDynamicsResultArgs.m_dofCount;
	}
	if (bodyUniqueId)
	{
		*bodyUniqueId = status->m_inverseDynamicsResultArgs.m_bodyUniqueId;
	}
	if (jointForces)
	{
		for (i32 i = 0; i < status->m_inverseDynamicsResultArgs.m_dofCount; i++)
		{
			jointForces[i] = status->m_inverseDynamicsResultArgs.m_jointForces[i];
		}
	}

	return true;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CalculateJacobianCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, i32 linkIndex, const double* localPosition, const double* jointPositionsQ, const double* jointVelocitiesQdot, const double* jointAccelerations)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_CALCULATE_JACOBIAN;
	command->m_updateFlags = 0;
	command->m_calculateJacobianArguments.m_bodyUniqueId = bodyUniqueId;
	command->m_calculateJacobianArguments.m_linkIndex = linkIndex;
	command->m_calculateJacobianArguments.m_localPosition[0] = localPosition[0];
	command->m_calculateJacobianArguments.m_localPosition[1] = localPosition[1];
	command->m_calculateJacobianArguments.m_localPosition[2] = localPosition[2];

	i32 numDofs = b3ComputeDofCount(physClient, bodyUniqueId);
	for (i32 i = 0; i < numDofs; i++)
	{
		command->m_calculateJacobianArguments.m_jointPositionsQ[i] = jointPositionsQ[i];
		command->m_calculateJacobianArguments.m_jointVelocitiesQdot[i] = jointVelocitiesQdot[i];
		command->m_calculateJacobianArguments.m_jointAccelerations[i] = jointAccelerations[i];
	}

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusJacobian(b3SharedMemoryStatusHandle statusHandle, i32* dofCount, double* linearJacobian, double* angularJacobian)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status == 0)
		return false;

	Assert(status->m_type == CMD_CALCULATED_JACOBIAN_COMPLETED);
	if (status->m_type != CMD_CALCULATED_JACOBIAN_COMPLETED)
		return false;

	if (dofCount)
	{
		*dofCount = status->m_jacobianResultArgs.m_dofCount;
	}
	if (linearJacobian)
	{
		for (i32 i = 0; i < status->m_jacobianResultArgs.m_dofCount * 3; i++)
		{
			linearJacobian[i] = status->m_jacobianResultArgs.m_linearJacobian[i];
		}
	}
	if (angularJacobian)
	{
		for (i32 i = 0; i < status->m_jacobianResultArgs.m_dofCount * 3; i++)
		{
			angularJacobian[i] = status->m_jacobianResultArgs.m_angularJacobian[i];
		}
	}

	return true;
}
DRX3D_SHARED_API void b3CalculateMassMatrixSetFlags(b3SharedMemoryCommandHandle commandHandle, i32 flags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*) commandHandle;
	command->m_calculateMassMatrixArguments.m_flags = flags;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CalculateMassMatrixCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId, const double* jointPositionsQ, i32 dofCountQ)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_CALCULATE_MASS_MATRIX;
	command->m_updateFlags = 0;
	
	for (i32 i = 0; i < dofCountQ; i++)
	{
		command->m_calculateMassMatrixArguments.m_jointPositionsQ[i] = jointPositionsQ[i];
	}
	command->m_calculateMassMatrixArguments.m_bodyUniqueId = bodyUniqueId;
	command->m_calculateMassMatrixArguments.m_dofCountQ = dofCountQ;
	command->m_calculateMassMatrixArguments.m_flags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusMassMatrix(b3PhysicsClientHandle physClient, b3SharedMemoryStatusHandle statusHandle, i32* dofCount, double* massMatrix)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);

	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	if (status == 0)
		return false;

	Assert(status->m_type == CMD_CALCULATED_MASS_MATRIX_COMPLETED);
	if (status->m_type != CMD_CALCULATED_MASS_MATRIX_COMPLETED)
		return false;

	if (dofCount)
	{
		*dofCount = status->m_massMatrixResultArgs.m_dofCount;
	}
	if (massMatrix)
	{
		cl->getCachedMassMatrix(status->m_massMatrixResultArgs.m_dofCount, massMatrix);
	}

	return true;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CollisionFilterCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_COLLISION_FILTER;
	command->m_collisionFilterArgs.m_bodyUniqueIdA = -1;
	command->m_collisionFilterArgs.m_bodyUniqueIdB = -1;
	command->m_collisionFilterArgs.m_linkIndexA = -2;
	command->m_collisionFilterArgs.m_linkIndexB = -2;
	command->m_collisionFilterArgs.m_enableCollision = 0;
	command->m_updateFlags = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3SetCollisionFilterPair(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA,
											i32 bodyUniqueIdB, i32 linkIndexA, i32 linkIndexB, i32 enableCollision)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_COLLISION_FILTER);
	command->m_updateFlags = D3_COLLISION_FILTER_PAIR;
	command->m_collisionFilterArgs.m_bodyUniqueIdA = bodyUniqueIdA;
	command->m_collisionFilterArgs.m_bodyUniqueIdB = bodyUniqueIdB;
	command->m_collisionFilterArgs.m_linkIndexA = linkIndexA;
	command->m_collisionFilterArgs.m_linkIndexB = linkIndexB;
	command->m_collisionFilterArgs.m_enableCollision = enableCollision;
}

DRX3D_SHARED_API void b3SetCollisionFilterGroupMask(b3SharedMemoryCommandHandle commandHandle, i32 bodyUniqueIdA,
												 i32 linkIndexA, i32 collisionFilterGroup, i32 collisionFilterMask)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_COLLISION_FILTER);
	command->m_updateFlags = D3_COLLISION_FILTER_GROUP_MASK;
	command->m_collisionFilterArgs.m_bodyUniqueIdA = bodyUniqueIdA;
	command->m_collisionFilterArgs.m_linkIndexA = linkIndexA;
	command->m_collisionFilterArgs.m_collisionFilterGroup = collisionFilterGroup;
	command->m_collisionFilterArgs.m_collisionFilterMask = collisionFilterMask;
}

///compute the joint positions to move the end effector to a desired target using inverse kinematics
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3CalculateInverseKinematicsCommandInit(b3PhysicsClientHandle physClient, i32 bodyUniqueId)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_CALCULATE_INVERSE_KINEMATICS;
	command->m_updateFlags = 0;
	command->m_calculateInverseKinematicsArguments.m_bodyUniqueId = bodyUniqueId;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsAddTargetPurePosition(b3SharedMemoryCommandHandle commandHandle, i32 endEffectorLinkIndex, const double targetPosition[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_TARGET_POSITION;
	command->m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0] = endEffectorLinkIndex;
	command->m_calculateInverseKinematicsArguments.m_targetPositions[0] = targetPosition[0];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[1] = targetPosition[1];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[2] = targetPosition[2];
	command->m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices = 1;

	command->m_calculateInverseKinematicsArguments.m_targetOrientation[0] = 0;
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[1] = 0;
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[2] = 0;
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[3] = 1;
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsAddTargetsPurePosition(b3SharedMemoryCommandHandle commandHandle, i32 numEndEffectorLinkIndices, i32k* endEffectorIndices, const double* targetPositions)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_TARGET_POSITION;
	command->m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices = numEndEffectorLinkIndices;

	for (i32 i = 0; i < numEndEffectorLinkIndices; i++)
	{
		command->m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[i] = endEffectorIndices[i];
		command->m_calculateInverseKinematicsArguments.m_targetPositions[i * 3 + 0] = targetPositions[i * 3 + 0];
		command->m_calculateInverseKinematicsArguments.m_targetPositions[i * 3 + 1] = targetPositions[i * 3 + 1];
		command->m_calculateInverseKinematicsArguments.m_targetPositions[i * 3 + 2] = targetPositions[i * 3 + 2];
	}
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[0] = 0;
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[1] = 0;
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[2] = 0;
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[3] = 1;
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsAddTargetPositionWithOrientation(b3SharedMemoryCommandHandle commandHandle, i32 endEffectorLinkIndex, const double targetPosition[3], const double targetOrientation[4])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_TARGET_POSITION + IK_HAS_TARGET_ORIENTATION;
	command->m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0] = endEffectorLinkIndex;
	command->m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices = 1;

	command->m_calculateInverseKinematicsArguments.m_targetPositions[0] = targetPosition[0];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[1] = targetPosition[1];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[2] = targetPosition[2];

	command->m_calculateInverseKinematicsArguments.m_targetOrientation[0] = targetOrientation[0];
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[1] = targetOrientation[1];
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[2] = targetOrientation[2];
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[3] = targetOrientation[3];
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsPosWithNullSpaceVel(b3SharedMemoryCommandHandle commandHandle, i32 numDof, i32 endEffectorLinkIndex, const double targetPosition[3], const double* lowerLimit, const double* upperLimit, const double* jointRange, const double* restPose)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_TARGET_POSITION + IK_HAS_NULL_SPACE_VELOCITY;
	command->m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0] = endEffectorLinkIndex;
	command->m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices = 1;

	command->m_calculateInverseKinematicsArguments.m_targetPositions[0] = targetPosition[0];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[1] = targetPosition[1];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[2] = targetPosition[2];

	for (i32 i = 0; i < numDof; ++i)
	{
		command->m_calculateInverseKinematicsArguments.m_lowerLimit[i] = lowerLimit[i];
		command->m_calculateInverseKinematicsArguments.m_upperLimit[i] = upperLimit[i];
		command->m_calculateInverseKinematicsArguments.m_jointRange[i] = jointRange[i];
		command->m_calculateInverseKinematicsArguments.m_restPose[i] = restPose[i];
	}
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsPosOrnWithNullSpaceVel(b3SharedMemoryCommandHandle commandHandle, i32 numDof, i32 endEffectorLinkIndex, const double targetPosition[3], const double targetOrientation[4], const double* lowerLimit, const double* upperLimit, const double* jointRange, const double* restPose)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_TARGET_POSITION + IK_HAS_TARGET_ORIENTATION + IK_HAS_NULL_SPACE_VELOCITY;
	command->m_calculateInverseKinematicsArguments.m_endEffectorLinkIndices[0] = endEffectorLinkIndex;
	command->m_calculateInverseKinematicsArguments.m_numEndEffectorLinkIndices = 1;

	command->m_calculateInverseKinematicsArguments.m_targetPositions[0] = targetPosition[0];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[1] = targetPosition[1];
	command->m_calculateInverseKinematicsArguments.m_targetPositions[2] = targetPosition[2];

	command->m_calculateInverseKinematicsArguments.m_targetOrientation[0] = targetOrientation[0];
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[1] = targetOrientation[1];
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[2] = targetOrientation[2];
	command->m_calculateInverseKinematicsArguments.m_targetOrientation[3] = targetOrientation[3];

	for (i32 i = 0; i < numDof; ++i)
	{
		command->m_calculateInverseKinematicsArguments.m_lowerLimit[i] = lowerLimit[i];
		command->m_calculateInverseKinematicsArguments.m_upperLimit[i] = upperLimit[i];
		command->m_calculateInverseKinematicsArguments.m_jointRange[i] = jointRange[i];
		command->m_calculateInverseKinematicsArguments.m_restPose[i] = restPose[i];
	}
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsSetCurrentPositions(b3SharedMemoryCommandHandle commandHandle, i32 numDof, const double* currentJointPositions)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_CURRENT_JOINT_POSITIONS;
	for (i32 i = 0; i < numDof; ++i)
	{
		command->m_calculateInverseKinematicsArguments.m_currentPositions[i] = currentJointPositions[i];
	}
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsSetMaxNumIterations(b3SharedMemoryCommandHandle commandHandle, i32 maxNumIterations)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_MAX_ITERATIONS;
	command->m_calculateInverseKinematicsArguments.m_maxNumIterations = maxNumIterations;
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsSetResidualThreshold(b3SharedMemoryCommandHandle commandHandle, double residualThreshold)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_RESIDUAL_THRESHOLD;
	command->m_calculateInverseKinematicsArguments.m_residualThreshold = residualThreshold;
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsSetJointDamping(b3SharedMemoryCommandHandle commandHandle, i32 numDof, const double* jointDampingCoeff)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= IK_HAS_JOINT_DAMPING;

	for (i32 i = 0; i < numDof; ++i)
	{
		command->m_calculateInverseKinematicsArguments.m_jointDamping[i] = jointDampingCoeff[i];
	}
}

DRX3D_SHARED_API void b3CalculateInverseKinematicsSelectSolver(b3SharedMemoryCommandHandle commandHandle, i32 solver)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CALCULATE_INVERSE_KINEMATICS);
	command->m_updateFlags |= solver;
}

DRX3D_SHARED_API i32 b3GetStatusInverseKinematicsJointPositions(b3SharedMemoryStatusHandle statusHandle,
															 i32* bodyUniqueId,
															 i32* dofCount,
															 double* jointPositions)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	Assert(status);
	if (status == 0)
		return 0;
	Assert(status->m_type == CMD_CALCULATE_INVERSE_KINEMATICS_COMPLETED);
	if (status->m_type != CMD_CALCULATE_INVERSE_KINEMATICS_COMPLETED)
		return 0;

	if (dofCount)
	{
		*dofCount = status->m_inverseKinematicsResultArgs.m_dofCount;
	}
	if (bodyUniqueId)
	{
		*bodyUniqueId = status->m_inverseKinematicsResultArgs.m_bodyUniqueId;
	}
	if (jointPositions)
	{
		for (i32 i = 0; i < status->m_inverseKinematicsResultArgs.m_dofCount; i++)
		{
			jointPositions[i] = status->m_inverseKinematicsResultArgs.m_jointPositions[i];
		}
	}

	return 1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestVREventsCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REQUEST_VR_EVENTS_DATA;
	command->m_updateFlags = VR_DEVICE_CONTROLLER;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3VREventsSetDeviceTypeFilter(b3SharedMemoryCommandHandle commandHandle, i32 deviceTypeFilter)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	if (command->m_type == CMD_REQUEST_VR_EVENTS_DATA)
	{
		command->m_updateFlags = deviceTypeFilter;
	}
}

DRX3D_SHARED_API void b3GetVREventsData(b3PhysicsClientHandle physClient, struct b3VREventsData* vrEventsData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedVREvents(vrEventsData);
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SetVRCameraStateCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_SET_VR_CAMERA_STATE;
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3SetVRCameraRootPosition(b3SharedMemoryCommandHandle commandHandle, const double rootPos[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_SET_VR_CAMERA_STATE);
	command->m_updateFlags |= VR_CAMERA_ROOT_POSITION;
	command->m_vrCameraStateArguments.m_rootPosition[0] = rootPos[0];
	command->m_vrCameraStateArguments.m_rootPosition[1] = rootPos[1];
	command->m_vrCameraStateArguments.m_rootPosition[2] = rootPos[2];
	return 0;
}

DRX3D_SHARED_API i32 b3SetVRCameraRootOrientation(b3SharedMemoryCommandHandle commandHandle, const double rootOrn[4])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_SET_VR_CAMERA_STATE);
	command->m_updateFlags |= VR_CAMERA_ROOT_ORIENTATION;
	command->m_vrCameraStateArguments.m_rootOrientation[0] = rootOrn[0];
	command->m_vrCameraStateArguments.m_rootOrientation[1] = rootOrn[1];
	command->m_vrCameraStateArguments.m_rootOrientation[2] = rootOrn[2];
	command->m_vrCameraStateArguments.m_rootOrientation[3] = rootOrn[3];

	return 0;
}

DRX3D_SHARED_API i32 b3SetVRCameraTrackingObject(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_SET_VR_CAMERA_STATE);
	command->m_updateFlags |= VR_CAMERA_ROOT_TRACKING_OBJECT;
	command->m_vrCameraStateArguments.m_trackingObjectUniqueId = objectUniqueId;
	return 0;
}

DRX3D_SHARED_API i32 b3SetVRCameraTrackingObjectFlag(b3SharedMemoryCommandHandle commandHandle, i32 flag)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_SET_VR_CAMERA_STATE);
	command->m_updateFlags |= VR_CAMERA_FLAG;
	command->m_vrCameraStateArguments.m_trackingObjectFlag = flag;
	return 0;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestKeyboardEventsCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	return b3RequestKeyboardEventsCommandInit2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestKeyboardEventsCommandInit2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_REQUEST_KEYBOARD_EVENTS_DATA;
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3GetKeyboardEventsData(b3PhysicsClientHandle physClient, struct b3KeyboardEventsData* keyboardEventsData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedKeyboardEvents(keyboardEventsData);
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3RequestMouseEventsCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REQUEST_MOUSE_EVENTS_DATA;
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3GetMouseEventsData(b3PhysicsClientHandle physClient, struct b3MouseEventsData* mouseEventsData)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	if (cl)
	{
		cl->getCachedMouseEvents(mouseEventsData);
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3ProfileTimingCommandInit(b3PhysicsClientHandle physClient, tukk name)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	i32 len = name ? strlen(name) : 0;
	command->m_type = CMD_PROFILE_TIMING;
	if (len > 0 && len < (MAX_FILENAME_LENGTH + 1))
	{
		
		strcpy(command->m_profile.m_name, name);
		command->m_profile.m_name[len] = 0;
	}
	else
	{
		command->m_profile.m_name[0] = 0;
	}
	command->m_profile.m_type = -1;
	command->m_profile.m_durationInMicroSeconds = 0;
	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3PushProfileTiming(b3PhysicsClientHandle physClient, tukk timingName)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	cl->pushProfileTiming(timingName);
}

DRX3D_SHARED_API void b3PopProfileTiming(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	cl->popProfileTiming();
}

DRX3D_SHARED_API void b3SetProfileTimingDuractionInMicroSeconds(b3SharedMemoryCommandHandle commandHandle, i32 duration)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_PROFILE_TIMING);
	if (command->m_type == CMD_PROFILE_TIMING)
	{
		command->m_profile.m_durationInMicroSeconds = duration;
	}
}

DRX3D_SHARED_API void b3SetProfileTimingType(b3SharedMemoryCommandHandle commandHandle, i32 type)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_PROFILE_TIMING);
	if (command->m_type == CMD_PROFILE_TIMING)
	{
		command->m_profile.m_type = type;
	}
}


DRX3D_SHARED_API b3SharedMemoryCommandHandle b3StateLoggingCommandInit(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_STATE_LOGGING;
	command->m_updateFlags = 0;
	command->m_stateLoggingArguments.m_numBodyUniqueIds = 0;
	command->m_stateLoggingArguments.m_deviceFilterType = VR_DEVICE_CONTROLLER;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3StateLoggingStart(b3SharedMemoryCommandHandle commandHandle, i32 loggingType, tukk fileName)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_START_LOG;
		i32 len = strlen(fileName);
		if (len < MAX_FILENAME_LENGTH)
		{
			strcpy(command->m_stateLoggingArguments.m_fileName, fileName);
		}
		else
		{
			command->m_stateLoggingArguments.m_fileName[0] = 0;
		}
		command->m_stateLoggingArguments.m_logType = loggingType;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3GetStatusLoggingUniqueId(b3SharedMemoryStatusHandle statusHandle)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	drx3DAssert(status);
	drx3DAssert(status->m_type == CMD_STATE_LOGGING_START_COMPLETED);
	if (status && status->m_type == CMD_STATE_LOGGING_START_COMPLETED)
	{
		return status->m_stateLoggingResultArgs.m_loggingUniqueId;
	}
	return -1;
}

DRX3D_SHARED_API i32 b3StateLoggingAddLoggingObjectUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 objectUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_FILTER_OBJECT_UNIQUE_ID;
		if (command->m_stateLoggingArguments.m_numBodyUniqueIds < MAX_SDF_BODIES)
		{
			command->m_stateLoggingArguments.m_bodyUniqueIds[command->m_stateLoggingArguments.m_numBodyUniqueIds++] = objectUniqueId;
		}
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetLinkIndexA(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexA)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type = CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_FILTER_LINK_INDEX_A;
		command->m_stateLoggingArguments.m_linkIndexA = linkIndexA;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetLinkIndexB(b3SharedMemoryCommandHandle commandHandle, i32 linkIndexB)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type = CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_FILTER_LINK_INDEX_B;
		command->m_stateLoggingArguments.m_linkIndexB = linkIndexB;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetBodyAUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 bodyAUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type = CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_FILTER_BODY_UNIQUE_ID_A;
		command->m_stateLoggingArguments.m_bodyUniqueIdA = bodyAUniqueId;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetBodyBUniqueId(b3SharedMemoryCommandHandle commandHandle, i32 bodyBUniqueId)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type = CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_FILTER_BODY_UNIQUE_ID_B;
		command->m_stateLoggingArguments.m_bodyUniqueIdB = bodyBUniqueId;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetMaxLogDof(b3SharedMemoryCommandHandle commandHandle, i32 maxLogDof)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_MAX_LOG_DOF;
		command->m_stateLoggingArguments.m_maxLogDof = maxLogDof;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetLogFlags(b3SharedMemoryCommandHandle commandHandle, i32 logFlags)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_LOG_FLAGS;
		command->m_stateLoggingArguments.m_logFlags = logFlags;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingSetDeviceTypeFilter(b3SharedMemoryCommandHandle commandHandle, i32 deviceTypeFilter)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_FILTER_DEVICE_TYPE;
		command->m_stateLoggingArguments.m_deviceFilterType = deviceTypeFilter;
	}
	return 0;
}

DRX3D_SHARED_API i32 b3StateLoggingStop(b3SharedMemoryCommandHandle commandHandle, i32 loggingUid)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_STATE_LOGGING);
	if (command->m_type == CMD_STATE_LOGGING)
	{
		command->m_updateFlags |= STATE_LOGGING_STOP_LOG;
		command->m_stateLoggingArguments.m_loggingUniqueId = loggingUid;
	}
	return 0;
}

///configure the 3D OpenGL debug visualizer (enable/disable GUI widgets, shadows, position camera etc)
DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitConfigureOpenGLVisualizer(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	//drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	return b3InitConfigureOpenGLVisualizer2((b3SharedMemoryCommandHandle)command);
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitConfigureOpenGLVisualizer2(b3SharedMemoryCommandHandle commandHandle)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	command->m_type = CMD_CONFIGURE_OPENGL_VISUALIZER;
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetVisualizationFlags(b3SharedMemoryCommandHandle commandHandle, i32 flag, i32 enabled)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_FLAGS;
		command->m_configureOpenGLVisualizerArguments.m_setFlag = flag;
		command->m_configureOpenGLVisualizerArguments.m_setEnabled = enabled;
	}
}

DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetLightPosition(b3SharedMemoryCommandHandle commandHandle, const float lightPosition[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_LIGHT_POSITION;
		command->m_configureOpenGLVisualizerArguments.m_lightPosition[0] = lightPosition[0];
		command->m_configureOpenGLVisualizerArguments.m_lightPosition[1] = lightPosition[1];
		command->m_configureOpenGLVisualizerArguments.m_lightPosition[2] = lightPosition[2];
	}
}

DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetLightRgbBackground(b3SharedMemoryCommandHandle commandHandle, const float rgbBackground[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_RGB_BACKGROUND;
		command->m_configureOpenGLVisualizerArguments.m_rgbBackground[0] = rgbBackground[0];
		command->m_configureOpenGLVisualizerArguments.m_rgbBackground[1] = rgbBackground[1];
		command->m_configureOpenGLVisualizerArguments.m_rgbBackground[2] = rgbBackground[2];
	}
}


DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetShadowMapResolution(b3SharedMemoryCommandHandle commandHandle, i32 shadowMapResolution)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_SHADOWMAP_RESOLUTION;
		command->m_configureOpenGLVisualizerArguments.m_shadowMapResolution = shadowMapResolution;
	}
}

DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetShadowMapIntensity(b3SharedMemoryCommandHandle commandHandle, double shadowMapIntensity)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_SHADOWMAP_INTENSITY;
		command->m_configureOpenGLVisualizerArguments.m_shadowMapIntensity = shadowMapIntensity;
	}
}


DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetShadowMapWorldSize(b3SharedMemoryCommandHandle commandHandle, i32 shadowMapWorldSize)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_SHADOWMAP_WORLD_SIZE;
		command->m_configureOpenGLVisualizerArguments.m_shadowMapWorldSize = shadowMapWorldSize;
	}
}


DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetRemoteSyncTransformInterval(b3SharedMemoryCommandHandle commandHandle, double remoteSyncTransformInterval)
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);
	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_REMOTE_SYNC_TRANSFORM_INTERVAL;
		command->m_configureOpenGLVisualizerArguments.m_remoteSyncTransformInterval = remoteSyncTransformInterval;
	}
}


DRX3D_SHARED_API void b3ConfigureOpenGLVisualizerSetViewMatrix(b3SharedMemoryCommandHandle commandHandle, float cameraDistance, float cameraPitch, float cameraYaw, const float cameraTargetPosition[3])
{
	struct SharedMemoryCommand* command = (struct SharedMemoryCommand*)commandHandle;
	drx3DAssert(command);
	drx3DAssert(command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER);

	if (command->m_type == CMD_CONFIGURE_OPENGL_VISUALIZER)
	{
		command->m_updateFlags |= COV_SET_CAMERA_VIEW_MATRIX;
		command->m_configureOpenGLVisualizerArguments.m_cameraDistance = cameraDistance;
		command->m_configureOpenGLVisualizerArguments.m_cameraPitch = cameraPitch;
		command->m_configureOpenGLVisualizerArguments.m_cameraYaw = cameraYaw;
		command->m_configureOpenGLVisualizerArguments.m_cameraTargetPosition[0] = cameraTargetPosition[0];
		command->m_configureOpenGLVisualizerArguments.m_cameraTargetPosition[1] = cameraTargetPosition[1];
		command->m_configureOpenGLVisualizerArguments.m_cameraTargetPosition[2] = cameraTargetPosition[2];
	}
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3InitRequestOpenGLVisualizerCameraCommand(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);

	command->m_type = CMD_REQUEST_OPENGL_VISUALIZER_CAMERA;
	command->m_updateFlags = 0;

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API i32 b3GetStatusOpenGLVisualizerCamera(b3SharedMemoryStatusHandle statusHandle, b3OpenGLVisualizerCameraInfo* camera)
{
	const SharedMemoryStatus* status = (const SharedMemoryStatus*)statusHandle;
	//drx3DAssert(status);
	if (status && status->m_type == CMD_REQUEST_OPENGL_VISUALIZER_CAMERA_COMPLETED)
	{
		*camera = status->m_visualizerCameraResultArgs;
		return 1;
	}
	return 0;
}

DRX3D_SHARED_API void b3SetTimeOut(b3PhysicsClientHandle physClient, double timeOutInSeconds)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	if (cl)
	{
		cl->setTimeOut(timeOutInSeconds);
	}
}

DRX3D_SHARED_API double b3GetTimeOut(b3PhysicsClientHandle physClient)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	if (cl)
	{
		return cl->getTimeOut();
	}
	return -1;
}

DRX3D_SHARED_API b3SharedMemoryCommandHandle b3SetAdditionalSearchPath(b3PhysicsClientHandle physClient, tukk path)
{
	PhysicsClient* cl = (PhysicsClient*)physClient;
	drx3DAssert(cl);
	drx3DAssert(cl->canSubmitCommand());
	struct SharedMemoryCommand* command = cl->getAvailableSharedMemoryCommand();
	drx3DAssert(command);
	command->m_type = CMD_SET_ADDITIONAL_SEARCH_PATH;
	command->m_updateFlags = 0;
	i32 len = strlen(path);
	if (len < MAX_FILENAME_LENGTH)
	{
		strcpy(command->m_searchPathArgs.m_path, path);
	}

	return (b3SharedMemoryCommandHandle)command;
}

DRX3D_SHARED_API void b3MultiplyTransforms(const double posA[3], const double ornA[4], const double posB[3], const double ornB[4], double outPos[3], double outOrn[4])
{
	b3Transform trA;
	b3Transform trB;
	trA.setOrigin(b3MakeVector3(posA[0], posA[1], posA[2]));
	trA.setRotation(b3Quat(ornA[0], ornA[1], ornA[2], ornA[3]));
	trB.setOrigin(b3MakeVector3(posB[0], posB[1], posB[2]));
	trB.setRotation(b3Quat(ornB[0], ornB[1], ornB[2], ornB[3]));
	b3Transform res = trA * trB;
	outPos[0] = res.getOrigin()[0];
	outPos[1] = res.getOrigin()[1];
	outPos[2] = res.getOrigin()[2];
	b3Quat orn = res.getRotation();
	outOrn[0] = orn[0];
	outOrn[1] = orn[1];
	outOrn[2] = orn[2];
	outOrn[3] = orn[3];
}

DRX3D_SHARED_API void b3InvertTransform(const double pos[3], const double orn[4], double outPos[3], double outOrn[4])
{
	b3Transform tr;
	tr.setOrigin(b3MakeVector3(pos[0], pos[1], pos[2]));
	tr.setRotation(b3Quat(orn[0], orn[1], orn[2], orn[3]));
	b3Transform trInv = tr.inverse();
	outPos[0] = trInv.getOrigin()[0];
	outPos[1] = trInv.getOrigin()[1];
	outPos[2] = trInv.getOrigin()[2];
	b3Quat invOrn = trInv.getRotation();
	outOrn[0] = invOrn[0];
	outOrn[1] = invOrn[1];
	outOrn[2] = invOrn[2];
	outOrn[3] = invOrn[3];
}

DRX3D_SHARED_API void b3QuatSlerp(const double startQuat[/*4*/], const double endQuat[/*4*/], double interpolationFraction, double outOrn[/*4*/])
{
	b3Quat start(startQuat[0], startQuat[1], startQuat[2], startQuat[3]);
	b3Quat end(endQuat[0], endQuat[1], endQuat[2], endQuat[3]);
	b3Quat result = start.slerp(end, interpolationFraction);
	outOrn[0] = result[0];
	outOrn[1] = result[1];
	outOrn[2] = result[2];
	outOrn[3] = result[3];
}

DRX3D_SHARED_API void b3RotateVector(const double quat[/*4*/], const double vec[/*3*/], double vecOut[/*3*/])
{
	b3Quat q(quat[0], quat[1], quat[2], quat[3]);
	b3Vec3 v = b3MakeVector3(vec[0], vec[1], vec[2]);
	b3Vec3 vout = b3QuatRotate(q, v);
	vecOut[0] = vout[0];
	vecOut[1] = vout[1];
	vecOut[2] = vout[2];
}

DRX3D_SHARED_API void b3CalculateVelocityQuaternion(const double startQuat[/*4*/], const double endQuat[/*4*/], double deltaTime, double angVelOut[/*3*/])
{
	b3Quat start(startQuat[0], startQuat[1], startQuat[2], startQuat[3]);
	b3Quat end(endQuat[0], endQuat[1], endQuat[2], endQuat[3]);
	b3Vec3 pos=b3MakeVector3(0, 0, 0);
	b3Vec3 linVel, angVel;
	b3TransformUtil::calculateVelocityQuaternion(pos, pos, start, end, deltaTime, linVel, angVel);
	angVelOut[0] = angVel[0];
	angVelOut[1] = angVel[1];
	angVelOut[2] = angVel[2];
}

DRX3D_SHARED_API void b3GetQuaternionFromAxisAngle(const double axis[/*3*/], double angle, double outQuat[/*4*/])
{
	b3Quat quat(b3MakeVector3(axis[0], axis[1], axis[2]), angle);
	outQuat[0] = quat[0];
	outQuat[1] = quat[1];
	outQuat[2] = quat[2];
	outQuat[3] = quat[3];
}
DRX3D_SHARED_API void b3GetAxisAngleFromQuaternion(const double quat[/*4*/], double axis[/*3*/], double* angle)
{
	b3Quat q(quat[0], quat[1], quat[2], quat[3]);
	b3Vec3 ax = q.getAxis();
	axis[0] = ax[0];
	axis[1] = ax[1];
	axis[2] = ax[2];
	*angle = q.getAngle();
}

DRX3D_SHARED_API void b3GetQuaternionDifference(const double startQuat[/*4*/], const double endQuat[/*4*/], double outOrn[/*4*/])
{
	b3Quat orn0(startQuat[0], startQuat[1], startQuat[2], startQuat[3]);
	b3Quat orn1a(endQuat[0], endQuat[1], endQuat[2], endQuat[3]);
	b3Quat orn1 = orn0.nearest(orn1a);
	b3Quat dorn = orn1 * orn0.inverse();
	outOrn[0] = dorn[0];
	outOrn[1] = dorn[1];
	outOrn[2] = dorn[2];
	outOrn[3] = dorn[3];
}

b3Scalar b3GetMatrixElem(const b3Matrix3x3& mat, i32 index)
{
	i32 i = index % 3;
	i32 j = index / 3;
	return mat[i][j];
}


static bool MyMatrixToEulerXYZ(const b3Matrix3x3& mat, b3Vec3& xyz)
{
	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	b3Scalar fi = b3GetMatrixElem(mat, 2);
	if (fi < b3Scalar(1.0f))
	{
		if (fi > b3Scalar(-1.0f))
		{
			xyz[0] = b3Atan2(-b3GetMatrixElem(mat, 5), b3GetMatrixElem(mat, 8));
			xyz[1] = b3Asin(b3GetMatrixElem(mat, 2));
			xyz[2] = b3Atan2(-b3GetMatrixElem(mat, 1), b3GetMatrixElem(mat, 0));
			return true;
		}
		else
		{
			// WARNING.  Not unique.  XA - ZA = -atan2(r10,r11)
			xyz[0] = -b3Atan2(b3GetMatrixElem(mat, 3), b3GetMatrixElem(mat, 4));
			xyz[1] = -D3_HALF_PI;
			xyz[2] = b3Scalar(0.0);
			return false;
		}
	}
	else
	{
		// WARNING.  Not unique.  XAngle + ZAngle = atan2(r10,r11)
		xyz[0] = b3Atan2(b3GetMatrixElem(mat, 3), b3GetMatrixElem(mat, 4));
		xyz[1] = D3_HALF_PI;
		xyz[2] = 0.0;
	}
	return false;
}


DRX3D_SHARED_API void b3GetAxisDifferenceQuaternion(const double startQuat[/*4*/], const double endQuat[/*4*/], double axisOut[/*3*/])
{
	b3Quat currentQuat(startQuat[0], startQuat[1], startQuat[2], startQuat[3]);
	b3Quat desiredQuat(endQuat[0], endQuat[1], endQuat[2], endQuat[3]);

	b3Quat relRot = currentQuat.inverse() * desiredQuat;
	b3Vec3 angleDiff;
	MyMatrixToEulerXYZ(b3Matrix3x3(relRot), angleDiff);
	axisOut[0] = angleDiff[0];
	axisOut[1] = angleDiff[1];
	axisOut[2] = angleDiff[2];

}

#ifdef DRX3D_ENABLE_VHACD
#include "VHACD.h"
#include <string>

i32 main_vhacd_ext(const STxt& fileNameIn, const STxt& fileNameOut, const STxt& fileNameLog, VHACD::IVHACD::Parameters& paramsVHACD);

DRX3D_SHARED_API void b3VHACD(tukk fileNameInput, tukk fileNameOutput, tukk fileNameLogging,
	double concavity, double alpha, double beta, double gamma, double minVolumePerCH, i32 resolution,
	i32 maxNumVerticesPerCH, i32 depth, i32 planeDownsampling, i32 convexhullDownsampling,
	i32 pca, i32 mode, i32 convexhullApproximation)
{
	STxt fileNameIn(fileNameInput);
	STxt fileNameOut(fileNameOutput);
	STxt fileNameLog(fileNameLogging);
	VHACD::IVHACD::Parameters params;
	if (concavity >= 0)
		params.m_concavity = concavity;
	if (alpha >= 0)
		params.m_alpha = alpha;
	if (beta >= 0)
		params.m_beta = beta;
	if (gamma >= 0)
		params.m_gamma = gamma;
	if (minVolumePerCH >= 0)
		params.m_minVolumePerCH = minVolumePerCH;
	if (resolution >= 0)
		params.m_resolution = (u32)resolution;
	if (maxNumVerticesPerCH >= 0)
		params.m_maxNumVerticesPerCH = (u32)maxNumVerticesPerCH;
	if (depth >= 0)
		params.m_depth = depth;
	if (planeDownsampling >= 0)
		params.m_planeDownsampling = planeDownsampling;
	if (convexhullDownsampling >= 0)
		params.m_convexhullDownsampling = convexhullDownsampling;
	if (pca >= 0)
		params.m_pca = pca;
	if (mode >= 0)
		params.m_mode = mode;
	if (convexhullApproximation >= 0)
		params.m_convexhullApproximation = convexhullApproximation;
	main_vhacd_ext(fileNameIn, fileNameOut, fileNameLog, params);
}
#endif


