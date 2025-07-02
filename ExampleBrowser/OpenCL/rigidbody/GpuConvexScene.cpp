#include "GpuConvexScene.h"

#include "GpuRigidBodyDemo.h"
#include "../OpenGLWindow/ShapeData.h"

#include "../OpenGLWindow/GLInstancingRenderer.h"
#include <drx3D/Common/b3Quat.h"
#include <drx3D/Common/Interfaces/CommonWindowInterface.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuSapBroadphase.h"
#include "../CommonOpenCL/GpuDemoInternalData.h"
#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"
#include "../OpenGLWindow/OpenGLInclude.h"
#include "../OpenGLWindow/GLInstanceRendererInternalData.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3LauncherCL.h"
#include "Bullet3OpenCL/RigidBody/b3GpuRigidBodyPipeline.h"
#include "Bullet3OpenCL/RigidBody/b3GpuNarrowPhase.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3Config.h"
#include "GpuRigidBodyDemoInternalData.h"

#include "Bullet3Dynamics/ConstraintSolver/b3Point2PointConstraint.h"
#include "../OpenGLWindow/GLPrimitiveRenderer.h"
#include "Bullet3OpenCL/Raycast/b3GpuRaycast.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3ConvexUtility.h"
#include "Bullet3Dynamics/ConstraintSolver/b3FixedConstraint.h"

#include "../OpenGLWindow/GLRenderToTexture.h"

static bool gUseInstancedCollisionShapes = true;
extern i32 gGpuArraySizeX;
extern i32 gGpuArraySizeY;
extern i32 gGpuArraySizeZ;

#include "GpuRigidBodyDemo.h"
#include <drx3D/Common/b3AlignedObjectArray.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3RaycastInfo.h"

class GpuConvexScene : public GpuRigidBodyDemo
{
protected:
	class b3GpuRaycast* m_raycaster;

public:
	GpuConvexScene(GUIHelperInterface* helper)
		: GpuRigidBodyDemo(helper), m_raycaster(0)
	{
	}
	virtual ~GpuConvexScene() {}
	virtual tukk getName()
	{
		return "Tetrahedra";
	}

	virtual void setupScene();

	virtual void destroyScene();

	virtual i32 createDynamicsObjects();

	virtual i32 createDynamicsObjects2(const float* vertices, i32 numVertices, i32k* indices, i32 numIndices);

	virtual void createStaticEnvironment();
};

class GpuConvexPlaneScene : public GpuConvexScene
{
public:
	GpuConvexPlaneScene(GUIHelperInterface* helper)
		: GpuConvexScene(helper) {}
	virtual ~GpuConvexPlaneScene() {}
	virtual tukk getName()
	{
		return "ConvexOnPlane";
	}

	virtual void createStaticEnvironment();
};

class GpuBoxPlaneScene : public GpuConvexPlaneScene
{
public:
	GpuBoxPlaneScene(GUIHelperInterface* helper) : GpuConvexPlaneScene(helper) {}
	virtual ~GpuBoxPlaneScene() {}
	virtual tukk getName()
	{
		return "BoxBox";
	}

	virtual i32 createDynamicsObjects();
};

class GpuTetraScene : public GpuConvexScene
{
protected:
	void createFromTetGenData(tukk ele, tukk node);

public:
	virtual tukk getName()
	{
		return "TetraBreakable";
	}

	virtual i32 createDynamicsObjects();
};

b3Vec4 colors[4] =
	{
		b3MakeVector4(1, 0, 0, 1),
		b3MakeVector4(0, 1, 0, 1),
		b3MakeVector4(0, 1, 1, 1),
		b3MakeVector4(1, 1, 0, 1),
};

void GpuConvexScene::setupScene()
{
	m_raycaster = new b3GpuRaycast(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue);

	i32 index = 0;
	createStaticEnvironment();

	index += createDynamicsObjects();

	m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();

	float camPos[4] = {0, 0, 0, 0};  //ci.arraySizeX,ci.arraySizeY/2,ci.arraySizeZ,0};
	//float camPos[4]={1,12.5,1.5,0};

	m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraTargetPosition(camPos[0], camPos[1], camPos[2]);
	m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraDistance(150);
	//m_instancingRenderer->setCameraYaw(85);
	m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraYaw(225);
	m_guiHelper->getRenderInterface()->getActiveCamera()->setCameraPitch(-30);

	m_guiHelper->getRenderInterface()->updateCamera(1);  //>updateCamera();

	char msg[1024];
	i32 numInstances = index;
	sprintf(msg, "Num objects = %d", numInstances);
	drx3DPrintf(msg);

	//if (ci.m_gui)
	//	ci.m_gui->setStatusBarMessage(msg,true);
}

void GpuConvexScene::destroyScene()
{
	delete m_raycaster;
	m_raycaster = 0;
}

i32 GpuConvexScene::createDynamicsObjects()
{
	i32 strideInBytes = 9 * sizeof(float);
	/*i32 numVertices = sizeof(barrel_vertices)/strideInBytes;
	i32 numIndices = sizeof(barrel_indices)/sizeof(i32);
	return createDynamicsObjects2(ci,barrel_vertices,numVertices,barrel_indices,numIndices);
	*/

	i32 numVertices = sizeof(tetra_vertices) / strideInBytes;
	i32 numIndices = sizeof(tetra_indices) / sizeof(i32);
	return createDynamicsObjects2(tetra_vertices, numVertices, tetra_indices, numIndices);
}

i32 GpuBoxPlaneScene::createDynamicsObjects()
{
	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);
	return createDynamicsObjects2(cube_vertices_textured, numVertices, cube_indices, numIndices);
}

i32 GpuConvexScene::createDynamicsObjects2(const float* vertices, i32 numVertices, i32k* indices, i32 numIndices)
{
	i32 strideInBytes = 9 * sizeof(float);
	i32 textureIndex = -1;
	if (0)
	{
		i32 width, height, n;

		tukk filename = "data/cube.png";
		u8k* image = 0;

		tukk prefix[] = {"./", "../", "../../", "../../../", "../../../../"};
		i32 numprefix = sizeof(prefix) / sizeof(tukk);

		for (i32 i = 0; !image && i < numprefix; i++)
		{
			char relativeFileName[1024];
			sprintf(relativeFileName, "%s%s", prefix[i], filename);
			image = loadImage(relativeFileName, width, height, n);
		}

		drx3DAssert(image);
		if (image)
		{
			textureIndex = m_instancingRenderer->registerTexture(image, width, height);
		}
	}

	i32 shapeId = m_guiHelper->getRenderInterface()->registerShape(&vertices[0], numVertices, indices, numIndices, D3_GL_TRIANGLES, textureIndex);
	//i32 group=1;
	//i32 mask=1;
	i32 index = 0;

	{
		i32 curColor = 0;
		float scaling[4] = {1, 1, 1, 1};
		i32 prevBody = -1;
		//i32 insta = 0;

		b3ConvexUtility* utilPtr = new b3ConvexUtility();

		{
			b3AlignedObjectArray<b3Vec3> verts;

			u8* vts = (u8*)vertices;
			for (i32 i = 0; i < numVertices; i++)
			{
				float* vertex = (float*)&vts[i * strideInBytes];
				verts.push_back(b3MakeVector3(vertex[0] * scaling[0], vertex[1] * scaling[1], vertex[2] * scaling[2]));
			}

			bool merge = true;
			if (numVertices)
			{
				utilPtr->initializePolyhedralFeatures(&verts[0], verts.size(), merge);
			}
		}

		i32 colIndex = -1;
		if (gUseInstancedCollisionShapes)
			colIndex = m_data->m_np->registerConvexHullShape(utilPtr);

		//i32 colIndex = m_data->m_np->registerSphereShape(1);
		for (i32 i = 0; i < gGpuArraySizeX; i++)
		{
			//printf("%d of %d\n", i, ci.arraySizeX);
			for (i32 j = 0; j < gGpuArraySizeY; j++)
			{
				for (i32 k = 0; k < gGpuArraySizeZ; k++)
				{
					//i32 colIndex = m_data->m_np->registerConvexHullShape(&vertices[0],strideInBytes,numVertices, scaling);
					if (!gUseInstancedCollisionShapes)
						colIndex = m_data->m_np->registerConvexHullShape(utilPtr);

					float mass = 1.f;
					if (j == 0)  //ci.arraySizeY-1)
					{
						//mass=0.f;
					}
					b3Vec3 position = b3MakeVector3(((j + 1) & 1) + i * 2.2, 1 + j * 2., ((j + 1) & 1) + k * 2.2);
					//b3Vec3 position = b3MakeVector3(i*2,1+j*2,k*2);
					//b3Vec3 position=b3MakeVector3(1,0.9,1);
					b3Quat orn(0, 0, 0, 1);

					b3Vec4 color = colors[curColor];
					curColor++;
					curColor &= 3;
					//					b3Vec4 scaling=b3MakeVector4(1,1,1,1);
					i32 id;
					id = m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
					i32 pid;
					pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(mass, position, orn, colIndex, index, false);

					if (prevBody >= 0)
					{
						//b3Point2PointConstraint* p2p = new b3Point2PointConstraint(pid,prevBody,b3Vec3(0,-1.1,0),b3Vec3(0,1.1,0));
						//						 m_data->m_rigidBodyPipeline->addConstraint(p2p);//,false);
					}
					prevBody = pid;

					index++;
				}
			}
		}
		delete utilPtr;
	}
	return index;
}

void GpuConvexScene::createStaticEnvironment()
{
	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);
	//i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	i32 shapeId = m_instancingRenderer->registerShape(&cube_vertices[0], numVertices, cube_indices, numIndices);
	//i32 group=1;
	//i32 mask=1;
	i32 index = 0;

	{
		b3Vec4 scaling = b3MakeVector4(400, 400, 400, 1);
		i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0], strideInBytes, numVertices, scaling);
		b3Vec3 position = b3MakeVector3(0, -400, 0);
		b3Quat orn(0, 0, 0, 1);

		b3Vec4 color = b3MakeVector4(0, 0, 1, 1);

		i32 id;
		id = m_instancingRenderer->registerGraphicsInstance(shapeId, position, orn, color, scaling);
		i32 pid;
		pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(0.f, position, orn, colIndex, index, false);
	}
}

void GpuConvexPlaneScene::createStaticEnvironment()
{
	i32 strideInBytes = 9 * sizeof(float);
	i32 numVertices = sizeof(cube_vertices) / strideInBytes;
	i32 numIndices = sizeof(cube_indices) / sizeof(i32);
	//i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	i32 shapeId = m_guiHelper->getRenderInterface()->registerShape(&cube_vertices[0], numVertices, cube_indices, numIndices);
	//	i32 group=1;
	//	i32 mask=1;
	i32 index = 0;

	{
		b3Vec4 scaling = b3MakeVector4(400, 400, 400, 1);
		i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0], strideInBytes, numVertices, scaling);
		b3Vec3 position = b3MakeVector3(0, -400, 0);
		b3Quat orn(0, 0, 0, 1);

		b3Vec4 color = b3MakeVector4(0, 0, 1, 1);

		i32 id;
		id = m_guiHelper->getRenderInterface()->registerGraphicsInstance(shapeId, position, orn, color, scaling);
		i32 pid;
		pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(0.f, position, orn, colIndex, index, false);
	}
}

/*
void GpuConvexPlaneScene::createStaticEnvironment(const ConstructionInfo& ci)
{
	i32 strideInBytes = 9*sizeof(float);
	i32 numVertices = sizeof(cube_vertices)/strideInBytes;
	i32 numIndices = sizeof(cube_indices)/sizeof(i32);
	//i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	i32 shapeId = ci.m_instancingRenderer->registerShape(&cube_vertices[0],numVertices,cube_indices,numIndices);
	i32 group=1;
	i32 mask=1;
	i32 index=0;

	
	{
		b3Vec4 scaling=b3MakeVector4(100,0.001,100,1);
		
	
		//i32 colIndex = m_data->m_np->registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
		b3Vec3 normal=b3MakeVector3(0,1,0);
		float constant=0.f;
		i32 colIndex = m_data->m_np->registerPlaneShape(normal,constant);//>registerConvexHullShape(&cube_vertices[0],strideInBytes,numVertices, scaling);
		b3Vec3 position=b3MakeVector3(0,0,0);
		

		
		b3Quat orn(0,0,0,1);

		b3Vec4 color=b3MakeVector4(0,0,1,1);

		i32 id = ci.m_instancingRenderer->registerGraphicsInstance(shapeId,position,orn,color,scaling);
		i32 pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(0.f,position,orn,colIndex,index,false);

	}

}
*/

struct TetraBunny
{
#include "bunny.inl"
};

struct TetraCube
{
#include "cube.inl"
};

static i32 nextLine(tukk buffer)
{
	i32 numBytesRead = 0;

	while (*buffer != '\n')
	{
		buffer++;
		numBytesRead++;
	}

	if (buffer[0] == 0x0a)
	{
		buffer++;
		numBytesRead++;
	}
	return numBytesRead;
}

static float mytetra_vertices[] =
	{
		-1.f, 0, -1.f, 0.5f, 0, 1, 0, 0, 0,
		-1.f, 0, 1.f, 0.5f, 0, 1, 0, 1, 0,
		1.f, 0, 1.f, 0.5f, 0, 1, 0, 1, 1,
		1.f, 0, -1.f, 0.5f, 0, 1, 0, 0, 1};

static i32 mytetra_indices[] =
	{
		0, 1, 2,
		3, 1, 2, 3, 2, 0,
		3, 0, 1};

/* Create from TetGen .ele, .face, .node data */
void GpuTetraScene::createFromTetGenData(tukk ele,
										 tukk node)
{
	b3Scalar scaling(10);

	b3AlignedObjectArray<b3Vec3> pos;
	i32 nnode = 0;
	i32 ndims = 0;
	i32 nattrb = 0;
	i32 hasbounds = 0;
	i32 result = sscanf(node, "%d %d %d %d", &nnode, &ndims, &nattrb, &hasbounds);
	result = sscanf(node, "%d %d %d %d", &nnode, &ndims, &nattrb, &hasbounds);
	node += nextLine(node);

	//b3AlignedObjectArray<b3Vec3> rigidBodyPositions;
	//b3AlignedObjectArray<i32> rigidBodyIds;

	pos.resize(nnode);
	for (i32 i = 0; i < pos.size(); ++i)
	{
		i32 index = 0;
		//i32			bound=0;
		float x, y, z;
		sscanf(node, "%d %f %f %f", &index, &x, &y, &z);

		//	sn>>index;
		//	sn>>x;sn>>y;sn>>z;
		node += nextLine(node);

		//for(i32 j=0;j<nattrb;++j)
		//	sn>>a;

		//if(hasbounds)
		//	sn>>bound;

		pos[index].setX(b3Scalar(x) * scaling);
		pos[index].setY(b3Scalar(y) * scaling);
		pos[index].setZ(b3Scalar(z) * scaling);
	}

	if (ele && ele[0])
	{
		i32 ntetra = 0;
		i32 ncorner = 0;
		i32 neattrb = 0;
		sscanf(ele, "%d %d %d", &ntetra, &ncorner, &neattrb);
		ele += nextLine(ele);

		//se>>ntetra;se>>ncorner;se>>neattrb;
		for (i32 i = 0; i < ntetra; ++i)
		{
			i32 index = 0;
			i32 ni[4];

			//se>>index;
			//se>>ni[0];se>>ni[1];se>>ni[2];se>>ni[3];
			sscanf(ele, "%d %d %d %d %d", &index, &ni[0], &ni[1], &ni[2], &ni[3]);
			ele += nextLine(ele);

			b3Vec3 average = b3MakeVector3(0, 0, 0);

			for (i32 v = 0; v < 4; v++)
			{
				average += pos[ni[v]];
			}
			average /= 4;

			for (i32 v = 0; v < 4; v++)
			{
				b3Vec3 shiftedPos = pos[ni[v]] - average;
				mytetra_vertices[0 + v * 9] = shiftedPos.getX();
				mytetra_vertices[1 + v * 9] = shiftedPos.getY();
				mytetra_vertices[2 + v * 9] = shiftedPos.getZ();
			}
			//todo: subtract average

			i32 strideInBytes = 9 * sizeof(float);
			i32 numVertices = sizeof(mytetra_vertices) / strideInBytes;
			i32 numIndices = sizeof(mytetra_indices) / sizeof(i32);
			i32 shapeId = m_instancingRenderer->registerShape(&mytetra_vertices[0], numVertices, mytetra_indices, numIndices);
			//	i32 group=1;
			//	i32 mask=1;

			{
				b3Vec4 scaling = b3MakeVector4(1, 1, 1, 1);
				i32 colIndex = m_data->m_np->registerConvexHullShape(&mytetra_vertices[0], strideInBytes, numVertices, scaling);
				b3Vec3 position = b3MakeVector3(0, 150, 0);
				//				position+=average;//*1.2;//*2;
				position += average * 1.2;  //*2;
				//rigidBodyPositions.push_back(position);
				b3Quat orn(0, 0, 0, 1);

				static i32 curColor = 0;
				b3Vec4 color = colors[curColor++];
				curColor &= 3;

				i32 id;
				id = m_instancingRenderer->registerGraphicsInstance(shapeId, position, orn, color, scaling);
				i32 pid;
				pid = m_data->m_rigidBodyPipeline->registerPhysicsInstance(1.f, position, orn, colIndex, 0, false);
				//rigidBodyIds.push_back(pid);
			}

			//for(i32 j=0;j<neattrb;++j)
			//	se>>a;
			//psb->appendTetra(ni[0],ni[1],ni[2],ni[3]);
		}
		//	printf("Nodes:  %u\r\n",psb->m_nodes.size());
		//	printf("Links:  %u\r\n",psb->m_links.size());
		//	printf("Faces:  %u\r\n",psb->m_faces.size());
		//	printf("Tetras: %u\r\n",psb->m_tetras.size());
	}

	m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();
	m_data->m_np->writeAllBodiesToGpu();
	m_data->m_bp->writeAabbsToGpu();
	m_data->m_rigidBodyPipeline->setupGpuAabbsFull();
	m_data->m_bp->calculateOverlappingPairs(m_data->m_config.m_maxBroadphasePairs);

	i32 numPairs = m_data->m_bp->getNumOverlap();
	cl_mem pairs = m_data->m_bp->getOverlappingPairBuffer();
	b3OpenCLArray<b3Int2> clPairs(m_clData->m_clContext, m_clData->m_clQueue);
	clPairs.setFromOpenCLBuffer(pairs, numPairs);
	b3AlignedObjectArray<b3Int2> allPairs;
	clPairs.copyToHost(allPairs);

	for (i32 p = 0; p < allPairs.size(); p++)
	{
		b3Vec3 posA, posB;
		b3Quat ornA, ornB;
		i32 bodyIndexA = allPairs[p].x;
		i32 bodyIndexB = allPairs[p].y;

		m_data->m_np->getObjectTransformFromCpu(posA, ornA, bodyIndexA);
		m_data->m_np->getObjectTransformFromCpu(posB, ornB, bodyIndexB);

		b3Vec3 pivotWorld = (posA + posB) * 0.5f;
		b3Transform transA, transB;
		transA.setIdentity();
		transA.setOrigin(posA);
		transA.setRotation(ornA);
		transB.setIdentity();
		transB.setOrigin(posB);
		transB.setRotation(ornB);
		b3Vec3 pivotInA = transA.inverse() * pivotWorld;
		b3Vec3 pivotInB = transB.inverse() * pivotWorld;

		b3Transform frameInA, frameInB;
		frameInA.setIdentity();
		frameInB.setIdentity();
		frameInA.setOrigin(pivotInA);
		frameInB.setOrigin(pivotInB);
		b3Quat relTargetAB = frameInA.getRotation() * frameInB.getRotation().inverse();

		//c = new b3FixedConstraint(pid,prevBody,frameInA,frameInB);
		float breakingThreshold = 45;  //37.f;
		//c->setBreakingImpulseThreshold(37.1);
		bool useGPU = true;
		if (useGPU)
		{
			i32 cid;
			cid = m_data->m_rigidBodyPipeline->createFixedConstraint(bodyIndexA, bodyIndexB, pivotInA, pivotInB, relTargetAB, breakingThreshold);
		}
		else
		{
			b3FixedConstraint* c = new b3FixedConstraint(bodyIndexA, bodyIndexB, frameInA, frameInB);
			c->setBreakingImpulseThreshold(breakingThreshold);
			m_data->m_rigidBodyPipeline->addConstraint(c);
		}
	}

	printf("numPairs = %d\n", numPairs);
}

i32 GpuTetraScene::createDynamicsObjects()
{
	//createFromTetGenData(TetraCube::getElements(),TetraCube::getNodes());
	createFromTetGenData(TetraBunny::getElements(), TetraBunny::getNodes());

	return 0;
}

class CommonExampleInterface* OpenCLBoxBoxCreateFunc(struct CommonExampleOptions& options)
{
	return new GpuBoxPlaneScene(options.m_guiHelper);
}
