// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptExtension.h>

namespace sxema
{
	DECLARE_SHARED_POINTERS(IScriptExtension)

	class CScriptExtensionMap : public IScriptExtensionMap
	{
	public:
		
		void AddExtension(const IScriptExtensionPtr& pExtension);

		// IScriptExtensionMap
		virtual IScriptExtension* QueryExtension(EScriptExtensionType type) override;
		virtual const IScriptExtension* QueryExtension(EScriptExtensionType type) const override;
		virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
		virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
		virtual void ProcessEvent(const SScriptEvent& event) override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		// ~IScriptExtensionMap

	private:

		typedef std::vector<IScriptExtensionPtr> Extensions;

		Extensions m_extensions;
	};
}