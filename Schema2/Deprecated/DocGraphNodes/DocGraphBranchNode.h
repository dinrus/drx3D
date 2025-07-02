// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphNodeBase.h>

namespace sxema2
{
	class CDocGraphBranchNode : public CDocGraphNodeBase
	{
	public:

		CDocGraphBranchNode(IScriptFile& file, IDocGraph& graph, const SGUID& guid = SGUID(), const SGUID& contextGUID = SGUID(), const SGUID& refGUID = SGUID(), Vec2 pos = Vec2(ZERO));

		// IScriptGraphNode
		virtual IAnyConstPtr GetCustomOutputDefault() const override;
		virtual size_t AddCustomOutput(const IAny& value) override;
		virtual void EnumerateOptionalOutputs(const ScriptGraphNodeOptionalOutputEnumerator& enumerator) override;
		virtual size_t AddOptionalOutput(tukk szName, EScriptGraphPortFlags flags, const CAggregateTypeId& typeId) override;
		virtual void RemoveOutput(size_t outputIdx) override;
		virtual void Refresh(const SScriptRefreshParams& params) override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		virtual void PreCompileSequence(IDocGraphSequencePreCompiler& preCompiler, size_t outputIdx) const override;
		virtual void LinkSequence(IDocGraphSequenceLinker& linker, size_t outputIdx, const LibFunctionId& functionId) const override;
		virtual void Compile(IDocGraphNodeCompiler& compiler, EDocGraphSequenceStep sequenceStep, size_t portIdx) const override;
		// ~IScriptGraphNode

	public:

		struct EInput
		{
			enum
			{
				In = 0,
				Value
			};
		};

		struct EOutput
		{
			enum
			{
				True = 0,
				False
			};
		};

		struct EStackFrame
		{
			enum
			{
				Value
			};
		};

		struct EMarker
		{
			enum
			{
				BranchFalse,
				BranchEnd
			};
		};

		void CompileInputs(IDocGraphNodeCompiler& compiler) const;
		void CompileTrue(IDocGraphNodeCompiler& compiler) const;
		void CompileFalse(IDocGraphNodeCompiler& compiler) const;
		void CompileEnd(IDocGraphNodeCompiler& compiler) const;

		bool m_bValue;
	};
}
