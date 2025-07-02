// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   settings.h
//  Version:     v1.00
//  Created:     14/1/2003 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio.NET
//  Description: General editor settings.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include <QString>

// DrxCommon
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

//////////////////////////////////////////////////////////////////////////
/** Various editor settings.
 */
struct SEditorSettings
{
	static void Load();
	static void LoadFrom(const QString& filename);

	static void LoadValue(tukk sSection, tukk sKey, i32& value);
	static void LoadValue(tukk sSection, tukk sKey, u32& value);
	static void LoadValue(tukk sSection, tukk sKey, u64& value);
	static void LoadValue(tukk sSection, tukk sKey, float& value);
	static void LoadValue(tukk sSection, tukk sKey, bool& value);
	static void LoadValue(tukk sSection, tukk sKey, string& value);

private:
	SEditorSettings();
};

