#include "../TinyRenderer.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <drx3D/OpenGLWindow/ShapeData.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Common/ResourcePath.h>
#include <drx3D/Common/b3Logging.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include "../geometry.h"
#include "../model.h"
#include "../our_gl.h"
#include "../tgaimage.h"

using namespace TinyRender;

struct DepthShader : public IShader
{
	Model* m_model;
	Matrix& m_modelMat;
	Matrix m_invModelMat;

	Matrix& m_projectionMat;
	Vec3f m_localScaling;
	Matrix& m_lightModelView;
	float m_lightDistance;

	mat<2, 3, float> varying_uv;   // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<4, 3, float> varying_tri;  // triangle coordinates (clip coordinates), written by VS, read by FS

	mat<3, 3, float> varying_nrm;  // normal per vertex to be interpolated by FS

	DepthShader(Model* model, Matrix& lightModelView, Matrix& projectionMat, Matrix& modelMat, Vec3f localScaling, float lightDistance)
		: m_model(model),
		  m_modelMat(modelMat),
		  m_projectionMat(projectionMat),
		  m_localScaling(localScaling),
		  m_lightModelView(lightModelView),
		  m_lightDistance(lightDistance)
	{
		m_nearPlane = m_projectionMat.col(3)[2] / (m_projectionMat.col(2)[2] - 1);
		m_farPlane = m_projectionMat.col(3)[2] / (m_projectionMat.col(2)[2] + 1);

		m_invModelMat = m_modelMat.invert_transpose();
	}
	virtual Vec4f vertex(i32 iface, i32 nthvert)
	{
		Vec2f uv = m_model->uv(iface, nthvert);
		varying_uv.set_col(nthvert, uv);
		varying_nrm.set_col(nthvert, proj<3>(m_invModelMat * embed<4>(m_model->normal(iface, nthvert), 0.f)));
		Vec3f unScaledVert = m_model->vert(iface, nthvert);
		Vec3f scaledVert = Vec3f(unScaledVert[0] * m_localScaling[0],
								 unScaledVert[1] * m_localScaling[1],
								 unScaledVert[2] * m_localScaling[2]);
		Vec4f gl_Vertex = m_projectionMat * m_lightModelView * embed<4>(scaledVert);
		varying_tri.set_col(nthvert, gl_Vertex);
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		Vec4f p = varying_tri * bar;
		color = TGAColor(255, 255, 255) * (p[2] / m_lightDistance);
		return false;
	}
};

struct Shader : public IShader
{
	Model* m_model;
	Vec3f m_light_dir_local;
	Vec3f m_light_color;
	Matrix& m_modelMat;
	Matrix m_invModelMat;
	Matrix& m_modelView1;
	Matrix& m_projectionMat;
	Vec3f m_localScaling;
	Matrix& m_lightModelView;
	Vec4f m_colorRGBA;
	Matrix& m_viewportMat;
	Matrix m_projectionModelViewMat;
	Matrix m_projectionLightViewMat;
	float m_ambient_coefficient;
	float m_diffuse_coefficient;
	float m_specular_coefficient;

	b3AlignedObjectArray<float>* m_shadowBuffer;

	i32 m_width;
	i32 m_height;

	i32 m_index;

	mat<2, 3, float> varying_uv;   // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<4, 3, float> varying_tri;  // triangle coordinates (clip coordinates), written by VS, read by FS
	mat<4, 3, float> varying_tri_light_view;
	mat<3, 3, float> varying_nrm;  // normal per vertex to be interpolated by FS
	mat<4, 3, float> world_tri;    // model triangle coordinates in the world space used for backface culling, written by VS

	Shader(Model* model, Vec3f light_dir_local, Vec3f light_color, Matrix& modelView, Matrix& lightModelView, Matrix& projectionMat, Matrix& modelMat, Matrix& viewportMat, Vec3f localScaling, const Vec4f& colorRGBA, i32 width, i32 height, b3AlignedObjectArray<float>* shadowBuffer, float ambient_coefficient = 0.6, float diffuse_coefficient = 0.35, float specular_coefficient = 0.05)
		: m_model(model),
		  m_light_dir_local(light_dir_local),
		  m_light_color(light_color),
		  m_modelMat(modelMat),
		  m_modelView1(modelView),
		  m_projectionMat(projectionMat),
		  m_localScaling(localScaling),
		  m_lightModelView(lightModelView),
		  m_colorRGBA(colorRGBA),
		  m_viewportMat(viewportMat),
		  m_ambient_coefficient(ambient_coefficient),
		  m_diffuse_coefficient(diffuse_coefficient),
		  m_specular_coefficient(specular_coefficient),

		  m_shadowBuffer(shadowBuffer),
		  m_width(width),
		  m_height(height)

	{
		m_nearPlane = m_projectionMat.col(3)[2] / (m_projectionMat.col(2)[2] - 1);
		m_farPlane = m_projectionMat.col(3)[2] / (m_projectionMat.col(2)[2] + 1);
		//printf("near=%f, far=%f\n", m_nearPlane, m_farPlane);
		m_invModelMat = m_modelMat.invert_transpose();
		m_projectionModelViewMat = m_projectionMat * m_modelView1;
		m_projectionLightViewMat = m_projectionMat * m_lightModelView;
	}
	virtual Vec4f vertex(i32 iface, i32 nthvert)
	{
		//D3_PROFILE("vertex");
		Vec2f uv = m_model->uv(iface, nthvert);
		varying_uv.set_col(nthvert, uv);
		varying_nrm.set_col(nthvert, proj<3>(m_invModelMat * embed<4>(m_model->normal(iface, nthvert), 0.f)));
		Vec3f unScaledVert = m_model->vert(iface, nthvert);
		Vec3f scaledVert = Vec3f(unScaledVert[0] * m_localScaling[0],
								 unScaledVert[1] * m_localScaling[1],
								 unScaledVert[2] * m_localScaling[2]);
		Vec4f gl_Vertex = m_projectionModelViewMat * embed<4>(scaledVert);
		varying_tri.set_col(nthvert, gl_Vertex);
		Vec4f world_Vertex = m_modelMat * embed<4>(scaledVert);
		world_tri.set_col(nthvert, world_Vertex);
		Vec4f gl_VertexLightView = m_projectionLightViewMat * embed<4>(scaledVert);
		varying_tri_light_view.set_col(nthvert, gl_VertexLightView);
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor& color)
	{
		//D3_PROFILE("fragment");
		Vec4f p = m_viewportMat * (varying_tri_light_view * bar);
		float depth = p[2];
		p = p / p[3];

		float index_x = d3Max(float(0.0), d3Min(float(m_width - 1), p[0]));
		float index_y = d3Max(float(0.0), d3Min(float(m_height - 1), p[1]));
		i32 idx = i32(index_x) + i32(index_y) * m_width;                       // index in the shadowbuffer array
		float shadow = 1.0;
		if (m_shadowBuffer && idx >=0 && idx <m_shadowBuffer->size())
		{
			shadow = 0.8 + 0.2 * (m_shadowBuffer->at(idx) < -depth + 0.05);  // magic coeff to avoid z-fighting
		}
		Vec3f bn = (varying_nrm * bar).normalize();
		Vec2f uv = varying_uv * bar;

		Vec3f reflection_direction = (bn * (bn * m_light_dir_local * 2.f) - m_light_dir_local).normalize();
        float specular = std::pow(d3Max(reflection_direction.z, 0.f),
                                    m_model->specular(uv));
        float diffuse = d3Max(0.f, bn * m_light_dir_local);

        color = m_model->diffuse(uv);
		color[0] *= m_colorRGBA[0];
		color[1] *= m_colorRGBA[1];
		color[2] *= m_colorRGBA[2];
		color[3] *= m_colorRGBA[3];

		for (i32 i = 0; i < 3; ++i)
		{
			i32 orgColor = 0;
			float floatColor = (m_ambient_coefficient * color[i] + shadow * (m_diffuse_coefficient * diffuse + m_specular_coefficient * specular) * color[i] * m_light_color[i]);
			if (floatColor==floatColor)
			{
				orgColor=i32(floatColor);
			}
			color[i] = d3Min(orgColor, 255);
		}

		return false;
	}
};

TinyRenderObjectData::TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer, b3AlignedObjectArray<float>* shadowBuffer)
	: m_model(0),
	  m_rgbColorBuffer(rgbColorBuffer),
	  m_depthBuffer(depthBuffer),
	  m_shadowBuffer(shadowBuffer),
	  m_segmentationMaskBufferPtr(0),
	  m_userData(0),
	  m_userIndex(-1),
	  m_objectIndex(-1),
	  m_doubleSided(false)
{
	Vec3f eye(1, 1, 3);
	Vec3f center(0, 0, 0);
	Vec3f up(0, 0, 1);
	m_lightDirWorld.setVal(0, 0, 0);
	m_lightColor.setVal(1, 1, 1);
	m_localScaling.setVal(1, 1, 1);
	m_modelMatrix = Matrix::identity();
	m_lightAmbientCoeff = 0.6;
	m_lightDiffuseCoeff = 0.35;
	m_lightSpecularCoeff = 0.05;

}

TinyRenderObjectData::TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer, b3AlignedObjectArray<float>* shadowBuffer, b3AlignedObjectArray<i32>* segmentationMaskBuffer, i32 objectIndex, i32 linkIndex)
	: m_model(0),
	  m_rgbColorBuffer(rgbColorBuffer),
	  m_depthBuffer(depthBuffer),
	  m_shadowBuffer(shadowBuffer),
	  m_segmentationMaskBufferPtr(segmentationMaskBuffer),
	  m_userData(0),
	  m_userIndex(-1),
	  m_objectIndex(objectIndex),
	  m_linkIndex(linkIndex),
	  m_doubleSided(false)
{
	Vec3f eye(1, 1, 3);
	Vec3f center(0, 0, 0);
	Vec3f up(0, 0, 1);
	m_lightDirWorld.setVal(0, 0, 0);
	m_lightColor.setVal(1, 1, 1);
	m_localScaling.setVal(1, 1, 1);
	m_modelMatrix = Matrix::identity();
	m_lightAmbientCoeff = 0.6;
	m_lightDiffuseCoeff = 0.35;
	m_lightSpecularCoeff = 0.05;
}

TinyRenderObjectData::TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer)
	: m_model(0),
	  m_rgbColorBuffer(rgbColorBuffer),
	  m_depthBuffer(depthBuffer),
	  m_shadowBuffer(0),
	  m_segmentationMaskBufferPtr(0),
	  m_userData(0),
	  m_userIndex(-1),
	  m_objectIndex(-1),
	m_doubleSided(false)
{
	Vec3f eye(1, 1, 3);
	Vec3f center(0, 0, 0);
	Vec3f up(0, 0, 1);
	m_lightDirWorld.setVal(0, 0, 0);
	m_lightDistance = 10;
	m_lightColor.setVal(1, 1, 1);
	m_localScaling.setVal(1, 1, 1);
	m_modelMatrix = Matrix::identity();
	m_lightAmbientCoeff = 0.6;
	m_lightDiffuseCoeff = 0.35;
	m_lightSpecularCoeff = 0.05;
}

TinyRenderObjectData::TinyRenderObjectData(TGAImage& rgbColorBuffer, b3AlignedObjectArray<float>& depthBuffer, b3AlignedObjectArray<i32>* segmentationMaskBuffer, i32 objectIndex)
	: m_model(0),
	  m_rgbColorBuffer(rgbColorBuffer),
	  m_depthBuffer(depthBuffer),
	  m_shadowBuffer(0),
	  m_segmentationMaskBufferPtr(segmentationMaskBuffer),
	  m_userData(0),
	  m_userIndex(-1),
	  m_objectIndex(objectIndex),
	m_doubleSided(false)
{
	Vec3f eye(1, 1, 3);
	Vec3f center(0, 0, 0);
	Vec3f up(0, 0, 1);
	m_lightDirWorld.setVal(0, 0, 0);
	m_lightColor.setVal(1, 1, 1);
	m_localScaling.setVal(1, 1, 1);
	m_modelMatrix = Matrix::identity();
	m_lightAmbientCoeff = 0.6;
	m_lightDiffuseCoeff = 0.35;
	m_lightSpecularCoeff = 0.05;
}

void TinyRenderObjectData::loadModel(tukk fileName, CommonFileIOInterface* fileIO)
{
	//todo(erwincoumans) move the file loading out of here
	char relativeFileName[1024];
	if (!fileIO->findResourcePath(fileName, relativeFileName, 1024))
	{
		printf("Не найден файл %s\n", fileName);
	}
	else
	{
		m_model = new Model(relativeFileName);
	}
}

void TinyRenderObjectData::registerMeshShape(const float* vertices, i32 numVertices, i32k* indices, i32 numIndices, const float rgbaColor[4],
											 u8* textureImage, i32 textureWidth, i32 textureHeight)
{
	if (0 == m_model)
	{
		{
			D3_PROFILE("setColorRGBA");

			m_model = new Model();
			m_model->setColorRGBA(rgbaColor);
		}
		if (textureImage)
		{
			{
				D3_PROFILE("setDiffuseTextureFromData");
				m_model->setDiffuseTextureFromData(textureImage, textureWidth, textureHeight);
			}
		}
		else
		{
			/*char relativeFileName[1024];
			if (ResourcePath::findResourcePath("floor_diffuse.tga", relativeFileName, 1024))
			{
				m_model->loadDiffuseTexture(relativeFileName);
			}
             */
		}
		{
			D3_PROFILE("reserveMemory");
			m_model->reserveMemory(numVertices, numIndices);
		}
		{
			D3_PROFILE("addVertex");
			for (i32 i = 0; i < numVertices; i++)
			{
				m_model->addVertex(vertices[i * 9],
								   vertices[i * 9 + 1],
								   vertices[i * 9 + 2],
								   vertices[i * 9 + 4],
								   vertices[i * 9 + 5],
								   vertices[i * 9 + 6],
								   vertices[i * 9 + 7],
								   vertices[i * 9 + 8]);
			}
		}
		{
			D3_PROFILE("addTriangle");
			for (i32 i = 0; i < numIndices; i += 3)
			{
				m_model->addTriangle(indices[i], indices[i], indices[i],
									 indices[i + 1], indices[i + 1], indices[i + 1],
									 indices[i + 2], indices[i + 2], indices[i + 2]);
			}
		}
	}
}

void TinyRenderObjectData::registerMesh2(AlignedObjectArray<Vec3>& vertices, AlignedObjectArray<Vec3>& normals, AlignedObjectArray<i32>& indices, CommonFileIOInterface* fileIO)
{
	if (0 == m_model)
	{
		i32 numVertices = vertices.size();
		i32 numIndices = indices.size();

		m_model = new Model();
		char relativeFileName[1024];
		if (fileIO->findResourcePath("floor_diffuse.tga", relativeFileName, 1024))
		{
			m_model->loadDiffuseTexture(relativeFileName);
		}

		for (i32 i = 0; i < numVertices; i++)
		{
			m_model->addVertex(vertices[i].x(),
							   vertices[i].y(),
							   vertices[i].z(),
							   normals[i].x(),
							   normals[i].y(),
							   normals[i].z(),
							   0.5, 0.5);
		}
		for (i32 i = 0; i < numIndices; i += 3)
		{
			m_model->addTriangle(indices[i], indices[i], indices[i],
								 indices[i + 1], indices[i + 1], indices[i + 1],
								 indices[i + 2], indices[i + 2], indices[i + 2]);
		}
	}
}

void TinyRenderObjectData::createCube(float halfExtentsX, float halfExtentsY, float halfExtentsZ, CommonFileIOInterface* fileIO)
{
	DefaultFileIO defaultFileIO;

	if (fileIO==0)
	{
		fileIO = &defaultFileIO;
	}
	m_model = new Model();

	char relativeFileName[1024];
	if (fileIO->findResourcePath("floor_diffuse.tga", relativeFileName, 1024))
	{
		m_model->loadDiffuseTexture(relativeFileName);
	}

	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices_textured) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);

	for (i32 i = 0; i < numVertices; i++)
	{
		m_model->addVertex(halfExtentsX * cube_vertices_textured[i * 9],
						   halfExtentsY * cube_vertices_textured[i * 9 + 1],
						   halfExtentsZ * cube_vertices_textured[i * 9 + 2],
						   cube_vertices_textured[i * 9 + 4],
						   cube_vertices_textured[i * 9 + 5],
						   cube_vertices_textured[i * 9 + 6],
						   cube_vertices_textured[i * 9 + 7],
						   cube_vertices_textured[i * 9 + 8]);
	}
	for (i32 i = 0; i < numIndices; i += 3)
	{
		m_model->addTriangle(cube_indices[i], cube_indices[i], cube_indices[i],
							 cube_indices[i + 1], cube_indices[i + 1], cube_indices[i + 1],
							 cube_indices[i + 2], cube_indices[i + 2], cube_indices[i + 2]);
	}
}

TinyRenderObjectData::~TinyRenderObjectData()
{
	delete m_model;
}

static bool equals(const Vec4f& vA, const Vec4f& vB)
{
	return false;
}

static void clipEdge(const mat<4, 3, float>& triangleIn, i32 vertexIndexA, i32 vertexIndexB, b3AlignedObjectArray<Vec4f>& vertices)
{
	Vec4f v0New = triangleIn.col(vertexIndexA);
	Vec4f v1New = triangleIn.col(vertexIndexB);

	bool v0Inside = v0New[3] > 0.f && v0New[2] > -v0New[3];
	bool v1Inside = v1New[3] > 0.f && v1New[2] > -v1New[3];

	if (v0Inside && v1Inside)
	{
	}
	else if (v0Inside || v1Inside)
	{
		float d0 = v0New[2] + v0New[3];
		float d1 = v1New[2] + v1New[3];
		float factor = 1.0 / (d1 - d0);
		Vec4f newVertex = (v0New * d1 - v1New * d0) * factor;
		if (v0Inside)
		{
			v1New = newVertex;
		}
		else
		{
			v0New = newVertex;
		}
	}
	else
	{
		return;
	}

	if (vertices.size() == 0 || !(equals(vertices[vertices.size() - 1], v0New)))
	{
		vertices.push_back(v0New);
	}

	vertices.push_back(v1New);
}

static bool clipTriangleAgainstNearplane(const mat<4, 3, float>& triangleIn, b3AlignedObjectArray<mat<4, 3, float> >& clippedTrianglesOut)
{
	//discard triangle if all vertices are behind near-plane
	if (triangleIn[3][0] < 0 && triangleIn[3][1] < 0 && triangleIn[3][2] < 0)
	{
		return true;
	}

	//accept triangle if all vertices are in front of the near-plane
	if (triangleIn[3][0] >= 0 && triangleIn[3][1] >= 0 && triangleIn[3][2] >= 0)
	{
		clippedTrianglesOut.push_back(triangleIn);
		return false;
	}

	Vec4f vtxCache[5];

	b3AlignedObjectArray<Vec4f> vertices;
	vertices.initializeFromBuffer(vtxCache, 0, 5);
	clipEdge(triangleIn, 0, 1, vertices);
	clipEdge(triangleIn, 1, 2, vertices);
	clipEdge(triangleIn, 2, 0, vertices);

	if (vertices.size() < 3)
		return true;

	if (equals(vertices[0], vertices[vertices.size() - 1]))
	{
		vertices.pop_back();
	}

	//create a fan of triangles
	for (i32 i = 1; i < vertices.size() - 1; i++)
	{
		mat<4, 3, float>& vtx = clippedTrianglesOut.expand();
		vtx.set_col(0, vertices[0]);
		vtx.set_col(1, vertices[i]);
		vtx.set_col(2, vertices[i + 1]);
	}
	return true;
}

void TinyRenderer::renderObject(TinyRenderObjectData& renderData)
{
	D3_PROFILE("renderObject");
	i32 width = renderData.m_rgbColorBuffer.get_width();
	i32 height = renderData.m_rgbColorBuffer.get_height();

	Vec3f light_dir_local = Vec3f(renderData.m_lightDirWorld[0], renderData.m_lightDirWorld[1], renderData.m_lightDirWorld[2]);
	Vec3f light_color = Vec3f(renderData.m_lightColor[0], renderData.m_lightColor[1], renderData.m_lightColor[2]);
	float light_distance = renderData.m_lightDistance;
	Model* model = renderData.m_model;
	if (0 == model)
		return;
	//discard invisible objects (zero alpha)
	if (model->getColorRGBA()[3] == 0)
		return;

	renderData.m_viewportMatrix = viewport(0, 0, width, height);

	b3AlignedObjectArray<float>& zbuffer = renderData.m_depthBuffer;
	b3AlignedObjectArray<float>* shadowBufferPtr = renderData.m_shadowBuffer;
	i32* segmentationMaskBufferPtr = (renderData.m_segmentationMaskBufferPtr && renderData.m_segmentationMaskBufferPtr->size()) ? &renderData.m_segmentationMaskBufferPtr->at(0) : 0;

	TGAImage& frame = renderData.m_rgbColorBuffer;

	{
		// light target is set to be the origin, and the up direction is set to be vertical up.
		Matrix lightViewMatrix = lookat(light_dir_local * light_distance, Vec3f(0.0, 0.0, 0.0), Vec3f(0.0, 0.0, 1.0));
		Matrix lightModelViewMatrix = lightViewMatrix * renderData.m_modelMatrix;
		Matrix modelViewMatrix = renderData.m_viewMatrix * renderData.m_modelMatrix;
		Vec3f localScaling(renderData.m_localScaling[0], renderData.m_localScaling[1], renderData.m_localScaling[2]);
		Matrix viewMatrixInv = renderData.m_viewMatrix.invert();
		Vec3 P(viewMatrixInv[0][3], viewMatrixInv[1][3], viewMatrixInv[2][3]);

		Shader shader(model, light_dir_local, light_color, modelViewMatrix, lightModelViewMatrix, renderData.m_projectionMatrix, renderData.m_modelMatrix, renderData.m_viewportMatrix, localScaling, model->getColorRGBA(), width, height, shadowBufferPtr, renderData.m_lightAmbientCoeff, renderData.m_lightDiffuseCoeff, renderData.m_lightSpecularCoeff);

		{
			D3_PROFILE("face");

			for (i32 i = 0; i < model->nfaces(); i++)
			{
				for (i32 j = 0; j < 3; j++)
				{
					shader.vertex(i, j);
				}

				if (!renderData.m_doubleSided)
				{
					// backface culling
					Vec3 v0(shader.world_tri.col(0)[0], shader.world_tri.col(0)[1], shader.world_tri.col(0)[2]);
					Vec3 v1(shader.world_tri.col(1)[0], shader.world_tri.col(1)[1], shader.world_tri.col(1)[2]);
					Vec3 v2(shader.world_tri.col(2)[0], shader.world_tri.col(2)[1], shader.world_tri.col(2)[2]);
					Vec3 N = (v1 - v0).cross(v2 - v0);
					if ((v0 - P).dot(N) >= 0)
						continue;
				}

				mat<4, 3, float> stackTris[3];

				b3AlignedObjectArray<mat<4, 3, float> > clippedTriangles;
				clippedTriangles.initializeFromBuffer(stackTris, 0, 3);

				bool hasClipped = clipTriangleAgainstNearplane(shader.varying_tri, clippedTriangles);

				if (hasClipped)
				{
					for (i32 t = 0; t < clippedTriangles.size(); t++)
					{
						triangleClipped(clippedTriangles[t], shader.varying_tri, shader, frame, &zbuffer[0], segmentationMaskBufferPtr, renderData.m_viewportMatrix, renderData.m_objectIndex + ((renderData.m_linkIndex + 1) << 24));
					}
				}
				else
				{
					triangle(shader.varying_tri, shader, frame, &zbuffer[0], segmentationMaskBufferPtr, renderData.m_viewportMatrix, renderData.m_objectIndex + ((renderData.m_linkIndex + 1) << 24));
				}
			}
		}
	}
}

void TinyRenderer::renderObjectDepth(TinyRenderObjectData& renderData)
{
	i32 width = renderData.m_rgbColorBuffer.get_width();
	i32 height = renderData.m_rgbColorBuffer.get_height();

	Vec3f light_dir_local = Vec3f(renderData.m_lightDirWorld[0], renderData.m_lightDirWorld[1], renderData.m_lightDirWorld[2]);
	float light_distance = renderData.m_lightDistance;
	Model* model = renderData.m_model;
	if (0 == model)
		return;

	renderData.m_viewportMatrix = viewport(0, 0, width, height);

	float* shadowBufferPtr = (renderData.m_shadowBuffer && renderData.m_shadowBuffer->size()) ? &renderData.m_shadowBuffer->at(0) : 0;
	i32* segmentationMaskBufferPtr = 0;

	TGAImage depthFrame(width, height, TGAImage::RGB);

	{
		// light target is set to be the origin, and the up direction is set to be vertical up.
		Matrix lightViewMatrix = lookat(light_dir_local * light_distance, Vec3f(0.0, 0.0, 0.0), Vec3f(0.0, 0.0, 1.0));
		Matrix lightModelViewMatrix = lightViewMatrix * renderData.m_modelMatrix;
		Matrix lightViewProjectionMatrix = renderData.m_projectionMatrix;
		Vec3f localScaling(renderData.m_localScaling[0], renderData.m_localScaling[1], renderData.m_localScaling[2]);

		DepthShader shader(model, lightModelViewMatrix, lightViewProjectionMatrix, renderData.m_modelMatrix, localScaling, light_distance);
		for (i32 i = 0; i < model->nfaces(); i++)
		{
			for (i32 j = 0; j < 3; j++)
			{
				shader.vertex(i, j);
			}

			mat<4, 3, float> stackTris[3];

			b3AlignedObjectArray<mat<4, 3, float> > clippedTriangles;
			clippedTriangles.initializeFromBuffer(stackTris, 0, 3);

			bool hasClipped = clipTriangleAgainstNearplane(shader.varying_tri, clippedTriangles);

			if (hasClipped)
			{
				for (i32 t = 0; t < clippedTriangles.size(); t++)
				{
					triangleClipped(clippedTriangles[t], shader.varying_tri, shader, depthFrame, shadowBufferPtr, segmentationMaskBufferPtr, renderData.m_viewportMatrix, renderData.m_objectIndex);
				}
			}
			else
			{
				triangle(shader.varying_tri, shader, depthFrame, shadowBufferPtr, segmentationMaskBufferPtr, renderData.m_viewportMatrix, renderData.m_objectIndex);
			}
		}
	}
}
