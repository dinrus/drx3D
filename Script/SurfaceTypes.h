// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SurfaceTypes.h
//  Version:     v1.00
//  Created:     30/9/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SurfaceTypes_h__
#define __SurfaceTypes_h__
#pragma once

#include <drx3D/Script/IScriptSystem.h>

struct ISurfaceType;

//////////////////////////////////////////////////////////////////////////
// SurfaceTypes loader.
//////////////////////////////////////////////////////////////////////////
class CScriptSurfaceTypesLoader
{
public:
	CScriptSurfaceTypesLoader();
	~CScriptSurfaceTypesLoader();

	void ReloadSurfaceTypes();
	bool LoadSurfaceTypes(tukk sFolder, bool bReload);
	void UnloadSurfaceTypes();

	void UnregisterSurfaceType(ISurfaceType* sfType);

private:
	std::vector<ISurfaceType*> m_surfaceTypes;
	std::vector<string>        m_folders;
};

#endif // __SurfaceTypes_h__
