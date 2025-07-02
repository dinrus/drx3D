// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptInterfaceImpl.h>
#include <drx3D/Schema/MultiPassSerializer.h>

#include <drx3D/Schema/ScriptElementBase.h>

namespace sxema
{
// Forward declare interfaces.
struct IEnvInterface;

class CScriptInterfaceImpl : public CScriptElementBase<IScriptInterfaceImpl>, public CMultiPassSerializer
{
public:

	CScriptInterfaceImpl();
	CScriptInterfaceImpl(const DrxGUID& guid, EDomain domain, const DrxGUID& refGUID);

	// IScriptElement
	virtual void EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const override;
	virtual void RemapDependencies(IGUIDRemapper& guidRemapper) override;
	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual void Serialize(Serialization::IArchive& archive) override;
	// ~IScriptElement

	// IScriptInterfaceImpl
	virtual EDomain GetDomain() const override;
	virtual DrxGUID   GetRefGUID() const override;
	// ~IScriptInterfaceImpl

protected:

	// CMultiPassSerializer
	virtual void LoadDependencies(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Save(Serialization::IArchive& archive, const ISerializationContext& context) override;
	virtual void Validate(Serialization::IArchive& archive, const ISerializationContext& context) override;
	// ~CMultiPassSerializer

private:

	void RefreshEnvInterfaceFunctions(const IEnvInterface& envInterface);
	//void RefreshScriptInterfaceFunctions(const IScriptFile& interfaceFile);
	//void RefreshScriptInterfaceTasks(const IScriptFile& iInterfaceFile);
	//void RefreshScriptInterfaceTaskPropertiess(const IScriptFile& interfaceFile, const DrxGUID& taskGUID);

private:

	// #SchematycTODO : Replace m_domain and m_refGUID with SElementId!!!
	EDomain m_domain;
	DrxGUID   m_refGUID;
};
} // sxema
