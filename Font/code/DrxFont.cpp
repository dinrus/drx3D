// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Font/StdAfx.h>

#if !defined(USE_NULLFONT_ALWAYS)

	#include <drx3D/Font/drxFont.h>
	#include <drx3D/Font/FFont.h>
	#include <drx3D/Font/FontTexture.h>
	#include <drx3D/Font/FontRenderer.h>

	#if !defined(_RELEASE)
static void DumpFontTexture(IConsoleCmdArgs* pArgs)
{
	if (pArgs->GetArgCount() != 2)
		return;

	tukk pFontName = pArgs->GetArg(1);

	if (pFontName && *pFontName && *pFontName != '0')
	{
		string fontFile(pFontName);
		fontFile += ".bmp";

		CFFont* pFont = (CFFont*) gEnv->pDrxFont->GetFont(pFontName);
		if (pFont)
		{
			pFont->GetFontTexture()->WriteToFile(fontFile.c_str());
			gEnv->pLog->LogWithType(IMiniLog::eInputResponse, "Dumped \"%s\" texture to \"%s\"!", pFontName, fontFile.c_str());
		}
	}
}

static void DumpFontNames(IConsoleCmdArgs* pArgs)
{
	string names = gEnv->pDrxFont->GetLoadedFontNames();
	gEnv->pLog->LogWithType(IMiniLog::eInputResponse, "Currently loaded fonts: %s", names.c_str());
}
	#endif

CDrxFont::CDrxFont(ISystem* pSystem)
	: m_pSystem(pSystem)
	, m_fonts()
	, m_rndPropIsRGBA(false)
	, m_rndPropHalfTexelOffset(0.5f)
{
	assert(m_pSystem);

	DrxLogAlways("Using FreeType %d.%d.%d", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);

	#if !defined(_RELEASE)
	REGISTER_COMMAND("r_DumpFontTexture", DumpFontTexture, 0,
	                 "Dumps the specified font's texture to a bitmap file\n"
	                 "Use r_DumpFontTexture to get the loaded font names\n"
	                 "Usage: r_DumpFontTexture <fontname>");
	REGISTER_COMMAND("r_DumpFontNames", DumpFontNames, 0,
	                 "Logs a list of fonts currently loaded");
	#endif
}

CDrxFont::~CDrxFont()
{
	for (FontMapItor it = m_fonts.begin(), itEnd = m_fonts.end(); it != itEnd; )
	{
		CFFont* pFont = it->second;
		++it; // iterate as Release() below will remove font from the map
		SAFE_RELEASE(pFont);
	}
}

void CDrxFont::Release()
{
	delete this;
}

IFFont* CDrxFont::NewFont(tukk pFontName)
{
	string name = pFontName;
	name.MakeLower();

	FontMapItor it = m_fonts.find(CONST_TEMP_STRING(name.c_str()));
	if (it != m_fonts.end())
		return it->second;

	CFFont* pFont = new CFFont(m_pSystem, this, name.c_str());
	m_fonts.insert(FontMapItor::value_type(name, pFont));
	return pFont;
}

IFFont* CDrxFont::GetFont(tukk pFontName) const
{
	FontMapConstItor it = m_fonts.find(CONST_TEMP_STRING(pFontName));
	return it != m_fonts.end() ? it->second : 0;
}

void CDrxFont::SetRendererProperties(IRenderer* pRenderer)
{
	if (pRenderer)
	{
		m_rndPropIsRGBA = (pRenderer->GetFeatures() & RFT_RGBA) != 0;
		m_rndPropHalfTexelOffset = 0.0f;
	}
}

void CDrxFont::GetMemoryUsage(IDrxSizer* pSizer) const
{
	if (!pSizer->Add(*this))
		return;

	pSizer->AddObject(m_fonts);

	#ifndef _LIB
	{
		SIZER_COMPONENT_NAME(pSizer, "STL Allocator Waste");
		DrxModuleMemoryInfo meminfo;
		ZeroStruct(meminfo);
		DrxGetMemoryInfoForModule(&meminfo);
		pSizer->AddObject((this + 2), (size_t)meminfo.STL_wasted);
	}
	#endif
}

string CDrxFont::GetLoadedFontNames() const
{
	string ret;
	for (FontMapConstItor it = m_fonts.begin(), itEnd = m_fonts.end(); it != itEnd; ++it)
	{
		CFFont* pFont = it->second;
		if (pFont)
		{
			if (!ret.empty())
				ret += ",";
			ret += pFont->GetName();
		}
	}
	return ret;
}

void CDrxFont::UnregisterFont(tukk pFontName)
{
	FontMapItor it = m_fonts.find(CONST_TEMP_STRING(pFontName));
	if (it != m_fonts.end())
		m_fonts.erase(it);
}

#endif
