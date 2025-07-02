// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/ScriptGraphBase.h>

namespace sxema2
{
	class CScriptPropertyGraph : public CScriptGraphBase
	{
	public:

		CScriptPropertyGraph(IScriptElement& element);

		// IScriptExtension
		virtual void Refresh_New(const SScriptRefreshParams& params) override;
		virtual void Serialize_New(Serialization::IArchive& archive) override;
		// ~IScriptExtension

		// IScriptGraph
		virtual EScriptGraphType GetType() const override;
		// ~IScriptGraph

	private:

		bool BeginNodeExists() const;
	};
}
