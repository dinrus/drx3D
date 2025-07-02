// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PHYSICSSYNC_H__
#define __PHYSICSSYNC_H__

#pragma once

#include <drx3D/CoreX/Containers/MiniQueue.h>

struct IPhysicalWorld;
class CGameChannel;
struct IDebugHistory;
struct IDebugHistoryUpr;

class CPhysicsSync
{
public:
	CPhysicsSync(CGameChannel* pParent);
	~CPhysicsSync();

	bool OnPacketHeader(CTimeValue);
	bool       OnPacketFooter();

	CTimeValue GetTime();
	void SetTime(CTimeValue);

	void UpdatedEntity(EntityId id)
	{
		m_updatedEntities.push_back(id);
	}

	bool IgnoreSnapshot() const { return false && m_ignoreSnapshot; }
	bool NeedToCatchup() const  { return m_catchup; }

private:
	void OutputDebug(float deltaPhys, float deltaPing, float averagePing, float ping, float stepForward, float deltaTimestamp);

	CGameChannel*    m_pParent;
	IPhysicalWorld*  m_pWorld;

	static i32k MAX_PING_SAMPLES = 128;
	struct SPastPing
	{
		CTimeValue when;
		float      value;
	};
	typedef MiniQueue<SPastPing, MAX_PING_SAMPLES> PingQueue;
	PingQueue m_pastPings;

	class CDebugTimeline;
	std::unique_ptr<CDebugTimeline> m_pDBTL;

	CTimeValue                      m_physPrevRemoteTime;
	CTimeValue                      m_pingEstimate;
	CTimeValue                      m_physEstimatedLocalLaggedTime;
	CTimeValue                      m_epochWhen;
	i32                             m_physEpochTimestamp;

	bool                            m_ignoreSnapshot;
	bool                            m_catchup;
	IDebugHistoryUpr*           m_pDebugHistory;
	std::vector<IDebugHistory*>     m_vpHistories;

	std::vector<EntityId>           m_updatedEntities;
	std::vector<IPhysicalEntity*>   m_clearEntities;
};

#endif
