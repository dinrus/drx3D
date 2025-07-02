// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/GUID.h>
#include <drx3D/Schema2/IProperties.h>

namespace sxema2
{
	struct IAction;

	DECLARE_SHARED_POINTERS(IAction)

	enum class EActionFlags
	{
		None                      = 0,
		Singleton                 = BIT(0), // Allow only of one instance of this action to run at a time per object.
		ServerOnly                = BIT(1), // Action should only be active on server.
		ClientOnly                = BIT(2), // Action should only be active on clients.
		NetworkReplicateServer    = BIT(3), // Action should be replicated on server.
		NetworkReplicateClients   = BIT(4)  // Action should be replicated on clients.
	};

	DECLARE_ENUM_CLASS_FLAGS(EActionFlags)

	struct IActionFactory
	{
		virtual ~IActionFactory() {}

		virtual SGUID GetActionGUID() const = 0;
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
		virtual void SetFlags(EActionFlags flags) = 0;
		virtual EActionFlags GetFlags() const = 0;
		virtual IActionPtr CreateAction() const = 0;
		virtual IPropertiesPtr CreateProperties() const = 0;
	};

	DECLARE_SHARED_POINTERS(IActionFactory)
}
