// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CFBitmap
{
public:
	CFBitmap();
	~CFBitmap();

	i32 Blur(i32 iIterations);
	i32 Scale(float fScaleX, float fScaleY);

	i32 BlitFrom(CFBitmap* pSrc, i32 iSX, i32 iSY, i32 iDX, i32 iDY, i32 iW, i32 iH);
	i32 BlitTo(CFBitmap* pDst, i32 iDX, i32 iDY, i32 iSX, i32 iSY, i32 iW, i32 iH);

	i32 Create(i32 iWidth, i32 iHeight);
	i32 Release();

	i32 SaveBitmap(const string& szFileName);
	i32 Get32Bpp(u32** pBuffer)
	{
		(*pBuffer) = new u32[m_iWidth * m_iHeight];

		if (!(*pBuffer))
		{
			return 0;
		}

		i32 iDataSize = m_iWidth * m_iHeight;

		for (i32 i = 0; i < iDataSize; i++)
		{
			(*pBuffer)[i] = (m_pData[i] << 24) | (m_pData[i] << 16) | (m_pData[i] << 8) | (m_pData[i]);
		}

		return 1;
	}

	i32   GetWidth()                       { return m_iWidth; }
	i32   GetHeight()                      { return m_iHeight; }

	void  SetRenderData(uk pRenderData) { m_pIRenderData = pRenderData; };
	uk GetRenderData()                  { return m_pIRenderData; };

	void GetMemoryUsage(class IDrxSizer* pSizer);

	u8* GetData() { return m_pData; }

public:

	i32            m_iWidth;
	i32            m_iHeight;

	u8* m_pData;
	uk          m_pIRenderData;
};
