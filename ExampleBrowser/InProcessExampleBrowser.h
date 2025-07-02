#ifndef IN_PROCESS_EXAMPLE_BROWSER_H
#define IN_PROCESS_EXAMPLE_BROWSER_H

#include <drxtypes.h>

struct InProcessExampleBrowserInternalData;

InProcessExampleBrowserInternalData* CreateInProcessExampleBrowser(i32 argc, tuk* argv2,
                                                                    bool useInProcessMemory);

bool IsExampleBrowserTerminated(InProcessExampleBrowserInternalData* data);

void ShutDownExampleBrowser(InProcessExampleBrowserInternalData* data);

class SharedMemoryInterface* GetSharedMemoryInterface(InProcessExampleBrowserInternalData* data);

///////////////////////

struct InProcessExampleBrowserMainThreadInternalData;

InProcessExampleBrowserMainThreadInternalData* CreateInProcessExampleBrowserMainThread(i32 argc,
                                                            tuk* argv, bool useInProcessMemory);

bool IsExampleBrowserMainThreadTerminated(InProcessExampleBrowserMainThreadInternalData* data);

void UpdateInProcessExampleBrowserMainThread(InProcessExampleBrowserMainThreadInternalData* data);

void ShutDownExampleBrowserMainThread(InProcessExampleBrowserMainThreadInternalData* data);

class SharedMemoryInterface* GetSharedMemoryInterfaceMainThread
                                          (InProcessExampleBrowserMainThreadInternalData* data);

//////////////////////

#endif  //IN_PROCESS_EXAMPLE_BROWSER_H
