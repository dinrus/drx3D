#ifndef COMMON_2D_CANVAS_INTERFACE_H
#define COMMON_2D_CANVAS_INTERFACE_H

#include <drxtypes.h>

struct Common2dCanvasInterface
{
	virtual ~Common2dCanvasInterface() {}
	virtual i32 createCanvas(tukk canvasName, i32 width, i32 height, i32 xPos, i32 yPos) = 0;
	virtual void destroyCanvas(i32 canvasId) = 0;
	virtual void setPixel(i32 canvasId, i32 x, i32 y, u8 red, u8 green, u8 blue, u8 alpha) = 0;
	virtual void getPixel(i32 canvasId, i32 x, i32 y, u8& red, u8& green, u8& blue, u8& alpha) = 0;

	virtual void refreshImageData(i32 canvasId) = 0;
};

#endif  //COMMON_2D_CANVAS_INTERFACE_H
