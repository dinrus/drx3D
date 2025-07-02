#ifndef GWEN_TEXTURE_WINDOW_H
#define GWEN_TEXTURE_WINDOW_H

#include <drxtypes.h>

struct MyGraphInput
{
	struct GwenInternalData* m_data;
	i32 m_xPos;
	i32 m_yPos;
	i32 m_width;
	i32 m_height;
	i32 m_borderWidth;
	tukk m_name;
	tukk m_texName;
	MyGraphInput(struct GwenInternalData* data)
		: m_data(data),
		  m_xPos(0),
		  m_yPos(0),
		  m_width(400),
		  m_height(400),
		  m_borderWidth(0),
		  m_name("GraphWindow"),
		  m_texName(0)
	{
	}
};
class MyGraphWindow* setupTextureWindow(const MyGraphInput& input);
void destroyTextureWindow(MyGraphWindow* window);

#endif  //GWEN_TEXTURE_WINDOW_H
