#ifndef OPENGL_GUI_HELPER_H
#define OPENGL_GUI_HELPER_H
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>

class CollisionShape;
class Transform2;
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>

struct OpenGLGuiHelper : public GUIHelperInterface
{
	struct OpenGLGuiHelperInternalData* m_data;

	OpenGLGuiHelper(struct CommonGraphicsApp* glApp, bool useOpenGL2);

	virtual ~OpenGLGuiHelper();

	virtual struct CommonRenderInterface* getRenderInterface();
	virtual const struct CommonRenderInterface* getRenderInterface() const;

	virtual void createRigidBodyGraphicsObject(RigidBody* body, const Vec3& color);

	virtual void createCollisionObjectGraphicsObject(CollisionObject2* body, const Vec3& color);

	virtual i32 registerTexture(u8k* texels, i32 width, i32 height);
	virtual i32 registerGraphicsShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId);
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling);
	virtual void removeAllGraphicsInstances();
	virtual void removeGraphicsInstance(i32 graphicsUid);
	virtual void changeInstanceFlags(i32 instanceUid, i32 flags);
	virtual void changeRGBAColor(i32 instanceUid, const double rgbaColor[4]);
	virtual void changeScaling(i32 instanceUid, const double scaling[3]);
	virtual void changeSpecularColor(i32 instanceUid, const double specularColor[3]);
	virtual void changeTexture(i32 textureUniqueId, u8k* rgbTexels, i32 width, i32 height);
	virtual void removeTexture(i32 textureUid);
	virtual i32 getShapeIndexFromInstance(i32 instanceUid);
	virtual void replaceTexture(i32 shapeIndex, i32 textureUid);
	virtual void updateShape(i32 shapeIndex, float* vertices, i32 numVertices);
	virtual void setBackgroundColor(const double rgbBackground[3]);
	virtual void createCollisionShapeGraphicsObject(CollisionShape* collisionShape);

	virtual void syncPhysicsToGraphics(const DiscreteDynamicsWorld* rbWorld);

	virtual void render(const DiscreteDynamicsWorld* rbWorld);

	virtual void createPhysicsDebugDrawer(DiscreteDynamicsWorld* rbWorld);

	virtual struct Common2dCanvasInterface* get2dCanvasInterface();

	virtual CommonParameterInterface* getParameterInterface();

	virtual struct CommonGraphicsApp* getAppInterface();

	virtual void setUpAxis(i32 axis);

	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ);
	virtual bool getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float cameraTarget[3]) const;

	virtual void copyCameraImageData(const float viewMatrix[16], const float projectionMatrix[16],
									 u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
									 float* depthBuffer, i32 depthBufferSizeInPixels,
									 i32* segmentationMaskBuffer, i32 segmentationMaskBufferSizeInPixels,
									 i32 startPixelIndex, i32 destinationWidth,
									 i32 destinationHeight, i32* numPixelsCopied);

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]);
	virtual void setProjectiveTexture(bool useProjectiveTexture);

	virtual void autogenerateGraphicsObjects(DiscreteDynamicsWorld* rbWorld);

	virtual void drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag);

	virtual void drawText3D(tukk txt, float posX, float posY, float posZ, float size);

	virtual i32 addUserDebugText3D(tukk txt, const double positionXYZ[3], const double textColorRGB[3], double size, double lifeTime)
	{
		return -1;
	}

	virtual i32 addUserDebugLine(const double debugLineFromXYZ[3], const double debugLineToXYZ[3], const double debugLineColorRGB[3], double lineWidth, double lifeTime, i32 trackingVisualShapeIndex, i32 replaceItemUid)
	{
		return -1;
	}
	virtual i32 addUserDebugParameter(tukk txt, double rangeMin, double rangeMax, double startValue)
	{
		return -1;
	}

	virtual void removeUserDebugItem(i32 debugItemUniqueId)
	{
	}
	virtual void removeAllUserDebugItems()
	{
	}

	void renderInternalGl2(i32 pass, const DiscreteDynamicsWorld* dynamicsWorld);

	void setVRMode(bool vrMode);

	void setVisualizerFlag(i32 flag, i32 enable);

	virtual void setVisualizerFlagCallback(VisualizerFlagCallback callback);

	virtual void dumpFramesToVideo(tukk mp4FileName);

	i32 createCheckeredTexture(i32 r, i32 g, i32 b);

	void computeSoftBodyVertices(CollisionShape* collisionShape,
								 AlignedObjectArray<GLInstanceVertex>& gfxVertices,
								 AlignedObjectArray<i32>& indices);
};

#endif  //OPENGL_GUI_HELPER_H
