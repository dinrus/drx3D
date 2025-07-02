// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{

class CScriptGraphReceiveSignalNode : public CScriptGraphNodeModel
{
private:

	struct EOutputIdx
	{
		enum : u32
		{
			Out = 0,
			FirstParam
		};
	};

public:

	CScriptGraphReceiveSignalNode();
	CScriptGraphReceiveSignalNode(const SElementId& signalId, const DrxGUID& objectGUID = DrxGUID());

	// CScriptGraphNodeModel
	virtual DrxGUID GetTypeGUID() const override;
	virtual void  CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void  Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	virtual void  LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  RemapDependencies(IGUIDRemapper& guidRemapper) override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	void                  GoToSignal();

	static SRuntimeResult ExecuteReceiveEnvSignal(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteReceiveScriptSignal(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	SElementId m_signalId;
	DrxGUID      m_objectGUID;
};

} // sxema
