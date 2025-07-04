// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IRuntime.h>

#include <drx3D/Schema2/ScriptGraphNodeBase.h>

namespace sxema2
{
	class CScriptGraphNodeFactory;

	class CScriptGraphSwitchNode : public CScriptGraphNodeBase
	{
	private:

		struct EInputIdx
		{
			enum : u32
			{
				Value = 0
			};
		};

		struct EOutputIdx
		{
			enum : u32
			{
				Default = 0,
				FirstCase
			};
		};

		struct EAttributeId
		{
			enum : u32
			{
				CaseCount,
				FirstCase
			};
		};

		struct SCase
		{
			SCase();
			SCase(const IAnyPtr& _pValue);

			void Serialize(Serialization::IArchive& archive);

			IAnyPtr pValue;
		};

		typedef std::vector<SCase>            Cases;
		typedef std::vector<CAggregateTypeId> TypeIds;

	public:

		CScriptGraphSwitchNode(const SGUID& guid);
		CScriptGraphSwitchNode(const SGUID& guid, const Vec2& pos);

		// IScriptGraphNode
		virtual SGUID GetTypeGUID() const override;
		virtual EScriptGraphColor GetColor() const override;
		virtual void Refresh(const SScriptRefreshParams& params) override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		virtual void RemapGUIDs(IGUIDRemapper& guidRemapper) override;
		virtual void Compile_New(IScriptGraphNodeCompiler& compiler) const override;
		// ~IScriptGraphNode

		static void RegisterCreator(CScriptGraphNodeFactory& factory);

	private:

		void CreateDefaultValue();
		void SerializeBasicInfo(Serialization::IArchive& archive);
		void SerializeCases(Serialization::IArchive& archive);
		void Edit(Serialization::IArchive& archive);
		void Validate(Serialization::IArchive& archive);

		static SRuntimeResult Execute(IObject* pObject, const SRuntimeActivationParams& activationParams, CRuntimeNodeData& data);

	public:

		static const SGUID s_typeGUID;

	private:

		CAggregateTypeId m_typeId;
		IAnyPtr          m_pDefaultValue;
		Cases            m_cases;
	};
}
