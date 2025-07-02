// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphContainerFindByValueNode.h>

#include <drx3D/Schema2/ICompiler.h>
#include <drx3D/Schema2/LibUtils.h>
#include <drx3D/Schema2/Deprecated/DocUtils.h>

#include <drx3D/Schema2/Deprecated/DocGraphNodes/DocGraphNodeBase.h>
#include <drx3D/Schema2/ScriptVariableDeclaration.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CDocGraphContainerFindByValueNode::CDocGraphContainerFindByValueNode(IScriptFile& file, IDocGraph& graph, const SGUID& guid, const SGUID& contextGUID, const SGUID& refGUID, Vec2 pos)
		: CDocGraphNodeBase(file, graph, guid, "", EScriptGraphNodeType::ContainerFindByValue, contextGUID, refGUID, pos)
		, m_pValue()
	{
		Refresh(SScriptRefreshParams(EScriptRefreshReason::Other)); // #SchematycTODO : Remove once we've moved over to unified load/save?
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CDocGraphContainerFindByValueNode::GetCustomOutputDefault() const
	{
		return IAnyConstPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphContainerFindByValueNode::AddCustomOutput(const IAny& value)
	{
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::EnumerateOptionalOutputs(const ScriptGraphNodeOptionalOutputEnumerator& enumerator) {}

	//////////////////////////////////////////////////////////////////////////
	size_t CDocGraphContainerFindByValueNode::AddOptionalOutput(tukk szName, EScriptGraphPortFlags flags, const CAggregateTypeId& typeId)
	{
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::RemoveOutput(size_t outputIdx) {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::Refresh(const SScriptRefreshParams& params)
	{
		LOADING_TIME_PROFILE_SECTION;

		CDocGraphNodeBase::Refresh(params);
		CDocGraphNodeBase::AddInput("In", EScriptGraphPortFlags::MultiLink | EScriptGraphPortFlags::Execute);
		CDocGraphNodeBase::AddOutput("True", EScriptGraphPortFlags::Execute);
		CDocGraphNodeBase::AddOutput("False", EScriptGraphPortFlags::Execute);
		CDocGraphNodeBase::AddOutput("Index", EScriptGraphPortFlags::MultiLink, GetAggregateTypeId<i32>());

		const IScriptContainer* pContainer = CDocGraphNodeBase::GetFile().GetContainer(CDocGraphNodeBase::GetRefGUID());
		if (pContainer)
		{
			stack_string name = "Container Find By Value: ";
			name.append(pContainer->GetName());
			CDocGraphNodeBase::SetName(name.c_str());

			const IEnvTypeDesc* pTypeDesc = gEnv->pSchematyc2->GetEnvRegistry().GetTypeDesc(pContainer->GetTypeGUID());
			if (pTypeDesc)
			{
				CDocGraphNodeBase::AddInput("Value", EScriptGraphPortFlags::None, pTypeDesc->GetTypeInfo().GetTypeId());
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		CDocGraphNodeBase::Serialize(archive);

		if (!m_pValue) // #SchematycTODO : Move to separate function?
		{
			const IScriptContainer* pContainer = CDocGraphNodeBase::GetFile().GetContainer(CDocGraphNodeBase::GetRefGUID());
			if (pContainer)
			{
				const IEnvTypeDesc* pTypeDesc = gEnv->pSchematyc2->GetEnvRegistry().GetTypeDesc(pContainer->GetTypeGUID());
				if (pTypeDesc)
				{
					m_pValue = pTypeDesc->Create();
				}
			}
		}

		if (m_pValue)
		{
			archive(*m_pValue, "Value", "Value");
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::PreCompileSequence(IDocGraphSequencePreCompiler& preCompiler, size_t outputIdx) const {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::LinkSequence(IDocGraphSequenceLinker& linker, size_t outputIdx, const LibFunctionId& functionId) const {}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::Compile(IDocGraphNodeCompiler& compiler, EDocGraphSequenceStep sequenceStep, size_t portIdx) const
	{
		switch (sequenceStep)
		{
		case EDocGraphSequenceStep::BeginInput:
			{
				CompileInputs(compiler);
				break;
			}
		case EDocGraphSequenceStep::BeginOutput:
			{
				switch (portIdx)
				{
				case EOutput::True:
					{
						CompileTrue(compiler);
						break;
					}
				case EOutput::False:
					{
						CompileFalse(compiler);
						break;
					}
				}
				break;
			}
		case EDocGraphSequenceStep::EndInput:
			{
				CompileEnd(compiler);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::CompileInputs(IDocGraphNodeCompiler& compiler) const
	{
		compiler.CreateStackFrame(*this, EStackFrame::Body);

		DRX_ASSERT(m_pValue);
		if (m_pValue)
		{
			const size_t valueStackPos = compiler.FindInputOnStack(*this, EInput::Value);

			if (valueStackPos != INVALID_INDEX)
			{
				compiler.Copy(valueStackPos, INVALID_INDEX, *m_pValue);
			}
			else
			{
				compiler.Push(*m_pValue);
			}

			compiler.AddOutputToStack(*this, EOutput::Index, MakeAny<i32>());
			compiler.ContainerFindByValue(CDocGraphNodeBase::GetRefGUID(), *m_pValue);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::CompileTrue(IDocGraphNodeCompiler& compiler) const
	{
		compiler.BranchIfZero(*this, EMarker::False);
		//compiler.Pop(1);
		compiler.CreateStackFrame(*this, EStackFrame::True);
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::CompileFalse(IDocGraphNodeCompiler& compiler) const
	{
		compiler.CollapseStackFrame(*this, EStackFrame::True);
		compiler.Branch(*this, EMarker::End);
		compiler.CreateMarker(*this, EMarker::False);
		//compiler.Pop(1);
		compiler.CreateStackFrame(*this, EStackFrame::False);
	}

	//////////////////////////////////////////////////////////////////////////
	void CDocGraphContainerFindByValueNode::CompileEnd(IDocGraphNodeCompiler& compiler) const
	{
		compiler.CollapseStackFrame(*this, EStackFrame::False);
		compiler.CreateMarker(*this, EMarker::End);
		compiler.CollapseStackFrame(*this, EStackFrame::Body);
	}
}
