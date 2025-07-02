// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   PipeUpr.h
   $Id$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 8:6:2005   14:17 : Created by Kirill Bulatsev

 *********************************************************************/
#ifndef __PipeUpr_H__
#define __PipeUpr_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <map>
#include <drx3D/AI/IAgent.h>

class CGoalPipe;
class CAISystem;
typedef std::map<string, CGoalPipe*> GoalMap;

class CPipeUpr
{
	friend class CAISystem;
public:
	enum ActionToTakeWhenDuplicateFound
	{
		SilentlyReplaceDuplicate,
		ReplaceDuplicateAndReportError,
	};

	CPipeUpr(void);
	~CPipeUpr(void);

	void       ClearAllGoalPipes();
	IGoalPipe* CreateGoalPipe(tukk pName,
	                          const ActionToTakeWhenDuplicateFound actionToTakeWhenDuplicateFound);
	IGoalPipe* OpenGoalPipe(tukk pName);
	CGoalPipe* IsGoalPipe(tukk pName);
	void       Serialize(TSerialize ser);

	/// For debug. Checks the script files for created and used goalpipes and
	/// dumps the unused goalpipes into console. Does not actually use the loaded pipes.
	void CheckGoalpipes();

private:

	// keeps all possible goal pipes that the agents can use
	GoalMap m_mapGoals;
	bool    m_bDynamicGoalPipes;  // to indicate if goalpipe is created after AISystem initialization, loading of \aiconfig.lua
};

#endif // __PipeUpr_H__
