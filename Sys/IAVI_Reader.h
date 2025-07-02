// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IAVI_Reader.h
//  Version:     v1.00
//  Описание: AVI files reader Interface.
// -------------------------------------------------------------------------
//  История:
//  Created:     28/02/2007 by MarcoC.
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IAVI_Reader_h__
#define __IAVI_Reader_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
struct IAVI_Reader
{
	virtual ~IAVI_Reader(){}
	virtual bool OpenFile(tukk szFilename) = 0;
	virtual void CloseFile() = 0;

	virtual i32  GetWidth() = 0;
	virtual i32  GetHeight() = 0;
	virtual i32  GetFPS() = 0;

	//! If no frame is passed, it will retrieve the current one and advance one frame.
	//! If a frame is specified, it will get that one.
	//! Notice the "const" - don't override this memory!.
	virtual u8k* QueryFrame(i32 nFrame = -1) = 0;

	virtual i32                  GetFrameCount() = 0;
	virtual i32                  GetAVIPos() = 0;
	virtual void                 SetAVIPos(i32 nFrame) = 0;
};

#endif // __IAVI_Reader_h__
