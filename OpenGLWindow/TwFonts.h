//  ---------------------------------------------------------------------------
//
//  @file       TwFonts.h
//  @brief      Bitmaps fonts
//  @author     Philippe Decaudin - http://www.antisphere.com
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//
//  note:       Private header
//
//  ---------------------------------------------------------------------------

#if !defined ANT_TW_FONTS_INCLUDED
#define ANT_TW_FONTS_INCLUDED

#include <drxtypes.h>
//#include <AntTweakBar.h>

/*
A source bitmap includes 224 characters starting from ascii char 32 (i.e. space) to ascii char 255:
  
 !"#$%&'()*+,-./0123456789:;<=>?
@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
`abcdefghijklmnopqrstuvwxyz{|}~
€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ
 ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿
ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß
àáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ

First column of a source bitmap is a delimiter with color=zero at the end of each line of characters.
Last row of a line of characters is a delimiter with color=zero at the last pixel of each character.

*/

struct CTexFont
{
	u8 *m_TexBytes;
	i32 m_TexWidth;   // power of 2
	i32 m_TexHeight;  // power of 2
	float m_CharU0[256];
	float m_CharV0[256];
	float m_CharU1[256];
	float m_CharV1[256];
	i32 m_CharWidth[256];
	i32 m_CharHeight;
	i32 m_NbCharRead;

	CTexFont();
	~CTexFont();
};

CTexFont *TwGenerateFont(u8k *_Bitmap, i32 _BmWidth, i32 _BmHeight);

extern CTexFont *g_DefaultSmallFont;
extern CTexFont *g_DefaultNormalFont;
extern CTexFont *g_DefaultNormalFontAA;
extern CTexFont *g_DefaultLargeFont;
extern CTexFont *g_DefaultFixed1Font;

void TwGenerateDefaultFonts();
void TwDeleteDefaultFonts();

#endif  // !defined ANT_TW_FONTS_INCLUDED
