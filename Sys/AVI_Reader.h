// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   AVI_Reader.h
//  Version:     v1.00
//  Описание: Чтение файлов AVI.
// -------------------------------------------------------------------------
//  История:
//  Created:     28/02/2007 by MarcoC.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __AVI_Reader_h__
#define __AVI_Reader_h__
#pragma once

#include <drx3D/Sys/IAVI_Reader.h>

struct tCaptureAVI_VFW;

//////////////////////////////////////////////////////////////////////////
// Читатель файлов AVI.
//////////////////////////////////////////////////////////////////////////
class CAVI_Reader : public IAVI_Reader
{
public:
	CAVI_Reader();
	~CAVI_Reader();

	bool OpenFile(tukk szFilename);
	void CloseFile();

	i32  GetWidth();
	i32  GetHeight();
	i32  GetFPS();

	// Если кадр не передан, будет получен текущий и продвинут на один вперёд.
	// Если кадр указан, будет получен он.
	// Заметьте "const", и не переопределяйте эту память!
	u8k* QueryFrame(i32 nFrame = -1);

	i32                  GetFrameCount();
	i32                  GetAVIPos();
	void                 SetAVIPos(i32 nFrame);

private:

	void PrintAVIError(i32 hr);

	void InitCapture_VFW();
	i32  OpenAVI_VFW(tukk filename);

	tCaptureAVI_VFW* m_capture;
};

#endif // __AVI_Reader_h__
