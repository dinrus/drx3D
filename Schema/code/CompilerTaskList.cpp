// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/CompilerTaskList.h>

namespace sxema
{
CCompilerTaskList::CCompilerTaskList()
{
	m_pendingGraphs.reserve(20);
}

void CCompilerTaskList::CompileGraph(const IScriptGraph& scriptGraph)
{
	m_pendingGraphs.push_back(&scriptGraph);
}

CCompilerTaskList::PendingGraphs& CCompilerTaskList::GetPendingGraphs()
{
	return m_pendingGraphs;
}
} // sxema
