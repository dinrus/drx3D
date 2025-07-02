// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: UI draw functions

   -------------------------------------------------------------------------
   История:
   - 07:11:2005: Created by Julien Darre

*************************************************************************/
#ifndef __UIDRAW_H__
#define __UIDRAW_H__

//-----------------------------------------------------------------------------------------------------

#include "IUIDraw.h"

//-----------------------------------------------------------------------------------------------------

class CUIDraw : public IUIDraw
{
public:
	void   Release();

	void   PreRender();
	void   PostRender();

	u32 GetColorARGB(u8 ucAlpha, u8 ucRed, u8 ucGreen, u8 ucBlue);

	i32    CreateTexture(tukk strName, bool dontRelease = true);

	bool   DeleteTexture(i32 iTextureID);

	void   GetTextureSize(i32 iTextureID, float& rfSizeX, float& rfSizeY);

	void   GetMemoryStatistics(IDrxSizer* s);

	void   DrawTriangle(float fX0, float fY0, float fX1, float fY1, float fX2, float fY2, u32 uiColor);

	void   DrawLine(float fX1, float fY1, float fX2, float fY2, u32 uiDiffuse);

	void   DrawQuadSimple(float fX, float fY, float fSizeX, float fSizeY, u32 uiDiffuse, i32 iTextureID);

	void   DrawQuad(float fX,
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
	                bool bUse169 = true);

	void DrawImage(i32 iTextureID, float fX,
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
	               float fT1 = 1.0f);

	void DrawImageCentered(i32 iTextureID, float fX,
	                       float fY,
	                       float fSizeX,
	                       float fSizeY,
	                       float fAngleInDegrees,
	                       float fRed,
	                       float fGreen,
	                       float fBlue,
	                       float fAlpha);

	void DrawTextSimple(IFFont* pFont,
	                    float fX, float fY,
	                    float fSizeX, float fSizeY,
	                    tukk strText, ColorF color,
	                    EUIDRAWHORIZONTAL eUIDrawHorizontal, EUIDRAWVERTICAL eUIDrawVertical);

	void DrawText(IFFont* pFont,
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
	              EUIDRAWVERTICAL eUIDrawVertical);

	void DrawWrappedText(IFFont* pFont,
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
	                     EUIDRAWVERTICAL eUIDrawVertical);

	void GetTextDim(IFFont* pFont,
	                float* fWidth,
	                float* fHeight,
	                float fSizeX,
	                float fSizeY,
	                tukk strText);

	void GetWrappedTextDim(IFFont* pFont,
	                       float* fWidth,
	                       float* fHeight,
	                       float fMaxWidth,
	                       float fSizeX,
	                       float fSizeY,
	                       tukk strText);

	// ~IUIDraw

	CUIDraw();
	~CUIDraw();

protected:
	void InternalDrawText(IFFont* pFont,
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
	                      EUIDRAWVERTICAL eUIDrawVertical);

	void InternalGetTextDim(IFFont* pFont,
	                        float* fWidth,
	                        float* fHeight,
	                        float fMaxWidth,
	                        float fSizeX,
	                        float fSizeY,
	                        tukk strText);

	typedef std::map<i32, ITexture*> TTexturesMap;
	TTexturesMap m_texturesMap;

	IRenderer*   m_pRenderer;
};

//-----------------------------------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------------------------------
