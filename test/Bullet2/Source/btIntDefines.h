
#ifndef BT_INT_DEFINES_H
#define BT_INT_DEFINES_H

#ifdef __GNUC__
#include <stdint.h>
#elif defined(_MSC_VER)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
typedef i32 int32_t;
typedef z64 int64_t;
typedef u32 uint32_t;
typedef zu64 uint64_t;
#endif

#endif  //BT_INT_DEFINES_H
