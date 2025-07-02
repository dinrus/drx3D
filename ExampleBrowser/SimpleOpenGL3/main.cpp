//#define USE_OPENGL2
#ifdef USE_OPENGL2
#include <drx3D/OpenGLWindow/SimpleOpenGL2App.h>
typedef SimpleOpenGL2App SimpleOpenGLApp;

#else
#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
typedef SimpleOpenGL3App SimpleOpenGLApp;

#endif  //USE_OPENGL2
//#include <drx/Core/Core.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3CommandLineArgs.h>
#include "assert.h"
#include <stdio.h>

static tuk gVideoFileName = 0;
static tuk gPngFileName = 0;

static b3WheelCallback sOldWheelCB = 0;
static b3ResizeCallback sOldResizeCB = 0;
static b3MouseMoveCallback sOldMouseMoveCB = 0;
static b3MouseButtonCallback sOldMouseButtonCB = 0;
static b3KeyboardCallback sOldKeyboardCB = 0;
//static b3RenderCallback sOldRenderCB = 0;

static float gWidth = 1024;
static float gHeight = 768;

void MyWheelCallback2(float deltax, float deltay)
{
	if (sOldWheelCB)
		sOldWheelCB(deltax, deltay);
}
void MyResizeCallback2(float width, float height)
{
	gWidth = width;
	gHeight = height;

	if (sOldResizeCB)
		sOldResizeCB(width, height);
}
void MyMouseMoveCallback2(float x, float y)
{
	printf("Движение Мыши: %f, %f\n", x, y);

	if (sOldMouseMoveCB)
		sOldMouseMoveCB(x, y);
}
void MyMouseButtonCallback2(i32 button, i32 state, float x, float y)
{
	if (sOldMouseButtonCB)
		sOldMouseButtonCB(button, state, x, y);
}

static void MyKeyboardCallback2(i32 keycode, i32 state)
{
	//keycodes are in examples/CommonInterfaces/CommonWindowInterface.h
	//for example B3G_ESCAPE for escape key
	//state == 1 for pressed, state == 0 for released.
	// use app->m_window->isModifiedPressed(...) to check for shift, escape and alt keys
	printf("MyKeyboardCallback получил ключ:%c в состоянии %d\n", keycode, state);
	if (sOldKeyboardCB)
		sOldKeyboardCB(keycode, state);
}

i32 main(i32 argc, tuk argv[])
{
	{
		b3CommandLineArgs myArgs(argc, argv);

		SimpleOpenGLApp* app = new SimpleOpenGLApp("OpenGL3", 1024, 768);

		app->m_renderer->getActiveCamera()->setCameraDistance(13);
		app->m_renderer->getActiveCamera()->setCameraPitch(0);
		app->m_renderer->getActiveCamera()->setCameraTargetPosition(0, 0, 0);
		sOldKeyboardCB = app->m_window->getKeyboardCallback();
		app->m_window->setKeyboardCallback(MyKeyboardCallback2);
		sOldMouseMoveCB = app->m_window->getMouseMoveCallback();
		app->m_window->setMouseMoveCallback(MyMouseMoveCallback2);
		sOldMouseButtonCB = app->m_window->getMouseButtonCallback();
		app->m_window->setMouseButtonCallback(MyMouseButtonCallback2);
		sOldWheelCB = app->m_window->getWheelCallback();
		app->m_window->setWheelCallback(MyWheelCallback2);
		sOldResizeCB = app->m_window->getResizeCallback();
		app->m_window->setResizeCallback(MyResizeCallback2);

		myArgs.GetCmdLineArgument("mp4_file", gVideoFileName);
		if (gVideoFileName)
			app->dumpFramesToVideo(gVideoFileName);

		myArgs.GetCmdLineArgument("png_file", gPngFileName);
		char fileName[1024];

		i32 textureWidth = 128;
		i32 textureHeight = 128;

		u8* image = new u8[textureWidth * textureHeight * 4];

		i32 textureHandle = app->m_renderer->registerTexture(image, textureWidth, textureHeight);

		do
		{
			static i32 frameCount = 0;
			frameCount++;
			if (gPngFileName)
			{
				printf("gPngFileName=%s\n", gPngFileName);

				sprintf(fileName, "%s%d.png", gPngFileName, frameCount++);
				app->dumpNextFrameToPng(fileName);
			}

			//update the texels of the texture using a simple pattern, animated using frame index
			for (i32 y = 0; y < textureHeight; ++y)
			{
				i32k t = (y + frameCount) >> 4;
				u8* pi = image + y * textureWidth * 3;
				for (i32 x = 0; x < textureWidth; ++x)
				{
					i32k s = x >> 4;
					u8k b = 180;
					u8 c = b + ((s + (t & 1)) & 1) * (255 - b);
					pi[0] = pi[1] = pi[2] = pi[3] = c;
					pi += 3;
				}
			}

			app->m_renderer->activateTexture(textureHandle);
			app->m_renderer->updateTexture(textureHandle, image);

			float color[4] = {1, 0, 0, 1};
			app->m_primRenderer->drawTexturedRect(100, 200, gWidth / 2 - 50, gHeight / 2 - 50, color, 0, 0, 1, 1, true);

			app->m_renderer->init();
			i32 upAxis = 1;
			app->m_renderer->updateCamera(upAxis);

			app->m_renderer->renderScene();

			app->drawGrid();
			char bla[1024];
			sprintf(bla, "Текст 2d:%d", frameCount);

			float yellow[4] = {1, 1, 0, 1};
			app->drawText(bla, 10, 10, 1, yellow);
			float position[3] = {1, 1, 1};
			float position2[3] = {0, 0, 5};

			float orientation[4] = {0, 0, 0, 1};

			app->drawText3D(bla, 0, 0, 1, 1);

			sprintf(bla, "Текст 3d битмапа лицом к камере:%d", frameCount);
			app->drawText3D(bla, position2, orientation, color, 1, CommonGraphicsApp::eDrawText3D_OrtogonalFaceCamera);

			sprintf(bla, "Текст 3d битмапа:%d", frameCount);
			app->drawText3D(bla, position, orientation, color, 0.001, 0);

			float green[4] = {0, 1, 0, 1};
			float blue[4] = {0, 0, 1, 1};

			sprintf(bla, "Текст 3d ttf лицом к камере:%d", frameCount);
			app->drawText3D(bla, position2, orientation, green, 1, CommonGraphicsApp::eDrawText3D_TrueType | CommonGraphicsApp::eDrawText3D_OrtogonalFaceCamera);

			app->drawText3D(bla, position2, orientation, green, 1, CommonGraphicsApp::eDrawText3D_TrueType | CommonGraphicsApp::eDrawText3D_OrtogonalFaceCamera);
			sprintf(bla, "Текст 3d ttf:%d", frameCount);
			b3Quat orn;
			orn.setEulerZYX(D3_HALF_PI / 2., 0, D3_HALF_PI / 2.);
			app->drawText3D(bla, position2, orn, blue, 1, CommonGraphicsApp::eDrawText3D_TrueType);

			app->swapBuffer();
		} while (!app->m_window->requestedExit());

		delete app;

		delete[] image;
	}
	return 0;
}
