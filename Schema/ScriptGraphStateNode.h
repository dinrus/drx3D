// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/SerializationQuickSearch.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{
class CScriptGraphStateNode : public CScriptGraphNodeModel
{
public:

	enum class EOutputType
	{
		Unknown,
		EnvSignal,
		ScriptSignal,
		ScriptTimer
	};

private:

	struct EInputIdx
	{
		enum : u32
		{
			In = 0,
			Value
		};
	};

	struct SOutputParams // #SchematycTODO : Rename STransitionParams?
	{
		SOutputParams();
		SOutputParams(EOutputType _type, const DrxGUID& _guid);

		void Serialize(Serialization::IArchive& archive);

		bool operator==(const SOutputParams& rhs) const;

		EOutputType type;
		DrxGUID       guid;
	};

	typedef SerializationUtils::SQuickSearchTypeWrapper<SOutputParams> Output;
	typedef std::vector<Output>                                        Outputs;

	struct SRuntimeData
	{
		SRuntimeData(u32 _stateIdx);
		SRuntimeData(const SRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SRuntimeData>& desc);

		u32       stateIdx;
	};

public:

	CScriptGraphStateNode();
	CScriptGraphStateNode(const DrxGUID& stateGUID);

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

	void                  Edit(Serialization::IArchive& archive);
	void                  Validate(Serialization::IArchive& archive);

	static SRuntimeResult Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	DrxGUID   m_stateGUID;
	Outputs m_outputs;
};
}
