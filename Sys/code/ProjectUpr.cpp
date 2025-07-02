// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ProjectUpr.h>

#include <drx3D/Sys/System.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/Enum.h>

#include <cctype>

// Temporary using statement to not break YASLI_ENUM_BEGIN_NESTED below
// Before fixing, validate that serialization to disk is the same, it currently serializes a string.
using namespace Drx;
using namespace Drx::ProjectUprInternals;

#if DRX_PLATFORM_WINDOWS
#include <Shlwapi.h>
#endif

YASLI_ENUM_BEGIN_NESTED(IPluginUpr, EPluginType, "PluginType")
YASLI_ENUM_VALUE_NESTED(IPluginUpr, EPluginType::Native, "Native")
YASLI_ENUM_VALUE_NESTED(IPluginUpr, EPluginType::Managed, "Managed")
YASLI_ENUM_END()

CProjectUpr::CProjectUpr()
	: m_sys_project(nullptr)
	, m_sys_game_name(nullptr)
	, m_sys_dll_game(nullptr)
	, m_sys_game_folder(nullptr)
{
	RegisterCVars();

	DrxFindRootFolderAndSetAsCurrentWorkingDirectory();
}

tukk CProjectUpr::GetCurrentProjectName() const
{
	return m_sys_game_name->GetString();
}

DrxGUID CProjectUpr::GetCurrentProjectGUID() const
{
	return m_project.guid;
}

tukk CProjectUpr::GetCurrentProjectDirectoryAbsolute() const
{
	return m_project.rootDirectory;
}

tukk CProjectUpr::GetCurrentAssetDirectoryRelative() const
{
	return gEnv->pDrxPak->GetGameFolder();
}

tukk CProjectUpr::GetCurrentAssetDirectoryAbsolute() const
{
	return m_project.assetDirectoryFullPath;
}

tukk CProjectUpr::GetProjectFilePath() const
{
	return m_project.filePath;
}

void CProjectUpr::StoreConsoleVariable(tukk szCVarName, tukk szValue)
{
	for (auto it = m_project.consoleVariables.begin(); it != m_project.consoleVariables.end(); ++it)
	{
		if (!stricmp(it->key, szCVarName))
		{
			it->value = szValue;

			return;
		}
	}

	m_project.consoleVariables.emplace_back(szCVarName, szValue);
}

void CProjectUpr::SaveProjectChanges()
{
	gEnv->pSystem->GetArchiveHost()->SaveJsonFile(m_project.filePath, Serialization::SStruct(m_project));
}

bool SProject::Serialize(Serialization::IArchive& ar)
{
	// Only save to the latest format
	if (ar.isOutput())
	{
		version = LatestProjectFileVersion;
	}

	ar(version, "version", "version");

	SProjectFileParser<LatestProjectFileVersion> parser;
	parser.Serialize(ar, *this);
	return true;
}

bool CProjectUpr::ParseProjectFile()
{
	tukk szEngineRootDirectory = gEnv->pSystem->GetRootFolder();

	const ICmdLineArg* arg = gEnv->pSystem->GetICmdLine()->FindArg(eCLAT_Pre, "project");
	string projectFile = arg != nullptr ? arg->GetValue() : m_sys_project->GetString();
	if (projectFile.size() == 0)
	{
		DrxLogAlways("\nRunning DRXENGINE without a project!");
		DrxLogAlways("	Using Engine Folder %s", szEngineRootDirectory);

		m_sys_game_name->Set("DRXENGINE - No Project");
		// Specify an assets directory despite having no project, this is to prevent DrxPak scanning engine root
		m_sys_game_folder->Set("Assets");
		return false;
	}

	string extension = PathUtil::GetExt(projectFile);
	if (extension.empty())
	{
		projectFile = PathUtil::ReplaceExtension(projectFile, "drxproject");
	}

#if DRX_PLATFORM_DURANGO
	if(true)
#elif DRX_PLATFORM_WINAPI
	if (PathIsRelative(projectFile.c_str()))
#elif DRX_PLATFORM_POSIX
	if (projectFile[0] != '/')
#endif
	{
		m_project.filePath = PathUtil::Make(szEngineRootDirectory, projectFile.c_str());
	}
	else
	{
		m_project.filePath = projectFile.c_str();
	}

#ifndef DRX_FORCE_DRXPROJECT_IN_PAK
	i32 flags = IDrxPak::FOPEN_ONDISK;
#else
	i32 flags = 0;
#endif
	CDrxFile file;
	file.Open(m_project.filePath.c_str(), "rb", flags);

	std::vector<char> projectFileJson;
	if (file.GetHandle() != nullptr)
	{
		projectFileJson.resize(file.GetLength());
	}

	if (projectFileJson.size() > 0 &&
		file.ReadRaw(projectFileJson.data(), projectFileJson.size()) == projectFileJson.size() &&
		gEnv->pSystem->GetArchiveHost()->LoadJsonBuffer(Serialization::SStruct(m_project), projectFileJson.data(), projectFileJson.size()))
	{
		if (m_project.version > LatestProjectFileVersion)
		{
			EQuestionResult result = DrxMessageBox("Attempting to start the engine with a potentially unsupported .drxproject made with a newer version of the engine!\nDo you want to continue?", "Loading unknown .drxproject version", eMB_YesCancel);
			if (result == eQR_Cancel)
			{
				DrxLogAlways("Unknown .drxproject version %i detected, user opted to quit", m_project.version);
				return false;
			}

			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Loading a potentially unsupported .drxproject made with a newer version of the engine!");
		}

		m_project.rootDirectory = PathUtil::RemoveSlash(PathUtil::ToUnixPath(PathUtil::GetPathWithoutFilename(m_project.filePath)));

		// Create the full path to the asset directory
		m_project.assetDirectoryFullPath = PathUtil::Make(m_project.rootDirectory, m_project.assetDirectory);
		// Ensure compatibility with all supported platform filesystems
		m_project.assetDirectoryFullPath.MakeLower();

		// Does directory exist
		if (!DrxDirectoryExists(m_project.assetDirectoryFullPath.c_str()))
		{
			EQuestionResult result = DrxMessageBox(string().Format("Attempting to start the engine with non-existent asset directory %s!"
				"\n\nPlease verify the asset directory in your .drxproject file, in case you expected the asset directory to exist!\n\nDo you want to create the directory?",
				m_project.assetDirectoryFullPath.c_str()), "Non-existent asset directory", eMB_YesCancel);
			if (result == eQR_Cancel)
			{
				DrxLogAlways("\tNon-existent asset directory %s detected, user opted to quit", m_project.assetDirectoryFullPath.c_str());
				return false;
			}

			DrxCreateDirectory(m_project.assetDirectoryFullPath.c_str());
		}

		// Set the legacy game folder and name
		m_sys_game_folder->Set(m_project.assetDirectory);
		m_sys_game_name->Set(m_project.name);

		for (SProject::SConsoleInstruction& consoleVariable : m_project.consoleVariables)
		{
			gEnv->pConsole->LoadConfigVar(consoleVariable.key, consoleVariable.value);
		}

		for (SProject::SConsoleInstruction& consoleCommand : m_project.consoleCommands)
		{
			stack_string command = consoleCommand.key;
			command.append(" ");
			command.append(consoleCommand.value.c_str());

			gEnv->pConsole->ExecuteString(command.c_str(), false, true);
		}

		auto gameDllIt = m_project.legacyGameDllPaths.find("any");

#if (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT)
		if (gameDllIt == m_project.legacyGameDllPaths.end())
		{
			gameDllIt = m_project.legacyGameDllPaths.find("win_x64");
		}
#elif (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT)
		if (gameDllIt == m_project.legacyGameDllPaths.end())
		{
			gameDllIt = m_project.legacyGameDllPaths.find("win_x86");
		}
#endif
		string legacyGameDllPath;

		// Set legacy Game DLL
		if (gameDllIt != m_project.legacyGameDllPaths.end())
		{
			legacyGameDllPath = gameDllIt->second;

			m_sys_dll_game->Set(legacyGameDllPath);
		}

		gEnv->pConsole->LoadConfigVar("sys_dll_game", legacyGameDllPath);

#ifdef DRX_PLATFORM_WINDOWS
		SetDllDirectoryW(DrxStringUtils::UTF8ToWStr(m_project.rootDirectory));
#endif

#ifndef DRX_PLATFORM_ORBIS
		DrxSetCurrentWorkingDirectory(m_project.rootDirectory);
#endif

		// Update the project file if the loaded version was outdated
		if (m_project.version != LatestProjectFileVersion)
		{
			// Add plug-ins that were made default since this version
			AddDefaultPlugins(m_project.version);

			// Add default plug-ins since they were hardcoded before version 1
			if (m_project.version == 0)
			{
				LoadLegacyPluginCSV();
				LoadLegacyGameCfg();
			}
			else if(m_project.version == 1)
			{
				// Generate GUID
				m_project.guid = DrxGUID::Create();
			}

			m_project.version = LatestProjectFileVersion;

			SaveProjectChanges();
		}
	}
	else if(m_sys_game_folder->GetString()[0] == '\0')
	{
		// No project folder found, and no legacy context to migrate from.
		m_project.filePath.clear();
	}

	// Check if we are migrating from legacy workflow
	if (CanMigrateFromLegacyWorkflow())
	{
		DrxLogAlways("\nMigrating from legacy project workflow to new %s file", m_project.filePath.c_str());

		// Migration will occur once MigrateFromLegacyWorkflowIfNecessary is called
		return true;
	}
	// Detect running engine without project directory
	else if (m_project.filePath.empty())
	{
		if (gEnv->bTesting)
		{
			DrxLogAlways("\nRunning engine in unit testing mode without project");

			// Create a temporary asset directory, as some systems rely on an assets directory existing.
			m_project.assetDirectory = "NoAssetFolder";
			m_project.assetDirectoryFullPath = PathUtil::Make(szEngineRootDirectory, m_project.assetDirectory);
			m_project.assetDirectoryFullPath.MakeLower();

			DrxCreateDirectory(m_project.assetDirectoryFullPath.c_str());

			m_sys_game_folder->Set(m_project.assetDirectory.c_str());

			return true;
		}
		else
		{
			DrxMessageBox("Attempting to start the engine without a project!\nPlease use a .drxproject file!", "Engine initialization failed", eMB_Error);
			return false;
		}
	}
	// Detect running without asset directory
	else if (m_project.assetDirectory.empty())
	{
		if (!gEnv->bTesting)
		{
			EQuestionResult result = DrxMessageBox("Attempting to start the engine without an asset directory!\nContinuing will put the engine into a readonly state where changes can't be saved, do you want to continue?", "No Assets directory", eMB_YesCancel);
			if (result == eQR_Cancel)
			{
				DrxLogAlways("\tNo asset directory detected, user opted to quit");
				return false;
			}
		}

		// Engine started without asset directory, we have to create a temporary directory in this case
		// This is done as many systems rely on checking for files in the asset directory, without one they will search the root or even the entire drive.
		m_project.assetDirectory = "NoAssetFolder";
		DrxLogAlways("\tSkipped use of assets directory");
	}

	DrxLogAlways("\nProject %s", GetCurrentProjectName());
	DrxLogAlways("\tUsing Project Folder %s", GetCurrentProjectDirectoryAbsolute());
	DrxLogAlways("\tUsing Engine Folder %s", szEngineRootDirectory);
	DrxLogAlways("\tUsing Asset Folder %s", GetCurrentAssetDirectoryAbsolute());

	return true;
}

void CProjectUpr::MigrateFromLegacyWorkflowIfNecessary()
{
	// Populate project data and save .drxproject if no project was used
	// This is done by assuming legacy game folder setup.
	if (CanMigrateFromLegacyWorkflow())
	{
		m_project.version = LatestProjectFileVersion;
		m_project.type = "DRXENGINE Project";
		m_project.name = m_sys_game_name->GetString();
		// Specify that drxproject file is in engine root
		m_project.engineVersionId = ".";

		char buffer[MAX_PATH];
		DrxGetCurrentDirectory(MAX_PATH, buffer);

		m_project.rootDirectory = PathUtil::RemoveSlash(PathUtil::ToUnixPath(buffer));
		m_project.assetDirectory = GetCurrentAssetDirectoryRelative();

		// Create the full path to the asset directory
		m_project.assetDirectoryFullPath = PathUtil::Make(m_project.rootDirectory, m_project.assetDirectory);
		m_project.assetDirectoryFullPath.MakeLower();

		// Make sure we include default plug-ins
		AddDefaultPlugins(0);
		LoadLegacyPluginCSV();
		LoadLegacyGameCfg();

		tukk legacyDllName = m_sys_dll_game->GetString();
		if (strlen(legacyDllName) > 0)
		{
			m_project.legacyGameDllPaths["any"] = PathUtil::RemoveExtension(legacyDllName);
		}

		string sProjectFile = PathUtil::Make(m_project.rootDirectory, m_sys_project->GetString());
		// Make sure we have the .drxproject extension
		sProjectFile = PathUtil::ReplaceExtension(sProjectFile, ".drxproject");
		gEnv->pSystem->GetArchiveHost()->SaveJsonFile(sProjectFile, Serialization::SStruct(m_project));
	}
}

void CProjectUpr::RegisterCVars()
{
	// Default to no project when running unit tests or shader cache generator
	bool bDefaultToNoProject = gEnv->bTesting || gEnv->pSystem->IsShaderCacheGenMode();

	m_sys_project = REGISTER_STRING("sys_project", bDefaultToNoProject ? "" : "game.drxproject", VF_NULL, "Specifies which project to load.\nLoads from the engine root if relative path, otherwise full paths are allowed to allow out-of-engine projects\nHas no effect if -project switch is used!");

	// Legacy
	m_sys_game_name = REGISTER_STRING("sys_game_name", "DRXENGINE", VF_DUMPTODISK, "Specifies the name to be displayed in the Launcher window title bar");
	m_sys_dll_game = REGISTER_STRING("sys_dll_game", "", VF_NULL, "Specifies the game DLL to load");
	m_sys_game_folder = REGISTER_STRING("sys_game_folder", "", VF_NULL, "Specifies the game folder to read all data from. Can be fully pathed for external folders or relative path for folders inside the root.");
}

//--- UTF8 parse helper routines

static tukk Parser_NextChar(tukk pStart, tukk pEnd)
{
	DRX_ASSERT(pStart != nullptr && pEnd != nullptr);

	if (pStart < pEnd)
	{
		DRX_ASSERT(0 <= *pStart && *pStart <= SCHAR_MAX);
		pStart++;
	}

	DRX_ASSERT(pStart <= pEnd);
	return pStart;
}

static tukk Parser_StrChr(tukk pStart, tukk pEnd, i32 c)
{
	DRX_ASSERT(pStart != nullptr && pEnd != nullptr);
	DRX_ASSERT(pStart <= pEnd);
	DRX_ASSERT(0 <= c && c <= SCHAR_MAX);
	tukk it = (tukk)memchr(pStart, c, pEnd - pStart);
	return (it != nullptr) ? it : pEnd;
}

static bool Parser_StrEquals(tukk pStart, tukk pEnd, tukk szKey)
{
	size_t klen = strlen(szKey);
	return (klen == pEnd - pStart) && memcmp(pStart, szKey, klen) == 0;
}

void CProjectUpr::LoadLegacyPluginCSV()
{
	FILE* pFile = gEnv->pDrxPak->FOpen("drxplugin.csv", "rb", IDrxPak::FLAGS_PATH_REAL);
	if (pFile == nullptr)
		return;

	std::vector<char> buffer;
	buffer.resize(gEnv->pDrxPak->FGetSize(pFile));

	gEnv->pDrxPak->FReadRawAll(buffer.data(), buffer.size(), pFile);
	gEnv->pDrxPak->FClose(pFile);

	tukk pTokenStart = buffer.data();
	tukk pBufferEnd = buffer.data() + buffer.size();
	while (pTokenStart != pBufferEnd)
	{
		tukk pNewline = Parser_StrChr(pTokenStart, pBufferEnd, '\n');
		tukk pSemicolon = Parser_StrChr(pTokenStart, pNewline, ';');

		Drx::IPluginUpr::EType pluginType = Drx::IPluginUpr::EType::Native;
		if (Parser_StrEquals(pTokenStart, pSemicolon, "C#"))
			pluginType = Drx::IPluginUpr::EType::Managed;

		// Parsing of plugin name
		pTokenStart = Parser_NextChar(pSemicolon, pNewline);
		pSemicolon = Parser_StrChr(pTokenStart, pNewline, ';');

		pTokenStart = Parser_NextChar(pSemicolon, pNewline);
		pSemicolon = Parser_StrChr(pTokenStart, pNewline, ';');

		string pluginClassName;
		pluginClassName.assign(pTokenStart, pSemicolon - pTokenStart);
		pluginClassName.Trim();

		pSemicolon = Parser_StrChr(pTokenStart, pNewline, ';');
		pTokenStart = Parser_NextChar(pSemicolon, pNewline);

		string pluginBinaryPath;
		pluginBinaryPath.assign(pTokenStart, pSemicolon - pTokenStart);
		pluginBinaryPath.Trim();

		pTokenStart = Parser_NextChar(pSemicolon, pNewline);
		pSemicolon = Parser_StrChr(pTokenStart, pNewline, ';');

		string pluginAssetDirectory;
		pluginAssetDirectory.assign(pTokenStart, pSemicolon - pTokenStart);
		pluginAssetDirectory.Trim();

		pTokenStart = Parser_NextChar(pNewline, pBufferEnd);
		AddPlugin(SPluginDefinition{ pluginType, pluginBinaryPath });
	}
}

void CProjectUpr::LoadLegacyGameCfg()
{
	string cfgPath = PathUtil::Make(m_project.assetDirectoryFullPath, "game.cfg");
	gEnv->pSystem->LoadConfiguration(cfgPath, this);
}

void CProjectUpr::OnLoadConfigurationEntry(tukk szKey, tukk szValue, tukk szGroup)
{
	// Make sure we set the value in the engine as well
	gEnv->pConsole->LoadConfigVar(szKey, szValue);

	// Store in our drxproject json, unless the CVar is one that we have in other properties
	if (strcmp(szKey, m_sys_dll_game->GetName()) != 0 && strcmp(szKey, m_sys_game_name->GetName()) != 0)
	{
		StoreConsoleVariable(szKey, szValue);
	}
}

void CProjectUpr::AddDefaultPlugins(u32 previousVersion)
{
	for (const CDrxPluginUpr::TDefaultPluginPair& defaultPlugin : CDrxPluginUpr::GetDefaultPlugins())
	{
		// If the version the plug-in was made default in is higher than the version we're upgrading from, add to project
		if (defaultPlugin.first > previousVersion)
		{
			AddPlugin(defaultPlugin.second);
		}
	}
}

void CProjectUpr::AddPlugin(const SPluginDefinition& definition)
{
	// Make sure duplicates aren't added
	for(const SPluginDefinition& pluginDefinition : m_project.plugins)
	{
		if (!stricmp(pluginDefinition.path, definition.path))
		{
			return;
		}
	}

	m_project.plugins.push_back(definition);
}

string CProjectUpr::LoadTemplateFile(tukk szPath, std::function<string(tukk szAlias)> aliasReplacementFunc) const
{
	CDrxFile file(szPath, "rb");
	if (file.GetHandle() == nullptr)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Failed to load template %s!", szPath);
		return "";
	}

	size_t fileLength = file.GetLength();
	file.SeekToBegin();

	std::vector<char> parsedString;
	parsedString.resize(fileLength);

	if (file.ReadRaw(parsedString.data(), fileLength) != fileLength)
	{
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "Failed to read template %s!", szPath);
		return "";
	}

	string finalText;
	finalText.reserve(parsedString.size());

	for (auto it = parsedString.cbegin(), end = parsedString.cend(); it != end; ++it)
	{
		// Words prefixed by $ are treated as aliases and replaced by the callback
		if (*it == '$')
		{
			// Double $ means we replace with one $
			if (it + 1 == end || *(it + 1) == '$')
			{
				finalText += '$';
				it += 1;
			}
			else
			{
				string alias;

				auto subIt = it + 1;
				for (; subIt != end && (std::isalpha(*subIt) || *subIt == '_'); ++subIt)
				{
					alias += *subIt;
				}

				it = subIt - 1;

				finalText += aliasReplacementFunc(alias);
			}
		}
		else
		{
			finalText += *it;
		}
	}

	return finalText;
}

void CProjectUpr::GetPluginInfo(u16 index, Drx::IPluginUpr::EType& typeOut, string& pathOut, DynArray<EPlatform>& platformsOut) const
{
	auto plugin = m_project.plugins[index];
	pathOut = plugin.path;
	typeOut = plugin.type;
	platformsOut.reserve(plugin.platforms.size());
	for (EPlatform platform : plugin.platforms)
	{
		platformsOut.push_back(platform);
	}
}
