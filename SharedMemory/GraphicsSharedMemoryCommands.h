#ifndef GRAPHICS_SHARED_MEMORY_COMMANDS_H
#define GRAPHICS_SHARED_MEMORY_COMMANDS_H

//this is a very experimental draft of commands. We will iterate on this API (commands, arguments etc)

#include <drx3D/SharedMemory/GraphicsSharedMemoryPublic.h>

#ifdef __GNUC__
#include <stdint.h>
typedef int32_t smInt32a_t;
typedef int64_t smInt64a_t;
typedef uint32_t smUint32a_t;
typedef uint64_t smUint64a_t;
#elif defined(_MSC_VER)
typedef __int32 smInt32a_t;
typedef __int64 smInt64a_t;
typedef unsigned __int32 smUint32a_t;
typedef unsigned __int64 smUint64a_t;
#else
typedef i32 smInt32a_t;
typedef z64 smInt64a_t;
typedef u32 smUint32a_t;
typedef zu64 smUint64a_t;
#endif

#ifdef __APPLE__
#define GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE (512 * 1024)
#else
 #define GRAPHICS_SHARED_MEMORY_MAX_STREAM_CHUNK_SIZE (4 * 1024 * 1024)
#endif

struct GraphicsCommand0
{
	i32 bla;
};

struct GraphicsUpAxisCommand
{
	i32 m_enableUpAxisY;
};

struct GraphicsStatus0
{
	i32 bla;
};

struct GraphicsVisualizerFlagCommand
{
	i32 m_visualizerFlag;
	i32 m_enable;
};

struct GraphicsUploadDataCommand
{
	i32 m_numBytes;
	i32 m_dataOffset;
	i32 m_dataSlot;
};

struct GraphicRegisterTextureCommand
{
	i32 m_width;
	i32 m_height;
};

struct GraphicsRegisterTextureStatus
{
	i32 m_textureId;
};

struct GraphicsRegisterGraphicsShapeCommand
{
	i32 m_numVertices;
	i32 m_numIndices;
	i32 m_primitiveType;
	i32 m_textureId;
};

struct GraphicsRegisterGraphicsShapeStatus
{
	i32 m_shapeId;
};

struct GraphicsRegisterGraphicsInstanceCommand
{
	
	i32 m_shapeIndex;
	float m_position[4];
	float m_quaternion[4];
	float m_color[4];
	float m_scaling[4];
};

struct GraphicsRegisterGraphicsInstanceStatus
{
	i32 m_graphicsInstanceId;
};

struct GraphicsSyncTransformsCommand
{
	i32 m_numPositions;
};

struct GraphicsRemoveInstanceCommand
{
	i32 m_graphicsUid;
};

struct GraphicsChangeRGBAColorCommand
{
	i32 m_graphicsUid;
	double m_rgbaColor[4];
};

struct GraphicsChangeScalingCommand
{
	i32 m_graphicsUid;
	double m_scaling[3];
};



struct GraphicsGetCameraInfoStatus
{
	i32 width;
	i32 height;
	float viewMatrix[16];
	float projectionMatrix[16];
	float camUp[3];
	float camForward[3];
	float hor[3];
	float vert[3];
	float yaw;
	float pitch;
	float camDist;
	float camTarget[3];
};


struct GraphicsSharedMemoryCommand
{
	i32 m_type;
	smUint64a_t m_timeStamp;
	i32 m_sequenceNumber;

	//m_updateFlags is a bit fields to tell which parameters need updating
	//for example m_updateFlags = SIM_PARAM_UPDATE_DELTA_TIME | SIM_PARAM_UPDATE_NUM_SOLVER_ITERATIONS;
	i32 m_updateFlags;

	union {
		struct GraphicsCommand0 m_graphicsCommand0;
		struct GraphicsUpAxisCommand m_upAxisYCommand;
		struct GraphicsVisualizerFlagCommand m_visualizerFlagCommand;
		struct GraphicsUploadDataCommand m_uploadDataCommand;
		struct GraphicRegisterTextureCommand m_registerTextureCommand;
		struct GraphicsRegisterGraphicsShapeCommand m_registerGraphicsShapeCommand;
		struct GraphicsRegisterGraphicsInstanceCommand m_registerGraphicsInstanceCommand;
		struct GraphicsSyncTransformsCommand m_syncTransformsCommand;
		struct GraphicsRemoveInstanceCommand m_removeGraphicsInstanceCommand;
		struct GraphicsChangeRGBAColorCommand m_changeRGBAColorCommand;
		struct GraphicsChangeScalingCommand m_changeScalingCommand;
	};
};

struct GraphicsSharedMemoryStatus
{
	i32 m_type;

	smUint64a_t m_timeStamp;
	i32 m_sequenceNumber;

	//m_streamBytes is only for internal purposes
	i32 m_numDataStreamBytes;
	tuk m_dataStream;

	//m_updateFlags is a bit fields to tell which parameters were updated,
	//m_updateFlags is ignored for most status messages
	i32 m_updateFlags;

	union {
		
		struct GraphicsStatus0 m_graphicsStatus0;
		struct GraphicsRegisterTextureStatus m_registerTextureStatus;
		struct GraphicsRegisterGraphicsShapeStatus m_registerGraphicsShapeStatus;
		struct GraphicsRegisterGraphicsInstanceStatus m_registerGraphicsInstanceStatus;
		struct GraphicsGetCameraInfoStatus m_getCameraInfoStatus;
	};
};

typedef struct GraphicsSharedMemoryStatus GraphicsSharedMemoryStatus_t;

#endif  //GRAPHICS_SHARED_MEMORY_COMMANDS_H
