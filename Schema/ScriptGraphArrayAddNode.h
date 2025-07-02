// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptVariableData.h>
#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{

// Forward declare classes.
class CAnyValue;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(CAnyValue)

class CScriptGraphArrayAddNode : public CScriptGraphNodeModel
{
private:

	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			Array,
			Value
		};
	};

	struct EOutputIdx
	{
		enum : u32
		{
			Out = 0
		};
	};

public:

	CScriptGraphArrayAddNode(const SElementId& typeId = SElementId());

	// CScriptGraphNodeModel
	virtual DrxGUID GetTypeGUID() const override;
	virtual void  CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void  Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	virtual void  LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  RemapDependencies(IGUIDRemapper& guidRemapper) override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	static SRuntimeResult Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	CScriptVariableData m_defaultValue;
};

} // sxema
