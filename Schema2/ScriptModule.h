// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptModule.h>

#include <drx3D/Schema2/ScriptElementBase.h>

namespace sxema2
{
	class CScriptModule : public CScriptElementBase<IScriptModule>
	{
	public:

		CScriptModule(IScriptFile& file);
		CScriptModule(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName);

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

		SGUID  m_guid;
		SGUID  m_scopeGUID;
		string m_name;
	};

	DECLARE_SHARED_POINTERS(CScriptModule)
}
