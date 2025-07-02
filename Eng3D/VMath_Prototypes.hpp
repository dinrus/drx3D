// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Version:     v1.00
//  Created:     Michael Kopietz
//  Описание: unified vector math lib
// -------------------------------------------------------------------------
//  История:		- created 1999  for Katmai and K3
//							-	...
//							-	integrated into drxengine
//
////////////////////////////////////////////////////////////////////////////
#ifndef __D_VMATH_H__
#define __D_VMATH_H__

ILINE vec4   Vec4(float x);
ILINE vec4   Vec4(float x, float y, float z, float w);
ILINE vec4   Vec4(u32 x, u32 y, u32 z, u32 w);
ILINE i32  Vec4int32(vec4 V, u32 Idx);
ILINE vec4   Vec4Zero();
ILINE vec4   Vec4One();
ILINE vec4   Vec4Four();
ILINE vec4   Vec4ZeroOneTwoThree();
ILINE vec4   Vec4FFFFFFFF();
ILINE vec4   Vec4Epsilon();
ILINE vec4   Add(vec4 V0, vec4 V1);
ILINE vec4   Sub(vec4 V0, vec4 V1);
ILINE vec4   Mul(vec4 V0, vec4 V1);
ILINE vec4   Div(vec4 V0, vec4 V1);
ILINE vec4   RcpFAST(vec4 V);
ILINE vec4   DivFAST(vec4 V0, vec4 V1);
ILINE vec4   Rcp(vec4 V);
ILINE vec4   Madd(vec4 V0, vec4 V1, vec4 V2);
ILINE vec4   Msub(vec4 V0, vec4 V1, vec4 V2);
ILINE vec4   Min(vec4 V0, vec4 V1);
ILINE vec4   Max(vec4 V0, vec4 V1);
ILINE vec4   floatToint32(vec4 V);
ILINE vec4   int32Tofloat(vec4 V);
ILINE vec4   CmpLE(vec4 V0, vec4 V1);
ILINE u32 SignMask(vec4 V);
ILINE vec4   And(vec4 V0, vec4 V1);
ILINE vec4   AndNot(vec4 V0, vec4 V1);
ILINE vec4   Or(vec4 V0, vec4 V1);
ILINE vec4   Xor(vec4 V0, vec4 V1);
ILINE vec4   Select(vec4 V0, vec4 V1, vec4 M);
ILINE vec4   SelectSign(vec4 V0, vec4 V1, vec4 M);

#endif
