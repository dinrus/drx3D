// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   BMPHelper.h
//  Version:     v1.00
//  Created:     28/11/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: BMPHelper
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BMPHELPER_H__
#define __BMPHELPER_H__

#pragma once

namespace BMPHelper
{
// load a BMP. if pByteData is 0, only reports dimensions
// when pFile is given, restores read location after getting dimensions only
bool LoadBMP(tukk filename, u8* pByteData, i32& width, i32& height, i32& depth, bool bForceInverseY = false);
bool LoadBMP(FILE* pFile, u8* pByteData, i32& width, i32& height, i32& depth, bool bForceInverseY = false);

// save pByteData BGR[A] into a new file 'filename'. if bFlipY flips y.
// if depth==4 assumes BGRA
bool SaveBMP(tukk filename, u8* pByteData, i32 width, i32 height, i32 depth, bool inverseY);
// save pByteData BGR[A] into a file pFile. if bFlipY flips y.
// if depth==4 assumes BGRA
bool   SaveBMP(FILE* pFile, u8* pByteData, i32 width, i32 height, i32 depth, bool inverseY);
// calculate size of BMP incl. Header and padding bytes
size_t CalcBMPSize(i32 width, i32 height, i32 depth);
};

#endif
