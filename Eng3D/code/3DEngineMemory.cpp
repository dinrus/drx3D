// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   3DEngineMemory.h
//  Version:     v1.00
//  Created:     23/04/2010 by Chris Raine.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
// -------------------------------------------------------------------------
//  История:
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/3DEngineMemory.h>

// Static CTemporaryPool instance
CTemporaryPool* CTemporaryPool::s_Instance = NULL;

namespace util
{
uk pool_allocate(size_t nSize)
{
	return CTemporaryPool::Get()->Allocate(nSize, 16);  // Align for possible SIMD types
}
void pool_free(uk ptr)
{
	return CTemporaryPool::Get()->Free(ptr);
}
}
