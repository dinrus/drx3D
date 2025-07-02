// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AIDebugRenderer.h
   $Id$
   Описание: Helper functions to draw some interesting debug shapes.

   -------------------------------------------------------------------------
   История:
   - 2006-9-19   : Created (AIDebugDrawHelpers.h) by Mikko Mononen
   - 2009-2-11   : Moved to DinrusAction by Evgeny Adamenkov

 *********************************************************************/

#ifndef _NULL_AI_DEBUG_RENDERER_H_
#define _NULL_AI_DEBUG_RENDERER_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/AI/IAIDebugRenderer.h>
#include <drx3D/CoreX/Renderer/ITexture.h>

struct CNullAIDebugRenderer : IAIDebugRenderer
{
	virtual float GetCameraFOV()                                             { return 0; }

	virtual Vec3  GetCameraPos()                                             { return Vec3(ZERO); }
	virtual float GetDebugDrawZ(const Vec3& vPoint, bool bUseTerrainOrWater) { return 0; }

	// Assume 800x600 screen resolution to prevent possible division by zero
	virtual i32  GetWidth()                                                                                                                       { return 800; }
	virtual i32  GetHeight()                                                                                                                      { return 600; }

	virtual void DrawAABB(const AABB& aabb, bool bSolid, const ColorB& color, const EBoundingBoxDrawStyle& bbDrawStyle)                           {}
	virtual void DrawAABB(const AABB& aabb, const Matrix34& matWorld, bool bSolid, const ColorB& color, const EBoundingBoxDrawStyle& bbDrawStyle) {}
	virtual void DrawArrow(const Vec3& vPos, const Vec3& vLength, float fWidth, const ColorB& color)                                              {}
	virtual void DrawCapsuleOutline(const Vec3& vPos0, const Vec3& vPos1, float fRadius, const ColorB& color)                                     {}
	virtual void DrawCircleOutline(const Vec3& vPos, float fRadius, const ColorB& color)                                                          {}
	virtual void DrawCircles(const Vec3& vPos,
	                         float fMinRadius, float fMaxRadius, i32 numRings,
	                         const ColorF& vInsideColor, const ColorF& vOutsideColor) {}
	virtual void      DrawCone(const Vec3& vPos, const Vec3& vDir, float fRadius, float fHeight, const ColorB& color, bool fDrawShaded = true)                                                                                                         {}
	virtual void      DrawCylinder(const Vec3& vPos, const Vec3& vDir, float fRadius, float fHeight, const ColorB& color, bool bDrawShaded = true)                                                                                                     {}
	virtual void      DrawEllipseOutline(const Vec3& vPos, float fRadiusX, float fRadiusY, float fOrientation, const ColorB& color)                                                                                                                    {}
	virtual void      Draw2dLabel(i32 nCol, i32 nRow, tukk szText, const ColorB& color)                                                                                                                                                         {}
	virtual void      Draw2dLabel(float fX, float fY, float fFontSize, const ColorB& color, bool bCenter, tukk text, ...) PRINTF_PARAMS(7, 8)                                                                                                   {}
	virtual void      Draw2dLabelEx(float fX, float fY, float fFontSize, const ColorB& color, bool bFixedSize, bool bMonospace, bool bCenter, bool bFramed, tukk text, ...) PRINTF_PARAMS(10, 11)                                               {}
	virtual void      Draw3dLabel(Vec3 vPos, float fFontSize, tukk text, ...) PRINTF_PARAMS(4, 5)                                                                                                                                               {}
	virtual void      Draw3dLabelEx(Vec3 vPos, float fFontSize, const ColorB& color, bool bFixedSize, bool bCenter, bool bDepthTest, bool bFramed, tukk text, ...) PRINTF_PARAMS(9, 10)                                                         {}
	virtual void      DrawLabel(Vec3 pos, SDrawTextInfo& ti, tukk text)                                                                                                                                                                         {}
	virtual void      Draw2dImage(float fX, float fY, float fWidth, float fHeight, i32 nTextureID, float fS0 = 0, float fT0 = 0, float fS1 = 1, float fT1 = 1, float fAngle = 0, float fR = 1, float fG = 1, float fB = 1, float fA = 1, float fZ = 1) {}
	virtual void      DrawLine(const Vec3& v0, const ColorB& colorV0, const Vec3& v1, const ColorB& colorV1, float fThickness = 1.0f)                                                                                                                  {}
	virtual void      DrawOBB(const OBB& obb, const Vec3& vPos, bool bSolid, const ColorB& color, const EBoundingBoxDrawStyle bbDrawStyle)                                                                                                             {}
	virtual void      DrawOBB(const OBB& obb, const Matrix34& matWorld, bool bSolid, const ColorB& color, const EBoundingBoxDrawStyle bbDrawStyle)                                                                                                     {}
	virtual void      DrawPolyline(const Vec3* va, u32 nPoints, bool bClosed, const ColorB& color, float fThickness = 1.0f)                                                                                                                         {}
	virtual void      DrawPolyline(const Vec3* va, u32 nPoints, bool bClosed, const ColorB* colorArray, float fThickness = 1.0f)                                                                                                                    {}
	virtual void      DrawRangeArc(const Vec3& vPos, const Vec3& vDir, float fAngle, float fRadius, float fWidth,
	                               const ColorB& colorFill, const ColorB& colorOutline, bool bDrawOutline) {}
	virtual void      DrawRangeBox(const Vec3& vPos, const Vec3& vDir, float fSizeX, float fSizeY, float fWidth,
	                               const ColorB& colorFill, const ColorB& colorOutline, bool bDrawOutline) {}
	virtual void      DrawRangeCircle(const Vec3& vPos, float fRadius, float fWidth,
	                                  const ColorB& colorFill, const ColorB& colorOutline, bool bDrawOutline) {}
	virtual void      DrawRangePolygon(const Vec3 polygon[], i32 nVertices, float fWidth,
	                                   const ColorB& colorFill, const ColorB& colorOutline, bool bDrawOutline) {}
	virtual void      DrawSphere(const Vec3& vPos, float fRadius, const ColorB& color, bool bDrawShaded = true)                                         {}
	virtual void      DrawTriangle(const Vec3& v0, const ColorB& colorV0, const Vec3& v1, const ColorB& colorV1, const Vec3& v2, const ColorB& colorV2) {}
	virtual void      DrawTriangles(const Vec3* va, u32 numPoints, const ColorB& color)                                                        {}
	virtual void      DrawWireFOVCone(const Vec3& vPos, const Vec3& vDir, float fRadius, float fFOV, const ColorB& color)                               {}
	virtual void      DrawWireSphere(const Vec3& vPos, float fRadius, const ColorB& color)                                                              {}

	virtual ITexture* LoadTexture(tukk sNameOfTexture, u32 nFlags)                                                                            { return 0; }

	// [9/16/2010 evgeny] ProjectToScreen is not guaranteed to work if used outside Renderer
	virtual bool         ProjectToScreen(float fInX, float fInY, float fInZ, float* fOutX, float* fOutY, float* fOutZ)                                       { return false; }

	virtual void         TextToScreen(float fX, float fY, tukk format, ...) PRINTF_PARAMS(4, 5)                                                       {}
	virtual void         TextToScreenColor(i32 nX, i32 nY, float fRed, float fGreen, float fBlue, float fAlpha, tukk format, ...) PRINTF_PARAMS(8, 9) {}

	virtual void         Init2DMode()                                                                                                                        {}
	virtual void         Init3DMode()                                                                                                                        {}

	virtual void         SetAlphaBlended(bool bOn)                                                                                                           {}
	virtual void         SetBackFaceCulling(bool bOn)                                                                                                        {}
	virtual void         SetDepthTest(bool bOn)                                                                                                              {}
	virtual void         SetDepthWrite(bool bOn)                                                                                                             {}
	virtual void         SetDrawInFront(bool bOn)                                                                                                            {}

	virtual void         SetMaterialColor(float fRed, float fGreen, float fBlue, float fAlpha)                                                               {}

	virtual u32 PopState()                                                                                                                          { return 0; }
	virtual u32 PushState()                                                                                                                         { return 0; }
};

#endif  // #ifndef _NULL_AI_DEBUG_RENDERER_H_
