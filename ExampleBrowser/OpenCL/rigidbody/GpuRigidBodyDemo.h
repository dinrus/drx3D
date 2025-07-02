#ifndef GPU_RIGID_BODY_DEMO_H
#define GPU_RIGID_BODY_DEMO_H

#include <drx3D/Common/b3Vec3.h"
#include "../CommonOpenCL/CommonOpenCLBase.h"

class GpuRigidBodyDemo : public CommonOpenCLBase
{
protected:
	class GLInstancingRenderer* m_instancingRenderer;
	class GLPrimitiveRenderer* m_primRenderer;
	class CommonWindowInterface* m_window;

	struct GpuRigidBodyDemoInternalData* m_data;

public:
	GpuRigidBodyDemo(GUIHelperInterface* helper);
	virtual ~GpuRigidBodyDemo();

	virtual void initPhysics();

	virtual void setupScene();

	virtual void destroyScene(){};

	virtual void exitPhysics();

	virtual void renderScene();

	void resetCamera();

	virtual void stepSimulation(float deltaTime);

	//for picking
	b3Vec3 getRayTo(i32 x, i32 y);
	virtual bool mouseMoveCallback(float x, float y);
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y);
	virtual bool keyboardCallback(i32 key, i32 state);

	u8* loadImage(tukk fileName, i32& width, i32& height, i32& n);
};

#endif  //GPU_RIGID_BODY_DEMO_H
