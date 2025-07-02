// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IFoundation.h>
#include <drx3D/Schema2/IEnvRegistry.h>

#include <drx3D/Schema2/Signal.h>

namespace sxema2
{
	typedef std::vector<string>	TStringVector;
	typedef std::vector<SGUID>	GUIDVector;

	// Foundation.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CFoundation : public IFoundation
	{
	public:

		CFoundation(const SGUID& guid, tukk name);

		// IFoundation
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual void SetDescription(tukk szDescription) override;
		virtual tukk GetDescription() const override;
		virtual void SetProperties(const IPropertiesPtr& pProperties) override;
		virtual IPropertiesConstPtr GetProperties() const override;
		virtual void UseNamespace(tukk szNamespace) override;
		virtual size_t GetNamespaceCount() const override;
		virtual tukk GetNamespace(size_t namespaceIdx) const override;
		virtual void AddAbstractInterface(const SGUID& guid) override;
		virtual size_t GetAbstractInterfaceCount() const override;
		virtual SGUID GetAbstractInterfaceGUID(size_t iAbstractInterface) const override;
		virtual void AddComponent(const SGUID& guid) override;
		virtual size_t GetComponentCount() const override;
		virtual SGUID GetComponentGUID(size_t iComponent) const override;
		// ~IFoundation

	protected:

		// IFoundation
		virtual bool AddExtension_Protected(const EnvTypeId& typeId, const IFoundationExtensionPtr& pExtension) override;
		virtual IFoundationExtensionPtr QueryExtension_Protected(const EnvTypeId& typeId) const override;
		// ~IFoundation

	private:

		typedef std::map<EnvTypeId, IFoundationExtensionPtr> TExtensionMap;

		SGUID          m_guid;
		string         m_name;
		string         m_description;
		IPropertiesPtr m_pProperties;
		TStringVector  m_namespaces;
		GUIDVector     m_abstractInterfaces;
		GUIDVector     m_components;
		TExtensionMap  m_extensions;
	};
}
