#ifndef TINY_RENDERER_H
#define TINY_RENDERER_H

#include "geometry.h"
#include "model.h"
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Vec3.h>

#include "tgaimage.h"

struct TinyRenderObjectData
{
	//Camera
	TinyRender::Matrix m_viewMatrix;
	TinyRender::Matrix m_projectionMatrix;
	TinyRender::Matrix m_viewportMatrix;
	Vec3 m_localScaling;
	Vec3 m_lightDirWorld;
	Vec3 m_lightColor;
	float m_lightDistance;
	float m_lightAmbientCoeff;
	float m_lightDiffuseCoeff;
	float m_lightSpecularCoeff;

	//Model (vertices, indices, textures, shader)
	TinyRender::Matrix m_modelMatrix;
	TinyRender::Model* m_model;
	//class IShader* m_shader; todo(erwincoumans) expose the shader, for now we use a default shader

	//Output

	TGAImage& m_rgbColorBuffer;
	b3AlignedObjectArray<float>& m_depthBuffer;              //required, hence a reference
	b3AlignedObjectArray<float>* m_shadowBuffer;             //optional, hence a pointer
	b3AlignedObjectArray<i32>* m_segmentationMaskBufferPtr;  //optional, hence a pointer

	TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer);
	TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer, b3AlignedObjectArray<i32>* segmentationMaskBuffer, i32 objectIndex);
	TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer, b3AlignedObjectArray<float>* shadowBuffer);
	TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer, b3AlignedObjectArray<float>* shadowBuffer, b3AlignedObjectArray<i32>* segmentationMaskBuffer, i32 objectIndex, i32 linkIndex);
	virtual ~TinyRenderObjectData();

	void loadModel(tukk fileName, struct CommonFileIOInterface* fileIO);
	void createCube(float HalfExtentsX, float HalfExtentsY, float HalfExtentsZ, struct CommonFileIOInterface* fileIO=0);
	void registerMeshShape(const float* vertices, i32 numVertices, i32k* indices, i32 numIndices, const float rgbaColor[4],
						   u8* textureImage = 0, i32 textureWidth = 0, i32 textureHeight = 0);

	void registerMesh2(AlignedObjectArray<Vec3>& vertices, AlignedObjectArray<Vec3>& normals, AlignedObjectArray<i32>& indices, struct CommonFileIOInterface* fileIO);

	uk m_userData;
	i32 m_userIndex;
	i32 m_objectIndex;
	i32 m_linkIndex;
	bool m_doubleSided;
};

class TinyRenderer
{
public:
	static void renderObjectDepth(TinyRenderObjectData& renderData);
	static void renderObject(TinyRenderObjectData& renderData);
};

#endif  // TINY_RENDERER_Hbla
