// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// SWIG не может обработать static_assert, поэтому он скрыт под макросом.
#if SWIG
#define static_assert(...)
#endif

static_assert(sizeof(char) == 1, "Неверный размер типа!");
static_assert(sizeof(float) == 4, "Неверный размер типа!");
static_assert(sizeof(i32) >= 4, "Неверный размер типа!");

typedef u8  uchar;
typedef signed char    schar;

typedef unsigned short ushort;
typedef signed short   sshort;

#if !defined(CLANG_FIX_UINT_REDEF)
typedef u32       uint;
#endif
typedef i32         sint;

typedef u64      ulong;
typedef i64        slong;

typedef zu64 ulonglong;
typedef z64   slonglong;

static_assert(sizeof(uchar) == sizeof(schar), "Неверный размер типа!");
static_assert(sizeof(ushort) == sizeof(sshort), "Неверный размер типа!");
static_assert(sizeof(uint) == sizeof(sint), "Неверный размер типа!");
static_assert(sizeof(ulong) == sizeof(slong), "Неверный размер типа!");
static_assert(sizeof(ulonglong) == sizeof(slonglong), "Неверный размер типа!");

static_assert(sizeof(uchar) <= sizeof(ushort), "Неверный размер типа!");
static_assert(sizeof(ushort) <= sizeof(uint), "Неверный размер типа!");
static_assert(sizeof(uint) <= sizeof(ulong), "Неверный размер типа!");
static_assert(sizeof(ulong) <= sizeof(ulonglong), "Неверный размер типа!");

typedef schar int8;
typedef schar sint8;
typedef uchar u8;
static_assert(sizeof(u8) == 1, "Неверный размер типа!");
static_assert(sizeof(sint8) == 1, "Неверный размер типа!");

typedef sshort i16;
typedef sshort sint16;
typedef ushort u16;
static_assert(sizeof(u16) == 2, "Неверный размер типа!");
static_assert(sizeof(sint16) == 2, "Неверный размер типа!");

typedef sint i32;
typedef sint sint32;
typedef uint u32;
static_assert(sizeof(u32) == 4, "Неверный размер типа!");
static_assert(sizeof(sint32) == 4, "Неверный размер типа!");

typedef slonglong int64;
typedef slonglong sint64;
typedef ulonglong uint64;
static_assert(sizeof(uint64) == 8, "Неверный размер типа!");
static_assert(sizeof(sint64) == 8, "Неверный размер типа!");

typedef float  f32;
typedef double f64;
static_assert(sizeof(f32) == 4, "Неверный размер типа!");
static_assert(sizeof(f64) == 8, "Неверный размер типа!");
