// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CONC_QUEUE_INCLUDED__
#define __CONC_QUEUE_INCLUDED__

#include "concqueue-mpmc-bounded.hpp"
#include "concqueue-mpsc.hpp"
#include "concqueue-spsc.hpp"
#include "concqueue-spsc-bounded.hpp"

// Recover assert macro meaning.
#include <drx3D/CoreX/Assert/DrxAssert.h>

#define BoundMPMC   concqueue::mpmc_bounded_queue_t
#define BoundSPSC   concqueue::spsc_bounded_queue_t
#define UnboundMPSC concqueue::mpsc_queue_t
#define UnboundSPSC concqueue::spsc_queue_t

template<template<typename> class queue, typename subject>
class ConcQueue : public queue<subject>
{

};

#endif
