// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphNodeBase.h>

namespace sxema2
{
	class CDocGraphAbstractInterfaceFunctionNode : public CDocGraphNodeBase
	{
	public:

		CDocGraphAbstractInterfaceFunctionNode(IScriptFile& file, IDocGraph& graph, const SGUID& guid = SGUID(), const SGUID& contextGUID = SGUID(), const SGUID& refGUID = SGUID(), Vec2 pos = Vec2(ZERO));

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

	private:

		struct EInput
		{
			enum
			{
				In = 0,
				Object,
				FirstParam
			};
		};

		struct EOutput
		{
			enum
			{
				True = 0,
				False,
				FirstParam
			};
		};

		struct EStackFrame
		{
			enum
			{
				Body,
				True,
				False
			};
		};

		struct EMarker
		{
			enum
			{
				False,
				End
			};
		};

		void CompileInputs(IDocGraphNodeCompiler& compiler) const;
		void CompileTrue(IDocGraphNodeCompiler& compiler) const;
		void CompileFalse(IDocGraphNodeCompiler& compiler) const;
		void CompileEnd(IDocGraphNodeCompiler& compiler) const;

		// #SchematycTODO : Can we store domain/origin?
		IAnyPtrVector m_inputValues;
		IAnyPtrVector m_outputValues;
	};
}
