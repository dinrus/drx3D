// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptGraph.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>

namespace sxema
{

// Forward declare classes.
class CScriptGraphNodeModel;

struct SScriptGraphNodePort
{
	SScriptGraphNodePort();
	SScriptGraphNodePort(const CUniqueId& _id, tukk _szName, const DrxGUID& _typeGUID, const ScriptGraphPortFlags& _flags, const CAnyValuePtr& _pData);

	CUniqueId            id;
	string               name;
	DrxGUID                typeGUID;
	ScriptGraphPortFlags flags;
	CAnyValuePtr         pData;
};

typedef std::vector<SScriptGraphNodePort> ScriptGraphNodePorts;

class CScriptGraphNodeLayout   // #SchematycTODO : Move to separate cpp/h files?
{
public:

	void                        SetName(tukk szBehavior, tukk szSubject = nullptr);
	tukk                 GetName() const;
	void                        SetStyleId(tukk szStyleId);
	tukk                 GetStyleId() const;
	ScriptGraphNodePorts&       GetInputs(); // #SchematycTODO : Rather than allowing non-const access should we just provide a Serialize() function?
	const ScriptGraphNodePorts& GetInputs() const;
	ScriptGraphNodePorts&       GetOutputs(); // #SchematycTODO : Rather than allowing non-const access should we just provide a Serialize() function?
	const ScriptGraphNodePorts& GetOutputs() const;

	void                        Exchange(CScriptGraphNodeLayout& rhs);

	inline void                 AddInput(tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags)
	{
		AddInput(CUniqueId::FromIdx(m_inputs.size()), szName, typeGUID, flags, CAnyValuePtr());
	}

	inline void AddInput(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags)
	{
		AddInput(id, szName, typeGUID, flags, CAnyValuePtr());
	}

	inline void AddInputWithData(tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyConstRef& value)
	{
		AddInput(CUniqueId::FromIdx(m_inputs.size()), szName, typeGUID, flags, CAnyValue::CloneShared(value));
	}

	inline void AddInputWithData(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyConstRef& value)
	{
		AddInput(id, szName, typeGUID, flags, CAnyValue::CloneShared(value));
	}

	inline void AddOutput(tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags)
	{
		AddOutput(CUniqueId::FromIdx(m_outputs.size()), szName, typeGUID, flags, CAnyValuePtr());
	}

	inline void AddOutput(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags)
	{
		AddOutput(id, szName, typeGUID, flags, CAnyValuePtr());
	}

	inline void AddOutputWithData(tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyConstRef& value)
	{
		AddOutput(CUniqueId::FromIdx(m_outputs.size()), szName, typeGUID, flags, CAnyValue::CloneShared(value));
	}

	inline void AddOutputWithData(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyConstRef& value)
	{
		AddOutput(id, szName, typeGUID, flags, CAnyValue::CloneShared(value));
	}

private:

	void AddInput(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyValuePtr& pData);
	void AddOutput(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyValuePtr& pData);

private:

	string               m_name;
	string               m_styleId;
	ScriptGraphNodePorts m_inputs;
	ScriptGraphNodePorts m_outputs;
};

class CScriptGraphNode : public IScriptGraphNode
{
public:

	CScriptGraphNode(const DrxGUID& guid, std::unique_ptr<CScriptGraphNodeModel> pModel);                  // #SchematycTODO : Make pModel the first parameter?
	CScriptGraphNode(const DrxGUID& guid, std::unique_ptr<CScriptGraphNodeModel> pModel, const Vec2& pos); // #SchematycTODO : Make pModel the first parameter?

	// IScriptGraphNode
	virtual void                 Attach(IScriptGraph& graph) override;
	virtual IScriptGraph&        GetGraph() override;
	virtual const IScriptGraph&  GetGraph() const override;
	virtual DrxGUID                GetTypeGUID() const override;
	virtual DrxGUID                GetGUID() const override;
	virtual tukk          GetName() const override;
	virtual tukk          GetStyleId() const override;
	virtual ScriptGraphNodeFlags GetFlags() const override;
	virtual void                 SetPos(Vec2 pos) override;
	virtual Vec2                 GetPos() const override;
	virtual u32               GetInputCount() const override;
	virtual u32               FindInputById(const CUniqueId& id) const override;
	virtual CUniqueId            GetInputId(u32 inputIdx) const override;
	virtual tukk          GetInputName(u32 inputIdx) const override;
	virtual DrxGUID                GetInputTypeGUID(u32 inputIdx) const override;
	virtual ScriptGraphPortFlags GetInputFlags(u32 inputIdx) const override;
	virtual CAnyConstPtr         GetInputData(u32 inputIdx) const override;
	virtual ColorB               GetInputColor(u32 inputIdx) const override;
	virtual u32               GetOutputCount() const override;
	virtual u32               FindOutputById(const CUniqueId& id) const override;
	virtual CUniqueId            GetOutputId(u32 outputIdx) const override;
	virtual tukk          GetOutputName(u32 outputIdx) const override;
	virtual DrxGUID                GetOutputTypeGUID(u32 outputIdx) const override;
	virtual ScriptGraphPortFlags GetOutputFlags(u32 outputIdx) const override;
	virtual CAnyConstPtr         GetOutputData(u32 outputIdx) const override;
	virtual ColorB               GetOutputColor(u32 outputIdx) const override;
	virtual void                 EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void                 RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void                 ProcessEvent(const SScriptEvent& event) override;
	virtual void                 Serialize(Serialization::IArchive& archive) override;
	virtual void                 Copy(Serialization::IArchive& archive) override;
	virtual void                 Paste(Serialization::IArchive& archive) override;
	virtual void                 Validate(const Validator& validator) const override;
	virtual void                 Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	// ~IScriptGraphNode

	void   SetFlags(const ScriptGraphNodeFlags& flags);
	u32 FindInputByName(tukk szName) const;
	u32 FindOutputByName(tukk szName) const;

private:

	void RefreshLayout();

	void SerializeBasicInfo(Serialization::IArchive& archive);
	void SerializeInputs(Serialization::IArchive& archive);

private:

	DrxGUID                                  m_guid;
	std::unique_ptr<CScriptGraphNodeModel> m_pModel;
	Vec2                                   m_pos = Vec2(ZERO);
	string                                 m_styleId;
	ScriptGraphNodeFlags                   m_flags;
	CScriptGraphNodeLayout                 m_layout;
	IScriptGraph*                          m_pGraph = nullptr;
};

} // sxema
