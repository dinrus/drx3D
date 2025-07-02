#include "../TimeSeriesExample.h"
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/CollisionCommon.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include "../TimeSeriesCanvas.h"

struct TimeSeriesExample : public CommonExampleInterface
{
	struct CommonGraphicsApp* m_app;
	struct TimeSeriesExampleInternalData* m_internalData;

	TimeSeriesExample(struct CommonGraphicsApp* app);

	virtual ~TimeSeriesExample();

	virtual void initPhysics();

	virtual void exitPhysics();

	virtual void stepSimulation(float deltaTime);

	virtual void physicsDebugDraw(i32 debugFlags);

	virtual void syncPhysicsToGraphics(struct GraphicsPhysicsBridge& gfxBridge);

	virtual bool mouseMoveCallback(float x, float y);

	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y);

	virtual bool keyboardCallback(i32 key, i32 state);

	virtual void renderScene()
	{
	}
};

struct TimeSeriesExampleInternalData
{
	TimeSeriesCanvas* m_timeSeriesCanvas;

	TimeSeriesExampleInternalData()
		: m_timeSeriesCanvas(0)
	{
	}
};

TimeSeriesExample::TimeSeriesExample(struct CommonGraphicsApp* app)
{
	m_app = app;
	m_internalData = new TimeSeriesExampleInternalData;
}

TimeSeriesExample::~TimeSeriesExample()
{
	delete m_internalData->m_timeSeriesCanvas;
	delete m_internalData;
}

void TimeSeriesExample::initPhysics()
{
	//request a visual bitma/texture we can render to

	m_internalData->m_timeSeriesCanvas = new TimeSeriesCanvas(m_app->m_2dCanvasInterface, 512, 512, "Test");
	m_internalData->m_timeSeriesCanvas->setupTimeSeries(3, 100, 0);
	m_internalData->m_timeSeriesCanvas->addDataSource("Some sine wave", 255, 0, 0);
	m_internalData->m_timeSeriesCanvas->addDataSource("Some cosine wave", 0, 255, 0);
	m_internalData->m_timeSeriesCanvas->addDataSource("Delta Time (*10)", 0, 0, 255);
	m_internalData->m_timeSeriesCanvas->addDataSource("Tan", 255, 0, 255);
	m_internalData->m_timeSeriesCanvas->addDataSource("Some cosine wave2", 255, 255, 0);
	m_internalData->m_timeSeriesCanvas->addDataSource("Empty source2", 255, 0, 255);
}

void TimeSeriesExample::exitPhysics()
{
}

void TimeSeriesExample::stepSimulation(float deltaTime)
{
	float time = m_internalData->m_timeSeriesCanvas->getCurrentTime();
	float v = sinf(time);
	m_internalData->m_timeSeriesCanvas->insertDataAtCurrentTime(v, 0, true);
	v = cosf(time);
	m_internalData->m_timeSeriesCanvas->insertDataAtCurrentTime(v, 1, true);
	v = tanf(time);
	m_internalData->m_timeSeriesCanvas->insertDataAtCurrentTime(v, 3, true);
	m_internalData->m_timeSeriesCanvas->insertDataAtCurrentTime(deltaTime * 10, 2, true);

	m_internalData->m_timeSeriesCanvas->nextTick();
}

void TimeSeriesExample::physicsDebugDraw(i32 debugDrawFlags)
{
}

bool TimeSeriesExample::mouseMoveCallback(float x, float y)
{
	return false;
}

bool TimeSeriesExample::mouseButtonCallback(i32 button, i32 state, float x, float y)
{
	return false;
}

bool TimeSeriesExample::keyboardCallback(i32 key, i32 state)
{
	return false;
}

void TimeSeriesExample::syncPhysicsToGraphics(GraphicsPhysicsBridge& gfxBridge)
{
}

CommonExampleInterface* TimeSeriesCreateFunc(struct CommonExampleOptions& options)
{
	return new TimeSeriesExample(options.m_guiHelper->getAppInterface());
}
