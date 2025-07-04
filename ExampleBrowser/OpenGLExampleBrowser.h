#ifndef OPENGL_BROWSER_GUI_H
#define OPENGL_BROWSER_GUI_H

#include "ExampleBrowserInterface.h"

class OpenGLExampleBrowser : public ExampleBrowserInterface
{
	struct OpenGLExampleBrowserInternalData* m_internalData;

public:
	OpenGLExampleBrowser(class ExampleEntries* examples);
	virtual ~OpenGLExampleBrowser();

	virtual CommonExampleInterface* getCurrentExample();

	virtual bool init(i32 argc, tuk argv[]);

	virtual void update(float deltaTime);

	virtual void updateGraphics();

	virtual bool requestedExit();

	virtual void setSharedMemoryInterface(class SharedMemoryInterface* sharedMem);

	static void registerFileImporter(tukk extension, CommonExampleInterface::CreateFunc*
	                                                                              createFunc);
};

#endif  //OPENGL_BROWSER_GUI_H
