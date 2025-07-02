// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IFoundation.h>
#include <drx3D/Schema2/IScriptFile.h>

#include <drx3D/Schema2/ScriptElementBase.h>
#include <drx3D/Schema2/ScriptUserDocumentation.h>

namespace sxema2
{
	class CScriptClass : public CScriptElementBase<IScriptClass>
	{
	public:

		// #SchematycTODO : Create two separate constructors: one default (before loading) and one for when element is created in editor.
		CScriptClass(IScriptFile& file, const SGUID& guid = SGUID(), const SGUID& scopeGUID = SGUID(), tukk szName = nullptr, const SGUID& foundationGUID = SGUID());

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
		virtual tukk GetAuthor() const override;
		virtual tukk GetDescription() const override;
		virtual SGUID GetFoundationGUID() const override;
		virtual IPropertiesConstPtr GetFoundationProperties() const override;
		// ~IScriptClass

	private:

		void RefreshFoundationProperties();
		void RefreshFoundationComponents();
		void Validate(Serialization::IArchive& archive);

		SGUID                    m_guid;
		SGUID                    m_scopeGUID;
		string                   m_name;
		SScriptUserDocumentation m_userDocumentation;
		SGUID                    m_foundationGUID;
		IPropertiesPtr           m_pFoundationProperties;
	};

	DECLARE_SHARED_POINTERS(CScriptClass)
}
