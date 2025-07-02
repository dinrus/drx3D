// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef ExecutionStackFileLogger_h
#define ExecutionStackFileLogger_h

#pragma once

#include <drx3D/AI/IBehaviorTree.h>

#ifdef USING_BEHAVIOR_TREE_EXECUTION_STACKS_FILE_LOG
namespace BehaviorTree
{
class ExecutionStackFileLogger
{
public:
	explicit ExecutionStackFileLogger(const EntityId entityId);
	~ExecutionStackFileLogger();
	void LogDebugTree(const DebugTree& debugTree, const UpdateContext& updateContext, const BehaviorTreeInstance& instance);

private:
	enum LogFileOpenState
	{
		CouldNotAdjustFileName,
		NotYetAttemptedToOpenForWriteAccess,
		OpenForWriteAccessFailed,
		OpenForWriteAccessSucceeded
	};

	ExecutionStackFileLogger(const ExecutionStackFileLogger&);
	ExecutionStackFileLogger& operator=(const ExecutionStackFileLogger&);

	void                      LogNodeRecursively(const DebugNode& debugNode, const UpdateContext& updateContext, const BehaviorTreeInstance& instance, i32k indentLevel);

	string           m_agentName;
	char             m_logFilePath[IDrxPak::g_nMaxPath];
	LogFileOpenState m_openState;
	CDrxFile         m_logFile;
};
}
#endif  // USING_BEHAVIOR_TREE_EXECUTION_STACKS_FILE_LOG

#endif  // ExecutionStackFileLogger_h
