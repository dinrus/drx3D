// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BREAKAGEPLAYBACK_H__
#define __BREAKAGEPLAYBACK_H__

#pragma once

#include <drx3D/Network/ClientContextView.h>

class CBreakagePlayback : public INetBreakagePlayback
{
public:
	CBreakagePlayback(CClientContextView* pView, SNetClientBreakDescriptionPtr pBreakOps);
	~CBreakagePlayback();

	virtual void     SpawnedEntity(i32 idx, EntityId id);
	virtual EntityId GetEntityIdForIndex(i32 idx);

private:
	_smart_ptr<CClientContextView> m_pView;
	SNetClientBreakDescriptionPtr  m_pBreakOps;
	DynArray<EntityId>             m_gotBreakOps;

	size_t                         m_ofs;
};

#endif
