// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

namespace sxema2
{
	struct IScriptGraphNode;

	DECLARE_SHARED_POINTERS(IScriptGraphNode);

	struct IScriptGraphNodeCreationMenuCommand
	{
		virtual ~IScriptGraphNodeCreationMenuCommand() {}

		virtual IScriptGraphNodePtr Execute(const Vec2& pos) = 0; // #SchematycTODO : Do we really need to pass position here?
	};

	DECLARE_SHARED_POINTERS(IScriptGraphNodeCreationMenuCommand)

	struct IScriptGraphNodeCreationMenu
	{
		virtual ~IScriptGraphNodeCreationMenu() {}

		virtual bool AddOption(tukk szLabel, tukk szDescription, tukk szWikiLink, const IScriptGraphNodeCreationMenuCommandPtr& pCommand) = 0;
	};
}
