// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/CompilerContext.h>

namespace sxema
{
class CCompilerTaskList : public ICompilerTaskList
{
public:

	typedef std::vector<const IScriptGraph*> PendingGraphs;

public:

	CCompilerTaskList();

	// ICompilerTaskList
	virtual void CompileGraph(const IScriptGraph& scriptGraph) override;
	// ~ICompilerTaskList

	PendingGraphs& GetPendingGraphs();

private:

	PendingGraphs m_pendingGraphs;
};
} // sxema
