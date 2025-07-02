// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{
class CScriptGraphSetNode : public CScriptGraphNodeModel
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
			Out = 0
		};
	};

	struct SRuntimeData
	{
		SRuntimeData(u32 _pos);
		SRuntimeData(const SRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SRuntimeData>& desc);

		u32       pos;
	};

public:

	CScriptGraphSetNode();
	CScriptGraphSetNode(const DrxGUID& referenceGUID);

	// CScriptGraphNodeModel
	virtual DrxGUID GetTypeGUID() const override;
	virtual void  CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void  Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	virtual void  LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  RemapDependencies(IGUIDRemapper& guidRemapper) override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	static SRuntimeResult Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	
public:

	static const DrxGUID ms_typeGUID;

private:

	DrxGUID m_referenceGUID;
};
}
