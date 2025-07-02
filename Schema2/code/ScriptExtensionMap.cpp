// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptExtensionMap.h>

#include <drx3D/Schema2/ILog.h>

SERIALIZATION_ENUM_BEGIN_NESTED(sxema2, EScriptExtensionId, "sxema Script Extension Id")
	SERIALIZATION_ENUM(sxema2::EScriptExtensionId::Graph, "Graph", "Graph")
SERIALIZATION_ENUM_END()

namespace sxema2
{
	void CScriptExtensionMap::AddExtension(const IScriptExtensionPtr& pExtension)
	{
		if(pExtension)
		{
			m_extensions.push_back(pExtension);
		}
	}

	IScriptExtension* CScriptExtensionMap::QueryExtension(EScriptExtensionId id)
	{
		for(Extensions::value_type& pExtension : m_extensions)
		{
			if(pExtension->GetId_New() == id)
			{
				return pExtension.get();
			}
		}
		return nullptr;
	}

	const IScriptExtension* CScriptExtensionMap::QueryExtension(EScriptExtensionId id) const
	{
		for(const Extensions::value_type& pExtension : m_extensions)
		{
			if(pExtension->GetId_New() == id)
			{
				return pExtension.get();
			}
		}
		return nullptr;
	}

	void CScriptExtensionMap::Refresh(const SScriptRefreshParams& params)
	{
		for(Extensions::value_type& pExtension : m_extensions)
		{
			pExtension->Refresh_New(params);
		}
	}

	void CScriptExtensionMap::Serialize(Serialization::IArchive& archive)
	{
		class CExtensionWrapper
		{
		public:

			inline CExtensionWrapper(IScriptExtension& extension)
				: m_extension(extension)
			{}

			void Serialize(Serialization::IArchive& archive)
			{
				m_extension.Serialize_New(archive);
			}

		private:

			IScriptExtension& m_extension;
		};

		const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<EScriptExtensionId>();
		for(Extensions::value_type& pExtension : m_extensions)
		{
			tukk szName = enumDescription.name(pExtension->GetId_New());
			SXEMA2_SYSTEM_ASSERT(szName && szName[0]);
			//archive(*pExtension, szName);
			archive(CExtensionWrapper(*pExtension), szName);
		}
	}

	void CScriptExtensionMap::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		for(Extensions::value_type& pExtension : m_extensions)
		{
			pExtension->RemapGUIDs_New(guidRemapper);
		}
	}
}
