// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_TypeUtils.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/IActionFactory.h>
#include <drx3D/Schema2/Properties.h>
#include <drx3D/Schema2/IEnvRegistry.h>
#include <drx3D/Schema2/StringUtils.h>

#define SXEMA2_MAKE_ACTION_FACTORY_SHARED(action, properties, actionGUID)                          sxema2::IActionFactoryPtr(std::make_shared<sxema2::CActionFactory<action, properties> >(actionGUID, sxema2::SGUID(), TemplateUtils::GetTypeName<action>(), __FILE__, "Code"))
#define SXEMA2_MAKE_COMPONENT_ACTION_FACTORY_SHARED(action, properties, actionGUID, componentGUID) sxema2::IActionFactoryPtr(std::make_shared<sxema2::CActionFactory<action, properties> >(actionGUID, componentGUID, TemplateUtils::GetTypeName<action>(), __FILE__, "Code"))

namespace sxema2
{
	template <typename TYPE> struct SActionPropertiesFactory
	{
		static inline IPropertiesPtr CreateProperties()
		{
			return Properties::MakeShared<TYPE>();
		}
	};

	template <> struct SActionPropertiesFactory<void>
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

	template <class ACTION, class PROPERTIES> class CActionFactory : public IActionFactory
	{
	public:

		inline CActionFactory(const SGUID& actionGUID, const SGUID& componentGUID, tukk szDeclaration, tukk szFileName, tukk szProjectDir)
			: m_actionGUID(actionGUID)
			, m_componentGUID(componentGUID)
			, m_flags(EActionFlags::None)
		{
			StringUtils::SeparateTypeNameAndNamespace(szDeclaration, m_name, m_namespace);
			SetFileName(szFileName, szProjectDir);
		}

		// IActionFactory

		virtual SGUID GetActionGUID() const override
		{
			return m_actionGUID;
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

		virtual void SetFlags(EActionFlags flags) override
		{
			m_flags = flags;
		}

		virtual EActionFlags GetFlags() const override
		{
			return m_flags;
		}

		virtual IActionPtr CreateAction() const override
		{
			return IActionPtr(std::make_shared<ACTION>());
		}

		virtual IPropertiesPtr CreateProperties() const override
		{
			return SActionPropertiesFactory<PROPERTIES>::CreateProperties();
		}

		// ~IActionFactory

	private:

		SGUID        m_actionGUID;
		SGUID        m_componentGUID;
		string       m_name;
		string       m_namespace;
		string       m_fileName;
		string       m_author;
		string       m_description;
		string       m_wikiLink;
		EActionFlags m_flags;
	};
}
