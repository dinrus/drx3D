// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphSendSignalNode.h>

#include <drx3D/Schema2/ICompiler.h>
#include <drx3D/Schema2/LibUtils.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/Deprecated/IGlobalFunction.h>
#include <drx3D/Schema2/ISerializationContext.h>

#include <drx3D/Schema2/ScriptVariableDeclaration.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EDocGraphSendSignalNodeTarget, "sxema Script Graph Send Signal Node Target")
	SERIALIZATION_ENUM(sxema2::EDocGraphSendSignalNodeTarget::Self, "self", "Send To Self")
	SERIALIZATION_ENUM(sxema2::EDocGraphSendSignalNodeTarget::Object, "object", "Send To Object")
	SERIALIZATION_ENUM(sxema2::EDocGraphSendSignalNodeTarget::Broadcast, "broadcast", "Broadcast To All Objects")
SERIALIZATION_ENUM_END()

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CDocGraphSendSignalNode::CDocGraphSendSignalNode(IScriptFile& file, IDocGraph& graph, const SGUID& guid, const SGUID& contextGUID, const SGUID& refGUID, Vec2 pos)
		: CDocGraphNodeBase(file, graph, guid, "Send Signal", EScriptGraphNodeType::SendSignal, contextGUID, refGUID, pos)
		, m_target(EDocGraphSendSignalNodeTarget::Self)
	{
		Refresh(SScriptRefreshParams(EScriptRefreshReason::Other));
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CDocGraphSendSignalNode::GetCustomOutputDefault() const
	{
		return IAnyConstPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphSendSignalNode::AddCustomOutput(const IAny& value)
	{
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::EnumerateOptionalOutputs(const ScriptGraphNodeOptionalOutputEnumerator& enumerator) {}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphSendSignalNode::AddOptionalOutput(tukk szName, EScriptGraphPortFlags flags, const CAggregateTypeId& typeId)
	{
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::RemoveOutput(size_t outputIdx) {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::Refresh(const SScriptRefreshParams& params)
	{
		LOADING_TIME_PROFILE_SECTION;

		CDocGraphNodeBase::Refresh(params);

		CDocGraphNodeBase::AddInput("In", EScriptGraphPortFlags::MultiLink | EScriptGraphPortFlags::Execute);
		if(m_target == EDocGraphSendSignalNodeTarget::Object)
		{
			CDocGraphNodeBase::AddInput("Object", EScriptGraphPortFlags::None, GetAggregateTypeId<ObjectId>());
		}
		CDocGraphNodeBase::AddOutput("Out", EScriptGraphPortFlags::Execute);

		const IScriptFile&   file = CDocGraphNodeBase::GetFile();
		const SGUID          refGUID = CDocGraphNodeBase::GetRefGUID();
		const IScriptSignal* pSignal = ScriptIncludeRecursionUtils::GetSignal(file, refGUID).second;
		if(pSignal)
		{
			stack_string name = "Send Signal: ";
			name.append(pSignal->GetName());
			CDocGraphNodeBase::SetName(name.c_str());

			const size_t signalInputCount = pSignal->GetInputCount();
			m_signalInputNames.resize(signalInputCount);
			m_signalInputValues.resize(signalInputCount);
			for(size_t signalInputIdx = 0; signalInputIdx < signalInputCount; ++ signalInputIdx)
			{
				IAnyConstPtr pSignalInputValue = pSignal->GetInputValue(signalInputIdx);
				DRX_ASSERT(pSignalInputValue);
				if(pSignalInputValue)
				{
					tukk szSignalInputName = pSignal->GetInputName(signalInputIdx);
					CDocGraphNodeBase::AddInput(szSignalInputName, EScriptGraphPortFlags::None, pSignalInputValue->GetTypeInfo().GetTypeId());
					m_signalInputNames[signalInputIdx] = szSignalInputName;
					if(!m_signalInputValues[signalInputIdx] || (m_signalInputValues[signalInputIdx]->GetTypeInfo().GetTypeId() != pSignalInputValue->GetTypeInfo().GetTypeId()))
					{
						m_signalInputValues[signalInputIdx] = pSignalInputValue->Clone();
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		CDocGraphNodeBase::Serialize(archive);

		if(SerializationContext::GetPass(archive) == ESerializationPass::PostLoad)
		{
			Refresh(SScriptRefreshParams(EScriptRefreshReason::Other)); // Ensure inputs exist before reading values. There should be a cleaner way to do this!
		}

		const EDocGraphSendSignalNodeTarget prevTarget = m_target;
		archive(m_target, "target", "Target");

		for(size_t signalInputIdx = 0, signalInputCount = m_signalInputValues.size(); signalInputIdx < signalInputCount; ++ signalInputIdx)
		{
			if(m_signalInputValues[signalInputIdx])
			{
				tukk szSignalInputName = m_signalInputNames[signalInputIdx].c_str();
				archive(*m_signalInputValues[signalInputIdx], szSignalInputName, szSignalInputName);
			}
		}

		if(m_target != prevTarget)
		{
			Refresh(SScriptRefreshParams(EScriptRefreshReason::Other));
		}

		if(archive.isValidation())
		{
			const IScriptFile&   file = CDocGraphNodeBase::GetFile();
			const SGUID          refGUID = CDocGraphNodeBase::GetRefGUID();
			const IScriptSignal* pSignal = ScriptIncludeRecursionUtils::GetSignal(file, refGUID).second;
			if(!pSignal)
			{
				char stringBuffer[StringUtils::s_guidStringBufferSize] = "";
				archive.error(*this, "Unable to retrieve signal: guid = %s", StringUtils::SysGUIDToString(refGUID.sysGUID, stringBuffer));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::PreCompileSequence(IDocGraphSequencePreCompiler& preCompiler, size_t outputIdx) const {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::LinkSequence(IDocGraphSequenceLinker& linker, size_t outputIdx, const LibFunctionId& functionId) const {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::Compile(IDocGraphNodeCompiler& compiler, EDocGraphSequenceStep sequenceStep, size_t portIdx) const
	{
		switch(sequenceStep)
		{
		case EDocGraphSequenceStep::BeginInput:
			{
				CompileInputs(compiler);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphSendSignalNode::GetFirstParamInput() const
	{
		return m_target == EDocGraphSendSignalNodeTarget::Object ? EInput::Object + 1 : EInput::In + 1;
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSendSignalNode::CompileInputs(IDocGraphNodeCompiler& compiler) const
	{
		switch(m_target)
		{
		case EDocGraphSendSignalNodeTarget::Self:
			{
				compiler.GetObject();
				DocGraphNodeUtils::CopyInputsToStack(*this, GetFirstParamInput(), m_signalInputValues, 0, compiler);
				compiler.SendSignal(CDocGraphNodeBase::GetRefGUID());
				break;
			}
		case EDocGraphSendSignalNodeTarget::Object:
			{
				const size_t stackPos = compiler.FindInputOnStack(*this, EInput::Object);
				if(stackPos != INVALID_INDEX)
				{
					compiler.Copy(stackPos, INVALID_INDEX, MakeAny(ObjectId()));
				}
				else
				{
					compiler.Push(MakeAny(ObjectId()));	// #SchematycTODO : Can we perhaps grey out the node if this is not connected?
				}
				DocGraphNodeUtils::CopyInputsToStack(*this, GetFirstParamInput(), m_signalInputValues, 0, compiler);
				compiler.SendSignal(CDocGraphNodeBase::GetRefGUID());
				break;
			}
		case EDocGraphSendSignalNodeTarget::Broadcast:
			{
				DocGraphNodeUtils::CopyInputsToStack(*this, GetFirstParamInput(), m_signalInputValues, 0, compiler);
				compiler.BroadcastSignal(CDocGraphNodeBase::GetRefGUID());
				break;
			}
		}
	}
}
