// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   AIDebugDrawHelpers.h
   $Id$
   Описание: Встомогательные функции для отрисовки некоторых интересных отладочных фигур.

   -------------------------------------------------------------------------
   История:
   - 2006-9-19   : Created by Mikko Mononen

 *********************************************************************/

#ifndef __AIDEBUGDRAWHELPERS_H__
#define __AIDEBUGDRAWHELPERS_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Math/Drx_Vector3.h>

// Draws a circle constrained on XY-plane.
void DebugDrawCircleOutline(IRenderer* pRend, const Vec3& pos, float rad, ColorB col);

// Draws a capsule constrained on XY-plane.
void DebugDrawCapsuleOutline(IRenderer* pRend, const Vec3& pos0, const Vec3& pos1, float rad, ColorB col);

// Draws a wire frame sphere (circle per plane).
void DebugDrawWireSphere(IRenderer* pRend, const Vec3& pos, float rad, ColorB col);

// Draws a wire frame cone.
void DebugDrawWireFOVCone(IRenderer* pRend, const Vec3& pos, const Vec3& dir, float rad, float fov, ColorB col);

// Draws an arrow constrained on XY-plane along the length-vector.
void DebugDrawArrow(IRenderer* pRend, const Vec3& pos, const Vec3& length, float width, ColorB col);

// Draws a circle constrained on XY-plane with hem.
void DebugDrawRangeCircle(IRenderer* pRend, const Vec3& pos, float rad, float width,
                          ColorB colFill, ColorB colOutline, bool drawOutline);

// Draws an arc constrained on XY-plane with hem.
void DebugDrawRangeArc(IRenderer* pRend, const Vec3& pos, const Vec3& dir, float angle, float rad, float width,
                       ColorB colFill, ColorB colOutline, bool drawOutline);

// Draws a box constrained on XY-plane with hem.
void DebugDrawRangeBox(IRenderer* pRend, const Vec3& pos, const Vec3& dir, float sizex, float sizey, float width,
                       ColorB colFill, ColorB colOutline, bool drawOutline);

// Draws a polygon with a hem constrained on XY-plane.
template<typename containerType>
void DebugDrawRangePolygon(IRenderer* pRend, const containerType& polygon, float width,
                           ColorB colFill, ColorB colOutline, bool drawOutline);

// Draw
void DebugDrawLabel(IRenderer* pRenderer, i32 col, i32 row, tukk szText, float* pColor);

// Draw agent circles.
void DebugDrawCircles(IRenderer* pRenderer, const Vec3& pos,
                      float minRadius, float maxRadius, i32 numRings,
                      const ColorB& insideCol, const ColorB& outsideCol);

// Debug oriented 2d ellipse
void DebugDrawEllipseOutline(IRenderer* pRend, const Vec3& pos, float radx, float rady, float orientation, ColorB col);

/// Returns the z value that should be used for drawing the point, depending on if it should be relative to terrain.
/// If relative to the terrain then if the m_dbgDrawOffsetZ is <= 0, -m_dbgDrawOffsetZ is returned
float GetDebugDrawZ(const Vec3& pt, bool useTerrain);

float GetDebugDrawOffset();

// Returns camera position.
Vec3 GetDebugCameraPos();

#endif
