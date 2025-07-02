// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptFile.h>

#include <drx3D/Schema2/ScriptElementBase.h>
#include <drx3D/Schema2/ScriptUserDocumentation.h>

namespace sxema2
{
	class CScriptClassBase : public CScriptElementBase<IScriptClassBase>
	{
	public:

		// #SchematycTODO : Create two separate constructors: one default (before loading) and one for when element is created in editor.
		CScriptClassBase(IScriptFile& file, const SGUID& guid = SGUID(), const SGUID& scopeGUID = SGUID(), const SGUID& refGUID = SGUID());

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

		// IScriptClass
		virtual SGUID GetRefGUID() const override;
		// ~IScriptClass

	private:

		void RefreshProperties(const IScriptFile& baseSchemaFile, const IScriptClass& baseClass);
		void Validate(Serialization::IArchive& archive);

		SGUID                    m_guid;
		SGUID                    m_scopeGUID;
		string                   m_name;
		SScriptUserDocumentation m_userDocumentation;
		SGUID                    m_refGUID;
	};

	DECLARE_SHARED_POINTERS(CScriptClassBase)
}
