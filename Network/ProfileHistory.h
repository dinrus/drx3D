// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROFILEHISTORY_H__
#define __PROFILEHISTORY_H__

#pragma once

#include <drx3D/Network/History.h>

class CProfileHistory : public CHistory
{
public:
	CProfileHistory(CContextView*);

	virtual bool ReadCurrentValue(const SReceiveContext& ctx, bool commit);
	virtual void HandleEvent(const SHistoricalEvent& event);

private:
	virtual bool NeedToSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item);
	virtual bool CanSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item);
	virtual bool SendCurrentValue(const SSendContext& ctx, CSyncContext* pSyncContext, u32& newValue);
};

#endif
