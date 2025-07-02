#include "GpuCompoundScene.h"
#include "GpuRigidBodyDemo.h"
#include "OpenGLWindow/ShapeData.h"

#include "OpenGLWindow/GLInstancingRenderer.h"
#include <drx3D/Common/b3Quat.h"
#include "OpenGLWindow/b3gWindowInterface.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuSapBroadphase.h"
#include "../GpuDemoInternalData.h"
#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"
#include "OpenGLWindow/OpenGLInclude.h"
#include "OpenGLWindow/GLInstanceRendererInternalData.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3LauncherCL.h"
#include "Bullet3OpenCL/RigidBody/b3GpuRigidBodyPipeline.h"
#include "Bullet3OpenCL/RigidBody/b3GpuNarrowPhase.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3Config.h"
#include "GpuRigidBodyDemoInternalData.h"
#include <drx3D/Common/b3Transform.h"

#include "OpenGLWindow/GLInstanceGraphicsShape.h"

#define NUM_COMPOUND_CHILDREN_X 4
#define NUM_COMPOUND_CHILDREN_Y 4
#define NUM_COMPOUND_CHILDREN_Z 4

void GpuCompoundScene::setupScene(const ConstructionInfo& ci)
{
	createStaticEnvironment(ci);

	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);
	float scaling[4] = {1, 1, 1, 1};

	GLInstanceVertex* cubeVerts = (GLInstanceVertex*)&cube_vertices[0];
	i32 stride2 = sizeof(GLInstanceVertex);
	drx3DAssert(stride2 == strideInBytes);
	i32 index = 0;
	i32 colIndex = -1;
	b3AlignedObjectArray<GLInstanceVertex> vertexArray;
	b3AlignedObjectArray<i32> indexArray;
	{
		i32 childColIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0], strideInBytes, numVertices, scaling);

		/*		b3Vec3 childPositions[3] = {
			b3Vec3(0,-2,0),
			b3Vec3(0,0,0),
			b3Vec3(0,0,2)
		};
		*/

		b3AlignedObjectArray<b3GpuChildShape> childShapes;

		for (i32 x = 0; x < NUM_COMPOUND_CHILDREN_X; x++)
			for (i32 y = 0; y < NUM_COMPOUND_CHILDREN_Y; y++)
				for (i32 z = 0; z < NUM_COMPOUND_CHILDREN_Z; z++)
				{
					i32 blax = x != 0 ? 1 : 0;
					i32 blay = y != 0 ? 1 : 0;
					i32 blaz = z != 0 ? 1 : 0;
					i32 bla = blax + blay + blaz;
					if (bla != 1)
						continue;

					//for now, only support polyhedral child shapes
					b3GpuChildShape child;
					child.m_shapeIndex = childColIndex;
					b3Vec3 pos = b3MakeVector3((x - NUM_COMPOUND_CHILDREN_X / 2.f) * 2, (y - NUM_COMPOUND_CHILDREN_X / 2.f) * 2, (z - NUM_COMPOUND_CHILDREN_X / 2.f) * 2);  //childPositions[i];
					b3Quat orn(0, 0, 0, 1);
					for (i32 v = 0; v < 4; v++)
					{
						child.m_childPosition[v] = pos[v];
						child.m_childOrientation[v] = orn[v];
					}
					childShapes.push_back(child);
					b3Transform tr;
					tr.setIdentity();
					tr.setOrigin(pos);
					tr.setRotation(orn);

					i32 baseIndex = vertexArray.size();
					for (i32 j = 0; j < numIndices; j++)
						indexArray.push_back(cube_indices[j] + baseIndex);

					//add transformed graphics vertices and indices
					for (i32 v = 0; v < numVertices; v++)
					{
						GLInstanceVertex vert = cubeVerts[v];
						b3Vec3 vertPos = b3MakeVector3(vert.xyzw[0], vert.xyzw[1], vert.xyzw[2]);
						b3Vec3 newPos = tr * vertPos;
						vert.xyzw[0] = newPos[0];
						vert.xyzw[1] = newPos[1];
						vert.xyzw[2] = newPos[2];
						vert.xyzw[3] = 0.f;
						vertexArray.push_back(vert);
					}
				}
		colIndex = m_data->m_np->registerCompoundShape(&childShapes);
	}

	//i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	i32 shapeId = ci.m_instancingRenderer->registerShape(&vertexArray[0].xyzw[0], vertexArray.size(), &indexArray[0], indexArray.size());

	b3Vec4 colors[4] =
		{
			b3MakeVector4(1, 0, 0, 1),
			b3MakeVector4(0, 1, 0, 1),
			b3MakeVector4(0, 0, 1, 1),
			b3MakeVector4(0, 1, 1, 1),
		};

	i32 curColor = 0;
	for (i32 i = 0; i < ci.arraySizeX; i++)
	{
		for (i32 j = 0; j < ci.arraySizeY; j++)
		{
			for (i32 k = 0; k < ci.arraySizeZ; k++)
			{
				float mass = 1;  //j==0? 0.f : 1.f;

				b3Vec3 position = b3MakeVector3((i - ci.arraySizeX / 2.) * ci.gapX, 35 + j * 3 * ci.gapY, (k - ci.arraySizeZ / 2.f) * ci.gapZ);
				//b3Quat orn(0,0,0,1);
				b3Quat orn(b3MakeVector3(1, 0, 0), 0.7);

				b3Vec4 color = colors[curColor];
				curColor++;
				curColor &= 3;
				b3Vec4 scaling = b3MakeVector4(1, 1, 1, 1);
				i32 id = ci.m_instancingRenderer->registerGraphicsInstance(shapeId, position, orn, color, scaling);
				i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass, position, orn, colIndex, index, false);

				index++;
			}
		}
	}

	m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();

	float camPos[4] = {0, 0, 0};  //65.5,4.5,65.5,0};
	//float camPos[4]={1,12.5,1.5,0};
	m_instancingRenderer->setCameraTargetPosition(camPos);
	m_instancingRenderer->setCameraDistance(320);
}

void GpuCompoundScene::createStaticEnvironment(const ConstructionInfo& ci)
{
	i32 strideInBytes = 9 * sizeof(float);

	//i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	i32 group = 1;
	i32 mask = 1;
	i32 index = 0;
	i32 colIndex = 0;

	{
		if (1)
		{
			float radius = 41;
			i32 prevGraphicsShapeIndex = -1;
			{
				if (radius >= 100)
				{
					i32 numVertices = sizeof(detailed_sphere_vertices) / strideInBytes;
					i32 numIndices = sizeof(detailed_sphere_indices) / sizeof(i32);
					prevGraphicsShapeIndex = ci.m_instancingRenderer->registerShape(&detailed_sphere_vertices[0], numVertices, detailed_sphere_indices, numIndices);
				}
				else
				{
					bool usePointSprites = false;
					if (usePointSprites)
					{
						i32 numVertices = sizeof(point_sphere_vertices) / strideInBytes;
						i32 numIndices = sizeof(point_sphere_indices) / sizeof(i32);
						prevGraphicsShapeIndex = ci.m_instancingRenderer->registerShape(&point_sphere_vertices[0], numVertices, point_sphere_indices, numIndices, D3_GL_POINTS);
					}
					else
					{
						if (radius >= 10)
						{
							i32 numVertices = sizeof(medium_sphere_vertices) / strideInBytes;
							i32 numIndices = sizeof(medium_sphere_indices) / sizeof(i32);
							prevGraphicsShapeIndex = ci.m_instancingRenderer->registerShape(&medium_sphere_vertices[0], numVertices, medium_sphere_indices, numIndices);
						}
						else
						{
							i32 numVertices = sizeof(low_sphere_vertices) / strideInBytes;
							i32 numIndices = sizeof(low_sphere_indices) / sizeof(i32);
							prevGraphicsShapeIndex = ci.m_instancingRenderer->registerShape(&low_sphere_vertices[0], numVertices, low_sphere_indices, numIndices);
						}
					}
				}
			}
			b3Vec4 colors[4] =
				{
					b3MakeVector4(1, 0, 0, 1),
					b3MakeVector4(0, 1, 0, 1),
					b3MakeVector4(0, 1, 1, 1),
					b3MakeVector4(1, 1, 0, 1),
				};

			i32 curColor = 1;

			//i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
			i32 colIndex = m_data->m_np->registerSphereShape(radius);  //>registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
			float mass = 0.f;

			//b3Vec3 position((j&1)+i*2.2,1+j*2.,(j&1)+k*2.2);
			b3Vec3 position = b3MakeVector3(0, -41, 0);

			b3Quat orn(0, 0, 0, 1);

			b3Vec4 color = colors[curColor];
			curColor++;
			curColor &= 3;
			b3Vec4 scaling = b3MakeVector4(radius, radius, radius, 1);
			i32 id = ci.m_instancingRenderer->registerGraphicsInstance(prevGraphicsShapeIndex, position, orn, color, scaling);
			i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass, position, orn, colIndex, index, false);

			index++;
		}
	}
}

void GpuCompoundPlaneScene::createStaticEnvironment(const ConstructionInfo& ci)
{
	i32 index = 0;
	b3Vec3 normal = b3MakeVector3(0, 1, 0);
	float constant = 0.f;
	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);

	b3Vec4 scaling = b3MakeVector4(400, 1., 400, 1);

	//i32 colIndex = m_data->m_np->registerPlaneShape(normal,constant);//>registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
	i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0], strideInBytes, numVertices, scaling);
	b3Vec3 position = b3MakeVector3(0, 0, 0);
	b3Quat orn(0, 0, 0, 1);
	//		b3Quat orn(b3Vec3(1,0,0),0.3);
	b3Vec4 color = b3MakeVector4(0, 0, 1, 1);

	i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0], numVertices, cube_indices, numIndices);

	i32 id = ci.m_instancingRenderer->registerGraphicsInstance(shapeId, position, orn, color, scaling);
	i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(0.f, position, orn, colIndex, index, false);
}