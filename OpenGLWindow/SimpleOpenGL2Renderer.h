
#ifndef SIMPLE_OPENGL2_RENDERER_H
#define SIMPLE_OPENGL2_RENDERER_H

#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include "SimpleCamera.h"

class SimpleOpenGL2Renderer : public CommonRenderInterface
{
	struct SimpleOpenGL2RendererInternalData* m_data;

	void drawSceneInternal(i32 pass, i32 cameraUpAxis);
	void drawOpenGL(i32 instanceIndex);

public:
	SimpleOpenGL2Renderer(i32 width, i32 height);
	virtual ~SimpleOpenGL2Renderer();

	virtual void init();

	virtual void updateCamera(i32 upAxis);

	virtual const CommonCameraInterface* getActiveCamera() const;
	virtual CommonCameraInterface* getActiveCamera();
	virtual void setActiveCamera(CommonCameraInterface* cam);

	virtual void setLightPosition(const float lightPos[3]);
	virtual void setLightPosition(const double lightPos[3]);
	virtual void setShadowMapResolution(i32 shadowMapResolution) {}
	virtual void setShadowMapIntensity(double shadowMapIntensity) {}
	virtual void setShadowMapWorldSize(float worldSize) {}
	virtual void resize(i32 width, i32 height);
	virtual void setBackgroundColor(const double rgbBackground[3]) {}

	virtual void removeAllInstances();
	virtual void removeGraphicsInstance(i32 instanceUid);

	virtual bool readSingleInstanceTransformToCPU(float* position, float* orientation, i32 srcIndex);
	virtual void writeSingleInstanceColorToCPU(const float* color, i32 srcIndex);
	virtual void writeSingleInstanceColorToCPU(const double* color, i32 srcIndex);
	virtual void writeSingleInstanceScaleToCPU(const float* scale, i32 srcIndex);
	virtual void writeSingleInstanceScaleToCPU(const double* scale, i32 srcIndex);
	virtual void writeSingleInstanceSpecularColorToCPU(const double* specular, i32 srcIndex) {}
	virtual void writeSingleInstanceSpecularColorToCPU(const float* specular, i32 srcIndex) {}
	virtual void writeSingleInstanceFlagsToCPU(i32 flags, i32 srcIndex) {}
	virtual void getCameraViewMatrix(float viewMat[16]) const;
	virtual void getCameraProjectionMatrix(float projMat[16]) const;
	virtual void drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, i32 numvertices, u32k* indices, i32 numIndices, float color[4], i32 textureIndex = -1, i32 vertexLayout = 0)
	{
	}

	virtual void renderScene();

	virtual i32 getScreenWidth();
	virtual i32 getScreenHeight();
	virtual i32 registerTexture(u8k* texels, i32 width, i32 height, bool flipTexelsY);
	virtual void updateTexture(i32 textureIndex, u8k* texels, bool flipTexelsY);
	virtual void activateTexture(i32 textureIndex);
	virtual void removeTexture(i32 textureIndex);

	virtual i32 registerGraphicsInstance(i32 shapeIndex, const double* position, const double* quaternion, const double* color, const double* scaling);

	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling);

	virtual void drawLines(const float* positions, const float color[4], i32 numPoints, i32 pointStrideInBytes, u32k* indices, i32 numIndices, float pointDrawSize);

	virtual void drawLine(const float from[4], const float to[4], const float color[4], float lineWidth);

	virtual i32 registerShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType = D3_GL_TRIANGLES, i32 textureIndex = -1);

	virtual void writeSingleInstanceTransformToCPU(const float* position, const float* orientation, i32 srcIndex);

	virtual void writeSingleInstanceTransformToCPU(const double* position, const double* orientation, i32 srcIndex);

	virtual i32 getTotalNumInstances() const;

	virtual void writeTransforms();

	virtual void drawLine(const double from[4], const double to[4], const double color[4], double lineWidth);

	virtual void drawPoint(const float* position, const float color[4], float pointDrawSize);

	virtual void drawPoint(const double* position, const double color[4], double pointDrawSize);

	virtual void drawPoints(const float* positions, const float* colors, i32 numPoints, i32 pointStrideInBytes, float pointDrawSize);

	virtual void updateShape(i32 shapeIndex, const float* vertices, i32 numVertices);

	virtual void clearZBuffer();

	virtual struct GLInstanceRendererInternalData* getInternalData()
	{
		return 0;
	}
};
#endif  //SIMPLE_OPENGL2_RENDERER_H
