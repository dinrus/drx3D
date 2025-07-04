#include "GpuRigidBodyDemo.h"
#include "../OpenGLWindow/ShapeData.h"
#include "../OpenGLWindow/GLInstancingRenderer.h"
#include <drx3D/Common/b3Quat.h"
#include <drx3D/Common/Interfaces/CommonWindowInterface.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuSapBroadphase.h"
#include "Bullet3OpenCL/BroadphaseCollision/b3GpuGridBroadphase.h"

#include "../CommonOpenCL/GpuDemoInternalData.h"
#include "Bullet3OpenCL/Initialize/b3OpenCLUtils.h"
#include "../OpenGLWindow/OpenGLInclude.h"
#include "../OpenGLWindow/GLInstanceRendererInternalData.h"
#include "Bullet3OpenCL/ParallelPrimitives/b3LauncherCL.h"
#include "Bullet3OpenCL/RigidBody/b3GpuRigidBodyPipeline.h"
#include "Bullet3OpenCL/RigidBody/b3GpuNarrowPhase.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3Config.h"
#include "GpuRigidBodyDemoInternalData.h"
#include "Bullet3Collision/BroadPhaseCollision/b3DynamicBvhBroadphase.h"
#include "Bullet3Collision/NarrowPhaseCollision/shared/b3RigidBodyData.h"
#include "Bullet3OpenCL/RigidBody/b3GpuNarrowPhaseInternalData.h"
#include "stb_image/stb_image.h"
#include "../OpenGLWindow/GLPrimitiveRenderer.h"

extern i32 gPreferredOpenCLDeviceIndex;
extern i32 gPreferredOpenCLPlatformIndex;
extern i32 gGpuArraySizeX;
extern i32 gGpuArraySizeY;
extern i32 gGpuArraySizeZ;

static b3KeyboardCallback oldCallback = 0;
extern bool gReset;
bool useUniformGrid = false;
bool convertOnCpu = false;

#define MSTRINGIFY(A) #A

static tukk s_rigidBodyKernelString = MSTRINGIFY(

	typedef struct
	{
		float4 m_pos;
		float4 m_quat;
		float4 m_linVel;
		float4 m_angVel;
		u32 m_collidableIdx;
		float m_invMass;
		float m_restituitionCoeff;
		float m_frictionCoeff;
	} Body;

	__kernel void
		copyTransformsToVBOKernel(__global Body* gBodies, __global float4* posOrnColor, i32k numNodes) {
			i32 nodeID = get_global_id(0);
			if (nodeID < numNodes)
			{
				posOrnColor[nodeID] = (float4)(gBodies[nodeID].m_pos.xyz, 1.0);
				posOrnColor[nodeID + numNodes] = gBodies[nodeID].m_quat;
			}
		});

GpuRigidBodyDemo::GpuRigidBodyDemo(GUIHelperInterface* helper)
	: CommonOpenCLBase(helper),
	  m_instancingRenderer(0),
	  m_window(0)
{
	if (helper->getRenderInterface()->getInternalData())
	{
		m_instancingRenderer = (GLInstancingRenderer*)helper->getRenderInterface();
	}
	else
	{
		m_instancingRenderer = 0;
	}

	m_window = helper->getAppInterface()->m_window;

	m_data = new GpuRigidBodyDemoInternalData;
	m_data->m_guiHelper = helper;
}

void GpuRigidBodyDemo::resetCamera()
{
	float dist = 114;
	float pitch = -35;
	float yaw = 52;
	float targetPos[3] = {0, 0, 0};
	m_data->m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
}

GpuRigidBodyDemo::~GpuRigidBodyDemo()
{
	delete m_data;
}

static void PairKeyboardCallback(i32 key, i32 state)
{
	if (key == 'R' && state)
	{
		//gReset = true;
	}

	//b3DefaultKeyboardCallback(key,state);
	oldCallback(key, state);
}

void GpuRigidBodyDemo::setupScene()
{
}

void GpuRigidBodyDemo::initPhysics()
{
	initCL(gPreferredOpenCLDeviceIndex, gPreferredOpenCLPlatformIndex);

	m_guiHelper->setUpAxis(1);

	if (m_clData->m_clContext)
	{
		i32 errNum = 0;

		cl_program rbProg = 0;
		m_data->m_copyTransformsToVBOKernel = b3OpenCLUtils::compileCLKernelFromString(m_clData->m_clContext, m_clData->m_clDevice, s_rigidBodyKernelString, "copyTransformsToVBOKernel", &errNum, rbProg);

		m_data->m_config.m_maxConvexBodies = d3Max(m_data->m_config.m_maxConvexBodies, gGpuArraySizeX * gGpuArraySizeY * gGpuArraySizeZ + 10);
		m_data->m_config.m_maxConvexShapes = m_data->m_config.m_maxConvexBodies;
		i32 maxPairsPerBody = 16;
		m_data->m_config.m_maxBroadphasePairs = maxPairsPerBody * m_data->m_config.m_maxConvexBodies;
		m_data->m_config.m_maxContactCapacity = m_data->m_config.m_maxBroadphasePairs;

		b3GpuNarrowPhase* np = new b3GpuNarrowPhase(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue, m_data->m_config);
		b3GpuBroadphaseInterface* bp = 0;

		if (useUniformGrid)
		{
			bp = new b3GpuGridBroadphase(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue);
		}
		else
		{
			bp = new b3GpuSapBroadphase(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue);
		}
		m_data->m_np = np;
		m_data->m_bp = bp;
		m_data->m_broadphaseDbvt = new b3DynamicBvhBroadphase(m_data->m_config.m_maxConvexBodies);

		m_data->m_rigidBodyPipeline = new b3GpuRigidBodyPipeline(m_clData->m_clContext, m_clData->m_clDevice, m_clData->m_clQueue, np, bp, m_data->m_broadphaseDbvt, m_data->m_config);

		setupScene();

		m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();
		np->writeAllBodiesToGpu();
		bp->writeAabbsToGpu();
	}

	m_guiHelper->getRenderInterface()->writeTransforms();
}

void GpuRigidBodyDemo::exitPhysics()
{
	destroyScene();

	delete m_data->m_instancePosOrnColor;
	delete m_data->m_rigidBodyPipeline;
	delete m_data->m_broadphaseDbvt;

	delete m_data->m_np;
	m_data->m_np = 0;
	delete m_data->m_bp;
	m_data->m_bp = 0;

	exitCL();
}

void GpuRigidBodyDemo::renderScene()
{
	m_guiHelper->getRenderInterface()->renderScene();
}

void GpuRigidBodyDemo::stepSimulation(float deltaTime)
{
	if (!m_instancingRenderer)
		return;

	bool animate = true;
	i32 numObjects = m_data->m_rigidBodyPipeline->getNumBodies();
	//printf("numObjects=%d\n",numObjects);
	if (numObjects > m_instancingRenderer->getInstanceCapacity())
	{
		static bool once = true;
		if (once)
		{
			once = false;
			drx3DAssert(0);
			drx3DError("m_instancingRenderer out-of-memory\n");
		}
		numObjects = m_instancingRenderer->getInstanceCapacity();
	}

	GLint err = glGetError();
	assert(err == GL_NO_ERROR);

	b3Vec4* positions = 0;
	if (animate && numObjects)
	{
		D3_PROFILE("gl2cl");

		if (!m_data->m_instancePosOrnColor)
		{
			GLuint vbo = m_instancingRenderer->getInternalData()->m_vbo;
			i32 arraySizeInBytes = numObjects * (3) * sizeof(b3Vec4);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			//			cl_bool blocking=  CL_TRUE;
			positions = (b3Vec4*)glMapBufferRange(GL_ARRAY_BUFFER, m_instancingRenderer->getMaxShapeCapacity(), arraySizeInBytes, GL_MAP_READ_BIT);  //GL_READ_WRITE);//GL_WRITE_ONLY
			GLint err = glGetError();
			assert(err == GL_NO_ERROR);
			m_data->m_instancePosOrnColor = new b3OpenCLArray<b3Vec4>(m_clData->m_clContext, m_clData->m_clQueue);
			m_data->m_instancePosOrnColor->resize(3 * numObjects);
			m_data->m_instancePosOrnColor->copyFromHostPointer(positions, 3 * numObjects, 0);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			err = glGetError();
			assert(err == GL_NO_ERROR);
		}
	}

	{
		D3_PROFILE("stepSimulation");
		m_data->m_rigidBodyPipeline->stepSimulation(1. / 60.f);
	}

	if (numObjects)
	{
		if (convertOnCpu)
		{
			b3GpuNarrowPhaseInternalData* npData = m_data->m_np->getInternalData();
			npData->m_bodyBufferGPU->copyToHost(*npData->m_bodyBufferCPU);

			b3AlignedObjectArray<b3Vec4> vboCPU;
			m_data->m_instancePosOrnColor->copyToHost(vboCPU);

			for (i32 i = 0; i < numObjects; i++)
			{
				b3Vec4 pos = (const b3Vec4&)npData->m_bodyBufferCPU->at(i).m_pos;
				b3Quat orn = npData->m_bodyBufferCPU->at(i).m_quat;
				pos.w = 1.f;
				vboCPU[i] = pos;
				vboCPU[i + numObjects] = (b3Vec4&)orn;
			}
			m_data->m_instancePosOrnColor->copyFromHost(vboCPU);
		}
		else
		{
			D3_PROFILE("cl2gl_convert");
			i32 ciErrNum = 0;
			cl_mem bodies = m_data->m_rigidBodyPipeline->getBodyBuffer();
			b3LauncherCL launch(m_clData->m_clQueue, m_data->m_copyTransformsToVBOKernel, "m_copyTransformsToVBOKernel");
			launch.setBuffer(bodies);
			launch.setBuffer(m_data->m_instancePosOrnColor->getBufferCL());
			launch.setConst(numObjects);
			launch.launch1D(numObjects);
			oclCHECKERROR(ciErrNum, CL_SUCCESS);
		}
	}

	if (animate && numObjects)
	{
		D3_PROFILE("cl2gl_upload");
		GLint err = glGetError();
		assert(err == GL_NO_ERROR);
		GLuint vbo = m_instancingRenderer->getInternalData()->m_vbo;

		i32 arraySizeInBytes = numObjects * (3) * sizeof(b3Vec4);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//		cl_bool blocking=  CL_TRUE;
		positions = (b3Vec4*)glMapBufferRange(GL_ARRAY_BUFFER, m_instancingRenderer->getMaxShapeCapacity(), arraySizeInBytes, GL_MAP_WRITE_BIT);  //GL_READ_WRITE);//GL_WRITE_ONLY
		err = glGetError();
		assert(err == GL_NO_ERROR);
		m_data->m_instancePosOrnColor->copyToHostPointer(positions, 3 * numObjects, 0);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		err = glGetError();
		assert(err == GL_NO_ERROR);
	}
}

b3Vec3 GpuRigidBodyDemo::getRayTo(i32 x, i32 y)
{
	if (!m_instancingRenderer)
		return b3MakeVector3(0, 0, 0);

	float top = 1.f;
	float bottom = -1.f;
	float nearPlane = 1.f;
	float tanFov = (top - bottom) * 0.5f / nearPlane;
	float fov = b3Scalar(2.0) * b3Atan(tanFov);

	b3Vec3 camPos, camTarget;
	m_instancingRenderer->getActiveCamera()->getCameraPosition(camPos);
	m_instancingRenderer->getActiveCamera()->getCameraTargetPosition(camTarget);

	b3Vec3 rayFrom = camPos;
	b3Vec3 rayForward = (camTarget - camPos);
	rayForward.normalize();
	float farPlane = 10000.f;
	rayForward *= farPlane;

	//	b3Vec3 rightOffset;
	b3Vec3 m_cameraUp = b3MakeVector3(0, 1, 0);
	b3Vec3 vertical = m_cameraUp;

	b3Vec3 hor;
	hor = rayForward.cross(vertical);
	hor.normalize();
	vertical = hor.cross(rayForward);
	vertical.normalize();

	float tanfov = tanf(0.5f * fov);

	hor *= 2.f * farPlane * tanfov;
	vertical *= 2.f * farPlane * tanfov;

	b3Scalar aspect;
	float width = m_instancingRenderer->getScreenWidth();
	float height = m_instancingRenderer->getScreenHeight();

	aspect = width / height;

	hor *= aspect;

	b3Vec3 rayToCenter = rayFrom + rayForward;
	b3Vec3 dHor = hor * 1.f / width;
	b3Vec3 dVert = vertical * 1.f / height;

	b3Vec3 rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
	rayTo += b3Scalar(x) * dHor;
	rayTo -= b3Scalar(y) * dVert;
	return rayTo;
}

u8* GpuRigidBodyDemo::loadImage(tukk fileName, i32& width, i32& height, i32& n)
{
	u8* data = stbi_load(fileName, &width, &height, &n, 3);
	return data;
}

bool GpuRigidBodyDemo::keyboardCallback(i32 key, i32 state)
{
	if (m_data)
	{
		if (key == B3G_ALT)
		{
			m_data->m_altPressed = state;
		}
		if (key == B3G_CONTROL)
		{
			m_data->m_controlPressed = state;
		}
	}
	return false;
}

bool GpuRigidBodyDemo::mouseMoveCallback(float x, float y)
{
	if (!m_instancingRenderer)
		return false;

	if (m_data->m_altPressed != 0 || m_data->m_controlPressed != 0)
		return false;

	if (m_data->m_pickBody >= 0 && m_data->m_pickConstraint >= 0)
	{
		m_data->m_rigidBodyPipeline->removeConstraintByUid(m_data->m_pickConstraint);
		b3Vec3 newRayTo = getRayTo(x, y);
		b3Vec3 rayFrom;
		//	b3Vec3 oldPivotInB = m_data->m_pickPivotInB;
		b3Vec3 newPivotB;
		m_guiHelper->getRenderInterface()->getActiveCamera()->getCameraPosition(rayFrom);
		b3Vec3 dir = newRayTo - rayFrom;
		dir.normalize();
		dir *= m_data->m_pickDistance;
		newPivotB = rayFrom + dir;
		m_data->m_pickPivotInB = newPivotB;
		m_data->m_rigidBodyPipeline->copyConstraintsToHost();
		m_data->m_pickConstraint = m_data->m_rigidBodyPipeline->createPoint2PointConstraint(m_data->m_pickBody, m_data->m_pickFixedBody, m_data->m_pickPivotInA, m_data->m_pickPivotInB, 1e30);
		m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();
		return true;
	}
	return false;
}
bool GpuRigidBodyDemo::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	if (!m_instancingRenderer)
		return false;

	if (state == 1)
	{
		if (button == 0 && (m_data->m_altPressed == 0 && m_data->m_controlPressed == 0))
		{
			b3AlignedObjectArray<b3RayInfo> rays;
			b3AlignedObjectArray<b3RayHit> hitResults;
			b3Vec3 camPos;
			m_guiHelper->getRenderInterface()->getActiveCamera()->getCameraPosition(camPos);

			b3RayInfo ray;
			ray.m_from = camPos;
			ray.m_to = getRayTo(x, y);
			rays.push_back(ray);
			b3RayHit hit;
			hit.m_hitFraction = 1.f;
			hitResults.push_back(hit);
			m_data->m_rigidBodyPipeline->castRays(rays, hitResults);
			if (hitResults[0].m_hitFraction < 1.f)
			{
				i32 hitBodyA = hitResults[0].m_hitBody;
				if (m_data->m_np->getBodiesCpu()[hitBodyA].m_invMass)
				{
					//printf("hit!\n");
					m_data->m_np->readbackAllBodiesToCpu();
					m_data->m_pickBody = hitBodyA;

					//pivotInA
					b3Vec3 pivotInB;
					pivotInB.setInterpolate3(ray.m_from, ray.m_to, hitResults[0].m_hitFraction);
					b3Vec3 posA;
					b3Quat ornA;
					m_data->m_np->getObjectTransformFromCpu(posA, ornA, hitBodyA);
					b3Transform tr;
					tr.setOrigin(posA);
					tr.setRotation(ornA);
					b3Vec3 pivotInA = tr.inverse() * pivotInB;
					if (m_data->m_pickFixedBody < 0)
					{
						b3Vec3 pos = b3MakeVector3(0, 0, 0);
						b3Quat orn(0, 0, 0, 1);
						i32 fixedSphere = m_data->m_np->registerConvexHullShape(0, 0, 0, 0);  //>registerSphereShape(0.1);
						m_data->m_pickFixedBody = m_data->m_rigidBodyPipeline->registerPhysicsInstance(0, pos, orn, fixedSphere, 0, false);
						m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();
						m_data->m_bp->writeAabbsToGpu();

						if (m_data->m_pickGraphicsShapeIndex < 0)
						{
							i32 strideInBytes = 9 * sizeof(float);
							i32 numVertices = sizeof(point_sphere_vertices) / strideInBytes;
							i32 numIndices = sizeof(point_sphere_indices) / sizeof(i32);
							m_data->m_pickGraphicsShapeIndex = m_guiHelper->getRenderInterface()->registerShape(&point_sphere_vertices[0], numVertices, point_sphere_indices, numIndices, D3_GL_POINTS);

							float color[4] = {1, 0, 0, 1};
							float scaling[4] = {1, 1, 1, 1};

							m_data->m_pickGraphicsShapeInstance = m_guiHelper->getRenderInterface()->registerGraphicsInstance(m_data->m_pickGraphicsShapeIndex, pivotInB, orn, color, scaling);
							m_guiHelper->getRenderInterface()->writeTransforms();
							delete m_data->m_instancePosOrnColor;
							m_data->m_instancePosOrnColor = 0;
						}
						else
						{
							m_guiHelper->getRenderInterface()->writeSingleInstanceTransformToCPU(pivotInB, orn, m_data->m_pickGraphicsShapeInstance);
							if (this->m_instancingRenderer)
								m_instancingRenderer->writeSingleInstanceTransformToGPU(pivotInB, orn, m_data->m_pickGraphicsShapeInstance);
							m_data->m_np->setObjectTransformCpu(pos, orn, m_data->m_pickFixedBody);
						}
					}
					pivotInB.w = 0.f;
					m_data->m_pickPivotInA = pivotInA;
					m_data->m_pickPivotInB = pivotInB;
					m_data->m_rigidBodyPipeline->copyConstraintsToHost();
					m_data->m_pickConstraint = m_data->m_rigidBodyPipeline->createPoint2PointConstraint(hitBodyA, m_data->m_pickFixedBody, pivotInA, pivotInB, 1e30);  //hitResults[0].m_hitResult0
					m_data->m_rigidBodyPipeline->writeAllInstancesToGpu();
					m_data->m_np->writeAllBodiesToGpu();
					m_data->m_pickDistance = (pivotInB - camPos).length();

					return true;
				}
			}
		}
	}
	else
	{
		if (button == 0)
		{
			if (m_data->m_pickConstraint >= 0)
			{
				m_data->m_rigidBodyPipeline->removeConstraintByUid(m_data->m_pickConstraint);
				m_data->m_pickConstraint = -1;
			}
		}
	}

	//printf("button=%d, state=%d\n",button,state);
	return false;
}
