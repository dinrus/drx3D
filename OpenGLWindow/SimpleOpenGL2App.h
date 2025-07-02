#ifndef SIMPLE_OPENGL2_APP_H
#define SIMPLE_OPENGL2_APP_H

#include <drx3D/Common/Interfaces/CommonGraphicsAppInterface.h>

class SimpleOpenGL2App : public CommonGraphicsApp
{
protected:
	struct SimpleOpenGL2AppInternalData* m_data;

public:
	SimpleOpenGL2App(tukk title, i32 width, i32 height);
	virtual ~SimpleOpenGL2App();

	virtual void drawGrid(DrawGridData data = DrawGridData());
	virtual void setUpAxis(i32 axis);
	virtual i32 getUpAxis() const;

	virtual void swapBuffer();
	virtual void drawText(tukk txt, i32 posX, i32 posY, float size, float colorRGBA[4]);

	virtual void drawTexturedRect(float x0, float y0, float x1, float y1, float color[4], float u0, float v0, float u1, float v1, i32 useRGBA){};
	virtual void setBackgroundColor(float red, float green, float blue);
	virtual i32 registerCubeShape(float halfExtentsX, float halfExtentsY, float halfExtentsZ, i32 textureIndex = -1, float textureScaling = 1);

	virtual i32 registerGraphicsUnitSphereShape(EnumSphereLevelOfDetail lod, i32 textureId = -1);
	virtual void drawText3D(tukk txt, float posX, float posZY, float posZ, float size);
	virtual void drawText3D(tukk txt, float position[3], float orientation[4], float color[4], float size, i32 optionFlag);

	virtual void registerGrid(i32 xres, i32 yres, float color0[4], float color1[4]);
};
#endif  //SIMPLE_OPENGL2_APP_H