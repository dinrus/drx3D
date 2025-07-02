// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     06/30/2010 by Morgan K
//  Описание: A debug class to track and output code checkpoint state
// -------------------------------------------------------------------------
//  История: Created by Morgan Kita
//
////////////////////////////////////////////////////////////////////////////

#ifndef __CODE_CHECKPOINT_DEBUG_MGR_H_
#define __CODE_CHECKPOINT_DEBUG_MGR_H_

#pragma once

#include <drx3D/Sys/ICodeCheckpointMgr.h>

#include <drx3D/Game/GameMechanismBase.h>

// Not for release
#ifndef _RELEASE

#define CODECHECKPOINT_DEBUG_ENABLED

#endif

#if defined CODECHECKPOINT_DEBUG_ENABLED
static void CmdCodeCheckPointSearch(IConsoleCmdArgs *pArgs);
#endif

class CCodeCheckpointDebugMgr : public CGameMechanismBase
{
public:

	typedef std::vector< std::pair<string, i32> > RecordNameCountPairs;

	struct CheckpointDebugRecord
	{
		CheckpointDebugRecord(const CCodeCheckpoint* checkPoint, tukk name, i32 checkpointIdx)
			: m_pCheckpoint(checkPoint),m_name(name), m_queried(false),m_lastHitTime(0.0f),
			m_currHitcount(0), m_prevHitcount(0), m_refCount(0), m_checkPointIdx(checkpointIdx)
		{
			if(m_pCheckpoint)
			{
				m_lastHitTime = gEnv->pTimer->GetCurrTime();
				m_currHitcount = checkPoint->HitCount();
			}
			else
			{
				m_queried = true;
			}
		}
		
		/// Function to update the reference count, defaults to an increment of 1.
		void UpdateWatched(i32 count = 1){m_refCount += count; m_queried = (m_refCount != 0);}

		const CCodeCheckpoint*	m_pCheckpoint;	/// The checkpoint if registered otherwise NULL
		string			m_name;		/// The name of the checkpoint (owned if m_pCheckpoint is NULL)

		float	 m_lastHitTime;				/// Last clock time this checkpoint was hit as determined by the snapshots
		u32 m_prevHitcount;					/// Hit count for this node as of the last snapshot
		u32 m_currHitcount;					/// Hit count for this node as of this snapshot

		u32 m_checkPointIdx;				/// Index into checkpoint manager

		bool m_queried;						/// Flag to indicate this checkpoint was specifically requested for at load time
		i32 m_refCount;						/// Count of references that requested this point for observation
	};

	///Predicate functions on records
	static bool RecordHasHits(CheckpointDebugRecord& rec){return rec.m_currHitcount > 0;}

	///Predicate functors on records

	//Checks for a named record and updates its watched information if found
	class RecordMatchAndUpdate
	{
	public:
		RecordMatchAndUpdate(const string& name, i32 count = 1)
			: m_name(name), m_count(count)
		{}

		bool operator() (CheckpointDebugRecord& record) const
		{
			if(record.m_name == m_name)
			{
				record.UpdateWatched(m_count);
				return true;
			}
			return false;
		}

	private:
		string m_name;
		i32 m_count;
	};

	class SearchRecordForString {

	public:

		SearchRecordForString(RecordNameCountPairs& namePairs, const string& substr = ""): m_namePairs(namePairs), m_searchStr(substr){}
		void operator() (const CheckpointDebugRecord& rec) 
		{
			// Is there a registered checkpoint for this record?
			if (rec.m_pCheckpoint)
			{
				//If the search string is empty, or the checkpoint contains the substring anywhere inside of it
				if(m_searchStr == "" || rec.m_name.find(m_searchStr) != string::npos)
					m_namePairs.push_back(std::make_pair(rec.m_name, rec.m_pCheckpoint->HitCount()));
			}
		}

	private:
		RecordNameCountPairs& m_namePairs;
		string m_searchStr;
	};

	CCodeCheckpointDebugMgr();
	virtual ~CCodeCheckpointDebugMgr(){s_pCodeCheckPointDebugUpr=NULL;}

	//Static function to retrieve singleton code checkpoint debug manager
	static CCodeCheckpointDebugMgr* RetrieveCodeCheckpointDebugMgr();

	///Update function will update the internal snapshot and do any debug drawing required
	virtual void Update(float dt);

	///Register name to be marked as a watch point
	void RegisterWatchPoint(const string& name);

	///Unregister name to be marked as a watch point
	void UnregisterWatchPoint(const string& name);

	void SearchCheckpoints(RecordNameCountPairs& outputList, string& searchStr) const;

	void ReadFile(tukk fileName);

	i32 GetLine( char * pBuff, FILE * fp );

	static i32 ft_debug_ccoverage;
	static float ft_debug_ccoverage_rate;
	static i32 ft_debug_ccoverage_maxlines;
	static i32 ft_debug_ccoverage_filter_maxcount;
	static i32 ft_debug_ccoverage_filter_mincount;



private:

	static CCodeCheckpointDebugMgr* s_pCodeCheckPointDebugUpr;

	///CVar variables
	i32 m_debug_ccoverage;
	float m_debug_ccoverage_rate;
	i32 m_debug_ccoverage_maxlines;
	i32 m_debug_ccoverage_filter_maxcount;
	i32 m_debug_ccoverage_filter_mincount;

	void UpdateRecord(const CCodeCheckpoint* pCheckpoint,CheckpointDebugRecord& record );
	void UpdateRecords();

	void DrawDebugInfo();

	typedef std::list<CheckpointDebugRecord> TCheckpointDebugList;
	typedef std::vector<CheckpointDebugRecord> TCheckpointDebugVector;

	typedef std::pair<string, i32>	TBookmarkPair;
	typedef std::map<string, i32> TBookmarkMap;
	TBookmarkMap m_bookmarkedNames;			/// List of points to be marked as watched with their current ref counts.

	///Keep watched points and unwatched points separate. This way when watched points must be queried/sorted, it can be done more quickly.
	TCheckpointDebugList m_watchedPoints;		/// Current snapshot of checkpoints that have been requested for monitoring
	TCheckpointDebugList m_unwatchedPoints;	/// Current snapshot of checkpoints that are not watched with all required debug information

	float m_timeSinceLastRun;
};

#endif