// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __UPDATEASPECTDATACONTEXT_H__
#define __UPDATEASPECTDATACONTEXT_H__

#pragma once

#include <drx3D/Network/INetContextListener.h>

class CNetContext;

// most of the implementation of CNetContextState::UpdateAspectData()
class CUpdateAspectDataContext
{
public:
	CUpdateAspectDataContext(CNetContext* pNetContext, SContextObject& obj, SContextObjectEx& objx, CTimeValue localPhysicsTime);
	~CUpdateAspectDataContext();

	void              RequestFetchAspects(const NetworkAspectType fetchAspects);
	void              FetchDataFromGame(NetworkAspectID i);

	NetworkAspectType GetFetchAspects() const
	{
		return m_fetchAspects;
	}
	NetworkAspectType GetTakenAspects() const
	{
		return m_takenAspects;
	}

private:
	CNetContext*      m_pNetContext;
	SContextObject&   m_obj;
	SContextObjectEx& m_objx;
	SContextObjectRef m_ref;
	CTimeValue        m_localPhysicsTime;
	NetworkAspectType m_allowedAspects;
	NetworkAspectType m_fetchAspects;
	NetworkAspectType m_takenAspects;
	TMemHdl           m_oldHdls[NumAspects];

	bool TakeChange(NetworkAspectID i, NetworkAspectType aspectBit, CByteOutputStream& stm, CMementoStreamAllocator& streamAllocatorForNewState);
};

#endif
