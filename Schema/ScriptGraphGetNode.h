// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{
class CScriptGraphGetNode : public CScriptGraphNodeModel
{
private:

	struct EOutputIdx
	{
		enum : u32
		{
			Value = 0
		};
	};

	struct SRuntimeData
	{
		SRuntimeData(u32 _pos);
		SRuntimeData(const SRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SRuntimeData>& desc);

		u32       pos;
	};

	struct SComponentPropertyRuntimeData
	{
		static void ReflectType(CTypeDesc<SComponentPropertyRuntimeData>& desc) { desc.SetGUID("8A133E1C-6EF3-4110-859E-7853956045F2"_drx_guid); };

		u32 componentMemberIndex;
		u32 componentIdx;
	};

public:

	CScriptGraphGetNode();
	CScriptGraphGetNode(const DrxGUID& referenceGUID,u32 componentMemberId=0);

	// CScriptGraphNodeModel
	virtual void  Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	virtual DrxGUID GetTypeGUID() const override;
	virtual void  CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void  LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  RemapDependencies(IGUIDRemapper& guidRemapper) override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	static SRuntimeResult Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteArray(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);
	static SRuntimeResult ExecuteGetComponentProperty(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	DrxGUID m_referenceGUID;
	u32 m_componentMemberId = 0;
};
}
