// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/DrxGUID.h>
#include "IDrxPluginUpr.h"

//! Main interface used to manage the currently run project (known by the .drxproject extension).
struct IProjectUpr
{
	virtual ~IProjectUpr() {}

	//! Gets the human readable name of the game, for example used for updating the window title on desktop
	virtual tukk  GetCurrentProjectName() const = 0;
	//! Gets the globally unique identifier for this project, used to uniquely identify certain assets with projects
	virtual DrxGUID      GetCurrentProjectGUID() const = 0;

	//! Gets the absolute path to the root of the project directory, where the .drxproject resides.
	//! \return Path without trailing separator.
	virtual tukk  GetCurrentProjectDirectoryAbsolute() const = 0;

	//! Gets the path to the assets directory, relative to project root
	virtual tukk  GetCurrentAssetDirectoryRelative() const = 0;
	//! Gets the absolute path to the asset directory
	virtual tukk  GetCurrentAssetDirectoryAbsolute() const = 0;

	virtual tukk  GetProjectFilePath() const = 0;

	//! Adds or updates the value of a CVar in the project configuration
	virtual void         StoreConsoleVariable(tukk szCVarName, tukk szValue) = 0;

	//! Saves the .drxproject file with new values from StoreConsoleVariable
	virtual void         SaveProjectChanges() = 0;

	//! Gets the number of plug-ins for the current project
	virtual u16k GetPluginCount() const = 0;
	//! Gets details on a specific plug-in by index
	//! \see GetPluginCount
	virtual void         GetPluginInfo(u16 index, Drx::IPluginUpr::EType& typeOut, string& pathOut, DynArray<EPlatform>& platformsOut) const = 0;
	//! Loads a project template, allowing substition of aliases prefixed by '$' in the provided lambda
	virtual string       LoadTemplateFile(tukk szPath, std::function<string(tukk szAlias)> aliasReplacementFunc) const = 0;
};
