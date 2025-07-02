// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Entity/IEntityClass.h>

struct IEntityAttributesUprForSchematycBaseEnv;

namespace SchematycBaseEnv
{
	class CEntityClassRegistrar
	{
	public:
		struct SEntityClass
		{
			SEntityClass();
			SEntityClass(const SEntityClass& rhs);

			~SEntityClass();

			string                                 editorCategory;
			string                                 icon;
			sxema2::SGUID                       libClassGUID;
			IEntityClassRegistry::SEntityClassDesc desc;
		};


		void Init();
		void Refresh();

	private:

		typedef std::map<string, SEntityClass> EntityClasses;

		void RegisterEntityClass(const sxema2::ILibClass& libClass, SEntityClass& entityClass, bool bNewEntityClass);
		void OnClassRegistration(const sxema2::ILibClassConstPtr& pLibClass);

		EntityClasses                   m_entityClasses;
		TemplateUtils::CConnectionScope m_connectionScope;
	};
}