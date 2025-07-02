// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef   __SCRIPTBIND_GAMESTATISTICS_H__
#define   __SCRIPTBIND_GAMESTATISTICS_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

#define STATISTICS_CHECK_BINDING 1

struct IStatsTracker;
class CGameStatistics;

class CScriptBind_GameStatistics : public CScriptableBase
{
public:
	CScriptBind_GameStatistics(CGameStatistics* pGS);
	virtual ~CScriptBind_GameStatistics();

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
#if STATISTICS_CHECK_BINDING
		pSizer->AddObject(m_boundTrackers);
#endif
	}

	//! <code>GameStatistics.PushGameScope(scopeID)</code>
	//!		<param name="scopeID">identifier of the scope to be placed on top of the stack.</param>
	//! <description>Pushes a scope on top of the stack.</description>
	virtual i32 PushGameScope(IFunctionHandler* pH, i32 scopeID);

	//! <code>GameStatistics.PopGameScope( [checkScopeId] )</code>
	//!		<param name="checkScopeId">(optional)</param>
	//! <description>Removes the scope from the top of the stack.</description>
	virtual i32 PopGameScope(IFunctionHandler* pH);

	//! <code>GameStatistics.CurrentScope()</code>
	//! <description>Returns the ID of current scope, -1 if stack is empty.</description>
	virtual i32 CurrentScope(IFunctionHandler* pH);

	//! <code>GameStatistics.AddGameElement( scopeID, elementID, locatorID, locatorValue [, table])</code>
	//! <description>Adds a game element to specified scope.</description>
	virtual i32 AddGameElement(IFunctionHandler* pH);

	//! <code>GameStatistics.RemoveGameElement( scopeID, elementID, locatorID, locatorValue )</code>
	//! <description>Removes element from the specified scope if data parameters match.</description>
	virtual i32 RemoveGameElement(IFunctionHandler* pH);

	//! <code>GameStatistics.Event()</code>
	virtual i32 Event(IFunctionHandler* pH);

	//! <code>GameStatistics.StateValue( )</code>
	virtual i32 StateValue(IFunctionHandler* pH);

	//! <code>GameStatistics.BindTracker( name, tracker )</code>
	bool BindTracker(IScriptTable* pT, tukk name, IStatsTracker* tracker);

	//! <code>GameStatistics.UnbindTracker( name, tracker )</code>
	bool UnbindTracker(IScriptTable* pT, tukk name, IStatsTracker* tracker);

private:
	void                    RegisterMethods();
	void                    RegisterGlobals();
	static IStatsTracker*   GetTracker(IFunctionHandler* pH);
	static IGameStatistics* GetStats(IFunctionHandler* pH);

	//CScriptableBase m_StatsTracker; //[FINDME][SergeyR]: ctor is protected
	CGameStatistics* m_pGS;
	IScriptSystem*   m_pSS;

#if STATISTICS_CHECK_BINDING
	std::set<IStatsTracker*> m_boundTrackers;
#endif
};

#endif //__SCRIPTBIND_GAMESTATISTICS_H__
