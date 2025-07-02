#ifndef EMPTY_EXAMPLE_H
#define EMPTY_EXAMPLE_H

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>

class EmptyExample : public CommonExampleInterface
{
public:
	EmptyExample() {}
	virtual ~EmptyExample() {}

	static CommonExampleInterface* CreateFunc(struct CommonExampleOptions& /* unusedOptions*/)
	{
		return new EmptyExample;
	}

	virtual void initPhysics() {}
	virtual void exitPhysics() {}
	virtual void stepSimulation(float deltaTime) {}
	virtual void renderScene() {}
	virtual void physicsDebugDraw(i32 debugFlags) {}
	virtual bool mouseMoveCallback(float x, float y) { return false; }
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y) { return false; }
	virtual bool keyboardCallback(i32 key, i32 state) { return false; }
};

#endif  //EMPTY_EXAMPLE_H
