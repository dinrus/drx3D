// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "GlyphBitmap.h"
#include <drx/plugin/freetype/ft2build.h>
#include <drx/plugin/freetype/freetype.h>

// Соответствует набору символов Unicode. This value covers all versions of the Unicode repertoire,
// including ASCII and Latin-1. Most fonts include a Unicode charmap, but not all of them.
#define FONT_ENCODING_UNICODE (FT_ENCODING_UNICODE)

// Corresponds to the Microsoft Symbol encoding, used to encode mathematical symbols in the 32..255 character code range.
// For more information, see `http://www.ceviz.net/symbol.htm'.
#define FONT_ENCODING_SYMBOL (FT_ENCODING_MS_SYMBOL)

// Corresponds to Microsoft's Japanese SJIS encoding.
// More info at `http://langsupport.japanreference.com/encoding.shtml'. See note on multi-byte encodings below.
#define FONT_ENCODING_SJIS (FT_ENCODING_MS_SJIS)

// Corresponds to the encoding system for Simplified Chinese, as used in China. Only found in some TrueType fonts.
#define FONT_ENCODING_GB2312 (FT_ENCODING_MS_GB2312)

// Corresponds to the encoding system for Traditional Chinese, as used in Taiwan and Hong Kong. Only found in some TrueType fonts.
#define FONT_ENCODING_BIG5 (FT_ENCODING_MS_BIG5)

// Corresponds to the Korean encoding system known as Wansung.
// This is a Microsoft encoding that is only found in some TrueType fonts.
// For more information, see `http://www.microsoft.com/typography/unicode/949.txt'.
#define FONT_ENCODING_WANSUNG (FT_ENCODING_MS_WANSUNG)

// The Korean standard character set (KS C-5601-1992), which corresponds to Windows code page 1361.
// This character set includes all possible Hangeul character combinations. Only found on some rare TrueType fonts.
#define FONT_ENCODING_JOHAB (FT_ENCODING_MS_JOHAB)

//------------------------------------------------------------------------------------
class CFontRenderer
{
public:

	CFontRenderer();
	~CFontRenderer();

	i32         LoadFromFile(const string& szFileName);
	i32         LoadFromMemory(u8* pBuffer, i32 iBufferSize);
	i32         Release();

	i32         SetGlyphBitmapSize(i32 iWidth, i32 iHeight);
	i32         GetGlyphBitmapSize(i32* pWidth, i32* pHeight);

	i32         SetSizeRatio(float fSizeRatio) { m_fSizeRatio = fSizeRatio; return 1; };
	float       GetSizeRatio()                 { return m_fSizeRatio; };

	i32         SetEncoding(FT_Encoding pEncoding);
	FT_Encoding GetEncoding() { return m_pEncoding; };

	i32         GetGlyph(CGlyphBitmap* pGlyphBitmap, u8* iGlyphWidth, u8* iGlyphHeight, char& iCharOffsetX, char& iCharOffsetY, i32 iX, i32 iY, i32 iCharCode);
	i32         GetGlyphScaled(CGlyphBitmap* pGlyphBitmap, i32* iGlyphWidth, i32* iGlyphHeight, i32 iX, i32 iY, float fScaleX, float fScaleY, i32 iCharCode);

	void        GetMemoryUsage(IDrxSizer* pSizer) const {}

private:

	FT_Library   m_pLibrary;
	FT_Face      m_pFace;
	FT_GlyphSlot m_pGlyph;
	float        m_fSizeRatio;

	FT_Encoding  m_pEncoding;

	i32          m_iGlyphBitmapWidth;
	i32          m_iGlyphBitmapHeight;
};
