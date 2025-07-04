// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_TypeUtils.h>

#include <drx3D/Schema2/IComponentFactory.h>
#include <drx3D/Schema2/Properties.h>
#include <drx3D/Schema2/IEnvRegistry.h>
#include <drx3D/Schema2/StringUtils.h>

#define SXEMA2_MAKE_COMPONENT_FACTORY_SHARED(component, properties, componentGUID) sxema2::IComponentFactoryPtr(std::make_shared<sxema2::CComponentFactory<component, properties> >(componentGUID, TemplateUtils::GetTypeName<component>(), __FILE__, "Code"))

namespace sxema2
{
	template <typename TYPE> struct SComponentPropertiesFactory
	{
		static inline IPropertiesPtr CreateProperties()
		{
			return Properties::MakeShared<TYPE>();
		}
	};

	template <> struct SComponentPropertiesFactory<void>
	{
		struct SEmptyProperties
		{
			void Serialize(Serialization::IArchive& archive) {}
		};

		static inline IPropertiesPtr CreateProperties()
		{
			return Properties::MakeShared<SEmptyProperties>();
		}
	};

	template <class COMPONENT, class PROPERTIES> class CComponentFactory : public IComponentFactory
	{
	public:

		inline CComponentFactory(const SGUID& componentGUID, tukk szDeclaration, tukk szFileName, tukk szProjectDir)
			: m_componentGUID(componentGUID)
			, m_flags(EComponentFlags::None)
		{
			StringUtils::SeparateTypeNameAndNamespace(szDeclaration, m_name, m_namespace);
			SetFileName(szFileName, szProjectDir);
		}

		// IComponentFactory

		virtual CTypeInfo GetTypeInfo() const override
		{
			return sxema2::GetTypeInfo<COMPONENT>();
		}

		virtual SGUID GetComponentGUID() const override
		{
			return m_componentGUID;
		}

		virtual void SetName(tukk szName) override
		{
			m_name = szName;
		}

		virtual tukk GetName() const override
		{
			return m_name.c_str();
		}

		virtual void SetNamespace(tukk szNamespace) override
		{
			m_namespace = szNamespace;
		}

		virtual tukk GetNamespace() const override
		{
			return m_namespace.c_str();
		}

		virtual void SetFileName(tukk szFileName, tukk szProjectDir) override
		{
			StringUtils::MakeProjectRelativeFileName(szFileName, szProjectDir, m_fileName);
		}

		virtual tukk GetFileName() const override
		{
			return m_fileName.c_str();
		}

		virtual void SetAuthor(tukk szAuthor) override
		{
			m_author = szAuthor;
		}

		virtual tukk GetAuthor() const override
		{
			return m_author.c_str();
		}

		virtual void SetDescription(tukk szDescription) override
		{
			m_description = szDescription;
		}

		virtual tukk GetDescription() const override
		{
			return m_description.c_str();
		}

		virtual void SetWikiLink(tukk szWikiLink) override
		{
			m_wikiLink = szWikiLink;
		}
		
		virtual tukk GetWikiLink() const override
		{
			return m_wikiLink.c_str();
		}

		virtual void AddDependency(EComponentDependencyType type, const SGUID& guid) override
		{
			m_dependencies.push_back(SDependency(type, guid));
		}

		virtual u32 GetDependencyCount() const override
		{
			return m_dependencies.size();
		}

		virtual EComponentDependencyType GetDependencyType(u32 dependencyIdx) const override
		{
			return dependencyIdx < m_dependencies.size() ? m_dependencies[dependencyIdx].type : EComponentDependencyType::None;
		}

		virtual SGUID GetDependencyGUID(u32 dependencyIdx) const override
		{
			return dependencyIdx < m_dependencies.size() ? m_dependencies[dependencyIdx].guid : SGUID();
		}

		virtual void SetAttachmentType(EComponentSocket socket, const SGUID& attachmentTypeGUID) override
		{
			if(socket < EComponentSocket::Count)
			{
				m_attachmentTypeGUIDs[static_cast<std::underlying_type<EComponentSocket>::type>(socket)] = attachmentTypeGUID;
			}
		}

		virtual SGUID GetAttachmentType(EComponentSocket socket) const override
		{
			return socket < EComponentSocket::Count ? m_attachmentTypeGUIDs[static_cast<std::underlying_type<EComponentSocket>::type>(socket)] : SGUID();
		}

		virtual void SetFlags(EComponentFlags flags) override
		{
			m_flags = flags;
		}

		virtual EComponentFlags GetFlags() const override
		{
			return m_flags;
		}

		virtual IComponentPtr CreateComponent() const override
		{
			return IComponentPtr(new COMPONENT());
		}

		virtual CTypeInfo GetPropertiesTypeInfo() const override
		{
			return sxema2::GetTypeInfo<PROPERTIES>();
		}

		virtual IPropertiesPtr CreateProperties() const override
		{
			return SComponentPropertiesFactory<PROPERTIES>::CreateProperties();
		}

		virtual void SetDefaultNetworkSpawnParams(const INetworkSpawnParamsPtr& pNetworkSpawnParams) override
		{
			m_pDefaultNetworkParams = pNetworkSpawnParams;
		}

		virtual INetworkSpawnParamsPtr CreateNetworkSpawnParams() const override
		{
			return m_pDefaultNetworkParams ? m_pDefaultNetworkParams->Clone() : INetworkSpawnParamsPtr();
		}

		// ~IComponentFactory

	private:

		struct SDependency
		{
			inline SDependency(EComponentDependencyType _type, const SGUID& _guid)
				: type(_type)
				, guid(_guid)
			{}

			EComponentDependencyType type;
			SGUID                    guid;
		};

		typedef std::vector<SDependency> DependencyVector;

		SGUID                  m_componentGUID;
		string                 m_name;
		string                 m_namespace;
		string                 m_fileName;
		string                 m_author;
		string                 m_description;
		string                 m_wikiLink;
		DependencyVector       m_dependencies;
		SGUID                  m_attachmentTypeGUIDs[static_cast<std::underlying_type<EComponentSocket>::type>(EComponentSocket::Count)];
		string                 m_socketType;
		EComponentFlags        m_flags;
		INetworkSpawnParamsPtr m_pDefaultNetworkParams;
	};
}
