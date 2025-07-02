// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/GUID.h>
#include <drx3D/Schema2/INetworkSpawnParams.h>
#include <drx3D/Schema2/IProperties.h>

namespace sxema2
{
	struct IComponent;

	DECLARE_SHARED_POINTERS(IComponent)

	enum class EComponentDependencyType
	{
		None,
		Soft, // Dependency must be initialized before component.
		Hard  // Dependency must exist and be initialized before component.
	};

	enum class EComponentSocket : u32
	{
		Parent = 0, // Socket used to attach component to parent.
		Child,      // Socket used to attach children to component.
		Count
	};

	enum class EComponentFlags
	{
		None                = 0,
		Singleton           = BIT(0), // Allow only of one instance of this component per object.
		HideInEditor        = BIT(1), // Hide component in editor.
		CreatePropertyGraph = BIT(2)  // Create property graph for each instance of this component.
	};

	DECLARE_ENUM_CLASS_FLAGS(EComponentFlags)

	struct IComponentFactory
	{
		virtual ~IComponentFactory() {}

		virtual CTypeInfo GetTypeInfo() const = 0; // #SchematycTODO : Do we really need CTypeInfo or will just EnvTypeId suffice?
		virtual SGUID GetComponentGUID() const = 0;
		virtual void SetName(tukk szName) = 0;
		virtual tukk GetName() const = 0;
		virtual void SetNamespace(tukk szNamespace) = 0;
		virtual tukk GetNamespace() const = 0;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) = 0;
		virtual tukk GetFileName() const = 0;
		virtual void SetAuthor(tukk szAuthor) = 0;
		virtual tukk GetAuthor() const = 0;
		virtual void SetDescription(tukk szDescription) = 0;
		virtual tukk GetDescription() const = 0;
		virtual void SetWikiLink(tukk szWikiLink) = 0;
		virtual tukk GetWikiLink() const = 0;
		virtual void AddDependency(EComponentDependencyType type, const SGUID& guid) = 0;
		virtual u32 GetDependencyCount() const = 0;
		virtual EComponentDependencyType GetDependencyType(u32 dependencyIdx) const = 0;
		virtual SGUID GetDependencyGUID(u32 dependencyIdx) const = 0;
		virtual void SetAttachmentType(EComponentSocket socket, const SGUID& attachmentTypeGUID) = 0;
		virtual SGUID GetAttachmentType(EComponentSocket socket) const = 0;
		virtual void SetFlags(EComponentFlags flags) = 0;
		virtual EComponentFlags GetFlags() const = 0;
		virtual IComponentPtr CreateComponent() const = 0;
		virtual CTypeInfo GetPropertiesTypeInfo() const = 0; // #SchematycTODO : Do we really need CTypeInfo or will just EnvTypeId suffice?
		virtual IPropertiesPtr CreateProperties() const = 0;
		virtual void SetDefaultNetworkSpawnParams(const INetworkSpawnParamsPtr& pSpawnParams) = 0;
		virtual INetworkSpawnParamsPtr CreateNetworkSpawnParams() const = 0;
	};

	DECLARE_SHARED_POINTERS(IComponentFactory)

	namespace EnvComponentUtils
	{
		inline bool IsDependency(const IComponentFactory& componentFactory, const SGUID& guid)
		{
			for(u32 dependencyIdx = 0, dependencyCount = componentFactory.GetDependencyCount(); dependencyIdx < dependencyCount; ++ dependencyIdx)
			{
				if(componentFactory.GetDependencyGUID(dependencyIdx) == guid)
				{
					return true;
				}
			}
			return false;
		}
	}
}
