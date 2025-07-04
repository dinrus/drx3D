// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptGraph.h>

namespace sxema2
{
	class CScriptGraphNodeBase : public IScriptGraphNode
	{
	private:

		struct SPort
		{
			SPort(EScriptGraphPortFlags _flags, tukk _szName, const CAggregateTypeId& _typeId);

			EScriptGraphPortFlags flags;
			string                name;
			CAggregateTypeId      typeId;
		};

		typedef std::vector<SPort> PortVector;

	public:

		CScriptGraphNodeBase(); // #SchematycTODO : Do we really need a default constructor?
		CScriptGraphNodeBase(const SGUID& guid);
		CScriptGraphNodeBase(const SGUID& guid, const Vec2& pos);

		// IScriptGraphNode
		virtual void Attach(IScriptGraphExtension& graph) override;
		virtual void SetGUID(const SGUID& guid) override;
		virtual SGUID GetGUID() const override;
		virtual void SetName(tukk szName) override;
		virtual tukk GetName() const override;
		virtual EScriptGraphNodeType GetType() const override;
		virtual SGUID GetContextGUID() const override;
		virtual SGUID GetRefGUID() const override;
		virtual void SetPos(Vec2 pos) override;
		virtual Vec2 GetPos() const override;
		virtual size_t GetInputCount() const override;
		virtual u32 FindInput(tukk szName) const override;
		virtual tukk GetInputName(size_t inputIdx) const override;
		virtual EScriptGraphPortFlags GetInputFlags(size_t inputIdx) const override;
		virtual CAggregateTypeId GetInputTypeId(size_t inputIdx) const override;
		virtual size_t GetOutputCount() const override;
		virtual u32 FindOutput(tukk szName) const override;
		virtual tukk GetOutputName(size_t outputIdx) const override;
		virtual EScriptGraphPortFlags GetOutputFlags(size_t outputIdx) const override;
		virtual CAggregateTypeId GetOutputTypeId(size_t outputIdx) const override;
		virtual IAnyConstPtr GetCustomOutputDefault() const override;
		virtual size_t AddCustomOutput(const IAny& value) override;
		virtual void EnumerateOptionalOutputs(const ScriptGraphNodeOptionalOutputEnumerator& enumerator) override;
		virtual size_t AddOptionalOutput(tukk szName, EScriptGraphPortFlags flags, const CAggregateTypeId& typeId) override;
		virtual void RemoveOutput(size_t outputIdx) override;
		virtual void Refresh(const SScriptRefreshParams& params) override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		virtual void RemapGUIDs(IGUIDRemapper& guidRemapper) override;
		virtual void PreCompileSequence(IDocGraphSequencePreCompiler& preCompiler, size_t outputIdx) const override;
		virtual void LinkSequence(IDocGraphSequenceLinker& linker, size_t outputIdx, const LibFunctionId& functionId) const override;
		virtual void Compile(IDocGraphNodeCompiler& compiler, EDocGraphSequenceStep sequenceStep, size_t portIdx) const override;
		// ~IScriptGraphNode

	protected:

		IScriptGraphExtension* GetGraph(); // #SchematycTODO : Move this function to the IScriptGraphNode interface? Should we return a reference?
		const IScriptGraphExtension* GetGraph() const; // #SchematycTODO : Move this function to the IScriptGraphNode interface? Should we return a reference?
		tukk GetDebugLabel() const; // #SchematycTODO : Move this function to the IScriptGraphNode interface?
		size_t AddInput(EScriptGraphPortFlags flags, tukk szName, const CAggregateTypeId& typeId = CAggregateTypeId());
		void SetInputTypeId(size_t inputIdx, const CAggregateTypeId& typeId); // #SchematycTODO : Remove!!!
		void RemoveInput(size_t inputIdx);
		size_t AddOutput(EScriptGraphPortFlags flags, tukk szName, const CAggregateTypeId& typeId = CAggregateTypeId());
		void SetOutputTypeId(size_t outputIdx, const CAggregateTypeId& typeId); // #SchematycTODO : Remove!!!

	private:

		void SerializeBasicInfo(Serialization::IArchive& archive);
		void SerializeDebugLabel(Serialization::IArchive& archive);
		void Edit(Serialization::IArchive& archive);

	private:

		IScriptGraphExtension* m_pGraph;
		SGUID                  m_guid;
		string                 m_name;
		string                 m_debugLabel;
		Vec2                   m_pos;
		PortVector             m_inputs;
		PortVector             m_outputs;
	};
}
