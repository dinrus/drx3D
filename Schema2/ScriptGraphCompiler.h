// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Rename CGraphCompiler?

#pragma once

#include <drx3D/Schema2/IRuntime.h>
#include <drx3D/Schema2/IScriptGraphNodeCompiler.h>

namespace sxema2
{
	class  CLibClass;
	struct IScriptGraphExtension;
	struct IScriptGraphNode;
	struct SCompiledFunction;
	struct SScriptGraphCompilerContext;

	class CScriptGraphCompiler
	{
	public:

		void CompileGraph(CLibClass& libClass, const IScriptGraphExtension& graph, CRuntimeFunction& runtimeFunction);

	private:

		void CompileFunction(CLibClass& libClass, const IScriptGraphExtension& graph, CRuntimeFunction& runtimeFunction, const IScriptGraphNode& graphNode, size_t outputIdx);
		u32 CompileFunctionForwardsRecursive(SScriptGraphCompilerContext& context, const IScriptGraphNode& graphNode, const SRuntimeActivationParams& activationParams);
		u32 CompileFunctionBackwardsRecursive(SScriptGraphCompilerContext& context, const IScriptGraphNode& graphNode, u32 outPos);
		u32 CompileNode(SScriptGraphCompilerContext& context, const IScriptGraphNode& graphNode);
		void OptimizeFunction(SCompiledFunction& compiledFunction);
		void IndexFunctionRecursive(SCompiledFunction& compiledFunction, std::vector<u32>& indices, u32 pos);
		void FinalizeFunction(const SCompiledFunction& compiledFunction, CRuntimeFunction& runtimeFunction);
	};
}
