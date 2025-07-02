// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: UI draw functions interface

   -------------------------------------------------------------------------
   История:
   - 07:11:2005: Created by Julien Darre

*************************************************************************/

#ifndef __IUIDRAW_H__
#define __IUIDRAW_H__

//-----------------------------------------------------------------------------------------------------

struct IFFont;

enum EUIDRAWHORIZONTAL
{
	UIDRAWHORIZONTAL_LEFT,
	UIDRAWHORIZONTAL_CENTER,
	UIDRAWHORIZONTAL_RIGHT
};

enum EUIDRAWVERTICAL
{
	UIDRAWVERTICAL_TOP,
	UIDRAWVERTICAL_CENTER,
	UIDRAWVERTICAL_BOTTOM
};

//-----------------------------------------------------------------------------------------------------

struct IUIDraw
{
	virtual ~IUIDraw(){}
	virtual void Release() = 0;

	virtual void PreRender() = 0;
	virtual void PostRender() = 0;

	// TODO: uintARGB or float,float,float,float ?

	virtual u32 GetColorARGB(u8 ucAlpha, u8 ucRed, u8 ucGreen, u8 ucBlue) = 0;

	virtual i32    CreateTexture(tukk strName, bool dontRelease = true) = 0;

	virtual bool   DeleteTexture(i32 iTextureID) = 0;

	virtual void   GetTextureSize(i32 iTextureID, float& rfSizeX, float& rfSizeY) = 0;

	virtual void   DrawTriangle(float fX0, float fY0, float fX1, float fY1, float fX2, float fY2, u32 uiColor) = 0;

	virtual void   DrawLine(float fX1, float fY1, float fX2, float fY2, u32 uiDiffuse) = 0;

	virtual void   DrawQuadSimple(float fX, float fY, float fSizeX, float fSizeY, u32 uiDiffuse, i32 iTextureID) = 0;

	virtual void   DrawQuad(float fX,
	                        float fY,
	                        float fSizeX,
	                        float fSizeY,
	                        u32 uiDiffuse = 0,
	                        u32 uiDiffuseTL = 0, u32 uiDiffuseTR = 0, u32 uiDiffuseDL = 0, u32 uiDiffuseDR = 0,
	                        i32 iTextureID = -1,
	                        float fUTexCoordsTL = 0.0f, float fVTexCoordsTL = 0.0f,
	                        float fUTexCoordsTR = 1.0f, float fVTexCoordsTR = 0.0f,
	                        float fUTexCoordsDL = 0.0f, float fVTexCoordsDL = 1.0f,
	                        float fUTexCoordsDR = 1.0f, float fVTexCoordsDR = 1.0f,
	                        bool bUse169 = true) = 0;

	virtual void DrawImage(i32 iTextureID, float fX,
	                       float fY,
	                       float fSizeX,
	                       float fSizeY,
	                       float fAngleInDegrees,
	                       float fRed,
	                       float fGreen,
	                       float fBlue,
	                       float fAlpha,
	                       float fS0 = 0.0f,
	                       float fT0 = 0.0f,
	                       float fS1 = 1.0f,
	                       float fT1 = 1.0f) = 0;

	virtual void DrawImageCentered(i32 iTextureID, float fX,
	                               float fY,
	                               float fSizeX,
	                               float fSizeY,
	                               float fAngleInDegrees,
	                               float fRed,
	                               float fGreen,
	                               float fBlue,
	                               float fAlpha) = 0;

	virtual void DrawTextSimple(IFFont* pFont,
	                            float fX, float fY,
	                            float fSizeX, float fSizeY,
	                            tukk strText, ColorF color,
	                            EUIDRAWHORIZONTAL eUIDrawHorizontal, EUIDRAWVERTICAL eUIDrawVertical) = 0;

	virtual void DrawText(IFFont* pFont,
	                      float fX,
	                      float fY,
	                      float fSizeX,
	                      float fSizeY,
	                      tukk strText,
	                      float fAlpha,
	                      float fRed,
	                      float fGreen,
	                      float fBlue,
	                      EUIDRAWHORIZONTAL eUIDrawHorizontalDocking,
	                      EUIDRAWVERTICAL eUIDrawVerticalDocking,
	                      EUIDRAWHORIZONTAL eUIDrawHorizontal,
	                      EUIDRAWVERTICAL eUIDrawVertical) = 0;

	virtual void DrawWrappedText(IFFont* pFont,
	                             float fX,
	                             float fY,
	                             float fMaxWidth,
	                             float fSizeX,
	                             float fSizeY,
	                             tukk strText,
	                             float fAlpha,
	                             float fRed,
	                             float fGreen,
	                             float fBlue,
	                             EUIDRAWHORIZONTAL eUIDrawHorizontalDocking,
	                             EUIDRAWVERTICAL eUIDrawVerticalDocking,
	                             EUIDRAWHORIZONTAL eUIDrawHorizontal,
	                             EUIDRAWVERTICAL eUIDrawVertical) = 0;

	virtual void GetTextDim(IFFont* pFont,
	                        float* fWidth,
	                        float* fHeight,
	                        float fSizeX,
	                        float fSizeY,
	                        tukk strText) = 0;

	virtual void GetWrappedTextDim(IFFont* pFont,
	                               float* fWidth,
	                               float* fHeight,
	                               float fMaxWidth,
	                               float fSizeX,
	                               float fSizeY,
	                               tukk strText) = 0;

};

//-----------------------------------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------------------------------
