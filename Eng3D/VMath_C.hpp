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
#pragma once

namespace NVMath
{

union vec4
{
	struct
	{
		float m_Xf;
		float m_Yf;
		float m_Zf;
		float m_Wf;
	};
	struct
	{
		u32 m_Xu;
		u32 m_Yu;
		u32 m_Zu;
		u32 m_Wu;
	};
	struct
	{
		i32 m_Xs;
		i32 m_Ys;
		i32 m_Zs;
		i32 m_Ws;
	};
	float  m_f[4];
	i32  m_s[4];
	u32 m_u[4];
};

#include "VMath_Prototypes.hpp"

#define SWIZZLEMASK5(X, Y, Z, W) ((X) | (Y << 2) | (Z << 4) | (W << 6))
#define SWIZZLEMASK4(N, X, Y, Z) N ## x = SWIZZLEMASK5(X, Y, Z, 0), \
  N ## y = SWIZZLEMASK5(X, Y, Z, 1),                                \
  N ## z = SWIZZLEMASK5(X, Y, Z, 2),                                \
  N ## w = SWIZZLEMASK5(X, Y, Z, 3),
#define SWIZZLEMASK3(N, X, Y)    SWIZZLEMASK4(N ## x, X, Y, 0) \
  SWIZZLEMASK4(N ## y, X, Y, 1)                                \
  SWIZZLEMASK4(N ## z, X, Y, 2)                                \
  SWIZZLEMASK4(N ## w, X, Y, 3)
#define SWIZZLEMASK2(N, X)       SWIZZLEMASK3(N ## x, X, 0) \
  SWIZZLEMASK3(N ## y, X, 1)                                \
  SWIZZLEMASK3(N ## z, X, 2)                                \
  SWIZZLEMASK3(N ## w, X, 3)
#define SWIZZLEMASK1 SWIZZLEMASK2(x, 0) \
  SWIZZLEMASK2(y, 1)                    \
  SWIZZLEMASK2(z, 2)                    \
  SWIZZLEMASK2(w, 3)

enum ESwizzleMask
{
	SWIZZLEMASK1
};
enum ECacheLvl
{
	ECL_LVL1,
	ECL_LVL2,
	ECL_LVL3,
};
#define BitX 1
#define BitY 2
#define BitZ 4
#define BitW 8

ILINE vec4 Vec4(float x, float y, float z, float w)
{
	vec4 Ret;
	Ret.m_Xf = x;
	Ret.m_Yf = y;
	Ret.m_Zf = z;
	Ret.m_Wf = w;
	return Ret;
}
ILINE vec4 Vec4(u32 x, u32 y, u32 z, u32 w)
{
	vec4 Ret;
	Ret.m_Xu = x;
	Ret.m_Yu = y;
	Ret.m_Zu = z;
	Ret.m_Wu = w;
	return Ret;
}
ILINE vec4 Vec4(float x)
{
	return Vec4(x, x, x, x);
}
ILINE float Vec4float(vec4 V, u32 Idx)
{
	return V.m_f[Idx];
}
template<i32 Idx>
ILINE float Vec4float(vec4 V)
{
	return Vec4float(V, Idx);
}
ILINE i32 Vec4int32(vec4 V, u32 Idx)
{
	return V.m_s[Idx];
}
template<i32 Idx>
ILINE i32 Vec4int32(vec4 V)
{
	return Vec4int32(V, Idx);
}
ILINE vec4 Vec4Zero()
{
	return Vec4(0.f);
}

ILINE vec4 Vec4One()
{
	return Vec4(1.f, 1.f, 1.f, 1.f);
}
ILINE vec4 Vec4Four()
{
	return Vec4(4.f, 4.f, 4.f, 4.f);
}
ILINE vec4 Vec4ZeroOneTwoThree()
{
	return Vec4(0.f, 1.f, 2.f, 3.f);
}
ILINE vec4 Vec4FFFFFFFF()
{
	return Vec4(~0u, ~0u, ~0u, ~0u);
}
ILINE vec4 Vec4Epsilon()
{
	return Vec4(FLT_EPSILON);
}
template<ECacheLvl L>
ILINE void Prefetch(ukk pData)
{
}
template<ESwizzleMask M>
ILINE vec4 Shuffle(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_u[M & 3], V0.m_u[(M >> 2) & 3], V1.m_u[(M >> 4) & 3], V1.m_u[(M >> 6) & 3]);
}
template<ESwizzleMask M>
ILINE vec4 Swizzle(vec4 V)
{
	return Shuffle<M>(V, V);
}
template<i32 INDEX>
ILINE vec4 Splat(vec4 V)
{
	DRX_ASSERT_MESSAGE(0, "Should not be reached!");
	return Vec4FFFFFFFF();
}
template<>
ILINE vec4 Splat<0>(vec4 V)
{
	return Shuffle<xxxx>(V, V);
}
template<>
ILINE vec4 Splat<1>(vec4 V)
{
	return Shuffle<yyyy>(V, V);
}
template<>
ILINE vec4 Splat<2>(vec4 V)
{
	return Shuffle<zzzz>(V, V);
}
template<>
ILINE vec4 Splat<3>(vec4 V)
{
	return Shuffle<wwww>(V, V);
}
ILINE vec4 Add(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf + V1.m_Xf,
	            V0.m_Yf + V1.m_Yf,
	            V0.m_Zf + V1.m_Zf,
	            V0.m_Wf + V1.m_Wf);
}
ILINE vec4 Sub(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf - V1.m_Xf,
	            V0.m_Yf - V1.m_Yf,
	            V0.m_Zf - V1.m_Zf,
	            V0.m_Wf - V1.m_Wf);
}
ILINE vec4 Mul(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf * V1.m_Xf,
	            V0.m_Yf * V1.m_Yf,
	            V0.m_Zf * V1.m_Zf,
	            V0.m_Wf * V1.m_Wf);
}
ILINE vec4 Div(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf / V1.m_Xf,
	            V0.m_Yf / V1.m_Yf,
	            V0.m_Zf / V1.m_Zf,
	            V0.m_Wf / V1.m_Wf);
}
ILINE vec4 RcpFAST(vec4 V)
{
	return Div(Vec4One(), V);
}
ILINE vec4 DivFAST(vec4 V0, vec4 V1)
{
	return Div(V0, V1);
}
ILINE vec4 Rcp(vec4 V)
{
	return Div(Vec4One(), V);
}
ILINE vec4 Madd(vec4 V0, vec4 V1, vec4 V2)
{
	return Add(V2, Mul(V0, V1));
}
ILINE vec4 Msub(vec4 V0, vec4 V1, vec4 V2)
{
	return Sub(Mul(V0, V1), V2);
}
ILINE vec4 Min(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf < V1.m_Xf ? V0.m_Xf : V1.m_Xf,
	            V0.m_Yf < V1.m_Yf ? V0.m_Yf : V1.m_Yf,
	            V0.m_Zf < V1.m_Zf ? V0.m_Zf : V1.m_Zf,
	            V0.m_Wf < V1.m_Wf ? V0.m_Wf : V1.m_Wf);
}
ILINE vec4 Max(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf > V1.m_Xf ? V0.m_Xf : V1.m_Xf,
	            V0.m_Yf > V1.m_Yf ? V0.m_Yf : V1.m_Yf,
	            V0.m_Zf > V1.m_Zf ? V0.m_Zf : V1.m_Zf,
	            V0.m_Wf > V1.m_Wf ? V0.m_Wf : V1.m_Wf);
}
ILINE vec4 floatToint32(vec4 V)
{
	return Vec4(static_cast<u32>(static_cast<i32>(V.m_Xf)),
	            static_cast<u32>(static_cast<i32>(V.m_Yf)),
	            static_cast<u32>(static_cast<i32>(V.m_Zf)),
	            static_cast<u32>(static_cast<i32>(V.m_Wf)));
}
ILINE vec4 int32Tofloat(vec4 V)
{
	return Vec4(static_cast<float>(V.m_Xs),
	            static_cast<float>(V.m_Ys),
	            static_cast<float>(V.m_Zs),
	            static_cast<float>(V.m_Ws));
}
ILINE vec4 CmpLE(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xf <= V1.m_Xf ? ~0u : 0u,
	            V0.m_Yf <= V1.m_Yf ? ~0u : 0u,
	            V0.m_Zf <= V1.m_Zf ? ~0u : 0u,
	            V0.m_Wf <= V1.m_Wf ? ~0u : 0u);
}
ILINE u32 SignMask(vec4 V)
{
	return (V.m_Xu >> 31) | ((V.m_Yu >> 31) << 1) | ((V.m_Zu >> 31) << 2) | ((V.m_Wu >> 31) << 3);
}
ILINE vec4 And(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xu & V1.m_Xu,
	            V0.m_Yu & V1.m_Yu,
	            V0.m_Zu & V1.m_Zu,
	            V0.m_Wu & V1.m_Wu);
}
ILINE vec4 AndNot(vec4 V0, vec4 V1)
{
	return Vec4((~V0.m_Xu) & V1.m_Xu,
	            (~V0.m_Yu) & V1.m_Yu,
	            (~V0.m_Zu) & V1.m_Zu,
	            (~V0.m_Wu) & V1.m_Wu);
}
ILINE vec4 Or(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xu | V1.m_Xu,
	            V0.m_Yu | V1.m_Yu,
	            V0.m_Zu | V1.m_Zu,
	            V0.m_Wu | V1.m_Wu);
}
ILINE vec4 Xor(vec4 V0, vec4 V1)
{
	return Vec4(V0.m_Xu ^ V1.m_Xu,
	            V0.m_Yu ^ V1.m_Yu,
	            V0.m_Zu ^ V1.m_Zu,
	            V0.m_Wu ^ V1.m_Wu);
}
ILINE vec4 Select(vec4 V0, vec4 V1, vec4 M)
{
	return SelectSign(V0, V1, M);
}
ILINE vec4 ShiftAR(vec4 V, u32 Count)
{
	return Vec4(static_cast<u32>(V.m_Xs >> Count),
	            static_cast<u32>(V.m_Ys >> Count),
	            static_cast<u32>(V.m_Zs >> Count),
	            static_cast<u32>(V.m_Ws >> Count));
}
ILINE vec4 SelectSign(vec4 V0, vec4 V1, vec4 M)
{
	M = ShiftAR(M, 31);
	return Or(AndNot(M, V0), And(M, V1));
}
template<i32 M>
ILINE vec4 SelectStatic(vec4 V0, vec4 V1)
{
	const vec4 mask = NVMath::Vec4(M & 0x1 ? ~0x0u : 0x0u, M & 0x2 ? ~0x0u : 0x0u, M & 0x4 ? ~0x0u : 0x0u, M & 0x8 ? ~0x0u : 0x0u);
	return Select(V0, V1, mask);
}

ILINE vec4 SelectBits(vec4 V0, vec4 V1, vec4 M)
{
	return Or(AndNot(M, V0), And(M, V1));
}

ILINE vec4 CmpEq(vec4 V0, vec4 V1)
{
	return Vec4(
	  V0.m_Xs == V1.m_Xs ? 0xffffffff : 0x0,
	  V0.m_Ys == V1.m_Ys ? 0xffffffff : 0x0,
	  V0.m_Zs == V1.m_Zs ? 0xffffffff : 0x0,
	  V0.m_Ws == V1.m_Ws ? 0xffffffff : 0x0);
}

ILINE void ExtractByteToFloat(vec4& rVOut0, vec4& rVOut1, vec4& rVOut2, vec4& rVOut3, vec4 VIn)
{
	tukk const VInBytes = (tukk)&VIn;
	vec4 VOutVecs[4];
	i32* VOutInts[4] =
	{
		reinterpret_cast<i32*>(&VOutVecs[0]),
		reinterpret_cast<i32*>(&VOutVecs[1]),
		reinterpret_cast<i32*>(&VOutVecs[2]),
		reinterpret_cast<i32*>(&VOutVecs[3])
	};

	VOutInts[0][0] = VInBytes[0];
	VOutInts[0][1] = VInBytes[1];
	VOutInts[0][2] = VInBytes[2];
	VOutInts[0][3] = VInBytes[3];
	VOutInts[1][0] = VInBytes[4];
	VOutInts[1][1] = VInBytes[5];
	VOutInts[1][2] = VInBytes[6];
	VOutInts[1][3] = VInBytes[7];
	VOutInts[2][0] = VInBytes[8];
	VOutInts[2][1] = VInBytes[9];
	VOutInts[2][2] = VInBytes[10];
	VOutInts[2][3] = VInBytes[11];
	VOutInts[3][0] = VInBytes[12];
	VOutInts[3][1] = VInBytes[13];
	VOutInts[3][2] = VInBytes[14];
	VOutInts[3][3] = VInBytes[15];

	rVOut0 = int32Tofloat(VOutVecs[0]);
	rVOut1 = int32Tofloat(VOutVecs[1]);
	rVOut2 = int32Tofloat(VOutVecs[2]);
	rVOut3 = int32Tofloat(VOutVecs[3]);
}

}
