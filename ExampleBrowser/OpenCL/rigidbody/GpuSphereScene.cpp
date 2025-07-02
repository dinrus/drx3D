#include "GpuSphereScene.h"
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
#include "Bullet3AppSupport/gwenUserInterface.h"

void GpuSphereScene::setupScene(const ConstructionInfo& ci)
{
	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);
	//i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);

	i32 group = 1;
	i32 mask = 1;
	i32 index = 0;
	bool writeInstanceToGpu = false;

	if (0)
	{
		float radius = 60;
		i32 prevGraphicsShapeIndex = -1;
		{
			if (1)  //radius>=100)
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

		i32 curColor = 0;

		//i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
		i32 colIndex = m_data->m_np->registerSphereShape(radius);  //>registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
		float mass = 0.f;

		//b3Vec3 position((j&1)+i*2.2,1+j*2.,(j&1)+k*2.2);
		b3Vec3 position = b3MakeVector3(0, 0, 0);

		b3Quat orn(0, 0, 0, 1);

		b3Vec4 color = colors[curColor];
		curColor++;
		curColor &= 3;
		b3Vec4 scaling = b3MakeVector4(radius, radius, radius, 1);
		i32 id = ci.m_instancingRenderer->registerGraphicsInstance(prevGraphicsShapeIndex, position, orn, color, scaling);
		i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass, position, orn, colIndex, index, writeInstanceToGpu);

		index++;
	}

	b3Vec4 colors[4] =
		{
			b3MakeVector4(1, 0, 0, 1),
			b3MakeVector4(0, 1, 0, 1),
			b3MakeVector4(0, 1, 1, 1),
			b3MakeVector4(1, 1, 0, 1),
		};

	i32 curColor = 0;
	float radius = 61;
	//i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
	i32 colIndex = m_data->m_np->registerSphereShape(radius);  //>registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
	i32 prevGraphicsShapeIndex = registerGraphicsSphereShape(ci, radius, false);

	//for (i32 i=0;i<ci.arraySizeX;i++)
	{
		//	for (i32 j=0;j<ci.arraySizeY;j++)
		{
			//		for (i32 k=0;k<ci.arraySizeZ;k++)
			{
				i32 i = 0, j = 0, k = 0;
				float mass = 0.f;

				b3Vec3 position = b3MakeVector3(0, 0, 0);
				//b3Vec3 position((j&1)+i*142.2,-51+j*142.,(j&1)+k*142.2);
				//b3Vec3 position(0,-41,0);//0,0,0);//i*radius*3,-41+j*radius*3,k*radius*3);

				b3Quat orn(0, 0, 0, 1);

				b3Vec4 color = colors[curColor];
				curColor++;
				curColor &= 3;
				b3Vec4 scaling = b3MakeVector4(radius, radius, radius, 1);
				i32 id = ci.m_instancingRenderer->registerGraphicsInstance(prevGraphicsShapeIndex, position, orn, color, scaling);
				i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass, position, orn, colIndex, index, writeInstanceToGpu);

				index++;
			}
		}
	}

	if (1)
	{
		i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0], numVertices, cube_indices, numIndices);
		b3Vec4 scaling = b3MakeVector4(0.5, 0.5, 0.5, 1);  //1,1,1,1);//0.1,0.1,0.1,1);
		i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0], strideInBytes, numVertices, scaling);
		b3Vec3 normal = b3MakeVector3(0, -1, 0);
		float constant = 2;

		for (i32 j = -10; j < 10; j++)
			for (i32 i = -10; i < 10; i++)
				for (i32 k = 0; k < 30; k++)
				//i32 i=0;i32 j=0;
				{
					//i32 colIndex = m_data->m_np->registerPlaneShape(normal,constant);//>registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
					b3Vec4 position = b3MakeVector4(2 * i, 70 + k * 2, 2 * j + 8, 0);
					//b3Quat orn(0,0,0,1);
					b3Quat orn(b3MakeVector3(1, 0, 0), 0.3);

					b3Vec4 color = b3MakeVector4(0, 0, 1, 1);

					i32 id = ci.m_instancingRenderer->registerGraphicsInstance(shapeId, position, orn, color, scaling);
					i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(1.f, position, orn, colIndex, index, false);

					index++;
				}
	}

	if (!writeInstanceToGpu)
	{
		m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();
	}

	float camPos[4] = {ci.arraySizeX, ci.arraySizeY / 2, ci.arraySizeZ, 0};
	//float camPos[4]={1,12.5,1.5,0};
	m_instancingRenderer->setCameraTargetPosition(camPos);
	m_instancingRenderer->setCameraDistance(130);

	char msg[1024];
	i32 numInstances = index;
	sprintf(msg, "Num objects = %d", numInstances);
	ci.m_gui->setStatusBarMessage(msg, true);
}
