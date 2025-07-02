// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptGraphNode.h>

#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/yasli/ClassFactory.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/SharedString.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>
#include <drx3D/Schema/SerializationContext.h>
#include <drx3D/Schema/ValidatorArchive.h>

namespace sxema
{
namespace
{
struct SInputSerializer
{
	inline SInputSerializer() {}

	inline SInputSerializer(const CAnyValuePtr& _pData)
		: pData(_pData)
	{}

	CAnyValuePtr pData;
};

inline bool Serialize(Serialization::IArchive& archive, SInputSerializer& value, tukk szName, tukk szLabel)
{
	if (value.pData)
	{
		archive(*value.pData, szName, szLabel);
		return true;
	}
	return false;
}

typedef std::map<CUniqueId, SInputSerializer> InputSerializers;
} // Anonymous

// #SchematycTODO: This is just a temporary solution to provide colors.
inline ColorB GetPortColor(const DrxGUID& typeGUID)
{
	struct SColors
	{
		DrxGUID typeGUID;
		ColorB  color;
	};

	static const SColors portColors[] =
	{
		{ GetTypeDesc<bool>().GetGUID(),          ColorB(0,   108, 217) },
		{ GetTypeDesc<i32>().GetGUID(),         ColorB(215, 55,  55)  },
		{ GetTypeDesc<u32>().GetGUID(),        ColorB(215, 55,  55)  },
		{ GetTypeDesc<float>().GetGUID(),         ColorB(185, 185, 185) },
		{ GetTypeDesc<Vec3>().GetGUID(),          ColorB(250, 232, 12)  },
		{ GetTypeDesc<DrxGUID>().GetGUID(),       ColorB(38,  184, 33)  },
		{ GetTypeDesc<CSharedString>().GetGUID(), ColorB(128, 100, 162) }
	};

	for (u32 portColorIdx = 0; portColorIdx < DRX_ARRAY_COUNT(portColors); ++portColorIdx)
	{
		if (portColors[portColorIdx].typeGUID == typeGUID)
		{
			return portColors[portColorIdx].color;
		}
	}
	return ColorB(28, 212, 22);
}
// ~SchematycTODO

SScriptGraphNodePort::SScriptGraphNodePort() {}

SScriptGraphNodePort::SScriptGraphNodePort(const CUniqueId& _id, tukk _szName, const DrxGUID& _typeGUID, const ScriptGraphPortFlags& _flags, const CAnyValuePtr& _pData)
	: id(_id)
	, name(_szName)
	, typeGUID(_typeGUID)
	, flags(_flags)
	, pData(_pData)
{}

void CScriptGraphNodeLayout::SetName(tukk szBehavior, tukk szSubject)
{
	const bool bShowBehavior = szBehavior && (szBehavior[0] != '\0');
	const bool bShowSubject = szSubject && (szSubject[0] != '\0');
	if (bShowBehavior)
	{
		m_name = szBehavior;
		if (bShowSubject)
		{
			m_name.append(" [");
			m_name.append(szSubject);
			m_name.append("]");
		}
	}
	else if (bShowSubject)
	{
		m_name = szSubject;
	}
}

tukk CScriptGraphNodeLayout::GetName() const
{
	return m_name.c_str();
}

void CScriptGraphNodeLayout::SetStyleId(tukk szStyleId)
{
	m_styleId = szStyleId;
}

tukk CScriptGraphNodeLayout::GetStyleId() const
{
	return m_styleId.c_str();
}

ScriptGraphNodePorts& CScriptGraphNodeLayout::GetInputs()
{
	return m_inputs;
}

const ScriptGraphNodePorts& CScriptGraphNodeLayout::GetInputs() const
{
	return m_inputs;
}

const ScriptGraphNodePorts& CScriptGraphNodeLayout::GetOutputs() const
{
	return m_outputs;
}

sxema::ScriptGraphNodePorts& CScriptGraphNodeLayout::GetOutputs()
{
	return m_outputs;
}

void CScriptGraphNodeLayout::Exchange(CScriptGraphNodeLayout& rhs)
{
	for (SScriptGraphNodePort& rhsInput : rhs.m_inputs)
	{
		if (rhsInput.pData)
		{
			for (SScriptGraphNodePort& input : m_inputs)
			{
				if (input.id == rhsInput.id)
				{
					if (input.typeGUID == rhsInput.typeGUID)
					{
						Any::CopyAssign(*rhsInput.pData, *input.pData);
					}
					break;
				}
			}
		}
	}

	std::swap(m_name, rhs.m_name);
	std::swap(m_styleId, rhs.m_styleId);
	std::swap(m_inputs, rhs.m_inputs);
	std::swap(m_outputs, rhs.m_outputs);
}

void CScriptGraphNodeLayout::AddInput(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyValuePtr& pData)
{
	// #SchematycTODO : Check for id collisions!!!
	m_inputs.push_back(SScriptGraphNodePort(id, szName, typeGUID, flags, pData));
}

void CScriptGraphNodeLayout::AddOutput(const CUniqueId& id, tukk szName, const DrxGUID& typeGUID, const ScriptGraphPortFlags& flags, const CAnyValuePtr& pData)
{
	// #SchematycTODO : Check for id collisions!!!
	m_outputs.push_back(SScriptGraphNodePort(id, szName, typeGUID, flags, pData));
}

CScriptGraphNode::CScriptGraphNode(const DrxGUID& guid, std::unique_ptr<CScriptGraphNodeModel> pModel)
	: m_guid(guid)
	, m_pModel(std::move(pModel))
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		m_pModel->Attach(*this);
		m_pModel->Init();
	}
}

CScriptGraphNode::CScriptGraphNode(const DrxGUID& guid, std::unique_ptr<CScriptGraphNodeModel> pModel, const Vec2& pos)
	: m_guid(guid)
	, m_pModel(std::move(pModel))
	, m_pos(pos)
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		m_pModel->Attach(*this);
		m_pModel->Init();
	}
}

void CScriptGraphNode::Attach(IScriptGraph& graph)
{
	m_pGraph = &graph;
}

IScriptGraph& CScriptGraphNode::GetGraph()
{
	SXEMA_CORE_ASSERT(m_pGraph);
	return *m_pGraph;
}

const IScriptGraph& CScriptGraphNode::GetGraph() const
{
	SXEMA_CORE_ASSERT(m_pGraph);
	return *m_pGraph;
}

DrxGUID CScriptGraphNode::GetTypeGUID() const
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		return m_pModel->GetTypeGUID();
	}
	return DrxGUID();
}

DrxGUID CScriptGraphNode::GetGUID() const
{
	return m_guid;
}

tukk CScriptGraphNode::GetName() const
{
	return m_layout.GetName();
}

tukk CScriptGraphNode::GetStyleId() const
{
	return m_layout.GetStyleId();
}

ScriptGraphNodeFlags CScriptGraphNode::GetFlags() const
{
	return m_flags;
}

void CScriptGraphNode::SetPos(Vec2 pos)
{
	m_pos = pos;
}

Vec2 CScriptGraphNode::GetPos() const
{
	return m_pos;
}

u32 CScriptGraphNode::GetInputCount() const
{
	return m_layout.GetInputs().size();
}

u32 CScriptGraphNode::FindInputById(const CUniqueId& id) const
{
	const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
	for (u32 inputIdx = 0, inputCount = inputs.size(); inputIdx < inputCount; ++inputIdx)
	{
		if (inputs[inputIdx].id == id)
		{
			return inputIdx;
		}
	}
	return InvalidIdx;
}

CUniqueId CScriptGraphNode::GetInputId(u32 inputIdx) const
{
	const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
	return inputIdx < inputs.size() ? inputs[inputIdx].id : CUniqueId();
}

tukk CScriptGraphNode::GetInputName(u32 inputIdx) const
{
	const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
	return inputIdx < inputs.size() ? inputs[inputIdx].name.c_str() : "";
}

DrxGUID CScriptGraphNode::GetInputTypeGUID(u32 inputIdx) const
{
	const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
	return inputIdx < inputs.size() ? inputs[inputIdx].typeGUID : DrxGUID();
}

ScriptGraphPortFlags CScriptGraphNode::GetInputFlags(u32 inputIdx) const
{
	const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
	return inputIdx < inputs.size() ? inputs[inputIdx].flags : ScriptGraphPortFlags();
}

CAnyConstPtr CScriptGraphNode::GetInputData(u32 inputIdx) const
{
	const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
	return inputIdx < inputs.size() ? inputs[inputIdx].pData.get() : nullptr;
}

ColorB CScriptGraphNode::GetInputColor(u32 inputIdx) const
{
	return GetPortColor(GetInputTypeGUID(inputIdx));
}

u32 CScriptGraphNode::GetOutputCount() const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	return outputs.size();
}

u32 CScriptGraphNode::FindOutputById(const CUniqueId& id) const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	for (u32 outputIdx = 0, outputCount = outputs.size(); outputIdx < outputCount; ++outputIdx)
	{
		if (outputs[outputIdx].id == id)
		{
			return outputIdx;
		}
	}
	return InvalidIdx;
}

CUniqueId CScriptGraphNode::GetOutputId(u32 outputIdx) const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	return outputIdx < outputs.size() ? outputs[outputIdx].id : CUniqueId();
}

tukk CScriptGraphNode::GetOutputName(u32 outputIdx) const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	return outputIdx < outputs.size() ? outputs[outputIdx].name.c_str() : "";
}

DrxGUID CScriptGraphNode::GetOutputTypeGUID(u32 outputIdx) const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	return outputIdx < outputs.size() ? outputs[outputIdx].typeGUID : DrxGUID();
}

ScriptGraphPortFlags CScriptGraphNode::GetOutputFlags(u32 outputIdx) const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	return outputIdx < outputs.size() ? outputs[outputIdx].flags : ScriptGraphPortFlags();
}

CAnyConstPtr CScriptGraphNode::GetOutputData(u32 outputIdx) const
{
	const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
	return outputIdx < outputs.size() ? outputs[outputIdx].pData.get() : nullptr;
}

ColorB CScriptGraphNode::GetOutputColor(u32 outputIdx) const
{
	return GetPortColor(GetOutputTypeGUID(outputIdx));
}

void CScriptGraphNode::EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const
{
	// #SchematycTODO : Implement me!!!
}

void CScriptGraphNode::RemapDependencies(IGUIDRemapper& guidRemapper)
{
	m_guid = guidRemapper.Remap(m_guid);
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		m_pModel->RemapDependencies(guidRemapper);
	}
}

void CScriptGraphNode::ProcessEvent(const SScriptEvent& event)
{
	switch (event.id)
	{
	case EScriptEventId::FileReload:
	case EScriptEventId::EditorAdd:
	case EScriptEventId::EditorPaste:
	case EScriptEventId::EditorRefresh:
		{
			RefreshLayout();
			break;
		}
	}
}

void CScriptGraphNode::Serialize(Serialization::IArchive& archive)
{
	ISerializationContext* pSerializationContext = SerializationContext::Get(archive);
	SXEMA_CORE_ASSERT(pSerializationContext);
	if (pSerializationContext)
	{
		switch (pSerializationContext->GetPass())
		{
		case ESerializationPass::LoadDependencies:
			{
				SerializeBasicInfo(archive);
				if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
				{
					m_pModel->LoadDependencies(archive, *pSerializationContext);
				}
				break;
			}
		case ESerializationPass::Load:
			{
				SerializeBasicInfo(archive);
				if (m_pModel)
				{
					m_pModel->Load(archive, *pSerializationContext);
				}
				SerializeInputs(archive);
				break;
			}
		case ESerializationPass::PostLoad:
			{
				if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
				{
					m_pModel->PostLoad(archive, *pSerializationContext);
				}
				RefreshLayout();
				SerializeInputs(archive);
				break;
			}
		case ESerializationPass::Save:
			{
				SerializeBasicInfo(archive);
				if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
				{
					m_pModel->Save(archive, *pSerializationContext);
				}
				SerializeInputs(archive);
				break;
			}
		case ESerializationPass::Edit:
			{
				if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
				{
					//archive(*m_pModel, "detail", "Detail");
					m_pModel->Edit(archive, *pSerializationContext);
				}
				SerializeInputs(archive);
				break;
			}
		case ESerializationPass::Validate:
			{
				if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
				{
					m_pModel->Validate(archive, *pSerializationContext);
				}
				break;
			}
		}
	}
}

void CScriptGraphNode::Copy(Serialization::IArchive& archive)
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		CSerializationContext serializationContext(SSerializationContextParams(archive, ESerializationPass::Save));
		m_pModel->Save(archive, serializationContext);
	}
	SerializeInputs(archive);
}

void CScriptGraphNode::Paste(Serialization::IArchive& archive)
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		{
			CSerializationContext serializationContext(SSerializationContextParams(archive, ESerializationPass::LoadDependencies));
			m_pModel->LoadDependencies(archive, serializationContext);
		}
		{
			CSerializationContext serializationContext(SSerializationContextParams(archive, ESerializationPass::Load));
			m_pModel->Load(archive, serializationContext);
		}
		{
			CSerializationContext serializationContext(SSerializationContextParams(archive, ESerializationPass::PostLoad));
			m_pModel->PostLoad(archive, serializationContext);
		}
	}
	RefreshLayout();
	SerializeInputs(archive);
}

void CScriptGraphNode::Validate(const Validator& validator) const
{
	CValidatorArchive validatorArchive = CValidatorArchive(SValidatorArchiveParams());
	CSerializationContext serializationContext(SSerializationContextParams(validatorArchive, ESerializationPass::Validate));
	const_cast<CScriptGraphNode*>(this)->Serialize(validatorArchive);
	validatorArchive.Validate(validator);
}

void CScriptGraphNode::Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		m_pModel->Compile(context, compiler);
	}
}

void CScriptGraphNode::SetFlags(const ScriptGraphNodeFlags& flags)
{
	m_flags = flags;
}

u32 CScriptGraphNode::FindInputByName(tukk szName) const
{
	SXEMA_CORE_ASSERT(szName);
	if (szName)
	{
		const ScriptGraphNodePorts& inputs = m_layout.GetInputs();
		for (u32 inputIdx = 0, inputCount = inputs.size(); inputIdx < inputCount; ++inputIdx)
		{
			if (strcmp(inputs[inputIdx].name.c_str(), szName) == 0)
			{
				return inputIdx;
			}
		}
	}
	return InvalidIdx;
}

u32 CScriptGraphNode::FindOutputByName(tukk szName) const
{
	SXEMA_CORE_ASSERT(szName);
	if (szName)
	{
		const ScriptGraphNodePorts& outputs = m_layout.GetOutputs();
		for (u32 outputIdx = 0, outputCount = outputs.size(); outputIdx < outputCount; ++outputIdx)
		{
			if (strcmp(outputs[outputIdx].name.c_str(), szName) == 0)
			{
				return outputIdx;
			}
		}
	}
	return InvalidIdx;
}

void CScriptGraphNode::RefreshLayout()
{
	if (m_pModel) // #SchematycTODO : Can we just assume this is always valid?
	{
		CScriptGraphNodeLayout newLayout;
		m_pModel->CreateLayout(newLayout);
		m_layout.Exchange(newLayout);
	}
}

void CScriptGraphNode::SerializeBasicInfo(Serialization::IArchive& archive)
{
	archive(m_guid, "guid");
	archive(m_pos, "pos");
}

void CScriptGraphNode::SerializeInputs(Serialization::IArchive& archive)
{
	ScriptGraphNodePorts& inputs = m_layout.GetInputs();

	u32 dataInputCount = 0;
	for (const SScriptGraphNodePort& input : inputs)
	{
		if (input.pData)
		{
			++dataInputCount;
		}
	}

	if (dataInputCount)
	{
		if (archive.isEdit())
		{
			if (archive.openBlock("inputs", "Inputs"))
			{
				for (SScriptGraphNodePort& input : inputs)
				{
					if (input.pData && input.flags.Check(EScriptGraphPortFlags::Editable))
					{
						tukk szName = input.name.c_str();
						archive(*input.pData, szName, szName);
					}
				}
				archive.closeBlock();
			}
		}
		else
		{
			InputSerializers inputSerializers;
			for (SScriptGraphNodePort& input : inputs)
			{
				if (input.pData && input.flags.Check(EScriptGraphPortFlags::Persistent))
				{
					inputSerializers.insert(InputSerializers::value_type(input.id, SInputSerializer(input.pData)));
				}
			}
			archive(inputSerializers, "inputs");
		}
	}
}
} // sxema
