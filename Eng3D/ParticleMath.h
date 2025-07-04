// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/ParticleCommon.h>

namespace pfx2
{

using namespace drxmath;
// 4x vector types of standard types
#ifdef DRX_PFX2_USE_SSE
typedef f32v4  floatv;
typedef u32v4  uint32v;
typedef i32v4  int32v;
typedef u32 uint8v;
typedef u32v4  UColv;
#else
typedef float  floatv;
typedef u32 uint32v;
typedef i32  int32v;
typedef u8  uint8v;
typedef UCol   UColv;
#endif

// 4x vector types of custom types
typedef uint32v           TParticleIdv;
typedef uint32v           TEmitterIdv;
typedef Vec2_tpl<floatv>  Vec2v;
typedef Vec3_tpl<floatv>  Vec3v;
typedef Vec4_tpl<floatv>  Vec4v;
typedef Quat_tpl<floatv>  Quatv;
typedef Plane_tpl<floatv> Planev;

#ifdef DRX_PFX2_USE_SSE
struct ColorFv
{
	ColorFv() {}
	ColorFv(floatv _r, floatv _g, floatv _b)
		: r(_r), g(_g), b(_b) {}
	floatv r, g, b;
};
#else
typedef ColorF ColorFv;
#endif

///////////////////////////////////////////////////////////////////////////
// Vector/scalar conversions

floatv  ToFloatv(float v);
uint32v ToUint32v(u32 v);
floatv  ToFloatv(int32v v);
floatv  ToFloatv(uint32v v);
uint32v ToUint32v(floatv v);
Vec3v   ToVec3v(Vec3 v);
Vec4v   ToVec4v(Vec4 v);
Planev  ToPlanev(Plane v);

u8  FloatToUFrac8Saturate(float v);

///////////////////////////////////////////////////////////////////////////
// General operator and function equivalents
template<typename T, typename U> T ILINE Add(const T& a, const U& b)              { return a + b; }
template<typename T, typename U> T ILINE Sub(const T& a, const U& b)              { return a - b; }
template<typename T, typename U> T ILINE Mul(const T& a, const U& b)              { return a * b; }

template<typename T, typename U> T ILINE MAdd(const T& a, const U& b, const T& c) { return a * b + c; }

template<typename T> T& SetMax(T& var, T const& value)                            { return var = std::max(var, value); };
template<typename T> T& SetMin(T& var, T const& value)                            { return var = std::min(var, value); };

template<typename T> T FiniteOr(T val, T alt)                                     { return std::isfinite(val) ? val : alt; }

///////////////////////////////////////////////////////////////////////////
// Vector functions

Vec3v Add(const Vec3v& a, const floatv b);
Vec3v Sub(const Vec3v& a, const floatv b);

Vec3  HMin(const Vec3v& v0);
Vec3  HMax(const Vec3v& v0);

// Efficient vector MAdd

template<typename T> Vec3_tpl<T> MAdd(const Vec3_tpl<T>& a, const Vec3_tpl<T>& b, const Vec3_tpl<T>& c);
template<typename T> Vec3_tpl<T> MAdd(const Vec3_tpl<T>& a, T b, const Vec3_tpl<T>& c);

// Quaternion functions

Quatv AddAngularVelocity(Quatv initial, Vec3v angularVel, floatv deltaTime);

// Color

ColorF  ToColorF(UCol color);
ColorFv ToColorFv(UColv color);
ColorFv ToColorFv(ColorF color);
UCol    ColorFToUCol(const ColorF& color);
UColv   ColorFvToUColv(const ColorFv& color);
UColv   ToUColv(UCol color);

#ifdef DRX_PFX2_USE_SSE
ColorFv operator+(const ColorFv& a, const ColorFv& b);
ColorFv operator*(const ColorFv& a, const ColorFv& b);
ColorFv operator*(const ColorFv& a, floatv b);
#endif

// Return the correct update time for a particle
template<typename T>
T DeltaTime(T frameTime, T normAge, T lifeTime);

// Return the start-time of a particle in the previous frame
template<typename T>
T StartTime(T curTime, T frameTime, T absAge);


// non vectorized

void RotateAxes(Vec3* v0, Vec3* v1, const float angle);
Vec3 PolarCoordToVec3(float azimuth, float altitude);

///////////////////////////////////////////////////////////////////////////
template<typename F>
struct Slope
{
	F scale;
	F start;

	Slope(F scale = 1, F start = 0) : scale(scale), start(start) {}
	template<typename F2> Slope(const Slope<F2>& o) : scale(convert<F>(o.scale)), start(convert<F>(o.start)) {}

	F operator()(F val) const { return MAdd(val, scale, start); }
};

}

// Drx_Math_SSE extension
template<> struct SIMD_traits<UCol>
{
	using scalar_t  = UCol;
	using vector4_t = pfx2::UColv;
};


#include "ParticleMathImpl.h"
