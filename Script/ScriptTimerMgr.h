// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
// ScriptTimerMgr.h: interface for the CScriptTimerMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTTIMERMGR_H__D0CF3857_AEE5_4606_ADF9_549779C7DEEC__INCLUDED_)
#define AFX_SCRIPTTIMERMGR_H__D0CF3857_AEE5_4606_ADF9_549779C7DEEC__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000
#include <map>

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Sys/TimeValue.h>
#include <drx3D/Network/ISerialize.h>

//////////////////////////////////////////////////////////////////////////
struct ScriptTimer
{
	i32              nTimerID;
	int64            nMillis;
	int64            nEndTime;
	int64            nStartTime;
	char             sFuncName[128];
	HSCRIPTFUNCTION  pScriptFunction;
	SmartScriptTable pUserData;
	bool             bUpdateDuringPause;

	ScriptTimer() : nTimerID(0), nMillis(0), nStartTime(0), nEndTime(0), bUpdateDuringPause(0), pScriptFunction(0) { sFuncName[0] = 0; }
	void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
};

typedef std::map<i32, ScriptTimer*> ScriptTimerMap;
typedef ScriptTimerMap::iterator    ScriptTimerMapItor;

//////////////////////////////////////////////////////////////////////////
class CScriptTimerMgr
{
public:
	CScriptTimerMgr(IScriptSystem* pScriptSystem);
	virtual ~CScriptTimerMgr();
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void Serialize(TSerialize& ser);

	// Create a new timer and put it in the list of managed timers.
	i32  AddTimer(ScriptTimer& timer);
	void RemoveTimer(i32 nTimerID);
	void Update(int64 nCurrentTime);
	void Reset();
	void Pause(bool bPause);
private:
	void DeleteTimer(ScriptTimer* pTimer);

	ScriptTimerMap m_mapTimers;
	ScriptTimerMap m_mapTempTimers;
	IScriptSystem* m_pScriptSystem;
	i32            m_nLastTimerID;
	i32            m_nTimerIDCurrentlyBeingCalled;
	bool           m_bPause;
};

#endif // !defined(AFX_SCRIPTTIMERMGR_H__D0CF3857_AEE5_4606_ADF9_549779C7DEEC__INCLUDED_)
