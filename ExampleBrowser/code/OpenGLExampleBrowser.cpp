#include "../OpenGLExampleBrowser.h"
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/OpenGLWindow/OpenGLInclude.h>
//#include <drx3D/OpenGLWindow/SimpleOpenGL2App.h>
#ifndef NO_OPENGL3
#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#endif
#include <drx3D/Common/Interfaces/CommonRenderInterface.h>
#ifdef __APPLE__
#include <drx3D/OpenGLWindow/MacOpenGLWindow.h>
#else
#ifdef _WIN32
#include <drx3D/OpenGLWindow/Win32OpenGLWindow.h>
#else
//let's cross the fingers it is Linux/X11
#ifdef DRX3D_USE_EGL
#include <drx3D/OpenGLWindow/EGLOpenGLWindow.h>
#else
#include <drx3D/OpenGLWindow/X11OpenGLWindow.h>
#endif  //DRX3D_USE_EGL
#endif  //_WIN32
#endif  //__APPLE__
#include <X/Gwen/Renderers/OpenGL_DebugFont.h>
#include <drx3D/Maths/Linear/Threads.h>
#include <drx3D/Common/b3Vec3.h>
#include "assert.h"
#include <stdio.h>
#include "../GwenGUISupport/gwenInternalData.h"
#include "../GwenGUISupport/gwenUserInterface.h"
#include <drx3D/Common/b3Clock.h>
#include <drx3D/Common/ChromeTraceUtil.h>
#include "../GwenGUISupport/GwenParameterInterface.h"
#ifndef DRX3D_NO_PROFILE
#include "../GwenGUISupport/GwenProfileWindow.h"
#endif
#include "../GwenGUISupport/GwenTextureWindow.h"
#include "../GwenGUISupport/GraphingTexture.h"
#include <drx3D/Common/Interfaces/Common2dCanvasInterface.h>
#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/b3CommandLineArgs.h>
#include <drx3D/OpenGLWindow/SimpleCamera.h>
#include <drx3D/OpenGLWindow/SimpleOpenGL2Renderer.h>
#include "../ExampleEntries.h"
#include "../OpenGLGuiHelper.h"
#include <drx3D/Common/b3FileUtils.h>

#include <drx3D/Maths/Linear/IDebugDraw.h>
//quick test for file import, @todo(erwincoumans) make it more general and add other file formats
#include <drx3D/Importers/URDF/ImportURDFSetup.h>
#include <drx3D/Importers/Bullet/SerializeSetup.h>
#include <drx3D/Common/b3HashMap.h>

struct GL3TexLoader : public MyTextureLoader
{
	b3HashMap<b3HashString, GLint> m_hashMap;

	virtual void LoadTexture(Gwen::Texture* pTexture)
	{
		Gwen::Txt namestr = pTexture->name.Get();
		tukk n = ~namestr;
		GLint* texIdPtr = m_hashMap[n];
		if (texIdPtr)
		{
			pTexture->m_intData = *texIdPtr;
		}
	}
	virtual void FreeTexture(Gwen::Texture* pTexture)
	{
	}
};

struct OpenGLExampleBrowserInternalData
{
	Gwen::Renderer::Base* m_gwenRenderer;
	CommonGraphicsApp* m_app;
#ifndef DRX3D_NO_PROFILE
	MyProfileWindow* m_profWindow;
#endif  //DRX3D_NO_PROFILE
	AlignedObjectArray<Gwen::Controls::TreeNode*> m_nodes;
	GwenUserInterface* m_gui;
	GL3TexLoader* m_myTexLoader;
	struct MyMenuItemHander* m_handler2;
	AlignedObjectArray<MyMenuItemHander*> m_handlers;

	OpenGLExampleBrowserInternalData()
		: m_gwenRenderer(0),
		  m_app(0),
		  //		m_profWindow(0),
		  m_gui(0),
		  m_myTexLoader(0),
		  m_handler2(0)
	{
	}
};

static CommonGraphicsApp* s_app = 0;

static CommonWindowInterface* s_window = 0;
static CommonParameterInterface* s_parameterInterface = 0;
static CommonRenderInterface* s_instancingRenderer = 0;
static OpenGLGuiHelper* s_guiHelper = 0;
#ifndef DRX3D_NO_PROFILE
static MyProfileWindow* s_profWindow = 0;
#endif  //DRX3D_NO_PROFILE
static SharedMemoryInterface* sSharedMem = 0;

#define DEMO_SELECTION_COMBOBOX 13
tukk startFileName = "drx3DExBrowser.ini";
char staticPngFileName[1024];
//static GwenUserInterface* gui  = 0;
static GwenUserInterface* gui2 = 0;
static i32 sCurrentDemoIndex = -1;
static i32 sCurrentHightlighted = 0;
static CommonExampleInterface* sCurrentDemo = 0;
static b3AlignedObjectArray<tukk > allNames;
static float gFixedTimeStep = 0;
bool gAllowRetina = true;
bool gDisableDemoSelection = false;
i32 gRenderDevice = -1;
i32 gWindowBackend = 0;
static class ExampleEntries* gAllExamples = 0;
bool sUseOpenGL2 = false;
#ifndef USE_OPENGL3
extern bool useShadowMap;
#endif

bool visualWireframe = false;
static bool renderVisualGeometry = true;
static bool renderGrid = true;
static bool gEnableRenderLoop = true;

bool renderGui = true;
static bool enable_experimental_opencl = false;

static bool gEnableDefaultKeyboardShortcuts = true;
static bool gEnableDefaultMousePicking = true;

i32 gDebugDrawFlags = 0;
static bool pauseSimulation = false;
static bool singleStepSimulation = false;
i32 midiBaseIndex = 176;
extern bool gDisableDeactivation;

i32 gSharedMemoryKey = -1;

///some quick test variable for the OpenCL examples

i32 gPreferredOpenCLDeviceIndex = -1;
i32 gPreferredOpenCLPlatformIndex = -1;
i32 gGpuArraySizeX = 45;
i32 gGpuArraySizeY = 55;
i32 gGpuArraySizeZ = 45;

//#include <float.h>
//u32 fp_control_state = _controlfp(_EM_INEXACT, _MCW_EM);

void deleteDemo()
{
	if (sCurrentDemo)
	{
		sCurrentDemo->exitPhysics();
		s_instancingRenderer->removeAllInstances();
		delete sCurrentDemo;
		sCurrentDemo = 0;
		delete s_guiHelper;
		s_guiHelper = 0;

		//		CProfileManager::CleanupMemory();
	}
}

tukk gPngFileName = 0;
i32 gPngSkipFrames = 0;

b3KeyboardCallback prevKeyboardCallback = 0;

void MyKeyboardCallback(i32 key, i32 state)
{
	//drx3DPrintf("key=%d, state=%d", key, state);
	bool handled = false;
	if (renderGui)
	{
		if (gui2 && !handled)
		{
			handled = gui2->keyboardCallback(key, state);
		}
	}

	if (!handled && sCurrentDemo)
	{
		handled = sCurrentDemo->keyboardCallback(key, state);
	}

	//checkout: is it desired to ignore keys, if the demo already handles them?
	//if (handled)
	//	return;

	if (gEnableDefaultKeyboardShortcuts)
	{
		if (key == 'a' && state)
		{
			gDebugDrawFlags ^= IDebugDraw::DBG_DrawAabb;
		}
		if (key == 'c' && state)
		{
			gDebugDrawFlags ^= IDebugDraw::DBG_DrawContactPoints;
		}
		if (key == 'd' && state)
		{
			gDebugDrawFlags ^= IDebugDraw::DBG_NoDeactivation;
			gDisableDeactivation = ((gDebugDrawFlags & IDebugDraw::DBG_NoDeactivation) != 0);
		}
		if (key == 'j' && state)
		{
			gDebugDrawFlags ^= IDebugDraw::DBG_DrawFrames;
		}

		if (key == 'k' && state)
		{
			gDebugDrawFlags ^= IDebugDraw::DBG_DrawConstraints;
		}

		if (key == 'l' && state)
		{
			gDebugDrawFlags ^= IDebugDraw::DBG_DrawConstraintLimits;
		}
		if (key == 'w' && state)
		{
			visualWireframe = !visualWireframe;
			gDebugDrawFlags ^= IDebugDraw::DBG_DrawWireframe;
		}

		if (key == 'v' && state)
		{
			renderVisualGeometry = !renderVisualGeometry;
		}
		if (key == 'g' && state)
		{
			renderGrid = !renderGrid;
			renderGui = !renderGui;
		}

		if (key == 'i' && state)
		{
			pauseSimulation = !pauseSimulation;
		}
		if (key == 'o' && state)
		{
			singleStepSimulation = true;
		}

		if (key == 'p')
		{
			if (state)
			{
				b3ChromeUtilsStartTimings();
			}
			else
			{
#ifdef _WIN32
				b3ChromeUtilsStopTimingsAndWriteJsonFile("timings");
#else
				b3ChromeUtilsStopTimingsAndWriteJsonFile("/tmp/timings");
#endif
			}
		}

#ifndef NO_OPENGL3
		if (key == 's' && state)
		{
			useShadowMap = !useShadowMap;
		}
#endif
		if (key == B3G_F1)
		{
			static i32 count = 0;
			if (state)
			{
				drx3DPrintf("F1 нажата %d", count++);

				if (gPngFileName)
				{
					drx3DPrintf("отключене дамп изображений");

					gPngFileName = 0;
				}
				else
				{
					gPngFileName = gAllExamples->getExampleName(sCurrentDemoIndex);
					drx3DPrintf("включен дамп изображения %s", gPngFileName);
				}
			}
			else
			{
				drx3DPrintf("F1 отпущена %d", count++);
			}
		}
	}
	if (key == B3G_ESCAPE && s_window)
	{
		s_window->setRequestExit();
	}

	if (prevKeyboardCallback)
		prevKeyboardCallback(key, state);
}

b3MouseMoveCallback prevMouseMoveCallback = 0;
static void MyMouseMoveCallback(float x, float y)
{
	bool handled = false;
	if (sCurrentDemo)
		handled = sCurrentDemo->mouseMoveCallback(x, y);
	if (renderGui)
	{
		if (!handled && gui2)
			handled = gui2->mouseMoveCallback(x, y);
	}
	if (!handled)
	{
		if (prevMouseMoveCallback)
			prevMouseMoveCallback(x, y);
	}
}

b3MouseButtonCallback prevMouseButtonCallback = 0;

static void MyMouseButtonCallback(i32 button, i32 state, float x, float y)
{
	bool handled = false;
	//try picking first
	if (sCurrentDemo)
		handled = sCurrentDemo->mouseButtonCallback(button, state, x, y);

	if (renderGui)
	{
		if (!handled && gui2)
			handled = gui2->mouseButtonCallback(button, state, x, y);
	}
	if (!handled)
	{
		if (prevMouseButtonCallback)
			prevMouseButtonCallback(button, state, x, y);
	}
	//	b3DefaultMouseButtonCallback(button,state,x,y);
}

#include <string.h>
struct FileImporterByExtension
{
	STxt m_extension;
	CommonExampleInterface::CreateFunc* m_createFunc;
};

static AlignedObjectArray<FileImporterByExtension> gFileImporterByExtension;

void OpenGLExampleBrowser::registerFileImporter(tukk extension, CommonExampleInterface::CreateFunc* createFunc)
{
	FileImporterByExtension fi;
	fi.m_extension = extension;
	fi.m_createFunc = createFunc;
	gFileImporterByExtension.push_back(fi);
}
#include <drx3D/SharedMemory/SharedMemoryPublic.h>

void OpenGLExampleBrowserVisualizerFlagCallback(i32 flag, bool enable)
{
	if (flag == COV_ENABLE_Y_AXIS_UP)
	{
		//either Y = up or Z
		i32 upAxis = enable ? 1 : 2;
		s_app->setUpAxis(upAxis);
	}

	if (flag == COV_ENABLE_RENDERING)
	{
		gEnableRenderLoop = (enable != 0);
	}

	if (flag == COV_ENABLE_SINGLE_STEP_RENDERING)
	{
		if (enable)
		{
			gEnableRenderLoop = false;
			singleStepSimulation = true;
		}
		else
		{
			gEnableRenderLoop = true;
			singleStepSimulation = false;
		}
	}

	if (flag == COV_ENABLE_SHADOWS)
	{
		useShadowMap = enable;
	}
	if (flag == COV_ENABLE_GUI)
	{
		renderGui = enable;
		renderGrid = enable;
	}

	if (flag == COV_ENABLE_KEYBOARD_SHORTCUTS)
	{
		gEnableDefaultKeyboardShortcuts = enable;
	}
	if (flag == COV_ENABLE_MOUSE_PICKING)
	{
		gEnableDefaultMousePicking = enable;
	}

	if (flag == COV_ENABLE_WIREFRAME)
	{
		visualWireframe = enable;
		if (visualWireframe)
		{
			gDebugDrawFlags |= IDebugDraw::DBG_DrawWireframe;
		}
		else
		{
			gDebugDrawFlags &= ~IDebugDraw::DBG_DrawWireframe;
		}
	}
}

void openFileDemo(tukk filename)
{
	deleteDemo();

	s_guiHelper = new OpenGLGuiHelper(s_app, sUseOpenGL2);
	s_guiHelper->setVisualizerFlagCallback(OpenGLExampleBrowserVisualizerFlagCallback);

	s_parameterInterface->removeAllParameters();

	CommonExampleOptions options(s_guiHelper, 1);
	options.m_fileName = filename;
	char fullPath[1024];
	sprintf(fullPath, "%s", filename);
	b3FileUtils::toLower(fullPath);

	for (i32 i = 0; i < gFileImporterByExtension.size(); i++)
	{
		if (strstr(fullPath, gFileImporterByExtension[i].m_extension.c_str()))
		{
			sCurrentDemo = gFileImporterByExtension[i].m_createFunc(options);
		}
	}

	if (sCurrentDemo)
	{
		sCurrentDemo->initPhysics();
		sCurrentDemo->resetCamera();
	}
}

void selectDemo(i32 demoIndex)
{
	bool resetCamera = (sCurrentDemoIndex != demoIndex);
	sCurrentDemoIndex = demoIndex;
	sCurrentHightlighted = demoIndex;
	i32 numDemos = gAllExamples->getNumRegisteredExamples();

	if (demoIndex > numDemos)
	{
		demoIndex = 0;
	}
	deleteDemo();

	CommonExampleInterface::CreateFunc* func = gAllExamples->getExampleCreateFunc(demoIndex);
	if (func)
	{
		if (s_parameterInterface)
		{
			s_parameterInterface->removeAllParameters();
		}
		i32 option = gAllExamples->getExampleOption(demoIndex);
		s_guiHelper = new OpenGLGuiHelper(s_app, sUseOpenGL2);
		s_guiHelper->setVisualizerFlagCallback(OpenGLExampleBrowserVisualizerFlagCallback);

		CommonExampleOptions options(s_guiHelper, option);
		options.m_sharedMem = sSharedMem;
		sCurrentDemo = (*func)(options);
		if (sCurrentDemo)
		{
			if (gui2)
			{
				gui2->setStatusBarMessage("Status: OK", false);
			}
			drx3DPrintf("Выбрано демо: %s", gAllExamples->getExampleName(demoIndex));
			if (gui2)
			{
				gui2->setExampleDescription(gAllExamples->getExampleDescription(demoIndex));
			}

			sCurrentDemo->initPhysics();
			if (resetCamera)
			{
				sCurrentDemo->resetCamera();
			}
		}
	}
}

#include <stdio.h>

static void saveCurrentSettings(i32 currentEntry, tukk startFileName)
{
	FILE* f = fopen(startFileName, "w");
	if (f)
	{
		fprintf(f, "--start_demo_name=%s\n", gAllExamples->getExampleName(sCurrentDemoIndex));
		fprintf(f, "--mouse_move_multiplier=%f\n", s_app->getMouseMoveMultiplier());
		fprintf(f, "--mouse_wheel_multiplier=%f\n", s_app->getMouseWheelMultiplier());
		float red, green, blue;
		s_app->getBackgroundColor(&red, &green, &blue);
		fprintf(f, "--background_color_red= %f\n", red);
		fprintf(f, "--background_color_green= %f\n", green);
		fprintf(f, "--background_color_blue= %f\n", blue);
		fprintf(f, "--fixed_timestep= %f\n", gFixedTimeStep);
		if (!gAllowRetina)
		{
			fprintf(f, "--disable_retina");
		}

		if (enable_experimental_opencl)
		{
			fprintf(f, "--enable_experimental_opencl\n");
		}
		//		if (sUseOpenGL2 )
		//		{
		//			fprintf(f,"--opengl2\n");
		//		}

		fclose(f);
	}
};

static void loadCurrentSettings(tukk startFileName, b3CommandLineArgs& args)
{
	//i32 currentEntry= 0;
	FILE* f = fopen(startFileName, "r");
	if (f)
	{
		char oneline[1024];
		tuk argv[] = {0, &oneline[0]};

		while (fgets(oneline, 1024, f) != NULL)
		{
			tuk pos;
			if ((pos = strchr(oneline, '\n')) != NULL)
				*pos = '\0';
			args.addArgs(2, argv);
		}
		fclose(f);
	}
};

void MyComboBoxCallback(i32 comboId, tukk item)
{
	//printf("comboId = %d, item = %s\n",comboId, item);
	if (comboId == DEMO_SELECTION_COMBOBOX)
	{
		//find selected item
		for (i32 i = 0; i < allNames.size(); i++)
		{
			if (strcmp(item, allNames[i]) == 0)
			{
				selectDemo(i);
				saveCurrentSettings(sCurrentDemoIndex, startFileName);
				break;
			}
		}
	}
}

//in case of multi-threading, don't submit messages while the GUI is rendering (causing crashes)
static bool gBlockGuiMessages = false;

void MyGuiPrintf(tukk msg)
{
	printf("drx3DPrintf: %s\n", msg);
	if (!gDisableDemoSelection && !gBlockGuiMessages)
	{
		gui2->textOutput(msg);
		gui2->forceUpdateScrollBars();
	}
}

void MyStatusBarPrintf(tukk msg)
{
	printf("drx3DPrintf: %s\n", msg);
	if (!gDisableDemoSelection && !gBlockGuiMessages)
	{
		bool isLeft = true;
		gui2->setStatusBarMessage(msg, isLeft);
	}
}

void MyStatusBarError(tukk msg)
{
	printf("Предупреждение: %s\n", msg);
	if (!gDisableDemoSelection && !gBlockGuiMessages)
	{
		bool isLeft = false;
		gui2->setStatusBarMessage(msg, isLeft);
		gui2->textOutput(msg);
		gui2->forceUpdateScrollBars();
	}
	Assert(0);
}

struct MyMenuItemHander : public Gwen::Event::Handler
{
	i32 m_buttonId;

	MyMenuItemHander(i32 buttonId)
		: m_buttonId(buttonId)
	{
	}

	void onButtonA(Gwen::Controls::Base* pControl)
	{
		//const Gwen::Txt& name = pControl->GetName();
		Gwen::Controls::TreeNode* node = (Gwen::Controls::TreeNode*)pControl;
		//	Gwen::Controls::Label* l = node->GetButton();

		Gwen::WTxt la = node->GetButton()->GetText();  // node->GetButton()->GetName();// GetText();
		Gwen::Txt laa = la.ToTxt();
		//	tukk ha = laa.c_str();

		//printf("selected %s\n", ha);
		//i32 dep = but->IsDepressed();
		//i32 tog = but->GetToggleState();
		//		if (m_data->m_toggleButtonCallback)
		//		(*m_data->m_toggleButtonCallback)(m_buttonId, tog);
	}
	void onButtonB(Gwen::Controls::Base* pControl)
	{
		Gwen::Controls::Label* label = (Gwen::Controls::Label*)pControl;
		Gwen::WTxt la = label->GetText();  // node->GetButton()->GetName();// GetText();
		Gwen::Txt laa = la.ToTxt();
		//tukk ha = laa.c_str();

		if (!gDisableDemoSelection)
		{
			selectDemo(sCurrentHightlighted);
			saveCurrentSettings(sCurrentDemoIndex, startFileName);
		}
	}
	void onButtonC(Gwen::Controls::Base* pControl)
	{
		/*Gwen::Controls::Label* label = (Gwen::Controls::Label*) pControl;
		Gwen::UnicodeString la = label->GetText();// node->GetButton()->GetName();// GetText();
		Gwen::Txt laa = Gwen::Utility::UnicodeToString(la);
		tukk ha = laa.c_str();


		printf("onButtonC ! %s\n", ha);
		*/
	}
	void onButtonD(Gwen::Controls::Base* pControl)
	{
		/*		Gwen::Controls::Label* label = (Gwen::Controls::Label*) pControl;
		Gwen::UnicodeString la = label->GetText();// node->GetButton()->GetName();// GetText();
		Gwen::Txt laa = Gwen::Utility::UnicodeToString(la);
		tukk ha = laa.c_str();
		*/

		//	printf("onKeyReturn ! \n");
		if (!gDisableDemoSelection)
		{
			selectDemo(sCurrentHightlighted);
			saveCurrentSettings(sCurrentDemoIndex, startFileName);
		}
	}

	void onButtonE(Gwen::Controls::Base* pControl)
	{
		//	printf("select %d\n",m_buttonId);
		sCurrentHightlighted = m_buttonId;
		gui2->setExampleDescription(gAllExamples->getExampleDescription(sCurrentHightlighted));
	}

	void onButtonF(Gwen::Controls::Base* pControl)
	{
		//printf("selection changed!\n");
	}

	void onButtonG(Gwen::Controls::Base* pControl)
	{
		//printf("onButtonG !\n");
	}
};

void quitCallback()
{
	s_window->setRequestExit();
}

void fileOpenCallback()
{
	char filename[1024];
	i32 len = s_window->fileOpenDialog(filename, 1024);
	if (len)
	{
		//todo(erwincoumans) check if it is actually URDF
		//printf("file open:%s\n", filename);
		openFileDemo(filename);
	}
}

#define MAX_GRAPH_WINDOWS 5

struct QuickCanvas : public Common2dCanvasInterface
{
	GL3TexLoader* m_myTexLoader;

	MyGraphWindow* m_gw[MAX_GRAPH_WINDOWS];
	GraphingTexture* m_gt[MAX_GRAPH_WINDOWS];
	i32 m_curNumGraphWindows;

	QuickCanvas(GL3TexLoader* myTexLoader)
		: m_myTexLoader(myTexLoader),
		  m_curNumGraphWindows(0)
	{
		for (i32 i = 0; i < MAX_GRAPH_WINDOWS; i++)
		{
			m_gw[i] = 0;
			m_gt[i] = 0;
		}
	}
	virtual ~QuickCanvas() {}
	virtual i32 createCanvas(tukk canvasName, i32 width, i32 height, i32 xPos, i32 yPos)
	{
		if (m_curNumGraphWindows < MAX_GRAPH_WINDOWS)
		{
			//find a slot
			i32 slot = m_curNumGraphWindows;
			Assert(slot < MAX_GRAPH_WINDOWS);
			if (slot >= MAX_GRAPH_WINDOWS)
				return 0;  //don't crash

			m_curNumGraphWindows++;

			MyGraphInput input(gui2->getInternalData());
			input.m_width = width;
			input.m_height = height;
			input.m_xPos = xPos;
			input.m_yPos = yPos;
			input.m_name = canvasName;
			input.m_texName = canvasName;
			m_gt[slot] = new GraphingTexture;
			m_gt[slot]->create(width, height);
			i32 texId = m_gt[slot]->getTextureId();
			m_myTexLoader->m_hashMap.insert(canvasName, texId);
			m_gw[slot] = setupTextureWindow(input);

			return slot;
		}
		return -1;
	}
	virtual void destroyCanvas(i32 canvasId)
	{
		Assert(canvasId >= 0);
		delete m_gt[canvasId];
		m_gt[canvasId] = 0;
		destroyTextureWindow(m_gw[canvasId]);
		m_gw[canvasId] = 0;
		m_curNumGraphWindows--;
	}
	virtual void setPixel(i32 canvasId, i32 x, i32 y, u8 red, u8 green, u8 blue, u8 alpha)
	{
		Assert(canvasId >= 0);
		Assert(canvasId < m_curNumGraphWindows);
		m_gt[canvasId]->setPixel(x, y, red, green, blue, alpha);
	}

	virtual void getPixel(i32 canvasId, i32 x, i32 y, u8& red, u8& green, u8& blue, u8& alpha)
	{
		Assert(canvasId >= 0);
		Assert(canvasId < m_curNumGraphWindows);
		m_gt[canvasId]->getPixel(x, y, red, green, blue, alpha);
	}

	virtual void refreshImageData(i32 canvasId)
	{
		m_gt[canvasId]->uploadImageData();
	}
};

OpenGLExampleBrowser::OpenGLExampleBrowser(class ExampleEntries* examples)
{
	m_internalData = new OpenGLExampleBrowserInternalData;

	gAllExamples = examples;
}

OpenGLExampleBrowser::~OpenGLExampleBrowser()
{
	deleteDemo();
	for (i32 i = 0; i < m_internalData->m_nodes.size(); i++)
	{
		delete m_internalData->m_nodes[i];
	}
	delete m_internalData->m_handler2;
	for (i32 i = 0; i < m_internalData->m_handlers.size(); i++)
	{
		delete m_internalData->m_handlers[i];
	}
	m_internalData->m_handlers.clear();
	m_internalData->m_nodes.clear();
	delete s_parameterInterface;
	s_parameterInterface = 0;
	delete s_app->m_2dCanvasInterface;
	s_app->m_2dCanvasInterface = 0;

#ifndef DRX3D_NO_PROFILE
	destroyProfileWindow(m_internalData->m_profWindow);
#endif

	m_internalData->m_gui->exit();

	delete m_internalData->m_gui;
	delete m_internalData->m_gwenRenderer;
	delete m_internalData->m_myTexLoader;

	delete m_internalData->m_app;
	s_app = 0;

	delete m_internalData;

	gFileImporterByExtension.clear();
	gAllExamples = 0;
}

#include "../EmptyExample.h"

bool OpenGLExampleBrowser::init(i32 argc, tuk argv[])
{
	b3CommandLineArgs args(argc, argv);

	loadCurrentSettings(startFileName, args);
	if (args.CheckCmdLineFlag("nogui"))
	{
		renderGrid = false;
		renderGui = false;
	}
	if (args.CheckCmdLineFlag("tracing"))
	{
		b3ChromeUtilsStartTimings();
	}
	args.GetCmdLineArgument("fixed_timestep", gFixedTimeStep);
	args.GetCmdLineArgument("png_skip_frames", gPngSkipFrames);
	///The OpenCL rigid body pipeline is experimental and
	///most OpenCL drivers and OpenCL compilers have issues with our kernels.
	///If you have a high-end desktop GPU such as AMD 7970 or better, or NVIDIA GTX 680 with up-to-date drivers
	///you could give it a try
	///Note that several old OpenCL physics examples still have to be ported over to this new Example Browser
	if (args.CheckCmdLineFlag("enable_experimental_opencl"))
	{
		enable_experimental_opencl = true;
		gAllExamples->initOpenCLExampleEntries();
	}

	if (args.CheckCmdLineFlag("disable_retina"))
	{
		gAllowRetina = false;
	}

	i32 width = 1024;
	i32 height = 768;

	if (args.CheckCmdLineFlag("width"))
	{
		args.GetCmdLineArgument("width", width);
	}
	if (args.CheckCmdLineFlag("height"))
	{
		args.GetCmdLineArgument("height", height);
	}

#ifndef NO_OPENGL3
	SimpleOpenGL3App* simpleApp = 0;
	sUseOpenGL2 = args.CheckCmdLineFlag("opengl2");
	args.GetCmdLineArgument("render_device", gRenderDevice);
	args.GetCmdLineArgument("window_backend", gWindowBackend);
#else
	sUseOpenGL2 = true;
#endif
	tukk appTitle = "drx3D Physics ExampleBrowser";
#if defined(_DEBUG) || defined(DEBUG)
	tukk optMode = "Debug build (slow)";
#else
	tukk optMode = "Release build";
#endif

#ifdef D3_USE_GLFW
	tukk glContext = "[glfw]";
#else
	tukk glContext = "[btgl]";
#endif
/*
	if (sUseOpenGL2)
	{
		char title[1024];
		sprintf(title, "%s using limited OpenGL2 fallback %s %s", appTitle, glContext, optMode);
		s_app = new SimpleOpenGL2App(title, width, height);
		s_app->m_renderer = new SimpleOpenGL2Renderer(width, height);
	}
*/
#ifndef NO_OPENGL3
	//else
//	{
		char title[1024];
		sprintf(title, "%s используется OpenGL3+ %s %s", appTitle, glContext, optMode);
		simpleApp = new SimpleOpenGL3App(title, width, height, gAllowRetina, gWindowBackend, gRenderDevice);
		s_app = simpleApp;
//	}
#endif
	m_internalData->m_app = s_app;
	tuk gVideoFileName = 0;
	args.GetCmdLineArgument("mp4", gVideoFileName);
	i32 gVideoFps = 0;
	args.GetCmdLineArgument("mp4fps", gVideoFps);
	if (gVideoFps)
	{
		simpleApp->setMp4Fps(gVideoFps);
	}

#ifndef NO_OPENGL3
	if (gVideoFileName)
		simpleApp->dumpFramesToVideo(gVideoFileName);
#endif

	s_instancingRenderer = s_app->m_renderer;
	s_window = s_app->m_window;

	width = s_window->getWidth();
	height = s_window->getHeight();

	prevMouseMoveCallback = s_window->getMouseMoveCallback();
	s_window->setMouseMoveCallback(MyMouseMoveCallback);

	prevMouseButtonCallback = s_window->getMouseButtonCallback();
	s_window->setMouseButtonCallback(MyMouseButtonCallback);
	prevKeyboardCallback = s_window->getKeyboardCallback();
	s_window->setKeyboardCallback(MyKeyboardCallback);

	s_app->m_renderer->getActiveCamera()->setCameraDistance(13);
	s_app->m_renderer->getActiveCamera()->setCameraPitch(0);
	s_app->m_renderer->getActiveCamera()->setCameraTargetPosition(0, 0, 0);

	float mouseMoveMult = s_app->getMouseMoveMultiplier();
	if (args.GetCmdLineArgument("mouse_move_multiplier", mouseMoveMult))
	{
		s_app->setMouseMoveMultiplier(mouseMoveMult);
	}

	float mouseWheelMult = s_app->getMouseWheelMultiplier();
	if (args.GetCmdLineArgument("mouse_wheel_multiplier", mouseWheelMult))
	{
		s_app->setMouseWheelMultiplier(mouseWheelMult);
	}

	args.GetCmdLineArgument("shared_memory_key", gSharedMemoryKey);

	float red, green, blue;
	s_app->getBackgroundColor(&red, &green, &blue);
	args.GetCmdLineArgument("background_color_red", red);
	args.GetCmdLineArgument("background_color_green", green);
	args.GetCmdLineArgument("background_color_blue", blue);
	s_app->setBackgroundColor(red, green, blue);

	b3SetCustomWarningMessageFunc(MyGuiPrintf);
	b3SetCustomPrintfFunc(MyGuiPrintf);
	b3SetCustomErrorMessageFunc(MyStatusBarError);

	assert(glGetError() == GL_NO_ERROR);

	{
		GL3TexLoader* myTexLoader = new GL3TexLoader;
		m_internalData->m_myTexLoader = myTexLoader;

		if (sUseOpenGL2)
		{
			m_internalData->m_gwenRenderer = new Gwen::Renderer::OpenGL_DebugFont(s_window->getRetinaScale());
		}
#ifndef NO_OPENGL3
		else
		{
			sth_stash* fontstash = simpleApp->getFontStash();
			m_internalData->m_gwenRenderer = new GwenOpenGL3CoreRenderer(simpleApp->m_primRenderer, fontstash, width, height, s_window->getRetinaScale(), myTexLoader);
		}
#endif

		gui2 = new GwenUserInterface;

		m_internalData->m_gui = gui2;

		m_internalData->m_myTexLoader = myTexLoader;

		gui2->init(width, height, m_internalData->m_gwenRenderer, s_window->getRetinaScale());
	}
	//gui = 0;// new GwenUserInterface;

	GL3TexLoader* myTexLoader = m_internalData->m_myTexLoader;
	// = myTexLoader;

	//

	if (gui2)
	{
		//	gui->getInternalData()->m_explorerPage
		Gwen::Controls::TreeControl* tree = gui2->getInternalData()->m_explorerTreeCtrl;

		//gui->getInternalData()->pRenderer->setTextureLoader(myTexLoader);

#ifndef DRX3D_NO_PROFILE
		s_profWindow = setupProfileWindow(gui2->getInternalData());
		m_internalData->m_profWindow = s_profWindow;
		profileWindowSetVisible(s_profWindow, false);
#endif  //DRX3D_NO_PROFILE
		gui2->setFocus();

		s_parameterInterface = s_app->m_parameterInterface = new GwenParameterInterface(gui2->getInternalData());
		s_app->m_2dCanvasInterface = new QuickCanvas(myTexLoader);

		///add some demos to the gAllExamples

		i32 numDemos = gAllExamples->getNumRegisteredExamples();

		//char nodeText[1024];
		//i32 curDemo = 0;
		i32 selectedDemo = 0;
		Gwen::Controls::TreeNode* curNode = tree;
		m_internalData->m_handler2 = new MyMenuItemHander(-1);

		tuk demoNameFromCommandOption = 0;
		args.GetCmdLineArgument("start_demo_name", demoNameFromCommandOption);
		if (demoNameFromCommandOption)
		{
			selectedDemo = -1;
		}

		tree->onReturnKeyDown.Add(m_internalData->m_handler2, &MyMenuItemHander::onButtonD);
		i32 firstAvailableDemoIndex = -1;
		Gwen::Controls::TreeNode* firstNode = 0;

		for (i32 d = 0; d < numDemos; d++)
		{
			//		sprintf(nodeText, "Node %d", i);
			Gwen::WTxt nodeUText = Gwen::WTxt(gAllExamples->getExampleName(d));
			if (gAllExamples->getExampleCreateFunc(d))  //was test for gAllExamples[d].m_menuLevel==1
			{
				Gwen::Controls::TreeNode* pNode = curNode->AddNode(nodeUText);

				if (firstAvailableDemoIndex < 0)
				{
					firstAvailableDemoIndex = d;
					firstNode = pNode;
				}

				if (d == selectedDemo)
				{
					firstAvailableDemoIndex = d;
					firstNode = pNode;
					//pNode->SetSelected(true);
					//tree->ExpandAll();
					//	tree->ForceUpdateScrollBars();
					//tree->OnKeyLeft(true);
					//	tree->OnKeyRight(true);

					//tree->ExpandAll();

					//	selectDemo(d);
				}

				if (demoNameFromCommandOption)
				{
					tukk demoName = gAllExamples->getExampleName(d);
					i32 res = strcmp(demoName, demoNameFromCommandOption);
					if (res == 0)
					{
						firstAvailableDemoIndex = d;
						firstNode = pNode;
					}
				}

#if 1
				MyMenuItemHander* handler = new MyMenuItemHander(d);
				m_internalData->m_handlers.push_back(handler);

				pNode->onNamePress.Add(handler, &MyMenuItemHander::onButtonA);
				pNode->GetButton()->onDoubleClick.Add(handler, &MyMenuItemHander::onButtonB);
				pNode->GetButton()->onDown.Add(handler, &MyMenuItemHander::onButtonC);
				pNode->onSelect.Add(handler, &MyMenuItemHander::onButtonE);
				pNode->onReturnKeyDown.Add(handler, &MyMenuItemHander::onButtonG);
				pNode->onSelectChange.Add(handler, &MyMenuItemHander::onButtonF);

#endif
				//			pNode->onKeyReturn.Add(handler, &MyMenuItemHander::onButtonD);
				//			pNode->GetButton()->onKeyboardReturn.Add(handler, &MyMenuItemHander::onButtonD);
				//		pNode->onNamePress.Add(handler, &MyMenuItemHander::onButtonD);
				//			pNode->onKeyboardPressed.Add(handler, &MyMenuItemHander::onButtonD);
				//			pNode->OnKeyPress
			}
			else
			{
				curNode = tree->AddNode(nodeUText);
				m_internalData->m_nodes.push_back(curNode);
			}
		}

		if (sCurrentDemo == 0)
		{
			if (firstAvailableDemoIndex >= 0)
			{
				firstNode->SetSelected(true);
				while (firstNode != tree)
				{
					firstNode->ExpandAll();
					firstNode = (Gwen::Controls::TreeNode*)firstNode->GetParent();
				}

				selectDemo(firstAvailableDemoIndex);
			}
		}
		free(demoNameFromCommandOption);
		demoNameFromCommandOption = 0;

		Assert(sCurrentDemo != 0);
		if (sCurrentDemo == 0)
		{
			printf("Ошибка, отсутствует демо/пример\n");
			exit(0);
		}

		gui2->registerFileOpenCallback(fileOpenCallback);
		gui2->registerQuitCallback(quitCallback);
	}

	return true;
}

CommonExampleInterface* OpenGLExampleBrowser::getCurrentExample()
{
	Assert(sCurrentDemo);
	return sCurrentDemo;
}

bool OpenGLExampleBrowser::requestedExit()
{
	return s_window->requestedExit();
}

void OpenGLExampleBrowser::updateGraphics()
{
	if (sCurrentDemo)
	{
		if (!pauseSimulation || singleStepSimulation)
		{
			//D3_PROFILE("sCurrentDemo->updateGraphics");
			sCurrentDemo->updateGraphics();
		}
	}
}

void OpenGLExampleBrowser::update(float deltaTime)
{

	b3ChromeUtilsEnableProfiling();

	if (!gEnableRenderLoop && !singleStepSimulation)
	{
		D3_PROFILE("updateGraphics");
		sCurrentDemo->updateGraphics();
		return;
	}

	D3_PROFILE("OpenGLExampleBrowser::update");
	//assert(glGetError() == GL_NO_ERROR);
	{
		D3_PROFILE("s_instancingRenderer");
		s_instancingRenderer->init();
	}
	DrawGridData dg;
	dg.upAxis = s_app->getUpAxis();

	{
		DRX3D_PROFILE("Обновить Камеру и Свет");

		s_instancingRenderer->updateCamera(dg.upAxis);
	}

	static i32 frameCount = 0;
	frameCount++;

	if (0)
	{
		DRX3D_PROFILE("Счётчик чертёжных кадров");
		char bla[1024];
		sprintf(bla, "Кадр %d", frameCount);
		s_app->drawText(bla, 10, 10);
	}

	if (gPngFileName)
	{
		static i32 skip = 0;
		skip--;
		if (skip < 0)
		{
			skip = gPngSkipFrames;
			//printf("gPngFileName=%s\n",gPngFileName);
			static i32 s_frameCount = 0;

			sprintf(staticPngFileName, "%s%d.png", gPngFileName, s_frameCount++);
			//drx3DPrintf("Made screenshot %s",staticPngFileName);
			s_app->dumpNextFrameToPng(staticPngFileName);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
	}

	if (sCurrentDemo)
	{
		if (!pauseSimulation || singleStepSimulation)
		{
			//printf("---------------------------------------------------\n");
			//printf("Framecount = %d\n",frameCount);
			D3_PROFILE("sCurrentDemo->stepSimulation");

			if (gFixedTimeStep > 0)
			{
				
				sCurrentDemo->stepSimulation(gFixedTimeStep);
			}
			else
			{
				sCurrentDemo->stepSimulation(deltaTime);  //1./60.f);
			}
		}

		if (renderGrid)
		{
			DRX3D_PROFILE("Чертёжная Сетка");
			//glPolygonOffset(3.0, 3);
			//glEnable(GL_POLYGON_OFFSET_FILL);
			s_app->drawGrid(dg);
		}
		if (renderVisualGeometry && ((gDebugDrawFlags & IDebugDraw::DBG_DrawWireframe) == 0))
		{
			if (visualWireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			DRX3D_PROFILE("Отображение Сцены");
			sCurrentDemo->renderScene();
		}
		else
		{
			D3_PROFILE("physicsDebugDraw");
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			sCurrentDemo->physicsDebugDraw(gDebugDrawFlags);
		}
	}

	{
		if (gui2 && s_guiHelper && s_guiHelper->getRenderInterface() && s_guiHelper->getRenderInterface()->getActiveCamera())
		{
			D3_PROFILE("setStatusBarMessage");
			char msg[1024];
			float camDist = s_guiHelper->getRenderInterface()->getActiveCamera()->getCameraDistance();
			float pitch = s_guiHelper->getRenderInterface()->getActiveCamera()->getCameraPitch();
			float yaw = s_guiHelper->getRenderInterface()->getActiveCamera()->getCameraYaw();
			float camTarget[3];
			float camPos[3];
			s_guiHelper->getRenderInterface()->getActiveCamera()->getCameraPosition(camPos);
			s_guiHelper->getRenderInterface()->getActiveCamera()->getCameraTargetPosition(camTarget);
			sprintf(msg, "camTargetPos=%2.2f,%2.2f,%2.2f, dist=%2.2f, pitch=%2.2f, yaw=%2.2f", camTarget[0], camTarget[1], camTarget[2], camDist, pitch, yaw);
			gui2->setStatusBarMessage(msg, true);
		}
	}

	static i32 toggle = 1;
	if (renderGui)
	{
		D3_PROFILE("renderGui");
#ifndef DRX3D_NO_PROFILE

		if (!pauseSimulation || singleStepSimulation)
		{
			if (isProfileWindowVisible(s_profWindow))
			{
				processProfileData(s_profWindow, false);
			}
		}
#endif  //#ifndef DRX3D_NO_PROFILE

		{
			D3_PROFILE("updateOpenGL");
			if (sUseOpenGL2)
			{
				saveOpenGLState(s_instancingRenderer->getScreenWidth() * s_window->getRetinaScale(), s_instancingRenderer->getScreenHeight() * s_window->getRetinaScale());
			}

			if (m_internalData->m_gui)
			{
				gBlockGuiMessages = true;
				m_internalData->m_gui->draw(s_instancingRenderer->getScreenWidth(), s_instancingRenderer->getScreenHeight());

				gBlockGuiMessages = false;
			}

			if (sUseOpenGL2)
			{
				restoreOpenGLState();
			}
		}
	}

	singleStepSimulation = false;

	toggle = 1 - toggle;
	{
		DRX3D_PROFILE("Sync Parameters");
		if (s_parameterInterface)
		{
			s_parameterInterface->syncParameters();
		}
	}
	{
		DRX3D_PROFILE("Swap Buffers");
		s_app->swapBuffer();
	}

	if (gui2)
	{
		D3_PROFILE("forceUpdateScrollBars");
		gui2->forceUpdateScrollBars();
	}
}

void OpenGLExampleBrowser::setSharedMemoryInterface(class SharedMemoryInterface* sharedMem)
{
	gDisableDemoSelection = true;
	sSharedMem = sharedMem;
}
