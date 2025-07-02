// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/ScriptGraphNodeBase.h>

#include <drx3D/Schema2/IRuntime.h>

namespace sxema2
{
	class CScriptGraphNodeFactory;

	class CScriptGraphCommentNode : public CScriptGraphNodeBase
	{
	public:
		static const SGUID s_typeGUID;

	public:
		CScriptGraphCommentNode(); // #SchematycTODO : Do we really need a default constructor?
		CScriptGraphCommentNode(const SGUID& guid);
		CScriptGraphCommentNode(const SGUID& guid, const Vec2& pos);

		// IScriptGraphNode
		virtual tukk GetComment() const override;
		virtual SGUID GetTypeGUID() const override;
		virtual EScriptGraphColor GetColor() const override;
		virtual void Refresh(const SScriptRefreshParams& params) override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		virtual void Compile_New(IScriptGraphNodeCompiler& compiler) const override;
		// ~IScriptGraphNode

		static void RegisterCreator(CScriptGraphNodeFactory& factory);

	private:
		string m_str;  // contents of the comment
	};
}
