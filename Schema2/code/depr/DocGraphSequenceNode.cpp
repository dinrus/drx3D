// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>

#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphSequenceNode.h>

#include <drx3D/Schema2/ICompiler.h>
#include <drx3D/Schema2/LibUtils.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>
#include <drx3D/Schema2/Deprecated/IGlobalFunction.h>

#include <drx3D/Schema2/ScriptVariableDeclaration.h>
#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphNodeBase.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CDocGraphSequenceNode::CDocGraphSequenceNode(IScriptFile& file, IDocGraph& graph, const SGUID& guid, const SGUID& contextGUID, const SGUID& refGUID, Vec2 pos)
		: CDocGraphNodeBase(file, graph, guid, "Sequence", EScriptGraphNodeType::Sequence, contextGUID, refGUID, pos)
	{
		Refresh(SScriptRefreshParams(EScriptRefreshReason::Other));
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CDocGraphSequenceNode::GetCustomOutputDefault() const
	{
		return MakeAnyShared(CPoolString());
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::EnumerateOptionalOutputs(const ScriptGraphNodeOptionalOutputEnumerator& enumerator) {}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphSequenceNode::AddCustomOutput(const IAny& value)
	{
		CPoolString	sequenceName;
		if(value.Get(sequenceName))
		{
			m_sequenceNames.push_back(sequenceName);
			return CDocGraphNodeBase::AddOutput(sequenceName.c_str(), EScriptGraphPortFlags::Removable | EScriptGraphPortFlags::Execute);
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphSequenceNode::AddOptionalOutput(tukk szName, EScriptGraphPortFlags flags, const CAggregateTypeId& typeId)
	{
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::RemoveOutput(size_t outputIdx)
	{
		CDocGraphNodeBase::RemoveOutput(outputIdx);
		m_sequenceNames.erase(m_sequenceNames.begin() + outputIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::Refresh(const SScriptRefreshParams& params)
	{
		LOADING_TIME_PROFILE_SECTION;
		CDocGraphNodeBase::Refresh(params);
		CDocGraphNodeBase::AddInput("In", EScriptGraphPortFlags::MultiLink | EScriptGraphPortFlags::Execute);
		for(PoolStringVector::iterator itSequenceName = m_sequenceNames.begin(), itEndSequenceName = m_sequenceNames.end(); itSequenceName != itEndSequenceName; ++ itSequenceName)
		{
			CDocGraphNodeBase::AddOutput(itSequenceName->c_str(), EScriptGraphPortFlags::Removable | EScriptGraphPortFlags::Execute);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		CDocGraphNodeBase::Serialize(archive);
		archive(m_sequenceNames, "m_sequenceNames", "Sequences");
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::PreCompileSequence(IDocGraphSequencePreCompiler& preCompiler, size_t outputIdx) const {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::LinkSequence(IDocGraphSequenceLinker& linker, size_t outputIdx, const LibFunctionId& functionId) const {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::Compile(IDocGraphNodeCompiler& compiler, EDocGraphSequenceStep sequenceStep, size_t portIdx) const
	{
		switch(sequenceStep)
		{
		case EDocGraphSequenceStep::BeginOutput:
			{
				CompileSequenceBegin(compiler, portIdx);
				break;
			}
		case EDocGraphSequenceStep::EndOutput:
			{
				CompileSequenceEnd(compiler, portIdx);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::CompileSequenceBegin(IDocGraphNodeCompiler& compiler, size_t portIdx) const
	{
		compiler.CreateStackFrame(*this, portIdx);
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphSequenceNode::CompileSequenceEnd(IDocGraphNodeCompiler& compiler, size_t portIdx) const
	{
		compiler.CollapseStackFrame(*this, portIdx);
	}
}
