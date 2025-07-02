// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{

// Forward declare interfaces.
struct IEnvFunction;
struct IScriptFunction;

class CScriptGraphFunctionNode : public CScriptGraphNodeModel
{
public:

	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			FirstParam
		};
	};

	struct EOutputIdx
	{
		enum : u32
		{
			Out = 0,
			FirstParam
		};
	};

	struct SEnvGlobalFunctionRuntimeData
	{
		SEnvGlobalFunctionRuntimeData(const IEnvFunction* _pEnvFunction);
		SEnvGlobalFunctionRuntimeData(const SEnvGlobalFunctionRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SEnvGlobalFunctionRuntimeData>& desc);

		const IEnvFunction* pEnvFunction;
	};

	struct SEnvComponentFunctionRuntimeData
	{
		SEnvComponentFunctionRuntimeData(const IEnvFunction* _pEnvFunction, u32 _componentIdx);
		SEnvComponentFunctionRuntimeData(const SEnvComponentFunctionRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SEnvComponentFunctionRuntimeData>& desc);

		const IEnvFunction* pEnvFunction;
		u32              componentIdx;
	};

	struct SScriptFunctionRuntimeData
	{
		SScriptFunctionRuntimeData(u32 _functionIdx);
		SScriptFunctionRuntimeData(const SScriptFunctionRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SScriptFunctionRuntimeData>& desc);

		u32      functionIdx;
	};

public:

	CScriptGraphFunctionNode();
	CScriptGraphFunctionNode(const SElementId& functionId, const DrxGUID& objectGUID);

	// CScriptGraphNodeModel
	virtual DrxGUID GetTypeGUID() const override;
	virtual void    CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void    Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	virtual void    LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void    Load(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void    Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void    Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void    Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void    RemapDependencies(IGUIDRemapper& guidRemapper) override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	void                  CreateInputsAndOutputs(CScriptGraphNodeLayout& layout, const IEnvFunction& envFunction);
	void                  CreateInputsAndOutputs(CScriptGraphNodeLayout& layout, const IScriptFunction& scriptFunction);

	void                  GoToFunction();

	static SRuntimeResult ExecuteEnvGlobalFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteEnvComponentFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteScriptFunction(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	SElementId m_functionId;
	DrxGUID    m_objectGUID;
};

} // sxema
