// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptGraphBeginNode.h>

#include <drx3D/Schema2/IDomainContext.h>
#include <drx3D/Schema2/IScriptGraphNodeCompiler.h>
#include <drx3D/Schema2/IScriptGraphNodeCreationMenu.h>

#include <drx3D/Schema2/ScriptGraphNodeFactory.h>

namespace sxema2
{
	const SGUID CScriptGraphBeginNode::s_typeGUID = "12bdfa06-ba95-4e48-bb2d-bb48a7080abc";

	CScriptGraphBeginNode::CScriptGraphBeginNode() {}

	CScriptGraphBeginNode::CScriptGraphBeginNode(const SGUID& guid)
		: CScriptGraphNodeBase(guid)
	{}

	CScriptGraphBeginNode::CScriptGraphBeginNode(const SGUID& guid, const Vec2& pos)
		: CScriptGraphNodeBase(guid, pos)
	{}

	SGUID CScriptGraphBeginNode::GetTypeGUID() const
	{
		return s_typeGUID;
	}

	EScriptGraphColor CScriptGraphBeginNode::GetColor() const
	{
		return EScriptGraphColor::Green;
	}

	void CScriptGraphBeginNode::Refresh(const SScriptRefreshParams& params)
	{
		LOADING_TIME_PROFILE_SECTION;

		CScriptGraphNodeBase::Refresh(params);

		CScriptGraphNodeBase::SetName("Begin");
		CScriptGraphNodeBase::AddOutput(EScriptGraphPortFlags::BeginSequence | EScriptGraphPortFlags::Execute, "Out");
	}

	void CScriptGraphBeginNode::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;

		CScriptGraphNodeBase::Serialize(archive);
	}

	void CScriptGraphBeginNode::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		CScriptGraphNodeBase::RemapGUIDs(guidRemapper);
	}

	void CScriptGraphBeginNode::Compile_New(IScriptGraphNodeCompiler& compiler) const
	{
		compiler.BindCallback(&Execute);
	}

	void CScriptGraphBeginNode::RegisterCreator(CScriptGraphNodeFactory& factory)
	{
		class CCreator : public IScriptGraphNodeCreator
		{
		public:

			// IScriptGraphNodeCreator

			virtual SGUID GetTypeGUID() const override
			{
				return CScriptGraphBeginNode::s_typeGUID;
			}

			virtual IScriptGraphNodePtr CreateNode() override
			{
				return std::make_shared<CScriptGraphBeginNode>(gEnv->pSchematyc2->CreateGUID());
			}

			virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IDomainContext& domainContext, const IScriptGraphExtension& graph) override {}

			// ~IScriptGraphNodeCreator
		};

		factory.RegisterCreator(std::make_shared<CCreator>());
	}

	SRuntimeResult CScriptGraphBeginNode::Execute(IObject* pObject, const SRuntimeActivationParams& activationParams, CRuntimeNodeData& data)
	{
		return SRuntimeResult(ERuntimeStatus::Continue, EOutputIdx::Out);
	}
}
