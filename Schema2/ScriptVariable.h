// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IScriptFile.h>

#include <drx3D/Schema2/ScriptElementBase.h>
#include <drx3D/Schema2/ScriptVariableDeclaration.h>

namespace sxema2
{
	class CScriptVariable : public CScriptElementBase<IScriptVariable>
	{
	public:

		CScriptVariable(IScriptFile& file);
		CScriptVariable(IScriptFile& file, const SGUID& guid, const SGUID& scopeGUID, tukk szName, const CAggregateTypeId& typeId);

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

		// IScriptVariable
		virtual CAggregateTypeId GetTypeId() const override;
		virtual IAnyConstPtr GetValue() const override;
		// ~IScriptVariable

	private:

		SGUID                      m_guid;
		SGUID                      m_scopeGUID;
		CScriptVariableDeclaration m_declaration;
	};

	DECLARE_SHARED_POINTERS(CScriptVariable)
}
