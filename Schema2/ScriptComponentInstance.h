// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptFile.h>

#include <drx3D/Schema2/ScriptElementBase.h>

namespace sxema2
{
	class CSerializationContext;

	class CScriptComponentInstance : public CScriptElementBase<IScriptComponentInstance>
	{
	public:

		CScriptComponentInstance(IScriptFile& file);
		CScriptComponentInstance(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const SGUID& componentGUID, EScriptComponentInstanceFlags flags);

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

		// IScriptComponentInstance
		virtual SGUID GetComponentGUID() const override;
		virtual EScriptComponentInstanceFlags GetFlags() const override;
		virtual IPropertiesConstPtr GetProperties() const override;
		// ~IScriptComponentInstance

	private:

		void ApplyComponent();
		void PopulateSerializationContext(Serialization::IArchive& archive);

		SGUID                         m_guid;
		SGUID                         m_scopeGUID;
		string                        m_name;
		SGUID                         m_componentGUID;
		EScriptComponentInstanceFlags m_flags;
		IPropertiesPtr                m_pProperties;
	};

	DECLARE_SHARED_POINTERS(CScriptComponentInstance)
}
