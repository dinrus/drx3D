#ifndef SIMPLE_OPENGL3_APP_H
#define SIMPLE_OPENGL3_APP_H

#include <drx3D/OpenGLWindow/GLInstancingRenderer.h>
#include <drx3D/OpenGLWindow/GLPrimitiveRenderer.h>
#include <drx3D/Common/Interfaces/CommonWindowInterface.h>
#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>

struct SimpleOpenGL3App : public CommonGraphicsApp
{
	struct SimpleInternalData* m_data;

	class GLPrimitiveRenderer* m_primRenderer;
	class GLInstancingRenderer* m_instancingRenderer;
	virtual void setBackgroundColor(float red, float green, float blue);
	virtual void setMp4Fps(i32 fps);

	SimpleOpenGL3App(tukk title, i32 width, i32 height, bool allowRetina = true, i32 windowType = 0, i32 renderDevice = -1, i32 maxNumObjectCapacity = 128 * 1024, i32 maxShapeCapacityInBytes = 128 * 1024 * 1024);

	virtual ~SimpleOpenGL3App();

	virtual i32 registerCubeShape(float halfExtentsX = 1.f, float halfExtentsY = 1.f, float halfExtentsZ = 1.f, i32 textureIndex = -1, float textureScaling = 1);
	virtual i32 registerGraphicsUnitSphereShape(EnumSphereLevelOfDetail lod, i32 textureId = -1);
	virtual void registerGrid(i32 xres, i32 yres, float color0[4], float color1[4]);
	void dumpNextFrameToPng(tukk pngFilename);
	void dumpFramesToVideo(tukk mp4Filename);
	virtual void getScreenPixels(u8* rgbaBuffer, i32 bufferSizeInBytes, float* depthBuffer, i32 depthBufferSizeInBytes);
	virtual void setViewport(i32 width, i32 height);

	void drawGrid(DrawGridData data = DrawGridData());
	virtual void setUpAxis(i32 axis);
	virtual i32 getUpAxis() const;

	virtual void swapBuffer();
	virtual void drawText(tukk txt, i32 posX, i32 posY, float size, float colorRGBA[4]);
	virtual void drawText3D(tukk txt, float posX, float posZY, float posZ, float size);
	virtual void drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag);

	virtual void drawTexturedRect(float x0, float y0, float x1, float y1, float color[4], float u0, float v0, float u1, float v1, i32 useRGBA);
	struct sth_stash* getFontStash();
};

#endif  //SIMPLE_OPENGL3_APP_H
