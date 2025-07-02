#ifndef GUI_HELPER_INTERFACE_H
#define GUI_HELPER_INTERFACE_H

#include <drxtypes.h>

class RigidBody;
class Vec3;
class CollisionObject2;
class DiscreteDynamicsWorld;
class CollisionShape;
struct Common2dCanvasInterface;
struct CommonParameterInterface;
struct CommonRenderInterface;
struct CommonGraphicsApp;

struct GUISyncPosition
{
	i32 m_graphicsInstanceId;
	float m_pos[4];
	float m_orn[4];
};

typedef void (*VisualizerFlagCallback)(i32 flag, bool enable);

///The drx3D 2 GraphicsPhysicsBridge let's the graphics engine create graphics representation and synchronize
struct GUIHelperInterface
{
	virtual ~GUIHelperInterface() {}

	virtual void createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color) = 0;

	virtual void createCollisionObjectGraphicsObject(CollisionObject2* obj, const Vec3& color) = 0;

	virtual void createCollisionShapeGraphicsObject(CollisionShape* collisionShape) = 0;

	virtual void syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld) = 0;
	virtual void syncPhysicsToGraphics2(const DiscreteDynamicsWorld* rbWorld) {}
	virtual void syncPhysicsToGraphics2(const GUISyncPosition* positions, i32 numPositions) {}

	virtual void render(const DiscreteDynamicsWorld* rbWorld) = 0;

	virtual void createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld) = 0;

	virtual i32 registerTexture(u8k* texels, i32 width, i32 height) = 0;
	virtual i32 registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId) = 0;
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) = 0;
	virtual void removeAllGraphicsInstances() = 0;
	virtual void removeGraphicsInstance(i32 graphicsUid) {}
	virtual void changeInstanceFlags(i32 instanceUid, i32 flags) {}
	virtual void changeRGBAColor(i32 instanceUid, const double rgbaColor[4]) {}
	virtual void changeScaling(i32 instanceUid, const double scaling[3]) {}
	virtual void changeSpecularColor(i32 instanceUid, const double specularColor[3]) {}
	virtual void changeTexture(i32 textureUniqueId, u8k* rgbTexels, i32 width, i32 height) {}
	virtual void updateShape(i32 shapeIndex, float* vertices, i32 numVertices) {}
	virtual i32 getShapeIndexFromInstance(i32 instanceUid) { return -1; }
	virtual void replaceTexture(i32 shapeIndex, i32 textureUid) {}
	virtual void removeTexture(i32 textureUid) {}
	virtual void setBackgroundColor(const double rgbBackground[3]) {}


	virtual Common2dCanvasInterface* get2dCanvasInterface() = 0;

	virtual CommonParameterInterface* getParameterInterface() = 0;

	virtual CommonRenderInterface* getRenderInterface() = 0;

	virtual const CommonRenderInterface* getRenderInterface() const
	{
		return 0;
	}

	virtual CommonGraphicsApp* getAppInterface() = 0;

	virtual void setUpAxis(i32 axis) = 0;

	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ) = 0;

	virtual bool getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const
	{
		return false;
	}

	virtual void setVisualizerFlag(i32 flag, i32 enable){};

	virtual void copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
									 u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
									 float* depthBuffer, i32 depthBufferSizeInPixels,
									 i32 startPixelIndex, i32 destinationWidth, i32 destinationHeight, i32* numPixelsCopied)
	{
		copyCameraImageData(viewMatrix, projectionMatrix, pixelsRGBA, rgbaBufferSizeInPixels,
							depthBuffer, depthBufferSizeInPixels,
							0, 0,
							startPixelIndex, destinationWidth,
							destinationHeight, numPixelsCopied);
	}

	virtual void copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
									 u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
									 float* depthBuffer, i32 depthBufferSizeInPixels,
									 i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
									 i32 startPixelIndex, i32 destinationWidth, i32 destinationHeight, i32* numPixelsCopied) = 0;
	virtual void debugDisplayCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
											 u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
											 float* depthBuffer, i32 depthBufferSizeInPixels,
											 i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
											 i32 startPixelIndex, i32 destinationWidth, i32 destinationHeight, i32* numPixelsCopied) {}

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]) {}
	virtual void setProjectiveTexture(bool useProjectiveTexture) {}

	virtual void autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld) = 0;

	virtual void drawText3D(tukk txt, float posX, float posY, float posZ, float size) {}
	virtual void drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag) {}

	virtual i32 addUserDebugText3D(tukk txt, const double positionXYZ[3], const double orientation[4], const double textColorRGB[3], double size, double lifeTime, i32 trackingVisualShapeIndex, i32 optionFlags, i32 replaceItemUid) { return -1; }
	virtual i32 addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid) { return -1; };
	virtual i32 addUserDebugPoints(const double debugPointPositionXYZ[], const double debugPointColorRGB[3], double pointSize, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid, i32 debugPointNum) { return -1; };
	virtual i32 addUserDebugParameter(tukk txt, double rangeMin, double rangeMax, double startValue) { return -1; };
	virtual i32 readUserDebugParameter(i32 itemUniqueId, double* value) { return 0; }

	virtual void removeUserDebugItem(i32 debugItemUniqueId){};
	virtual void removeAllUserDebugItems(){};
	virtual void removeAllUserParameters() {};

	virtual void setVisualizerFlagCallback(VisualizerFlagCallback callback) {}

	//empty name stops dumping video
	virtual void dumpFramesToVideo(tukk mp4FileName){};
	virtual void drawDebugDrawerLines(){}
	virtual void clearLines(){}
	virtual bool isRemoteVisualizer() { return false; }
};

///the DummyGUIHelper does nothing, so we can test the examples without GUI/graphics (in 'console mode')
struct DummyGUIHelper : public GUIHelperInterface
{
	DummyGUIHelper()
	{

	}
	virtual ~DummyGUIHelper() {}

	virtual void createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color) {}

	virtual void createCollisionObjectGraphicsObject(CollisionObject2* obj, const Vec3& color) {}

	virtual void createCollisionShapeGraphicsObject(CollisionShape* collisionShape) {}

	virtual void syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld) {}

	virtual void render(const DiscreteDynamicsWorld* rbWorld) {}

	virtual void createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld) {}

	virtual i32 registerTexture(u8k* texels, i32 width, i32 height) { return -1; }
	virtual i32 registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId) { return -1; }
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) { return -1; }
	virtual void removeAllGraphicsInstances() {}
	virtual void removeGraphicsInstance(i32 graphicsUid) {}
	virtual void changeRGBAColor(i32 instanceUid, const double rgbaColor[4]) {}
	virtual void changeScaling(i32 instanceUid, const double scaling[3]) {}

	virtual Common2dCanvasInterface* get2dCanvasInterface()
	{
		return 0;
	}

	virtual CommonParameterInterface* getParameterInterface()
	{
		return 0;
	}

	virtual CommonRenderInterface* getRenderInterface()
	{
		return 0;
	}

	virtual CommonGraphicsApp* getAppInterface()
	{
		return 0;
	}

	virtual void setUpAxis(i32 axis)
	{
	}
	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ)
	{
	}

	virtual void copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
									 u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
									 float* depthBuffer, i32 depthBufferSizeInPixels,
									 i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
									 i32 startPixelIndex, i32 width, i32 height, i32* numPixelsCopied)

	{
		if (numPixelsCopied)
			*numPixelsCopied = 0;
	}

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16])
	{
	}

	virtual void setProjectiveTexture(bool useProjectiveTexture)
	{
	}

	virtual void autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld)
	{
	}

	virtual void drawText3D(tukk txt, float posX, float posZY, float posZ, float size)
	{
	}

	virtual void drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag)
	{
	}

	virtual i32 addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid)
	{
		return -1;
	}
	virtual i32 addUserDebugPoints(const double debugPointPositionXYZ[3], const double debugPointColorRGB[3], double pointSize, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid, i32 debugPointNum)
	{
		return -1;
	};
	virtual void removeUserDebugItem(i32 debugItemUniqueId)
	{
	}
	virtual void removeAllUserDebugItems()
	{
	}
};

#endif  //GUI_HELPER_INTERFACE_H
