// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Created by: Michael Kopietz
// Modified: -
//
//---------------------------------------------------------------------------

#ifndef __CDRXPOOLALLOC__
#define __CDRXPOOLALLOC__

#if defined(POOLALLOCTESTSUIT)
//cheat just for unit testing on windows
	#include <drx3D/CoreX/BaseTypes.h>
	#define ILINE inline
#endif

#if DRX_PLATFORM_POSIX
	#define CPA_ALLOC memalign
	#define CPA_FREE  free
#else
	#define CPA_ALLOC _aligned_malloc
	#define CPA_FREE  _aligned_free
#endif
#define CPA_ASSERT  assert
#define CPA_ASSERT_STATIC(X) { u8 assertdata[(X) ? 0 : 1]; }
#define CPA_BREAK   __debugbreak()

#if DRX_PLATFORM_APPLE
ILINE uk memalign(size_t nAlign, size_t nSize)
{
	uk pBlock(NULL);
	posix_memalign(&pBlock, nSize, nAlign);
	return pBlock;
}
#endif // DRX_PLATFORM_APPLE

#include "List.h"
#include "Memory.h"
#include "Container.h"
#include "Allocator.h"
#include "Defrag.h"
#include "STLWrapper.h"
#include "Inspector.h"
#include "Fallback.h"
#if !defined(POOLALLOCTESTSUIT)
	#include "ThreadSafe.h"
#endif

#undef CPA_ASSERT
#undef CPA_ASSERT_STATIC
#undef CPA_BREAK

#endif
