// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxSystem/File/IFileChangeMonitor.h>
#include <DrxMono/IMonoRuntime.h>
#include <DrxSchematyc/CoreAPI.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <vector>

#include <IPlugin.h>

struct ICSharpMessageListener
{
	virtual ~ICSharpMessageListener() {}

	virtual void OnMessagesUpdated(string messages) = 0;
};

typedef CListenerSet<ICSharpMessageListener*> CSharpMessageListeners;

class CCSharpEditorPlugin final
	: public IPlugin,
	  public Schematyc::IEnvRegistryListener,
	  public IFileChangeListener,
	  public IMonoCompileListener,
	  public ISystemEventListener,
	  public IAutoEditorNotifyListener
{
public:
	// IPlugin
	i32       GetPluginVersion() override     { return 1; }
	tukk GetPluginName() override        { return "CSharp Editor"; }
	tukk GetPluginDescription() override { return "CSharp Editor plugin"; }
	// ~IPlugin

	CCSharpEditorPlugin();
	virtual ~CCSharpEditorPlugin();

	virtual void   RegisterMessageListener(ICSharpMessageListener* pListener)   { m_messageListeners.Add(pListener); }
	virtual void   UnregisterMessageListener(ICSharpMessageListener* pListener) { m_messageListeners.Remove(pListener); }
	virtual string GetCompileMessage()                                          { return m_compileMessage; }

	// Returns the path to the C# solution file.
	tukk GetCSharpSolutionPath() const { return m_csharpSolutionPath; };

	// Opens the specified file in the editor that's set in the preferences.
	bool OpenCSharpFile(const string& filePath);
	// Opens the specified file at the specified line in the editor that's set in the preferences.
	bool OpenCSharpFile(const string& filePath, i32k line);

	// Opens the CompiledMonoLibrary solution in the text editor specified in the File Preferences. Returns false if the file couldn't be opened.
	bool OpenCSharpSolution();

	// IFileChangeListener
	virtual void OnFileChange(tukk sFilename, EChangeType eType) override;
	// ~IFileChangeListener

	// IMonoCompileListener
	virtual void OnCompileFinished(tukk szCompileMessage) override;
	// ~IMonoCompileListener

	// Schematyc::IEnvRegistryListener
	virtual void OnEnvElementAdd(Schematyc::IEnvElementPtr pElement) override;
	virtual void OnEnvElementDelete(Schematyc::IEnvElementPtr pElement) override;
	// ~Schematyc::IEnvRegistryListener

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IAutoEditorNotifyListener
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent aEventId) override;
	// ~IAutoEditorNotifyListener

	static CCSharpEditorPlugin* GetInstance() { return s_pInstance; }

private:
	bool                   m_initialized = false;
	HANDLE                 m_textEditorHandle;
	string                 m_createdTextEditor;
	std::vector<string>    m_changedFiles;
	bool                   m_reloadPlugins = false;
	bool                   m_isSandboxInFocus = true;
	string                 m_compileMessage;
	CSharpMessageListeners m_messageListeners;
	string                 m_csharpSolutionPath;

	// Checks the machine for an installation of VS2017+, and sets this as the default text editor.
	void SetDefaultTextEditor();
	// Updates the plugins and solutions if required.
	void UpdatePluginsAndSolution();
	// Reloads the managed plugings.
	void ReloadPlugins() const;
	// Regenerates the .sln and .csproj files of the Sandbox C# files.
	void RegenerateSolution() const;
	// Find files with a specific extension in a folder recursively.
	void FindSourceFilesInDirectoryRecursive(tukk szDirectory, tukk szExtension, std::vector<string>& sourceFiles) const;
	// Checks if m_textEditorHandle refers to a valid application, and focuses the window if it does.
	bool HasExistingTextEditor() const;
	// Opens the specified file in the C# solution based. Returns false if the file couldn't be opened.
	bool OpenFileInSolution(const string& filePath);
	// Opens the specified file on a specific line in the C# solution. Returns false if the file couldn't be opened.
	bool OpenFileInSolution(const string& filePath, i32k line);
	// Opens the specified file in the text editor specified in the File Preferences. Returns false if the file couldn't be opened.
	bool OpenFileInTextEditor(const string& filePath) const;
	// Opens the specified file on a specific line in the text editor specified in the File Preferences. Returns false if the file couldn't be opened.
	bool OpenFileInTextEditor(const string& filePath, i32k line) const;
	// Opens the specified file by Windows association. If that fails it will try to open it in Notepad instead.
	bool OpenCSharpFileSafe(const string& filePath) const;

private:
	static CCSharpEditorPlugin* s_pInstance;
};

REGISTER_PLUGIN(CCSharpEditorPlugin)

