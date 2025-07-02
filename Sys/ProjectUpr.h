// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IProjectUpr.h>
#include <drx3D/Sys/IDrxPluginUpr.h>
#include <drx3D/CoreX/Serialization/yasli/STL.h>

YASLI_ENUM_BEGIN(EPlatform, "Platform")
YASLI_ENUM(EPlatform::Windows, "Windows", "Windows")
YASLI_ENUM(EPlatform::Linux, "Linux", "Linux")
YASLI_ENUM(EPlatform::MacOS, "MacOS", "MacOS")
YASLI_ENUM(EPlatform::XboxOne, "XboxOne", "XboxOne")
YASLI_ENUM(EPlatform::PS4, "PS4", "PS4")
YASLI_ENUM(EPlatform::Android, "Android", "Android")
YASLI_ENUM(EPlatform::iOS, "iOS", "iOS")
YASLI_ENUM_END()

struct SPluginDefinition
{
	Drx::IPluginUpr::EType type;
	string path;
	//! Determines the platforms for which this plug-in should be loaded
	//! An empty vector indicates that we should always load
	std::vector<EPlatform> platforms;
	
	SPluginDefinition() = default;
	
	SPluginDefinition(Drx::IPluginUpr::EType type, string path): type(type), path(path)	{}
	
	SPluginDefinition(Drx::IPluginUpr::EType type, string path, EPlatform plat): type(type), path(path)	{/*platforms <<= plat;*/}

	void Serialize(Serialization::IArchive& ar)
	{
		ar(type, "type", "type");
		ar(path, "path", "path");

		if (ar.isInput() || !platforms.empty())
		{
			ar(platforms, "platforms", "platforms");
		}
	}

	bool operator==(const SPluginDefinition& rhs) const
	{
		return type == rhs.type && path == rhs.path;
	}
	bool operator!=(const SPluginDefinition& rhs) const
	{
		return !(*this == rhs);
	}

};

namespace Drx
{
	namespace ProjectUprInternals
	{
		struct SProject
		{
			// Serialize the project file
			bool Serialize(Serialization::IArchive& ar);

			u32 version = 0;
			// why do we need this?
			string type;
			// Project name
			string name;
			DrxGUID guid;
			// Path to the .drxproject file
			string filePath;
			string engineVersionId;

			// Directory containing the .drxproject file
			string rootDirectory;
			// Directory game will search for assets in (relative to project directory)
			string assetDirectory;
			// Full path to the asset directory
			string assetDirectoryFullPath;

			// Directory containing native and managed code (relative to project directory)
			string codeDirectory;

			// List of plug-ins to load
			std::vector<SPluginDefinition> plugins;

			std::unordered_map<string, string, stl::hash_strcmp<string>> legacyGameDllPaths;

			struct SConsoleInstruction
			{
				SConsoleInstruction() {}
				SConsoleInstruction(tukk szKey, tukk szValue)
					: key(szKey)
					, value(szValue) {}

				void Serialize(Serialization::IArchive& ar)
				{
					ar(key, "name", "name");
					ar(value, "value", "value");
				}

				string key;
				string value;
			};

			// Specialized CVar values for the project
			std::vector<SConsoleInstruction> consoleVariables;
			// Specialized console commands for the project
			std::vector<SConsoleInstruction> consoleCommands;
		};

		template<u32 version> struct SProjectFileParser {};

		//! Latest version of the project syntax
		//! Bump this when syntax changes, or default plug-ins are added
		//! This allows us to automatically migrate and support older versions
		//! Version 0 = pre-project system, allows for migrating from legacy (game.cfg etc) to .drxproject
		constexpr u32 LatestProjectFileVersion = 3;

		template<>
		struct SProjectFileParser<LatestProjectFileVersion>
		{
			void Serialize(Serialization::IArchive& ar, SProject& project)
			{
				struct SRequire
				{
					SRequire(SProject& _project) : project(_project) {}

					void Serialize(Serialization::IArchive& ar)
					{
						ar(project.engineVersionId, "engine", "engine");
						ar(project.plugins, "plugins", "plugins");
					}

					SProject& project;
				};

				struct SInfo
				{
					SInfo(SProject& _project) : project(_project) {}

					void Serialize(Serialization::IArchive& ar)
					{
						ar(project.name, "name", "name");
						ar(project.guid, "guid", "guid");
					}

					SProject& project;
				};

				struct SContent
				{
					struct SLibrary
					{
						struct SShared
						{
							void Serialize(Serialization::IArchive& ar)
							{
								ar(libPathAny, "any", "any");
								ar(libPathWin64, "win_x64", "win_x64");
								ar(libPathWin32, "win_x86", "win_x86");
							}

							string libPathAny;
							string libPathWin64;
							string libPathWin32;
						};

						void Serialize(Serialization::IArchive& ar)
						{
							ar(name, "name", "name");
							ar(shared, "shared", "shared");
						}

						string name;
						SShared shared;
					};

					SContent(SProject& _project) : project(_project) {}

					void Serialize(Serialization::IArchive& ar)
					{
						if (ar.isOutput())
						{
							assetDirectories = { project.assetDirectory };
							codeDirectories = { project.codeDirectory };

							libraries.resize(1);
							libraries[0].name = project.name;
							libraries[0].shared.libPathAny = project.legacyGameDllPaths["any"];
							libraries[0].shared.libPathWin64 = project.legacyGameDllPaths["win_x64"];
							libraries[0].shared.libPathWin32 = project.legacyGameDllPaths["win_x86"];
						}

						ar(assetDirectories, "assets", "assets");
						ar(codeDirectories, "code", "code");
						ar(libraries, "libs", "libs");

						if (ar.isInput())
						{
							project.assetDirectory = assetDirectories.size() > 0 ? assetDirectories.front() : "";
							project.codeDirectory = codeDirectories.size() > 0 ? codeDirectories.front() : "";

							if (libraries.size() > 0)
							{
								project.legacyGameDllPaths["any"] = libraries[0].shared.libPathAny;
								project.legacyGameDllPaths["win_x64"] = libraries[0].shared.libPathWin64;
								project.legacyGameDllPaths["win_x86"] = libraries[0].shared.libPathWin32;
							}
						}
					}

					std::vector<string> assetDirectories;
					std::vector<string> codeDirectories;
					std::vector<SLibrary> libraries;

					SProject& project;
				};

				ar(project.type, "type", "type");
				ar(SInfo(project), "info", "info");
				ar(SContent(project), "content", "content");
				ar(SRequire(project), "require", "require");
				ar(project.consoleVariables, "console_variables", "console_variables");
				ar(project.consoleCommands, "console_commands", "console_commands");
			}
		};
	}
}

class CProjectUpr final
	: public IProjectUpr
	, public ILoadConfigurationEntrySink
{
public:
	CProjectUpr();
	virtual ~CProjectUpr() {}

	bool                                  ParseProjectFile();
	void                                  MigrateFromLegacyWorkflowIfNecessary();

	// IProjectUpr
	virtual tukk                   GetCurrentProjectName() const override;
	virtual DrxGUID                       GetCurrentProjectGUID() const override;

	virtual tukk                   GetCurrentProjectDirectoryAbsolute() const override;

	virtual tukk                   GetCurrentAssetDirectoryRelative() const override;
	virtual tukk                   GetCurrentAssetDirectoryAbsolute() const override;
	virtual tukk                   GetProjectFilePath() const override;

	virtual void                          StoreConsoleVariable(tukk szCVarName, tukk szValue) override;
	virtual void                          SaveProjectChanges() override;

	virtual u16k                  GetPluginCount() const override { return static_cast<u16>(m_project.plugins.size()); };
	virtual void                          GetPluginInfo(u16 index, Drx::IPluginUpr::EType& typeOut, string& pathOut, DynArray<EPlatform>& platformsOut) const override;

	virtual string                        LoadTemplateFile(tukk szPath, std::function<string(tukk szAlias)> aliasReplacementFunc) const override;
	// ~IProjectUpr

	// ILoadConfigurationEntrySink
	virtual void                          OnLoadConfigurationEntry(tukk szKey, tukk szValue, tukk szGroup) override;
	// ~ILoadConfigurationEntrySink

	const std::vector<SPluginDefinition>& GetPluginDefinitions() const { return m_project.plugins; }

protected:
	void RegisterCVars();

	void LoadLegacyPluginCSV();
	void LoadLegacyGameCfg();
	void AddDefaultPlugins(u32 previousVersion);

	void AddPlugin(const SPluginDefinition& definition);

	void FindSourceFilesInDirectoryRecursive(tukk szDirectory, tukk szExtension, std::vector<string>& sourceFiles) const;

	bool CanMigrateFromLegacyWorkflow() const { return m_project.version == 0 && m_sys_game_folder->GetString()[0] != '\0' && !m_project.filePath.empty(); }

protected:
	Drx::ProjectUprInternals::SProject m_project;

	ICVar* m_sys_project;

	// Legacy CVars
	ICVar* m_sys_game_name;
	ICVar* m_sys_dll_game;
	ICVar* m_sys_game_folder;
};