// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/GUID.h>

#include <drx3D/Schema/ScriptVariableData.h>
#include <drx3D/Schema/ScriptGraphNodeModel.h>

namespace sxema
{
class CScriptGraphFormatStringNode : public CScriptGraphNodeModel
{
private:

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
			Result
		};
	};

public: // #SchematycTODO :  Workaround for serialization of EElementForm!

	enum class EElementForm
	{
		Const,
		Input
	};

	struct SElement : public CMultiPassSerializer
	{
		SElement();

		// CMultiPassSerializer
		virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
		virtual void PostLoad(Serialization::IArchive& archive, const ISerializationContext& context) override;
		virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
		virtual void Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
		// ~CMultiPassSerializer

		bool IsValidInput() const;

		EElementForm        form;
		string              text;
		CScriptVariableData data;
	};

	typedef std::vector<SElement> Elements;

	DECLARE_SHARED_POINTERS(Elements)

	struct SRuntimeData
	{
		SRuntimeData(const ElementsPtr& _pElements);
		SRuntimeData(const SRuntimeData& rhs);

		static void ReflectType(CTypeDesc<SRuntimeData>& desc);

		ElementsPtr  pElements;
	};

public:

	CScriptGraphFormatStringNode();

	// CScriptGraphNodeModel
	virtual DrxGUID GetTypeGUID() const override;
	virtual void  CreateLayout(CScriptGraphNodeLayout& layout) override;
	virtual void  Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const override;
	virtual void  LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  PostLoad(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  Edit(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void  RemapDependencies(IGUIDRemapper& guidRemapper) override;
	// ~CScriptGraphNodeModel

	static void Register(CScriptGraphNodeFactory& factory);

private:

	void                  SerializeElements(Serialization::IArchive& archive);

	static SRuntimeResult Execute(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

public:

	static const DrxGUID ms_typeGUID;

private:

	Elements m_elements;
};
}
