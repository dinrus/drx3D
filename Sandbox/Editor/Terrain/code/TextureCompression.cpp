// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "TextureCompression.h"

#undef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)              \
  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
   ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#include <Drx3DEngine/ImageExtensionHelper.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFile* CTextureCompression::m_pFile = NULL;
DrxCriticalSection CTextureCompression::m_sFileLock;

CTextureCompression::CTextureCompression(const bool bHighQuality) : m_bHighQuality(bHighQuality)
{
}

CTextureCompression::~CTextureCompression()
{
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompression::SaveCompressedMipmapLevel(ukk data, size_t size, uk userData)
{
	assert(m_pFile);

	// without header
	m_pFile->Write(data, size);
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompression::WriteDDSToFile(CFile& file, u8* dat, i32 w, i32 h, i32 Size, ETEX_Format eSrcF, ETEX_Format eDstF, i32 NumMips, const bool bHeader, const bool bDither)
{
	if (bHeader)
	{
		DWORD dwMagic;
		CImageExtensionHelper::DDS_HEADER ddsh;
		ZeroStruct(ddsh);

		dwMagic = MAKEFOURCC('D', 'D', 'S', ' ');
		file.Write(&dwMagic, sizeof(DWORD));

		ddsh.dwSize = sizeof(CImageExtensionHelper::DDS_HEADER);
		ddsh.dwWidth = w;
		ddsh.dwHeight = h;
		ddsh.dwMipMapCount = NumMips;
		if (!NumMips)
			ddsh.dwMipMapCount = 1;
		ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
		ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;
		ddsh.ddspf = DDSFormats::GetDescByFormat(eDstF);

		switch (eDstF)
		{
		case eTF_BC1:
			ddsh.ddspf = DDSFormats::DDSPF_DXT1;
			break;
		case eTF_BC2:
			ddsh.ddspf = DDSFormats::DDSPF_DXT3;
			break;
		case eTF_BC3:
			ddsh.ddspf = DDSFormats::DDSPF_DXT5;
			break;
		case eTF_B5G6R5:
			ddsh.ddspf = DDSFormats::DDSPF_R5G6B5;
			break;
		case eTF_B8G8R8:
		case eTF_L8V8U8:
			ddsh.ddspf = DDSFormats::DDSPF_R8G8B8;
			break;
		case eTF_R8G8B8A8:
			ddsh.ddspf = DDSFormats::DDSPF_A8R8G8B8;
			break;
		case eTF_L8:
			ddsh.ddspf = DDSFormats::DDSPF_L8;
			break;
		case eTF_A8L8:
			ddsh.ddspf = DDSFormats::DDSPF_A8L8;
			break;
		default:
			assert(0);
			return;
		}
		file.Write(&ddsh, sizeof(ddsh));
	}

	byte* data = NULL;

	if (eDstF == eTF_B8G8R8 || eDstF == eTF_L8V8U8)
	{
		data = new byte[Size];
		i32 n = Size / 3;
		for (i32 i = 0; i < n; i++)
		{
			data[i * 3 + 0] = dat[i * 3 + 2];
			data[i * 3 + 1] = dat[i * 3 + 1];
			data[i * 3 + 2] = dat[i * 3 + 0];
		}
		file.Write(data, Size);
	}
	else if (eDstF == eTF_R8G8B8A8)
	{
		data = new byte[Size];
		i32 n = Size / 4;
		for (i32 i = 0; i < n; i++)
		{
			data[i * 4 + 0] = dat[i * 4 + 2];
			data[i * 4 + 1] = dat[i * 4 + 1];
			data[i * 4 + 2] = dat[i * 4 + 0];
			data[i * 4 + 3] = dat[i * 4 + 3];
		}
		file.Write(data, Size);
	}
	else if (eDstF == eTF_B5G6R5)    // input B8G8R8X8, output B5G6R5
	{
		data = new byte[Size / 2];
		i32 n = Size / 4;
		for (i32 i = 0; i < n; i++)
		{
			ColorB rgba(dat[i * 4 + 0], dat[i * 4 + 1], dat[i * 4 + 2], dat[i * 4 + 3]);

			*(u16*)(&data[i * 2]) = rgba.pack_rgb565();
		}
		file.Write(data, Size / 2);
	}
	else if (eDstF == eTF_B4G4R4A4)    // input B8G8R8X8, output B4G4R4A4
	{
		data = new byte[Size / 2];
		i32 n = Size / 4;

		if (bDither)
			for (i32 i = 0; i < n; i++)
			{
				i32 a = clamp_tpl((i32)dat[i * 4 + 0] + (rand() % 16) - 8, 0, 255);
				i32 b = clamp_tpl((i32)dat[i * 4 + 1] + (rand() % 16) - 8, 0, 255);
				i32 c = clamp_tpl((i32)dat[i * 4 + 2] + (rand() % 16) - 8, 0, 255);
				i32 d = clamp_tpl((i32)dat[i * 4 + 3] + (rand() % 16) - 8, 0, 255);

				*(u16*)(&data[i * 2]) = ColorB(a, b, c, d).pack_argb4444();
			}
		else
			for (i32 i = 0; i < n; i++)
			{
				ColorB rgba(dat[i * 4 + 0], dat[i * 4 + 1], dat[i * 4 + 2], dat[i * 4 + 3]);

				*(u16*)(&data[i * 2]) = rgba.pack_argb4444();
			}
		file.Write(data, Size / 2);
	}
	else if (eSrcF == eTF_R8G8B8A8 && eDstF == eTF_L8)
	{
		i32k n = Size / 4;
		data = new byte[n];

		for (i32 i = 0; i < n; i++)
			data[i] = dat[i * 4 + 1];     // copy green
		file.Write(data, n);
	}
	else if (eSrcF == eTF_R8G8B8A8 && eDstF == eTF_A8L8)
	{
		data = new byte[Size / 2];
		i32k n = Size / 4;

		for (i32 i = 0; i < n; i++)
		{
			data[i * 2 + 0] = dat[i * 4 + 1];     // copy green
			data[i * 2 + 1] = dat[i * 4 + 3];     // copy alpha
		}
		file.Write(data, n);
	}
	else if (eSrcF == eTF_R8G8B8A8 && CImageExtensionHelper::IsBlockCompressed(eDstF))
	{
		bool bUseHW = false;//!m_bHighQuality;

		AUTO_LOCK(m_sFileLock);
		m_pFile = &file;
		GetIEditorImpl()->GetRenderer()->DXTCompress(dat, w, h, eDstF, bUseHW, false, 4, SaveCompressedMipmapLevel);
		m_pFile = 0;
	}
	else
	{
		assert(eSrcF == eDstF);

		file.Write(dat, Size);
	}

	SAFE_DELETE_ARRAY(data);
}

