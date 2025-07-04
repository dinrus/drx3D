// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   AVI_Reader.h
//  Version:     v1.00
//  Описание: AVI files reader.
// -------------------------------------------------------------------------
//  История:
//  Created:     28/02/2007 by MarcoC.
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/System.h>

#include <drx3D/Sys/AVI_Reader.h>

//////////////////////////////////////////////////////////////////////////
IAVI_Reader* CSystem::CreateAVIReader()
{
#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_DURANGO && !DRX_PLATFORM_ORBIS
	CAVI_Reader* pAVIReader = new CAVI_Reader();
	return (pAVIReader);
#else
	return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CSystem::ReleaseAVIReader(IAVI_Reader* pAVIReader)
{
#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_DURANGO && !DRX_PLATFORM_ORBIS
	SAFE_DELETE(pAVIReader);
#endif
}

#if DRX_PLATFORM_WINDOWS
	#include <vfw.h>
//LINK_SYSTEM_LIBRARY("libvfw32.a")

//////////////////////////////////////////////////////////////////////////
typedef struct tCaptureAVI_VFW
{
	PAVIFILE          avifile;
	PAVISTREAM        avistream;
	PGETFRAME         getframe;
	AVISTREAMINFO     aviinfo;
	BITMAPINFOHEADER* bmih;
	double            fps;
	i32               pos;
	i32               w;
	i32               h;
	i32               start_index;
	i32               end_index;
	u8*    pImage;
} tCaptureAVI_VFW;

//////////////////////////////////////////////////////////////////////////
CAVI_Reader::CAVI_Reader()
{
	m_capture = new tCaptureAVI_VFW;
	memset(m_capture, 0, sizeof(*m_capture));
}

//////////////////////////////////////////////////////////////////////////
CAVI_Reader::~CAVI_Reader()
{
	CloseFile();
	SAFE_DELETE(m_capture);
}

//////////////////////////////////////////////////////////////////////////
void CAVI_Reader::InitCapture_VFW()
{
	static i32 isInitialized = 0;
	if (!isInitialized)
	{
		AVIFileInit();
		isInitialized = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
i32  CAVI_Reader::GetWidth()      { return(m_capture->w); }
i32  CAVI_Reader::GetHeight()     { return(m_capture->h); }
i32  CAVI_Reader::GetFPS()        { return((i32)m_capture->fps); }
//////////////////////////////////////////////////////////////////////////
i32  CAVI_Reader::GetFrameCount() { return(m_capture->end_index - m_capture->start_index); }
i32  CAVI_Reader::GetAVIPos()     { return(m_capture->pos); }
//////////////////////////////////////////////////////////////////////////
void CAVI_Reader::SetAVIPos(i32 nFrame)
{
	if (nFrame < m_capture->start_index)
		nFrame = m_capture->start_index;
	else if (nFrame > m_capture->end_index)
		nFrame = m_capture->end_index;
	m_capture->pos = nFrame;
}

//////////////////////////////////////////////////////////////////////////
void CAVI_Reader::PrintAVIError(i32 hr)
{

	if (hr == AVIERR_UNSUPPORTED)
		OutputDebugString("AVIERR_UNSUPPORTED\n");
	if (hr == AVIERR_BADFORMAT)
		OutputDebugString("AVIERR_BADFORMAT\n");
	if (hr == AVIERR_MEMORY)
		OutputDebugString("AVIERR_MEMORY\n");
	if (hr == AVIERR_INTERNAL)
		OutputDebugString("AVIERR_INTERNAL\n");
	if (hr == AVIERR_BADFLAGS)
		OutputDebugString("AVIERR_BADFLAGS\n");
	if (hr == AVIERR_BADPARAM)
		OutputDebugString("AVIERR_BADPARAM\n");
	if (hr == AVIERR_BADSIZE)
		OutputDebugString("AVIERR_BADSIZE\n");
	if (hr == AVIERR_BADHANDLE)
		OutputDebugString("AVIERR_BADHANDLE\n");
	if (hr == AVIERR_FILEREAD)
		OutputDebugString("AVIERR_FILEREAD\n");
	if (hr == AVIERR_FILEWRITE)
		OutputDebugString("AVIERR_FILEWRITE\n");
	if (hr == AVIERR_FILEOPEN)
		OutputDebugString("AVIERR_FILEOPEN\n");
	if (hr == AVIERR_COMPRESSOR)
		OutputDebugString("AVIERR_COMPRESSOR\n");
	if (hr == AVIERR_NOCOMPRESSOR)
		OutputDebugString("AVIERR_NOCOMPRESSOR\n");
	if (hr == AVIERR_READONLY)
		OutputDebugString("AVIERR_READONLY\n");
	if (hr == AVIERR_NODATA)
		OutputDebugString("AVIERR_NODATA\n");
	if (hr == AVIERR_BUFFERTOOSMALL)
		OutputDebugString("AVIERR_BUFFERTOOSMALL\n");
	if (hr == AVIERR_CANTCOMPRESS)
		OutputDebugString("AVIERR_CANTCOMPRESS\n");
	if (hr == AVIERR_USERABORT)
		OutputDebugString("AVIERR_USERABORT\n");
	if (hr == AVIERR_ERROR)
		OutputDebugString("AVIERR_ERROR\n");
}

//////////////////////////////////////////////////////////////////////////
void CAVI_Reader::CloseFile()
{

	if (m_capture->getframe)
	{
		AVIStreamGetFrameClose(m_capture->getframe);
		m_capture->getframe = 0;
	}

	if (m_capture->avistream)
	{
		AVIStreamRelease(m_capture->avistream);
		m_capture->avistream = 0;
	}

	if (m_capture->avifile)
	{
		AVIFileRelease(m_capture->avifile);
		m_capture->avifile = 0;
	}

	// static memory allocated by windows AVIStreamGetFrame should not be freed
	//if (m_capture->pImage)
	//	SAFE_DELETE_ARRAY(m_capture->pImage);

	//m_capture->bmih = 0;
	//m_capture->pos = 0;
	memset(m_capture, 0, sizeof(*m_capture));
}

//////////////////////////////////////////////////////////////////////////
u8k* CAVI_Reader::QueryFrame(i32 nFrame)
{
	if (m_capture->avistream)
	{
		if (nFrame >= 0)
			SetAVIPos(nFrame);

		m_capture->bmih = (BITMAPINFOHEADER*)AVIStreamGetFrame(m_capture->getframe, m_capture->pos++);

		if (m_capture->bmih)
			m_capture->pImage = (u8*)(m_capture->bmih + 1);
		else
			m_capture->pImage = NULL;
	}
	else
		m_capture->pImage = NULL;

	return (m_capture->pImage);
}

//////////////////////////////////////////////////////////////////////////
i32 CAVI_Reader::OpenAVI_VFW(tukk filename)
{
	i32 hr;

	CloseFile();

	InitCapture_VFW();

	hr = AVIFileOpen(&m_capture->avifile, filename, OF_READ, NULL);
	if (SUCCEEDED(hr))
	{
		hr = AVIFileGetStream(m_capture->avifile, &m_capture->avistream, streamtypeVIDEO, 0);
		if (SUCCEEDED(hr))
		{
			hr = AVIStreamInfo(m_capture->avistream, &m_capture->aviinfo,
			                   sizeof(m_capture->aviinfo));
			if (SUCCEEDED(hr))
			{
				m_capture->w = m_capture->aviinfo.rcFrame.right -
				               m_capture->aviinfo.rcFrame.left;
				m_capture->h = m_capture->aviinfo.rcFrame.bottom -
				               m_capture->aviinfo.rcFrame.top;

				BITMAPINFOHEADER bmih;
				memset(&bmih, 0, sizeof(bmih));
				bmih.biSize = sizeof(bmih);
				bmih.biWidth = m_capture->w;
				bmih.biHeight = m_capture->h;
				bmih.biBitCount = (WORD)24;
				bmih.biCompression = BI_RGB;
				bmih.biPlanes = 1;

				m_capture->start_index = (i32)m_capture->aviinfo.dwStart;
				m_capture->end_index = m_capture->start_index + (i32)m_capture->aviinfo.dwLength;
				m_capture->fps = ((double)m_capture->aviinfo.dwRate) / m_capture->aviinfo.dwScale;
				m_capture->pos = m_capture->start_index;
				m_capture->getframe = AVIStreamGetFrameOpen(m_capture->avistream, &bmih);
				if (m_capture->getframe != 0)
					return (1);
			}
			else
				PrintAVIError(hr);
		}
		else
			PrintAVIError(hr);
	}
	else
		PrintAVIError(hr);

	CloseFile();

	return (0);
}

//////////////////////////////////////////////////////////////////////////
bool CAVI_Reader::OpenFile(tukk filename)
{
	if (!filename)
		return (false);

	if (OpenAVI_VFW(filename) != 1)
		return (false);

	return (true);
}

#endif
