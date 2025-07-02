// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CGlyphBitmap
{
public:
	CGlyphBitmap();
	~CGlyphBitmap();

	i32            Create(i32 iWidth, i32 iHeight);
	i32            Release();

	u8* GetBuffer() { return m_pBuffer; };

	i32            Blur(i32 iIterations);
	i32            Scale(float fScaleX, float fScaleY);

	i32            Clear();

	i32            BlitTo8(u8* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth);
	i32            BlitTo32(u32* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth);

	i32            BlitScaledTo8(u8* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth, i32 iDestHeight, i32 iDestBufferWidth);
	i32            BlitScaledTo32(u8* pBuffer, i32 iSrcX, i32 iSrcY, i32 iSrcWidth, i32 iSrcHeight, i32 iDestX, i32 iDestY, i32 iDestWidth, i32 iDestHeight, i32 iDestBufferWidth);

	i32            GetWidth() const  { return m_iWidth; }
	i32            GetHeight() const { return m_iHeight; }

	void           GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_pBuffer, m_iWidth * m_iHeight);
	}

private:

	u8* m_pBuffer;
	i32            m_iWidth;
	i32            m_iHeight;
};
