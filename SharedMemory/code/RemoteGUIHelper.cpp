#include "../RemoteGUIHelper.h"

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3Logging.h>
#include "../GraphicsSharedMemoryCommands.h"
#include "../PosixSharedMemory.h"
#include "../Win32SharedMemory.h"
#include "../GraphicsSharedMemoryBlock.h"
#include <drx3D/Common/b3Scalar.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>

struct RemoteGUIHelperInternalData
{
	//	GUIHelperInterface* m_guiHelper;
	bool m_waitingForServer;
	GraphicsSharedMemoryBlock* m_testBlock1;
	SharedMemoryInterface* m_sharedMemory;
	GraphicsSharedMemoryStatus m_lastServerStatus;
	i32 m_sharedMemoryKey;
	bool m_isConnected;

	RemoteGUIHelperInternalData()
		: m_waitingForServer(false),
		  m_testBlock1(0)
	{
#ifdef _WIN32
		m_sharedMemory = new Win32SharedMemoryClient();
#else
		m_sharedMemory = new PosixSharedMemory();
#endif
		m_sharedMemoryKey = GRAPHICS_SHARED_MEMORY_KEY;
		m_isConnected = false;
		connect();
	}

	virtual ~RemoteGUIHelperInternalData()
	{
		disconnect();
		delete m_sharedMemory;
	}

	virtual bool isConnected()
	{
		return m_isConnected;
	}

	bool canSubmitCommand() const
	{
		if (m_isConnected && !m_waitingForServer)
		{
			if (m_testBlock1->m_magicId == GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}

	struct GraphicsSharedMemoryCommand* getAvailableSharedMemoryCommand()
	{
		static i32 sequence = 0;
		if (m_testBlock1)
		{
			m_testBlock1->m_clientCommands[0].m_sequenceNumber = sequence++;
			return &m_testBlock1->m_clientCommands[0];
		}
		return 0;
	}

	bool submitClientCommand(const GraphicsSharedMemoryCommand& command)
	{
		/// at the moment we allow a maximum of 1 outstanding command, so we check for this
		// once the server processed the command and returns a status, we clear the flag
		// "m_data->m_waitingForServer" and allow submitting the next command
		Assert(!m_waitingForServer);
		if (!m_waitingForServer)
		{
			//printf("submit command of type %d\n", command.m_type);

			if (&m_testBlock1->m_clientCommands[0] != &command)
			{
				m_testBlock1->m_clientCommands[0] = command;
			}
			m_testBlock1->m_numClientCommands++;
			m_waitingForServer = true;
			return true;
		}
		return false;
	}

	const GraphicsSharedMemoryStatus* processServerStatus()
	{
		// SharedMemoryStatus* stat = 0;

		if (!m_testBlock1)
		{
			m_lastServerStatus.m_type = GFX_CMD_SHARED_MEMORY_NOT_INITIALIZED;
			return &m_lastServerStatus;
		}

		if (!m_waitingForServer)
		{
			return 0;
		}

		if (m_testBlock1->m_magicId != GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
		{
			m_lastServerStatus.m_type = GFX_CMD_SHARED_MEMORY_NOT_INITIALIZED;
			return &m_lastServerStatus;
		}

		if (m_testBlock1->m_numServerCommands >
			m_testBlock1->m_numProcessedServerCommands)
		{
			D3_PROFILE("processServerCMD");
			drx3DAssert(m_testBlock1->m_numServerCommands ==
					 m_testBlock1->m_numProcessedServerCommands + 1);

			const GraphicsSharedMemoryStatus& serverCmd = m_testBlock1->m_serverCommands[0];

			m_lastServerStatus = serverCmd;

			//       EnumSharedMemoryServerStatus s = (EnumSharedMemoryServerStatus)serverCmd.m_type;
			// consume the command
			switch (serverCmd.m_type)
			{
				case GFX_CMD_CLIENT_COMMAND_COMPLETED:
				{
					D3_PROFILE("CMD_CLIENT_COMMAND_COMPLETED");

					break;
				}
				default:
				{
				}
			}

			m_testBlock1->m_numProcessedServerCommands++;
			// we don't have more than 1 command outstanding (in total, either server or client)
			drx3DAssert(m_testBlock1->m_numProcessedServerCommands ==
					 m_testBlock1->m_numServerCommands);

			if (m_testBlock1->m_numServerCommands ==
				m_testBlock1->m_numProcessedServerCommands)
			{
				m_waitingForServer = false;
			}
			else
			{
				m_waitingForServer = true;
			}

			return &m_lastServerStatus;
		}
		return 0;
	}

	bool connect()
	{
		/// server always has to create and initialize shared memory
		bool allowCreation = false;
		m_testBlock1 = (GraphicsSharedMemoryBlock*)m_sharedMemory->allocateSharedMemory(
			m_sharedMemoryKey, GRAPHICS_SHARED_MEMORY_SIZE, allowCreation);

		if (m_testBlock1)
		{
			if (m_testBlock1->m_magicId != GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER)
			{
				drx3DError("Error connecting to shared memory: please start server before client\n");
				m_sharedMemory->releaseSharedMemory(m_sharedMemoryKey,
													GRAPHICS_SHARED_MEMORY_SIZE);
				m_testBlock1 = 0;
				return false;
			}
			else
			{
				m_isConnected = true;
			}
		}
		else
		{
			drx3DWarning("Cannot connect to shared memory");
			return false;
		}
		return true;
	}

	void disconnect()
	{
		if (m_isConnected && m_sharedMemory)
		{
			m_sharedMemory->releaseSharedMemory(m_sharedMemoryKey, GRAPHICS_SHARED_MEMORY_SIZE);
		}
		m_isConnected = false;
	}
};

RemoteGUIHelper::RemoteGUIHelper()
{
	m_data = new RemoteGUIHelperInternalData;
	if (m_data->canSubmitCommand())
	{
		removeAllGraphicsInstances();
	}
}

RemoteGUIHelper::~RemoteGUIHelper()
{
	delete m_data;
}

bool RemoteGUIHelper::isConnected() const
{
	return m_data->isConnected();
}

void RemoteGUIHelper::setVisualizerFlag(i32 flag, i32 enable)
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_updateFlags = 0;
		cmd->m_visualizerFlagCommand.m_visualizerFlag = flag;
		cmd->m_visualizerFlagCommand.m_enable = enable;
		cmd->m_type = GFX_CMD_SET_VISUALIZER_FLAG;
		m_data->submitClientCommand(*cmd);
	}
	const GraphicsSharedMemoryStatus* status = 0;
	while ((status = m_data->processServerStatus()) == 0)
	{
	}
}

void RemoteGUIHelper::createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color)
{
	printf("createRigidBodyGraphicsObject\n");
}

bool RemoteGUIHelper::getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		
		cmd->m_updateFlags = 0;
		cmd->m_type = GFX_CMD_GET_CAMERA_INFO;
		m_data->submitClientCommand(*cmd);
	}
	const GraphicsSharedMemoryStatus* status = 0;
	while ((status = m_data->processServerStatus()) == 0)
	{
	}
	if (status->m_type == GFX_CMD_GET_CAMERA_INFO_COMPLETED)
	{
		*width = status->m_getCameraInfoStatus.width;
		*height = status->m_getCameraInfoStatus.height;
		for (i32 i = 0; i < 16; i++)
		{
			viewMatrix[i] = status->m_getCameraInfoStatus.viewMatrix[i];
			projectionMatrix[i] = status->m_getCameraInfoStatus.projectionMatrix[i];
		}
		for (i32 i = 0; i < 3; i++)
		{
			camUp[i] = status->m_getCameraInfoStatus.camUp[i];
			camForward[i] = status->m_getCameraInfoStatus.camForward[i];
			hor[i] = status->m_getCameraInfoStatus.hor[i];
			vert[i] = status->m_getCameraInfoStatus.vert[i];
			camTarget[i] = status->m_getCameraInfoStatus.camTarget[i];
		}
		*yaw = status->m_getCameraInfoStatus.yaw;
		*pitch = status->m_getCameraInfoStatus.pitch;
		*camDist = status->m_getCameraInfoStatus.camDist;
		return true;
	}
	return false;
}

void RemoteGUIHelper::createCollisionObjectGraphicsObject(CollisionObject2* body, const Vec3& color)
{
	if (body->getUserIndex() < 0)
	{
		CollisionShape* shape = body->getCollisionShape();
		Transform2 startTransform = body->getWorldTransform();
		i32 graphicsShapeId = shape->getUserIndex();
		if (graphicsShapeId >= 0)
		{
			//	Assert(graphicsShapeId >= 0);
			//the graphics shape is already scaled
			float localScaling[4] = {1.f, 1.f, 1.f, 1.f};
			float colorRGBA[4] = {(float)color[0], (float)color[1], (float)color[2], (float)color[3]};
			float pos[4] = {(float)startTransform.getOrigin()[0], (float)startTransform.getOrigin()[1], (float)startTransform.getOrigin()[2], (float)startTransform.getOrigin()[3]};
			float orn[4] = {(float)startTransform.getRotation()[0], (float)startTransform.getRotation()[1], (float)startTransform.getRotation()[2], (float)startTransform.getRotation()[3]};
			i32 graphicsInstanceId = registerGraphicsInstance(graphicsShapeId, pos, orn, colorRGBA, localScaling);
			body->setUserIndex(graphicsInstanceId);
		}
	}
}

void RemoteGUIHelper::createCollisionShapeGraphicsObject(CollisionShape* collisionShape)
{
	printf("createCollisionShapeGraphicsObject\n");
}

void RemoteGUIHelper::syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld)
{
}

void RemoteGUIHelper::syncPhysicsToGraphics2(const DiscreteDynamicsWorld* rbWorld)
{
	b3AlignedObjectArray<GUISyncPosition> updatedPositions;

	i32 numCollisionObjects = rbWorld->getNumCollisionObjects();
	{
		D3_PROFILE("write all InstanceTransformToCPU2");
		for (i32 i = 0; i < numCollisionObjects; i++)
		{
			//D3_PROFILE("writeSingleInstanceTransformToCPU");
			CollisionObject2* colObj = rbWorld->getCollisionObjectArray()[i];
			CollisionShape* collisionShape = colObj->getCollisionShape();

			Vec3 pos = colObj->getWorldTransform().getOrigin();
			Quat orn = colObj->getWorldTransform().getRotation();
			i32 index = colObj->getUserIndex();
			if (index >= 0)
			{
				GUISyncPosition p;
				p.m_graphicsInstanceId = index;
				for (i32 q = 0; q < 4; q++)
				{
					p.m_pos[q] = pos[q];
					p.m_orn[q] = orn[q];
				}
				updatedPositions.push_back(p);
			}
		}
	}

	if (updatedPositions.size())
	{
		syncPhysicsToGraphics2(&updatedPositions[0], updatedPositions.size());
	}
}

void RemoteGUIHelper::syncPhysicsToGraphics2(const GUISyncPosition* positions, i32 numPositions)
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		uploadData((u8*)positions, numPositions * sizeof(GUISyncPosition), 0);
		cmd->m_updateFlags = 0;
		cmd->m_syncTransformsCommand.m_numPositions = numPositions;
		cmd->m_type = GFX_CMD_SYNCHRONIZE_TRANSFORMS;
		m_data->submitClientCommand(*cmd);
	}
	const GraphicsSharedMemoryStatus* status = 0;
	while ((status = m_data->processServerStatus()) == 0)
	{
	}
}

void RemoteGUIHelper::render(const DiscreteDynamicsWorld* rbWorld)
{
}

void RemoteGUIHelper::createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld)
{
}

i32 RemoteGUIHelper::uploadData(u8k* data, i32 sizeInBytes, i32 slot)
{
	i32 chunkSize = GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE;
	i32 remainingBytes = sizeInBytes;
	i32 offset = 0;
	while (remainingBytes)
	{
		Assert(remainingBytes >= 0);
		i32 curBytes = d3Min(remainingBytes, chunkSize);
		GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
		if (cmd)
		{
			for (i32 i = 0; i < curBytes; i++)
			{
				m_data->m_testBlock1->m_bulletStreamData[i] = data[i + offset];
			}

			cmd->m_updateFlags = 0;
			cmd->m_type = GFX_CMD_UPLOAD_DATA;
			cmd->m_uploadDataCommand.m_numBytes = curBytes;
			cmd->m_uploadDataCommand.m_dataOffset = offset;
			cmd->m_uploadDataCommand.m_dataSlot = slot;
			m_data->submitClientCommand(*cmd);

			const GraphicsSharedMemoryStatus* status = 0;
			while ((status = m_data->processServerStatus()) == 0)
			{
			}
			offset += curBytes;
			remainingBytes -= curBytes;
		}
	}
	return 0;
}

i32 RemoteGUIHelper::registerTexture(u8k* texels, i32 width, i32 height)
{
	i32 textureId = -1;

	//first upload all data

	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		i32 sizeInBytes = width * height * 3;  //rgb
		uploadData(texels, sizeInBytes, 0);
		cmd->m_updateFlags = 0;
		cmd->m_type = GFX_CMD_REGISTER_TEXTURE;
		cmd->m_registerTextureCommand.m_width = width;
		cmd->m_registerTextureCommand.m_height = height;
		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
		if (status->m_type == GFX_CMD_REGISTER_TEXTURE_COMPLETED)
		{
			textureId = status->m_registerTextureStatus.m_textureId;
		}
	}

	return textureId;
}

i32 RemoteGUIHelper::registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId)
{
	i32 shapeId = -1;

	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		uploadData((u8*)vertices, numvertices * 9 * sizeof(float), 0);
		uploadData((u8*)indices, numIndices * sizeof(i32), 1);
		cmd->m_type = GFX_CMD_REGISTER_GRAPHICS_SHAPE;
		cmd->m_updateFlags = 0;
		cmd->m_registerGraphicsShapeCommand.m_numVertices = numvertices;
		cmd->m_registerGraphicsShapeCommand.m_numIndices = numIndices;
		cmd->m_registerGraphicsShapeCommand.m_primitiveType = primitiveType;
		cmd->m_registerGraphicsShapeCommand.m_textureId = textureId;

		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
		if (status->m_type == GFX_CMD_REGISTER_GRAPHICS_SHAPE_COMPLETED)
		{
			shapeId = status->m_registerGraphicsShapeStatus.m_shapeId;
		}
	}

	return shapeId;
}

i32 RemoteGUIHelper::registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling)
{
	i32 graphicsInstanceId = -1;

	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_type = GFX_CMD_REGISTER_GRAPHICS_INSTANCE;
		cmd->m_updateFlags = 0;
		cmd->m_registerGraphicsInstanceCommand.m_shapeIndex = shapeIndex;
		for (i32 i = 0; i < 4; i++)
		{
			cmd->m_registerGraphicsInstanceCommand.m_position[i] = position[i];
			cmd->m_registerGraphicsInstanceCommand.m_quaternion[i] = quaternion[i];
			cmd->m_registerGraphicsInstanceCommand.m_color[i] = color[i];
			cmd->m_registerGraphicsInstanceCommand.m_scaling[i] = scaling[i];
		}
		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
		if (status->m_type == GFX_CMD_REGISTER_GRAPHICS_INSTANCE_COMPLETED)
		{
			graphicsInstanceId = status->m_registerGraphicsInstanceStatus.m_graphicsInstanceId;
		}
	}
	return graphicsInstanceId;
}

void RemoteGUIHelper::removeAllGraphicsInstances()
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_updateFlags = 0;
		cmd->m_type = GFX_CMD_REMOVE_ALL_GRAPHICS_INSTANCES;
		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
	}
}

void RemoteGUIHelper::removeGraphicsInstance(i32 graphicsUid)
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_updateFlags = 0;
		cmd->m_type = GFX_CMD_REMOVE_SINGLE_GRAPHICS_INSTANCE;
		cmd->m_removeGraphicsInstanceCommand.m_graphicsUid = graphicsUid;
		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
	}
}

void RemoteGUIHelper::changeScaling(i32 instanceUid, const double scaling[3])
{

}
void RemoteGUIHelper::changeRGBAColor(i32 instanceUid, const double rgbaColor[4])
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_updateFlags = 0;
		cmd->m_type = GFX_CMD_CHANGE_RGBA_COLOR;
		cmd->m_changeRGBAColorCommand.m_graphicsUid = instanceUid;
		for (i32 i = 0; i < 4; i++)
		{
			cmd->m_changeRGBAColorCommand.m_rgbaColor[i] = rgbaColor[i];
		}
		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
	}
}
Common2dCanvasInterface* RemoteGUIHelper::get2dCanvasInterface()
{
	return 0;
}

CommonParameterInterface* RemoteGUIHelper::getParameterInterface()
{
	return 0;
}

CommonRenderInterface* RemoteGUIHelper::getRenderInterface()
{
	return 0;
}

CommonGraphicsApp* RemoteGUIHelper::getAppInterface()
{
	return 0;
}

void RemoteGUIHelper::setUpAxis(i32 axis)
{
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	if (cmd)
	{
		cmd->m_updateFlags = 0;
		cmd->m_upAxisYCommand.m_enableUpAxisY = axis == 1;
		cmd->m_type = GFX_CMD_0;
		m_data->submitClientCommand(*cmd);
		const GraphicsSharedMemoryStatus* status = 0;
		while ((status = m_data->processServerStatus()) == 0)
		{
		}
	}
}
void RemoteGUIHelper::resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
{
}

void RemoteGUIHelper::copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
										  u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
										  float* depthBuffer, i32 depthBufferSizeInPixels,
										  i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
										  i32 startPixelIndex, i32 width, i32 height, i32* numPixelsCopied)

{
	if (numPixelsCopied)
		*numPixelsCopied = 0;
}

void RemoteGUIHelper::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
}

void RemoteGUIHelper::setProjectiveTexture(bool useProjectiveTexture)
{
}

void RemoteGUIHelper::autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld)
{
}

void RemoteGUIHelper::drawText3D(tukk txt, float posX, float posZY, float posZ, float size)
{
}

void RemoteGUIHelper::drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag)
{
}

i32 RemoteGUIHelper::addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid)
{
	return -1;
}
i32 RemoteGUIHelper::addUserDebugPoints(const double debugPointPositionXYZ[3], const double debugPointColorRGB[3], double pointSize, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid, i32 debugPointNum)
{
	return -1;
}
void RemoteGUIHelper::removeUserDebugItem(i32 debugItemUniqueId)
{
}
void RemoteGUIHelper::removeAllUserDebugItems()
{
}
