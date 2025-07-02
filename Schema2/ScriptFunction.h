// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptFunction.h>

#include <drx3D/Schema2/ScriptElementBase.h>
#include <drx3D/Schema2/ScriptUserDocumentation.h>
#include <drx3D/Schema2/ScriptVariableDeclaration.h>

namespace sxema2
{
	class CScriptFunction : public CScriptElementBase<IScriptFunction>
	{
	private:

		struct SInput
		{
			void Serialize(Serialization::IArchive& archive);

			SGUID                      guid;
			CScriptVariableDeclaration declaration;
		};

		typedef std::vector<SInput> Inputs;

		struct SOutput
		{
			void Serialize(Serialization::IArchive& archive);

			SGUID                      guid;
			CScriptVariableDeclaration declaration;
		};

		typedef std::vector<SOutput> Outputs;

	public:

		CScriptFunction(IScriptFile& file);
		CScriptFunction(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName);

		// IScriptElement
		virtual EAccessor GetAccessor() const override;
		virtual SGUID GetGUID() const override;
		virtual SGUID GetScopeGUID() const override;
		virtual bool SetName(tukk szName) override;
		virtual tukk GetName() const override;
		virtual void EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const override;
		virtual void Refresh(const SScriptRefreshParams& params) override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		virtual void RemapGUIDs(IGUIDRemapper& guidRemapper) override;
		// ~IScriptElement

	private:

		SGUID                    m_guid;
		SGUID                    m_scopeGUID;
		string                   m_name;
		Inputs                   m_inputs;
		Outputs                  m_outputs;
		SScriptUserDocumentation m_userDocumentation;
	};

	DECLARE_SHARED_POINTERS(CScriptFunction)
}
