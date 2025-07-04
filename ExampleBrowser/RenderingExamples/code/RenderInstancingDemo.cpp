#ifndef RENDER_INSTANCING_DEMO_H
#define RENDER_INSTANCING_DEMO_H

#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>

///quick demo showing the right-handed coordinate system and positive rotations around each axis
class RenderInstancingDemo : public CommonExampleInterface
{
	CommonGraphicsApp* m_app;
	float m_x;
	float m_y;
	float m_z;
	b3AlignedObjectArray<i32> m_movingInstances;
	enum
	{
		numCubesX = 20,
		numCubesY = 20
	};

public:
	RenderInstancingDemo(CommonGraphicsApp* app)
		: m_app(app),
		  m_x(0),
		  m_y(0),
		  m_z(0)
	{
		m_app->setUpAxis(2);

		{
			b3Vec3 extents = b3MakeVector3(100, 100, 100);
			extents[m_app->getUpAxis()] = 1;

			i32 xres = 20;
			i32 yres = 20;

			b3Vec4 color0 = b3MakeVector4(0.1, 0.1, 0.1, 1);
			b3Vec4 color1 = b3MakeVector4(0.6, 0.6, 0.6, 1);
			m_app->registerGrid(xres, yres, color0, color1);
		}

		{
			i32 boxId = m_app->registerCubeShape(0.1, 0.1, 0.1);

			for (i32 i = -numCubesX / 2; i < numCubesX / 2; i++)
			{
				for (i32 j = -numCubesY / 2; j < numCubesY / 2; j++)
				{
					b3Vec3 pos = b3MakeVector3(i, j, j);
					pos[app->getUpAxis()] = 1;
					b3Quat orn(0, 0, 0, 1);
					b3Vec4 color = b3MakeVector4(0.3, 0.3, 0.3, 1);
					b3Vec3 scaling = b3MakeVector3(1, 1, 1);
					i32 instanceId = m_app->m_renderer->registerGraphicsInstance(boxId, pos, orn, color, scaling);
					m_movingInstances.push_back(instanceId);
				}
			}
		}

		m_app->m_renderer->writeTransforms();
	}
	virtual ~RenderInstancingDemo()
	{
	}

	virtual void physicsDebugDraw(i32 debugDrawMode)
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
		i32 index = 0;
		for (i32 i = -numCubesX / 2; i < numCubesX / 2; i++)
		{
			for (i32 j = -numCubesY / 2; j < numCubesY / 2; j++)
			{
				b3Vec3 pos = b3MakeVector3(i, j, j);
				pos[m_app->getUpAxis()] = 1 + 1 * b3Sin(m_x + i - j);
				float orn[4] = {0, 0, 0, 1};
				m_app->m_renderer->writeSingleInstanceTransformToCPU(pos, orn, m_movingInstances[index++]);
			}
		}
		m_app->m_renderer->writeTransforms();
	}
	virtual void renderScene()
	{
		m_app->m_renderer->renderScene();
	}

	virtual void physicsDebugDraw()
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
		float dist = 13;
		float pitch = -13;
		float yaw = 50;
		float targetPos[3] = {-1, 0, -0.3};
		if (m_app->m_renderer && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0], targetPos[1], targetPos[2]);
		}
	}
};

class CommonExampleInterface* RenderInstancingCreateFunc(struct CommonExampleOptions& options)
{
	return new RenderInstancingDemo(options.m_guiHelper->getAppInterface());
}

#endif  //RENDER_INSTANCING_DEMO_H
