#ifndef REMOTE_HELPER_TCP_H
#define REMOTE_HELPER_TCP_H

#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>

///a RemoteGUIHelper will connect to an existing graphics server over TCP
struct RemoteGUIHelperTCP : public GUIHelperInterface
{
	struct RemoteGUIHelperTCPInternalData* m_data;

	RemoteGUIHelperTCP(tukk hostName, i32 port);

	virtual ~RemoteGUIHelperTCP();

	virtual void setVisualizerFlag(i32 flag, i32 enable);

	virtual void createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color);

	virtual void createCollisionObjectGraphicsObject(CollisionObject2* body, const Vec3& color);

	virtual void createCollisionShapeGraphicsObject(CollisionShape* collisionShape);

	virtual bool getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float camTarget[3]) const;

	virtual void syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld);
	virtual void syncPhysicsToGraphics2(const class DiscreteDynamicsWorld* rbWorld);
	virtual void syncPhysicsToGraphics2(const GUISyncPosition* positions, i32 numPositions);

	virtual void render(const DiscreteDynamicsWorld* rbWorld);

	virtual void createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld);

	virtual i32 registerTexture(u8k* texels, i32 width, i32 height);
	virtual i32 registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId);
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling);
	virtual void removeAllGraphicsInstances();
	virtual void removeGraphicsInstance(i32 graphicsUid);
	virtual void changeRGBAColor(i32 instanceUid, const double rgbaColor[4]);

	virtual Common2dCanvasInterface* get2dCanvasInterface();

	virtual CommonParameterInterface* getParameterInterface();

	virtual CommonRenderInterface* getRenderInterface();

	virtual CommonGraphicsApp* getAppInterface();

	virtual void setUpAxis(i32 axis);
	
	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ);
	
	virtual void copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
									 u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
									 float* depthBuffer, i32 depthBufferSizeInPixels,
									 i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
									 i32 startPixelIndex, i32 width, i32 height, i32* numPixelsCopied);

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]);
	
	virtual void setProjectiveTexture(bool useProjectiveTexture);

	virtual void autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld);

	virtual void drawText3D(tukk txt, float posX, float posZY, float posZ, float size);

	virtual void drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag);

	virtual i32 addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid);
	virtual i32 addUserDebugPoints(const double debugPointPositionXYZ[3], const double debugPointColorRGB[3], double pointSize, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid, i32 debugPointNum);
	virtual void removeUserDebugItem(i32 debugItemUniqueId);
	virtual void removeAllUserDebugItems();

	i32 uploadData(u8k* data, i32 sizeInBytes, i32 slot);
	virtual bool isRemoteVisualizer() { return true; }
};

#endif  //REMOTE_HELPER_TCP_H
