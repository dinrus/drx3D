#ifndef COMMON_EXAMPLE_INTERFACE_H
#define COMMON_EXAMPLE_INTERFACE_H

#include <drxtypes.h>

struct CommandProcessorCreationInterface
{
	virtual ~CommandProcessorCreationInterface() {}
	virtual class CommandProcessorInterface* createCommandProcessor() = 0;
	virtual void deleteCommandProcessor(CommandProcessorInterface*) = 0;
};

struct CommonExampleOptions
{
	struct GUIHelperInterface* m_guiHelper;

	//Those are optional, some examples will use them others don't. Each example should work with them being 0.
	i32 m_option;
	tukk m_fileName;
	class SharedMemoryInterface* m_sharedMem;
	CommandProcessorCreationInterface* m_commandProcessorCreation;
	bool m_skipGraphicsUpdate;

	CommonExampleOptions(struct GUIHelperInterface* helper, i32 option = 0)
		: m_guiHelper(helper),
		  m_option(option),
		  m_fileName(0),
		  m_sharedMem(0),
		  m_commandProcessorCreation(0),
		  m_skipGraphicsUpdate(false)
	{
	}
};

class CommonExampleInterface
{
public:
	typedef class CommonExampleInterface*(CreateFunc)(CommonExampleOptions& options);

	virtual ~CommonExampleInterface()
	{
	}

	virtual void initPhysics() = 0;
	virtual void exitPhysics() = 0;
	virtual void updateGraphics() {}
	virtual void stepSimulation(float deltaTime) = 0;
	virtual void renderScene() = 0;
	virtual void physicsDebugDraw(i32 debugFlags) = 0;  //for now we reuse the flags in drx3D/src/LinearMath/IDebugDraw.h
	//reset camera is only called when switching demo. this way you can restart (initPhysics) and watch in a specific location easier
	virtual void resetCamera(){};
	virtual bool mouseMoveCallback(float x, float y) = 0;
	virtual bool mouseButtonCallback(i32 button, i32 state, float x, float y) = 0;
	virtual bool keyboardCallback(i32 key, i32 state) = 0;

	virtual void vrControllerMoveCallback(i32 controllerId, float pos[4],
	                        float orientation[4], float analogAxis, float auxAnalogAxes[10]) {}
	virtual void vrControllerButtonCallback(i32 controllerId, i32 button, i32 state, float pos[4],
	                                                                      float orientation[4]) {}
	virtual void vrHMDMoveCallback(i32 controllerId, float pos[4], float orientation[4]) {}
	virtual void vrGenericTrackerMoveCallback(i32 controllerId, float pos[4], float orientation[4]) {}

	virtual void processCommandLineArgs(i32 argc, tuk argv[]){};
};

class ExampleEntries
{
public:
	virtual ~ExampleEntries() {}

	virtual void initExampleEntries() = 0;

	virtual void initOpenCLExampleEntries() = 0;

	virtual i32 getNumRegisteredExamples() = 0;

	virtual CommonExampleInterface::CreateFunc* getExampleCreateFunc(i32 index) = 0;

	virtual tukk getExampleName(i32 index) = 0;

	virtual tukk getExampleDescription(i32 index) = 0;

	virtual i32 getExampleOption(i32 index) = 0;
};

CommonExampleInterface* StandaloneExampleCreateFunc(CommonExampleOptions& options);

#ifdef D3_USE_STANDALONE_EXAMPLE
#define D3_STANDALONE_EXAMPLE(ExampleFunc)                                             \
	CommonExampleInterface* StandaloneExampleCreateFunc(CommonExampleOptions& options) \
	{                                                                                  \
		return ExampleFunc(options);                                                   \
	}
#else  //D3_USE_STANDALONE_EXAMPLE
#define D3_STANDALONE_EXAMPLE(ExampleFunc)
#endif  //D3_USE_STANDALONE_EXAMPLE

#endif  //COMMON_EXAMPLE_INTERFACE_H
