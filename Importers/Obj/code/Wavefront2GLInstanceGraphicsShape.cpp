#include "../Wavefront2GLInstanceGraphicsShape.h"

#include  <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include  <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include  <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include  <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include  <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>

GLInstanceGraphicsShape* btgCreateGraphicsShapeFromWavefrontObj(const bt_tinyobj::attrib_t& attribute, std::vector<bt_tinyobj::shape_t>& shapes, bool flatShading)
{
	b3AlignedObjectArray<GLInstanceVertex>* vertices = new b3AlignedObjectArray<GLInstanceVertex>;
	{
		//		i32 numVertices = obj->vertexCount;
		//	i32 numIndices = 0;
		b3AlignedObjectArray<i32>* indicesPtr = new b3AlignedObjectArray<i32>;

		for (i32 s = 0; s < (i32)shapes.size(); s++)
		{
			bt_tinyobj::shape_t& shape = shapes[s];
			i32 faceCount = shape.mesh.indices.size();

			for (i32 f = 0; f < faceCount; f += 3)
			{
				//Vec3 normal(face.m_plane[0],face.m_plane[1],face.m_plane[2]);
				if (1)
				{
					Vec3 normal(0, 1, 0);
					i32 vtxBaseIndex = vertices->size();

					if (f < 0 && f >= i32(shape.mesh.indices.size()))
					{
						continue;
					}

					GLInstanceVertex vtx0;
					bt_tinyobj::index_t v_0 = shape.mesh.indices[f];
					vtx0.xyzw[0] = attribute.vertices[3 * v_0.vertex_index];
					vtx0.xyzw[1] = attribute.vertices[3 * v_0.vertex_index + 1];
					vtx0.xyzw[2] = attribute.vertices[3 * v_0.vertex_index + 2];
					vtx0.xyzw[3] = 0.f;

					if (attribute.texcoords.size())
					{
						i32 uv0Index = 2 * v_0.texcoord_index;
						i32 uv1Index = 2 * v_0.texcoord_index + 1;
						if (uv0Index >= 0 && uv1Index >= 0 && (uv0Index < i32(attribute.texcoords.size()) && (uv1Index < attribute.texcoords.size())))
						{
							vtx0.uv[0] = attribute.texcoords[uv0Index];
							vtx0.uv[1] = attribute.texcoords[uv1Index];
						}
						else
						{
							//	drx3DWarning("obj texture coordinate out-of-range!");
							vtx0.uv[0] = 0;
							vtx0.uv[1] = 0;
						}
					}
					else
					{
						vtx0.uv[0] = 0.5;
						vtx0.uv[1] = 0.5;
					}

					GLInstanceVertex vtx1;
					bt_tinyobj::index_t v_1 = shape.mesh.indices[f + 1];
					vtx1.xyzw[0] = attribute.vertices[3 * v_1.vertex_index];
					vtx1.xyzw[1] = attribute.vertices[3 * v_1.vertex_index + 1];
					vtx1.xyzw[2] = attribute.vertices[3 * v_1.vertex_index + 2];
					vtx1.xyzw[3] = 0.f;

					if (attribute.texcoords.size())
					{
						i32 uv0Index = 2 * v_1.texcoord_index;
						i32 uv1Index = 2 * v_1.texcoord_index + 1;
						if (uv0Index >= 0 && uv1Index >= 0 && (uv0Index < attribute.texcoords.size()) && (uv1Index < attribute.texcoords.size()))
						{
							vtx1.uv[0] = attribute.texcoords[uv0Index];
							vtx1.uv[1] = attribute.texcoords[uv1Index];
						}
						else
						{
							//	drx3DWarning("obj texture coordinate out-of-range!");
							vtx1.uv[0] = 0;
							vtx1.uv[1] = 0;
						}
					}
					else
					{
						vtx1.uv[0] = 0.5f;
						vtx1.uv[1] = 0.5f;
					}

					GLInstanceVertex vtx2;
					bt_tinyobj::index_t v_2 = shape.mesh.indices[f + 2];
					vtx2.xyzw[0] = attribute.vertices[3 * v_2.vertex_index];
					vtx2.xyzw[1] = attribute.vertices[3 * v_2.vertex_index + 1];
					vtx2.xyzw[2] = attribute.vertices[3 * v_2.vertex_index + 2];
					vtx2.xyzw[3] = 0.f;
					if (attribute.texcoords.size())
					{
						i32 uv0Index = 2 * v_2.texcoord_index;
						i32 uv1Index = 2 * v_2.texcoord_index + 1;

						if (uv0Index >= 0 && uv1Index >= 0 && (uv0Index < attribute.texcoords.size()) && (uv1Index < attribute.texcoords.size()))
						{
							vtx2.uv[0] = attribute.texcoords[uv0Index];
							vtx2.uv[1] = attribute.texcoords[uv1Index];
						}
						else
						{
							//drx3DWarning("obj texture coordinate out-of-range!");
							vtx2.uv[0] = 0;
							vtx2.uv[1] = 0;
						}
					}
					else
					{
						vtx2.uv[0] = 0.5;
						vtx2.uv[1] = 0.5;
					}

					Vec3 v0(vtx0.xyzw[0], vtx0.xyzw[1], vtx0.xyzw[2]);
					Vec3 v1(vtx1.xyzw[0], vtx1.xyzw[1], vtx1.xyzw[2]);
					Vec3 v2(vtx2.xyzw[0], vtx2.xyzw[1], vtx2.xyzw[2]);

					u32 maxIndex = 0;
					unsigned n0Index = shape.mesh.indices[f].normal_index;
					unsigned n1Index = shape.mesh.indices[f + 1].normal_index;
					unsigned n2Index = shape.mesh.indices[f + 2].normal_index;

					maxIndex = d3Max(maxIndex, 3 * n0Index + 0);
					maxIndex = d3Max(maxIndex, 3 * n0Index + 1);
					maxIndex = d3Max(maxIndex, 3 * n0Index + 2);
					maxIndex = d3Max(maxIndex, 3 * n1Index + 0);
					maxIndex = d3Max(maxIndex, 3 * n1Index + 1);
					maxIndex = d3Max(maxIndex, 3 * n1Index + 2);
					maxIndex = d3Max(maxIndex, 3 * n2Index + 0);
					maxIndex = d3Max(maxIndex, 3 * n2Index + 1);
					maxIndex = d3Max(maxIndex, 3 * n2Index + 2);

					bool hasNormals = (attribute.normals.size() && maxIndex < attribute.normals.size());

					if (flatShading || !hasNormals)
					{
						normal = (v1 - v0).cross(v2 - v0);
						Scalar len2 = normal.length2();
						//skip degenerate triangles
						if (len2 > SIMD_EPSILON)
						{
							normal.normalize();
						}
						else
						{
							normal.setVal(0, 0, 0);
						}
						vtx0.normal[0] = normal[0];
						vtx0.normal[1] = normal[1];
						vtx0.normal[2] = normal[2];
						vtx1.normal[0] = normal[0];
						vtx1.normal[1] = normal[1];
						vtx1.normal[2] = normal[2];
						vtx2.normal[0] = normal[0];
						vtx2.normal[1] = normal[1];
						vtx2.normal[2] = normal[2];
					}
					else
					{
						vtx0.normal[0] = attribute.normals[3 * n0Index+ 0];
						vtx0.normal[1] = attribute.normals[3 * n0Index+ 1];
						vtx0.normal[2] = attribute.normals[3 * n0Index+ 2];
						vtx1.normal[0] = attribute.normals[3 * n1Index+ 0];
						vtx1.normal[1] = attribute.normals[3 * n1Index+ 1];
						vtx1.normal[2] = attribute.normals[3 * n1Index+ 2];
						vtx2.normal[0] = attribute.normals[3 * n2Index+ 0];
						vtx2.normal[1] = attribute.normals[3 * n2Index+ 1];
						vtx2.normal[2] = attribute.normals[3 * n2Index+ 2];
					}
					vertices->push_back(vtx0);
					vertices->push_back(vtx1);
					vertices->push_back(vtx2);
					indicesPtr->push_back(vtxBaseIndex);
					indicesPtr->push_back(vtxBaseIndex + 1);
					indicesPtr->push_back(vtxBaseIndex + 2);
				}
			}
		}

		GLInstanceGraphicsShape* gfxShape = new GLInstanceGraphicsShape;
		gfxShape->m_vertices = vertices;
		gfxShape->m_numvertices = vertices->size();
		gfxShape->m_indices = indicesPtr;
		gfxShape->m_numIndices = indicesPtr->size();
		for (i32 i = 0; i < 4; i++)
			gfxShape->m_scaling[i] = 1;  //bake the scaling into the vertices
		return gfxShape;
	}
}
