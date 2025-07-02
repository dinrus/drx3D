// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptGraphCommentNode.h>

#include <drx3D/Schema2/IDomainContext.h>
#include <drx3D/Schema2/IScriptGraphNodeCompiler.h>
#include <drx3D/Schema2/IScriptGraphNodeCreationMenu.h>

#include <drx3D/Schema2/ScriptGraphNodeFactory.h>

namespace sxema2
{
	const SGUID CScriptGraphCommentNode::s_typeGUID = "1EA03A11-C06C-4A26-B611-563CB152E57F";

	CScriptGraphCommentNode::CScriptGraphCommentNode()
		: m_str()
	{
		SetName("Comment");
	}

	CScriptGraphCommentNode::CScriptGraphCommentNode(const SGUID& guid)
		: CScriptGraphNodeBase(guid)
		, m_str()
	{
		SetName("Comment");
	}

	CScriptGraphCommentNode::CScriptGraphCommentNode(const SGUID& guid, const Vec2& pos)
		: CScriptGraphNodeBase(guid, pos)
		, m_str()
	{
		SetName("Comment");
	}

	tukk CScriptGraphCommentNode::GetComment() const
	{
		return m_str.c_str();
	}

	SGUID CScriptGraphCommentNode::GetTypeGUID() const
	{
		return s_typeGUID;
	}

	EScriptGraphColor CScriptGraphCommentNode::GetColor() const
	{
		return EScriptGraphColor::White;
	}

	void CScriptGraphCommentNode::Refresh(const SScriptRefreshParams& params)
	{
		CScriptGraphNodeBase::Refresh(params);
		SetName("Comment");
	}

	void CScriptGraphCommentNode::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		CScriptGraphNodeBase::Serialize(archive);
		archive(m_str, "str", "Comment Contents");
	}

	void CScriptGraphCommentNode::Compile_New(IScriptGraphNodeCompiler& compiler) const
	{
	}

	void CScriptGraphCommentNode::RegisterCreator(CScriptGraphNodeFactory& factory)
	{
		struct SNodeCreationMenuCommand : public IScriptGraphNodeCreationMenuCommand
		{
			// IMenuCommand
			IScriptGraphNodePtr Execute(const Vec2& pos)
			{
				return std::make_shared<CScriptGraphCommentNode>(gEnv->pSchematyc2->CreateGUID(), pos);
			}
			// ~IMenuCommand
		};

		struct SCreator : public IScriptGraphNodeCreator
		{
			// IScriptGraphNodeCreator
			virtual SGUID GetTypeGUID() const override
			{
				return CScriptGraphCommentNode::s_typeGUID;
			}

			virtual IScriptGraphNodePtr CreateNode() override
			{
				return std::make_shared<CScriptGraphCommentNode>(gEnv->pSchematyc2->CreateGUID());
			}

			virtual void PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu, const IDomainContext& domainContext, const IScriptGraphExtension& graph) override
			{
				nodeCreationMenu.AddOption("Comment", "Add a simple comment", "", std::make_shared<SNodeCreationMenuCommand>());
			}
			// ~IScriptGraphNodeCreator
		};

		factory.RegisterCreator(std::make_shared<SCreator>());
	}
}
