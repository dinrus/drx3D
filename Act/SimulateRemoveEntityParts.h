// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network breakability: generic 'remove parts' events - try not to use
   -------------------------------------------------------------------------
   История:
   - 22/01/2007   10:34 : Created by Craig Tiller
*************************************************************************/
#ifndef __SIMULATEREMOVEENTITYPARTS_H__
#define __SIMULATEREMOVEENTITYPARTS_H__

#pragma once

#if !NET_USE_SIMPLE_BREAKAGE

	#include "ObjectSelector.h"

struct SSimulateRemoveEntityParts
{
	CObjectSelector ent;
	u32    partIds[4];
	float           massOrg;

	void            SerializeWith(TSerialize ser);
};

struct SSimulateRemoveEntityPartsMessage : public SSimulateRemoveEntityParts
{
	SSimulateRemoveEntityPartsMessage() : breakId(-1) {}
	SSimulateRemoveEntityPartsMessage(const SSimulateRemoveEntityParts& p, i32 bid) : SSimulateRemoveEntityParts(p), breakId(bid) {}
	i32 breakId;

	void SerializeWith(TSerialize ser);
};

struct SSimulateRemoveEntityPartsInfo : public IBreakDescriptionInfo, public SSimulateRemoveEntityParts
{
	void GetAffectedRegion(AABB& aabb);
	void AddSendables(INetSendableSink* pSink, i32 brkId);
};

#endif // !NET_USE_SIMPLE_BREAKAGE

#endif
