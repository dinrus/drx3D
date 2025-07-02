// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/ICompiler.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{
// Forward declare interfaces.
struct IEnvClass;
struct IScriptClass;
struct IScriptConstructor;
struct IScriptElement;
struct IScriptGraph;
struct IScriptSignalReceiver;
struct IScriptStateMachine;
struct IScriptState;
struct IScriptTimer;
struct IScriptVariable;
// Forward declare structures.
struct SCompilerContext;
// Forward declare classes.
class CRuntimeClass;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(CRuntimeClass)

class CCompiler : public ICompiler
{
private:

	typedef std::unordered_map<DrxGUID, CRuntimeClassPtr> Classes;

	struct SSignals
	{
		ClassCompilationSignal classCompilation;
	};

	typedef std::vector<const IScriptClass*> InheritanceChain;

public:

	// ICompiler
	virtual void                           CompileAll() override;
	virtual void                           CompileDependencies(const DrxGUID& guid) override;
	virtual ClassCompilationSignal::Slots& GetClassCompilationSignalSlots() override;
	// ~ICompiler

private:

	bool CompileClass(const IScriptClass& scriptClass);
	bool CompileComponentInstancesRecursive(SCompilerContext& context, CRuntimeClass& runtimeClass, u32 parentIdx, const IScriptElement& scriptScope) const;
	bool CompileElementsRecursive(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptElement& scriptScope) const;
	bool CompileConstructor(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptConstructor& scriptConstructor) const;
	bool CompileStateMachine(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptStateMachine& scriptStateMachine) const;
	bool CompileState(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptState& scriptState) const;
	bool CompileVariable(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptVariable& scriptVariable) const;
	bool CompileTimer(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptTimer& scriptTimer) const;
	bool CompileSignalReceiver(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptSignalReceiver& scriptSignalReceiver) const;
	bool CompileGraph(SCompilerContext& context, CRuntimeClass& runtimeClass, const IScriptGraph& scriptGraph) const;

private:

	SSignals m_signals;
};
} // sxema
