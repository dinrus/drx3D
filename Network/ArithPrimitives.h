// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  primitive curve integrators used for delta state
               compression on an arithmetic stream
   -------------------------------------------------------------------------
   История:
   - 02/09/2004   12:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __ARITHPRIMITIVES_H__
#define __ARITHPRIMITIVES_H__

#pragma once

#include <drx3D/Network/Config.h>

#if USE_MEMENTO_PREDICTORS

	#include <drx3D/Network/CommStream.h>

void SquarePulseProbabilityWrite(
  CCommOutputStream& stm,
  u32 nValue,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  bool* bHit = NULL
  );
u32 SquarePulseProbabilityRead(
  CCommInputStream& stm,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  bool* bHit = NULL
  );
float SquarePulseProbabilityEstimate(
  u32 nValue,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange
  );

void HalfSquareProbabilityWrite(
  CCommOutputStream& stm,
  u32 nValue,
  u32 nDropPoint,
  u32 nHeight,
  u32 nRange,
  u8 nInRangePercentage,
  u8 recurseLimit = 5
  );
float HalfSquareProbabilityEstimate(
  u32 nValue,
  u32 nDropPoint,
  u32 nHeight,
  u32 nRange,
  u8 nInRangePercentage,
  u8 recurseLimit = 5
  );
bool HalfSquareProbabilityRead(
  u32& value,
  CCommInputStream& stm,
  u32 nDropPoint,
  u32 nHeight,
  u32 nRange,
  u8 nInRangePercentage,
  u8 recurseLimit = 5
  );

void SquarePulseProbabilityWriteImproved(
  CCommOutputStream& stm,
  u32 nValue,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  u8 nInRangePrecentage,
  u8 nMaxBits,
  bool* bHit = NULL
  );
bool SquarePulseProbabilityReadImproved(
  u32& value,
  CCommInputStream& stm,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  u8 nInRangePrecentage,
  u8 nMaxBits,
  bool* bHit = NULL
  );
float SquarePulseProbabilityEstimateImproved(
  u32 nValue,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  u8 nInRangePrecentage,
  u8 nMaxBits
  );

void ThreeSquarePulseProbabilityWrite(
  CCommOutputStream& stm,
  u32 nValue,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  bool* bHit = NULL
  );
u32 ThreeSquarePulseProbabilityRead(
  CCommInputStream& stm,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange,
  bool* bHit = NULL
  );
float ThreeSquarePulseProbabilityEstimate(
  u32 nValue,
  u32 nLeft,
  u32 nRight,
  u32 nHeight,
  u32 nRange
  );

void CliffProbabilityWrite(
  CCommOutputStream& stm,
  i32 nValue,
  i32 nLowPoint,
  i32 nHighPoint,
  u16 nHeight,
  i32 nRange
  );
float CliffProbabilityEstimate(
  i32 nValue,
  i32 nLowPoint,
  i32 nHighPoint,
  u16 nHeight,
  i32 nRange
  );
i32 CliffProbabilityRead(
  CCommOutputStream& stm,
  i32 nLowPoint,
  i32 nHighPoint,
  u16 nHeight,
  i32 nRange
  );

#endif

#endif
