///@file Конфигурация Библиотеки Inverse Dynamics,
///	  касающаяся выбора библиотеки линейной алгебры и подлежащего скалярного типа.
#ifndef IDCONFIG_H_
#define IDCONFIG_H_

// If true, enable jacobian calculations.
// This adds a 3xN matrix to every body, + 2 3-Vectors.
// so it is not advised for large systems if it is not absolutely necessary.
// Also, this is not required for standard inverse dynamics calculations.
// Will only work with vector math libraries that support 3xN matrices.
#define DRX3D_ID_WITH_JACOBIANS

// If we have a custom configuration, compile without using other parts of bullet.
#ifdef DRX3D_CUSTOM_INVERSE_DYNAMICS_CONFIG_H
#include <cmath>
#define DRX3D_ID_WO_BULLET
#define DRX3D_ID_SQRT(x) std::sqrt(x)
#define DRX3D_ID_FABS(x) std::fabs(x)
#define DRX3D_ID_COS(x) std::cos(x)
#define DRX3D_ID_SIN(x) std::sin(x)
#define DRX3D_ID_ATAN2(x, y) std::atan2(x, y)
#define DRX3D_ID_POW(x, y) std::pow(x, y)
#define DRX3D_ID_SNPRINTF snprintf
#define DRX3D_ID_PI M_PI
#define DRX3D_ID_USE_DOUBLE_PRECISION
#else
#define DRX3D_ID_SQRT(x) Sqrt(x)
#define DRX3D_ID_FABS(x) Fabs(x)
#define DRX3D_ID_COS(x) Cos(x)
#define DRX3D_ID_SIN(x) Sin(x)
#define DRX3D_ID_ATAN2(x, y) Atan2(x, y)
#define DRX3D_ID_POW(x, y) Pow(x, y)
#define DRX3D_ID_PI SIMD_PI
#ifdef _WIN32
#define DRX3D_ID_SNPRINTF _snprintf
#else
#define DRX3D_ID_SNPRINTF snprintf
#endif  //
#endif
// error messages
#include <drx3D/Physics/Dynamics/Inverse/IDErrorMessages.h>

#ifdef DRX3D_CUSTOM_INVERSE_DYNAMICS_CONFIG_H
/*
#include "IDConfigEigen.h>
#include "IDConfigBuiltin.h>
*/
#define INVDYN_INCLUDE_HELPER_2(x) #x
#define INVDYN_INCLUDE_HELPER(x) INVDYN_INCLUDE_HELPER_2(x)
#include INVDYN_INCLUDE_HELPER(DRX3D_CUSTOM_INVERSE_DYNAMICS_CONFIG_H)
#ifndef drx3d_inverse
#error "custom inverse dynamics config, but no custom namespace defined"
#endif

#define DRX3D_ID_MAX(a, b) std::max(a, b)
#define DRX3D_ID_MIN(a, b) std::min(a, b)

#else
#define drx3d_inverse drx3d_inverseB3
// Use default configuration with bullet's types
// Use the same scalar type as rest of bullet library
#include <drx3D/Maths/Linear/Scalar.h>
typedef Scalar idScalar;
#include <drx3D/Maths/Linear/MinMax.h>
#define DRX3D_ID_MAX(a, b) d3Max(a, b)
#define DRX3D_ID_MIN(a, b) d3Min(a, b)

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define DRX3D_ID_USE_DOUBLE_PRECISION
#endif

#ifndef DRX3D_USE_INVERSE_DYNAMICS_WITH_BULLET2

// use bullet types for arrays and array indices
#include <drx3D/Common/b3AlignedObjectArray.h>
// this is to make it work with C++2003, otherwise we could do this:
// template <typename T>
// using idArray = b3AlignedObjectArray<T>;
template <typename T>
struct idArray
{
	typedef b3AlignedObjectArray<T> type;
};
typedef i32 idArrayIdx;
#define ID_DECLARE_ALIGNED_ALLOCATOR() D3_DECLARE_ALIGNED_ALLOCATOR()

#else  // DRX3D_USE_INVERSE_DYNAMICS_WITH_BULLET2

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
template <typename T>
struct idArray
{
	typedef AlignedObjectArray<T> type;
};
typedef i32 idArrayIdx;
#define ID_DECLARE_ALIGNED_ALLOCATOR() DRX3D_DECLARE_ALIGNED_ALLOCATOR()

#endif
 // DRX3D_USE_INVERSE_DYNAMICS_WITH_BULLET2

// use bullet's allocator functions
#define idMalloc AllocFunc
#define idFree FreeFunc

#define ID_LINEAR_MATH_USE_BULLET
#include <drx3D/Physics/Dynamics/Inverse/details/IDLinearMathInterface.h>
#endif
#endif
