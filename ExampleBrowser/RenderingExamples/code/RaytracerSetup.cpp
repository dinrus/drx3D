#include "../RaytracerSetup.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include <drx3D/Physics/Collision/NarrowPhase/SubSimplexConvexCast.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>

struct RaytracerPhysicsSetup : public CommonExampleInterface
{
	struct CommonGraphicsApp* m_app;
	struct RaytracerInternalData* m_internalData;

	RaytracerPhysicsSetup(struct CommonGraphicsApp* app);

	virtual ~RaytracerPhysicsSetup();

	virtual void initPhysics();

	virtual void exitPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void physicsDebugDraw(i32 debugFlags);

	virtual void syncPhysicsToGraphics(struct GraphicsPhysicsBridge& gfxBridge);

	///worldRaytest performs a ray versus all objects in a collision world, returning true is a hit is found (filling in worldNormal and worldHitPoint)
	bool worldRaytest(const Vec3& rayFrom, const Vec3& rayTo, Vec3& worldNormal, Vec3& worldHitPoint);

	///singleObjectRaytest performs a ray versus one collision shape, returning true is a hit is found (filling in worldNormal and worldHitPoint)
	bool singleObjectRaytest(const Vec3& rayFrom, const Vec3& rayTo, Vec3& worldNormal, Vec3& worldHitPoint);

	///lowlevelRaytest performs a ray versus convex shape, returning true is a hit is found (filling in worldNormal and worldHitPoint)
	bool lowlevelRaytest(const Vec3& rayFrom, const Vec3& rayTo, Vec3& worldNormal, Vec3& worldHitPoint);

	virtual bool mouseMoveCallback(float x, float y);

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y);

	virtual bool keyboardCallback(i32 key, i32 state);

	virtual void renderScene()
	{
	}
};

struct RaytracerInternalData
{
	i32 m_canvasIndex;
	struct Common2dCanvasInterface* m_canvas;

	i32 m_width;
	i32 m_height;

	AlignedObjectArray<ConvexShape*> m_shapePtr;
	AlignedObjectArray<Transform2> m_transforms;
	VoronoiSimplexSolver m_simplexSolver;
	Scalar m_pitch;
	Scalar m_roll;
	Scalar m_yaw;

	RaytracerInternalData()
		: m_canvasIndex(-1),
		  m_canvas(0),
#ifdef _DEBUG
		  m_width(64),
		  m_height(64),
#else
		  m_width(128),
		  m_height(128),
#endif
		  m_pitch(0),
		  m_roll(0),
		  m_yaw(0)
	{
		ConeShape* cone = new ConeShape(1, 1);
		SphereShape* sphere = new SphereShape(1);
		BoxShape* box = new BoxShape(Vec3(1, 1, 1));
		m_shapePtr.push_back(cone);
		m_shapePtr.push_back(sphere);
		m_shapePtr.push_back(box);

		updateTransforms();
	}
	void updateTransforms()
	{
		i32 numObjects = m_shapePtr.size();
		m_transforms.resize(numObjects);
		for (i32 i = 0; i < numObjects; i++)
		{
			m_transforms[i].setIdentity();
			Vec3 pos(0.f, 0.f, -(2.5 * numObjects * 0.5) + i * 2.5f);
			m_transforms[i].setIdentity();
			m_transforms[i].setOrigin(pos);
			Quat orn;
			if (i < 2)
			{
				orn.setEuler(m_yaw, m_pitch, m_roll);
				m_transforms[i].setRotation(orn);
			}
		}
		m_pitch += 0.005f;
		m_yaw += 0.01f;
	}
};

RaytracerPhysicsSetup::RaytracerPhysicsSetup(struct CommonGraphicsApp* app)
{
	m_app = app;
	m_internalData = new RaytracerInternalData;
}

RaytracerPhysicsSetup::~RaytracerPhysicsSetup()
{
	delete m_internalData;
}

void RaytracerPhysicsSetup::initPhysics()
{
	//request a visual bitma/texture we can render to

	m_internalData->m_canvas = m_app->m_2dCanvasInterface;

	if (m_internalData->m_canvas)
	{
		m_internalData->m_canvasIndex = m_internalData->m_canvas->createCanvas("raytracer", m_internalData->m_width, m_internalData->m_height, 15, 55);
		for (i32 i = 0; i < m_internalData->m_width; i++)
		{
			for (i32 j = 0; j < m_internalData->m_height; j++)
			{
				u8 red = 255;
				u8 green = 255;
				u8 blue = 255;
				u8 alpha = 255;
				m_internalData->m_canvas->setPixel(m_internalData->m_canvasIndex, i, j, red, green, blue, alpha);
			}
		}
		m_internalData->m_canvas->refreshImageData(m_internalData->m_canvasIndex);

		//i32 bitmapId = gfxBridge.createRenderBitmap(width,height);
	}
}

///worldRaytest performs a ray versus all objects in a collision world, returning true is a hit is found (filling in worldNormal and worldHitPoint)
bool RaytracerPhysicsSetup::worldRaytest(const Vec3& rayFrom, const Vec3& rayTo, Vec3& worldNormal, Vec3& worldHitPoint)
{
	return false;
}

///singleObjectRaytest performs a ray versus one collision shape, returning true is a hit is found (filling in worldNormal and worldHitPoint)
bool RaytracerPhysicsSetup::singleObjectRaytest(const Vec3& rayFrom, const Vec3& rayTo, Vec3& worldNormal, Vec3& worldHitPoint)
{
	return false;
}

///lowlevelRaytest performs a ray versus convex shape, returning true is a hit is found (filling in worldNormal and worldHitPoint)
bool RaytracerPhysicsSetup::lowlevelRaytest(const Vec3& rayFrom, const Vec3& rayTo, Vec3& worldNormal, Vec3& worldHitPoint)
{
	Scalar closestHitResults = 1.f;

	bool hasHit = false;
	ConvexCast::CastResult rayResult;
	SphereShape pointShape(0.0f);
	Transform2 rayFromTrans;
	Transform2 rayToTrans;

	rayFromTrans.setIdentity();
	rayFromTrans.setOrigin(rayFrom);
	rayToTrans.setIdentity();
	rayToTrans.setOrigin(rayTo);

	i32 numObjects = m_internalData->m_shapePtr.size();

	for (i32 s = 0; s < numObjects; s++)
	{
		//do some culling, ray versus aabb
		Vec3 aabbMin, aabbMax;
		m_internalData->m_shapePtr[s]->getAabb(m_internalData->m_transforms[s], aabbMin, aabbMax);
		Scalar hitLambda = 1.f;
		Vec3 hitNormal;
		CollisionObject2 tmpObj;
		tmpObj.setWorldTransform(m_internalData->m_transforms[s]);

		if (RayAabb(rayFrom, rayTo, aabbMin, aabbMax, hitLambda, hitNormal))
		{
			//reset previous result

			//choose the continuous collision detection method
			SubsimplexConvexCast convexCaster(&pointShape, m_internalData->m_shapePtr[s], &m_internalData->m_simplexSolver);
			//GjkConvexCast convexCaster(&pointShape,shapePtr[s],&simplexSolver);
			//ContinuousConvexCollision convexCaster(&pointShape,shapePtr[s],&simplexSolver,0);

			if (convexCaster.calcTimeOfImpact(rayFromTrans, rayToTrans, m_internalData->m_transforms[s], m_internalData->m_transforms[s], rayResult))
			{
				if (rayResult.m_fraction < closestHitResults)
				{
					closestHitResults = rayResult.m_fraction;

					worldNormal = m_internalData->m_transforms[s].getBasis() * rayResult.m_normal;
					worldNormal.normalize();
					hasHit = true;
				}
			}
		}
	}

	return hasHit;
}

void RaytracerPhysicsSetup::exitPhysics()
{
	if (m_internalData->m_canvas && m_internalData->m_canvasIndex >= 0)
	{
		m_internalData->m_canvas->destroyCanvas(m_internalData->m_canvasIndex);
	}
}

void RaytracerPhysicsSetup::stepSimulation(float deltaTime)
{
	m_internalData->updateTransforms();

	float top = 1.f;
	float bottom = -1.f;
	float nearPlane = 1.f;

	float tanFov = (top - bottom) * 0.5f / nearPlane;

	float fov = 2.0 * atanf(tanFov);

	Vec3 cameraPosition(5, 0, 0);
	Vec3 cameraTargetPosition(0, 0, 0);

	if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
	{
		m_app->m_renderer->getActiveCamera()->getCameraPosition(cameraPosition);
		m_app->m_renderer->getActiveCamera()->getCameraTargetPosition(cameraTargetPosition);
	}

	Vec3 rayFrom = cameraPosition;
	Vec3 rayForward = cameraTargetPosition - cameraPosition;

	rayForward.normalize();
	float farPlane = 600.f;
	rayForward *= farPlane;

	Vec3 rightOffset;
	Vec3 vertical(0.f, 1.f, 0.f);
	Vec3 hor;
	hor = rayForward.cross(vertical);
	hor.normalize();
	vertical = hor.cross(rayForward);
	vertical.normalize();

	float tanfov = tanf(0.5f * fov);

	hor *= 2.f * farPlane * tanfov;
	vertical *= 2.f * farPlane * tanfov;

	Vec3 rayToCenter = rayFrom + rayForward;

	Vec3 dHor = hor * 1.f / float(m_internalData->m_width);
	Vec3 dVert = vertical * 1.f / float(m_internalData->m_height);

	//	i32	mode = 0;
	i32 x, y;

	for (x = 0; x < m_internalData->m_width; x++)
	{
		for (y = 0; y < m_internalData->m_height; y++)
		{
			Vec4 rgba(0, 0, 0, 0);
			Vec3 rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
			rayTo += x * dHor;
			rayTo -= y * dVert;
			Vec3 worldNormal(0, 0, 0);
			Vec3 worldPoint(0, 0, 0);

			bool hasHit = false;
			i32 mode = 0;
			switch (mode)
			{
				case 0:
					hasHit = lowlevelRaytest(rayFrom, rayTo, worldNormal, worldPoint);
					break;
				case 1:
					hasHit = singleObjectRaytest(rayFrom, rayTo, worldNormal, worldPoint);
					break;
				case 2:
					hasHit = worldRaytest(rayFrom, rayTo, worldNormal, worldPoint);
					break;
				default:
				{
				}
			}

			if (hasHit)
			{
				float lightVec0 = worldNormal.dot(Vec3(0, -1, -1));  //0.4f,-1.f,-0.4f));
				float lightVec1 = worldNormal.dot(Vec3(-1, 0, -1));  //-0.4f,-1.f,-0.4f));

				rgba = Vec4(lightVec0, lightVec1, 0, 1.f);
				rgba.setMin(Vec3(1, 1, 1));
				rgba.setMax(Vec3(0.2, 0.2, 0.2));
				rgba[3] = 1.f;
				u8 red = rgba[0] * 255;
				u8 green = rgba[1] * 255;
				u8 blue = rgba[2] * 255;
				u8 alpha = 255;
				m_internalData->m_canvas->setPixel(m_internalData->m_canvasIndex, x, y, red, green, blue, alpha);
			}
			else
			{
				//	Vec4 rgba = raytracePicture->getPixel(x,y);
			}
			if (!rgba.length2())
			{
				m_internalData->m_canvas->setPixel(m_internalData->m_canvasIndex, x, y, 255, 0, 0, 255);
			}
		}
	}
	m_internalData->m_canvas->refreshImageData(m_internalData->m_canvasIndex);
}

void RaytracerPhysicsSetup::physicsDebugDraw(i32 debugDrawFlags)
{
}

bool RaytracerPhysicsSetup::mouseMoveCallback(float x, float y)
{
	return false;
}

bool RaytracerPhysicsSetup::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	return false;
}

bool RaytracerPhysicsSetup::keyboardCallback(i32 key, i32 state)
{
	return false;
}

void RaytracerPhysicsSetup::syncPhysicsToGraphics(GraphicsPhysicsBridge& gfxBridge)
{
}

CommonExampleInterface* RayTracerCreateFunc(struct CommonExampleOptions& options)
{
	return new RaytracerPhysicsSetup(options.m_guiHelper->getAppInterface());
}
