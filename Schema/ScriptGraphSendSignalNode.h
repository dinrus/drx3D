// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{

class CScriptGraphSendSignalNode : public CScriptGraphNodeModel
{
public:

	enum class ETarget
	{
		Self = 0,
		Object,
		Broadcast,
		Entity
	};

private:

	struct EOutputIdx
	{
		enum : u32
		{
			Out = 0
		};
	};

	struct SRuntimeData
	{
		SRuntimeData(const DrxGUID& _signalGUID);
		SRuntimeData(const SRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SRuntimeData>& desc);

		DrxGUID       signalGUID;
	};

public:

	CScriptGraphSendSignalNode();
	CScriptGraphSendSignalNode(const DrxGUID& signalGUID);

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

	static SRuntimeResult ExecuteSendToSelf(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteSendToObject(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteSendToEntity(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteBroadcast(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	DrxGUID   m_signalGUID;
	ETarget m_target;
};

} // sxema
