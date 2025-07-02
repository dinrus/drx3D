// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "PluginAPI.h"

struct PLUGIN_API SMFCFonts
{
	//! Use instead of gMFCFonts. Will come in handy for MFC plugins
	static SMFCFonts& GetInstance();

	SMFCFonts();
	HFONT hSystemFont;         // Default system GUI font.
	HFONT hSystemFontBold;     // Default system GUI bold font.
	HFONT hSystemFontItalic;   // Default system GUI italic font.
	i32   nDefaultFontHieght;  // Default font height for 8 logical units.
};

extern SMFCFonts gMFCFonts;

