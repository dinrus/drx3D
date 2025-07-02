#ifndef EGL_RENDERER_VISUAL_SHAPE_CONVERTER_H
#define EGL_RENDERER_VISUAL_SHAPE_CONVERTER_H

#include <drx3D/Importers/URDF/UrdfRenderingInterface.h>

struct EGLRendererVisualShapeConverter : public UrdfRenderingInterface
{
	struct EGLRendererVisualShapeConverterInternalData* m_data;

	EGLRendererVisualShapeConverter();

	virtual ~EGLRendererVisualShapeConverter();

	virtual i32 convertVisualShapes(i32 linkIndex, tukk pathPrefix, const Transform2& localInertiaFrame, const UrdfLink* linkPtr, const UrdfModel* model, i32 orgGraphicsUniqueId, i32 bodyUniqueId, struct CommonFileIOInterface* fileIO);

	virtual i32 getNumVisualShapes(i32 bodyUniqueId);

	virtual i32 getVisualShapesData(i32 bodyUniqueId, i32 shapeIndex, struct b3VisualShapeData* shapeData);

	virtual i32 registerShapeAndInstance(const b3VisualShapeData& visualShape, const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureId, i32 orgGraphicsUniqueId, i32 bodyUniqueId, i32 linkIndex);

	virtual void updateShape(i32 shapeUniqueId, const Vec3* vertices, i32 numVertices, const Vec3* normals, i32 numNormals);

	virtual void changeRGBAColor(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, const double rgbaColor[4]);

	virtual void changeInstanceFlags(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex, i32 flags);

	virtual void changeShapeTexture(i32 objectUniqueId, i32 linkIndex, i32 shapeIndex, i32 textureUniqueId);

	virtual void removeVisualShape(i32 shapeUid);

	virtual void setUpAxis(i32 axis);

	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX, float camPosY, float camPosZ);

	virtual void clearBuffers(struct TGAColor& clearColor);

	virtual void resetAll();

	virtual void getWidthAndHeight(i32& width, i32& height);
	virtual void setWidthAndHeight(i32 width, i32 height);
	virtual void setLightDirection(float x, float y, float z);
	virtual void setLightColor(float x, float y, float z);
	virtual void setLightDistance(float dist);
	virtual void setLightAmbientCoeff(float ambientCoeff);
	virtual void setLightDiffuseCoeff(float diffuseCoeff);
	virtual void setLightSpecularCoeff(float specularCoeff);
	virtual void setShadow(bool hasShadow);
	virtual void setFlags(i32 flags);

	virtual void copyCameraImageData(u8* pixelsRGBA, i32 rgbaBufferSizeInPixels, float* depthBuffer, i32 depthBufferSizeInPixels, i32* segmentationMaskBuffer, i32 segmentationMaskSizeInPixels, i32 startPixelIndex, i32* widthPtr, i32* heightPtr, i32* numPixelsCopied);
	void copyCameraImageDataGL(u8* pixelsRGBA, i32 rgbaBufferSizeInPixels, float* depthBuffer, i32 depthBufferSizeInPixels, i32* segmentationMaskBuffer, i32 segmentationMaskSizeInPixels, i32 startPixelIndex, i32* widthPtr, i32* heightPtr, i32* numPixelsCopied);

	virtual void render();
	virtual void render(const float viewMat[16], const float projMat[16]);

	virtual i32 loadTextureFile(tukk filename, struct CommonFileIOInterface* fileIO);
	virtual i32 registerTexture(u8* texels, i32 width, i32 height);
	virtual i32 registerTextureInternal(u8* texels, i32 width, i32 height);
	

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]);
	virtual void setProjectiveTexture(bool useProjectiveTexture);

	virtual void syncTransform(i32 shapeUid, const class Transform2& worldTransform, const class Vec3& localScaling);

	virtual void mouseMoveCallback(float x, float y);
	virtual void mouseButtonCallback(i32 button, i32 state, float x, float y);

	virtual bool getCameraInfo(i32* width, i32* height, float viewMatrix[16], float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3], float vert[3], float* yaw, float* pitch, float* camDist, float cameraTarget[3]) const;
	

};

#endif  //EGL_RENDERER_VISUAL_SHAPE_CONVERTER_H
