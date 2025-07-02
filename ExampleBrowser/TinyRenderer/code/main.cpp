#include <drx3D/OpenGLWindow/SimpleOpenGL3App.h>
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3CommandLineArgs.h>
#include <drx3D/Common/b3Transform.h>
#include <drx3D/Common/DefaultFileIO.h>
#include "assert.h"
#include <stdio.h>

tuk gVideoFileName = 0;
tuk gPngFileName = 0;

static b3WheelCallback sOldWheelCB = 0;
static b3ResizeCallback sOldResizeCB = 0;
static b3MouseMoveCallback sOldMouseMoveCB = 0;
static b3MouseButtonCallback sOldMouseButtonCB = 0;
static b3KeyboardCallback sOldKeyboardCB = 0;
//static b3RenderCallback sOldRenderCB = 0;

float gWidth = 0;
float gHeight = 0;

void MyWheelCallback(float deltax, float deltay)
{
	if (sOldWheelCB)
		sOldWheelCB(deltax, deltay);
}
void MyResizeCallback(float width, float height)
{
	gWidth = width;
	gHeight = height;

	if (sOldResizeCB)
		sOldResizeCB(width, height);
}
void MyMouseMoveCallback(float x, float y)
{
	printf("Движение Мыши: %f, %f\n", x, y);

	if (sOldMouseMoveCB)
		sOldMouseMoveCB(x, y);
}
void MyMouseButtonCallback(i32 button, i32 state, float x, float y)
{
	if (sOldMouseButtonCB)
		sOldMouseButtonCB(button, state, x, y);
}

void MyKeyboardCallback(i32 keycode, i32 state)
{
	//keycodes are in examples/CommonInterfaces/CommonWindowInterface.h
	//for example B3G_ESCAPE for escape key
	//state == 1 for pressed, state == 0 for released.
	// use app->m_window->isModifiedPressed(...) to check for shift, escape and alt keys
	printf("MyKeyboardCallback rполучил ключ:%c в состоянии %d\n", keycode, state);
	if (sOldKeyboardCB)
		sOldKeyboardCB(keycode, state);
}
#include "../TinyRenderer.h"
#include "../our_gl.h"


i32 main(i32 argc, tuk argv[])
{
	b3CommandLineArgs myArgs(argc, argv);

	SimpleOpenGL3App* app = new SimpleOpenGL3App("SimpleOpenGL3App", 640, 480, true);

	app->m_instancingRenderer->getActiveCamera()->setCameraDistance(13);
	app->m_instancingRenderer->getActiveCamera()->setCameraPitch(0);
	app->m_instancingRenderer->getActiveCamera()->setCameraTargetPosition(0, 0, 0);
	sOldKeyboardCB = app->m_window->getKeyboardCallback();
	app->m_window->setKeyboardCallback(MyKeyboardCallback);
	sOldMouseMoveCB = app->m_window->getMouseMoveCallback();
	app->m_window->setMouseMoveCallback(MyMouseMoveCallback);
	sOldMouseButtonCB = app->m_window->getMouseButtonCallback();
	app->m_window->setMouseButtonCallback(MyMouseButtonCallback);
	sOldWheelCB = app->m_window->getWheelCallback();
	app->m_window->setWheelCallback(MyWheelCallback);
	sOldResizeCB = app->m_window->getResizeCallback();
	app->m_window->setResizeCallback(MyResizeCallback);

	i32 textureWidth = gWidth;
	i32 textureHeight = gHeight;
	TGAImage rgbColorBuffer(gWidth, gHeight, TGAImage::RGB);
	b3AlignedObjectArray<float> depthBuffer;
	depthBuffer.resize(gWidth * gHeight);

	TinyRenderObjectData renderData(rgbColorBuffer, depthBuffer);  //, "african_head/african_head.obj");//floor.obj");

	DefaultFileIO fileIO;
	//renderData.loadModel("african_head/african_head.obj", &fileIO);
	
	renderData.loadModel("floor.obj",&fileIO);

	//renderData.createCube(1,1,1);

	myArgs.GetCmdLineArgument("mp4_file", gVideoFileName);
	if (gVideoFileName)
		app->dumpFramesToVideo(gVideoFileName);

	myArgs.GetCmdLineArgument("png_file", gPngFileName);
	char fileName[1024];

	u8* image = new u8[textureWidth * textureHeight * 4];

	i32 textureHandle = app->m_renderer->registerTexture(image, textureWidth, textureHeight);

	i32 cubeIndex = app->registerCubeShape(1, 1, 1);

	b3Vec3 pos = b3MakeVector3(0, 0, 0);
	b3Quat orn(0, 0, 0, 1);
	float color[4] = {1,1,1,1};
	
	b3Vec3 scaling = b3MakeVector3(1, 1, 1);
	app->m_renderer->registerGraphicsInstance(cubeIndex, pos, orn, color, scaling);
	app->m_renderer->writeTransforms();

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

		app->m_instancingRenderer->init();
		app->m_instancingRenderer->updateCamera();

		///clear the color and z (depth) buffer
		for (i32 y = 0; y < textureHeight; ++y)
		{
			u8* pi = image + (y)*textureWidth * 3;
			for (i32 x = 0; x < textureWidth; ++x)
			{
				TGAColor color;
				color.bgra[0] = 255;
				color.bgra[1] = 255;
				color.bgra[2] = 255;
				color.bgra[3] = 255;

				renderData.m_rgbColorBuffer.set(x, y, color);
				renderData.m_depthBuffer[x + y * textureWidth] = -1e30f;
			}
		}

		float projMat[16];
		app->m_instancingRenderer->getActiveCamera()->getCameraProjectionMatrix(projMat);
		float viewMat[16];
		app->m_instancingRenderer->getActiveCamera()->getCameraViewMatrix(viewMat);
		D3_ATTRIBUTE_ALIGNED16(float modelMat[16]);

		//sync the object transform
		b3Transform tr;
		tr.setIdentity();
		static float posUp = 0.f;
		// posUp += 0.001;
		b3Vec3 org = b3MakeVector3(0, 0, posUp);
		tr.setOrigin(org);
		tr.getOpenGLMatrix(modelMat);

		TinyRender::Vec3f       eye(1,1,3);
		TinyRender::Vec3f    center(0,0,0);
		TinyRender::Vec3f        up(0,1,0);
    
		renderData.m_viewMatrix = TinyRender::lookat(eye, center, up);
		renderData.m_viewportMatrix = TinyRender::viewport(gWidth/8, gHeight/8, gWidth*3/4, gHeight*3/4);
		renderData.m_projectionMatrix = TinyRender::projection(-1.f/(eye-center).norm());


		for (i32 i = 0; i < 4; i++)
		{
			for (i32 j = 0; j < 4; j++)
			{
				renderData.m_viewMatrix[i][j] = viewMat[i + 4 * j];
				renderData.m_modelMatrix[i][j] = modelMat[i + 4 * j];
			}
		}

		//render the object
		float color2[4] = { 1,1,1,1 };
		renderData.m_model->setColorRGBA(color2);
		TinyRenderer::renderObject(renderData);

#if 1
		//update the texels of the texture using a simple pattern, animated using frame index
		for (i32 y = 0; y < textureHeight; ++y)
		{
			u8* pi = image + (y)*textureWidth * 3;
			for (i32 x = 0; x < textureWidth; ++x)
			{
				TGAColor color = renderData.m_rgbColorBuffer.get(x, y);
				pi[0] = color.bgra[2];
				pi[1] = color.bgra[1];
				pi[2] = color.bgra[0];
				pi[3] = 255;
				pi += 3;
			}
		}
#else

		//update the texels of the texture using a simple pattern, animated using frame index
		for (i32 y = 0; y < textureHeight; ++y)
		{
			i32k t = (y + frameCount) >> 4;
			u8* pi = image + y * textureWidth * 3;
			for (i32 x = 0; x < textureWidth; ++x)
			{
				TGAColor color = renderData.m_rgbColorBuffer.get(x, y);

				i32k s = x >> 4;
				u8k b = 180;
				u8 c = b + ((s + (t & 1)) & 1) * (255 - b);
				pi[0] = pi[1] = pi[2] = pi[3] = c;
				pi += 3;
			}
		}
#endif

		app->m_renderer->activateTexture(textureHandle);
		app->m_renderer->updateTexture(textureHandle, image);

		float color[4] = {1, 1, 1, 1};
		app->m_primRenderer->drawTexturedRect(0, 0, gWidth / 3, gHeight / 3, color, 0, 0, 1, 1, true);

		app->m_renderer->renderScene();
		app->drawGrid();
		char bla[1024];
		sprintf(bla, "Простой тест-кадр %d", frameCount);
		float colorRGBA[4] = { 1,1,1,1 };
		app->drawText(bla, 10, 10,1, colorRGBA);
		app->swapBuffer();
	} while (!app->m_window->requestedExit());

	delete app;
	return 0;
}
