#ifndef GRAPHING_TEXTURE_H
#define GRAPHING_TEXTURE_H
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

struct GraphingTexture
{
	i32 m_textureId;
	//assume rgba (8 bit per component, total of 32bit per pixel, for m_width*m_height pixels)
	AlignedObjectArray<u8> m_imageData;
	i32 m_width;
	i32 m_height;

	GraphingTexture();
	virtual ~GraphingTexture();

	bool create(i32 texWidth, i32 texHeight);
	void destroy();

	void setPixel(i32 x, i32 y, u8 red, u8 green, u8 blue, u8 alpha)
	{
		if (y >= 0 && y < m_height && x >= 0 && x < m_width)
		{
			m_imageData[x * 4 + y * 4 * m_width + 0] = red;
			m_imageData[x * 4 + y * 4 * m_width + 1] = green;
			m_imageData[x * 4 + y * 4 * m_width + 2] = blue;
			m_imageData[x * 4 + y * 4 * m_width + 3] = alpha;
		}
	}

	void getPixel(i32 x, i32 y, u8& red, u8& green, u8& blue, u8& alpha)
	{
		red = m_imageData[x * 4 + y * 4 * m_width + 0];
		green = m_imageData[x * 4 + y * 4 * m_width + 1];
		blue = m_imageData[x * 4 + y * 4 * m_width + 2];
		alpha = m_imageData[x * 4 + y * 4 * m_width + 3];
	}
	void uploadImageData();

	i32 getTextureId()
	{
		return m_textureId;
	}
};

#endif  //GRAPHING_TEXTURE_H
