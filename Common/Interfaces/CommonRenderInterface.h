#ifndef COMMON_RENDER_INTERFACE_H
#define COMMON_RENDER_INTERFACE_H

#include <drxtypes.h>

struct CommonCameraInterface;

enum
{
	D3_GL_TRIANGLES = 1,
	D3_GL_POINTS
};

enum
{
	D3_INSTANCE_TRANSPARANCY = 1,
	D3_INSTANCE_TEXTURE = 2,
	D3_INSTANCE_DOUBLE_SIDED = 4,
};

enum
{
	D3_DEFAULT_RENDERMODE = 1,
	//D3_WIREFRAME_RENDERMODE,
	D3_CREATE_SHADOWMAP_RENDERMODE,
	D3_USE_SHADOWMAP_RENDERMODE,
	D3_USE_SHADOWMAP_RENDERMODE_REFLECTION,
	D3_USE_SHADOWMAP_RENDERMODE_REFLECTION_PLANE,
	D3_USE_PROJECTIVE_TEXTURE_RENDERMODE,
	D3_SEGMENTATION_MASK_RENDERMODE,
};

struct GfxVertexFormat0
{
	float x, y, z, w;
	float unused0, unused1, unused2, unused3;
	float u, v;
};

struct GfxVertexFormat1
{
	float x, y, z, w;
	float nx, ny, nz;
	float u, v;
};

struct CommonRenderInterface
{
	virtual ~CommonRenderInterface() {}
	virtual void init() = 0;
	virtual void updateCamera(i32 upAxis) = 0;
	virtual void removeAllInstances() = 0;
	virtual void removeGraphicsInstance(i32 instanceUid) = 0;

	virtual const CommonCameraInterface* getActiveCamera() const = 0;
	virtual CommonCameraInterface* getActiveCamera() = 0;
	virtual void setActiveCamera(CommonCameraInterface* cam) = 0;

	virtual void setLightPosition(const float lightPos[3]) = 0;
	virtual void setLightPosition(const double lightPos[3]) = 0;
	virtual void setBackgroundColor(const double rgbBackground[3]) = 0;
	
	virtual void setShadowMapResolution(i32 shadowMapResolution) = 0;
	virtual void setShadowMapIntensity(double shadowMapIntensity) = 0;
	
	virtual void setShadowMapWorldSize(float worldSize) = 0;
	
	virtual void setProjectiveTextureMatrices(const float viewMatrix[16], const float projectionMatrix[16]){};
	virtual void setProjectiveTexture(bool useProjectiveTexture){};

	virtual void renderScene() = 0;
	virtual void renderSceneInternal(i32 renderMode = D3_DEFAULT_RENDERMODE){};
	virtual i32 getScreenWidth() = 0;
	virtual i32 getScreenHeight() = 0;

	virtual void resize(i32 width, i32 height) = 0;

	virtual i32 registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling) = 0;
	virtual i32 registerGraphicsInstance(i32 shapeIndex, const double* position, const double* quaternion, const double* color, const double* scaling) = 0;
	virtual void drawLines(const float* positions, const float color[4], i32 numPoints, i32 pointStrideInBytes, u32k* indices, i32 numIndices, float pointDrawSize) = 0;
	virtual void drawLine(const float from[4], const float to[4], const float color[4], float lineWidth) = 0;
	virtual void drawLine(const double from[4], const double to[4], const double color[4], double lineWidth) = 0;
	virtual void drawPoint(const float* position, const float color[4], float pointDrawSize) = 0;
	virtual void drawPoint(const double* position, const double color[4], double pointDrawSize) = 0;
	virtual void drawPoints(const float* positions, const float* colors, i32 numPoints, i32 pointStrideInBytes, float pointDrawSize) = 0;
	virtual void drawTexturedTriangleMesh(float worldPosition[3], float worldOrientation[4], const float* vertices, i32 numvertices, u32k* indices, i32 numIndices, float color[4], i32 textureIndex = -1, i32 vertexLayout = 0) = 0;

	virtual i32 registerShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType = D3_GL_TRIANGLES, i32 textureIndex = -1) = 0;
	virtual void updateShape(i32 shapeIndex, const float* vertices, i32 numVertices) = 0;

	virtual i32 registerTexture(u8k* texels, i32 width, i32 height, bool flipPixelsY = true) = 0;
	virtual void updateTexture(i32 textureIndex, u8k* texels, bool flipPixelsY = true) = 0;
	virtual void activateTexture(i32 textureIndex) = 0;
	virtual void replaceTexture(i32 shapeIndex, i32 textureIndex){};
	virtual void removeTexture(i32 textureIndex) = 0;

	virtual void setPlaneReflectionShapeIndex(i32 index) {}

	virtual i32 getShapeIndexFromInstance(i32 srcIndex) { return -1; }

	virtual bool readSingleInstanceTransformToCPU(float* position, float* orientation, i32 srcIndex) = 0;

	virtual void writeSingleInstanceTransformToCPU(const float* position, const float* orientation, i32 srcIndex) = 0;
	virtual void writeSingleInstanceTransformToCPU(const double* position, const double* orientation, i32 srcIndex) = 0;
	virtual void writeSingleInstanceColorToCPU(const float* color, i32 srcIndex) = 0;
	virtual void writeSingleInstanceColorToCPU(const double* color, i32 srcIndex) = 0;
	virtual void writeSingleInstanceScaleToCPU(const float* scale, i32 srcIndex) = 0;
	virtual void writeSingleInstanceScaleToCPU(const double* scale, i32 srcIndex) = 0;
	virtual void writeSingleInstanceSpecularColorToCPU(const double* specular, i32 srcIndex) = 0;
	virtual void writeSingleInstanceSpecularColorToCPU(const float* specular, i32 srcIndex) = 0;
	virtual void writeSingleInstanceFlagsToCPU(i32 flags, i32 srcIndex) = 0;
	
	virtual i32 getTotalNumInstances() const = 0;

	virtual void writeTransforms() = 0;

	virtual void clearZBuffer() = 0;

	//This is internal access to OpenGL3+ features, mainly used for OpenCL-OpenGL interop
	//Only the GLInstancingRenderer supports it, just return 0 otherwise.
	virtual struct GLInstanceRendererInternalData* getInternalData() = 0;
};

template <typename T>
inline i32 projectWorldCoordToScreen(T objx, T objy, T objz,
									 const T modelMatrix[16],
									 const T projMatrix[16],
									 i32k viewport[4],
									 T* winx, T* winy, T* winz)
{
	i32 i;
	T in2[4];
	T tmp[4];

	in2[0] = objx;
	in2[1] = objy;
	in2[2] = objz;
	in2[3] = T(1.0);

	for (i = 0; i < 4; i++)
	{
		tmp[i] = in2[0] * modelMatrix[0 * 4 + i] + in2[1] * modelMatrix[1 * 4 + i] +
				 in2[2] * modelMatrix[2 * 4 + i] + in2[3] * modelMatrix[3 * 4 + i];
	}

	T out[4];
	for (i = 0; i < 4; i++)
	{
		out[i] = tmp[0] * projMatrix[0 * 4 + i] + tmp[1] * projMatrix[1 * 4 + i] + tmp[2] * projMatrix[2 * 4 + i] + tmp[3] * projMatrix[3 * 4 + i];
	}

	if (out[3] == T(0.0))
		return 0;
	out[0] /= out[3];
	out[1] /= out[3];
	out[2] /= out[3];
	/* Map x, y and z to range 0-1 */
	out[0] = out[0] * T(0.5) + T(0.5);
	out[1] = out[1] * T(0.5) + T(0.5);
	out[2] = out[2] * T(0.5) + T(0.5);

	/* Map x,y to viewport */
	out[0] = out[0] * viewport[2] + viewport[0];
	out[1] = out[1] * viewport[3] + viewport[1];

	*winx = out[0];
	*winy = out[1];
	*winz = out[2];
	return 1;
}

#endif  //COMMON_RENDER_INTERFACE_H
