// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Font/StdAfx.h>
#include <drx3D/Font/GlyphBitmap.h>
#include <math.h>

//-------------------------------------------------------------------------------------------------
CGlyphBitmap::CGlyphBitmap()
	: m_iWidth(0), m_iHeight(0), m_pBuffer(0)
{
}

//-------------------------------------------------------------------------------------------------
CGlyphBitmap::~CGlyphBitmap()
{
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::Create(i32 iWidth, i32 iHeight)
{
	Release();

	m_pBuffer = new u8[iWidth * iHeight];

	if (!m_pBuffer)
	{
		return 0;
	}

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::Release()
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
	}
	m_pBuffer = 0;
	m_iWidth = m_iHeight = 0;

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::Blur(i32 iIterations)
{
	i32 cSum;
	i32 yOffset;
	i32 yupOffset;
	i32 ydownOffset;

	for (i32 i = 0; i < iIterations; i++)
	{
		for (i32 y = 0; y < m_iHeight; y++)
		{
			yOffset = y * m_iWidth;

			if (y - 1 >= 0)
			{
				yupOffset = (y - 1) * m_iWidth;
			}
			else
			{
				yupOffset = (y) * m_iWidth;
			}

			if (y + 1 < m_iHeight)
			{
				ydownOffset = (y + 1) * m_iWidth;
			}
			else
			{
				ydownOffset = (y) * m_iWidth;
			}

			for (i32 x = 0; x < m_iWidth; x++)
			{
				cSum = m_pBuffer[yupOffset + x] + m_pBuffer[ydownOffset + x];

				if (x - 1 >= 0)
				{
					cSum += m_pBuffer[yOffset + x - 1];
				}
				else
				{
					cSum += m_pBuffer[yOffset + x];
				}

				if (x + 1 < m_iWidth)
				{
					cSum += m_pBuffer[yOffset + x + 1];
				}
				else
				{
					cSum += m_pBuffer[yOffset + x];
				}

				m_pBuffer[yOffset + x] = cSum >> 2;
			}
		}
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::Scale(float fScaleX, float fScaleY)
{
	i32 iNewWidth = (i32)(m_iWidth * fScaleX);
	i32 iNewHeight = (i32)(m_iHeight * fScaleY);

	u8* pNewBuffer = new u8[iNewWidth * iNewHeight];

	if (!pNewBuffer)
	{
		return 0;
	}

	float xFactor = m_iWidth / (float)iNewWidth;
	float yFactor = m_iHeight / (float)iNewHeight;

	float xFractioned, yFractioned, xFraction, yFraction, oneMinusX, oneMinusY, fR0, fR1;
	i32 xCeil, yCeil, xFloor, yFloor, yNewOffset;

	u8 c0, c1, c2, c3;

	for (i32 y = 0; y < iNewHeight; ++y)
	{
		yFractioned = y * yFactor;
		yFloor = (i32)floor_tpl(yFractioned);
		yCeil = yFloor + 1;

		if (yCeil >= m_iHeight)
		{
			yCeil = yFloor;
		}

		yFraction = yFractioned - yFloor;
		oneMinusY = 1.0f - yFraction;

		yNewOffset = y * iNewWidth;

		for (i32 x = 0; x < iNewWidth; ++x)
		{
			xFractioned = x * xFactor;
			xFloor = (i32)floor_tpl(xFractioned);
			xCeil = xFloor + 1;

			if (xCeil >= m_iWidth)
			{
				xCeil = xFloor;
			}

			xFraction = xFractioned - xFloor;
			oneMinusX = 1.0f - xFraction;

			c0 = m_pBuffer[yFloor * m_iWidth + xFloor];
			c1 = m_pBuffer[yFloor * m_iWidth + xCeil];
			c2 = m_pBuffer[yCeil * m_iWidth + xFloor];
			c3 = m_pBuffer[yCeil * m_iWidth + xCeil];

			fR0 = (oneMinusX * c0 + xFraction * c1);
			fR1 = (oneMinusX * c2 + xFraction * c3);

			pNewBuffer[yNewOffset + x] = (u8)((oneMinusY * fR0) + (yFraction * fR1));
		}
	}

	m_iWidth = iNewWidth;
	m_iHeight = iNewHeight;

	delete[] m_pBuffer;
	m_pBuffer = pNewBuffer;

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::Clear()
{
	memset(m_pBuffer, 0, m_iWidth * m_iHeight);

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::BlitTo8(u8* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth)
{
	i32 ySrcOffset, yDestOffset;

	for (i32 y = 0; y < iSrcHeight; y++)
	{
		ySrcOffset = (iSrcY + y) * m_iWidth;
		yDestOffset = (iDestY + y) * iDestWidth;

		for (i32 x = 0; x < iSrcWidth; x++)
		{
			pBuffer[yDestOffset + iDestX + x] = m_pBuffer[ySrcOffset + iSrcX + x];
		}
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::BlitTo32(u32* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth)
{
	i32 ySrcOffset, yDestOffset;
	char cColor;

	for (i32 y = 0; y < iSrcHeight; y++)
	{
		ySrcOffset = (iSrcY + y) * m_iWidth;
		yDestOffset = (iDestY + y) * iDestWidth;

		for (i32 x = 0; x < iSrcWidth; x++)
		{
			cColor = m_pBuffer[ySrcOffset + iSrcX + x];

			pBuffer[yDestOffset + iDestX + x] = (cColor << 24) | (255 << 16) | (255 << 8) | 255;
		}
	}

	return 1;
}

//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::BlitScaledTo8(u8* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth, i32 iDestHeight, i32 iDestBufferWidth)
{
	i32 iNewWidth = (i32)iDestWidth;
	i32 iNewHeight = (i32)iDestHeight;

	u8* pNewBuffer = pBuffer;

	float xFactor = iSrcWidth / (float)iNewWidth;
	float yFactor = iSrcHeight / (float)iNewHeight;

	float xFractioned, yFractioned, xFraction, yFraction, oneMinusX, oneMinusY, fR0, fR1;
	i32 xCeil, yCeil, xFloor, yFloor, yNewOffset;

	u8 c0, c1, c2, c3;

	for (i32 y = 0; y < iNewHeight; ++y)
	{
		yFractioned = y * yFactor;
		yFloor = (i32)floor_tpl(yFractioned);
		yCeil = yFloor + 1;

		yFraction = yFractioned - yFloor;
		oneMinusY = 1.0f - yFraction;

		yNewOffset = y * iDestBufferWidth;

		yFloor += iSrcY;
		yCeil += iSrcY;

		if (yCeil >= m_iHeight)
		{
			yCeil = yFloor;
		}

		for (i32 x = 0; x < iNewWidth; ++x)
		{
			xFractioned = x * xFactor;
			xFloor = (i32)floor_tpl(xFractioned);
			xCeil = xFloor + 1;

			xFraction = xFractioned - xFloor;
			oneMinusX = 1.0f - xFraction;

			xFloor += iSrcY;
			xCeil += iSrcY;

			if (xCeil >= m_iWidth)
			{
				xCeil = xFloor;
			}

			c0 = m_pBuffer[yFloor * m_iWidth + xFloor];
			c1 = m_pBuffer[yFloor * m_iWidth + xCeil];
			c2 = m_pBuffer[yCeil * m_iWidth + xFloor];
			c3 = m_pBuffer[yCeil * m_iWidth + xCeil];

			fR0 = (oneMinusX * c0 + xFraction * c1);
			fR1 = (oneMinusX * c2 + xFraction * c3);

			pNewBuffer[yNewOffset + x + iDestX] = (u8)((oneMinusY * fR0) + (yFraction * fR1));
		}
	}

	return 1;
}

#if defined(__GNUC__)
	#if __GNUC__ >= 4 && __GNUC__MINOR__ < 7
		#pragma GCC diagnostic ignored "-Woverflow"
	#endif
#endif
//-------------------------------------------------------------------------------------------------
i32 CGlyphBitmap::BlitScaledTo32(u8* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth, i32 iDestHeight, i32 iDestBufferWidth)
{
	i32 iNewWidth = (i32)iDestWidth;
	i32 iNewHeight = (i32)iDestHeight;

	u8* pNewBuffer = pBuffer;

	float xFactor = iSrcWidth / (float)iNewWidth;
	float yFactor = iSrcHeight / (float)iNewHeight;

	float xFractioned, yFractioned, xFraction, yFraction, oneMinusX, oneMinusY, fR0, fR1;
	i32 xCeil, yCeil, xFloor, yFloor, yNewOffset;

	u8 c0, c1, c2, c3, cColor;

	for (i32 y = 0; y < iNewHeight; ++y)
	{
		yFractioned = y * yFactor;
		yFloor = (i32)floor_tpl(yFractioned);
		yCeil = yFloor + 1;

		yFraction = yFractioned - yFloor;
		oneMinusY = 1.0f - yFraction;

		yNewOffset = y * iDestBufferWidth;

		yFloor += iSrcY;
		yCeil += iSrcY;

		if (yCeil >= m_iHeight)
		{
			yCeil = yFloor;
		}

		for (i32 x = 0; x < iNewWidth; ++x)
		{
			xFractioned = x * xFactor;
			xFloor = (i32)floor_tpl(xFractioned);
			xCeil = xFloor + 1;

			xFraction = xFractioned - xFloor;
			oneMinusX = 1.0f - xFraction;

			xFloor += iSrcY;
			xCeil += iSrcY;

			if (xCeil >= m_iWidth)
			{
				xCeil = xFloor;
			}

			c0 = m_pBuffer[yFloor * m_iWidth + xFloor];
			c1 = m_pBuffer[yFloor * m_iWidth + xCeil];
			c2 = m_pBuffer[yCeil * m_iWidth + xFloor];
			c3 = m_pBuffer[yCeil * m_iWidth + xCeil];

			fR0 = (oneMinusX * c0 + xFraction * c1);
			fR1 = (oneMinusX * c2 + xFraction * c3);

			cColor = (u8)((oneMinusY * fR0) + (yFraction * fR1));

			pNewBuffer[yNewOffset + x + iDestX] = 0xffffff | (cColor << 24);
		}
	}

	return 1;
}
#if defined(__GNUC__)
	#if __GNUC__ >= 4 && __GNUC__MINOR__ < 7
		#pragma GCC diagnostic error  "-Woverflow"
	#endif
#endif
//-------------------------------------------------------------------------------------------------
