// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IInterfaceMap.h>

namespace sxema
{
// Forward declare interfaces.
struct IScriptGraph;

struct ICompilerTaskList
{
	virtual ~ICompilerTaskList() {}

	virtual void CompileGraph(const IScriptGraph& scriptGraph) = 0;
};

typedef IInterfaceMap<void> ICompilerInterfaceMap;

struct SCompilerContext
{
	inline SCompilerContext(ICompilerTaskList& _tasks, ICompilerInterfaceMap& _interfaces)
		: tasks(_tasks)
		, interfaces(_interfaces)
	{}

	ICompilerTaskList&     tasks;
	ICompilerInterfaceMap& interfaces;
};
} // sxema
