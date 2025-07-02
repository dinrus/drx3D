// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{
class CScriptGraphBranchNode : public CScriptGraphNodeModel
{
private:

	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			Value
		};
	};

	struct EOutputIdx
	{
		enum : u32
		{
			True = 0,
			False
		};
	};

public:

	CScriptGraphBranchNode();

	// CScriptGraphNodeModel
	virtual DrxGUID GetTypeGUID() const override;
	virtual void  CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void  Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	static SRuntimeResult Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;
};
}
