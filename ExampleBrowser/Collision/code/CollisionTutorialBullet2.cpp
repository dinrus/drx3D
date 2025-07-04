
#include "../CollisionTutorialBullet2.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/Transform2.h>

#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include "../../RenderingExamples/TimeSeriesCanvas.h"
#include <X/stb/stb_image.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3Matrix3x3.h>
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include "../CollisionSdkC_Api.h"
#include <drx3D/Maths/Linear/Quickprof.h>

///Not Invented Here link reminder http://www.joelonsoftware.com/articles/fog0000000007.html

///todo: use the 'userData' to prevent this use of global variables
static i32 gTotalPoints = 0;
i32k sPointCapacity = 10000;
i32k sNumCompounds = 10;
i32k sNumSpheres = 10;

lwContactPoint pointsOut[sPointCapacity];
i32 numNearCallbacks = 0;
static Vec4 sColors[4] =
	{
		Vec4(1, 0.7, 0.7, 1),
		Vec4(1, 1, 0.7, 1),
		Vec4(0.7, 1, 0.7, 1),
		Vec4(0.7, 1, 1, 1),
};

void myNearCallback(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, uk userData, plCollisionObjectHandle objA, plCollisionObjectHandle objB)
{
	numNearCallbacks++;
	i32 remainingCapacity = sPointCapacity - gTotalPoints;
	Assert(remainingCapacity > 0);

	if (remainingCapacity > 0)
	{
		lwContactPoint* pointPtr = &pointsOut[gTotalPoints];
		i32 numNewPoints = plCollide(sdkHandle, worldHandle, objA, objB, pointPtr, remainingCapacity);
		Assert(numNewPoints <= remainingCapacity);
		gTotalPoints += numNewPoints;
	}
}

class CollisionTutorialBullet2 : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	GUIHelperInterface* m_guiHelper;
	i32 m_tutorialIndex;

	TimeSeriesCanvas* m_timeSeriesCanvas0;

	plCollisionSdkHandle m_collisionSdkHandle;
	plCollisionWorldHandle m_collisionWorldHandle;

	//	i32 m_stage;
	//	i32 m_counter;

public:
	CollisionTutorialBullet2(GUIHelperInterface* guiHelper, i32 tutorialIndex)
		: m_app(guiHelper->getAppInterface()),
		  m_guiHelper(guiHelper),
		  m_tutorialIndex(tutorialIndex),
		  m_timeSeriesCanvas0(0),
		  m_collisionSdkHandle(0),
		  m_collisionWorldHandle(0)
	//	m_stage(0),
	//	m_counter(0)
	{
		gTotalPoints = 0;
		m_app->setUpAxis(1);

		switch (m_tutorialIndex)
		{
			case TUT_SPHERE_PLANE_RTB3:
			case TUT_SPHERE_PLANE_BULLET2:
			{
				if (m_tutorialIndex == TUT_SPHERE_PLANE_BULLET2)
				{
					m_collisionSdkHandle = plCreateBullet2CollisionSdk();
				}
				else
				{
#ifndef DISABLE_REAL_TIME_BULLET3_COLLISION_SDK
					m_collisionSdkHandle = plCreateRealTimeBullet3CollisionSdk();
#endif  //DISABLE_REAL_TIME_BULLET3_COLLISION_SDK
				}
				if (m_collisionSdkHandle)
				{
					i32 maxNumObjsCapacity = 1024;
					i32 maxNumShapesCapacity = 1024;
					i32 maxNumPairsCapacity = 16384;
					AlignedObjectArray<plCollisionObjectHandle> colliders;
					m_collisionWorldHandle = plCreateCollisionWorld(m_collisionSdkHandle, maxNumObjsCapacity, maxNumShapesCapacity, maxNumPairsCapacity);
					//create objects, do query etc
					{
						float radius = 1.f;

						uk userPointer = 0;
						{
							for (i32 j = 0; j < sNumCompounds; j++)
							{
								plCollisionShapeHandle compoundShape = plCreateCompoundShape(m_collisionSdkHandle, m_collisionWorldHandle);

								for (i32 i = 0; i < sNumSpheres; i++)
								{
									Vec3 childPos(i * 1.5, 0, 0);
									Quat childOrn(0, 0, 0, 1);

									Vec3 scaling(radius, radius, radius);

									plCollisionShapeHandle childShape = plCreateSphereShape(m_collisionSdkHandle, m_collisionWorldHandle, radius);
									plAddChildShape(m_collisionSdkHandle, m_collisionWorldHandle, compoundShape, childShape, childPos, childOrn);

									//m_guiHelper->createCollisionObjectGraphicsObject(colObj,color);
								}
								if (m_tutorialIndex == TUT_SPHERE_PLANE_BULLET2)
								{
									CollisionShape* colShape = (CollisionShape*)compoundShape;
									m_guiHelper->createCollisionShapeGraphicsObject(colShape);
								}
								else
								{
								}

								{
									Vec3 pos(j * sNumSpheres * 1.5, -2.4, 0);
									Quat orn(0, 0, 0, 1);
									plCollisionObjectHandle colObjHandle = plCreateCollisionObject(m_collisionSdkHandle, m_collisionWorldHandle, userPointer, -1, compoundShape, pos, orn);
									if (m_tutorialIndex == TUT_SPHERE_PLANE_BULLET2)
									{
										CollisionObject2* colObj = (CollisionObject2*)colObjHandle;
										Vec4 color = sColors[j & 3];
										m_guiHelper->createCollisionObjectGraphicsObject(colObj, color);
										colliders.push_back(colObjHandle);
										plAddCollisionObject(m_collisionSdkHandle, m_collisionWorldHandle, colObjHandle);
									}
								}
							}
						}
					}

					{
						plCollisionShapeHandle colShape = plCreatePlaneShape(m_collisionSdkHandle, m_collisionWorldHandle, 0, 1, 0, -3.5);
						Vec3 pos(0, 0, 0);
						Quat orn(0, 0, 0, 1);
						uk userPointer = 0;
						plCollisionObjectHandle colObj = plCreateCollisionObject(m_collisionSdkHandle, m_collisionWorldHandle, userPointer, 0, colShape, pos, orn);
						colliders.push_back(colObj);
						plAddCollisionObject(m_collisionSdkHandle, m_collisionWorldHandle, colObj);
					}

					i32 numContacts = plCollide(m_collisionSdkHandle, m_collisionWorldHandle, colliders[0], colliders[1], pointsOut, sPointCapacity);
					printf("numContacts = %d\n", numContacts);
					uk myUserPtr = 0;

					plWorldCollide(m_collisionSdkHandle, m_collisionWorldHandle, myNearCallback, myUserPtr);
					printf("total points=%d\n", gTotalPoints);

					//plRemoveCollisionObject(m_collisionSdkHandle,m_collisionWorldHandle,colObj);
					//plDeleteCollisionObject(m_collisionSdkHandle,colObj);
					//plDeleteShape(m_collisionSdkHandle,colShape);
				}

				/*
				m_timeSeriesCanvas0 = new TimeSeriesCanvas(m_app->m_2dCanvasInterface,512,256,"Constant Velocity");
				
				m_timeSeriesCanvas0 ->setupTimeSeries(2,60, 0);
				m_timeSeriesCanvas0->addDataSource("X position (m)", 255,0,0);
				m_timeSeriesCanvas0->addDataSource("X velocity (m/s)", 0,0,255);
				m_timeSeriesCanvas0->addDataSource("dX/dt (m/s)", 0,0,0);
				 */
				break;
			}

			default:
			{
				m_timeSeriesCanvas0 = new TimeSeriesCanvas(m_app->m_2dCanvasInterface, 512, 256, "Unknown");
				m_timeSeriesCanvas0->setupTimeSeries(1, 60, 0);
			}
		};

		{
			i32 boxId = m_app->registerCubeShape(100, 0.01, 100);
			b3Vec3 pos = b3MakeVector3(0, -3.5, 0);
			b3Quat orn(0, 0, 0, 1);
			b3Vec4 color = b3MakeVector4(1, 1, 1, 1);
			b3Vec3 scaling = b3MakeVector3(1, 1, 1);
			m_app->m_renderer->registerGraphicsInstance(boxId, pos, orn, color, scaling);
		}

		{
			i32 textureIndex = -1;

			if (1)
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
					image = stbi_load(relativeFileName, &width, &height, &n, 3);
				}

				drx3DAssert(image);
				if (image)
				{
					textureIndex = m_app->m_renderer->registerTexture(image, width, height);
				}
			}
		}

		m_app->m_renderer->writeTransforms();
	}
	virtual ~CollisionTutorialBullet2()
	{
		delete m_timeSeriesCanvas0;

		plDeleteCollisionWorld(m_collisionSdkHandle, m_collisionWorldHandle);

		plDeleteCollisionSdk(m_collisionSdkHandle);

		m_timeSeriesCanvas0 = 0;
	}

	virtual void initPhysics()
	{
	}
	virtual void exitPhysics()
	{
	}

	virtual void stepSimulation(float deltaTime)
	{
#ifndef DRX3D_NO_PROFILE
		CProfileManager::Reset();
#endif

		uk myUserPtr = 0;

		gTotalPoints = 0;
		numNearCallbacks = 0;
		{
			DRX3D_PROFILE("plWorldCollide");
			if (m_collisionSdkHandle && m_collisionWorldHandle)
			{
				plWorldCollide(m_collisionSdkHandle, m_collisionWorldHandle, myNearCallback, myUserPtr);
			}
		}

#if 0
		switch (m_tutorialIndex)
		{
			case TUT_SPHERE_SPHERE:
			{
				if (m_timeSeriesCanvas0)
				{
					float xPos = 0.f;
					float xVel = 1.f;
					m_timeSeriesCanvas0->insertDataAtCurrentTime(xPos,0,true);
					m_timeSeriesCanvas0->insertDataAtCurrentTime(xVel,1,true);
				}
				break;
			}
			

			default:
			{
			}
			
		};
#endif

		if (m_timeSeriesCanvas0)
			m_timeSeriesCanvas0->nextTick();

		//	m_app->m_renderer->writeSingleInstanceTransformToCPU(m_bodies[i]->m_worldPose.m_position, m_bodies[i]->m_worldPose.m_orientation, m_bodies[i]->m_graphicsIndex);

		m_app->m_renderer->writeTransforms();
#ifndef DRX3D_NO_PROFILE
		CProfileManager::Increment_Frame_Counter();
#endif
	}
	virtual void renderScene()
	{
		if (m_app && m_app->m_renderer)
		{
			m_app->m_renderer->renderScene();

			m_app->m_renderer->clearZBuffer();

			m_app->drawText3D("X", 1, 0, 0, 1);
			m_app->drawText3D("Y", 0, 1, 0, 1);
			m_app->drawText3D("Z", 0, 0, 1, 1);

			for (i32 i = 0; i < gTotalPoints; i++)
			{
				const lwContactPoint& contact = pointsOut[i];
				Vec3 color(1, 1, 0);
				Scalar lineWidth = 3;
				if (contact.m_distance < 0)
				{
					color.setVal(1, 0, 0);
				}
				m_app->m_renderer->drawLine(contact.m_ptOnAWorld, contact.m_ptOnBWorld, color, lineWidth);
			}
		}
	}

	virtual void physicsDebugDraw(i32 debugDrawFlags)
	{
	}
	virtual bool mouseMoveCallback(float x, float y)
	{
		return false;
	}
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y)
	{
		return false;
	}
	virtual bool keyboardCallback(i32 key, i32 state)
	{
		return false;
	}

	virtual void resetCamera()
	{
		float dist = 10.5;
		float pitch = -32;
		float yaw = 136;
		float targetPos[3] = {0, 0, 0};
		if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
		}
	}
};

class CommonExampleInterface* CollisionTutorialBullet2CreateFunc(struct CommonExampleOptions& options)
{
	return new CollisionTutorialBullet2(options.m_guiHelper, options.m_option);
}
