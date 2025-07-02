#ifndef GL_INSTANCING_RENDERER_H
#define GL_INSTANCING_RENDERER_H

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include "SimpleCamera.h"

class GLInstancingRenderer : public CommonRenderInterface
{
	b3AlignedObjectArray<struct b3GraphicsInstance*> m_graphicsInstances;

	struct InternalDataRenderer* m_data;

	bool m_textureenabled;
	bool m_textureinitialized;

	i32 m_screenWidth;
	i32 m_screenHeight;

	i32 m_upAxis;

	i32 m_planeReflectionShapeIndex;

	i32 registerGraphicsInstanceInternal(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling);
	void rebuildGraphicsInstances();

public:
	GLInstancingRenderer(i32 m_maxObjectCapacity, i32 maxShapeCapacityInBytes = 56 * 1024 * 1024);
	virtual ~GLInstancingRenderer();

	virtual void init();

	virtual void renderScene();
	virtual void renderSceneInternal(i32 orgRenderMode = D3_DEFAULT_RENDERMODE);

	void InitShaders();
	void CleanupShaders();
	virtual void removeAllInstances();
	virtual void removeGraphicsInstance(i32 instanceUid);

	virtual void updateShape(i32 shapeIndex, const float* vertices, i32 numVertices);

	///vertices must be in the format x,y,z, nx,ny,nz, u,v
	virtual i32 registerShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType = D3_GL_TRIANGLES, i32 textureIndex = -1);

	virtual i32 registerTexture(u8k* texels, i32 width, i32 height, bool flipPixelsY = true);
	virtual void updateTexture(i32 textureIndex, u8k* texels, bool flipPixelsY = true);
	virtual void activateTexture(i32 textureIndex);
	virtual void replaceTexture(i32 shapeIndex, i32 textureId);
	virtual i32 getShapeIndexFromInstance(i32 srcIndex);
	virtual void removeTexture(i32 textureIndex);

	///position x,y,z, quaternion x,y,z,w, color r,g,b,a, scaling x,y,z
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling);
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const double* position, const double* quaternion, const double* color, const double* scaling);

	void writeTransforms();

	virtual bool readSingleInstanceTransformToCPU(float* position, float* orientation, i32 srcIndex);

	virtual void writeSingleInstanceTransformToCPU(const float* position, const float* orientation, i32 srcIndex);
	virtual void writeSingleInstanceTransformToCPU(const double* position, const double* orientation, i32 srcIndex)
	{
		float pos[4];
		float orn[4];
		pos[0] = (float)position[0];
		pos[1] = (float)position[1];
		pos[2] = (float)position[2];
		pos[3] = (float)position[3];
		orn[0] = (float)orientation[0];
		orn[1] = (float)orientation[1];
		orn[2] = (float)orientation[2];
		orn[3] = (float)orientation[3];
		writeSingleInstanceTransformToCPU(pos, orn, srcIndex);
	}

	virtual void readSingleInstanceTransformFromCPU(i32 srcIndex, float* position, float* orientation);

	virtual void writeSingleInstanceTransformToGPU(float* position, float* orientation, i32 srcIndex);

	virtual void writeSingleInstanceColorToCPU(const float* color, i32 srcIndex);
	virtual void writeSingleInstanceColorToCPU(const double* color, i32 srcIndex);
	virtual void writeSingleInstanceFlagsToCPU(i32 flags, i32 srcIndex2);

	virtual void writeSingleInstanceSpecularColorToCPU(const double* specular, i32 srcIndex2);
	virtual void writeSingleInstanceSpecularColorToCPU(const float* specular, i32 srcIndex2);

	virtual void writeSingleInstanceScaleToCPU(const float* scale, i32 srcIndex);
	virtual void writeSingleInstanceScaleToCPU(const double* scale, i32 srcIndex);

	virtual struct GLInstanceRendererInternalData* getInternalData();

	virtual void drawLine(const float from[4], const float to[4], const float color[4], float lineWidth = 1);
	virtual void drawLine(const double from[4], const double to[4], const double color[4], double lineWidth = 1);
	virtual void drawLines(const float* positions, const float color[4], i32 numPoints, i32 pointStrideInBytes, u32k* indices, i32 numIndices, float pointDrawSize);
	virtual void drawPoints(const float* positions, const float color[4], i32 numPoints, i32 pointStrideInBytes, float pointDrawSize);
	virtual void drawPoint(const float* position, const float color[4], float pointSize = 1);
	virtual void drawPoint(const double* position, const double color[4], double pointDrawSize = 1);
	virtual void drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, i32 numvertices, u32k* indices, i32 numIndices, float color[4], i32 textureIndex = -1, i32 vertexLayout = 0);

	virtual void updateCamera(i32 upAxis = 1);

	virtual const CommonCameraInterface* getActiveCamera() const;
	virtual CommonCameraInterface* getActiveCamera();
	virtual void setActiveCamera(CommonCameraInterface* cam);

	virtual void setLightPosition(const float lightPos[3]);
	virtual void setLightPosition(const double lightPos[3]);
	virtual void setShadowMapResolution(i32 shadowMapResolution);
	virtual void setShadowMapIntensity(double shadowMapIntensity);
	virtual void setShadowMapWorldSize(float worldSize);
	void setLightSpecularIntensity(const float lightSpecularIntensity[3]);
	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]);
	virtual void setProjectiveTexture(bool useProjectiveTexture);
	virtual void setBackgroundColor(const double rgbBackground[3]);
	virtual void resize(i32 width, i32 height);
	virtual i32 getScreenWidth()
	{
		return m_screenWidth;
	}
	virtual i32 getScreenHeight()
	{
		return m_screenHeight;
	}

	virtual i32 getMaxShapeCapacity() const;

	virtual i32 getInstanceCapacity() const;

	virtual i32 getTotalNumInstances() const;

	virtual void enableShadowMap();

	virtual void setPlaneReflectionShapeIndex(i32 index);

	virtual void clearZBuffer();

	virtual void setRenderFrameBuffer(u32 renderFrameBuffer);
};

#endif  //GL_INSTANCING_RENDERER_H
