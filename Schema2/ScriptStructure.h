// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptFile.h>

#include <drx3D/Schema2/ScriptElementBase.h>
#include <drx3D/Schema2/ScriptUserDocumentation.h>
#include <drx3D/Schema2/ScriptVariableDeclaration.h>

namespace sxema2
{
	class CScriptStructure : public CScriptElementBase<IScriptStructure>
	{
	private:

		typedef std::vector<CScriptVariableDeclaration> Fields;

	public:

		// #SchematycTODO : Create two separate constructors: one default (before loading) and one for when element is created in editor.
		CScriptStructure(IScriptFile& file, const SGUID& guid = SGUID(), const SGUID& scopeGUID = SGUID(), tukk szName = nullptr);

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

		// IScriptStructure
		virtual CAggregateTypeId GetTypeId() const override;
		virtual size_t GetFieldCount() const override;
		virtual tukk GetFieldName(size_t fieldIdx) const override;
		virtual IAnyConstPtr GetFieldValue(size_t fieldIdx) const override;
		// ~IScriptStructure

	private:

		SGUID                    m_guid;
		SGUID                    m_scopeGUID;
		string                   m_name;
		Fields                   m_fields;
		SScriptUserDocumentation m_userDocumentation;
	};

	DECLARE_SHARED_POINTERS(CScriptStructure)
}
