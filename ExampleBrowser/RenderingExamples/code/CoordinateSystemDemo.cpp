
#include "../CoordinateSystemDemo.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
///quick demo showing the right-handed coordinate system and positive rotations around each axis
class CoordinateSystemDemo : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	float m_x;
	float m_y;
	float m_z;

public:
	CoordinateSystemDemo(CommonGraphicsApp* app)
		: m_app(app),
		  m_x(0),
		  m_y(0),
		  m_z(0)
	{
		m_app->setUpAxis(2);

		{
			i32 boxId = m_app->registerCubeShape(0.1, 0.1, 0.1);
			Vec3 pos(0, 0, 0);
			Quat orn(0, 0, 0, 1);
			Vec4 color(0.3, 0.3, 0.3, 1);
			Vec3 scaling(1, 1, 1);
			m_app->m_renderer->registerGraphicsInstance(boxId, pos, orn, color, scaling);
		}

		m_app->m_renderer->writeTransforms();
	}
	virtual ~CoordinateSystemDemo()
	{
	}

	virtual void initPhysics()
	{
	}
	virtual void exitPhysics()
	{
	}
	virtual void stepSimulation(float deltaTime)
	{
		m_x += 0.01f;
		m_y += 0.01f;
		m_z += 0.01f;
	}
	virtual void renderScene()
	{
		m_app->m_renderer->renderScene();
		m_app->drawText3D("X", 1, 0, 0, 1);
		m_app->drawText3D("Y", 0, 1, 0, 1);
		m_app->drawText3D("Z", 0, 0, 1, 1);
	}

	virtual void drawArc(const Vec3& center, const Vec3& normal, const Vec3& axis, Scalar radiusA, Scalar radiusB, Scalar minAngle, Scalar maxAngle,
						 const Vec3& color, bool drawSect, Scalar stepDegrees = Scalar(10.f))
	{
		Scalar lineWidth = 3;
		const Vec3& vx = axis;
		Vec3 vy = normal.cross(axis);
		Scalar step = stepDegrees * SIMD_RADS_PER_DEG;
		i32 nSteps = (i32)Fabs((maxAngle - minAngle) / step);
		if (!nSteps) nSteps = 1;
		Vec3 prev = center + radiusA * vx * Cos(minAngle) + radiusB * vy * Sin(minAngle);
		if (drawSect)
		{
			m_app->m_renderer->drawLine(center, prev, color, lineWidth);
		}
		for (i32 i = 1; i <= nSteps; i++)
		{
			Scalar angle = minAngle + (maxAngle - minAngle) * Scalar(i) / Scalar(nSteps);
			Vec3 next = center + radiusA * vx * Cos(angle) + radiusB * vy * Sin(angle);
			m_app->m_renderer->drawLine(prev, next, color, lineWidth);
			prev = next;
		}
		if (drawSect)
		{
			m_app->m_renderer->drawLine(center, prev, color, lineWidth);
		}
	}

	virtual void physicsDebugDraw(i32 debugDrawFlags)
	{
		Vec3 xUnit(1, 0, 0);
		Vec3 yUnit(0, 1, 0);
		Vec3 zUnit(0, 0, 1);

		Scalar lineWidth = 3;

		Quat rotAroundX(xUnit, m_x);
		Quat rotAroundY(yUnit, m_y);
		Quat rotAroundZ(zUnit, m_z);

		Scalar radius = 0.5;
		Vec3 toX = radius * quatRotate(rotAroundX, yUnit);
		Vec3 toY = radius * quatRotate(rotAroundY, xUnit);
		Vec3 toZ = radius * quatRotate(rotAroundZ, xUnit);

		m_app->m_renderer->drawLine(xUnit + toX + quatRotate(rotAroundX, Vec3(0, 0.1, -0.2)), xUnit + toX, xUnit, lineWidth);
		m_app->m_renderer->drawLine(xUnit + toX + quatRotate(rotAroundX, Vec3(0, -0.2, -0.2)), xUnit + toX, xUnit, lineWidth);
		//draw the letter 'x' on the x-axis
		//m_app->m_renderer->drawLine(xUnit-0.1*zUnit+0.1*yUnit,xUnit+0.1*zUnit-0.1*yUnit,xUnit,lineWidth);
		//m_app->m_renderer->drawLine(xUnit+0.1*zUnit+0.1*yUnit,xUnit-0.1*zUnit-0.1*yUnit,xUnit,lineWidth);

		m_app->m_renderer->drawLine(xUnit + toX + quatRotate(rotAroundX, Vec3(0, -0.2, -0.2)), xUnit + toX, xUnit, lineWidth);

		m_app->m_renderer->drawLine(yUnit + toY + quatRotate(rotAroundY, Vec3(-0.2, 0, 0.2)), yUnit + toY, yUnit, lineWidth);
		m_app->m_renderer->drawLine(yUnit + toY + quatRotate(rotAroundY, Vec3(0.1, 0, 0.2)), yUnit + toY, yUnit, lineWidth);
		m_app->m_renderer->drawLine(zUnit + toZ + quatRotate(rotAroundZ, Vec3(0.1, -0.2, 0)), zUnit + toZ, zUnit, lineWidth);
		m_app->m_renderer->drawLine(zUnit + toZ + quatRotate(rotAroundZ, Vec3(-0.2, -0.2, 0)), zUnit + toZ, zUnit, lineWidth);

		drawArc(xUnit, xUnit, toX.normalized(), radius, radius, 0.4, SIMD_2_PI, xUnit, false);
		drawArc(yUnit, yUnit, toY.normalized(), radius, radius, 0.4, SIMD_2_PI, yUnit, false);
		drawArc(zUnit, zUnit, toZ.normalized(), radius, radius, 0.4, SIMD_2_PI, zUnit, false);
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
		float dist = 3.5;
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

CommonExampleInterface* CoordinateSystemCreateFunc(struct CommonExampleOptions& options)
{
	return new CoordinateSystemDemo(options.m_guiHelper->getAppInterface());
}
