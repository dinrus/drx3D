#include "../RemoteGUIHelperTCP.h"

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Common/b3Logging.h>
#include "../GraphicsSharedMemoryCommands.h"

#include "../GraphicsSharedMemoryBlock.h"

#include <drx3D/Common/b3Scalar.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>

#include <drx3D/Network2/ActiveSocket.h>
#include <string>
static u32 b3DeserializeInt3(u8k* input)
{
	u32 tmp = (input[3] << 24) + (input[2] << 16) + (input[1] << 8) + input[0];
	return tmp;
}
static bool gVerboseNetworkMessagesClient3 = true;//false;

tukk cmd2txt[]=
{
	"GFX_CMD_INVALID",
	"GFX_CMD_0",
	"GFX_CMD_SET_VISUALIZER_FLAG",
	"GFX_CMD_UPLOAD_DATA",
	"GFX_CMD_REGISTER_TEXTURE",
	"GFX_CMD_REGISTER_GRAPHICS_SHAPE",
	"GFX_CMD_REGISTER_GRAPHICS_INSTANCE",
	"GFX_CMD_SYNCHRONIZE_TRANSFORMS",
	"GFX_CMD_REMOVE_ALL_GRAPHICS_INSTANCES",
	"GFX_CMD_REMOVE_SINGLE_GRAPHICS_INSTANCE",
	"GFX_CMD_CHANGE_RGBA_COLOR",
	"GFX_CMD_GET_CAMERA_INFO",
	//don't go beyond this command!
	"GFX_CMD_MAX_CLIENT_COMMANDS",
};


struct RemoteGUIHelperTCPInternalData
{
	//	GUIHelperInterface* m_guiHelper;
	bool m_waitingForServer;
	STxt m_hostName;
	i32 m_port;
	
	GraphicsSharedMemoryStatus m_lastServerStatus;
	CActiveSocket m_tcpSocket;
	bool m_isConnected;
	b3AlignedObjectArray<u8> m_tempBuffer;
	GraphicsSharedMemoryStatus m_lastStatus;
	GraphicsSharedMemoryCommand m_command;
	double m_timeOutInSeconds;
	b3AlignedObjectArray<char> m_stream;

	RemoteGUIHelperTCPInternalData(tukk hostName, i32 port)
		: m_waitingForServer(false),
		m_hostName(hostName),
		m_port(port),
		m_timeOutInSeconds(60.)
	{
		m_isConnected = false;
		connect();
		
	}

	virtual ~RemoteGUIHelperTCPInternalData()
	{
		disconnect();
		
	}

	virtual bool isConnected()
	{
		return m_isConnected;
	}

	bool canSubmitCommand() const
	{
		if (m_isConnected && !m_waitingForServer)
		{
			return true;
		}
		return false;
	}

	struct GraphicsSharedMemoryCommand* getAvailableSharedMemoryCommand()
	{
		static i32 sequence = 0;
		m_command.m_sequenceNumber = sequence++;
		return &m_command;
	}

	bool submitClientCommand(const GraphicsSharedMemoryCommand& command)
	{
		if (gVerboseNetworkMessagesClient3)
			printf("submitClientCommand: %d %s\n", command.m_type, cmd2txt[command.m_type]);
		/// at the moment we allow a maximum of 1 outstanding command, so we check for this
		// once the server processed the command and returns a status, we clear the flag
		// "m_data->m_waitingForServer" and allow submitting the next command
		Assert(!m_waitingForServer);
		if (!m_waitingForServer)
		{
			i32 sz = 0;
			u8* data = 0;
			m_tempBuffer.clear();
			sz = sizeof(GraphicsSharedMemoryCommand);
			data = (u8*)&command;
			//printf("submit command of type %d\n", command.m_type);
			m_tcpSocket.Send((u8k*)data, sz);
			m_waitingForServer = true;
			return true;
		}
		return false;
	}

	const GraphicsSharedMemoryStatus* processServerStatus()
	{

		bool hasStatus = false;

		//i32 serviceResult = enet_host_service(m_client, &m_event, 0);
		i32 maxLen = 4 + sizeof(GraphicsSharedMemoryStatus) + GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE;

		i32 rBytes = m_tcpSocket.Receive(maxLen);
		if (rBytes <= 0)
			return 0;

		//append to tmp buffer
		//recBytes
		
		u8* d2 = (u8*)m_tcpSocket.GetData();

		i32 curSize = m_tempBuffer.size();
		m_tempBuffer.resize(curSize + rBytes);
		for (i32 i = 0; i < rBytes; i++)
		{
			m_tempBuffer[curSize + i] = d2[i];
		}

		i32 packetSizeInBytes = -1;

		if (m_tempBuffer.size() >= 4)
		{
			packetSizeInBytes = b3DeserializeInt3(&m_tempBuffer[0]);
		}

		if (m_tempBuffer.size() == packetSizeInBytes)
		{
			u8* data = &m_tempBuffer[0];
			if (gVerboseNetworkMessagesClient3)
			{
				printf("A packet of length %d bytes received\n", m_tempBuffer.size());
			}

			hasStatus = true;
			GraphicsSharedMemoryStatus* statPtr = (GraphicsSharedMemoryStatus*)&data[4];
#if 0
			if (statPtr->m_type == CMD_STEP_FORWARD_SIMULATION_COMPLETED)
			{
				GraphicsSharedMemoryStatus dummy;
				dummy.m_type = CMD_STEP_FORWARD_SIMULATION_COMPLETED;
				m_lastStatus = dummy;
				m_stream.resize(0);
			}
			else
#endif
			{
				m_lastStatus = *statPtr;
				i32 streamOffsetInBytes = 4 + sizeof(GraphicsSharedMemoryStatus);
				i32 numStreamBytes = packetSizeInBytes - streamOffsetInBytes;
				m_stream.resize(numStreamBytes);
				for (i32 i = 0; i < numStreamBytes; i++)
				{
					m_stream[i] = data[i + streamOffsetInBytes];
				}
			}
			m_tempBuffer.clear();
			m_waitingForServer = false;
			if (gVerboseNetworkMessagesClient3)
				printf("processServerStatus: %d\n", m_lastStatus.m_type);
			return &m_lastStatus;
		}

		return 0;
	}

	bool connect()
	{
		if (m_isConnected)
			return true;

		m_tcpSocket.Initialize();
    
		m_isConnected = m_tcpSocket.Open(m_hostName.c_str(), m_port);
		if (m_isConnected)
		{
			m_tcpSocket.SetSendTimeout(m_timeOutInSeconds, 0);
			m_tcpSocket.SetReceiveTimeout(m_timeOutInSeconds, 0);
			
		}
		i32 key = GRAPHICS_SHARED_MEMORY_MAGIC_NUMBER;
		m_tcpSocket.Send((u8*)&key, 4);
		m_tcpSocket.SetBlocking();
		return m_isConnected;
	
	}

	void disconnect()
	{
		const char msg[16] = "disconnect";
		m_tcpSocket.Send((u8k*)msg, 10);
		m_tcpSocket.Close();
		m_isConnected = false;
	}
};

RemoteGUIHelperTCP::RemoteGUIHelperTCP(tukk hostName, i32 port)
{
	m_data = new RemoteGUIHelperTCPInternalData(hostName, port);
	if (m_data->canSubmitCommand())
	{
		removeAllGraphicsInstances();
	}
}

RemoteGUIHelperTCP::~RemoteGUIHelperTCP()
{
	delete m_data;
}

void RemoteGUIHelperTCP::setVisualizerFlag(i32 flag, i32 enable)
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

void RemoteGUIHelperTCP::createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color)
{
	printf("todo: createRigidBodyGraphicsObject\n");
}

bool RemoteGUIHelperTCP::getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const
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

void RemoteGUIHelperTCP::createCollisionObjectGraphicsObject(CollisionObject2* body, const Vec3& color)
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

void RemoteGUIHelperTCP::createCollisionShapeGraphicsObject(CollisionShape* collisionShape)
{
	printf("todo; createCollisionShapeGraphicsObject\n");
}

void RemoteGUIHelperTCP::syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld)
{
}

void RemoteGUIHelperTCP::syncPhysicsToGraphics2(const DiscreteDynamicsWorld* rbWorld)
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

void RemoteGUIHelperTCP::syncPhysicsToGraphics2(const GUISyncPosition* positions, i32 numPositions)
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

void RemoteGUIHelperTCP::render(const DiscreteDynamicsWorld* rbWorld)
{
}

void RemoteGUIHelperTCP::createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld)
{
}

i32 RemoteGUIHelperTCP::uploadData(u8k* data, i32 sizeInBytes, i32 slot)
{
	i32 chunkSize = 1024;// GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE;
	i32 remainingBytes = sizeInBytes;
	i32 offset = 0;
	
	GraphicsSharedMemoryCommand* cmd = m_data->getAvailableSharedMemoryCommand();
	cmd->m_updateFlags = 0;
	cmd->m_type = GFX_CMD_UPLOAD_DATA;
	cmd->m_uploadDataCommand.m_numBytes = sizeInBytes;
	cmd->m_uploadDataCommand.m_dataOffset = offset;
	cmd->m_uploadDataCommand.m_dataSlot = slot;
	m_data->submitClientCommand(*cmd);

	


	const GraphicsSharedMemoryStatus* status = 0;
	while ((status = m_data->processServerStatus()) == 0)
	{
	}

	while (remainingBytes > 0)
	{
		i32 curBytes = d3Min(remainingBytes, chunkSize);
		m_data->m_tcpSocket.Send((u8k*)data+offset, curBytes);
		if (gVerboseNetworkMessagesClient3)
			printf("sending %d bytes\n", curBytes);
		remainingBytes -= curBytes;
		offset += curBytes;
	}
	if (gVerboseNetworkMessagesClient3)
		printf("send all bytes!\n");

	status = 0;
	while ((status = m_data->processServerStatus()) == 0)
	{
	}
	return 0;
}

i32 RemoteGUIHelperTCP::registerTexture(u8k* texels, i32 width, i32 height)
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

i32 RemoteGUIHelperTCP::registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId)
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

i32 RemoteGUIHelperTCP::registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling)
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

void RemoteGUIHelperTCP::removeAllGraphicsInstances()
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

void RemoteGUIHelperTCP::removeGraphicsInstance(i32 graphicsUid)
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
void RemoteGUIHelperTCP::changeRGBAColor(i32 instanceUid, const double rgbaColor[4])
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
Common2dCanvasInterface* RemoteGUIHelperTCP::get2dCanvasInterface()
{
	return 0;
}

CommonParameterInterface* RemoteGUIHelperTCP::getParameterInterface()
{
	return 0;
}

CommonRenderInterface* RemoteGUIHelperTCP::getRenderInterface()
{
	return 0;
}

CommonGraphicsApp* RemoteGUIHelperTCP::getAppInterface()
{
	return 0;
}

void RemoteGUIHelperTCP::setUpAxis(i32 axis)
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
void RemoteGUIHelperTCP::resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
{
}

void RemoteGUIHelperTCP::copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
										  u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
										  float* depthBuffer, i32 depthBufferSizeInPixels,
										  i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
										  i32 startPixelIndex, i32 width, i32 height, i32* numPixelsCopied)

{
	if (numPixelsCopied)
		*numPixelsCopied = 0;
}

void RemoteGUIHelperTCP::setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
{
}

void RemoteGUIHelperTCP::setProjectiveTexture(bool useProjectiveTexture)
{
}

void RemoteGUIHelperTCP::autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld)
{
}

void RemoteGUIHelperTCP::drawText3D(tukk txt, float posX, float posZY, float posZ, float size)
{
}

void RemoteGUIHelperTCP::drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag)
{
}

i32 RemoteGUIHelperTCP::addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid)
{
	return -1;
}
i32 RemoteGUIHelperTCP::addUserDebugPoints(const double debugPointPositionXYZ[3], const double debugPointColorRGB[3], double pointSize, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid, i32 debugPointNum)
{
	return -1;
}
void RemoteGUIHelperTCP::removeUserDebugItem(i32 debugItemUniqueId)
{
}
void RemoteGUIHelperTCP::removeAllUserDebugItems()
{
}
