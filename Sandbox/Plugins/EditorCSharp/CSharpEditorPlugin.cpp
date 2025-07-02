// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "StdAfx.h"
#include "CSharpEditorPlugin.h"

#include <Include/SandboxAPI.h>
#include <IEditorImpl.h>
#include "QT/QToolTabManager.h"
#include <QProcess>

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetManager.h>
#include <AssetSystem/AssetType.h>
#include <CSharpOutputWindow.h>
#include <IObjectManager.h>
#include <Objects/ClassDesc.h>
#include <DrxSystem/IProjectManager.h>
#include <Preferences/GeneralPreferences.h>

#include <drx3D/CoreX/Platform/platform_impl.inl>

CCSharpEditorPlugin* CCSharpEditorPlugin::s_pInstance = nullptr;

CCSharpEditorPlugin::CCSharpEditorPlugin()
	: m_messageListeners(3)
{
	s_pInstance = this;

	m_csharpSolutionPath = PathUtil::Make(gEnv->pSystem->GetIProjectManager()->GetCurrentProjectDirectoryAbsolute(), "Game.sln");

	GetIEditor()->GetFileMonitor()->RegisterListener(this, "", "cs");

	gEnv->pSchematyc->GetEnvRegistry().RegisterListener(this);

	DRX_ASSERT(gEnv->pMonoRuntime != nullptr);
	if (gEnv->pMonoRuntime != nullptr)
	{
		gEnv->pMonoRuntime->RegisterCompileListener(this);

		m_compileMessage = gEnv->pMonoRuntime->GetLatestCompileMessage();
	}

	// Regenerate the plugins in case the files were changed when the Sandbox was closed
	RegenerateSolution();

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CCSharpEditorPlugin");
}

CCSharpEditorPlugin::~CCSharpEditorPlugin()
{
	GetIEditor()->GetFileMonitor()->UnregisterListener(this);

	gEnv->pSchematyc->GetEnvRegistry().UnregisterListener(this);

	if (gEnv->pMonoRuntime != nullptr)
	{
		gEnv->pMonoRuntime->UnregisterCompileListener(this);
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
}

void CCSharpEditorPlugin::SetDefaultTextEditor()
{
	string textEditor = gEditorFilePreferences.textEditorCSharp;

	IDrxPak* pDrxPak = gEnv->pDrxPak;

	// Only change it when it's set to the default value or no value.
	if (!textEditor.IsEmpty() && textEditor != "devenv.exe")
	{
		return;
	}

	char szVSWherePath[_MAX_PATH];
	ExpandEnvironmentStringsA("%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe", szVSWherePath, DRX_ARRAY_COUNT(szVSWherePath));

	if (!pDrxPak->IsFileExist(szVSWherePath))
	{
		return;
	}

	QProcess process;
	process.start(szVSWherePath, QStringList() << "-format" << "value" << "-property" << "installationPath");
	if (!process.waitForStarted())
	{
		DrxLog("Unable to detect installed versions of Visual Studio because vswhere.exe could not be started.");
		return;
	}
	if (!process.waitForFinished() && process.exitStatus() != QProcess::ExitStatus::NormalExit)
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to find Visual Studio installations because vswhere.exe crashed!");
		return;
	}

	QByteArray qtOutput = process.readAllStandardOutput();
	string output = qtOutput.toStdString().c_str();

	string installationPath;
	bool exists = false;
	i32 pos = 0;
	string path;
	while (!(path = output.Tokenize("\r\n", pos)).empty())
	{
		installationPath = string().Format("%s/Common7/IDE/devenv.exe", path);
		if (pDrxPak->IsFileExist(installationPath))
		{
			exists = true;
			break;
		}
	}

	if (exists && !installationPath.IsEmpty())
	{
		gEditorFilePreferences.textEditorCSharp = string().Format("\"%s\"", installationPath);
		GetIEditor()->GetPreferences()->Save();
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to find the executable of Visual Studio 2017 or later!");
	}
}

void CCSharpEditorPlugin::OnFileChange(tukk szFilename, EChangeType type)
{
	switch (type)
	{
	case IFileChangeListener::eChangeType_Created:
		{
			// If a file was deleted and created at the same time, remove it from the changed list. It's probably only modified.
			// Otherwise add it to the changed-files list, because the solution needs to be generated again.
			if (!stl::find_and_erase(m_changedFiles, szFilename))
			{
				m_changedFiles.emplace_back(szFilename);
			}
			m_reloadPlugins = true;
			break;
		}

	case IFileChangeListener::eChangeType_RenamedNewName:
		{
			// If a file was deleted/renamed and created again remove it from the changed list. It's probably only modified.
			stl::find_and_erase(m_changedFiles, szFilename);
			m_reloadPlugins = true;
			break;
		}

	case IFileChangeListener::eChangeType_Modified:
		{
			m_reloadPlugins = true;
			break;
		}

	case IFileChangeListener::eChangeType_RenamedOldName:
	case IFileChangeListener::eChangeType_Deleted:
		{
			m_changedFiles.emplace_back(szFilename);
			m_reloadPlugins = true;
			break;
		}

	case IFileChangeListener::eChangeType_Unknown:
	default:
		break;
	}
}

void CCSharpEditorPlugin::OnCompileFinished(tukk szCompileMessage)
{
	m_compileMessage = szCompileMessage;

	for (CSharpMessageListeners::Notifier notifier(m_messageListeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnMessagesUpdated(m_compileMessage);
	}
}

void CCSharpEditorPlugin::OnEnvElementAdd(Schematyc::IEnvElementPtr pElement)
{
	if (pElement->GetType() != Schematyc::EEnvElementType::Component)
	{
		return;
	}

	auto pComponent = static_cast<Schematyc::IEnvComponent*>(pElement.get());
	if (strlen(pComponent->GetDesc().GetLabel()) == 0)
	{
		return;
	}

	if (pComponent->GetDesc().GetComponentFlags().Check(EEntityComponentFlags::HideFromInspector))
	{
		return;
	}

	CObjectClassDesc* clsDesc = GetIEditor()->GetObjectManager()->FindClass("EntityWithComponent");
	if (!clsDesc)
	{
		return;
	}

	tukk szCategory = pComponent->GetDesc().GetEditorCategory();
	if (szCategory == nullptr || szCategory[0] == '\0')
	{
		szCategory = "General";
	}

	char buffer[37];
	pComponent->GetDesc().GetGUID().ToString(buffer);

	string sLabel = szCategory;
	sLabel.append("/");
	sLabel.append(pComponent->GetDesc().GetLabel());

	tukk description = pComponent->GetDesc().GetDescription();

	clsDesc->m_itemAdded(sLabel.c_str(), buffer, description);
}

void CCSharpEditorPlugin::OnEnvElementDelete(Schematyc::IEnvElementPtr pElement)
{
	if (pElement->GetType() != Schematyc::EEnvElementType::Component)
	{
		return;
	}

	auto pComponent = static_cast<Schematyc::IEnvComponent*>(pElement.get());
	if (strlen(pComponent->GetDesc().GetLabel()) == 0)
	{
		return;
	}

	if (pComponent->GetDesc().GetComponentFlags().Check(EEntityComponentFlags::HideFromInspector))
	{
		return;
	}

	CObjectClassDesc* clsDesc = GetIEditor()->GetObjectManager()->FindClass("EntityWithComponent");
	if (!clsDesc)
	{
		return;
	}

	tukk szCategory = pComponent->GetDesc().GetEditorCategory();
	if (szCategory == nullptr || szCategory[0] == '\0')
	{
		szCategory = "General";
	}

	char buffer[37];
	pComponent->GetDesc().GetGUID().ToString(buffer);

	string sLabel = szCategory;
	sLabel.append("/");
	sLabel.append(pComponent->GetDesc().GetLabel());

	clsDesc->m_itemRemoved(sLabel.c_str(), buffer);
}

void CCSharpEditorPlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	if (event == ESYSTEM_EVENT_CHANGE_FOCUS)
	{
		if (wparam == 0)
		{
			// Lost focus
			m_isSandboxInFocus = false;
		}
		else
		{
			// Got focus back
			m_isSandboxInFocus = true;
			UpdatePluginsAndSolution();
		}
	}
}

void CCSharpEditorPlugin::OnEditorNotifyEvent(EEditorNotifyEvent aEventId)
{
	if (aEventId == eNotify_OnIdleUpdate)
	{
		if (!m_initialized)
		{
			m_initialized = true;
			SetDefaultTextEditor();
		}

		// If a compile message was sent during compilation, open when Editor is fully initialized
		if (!m_compileMessage.empty())
		{
			GetIEditor()->OpenView("C# Output");
			DrxLogAlways(m_compileMessage);
			m_compileMessage.clear();
		}

		if (m_isSandboxInFocus)
		{
			UpdatePluginsAndSolution();
		}
	}
}

void CCSharpEditorPlugin::UpdatePluginsAndSolution()
{
	if (!m_changedFiles.empty())
	{
		RegenerateSolution();
		m_changedFiles.clear();
	}

	if (m_reloadPlugins)
	{
		ReloadPlugins();
		m_reloadPlugins = false;
	}
}

void CCSharpEditorPlugin::RegenerateSolution() const
{
	IProjectManager* pProjectManager = gEnv->pSystem->GetIProjectManager();
	if (!pProjectManager)
	{
		return;
	}

	tukk szAssetsDirectory = pProjectManager->GetCurrentAssetDirectoryRelative();
	tukk szDirectory = pProjectManager->GetCurrentProjectDirectoryAbsolute();
	std::vector<string> sourceFiles;
	FindSourceFilesInDirectoryRecursive(szAssetsDirectory, "*.cs", sourceFiles);
	if (sourceFiles.size() == 0)
	{
		return;
	}

	string includes;
	for (const string& sourceFile : sourceFiles)
	{
		string sourceFileRelativePath = sourceFile;

		const auto fullpath = PathUtil::ToUnixPath(sourceFile.c_str());
		const auto rootDataFolder = PathUtil::ToUnixPath(PathUtil::AddSlash(szDirectory));
		if (fullpath.length() > rootDataFolder.length() && strnicmp(fullpath.c_str(), rootDataFolder.c_str(), rootDataFolder.length()) == 0)
		{
			sourceFileRelativePath = fullpath.substr(rootDataFolder.length(), fullpath.length() - rootDataFolder.length());
		}

		includes += "    <Compile Include=\"" + PathUtil::ToDosPath(sourceFileRelativePath) + "\" />\n";
	}

	string pluginReferences;
	u16 pluginCount = pProjectManager->GetPluginCount();
	for (u16 i = 0; i < pluginCount; ++i)
	{
		Drx::IPluginManager::EType type;
		DynArray<EPlatform> platforms;
		string pluginPath;
		pProjectManager->GetPluginInfo(i, type, pluginPath, platforms);
		if (type != Drx::IPluginManager::EType::Managed)
		{
			continue;
		}

		bool include = platforms.empty();
		if (!include)
		{
			for (EPlatform platform : platforms)
			{
				include = platform == EPlatform::Current;
				if (include)
				{
					break;
				}
			}
		}

		if (include)
		{
			string pluginName = PathUtil::GetFileName(pluginPath);
			pluginReferences += "    <Reference Include=\"" + pluginName + "\">\n"
			                    "      <HintPath>" + pluginPath + "</HintPath>\n"
			                    "      <Private>False</Private>\n"
			                    "    </Reference>\n";
		}
	}

	string csProjName = "Sandbox.Game";
	string csProjFilename = csProjName + ".csproj";

	string projectFilePath = PathUtil::Make(szDirectory, csProjFilename.c_str());
	CDrxFile projectFile(projectFilePath.c_str(), "wb", IDrxPak::FLAGS_NO_LOWCASE);
	if (projectFile.GetHandle() != nullptr)
	{
		DrxGUID guid = pProjectManager->GetCurrentProjectGUID();
		string projectName = pProjectManager->GetCurrentProjectName();
		string projectFilePath = pProjectManager->GetProjectFilePath();
		string assemblyName = gEnv->pMonoRuntime->GetGeneratedAssemblyName();
		string projectFileContents = pProjectManager->LoadTemplateFile("%ENGINE%/EngineAssets/Templates/ManagedProjectTemplate.csproj.txt", [guid, projectName, projectFilePath, szDirectory, assemblyName, includes, pluginReferences](tukk szAlias) -> string
		{
			if (!strcmp(szAlias, "csproject_guid"))
			{
			  char buff[40];
			  guid.ToString(buff);

			  return buff;
			}
			else if (!strcmp(szAlias, "project_name"))
			{
			  return projectName;
			}
			else if (!strcmp(szAlias, "assembly_name"))
			{
			  return assemblyName;
			}
			else if (!strcmp(szAlias, "engine_bin_directory"))
			{
			  char szEngineExecutableFolder[_MAX_PATH];
			  DrxGetExecutableFolder(DRX_ARRAY_COUNT(szEngineExecutableFolder), szEngineExecutableFolder);

			  return szEngineExecutableFolder;
			}
			else if (!strcmp(szAlias, "project_file"))
			{
			  return projectFilePath;
			}
			else if (!strcmp(szAlias, "output_path"))
			{
			  return PathUtil::Make(szDirectory, "bin");
			}
			else if (!strcmp(szAlias, "includes"))
			{
			  return includes;
			}
			else if (!strcmp(szAlias, "managed_plugin_references"))
			{
			  return pluginReferences;
			}

			DRX_ASSERT_MESSAGE(false, "Unhandled alias!");
			return "";
		});

		projectFile.Write(projectFileContents.data(), projectFileContents.size());

		string solutionFilePath = GetCSharpSolutionPath();
		CDrxFile solutionFile(solutionFilePath.c_str(), "wb", IDrxPak::FLAGS_NO_LOWCASE);
		if (solutionFile.GetHandle() != nullptr)
		{
			DrxGUID guid = pProjectManager->GetCurrentProjectGUID();
			string solutionFileContents = pProjectManager->LoadTemplateFile("%ENGINE%/EngineAssets/Templates/ManagedSolutionTemplate.sln.txt", [guid, csProjFilename, csProjName](tukk szAlias) -> string
			{
				if (!strcmp(szAlias, "project_name"))
				{
				  return csProjName;
				}
				else if (!strcmp(szAlias, "csproject_name"))
				{
				  return csProjFilename;
				}
				else if (!strcmp(szAlias, "csproject_guid"))
				{
				  return guid.ToString().MakeUpper();
				}
				else if (!strcmp(szAlias, "solution_guid"))
				{
				  // Normally the solution guid would be a GUID that is deterministic but unique to the build tree.
				  return "0C7CC5CD-410D-443B-8223-108F849EAA5C";
				}

				DRX_ASSERT_MESSAGE(false, "Unhandled alias!");
				return "";
			});

			solutionFile.Write(solutionFileContents.data(), solutionFileContents.size());
		}
		else
		{
			DRX_ASSERT_MESSAGE(false, "Unable to create C# solution file!");
		}
	}
	else
	{
		DRX_ASSERT_MESSAGE(false, "Unable to create C# project file!");
	}
}

void CCSharpEditorPlugin::FindSourceFilesInDirectoryRecursive(tukk szDirectory, tukk szExtension, std::vector<string>& sourceFiles) const
{
	string searchPath = PathUtil::Make(szDirectory, szExtension);

	_finddata_t fd;
	intptr_t handle = gEnv->pDrxPak->FindFirst(searchPath, &fd, IDrxPak::FLAGS_NEVER_IN_PAK);
	if (handle != -1)
	{
		do
		{
			sourceFiles.emplace_back(PathUtil::Make(szDirectory, fd.name));
		}
		while (gEnv->pDrxPak->FindNext(handle, &fd) >= 0);

		gEnv->pDrxPak->FindClose(handle);
	}

	// Find additional directories
	searchPath = PathUtil::Make(szDirectory, "*.*");

	handle = gEnv->pDrxPak->FindFirst(searchPath, &fd, IDrxPak::FLAGS_NEVER_IN_PAK);
	if (handle != -1)
	{
		do
		{
			if (fd.attrib & _A_SUBDIR)
			{
				if (strcmp(fd.name, ".") != 0 && strcmp(fd.name, "..") != 0)
				{
					string sDirectory = PathUtil::Make(szDirectory, fd.name);

					FindSourceFilesInDirectoryRecursive(sDirectory, szExtension, sourceFiles);
				}
			}
		}
		while (gEnv->pDrxPak->FindNext(handle, &fd) >= 0);

		gEnv->pDrxPak->FindClose(handle);
	}
}

void CCSharpEditorPlugin::ReloadPlugins() const
{
	if (gEnv->pMonoRuntime != nullptr)
	{
		gEnv->pMonoRuntime->ReloadPluginDomain();
	}
}

bool CCSharpEditorPlugin::OpenCSharpFile(const string& filePath)
{
	if (gEditorFilePreferences.openCSharpSolution && OpenFileInSolution(filePath))
	{
		return true;
	}

	if (OpenFileInTextEditor(filePath))
	{
		return true;
	}
	return OpenCSharpFileSafe(filePath);
}

bool CCSharpEditorPlugin::OpenCSharpFile(const string& filePath, i32k line)
{
	if (gEditorFilePreferences.openCSharpSolution && OpenFileInSolution(filePath, line))
	{
		return true;
	}

	if (OpenFileInTextEditor(filePath, line))
	{
		return true;
	}
	return OpenCSharpFileSafe(filePath);
}

bool CCSharpEditorPlugin::OpenCSharpSolution()
{
	// If a text editor is already open we assume it's still running our solution.
	if (HasExistingTextEditor())
	{
		return true;
	}

	string textEditor = gEditorFilePreferences.textEditorCSharp;
	// No active process found, so open the solution file with the selected text editor.
	if (!textEditor.empty())
	{
		string solutionFile = GetCSharpSolutionPath();
		SHELLEXECUTEINFO shellInfo = SHELLEXECUTEINFO();
		shellInfo.lpVerb = "open";
		shellInfo.lpFile = textEditor.c_str();
		shellInfo.lpParameters = string().Format("\"%s\"", solutionFile).c_str();
		shellInfo.nShow = SW_SHOWNORMAL;
		shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shellInfo.cbSize = sizeof(shellInfo);

		if (!ShellExecuteEx(&shellInfo) || (DWORD_PTR)shellInfo.hInstApp <= 32)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to open the C# solution with %s!", textEditor);
			return false;
		}

		m_textEditorHandle = shellInfo.hProcess;
		m_createdTextEditor = textEditor;
		return true;
	}
	return false;
}

bool CCSharpEditorPlugin::OpenFileInSolution(const string& filePath)
{
	if (HasExistingTextEditor())
	{
		if (OpenFileInTextEditor(filePath))
		{
			return true;
		}
	}

	string textEditor = gEditorFilePreferences.textEditorCSharp;
	// No active process found, so open the solution file with the selected text editor.
	if (!textEditor.empty())
	{
		string solutionFile = GetCSharpSolutionPath();
		SHELLEXECUTEINFO shellInfo = SHELLEXECUTEINFO();
		shellInfo.lpVerb = "open";
		shellInfo.lpFile = textEditor.c_str();
		shellInfo.lpParameters = string().Format("\"%s\" \"%s\"", solutionFile, filePath).c_str();
		shellInfo.nShow = SW_SHOWNORMAL;
		shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shellInfo.cbSize = sizeof(shellInfo);

		if (!ShellExecuteEx(&shellInfo) || (DWORD_PTR)shellInfo.hInstApp <= 32)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to open file %s and the C# solution with \"%s\"!", filePath, textEditor);
			return false;
		}

		m_textEditorHandle = shellInfo.hProcess;
		m_createdTextEditor = textEditor;
		return true;
	}
	return false;
}

bool CCSharpEditorPlugin::OpenFileInSolution(const string& filePath, i32k line)
{
	string textEditor = gEditorFilePreferences.textEditorCSharp;
	string commandFormat = "";

	if (textEditor.find("XamarinStudio.exe") != string::npos)
	{
		commandFormat.Format(";%i;0", line);
	}
	else
	{
		// Opening a file on a specific line is not supported on all platforms. Instead only the file itself is opened.
		return OpenFileInSolution(filePath);
	}

	if (HasExistingTextEditor())
	{
		if (OpenFileInTextEditor(filePath, line))
		{
			return true;
		}
	}

	// No active process found, so open the solution file with the selected text editor.
	if (!textEditor.empty())
	{
		string solutionFile = GetCSharpSolutionPath();
		SHELLEXECUTEINFO shellInfo = SHELLEXECUTEINFO();
		shellInfo.lpVerb = "open";
		shellInfo.lpFile = textEditor.c_str();
		shellInfo.lpParameters = string().Format("\"%s\" \"%s\"%s", solutionFile, filePath, commandFormat).c_str();
		shellInfo.nShow = SW_SHOWNORMAL;
		shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shellInfo.cbSize = sizeof(shellInfo);

		if (!ShellExecuteEx(&shellInfo) || (DWORD_PTR)shellInfo.hInstApp <= 32)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to open file %s and the C# solution with \"%s\"!", filePath, textEditor);
			return false;
		}

		m_textEditorHandle = shellInfo.hProcess;
		m_createdTextEditor = textEditor;
		return true;
	}
	return false;
}

bool CCSharpEditorPlugin::OpenFileInTextEditor(const string& filePath) const
{
	string textEditor = gEditorFilePreferences.textEditorCSharp;
	// No active process found, so open the solution file with the selected text editor.
	if (!textEditor.empty())
	{
		SHELLEXECUTEINFO shellInfo = SHELLEXECUTEINFO();
		shellInfo.lpVerb = "open";
		shellInfo.lpFile = textEditor.c_str();
		shellInfo.lpParameters = string().Format("\"%s\" /edit", filePath).c_str();
		shellInfo.nShow = SW_SHOWNORMAL;
		shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shellInfo.cbSize = sizeof(shellInfo);

		if (!ShellExecuteEx(&shellInfo) || (DWORD_PTR)shellInfo.hInstApp <= 32)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to open file %s and the C# solution with \"%s\"!", filePath, textEditor);
			return false;
		}
		return true;
	}
	return false;
}

bool CCSharpEditorPlugin::OpenFileInTextEditor(const string& filePath, i32k line) const
{
	string textEditor = gEditorFilePreferences.textEditorCSharp;
	string commandFormat = "";

	if (textEditor.find("XamarinStudio.exe") != string::npos)
	{
		commandFormat.Format(";%i;0", line);
	}
	else
	{
		// Opening a file on a specific line is not supported on all platforms. Instead only the file itself is opened.
		return OpenFileInTextEditor(filePath);
	}

	// No active process found, so open the solution file with the selected text editor.
	if (!textEditor.empty())
	{
		SHELLEXECUTEINFO shellInfo = SHELLEXECUTEINFO();
		shellInfo.lpVerb = "open";
		shellInfo.lpFile = textEditor.c_str();
		shellInfo.lpParameters = string().Format("\"%s\"%s /edit", filePath, commandFormat).c_str();
		shellInfo.nShow = SW_SHOWNORMAL;
		shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shellInfo.cbSize = sizeof(shellInfo);

		if (!ShellExecuteEx(&shellInfo) || (DWORD_PTR)shellInfo.hInstApp <= 32)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to open file %s and the C# solution with \"%s\"!", filePath, textEditor);
			return false;
		}
		return true;
	}
	return false;
}

BOOL CALLBACK SetWindowToForeground(HWND hwnd, LPARAM lparam)
{
	DWORD processId = lparam;
	DWORD id = 0;
	GetWindowThreadProcessId(hwnd, &id);

	if (id == processId)
	{
		SetForegroundWindow(hwnd);
		return false;
	}
	return true;
}

bool CCSharpEditorPlugin::HasExistingTextEditor() const
{
	string textEditor = gEditorFilePreferences.textEditorCSharp;

	// The user could've set a new preferred text editor. In that case disregard the old text editor.
	if (textEditor != m_createdTextEditor)
	{
		return false;
	}

	// Try to set the window to the foreground first if it's still active. Otherwise, open a new window with the solution.
	DWORD exitCode;
	if (m_textEditorHandle && GetExitCodeProcess(m_textEditorHandle, &exitCode))
	{
		if (exitCode == STILL_ACTIVE)
		{
			DWORD processId = GetProcessId(m_textEditorHandle);
			// Invert the value because EnumWindows will return false if SetWindowToForeground found the window, and true if it didn't.
			return !EnumWindows(&SetWindowToForeground, processId);

		}
	}
	return false;
}

bool CCSharpEditorPlugin::OpenCSharpFileSafe(const string& filePath) const
{
	HINSTANCE hInst = ShellExecute(NULL, "open", filePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	if ((DWORD_PTR)hInst > 32)
	{
		return true;
	}

	tukk szDialog = "Can't open the file. "
	                       "You can specify a source editor in Sandbox Preferences or create an association in Windows. "
	                       "Do you want to open the file in the Notepad?";
	if (QDialogButtonBox::StandardButton::Yes == CQuestionDialog::SQuestion(QObject::tr(""),
	                                                                        QObject::tr(szDialog),
	                                                                        QDialogButtonBox::StandardButton::Yes | QDialogButtonBox::StandardButton::No))
	{
		hInst = ShellExecute(NULL, "open", "notepad", filePath.c_str(), NULL, SW_SHOWNORMAL);
		return (DWORD_PTR)hInst > 32;
	}
	return false;
}
