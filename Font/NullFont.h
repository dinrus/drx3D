// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Dummy font implementation (dedicated server)
   -------------------------------------------------------------------------
   История:
   - Jun 30,2006:	Created by Sascha Demetrio

*************************************************************************/

#ifndef __NULLFONT_H__
#define __NULLFONT_H__

#pragma once

#if defined(USE_NULLFONT)

	#include <drx3D/Font/IFont.h>

class CNullFont : public IFFont
{
public:
	CNullFont() {}
	virtual ~CNullFont() {}

	virtual void         Release()                                                                                                           {}
	virtual bool         Load(tukk pFontFilePath, u32 width, u32 height, u32 flags)                        { return true; }
	virtual bool         Load(tukk pXMLFile)                                                                                          { return true; }
	virtual void         Free()                                                                                                              {}

	virtual void         DrawString(float x, float y, tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx)              {}
	virtual void         DrawString(float x, float y, float z, tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx)     {}

	virtual void         DrawStringW(float x, float y, const wchar_t* pStr, const bool asciiMultiLine, const STextDrawContext& ctx)          {}
	virtual void         DrawStringW(float x, float y, float z, const wchar_t* pStr, const bool asciiMultiLine, const STextDrawContext& ctx) {}

	virtual Vec2         GetTextSize(tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx)                               { return Vec2(0.0f, 0.0f); }
	virtual Vec2         GetTextSizeW(const wchar_t* pStr, const bool asciiMultiLine, const STextDrawContext& ctx)                           { return Vec2(0.0f, 0.0f); }

	virtual size_t       GetTextLength(tukk pStr, const bool asciiMultiLine) const                                                    { return 0; }
	virtual size_t       GetTextLengthW(const wchar_t* pStr, const bool asciiMultiLine) const                                                { return 0; }

	virtual void         WrapText(string& result, float maxWidth, tukk pStr, const STextDrawContext& ctx)                             { result = pStr; }
	virtual void         WrapTextW(wstring& result, float maxWidth, const wchar_t* pStr, const STextDrawContext& ctx)                        { result = pStr; }

	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const                                                                             {}

	virtual void         GetGradientTextureCoord(float& minU, float& minV, float& maxU, float& maxV) const                                   {}

	virtual u32 GetEffectId(tukk pEffectName) const                                                                          { return 0; }
	virtual void         RenderCallback(float x, float y, float z, tukk pStr, const bool asciiMultiLine, const STextDrawContext& ctx, IRenderAuxGeom *pAux) {}
};

class CDrxNullFont : public IDrxFont
{
public:
	virtual void    Release()                                   {}
	virtual IFFont* NewFont(tukk pFontName)              { return &ms_nullFont; }
	virtual IFFont* GetFont(tukk pFontName) const        { return &ms_nullFont; }
	virtual void    SetRendererProperties(IRenderer* pRenderer) {}
	virtual void    GetMemoryUsage(IDrxSizer* pSizer) const     {}
	virtual string  GetLoadedFontNames() const                  { return ""; }

private:
	static CNullFont ms_nullFont;
};

#endif // USE_NULLFONT

#endif
