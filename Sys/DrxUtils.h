// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <cstddef> // size_t

namespace Detail
{
template<typename T, size_t size>
char (&ArrayCountHelper(T(&)[size]))[size];
}

#define DRX_ARRAY_COUNT(arr) (i32) sizeof( ::Detail::ArrayCountHelper(arr))
