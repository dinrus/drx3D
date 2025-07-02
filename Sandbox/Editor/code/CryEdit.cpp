// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>

#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")

#include <drx3D/CoreX/DrxCustomTypes.h>
#include "DrxEdit.h"

#include "GameExporter.h"
#include "GameResourcesExporter.h"

#include "DrxEditDoc.h"
#include "Dialogs/QStringDialog.h"
#include "Dialogs/ToolbarDialog.h"
#include "LinkTool.h"
#include "AlignTool.h"
#include "MissionScript.h"
#include "LevelEditor/NewLevelDialog.h"
#include "LevelEditor/LevelEditorViewport.h"
#include "PhysTool.h"
#include "Vegetation/VegetationMap.h"
#include <Preferences/GeneralPreferences.h>
#include "MainThreadWorker.h"
#include "Terrain/TerrainManager.h"
#include "Terrain/Heightmap.h"

#include "ProcessInfo.h"

#include "ViewManager.h"
#include "ModelViewport.h"
#include "RenderViewport.h"
#include <Preferences/ViewportPreferences.h>

#include "PluginManager.h"

#include "Objects/Group.h"
#include "Objects/CameraObject.h"
#include "Objects/EntityScript.h"
#include "Objects/ObjectLoader.h"
#include "Objects/PrefabObject.h"
#include "Prefabs/PrefabManager.h"

#include "IEditorImpl.h"
#include "SplashScreen.h"
#include "Grid.h"

#include "ObjectCloneTool.h"

#include "Mission.h"
#include "MissionSelectDialog.h"
#include "IUndoManager.h"
#include "MissionProps.h"

#include "GameEngine.h"

#include "AI\AIManager.h"

#include "Geometry\EdMesh.h"

#include <io.h>
#include <DrxScriptSystem/IScriptSystem.h>
#include <DrxEntitySystem/IEntitySystem.h>
#include <Drx3DEngine/I3DEngine.h>
#include <drx3D/Sys/ITimer.h>
#include <DrxGame/IGame.h>
#include <DrxGame/IGameFramework.h>
#include <IItemSystem.h>
#include <DrxAnimation/IDrxAnimation.h>
#include <DrxPhysics/IPhysics.h>
#include <drx3D/Act/IGameRulesSystem.h>
#include <IBackgroundScheduleManager.h>
#include <DrxSandbox/IEditorGame.h>
#include "IDevManager.h"

#include "Dialogs/QNumericBoxDialog.h"

#include "DrxEdit.h"
#include "ShaderCache.h"
#include "GotoPositionDlg.h"
#include "FilePathUtil.h"

#include <drx3D/CoreX/String/StringUtils.h>

#include <DrxSandbox/ScopedVariableSetter.h>

#include "Util/EditorAutoLevelLoadTest.h"
#include "Util/Ruler.h"
#include "Util/IndexedFiles.h"

#include "ResourceCompilerHelpers.h"

#include "Mannequin/MannequinChangeMonitor.h"

#include "Util/BoostPythonHelpers.h"
#include "Export/ExportManager.h"

#include "LevelIndependentFileMan.h"
#include "Objects/ObjectLayerManager.h"
#include "Dialogs/DuplicatedObjectsHandlerDlg.h"
#include "IconManager.h"
#include "UI/UIManager.h"
#include "EditMode/VertexSnappingModeTool.h"

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>
#include <drx3D/CoreX/Platform/WindowsUtils.h>

#include <DrxAISystem/IAIObjectManager.h>

#include <drx3D/Sys/Profilers/IStatoscope.h>

#include "QToolWindowManager/QToolWindowManager.h"
#include "Controls/QuestionDialog.h"
#include "QFullScreenWidget.h"

#include "QT/Widgets/QWaitProgress.h"

#include "FileDialogs/SystemFileDialog.h"
#include "ConfigurationManager.h"

#include <Notifications/NotificationCenter.h>
#include <QtUtil.h>

#include "QT/QtMainFrame.h"
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QUrl>

#include <drx3D/Sys/IDrxLink.h>

#include "Material/MaterialManager.h"
#include "AssetSystem/AssetManager.h"

//////////////////////////////////////////////////////////////////////////
namespace
{

string PyGetGameFolder()
{
	string gameFolder = PathUtil::Make(PathUtil::GetEnginePath(), PathUtil::GetGameFolder()).c_str();
	return gameFolder;
}

void PyOpenLevel(tukk pLevelName)
{
	CDrxEditApp::GetInstance()->LoadLevel(pLevelName);
}

void PyOpenLevelNoPrompt(tukk pLevelName)
{
	CDrxEditApp::GetInstance()->DiscardLevelChanges();
	PyOpenLevel(pLevelName);
}

i32 PyCreateLevel(tukk levelName, i32 resolution, float unitSize, bool bUseTerrain)
{
	CAssetType* pLevelType = GetIEditorImpl()->GetAssetManager()->FindAssetType("Level");
	DRX_ASSERT(pLevelType);
	const string levelPath = string().Format("%s.level.cryasset", levelName);

	CLevelType::SCreateParams params {};
	params.resolution = resolution;
	params.unitSize = unitSize;
	params.bUseTerrain = bUseTerrain;
	return pLevelType->Create(levelPath, &params) ? CDrxEditApp::ECLR_OK : CDrxEditApp::ECLR_DIR_CREATION_FAILED;
}

tukk PyGetCurrentLevelName()
{
	return GetIEditorImpl()->GetGameEngine()->GetLevelName();
}

tukk PyGetCurrentLevelPath()
{
	return GetIEditorImpl()->GetGameEngine()->GetLevelPath();
}

void Command_UnloadPlugins()
{
	GetIEditorImpl()->GetPluginManager()->UnloadAllPlugins();
}

void Command_LoadPlugins()
{
	Command_UnloadPlugins();

	char path[MAX_PATH];
	DrxGetExecutableFolder(DRX_ARRAY_COUNT(path), path);
	const string executableFilePath = PathUtil::Make(path, "EditorPlugins\\*.dll");
	GetIEditorImpl()->GetPluginManager()->LoadPlugins(executableFilePath.c_str());
}

boost::python::tuple PyGetCurrentViewPosition()
{
	Vec3 pos = GetIEditorImpl()->GetSystem()->GetViewCamera().GetPosition();
	return boost::python::make_tuple(pos.x, pos.y, pos.z);
}

boost::python::tuple PyGetCurrentViewRotation()
{
	Ang3 ang = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(GetIEditorImpl()->GetSystem()->GetViewCamera().GetMatrix())));
	return boost::python::make_tuple(ang.x, ang.y, ang.z);
}

void PySetCurrentViewPosition(float x, float y, float z)
{
	CViewport* pRenderViewport = GetIEditorImpl()->GetViewManager()->GetGameViewport();
	if (pRenderViewport)
	{
		Matrix34 tm = pRenderViewport->GetViewTM();
		tm.SetTranslation(Vec3(x, y, z));
		pRenderViewport->SetViewTM(tm);
	}
}

void PySetCurrentViewRotation(float x, float y, float z)
{
	CViewport* pRenderViewport = GetIEditorImpl()->GetViewManager()->GetGameViewport();
	if (pRenderViewport)
	{
		Matrix34 tm = pRenderViewport->GetViewTM();
		tm.SetRotationXYZ(Ang3(DEG2RAD(x), DEG2RAD(y), DEG2RAD(z)), tm.GetTranslation());
		pRenderViewport->SetViewTM(tm);
	}
}
}

REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PyOpenLevel, general, open_level,
                                     "Opens a level.",
                                     "general.open_level(str levelName)");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PyOpenLevelNoPrompt, general, open_level_no_prompt,
                                     "Opens a level. Doesn't prompt user about saving a modified level",
                                     "general.open_level_no_prompt(str levelName)");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PyCreateLevel, general, create_level,
                                     "Creates a level with the parameters of 'levelName', 'resolution', 'unitSize' and 'bUseTerrain'.",
                                     "general.create_level(str levelName, i32 resolution, float unitSize, bool useTerrain)");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PyGetGameFolder, general, get_game_folder,
                                     "Gets the path to the Game folder of current project.",
                                     "general.get_game_folder()");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PyGetCurrentLevelName, general, get_current_level_name,
                                     "Gets the name of the current level.",
                                     "general.get_current_level_name()");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PyGetCurrentLevelPath, general, get_current_level_path,
                                     "Gets the fully specified path of the current level.",
                                     "general.get_current_level_path()");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(Command_LoadPlugins, general, load_all_plugins,
                                     "Loads all available plugins.",
                                     "general.load_all_plugins()");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(Command_UnloadPlugins, general, unload_all_plugins,
                                     "Unloads all available plugins.",
                                     "general.unload_all_plugins()");
REGISTER_ONLY_PYTHON_COMMAND_WITH_EXAMPLE(PyGetCurrentViewPosition, general, get_current_view_position,
                                          "Returns the position of the current view as a list of 3 floats.",
                                          "general.get_current_view_position()");
REGISTER_ONLY_PYTHON_COMMAND_WITH_EXAMPLE(PyGetCurrentViewRotation, general, get_current_view_rotation,
                                          "Returns the rotation of the current view as a list of 3 floats, each of which represents x, y, z Euler angles.",
                                          "general.get_current_view_rotation()");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PySetCurrentViewPosition, general, set_current_view_position,
                                     "Sets the position of the current view as given x, y, z coordinates.",
                                     "general.set_current_view_position(float xValue, float yValue, float zValue)");
REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(PySetCurrentViewRotation, general, set_current_view_rotation,
                                     "Sets the rotation of the current view as given x, y, z Euler angles.",
                                     "general.set_current_view_rotation(float xValue, float yValue, float zValue)");

/////////////////////////////////////////////////////////////////////////////
// The one and only CDrxEditApp object
//////////////////////////////////////////////////////////////////////////
CDrxEditApp theApp;

enum
{
	DefaultExportSettings_ExportToPC      = true,
	DefaultExportSettings_ExportToConsole = false,
};

const UINT kDisplayMenuIndex          = 3;
const UINT kRememberLocationMenuIndex = 12;
const UINT kGotoLocationMenuIndex     = 13;

#define ERROR_LEN 256

namespace
{
CDrxEditApp* s_pCDrxEditApp = 0;
}
CDrxEditApp* CDrxEditApp::GetInstance()
{
	return s_pCDrxEditApp;
}

/////////////////////////////////////////////////////////////////////////////

class CTagLocationsSerializer
{
private:
	struct STag
	{
		STag()
			: position(0.0f, 0.0f, 0.0f)
			, angles(0.0f, 0.0f, 0.0f)
			, segment(0, 0)
		{
		}

		STag(const Vec3& _position, const Ang3& _angles, const Vec2i& _segment)
			: position(_position)
			, angles(_angles)
			, segment(_segment)
		{
		}

		void Serialize(Serialization::IArchive& archive)
		{
			archive(position, "position");
			archive(angles, "angles");
			archive(segment, "segment");
		}

		Vec3  position;
		Ang3  angles;
		Vec2i segment;
	};
	typedef std::vector<STag> TTags;

public:
	CTagLocationsSerializer()
	{
		m_tags.reserve(12);
	}

	void AddTag(const Vec3& position, const Ang3& angles, const Vec2i& segment)
	{
		m_tags.push_back(STag(position, angles, segment));
	}

	void Serialize(Serialization::IArchive& archive)
	{
		archive(m_tags, "tags");
	}

private:
	TTags m_tags;
};

/////////////////////////////////////////////////////////////////////////////
// CDrxEditApp construction
CDrxEditApp::CDrxEditApp()
{
	s_pCDrxEditApp = this;
	m_mutexApplication = NULL;

	drx_strcpy(m_sPreviewFile, "");

#ifdef _DEBUG
	i32 tmpDbgFlag;
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	// Clear the upper 16 bits and OR in the desired freqency
	tmpDbgFlag = (tmpDbgFlag & 0x0000FFFF) | (32768 << 16);
	//tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);

	// Check heap every
	//_CrtSetBreakAlloc(119065);
#endif

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pEditor = 0;
	m_bExiting = false;
	m_bPreviewMode = false;
	m_bConsoleMode = false;
	m_bTestMode = false;
	m_bPrecacheShaderList = false;
	m_bStatsShaderList = false;
	m_bMergeShaders = false;
	m_bLevelLoadTestMode = false;
	m_suspendUpdate = false;

	ZeroStruct(m_tagLocations);
	ZeroStruct(m_tagAngles);
	ZeroStruct(m_tagSegmentsXY);

	m_fastRotateAngle = 45;
	m_moveSpeedStep = 0.1f;

	m_bForceProcessIdle = false;
	m_bKeepEditorActive = false;

	m_initSegmentsToOpen = 0;
}

//////////////////////////////////////////////////////////////////////////
CDrxEditApp::~CDrxEditApp()
{
}

class CEditCommandLineInfo
{
public:
	i32    m_paramNum;
	i32    m_exportParamNum;
	bool   m_bTest;
	bool   m_bAutoLoadLevel;
	bool   m_bExport;
	bool   m_bMatEditMode;
	bool   m_bExportTexture;
	bool   m_bPrecacheShaders;
	bool   m_bPrecacheShadersLevels;
	bool   m_bPrecacheShaderList;
	bool   m_bStatsShaders;
	bool   m_bStatsShaderList;
	bool   m_bMergeShaders;
	bool   m_bExportAI;
	bool   m_bConsoleMode;
	bool   m_bDeveloperMode;
	bool   m_bMemReplay;
	bool   m_bRunPythonScript;
	string m_file;
	string m_gameCmdLine;
	string m_swCmdLine;
	string m_strFileName;

	CEditCommandLineInfo()
		: m_exportParamNum(-1)
		, m_paramNum(0)
		, m_bExport(false)
		, m_bMatEditMode(false)
		, m_bPrecacheShaders(false)
		, m_bPrecacheShadersLevels(false)
		, m_bPrecacheShaderList(false)
		, m_bStatsShaders(false)
		, m_bStatsShaderList(false)
		, m_bMergeShaders(false)
		, m_bTest(false)
		, m_bAutoLoadLevel(false)
		, m_bExportTexture(false)
		, m_bExportAI(false)
		, m_bConsoleMode(false)
		, m_bDeveloperMode(false)
		, m_bMemReplay(false)
		, m_bRunPythonScript(false)
	{
		i32 argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (!argv)
			return;

		for (i32 i = 0; i < argc; ++i)
		{
			if ((*argv[i] == L'-') || (*argv[i] == L'/'))
			{
				ParseParam(argv[i] + 1, true);
			}
			else if (m_strFileName.IsEmpty()) // first non - flag parameter on the command line.
			{
				m_strFileName = DrxStringUtils::WStrToUTF8(argv[i]);
			}
			else
			{
				ParseParam(argv[i], false);
			}
		}

		LocalFree(argv);
	}

private:
	void ParseParam(LPWSTR lpszParam, BOOL bFlag)
	{
		if (bFlag && wcsicmp(lpszParam, L"export") == 0)
		{
			m_exportParamNum = m_paramNum;
			m_bExport = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"exportTexture") == 0)
		{
			m_exportParamNum = m_paramNum;
			m_bExportTexture = true;
			m_bExport = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"exportAI") == 0)
		{
			m_exportParamNum = m_paramNum;
			m_bExportAI = true;
			m_bExport = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"test") == 0)
		{
			m_bTest = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"auto_level_load") == 0)
		{
			m_bAutoLoadLevel = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"PrecacheShaders") == 0)
		{
			m_bPrecacheShaders = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"PrecacheShadersLevels") == 0)
		{
			m_bPrecacheShadersLevels = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"PrecacheShaderList") == 0)
		{
			m_bPrecacheShaderList = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"StatsShaders") == 0)
		{
			m_bStatsShaders = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"StatsShaderList") == 0)
		{
			m_bStatsShaderList = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"MergeShaders") == 0)
		{
			m_bMergeShaders = true;
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"VTUNE") == 0)
		{
			m_gameCmdLine += " -VTUNE";
			return;
		}
		else if (bFlag && wcsicmp(lpszParam, L"MatEdit") == 0)
		{
			m_bMatEditMode = true;
		}
		else if (bFlag && wcsicmp(lpszParam, L"BatchMode") == 0)
		{
			m_bConsoleMode = true;
		}
		else if (bFlag && wcsicmp(lpszParam, L"devmode") == 0)
		{
			m_bDeveloperMode = true;
		}
		else if (bFlag && wcsicmp(lpszParam, L"memreplay") == 0)
		{
			m_bMemReplay = true;
		}
		else if (bFlag && wcsicmp(lpszParam, L"runpython") == 0)
		{
			m_bRunPythonScript = true;
		}

		if (!bFlag)
		{
			// otherwise it thinks that command parameters included between [] is a filename and tries to load it.
			if (lpszParam[0] == L'[')
				return;

			m_file = DrxStringUtils::WStrToUTF8(lpszParam);
		}
		m_paramNum++;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CTheApp::FirstInstance
//		FirstInstance checks for an existing instance of the application.
//		If one is found, it is activated.
//
//    This function uses a technique similar to that described in KB
//    article Q141752	to locate the previous instance of the application. .
BOOL CDrxEditApp::FirstInstance(bool bForceNewInstance)
{
	CWnd* pwndFirst = CWnd::FindWindow(_T("DrxEditorClass"), NULL);
	if (pwndFirst && !bForceNewInstance)
	{
		// another instance is already running - activate it
		CWnd* pwndPopup = pwndFirst->GetLastActivePopup();
		pwndFirst->SetForegroundWindow();
		if (pwndFirst->IsIconic())
			pwndFirst->ShowWindow(SW_SHOWNORMAL);
		if (pwndFirst != pwndPopup)
			pwndPopup->SetForegroundWindow();

		if (m_bPreviewMode)
		{
			// IF in preview mode send this window copy data message to load new preview file.
			COPYDATASTRUCT cd;
			ZeroStruct(cd);
			cd.dwData = 100;
			cd.cbData = strlen(m_sPreviewFile);
			cd.lpData = m_sPreviewFile;
			pwndFirst->SendMessage(WM_COPYDATA, 0, (LPARAM)&cd);
		}
		return FALSE;
	}
	else
	{
		// this is the first instance
		// Register your unique class name that you wish to use
		WNDCLASS wndcls;
		ZeroStruct(wndcls);
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		//wndcls.lpfnWndProc = EditorWndProc;
		wndcls.hInstance = AfxGetInstanceHandle();
		//wndcls.hIcon = LoadIcon(0,IDR_MAINFRAME); // or load a different icon.
		//wndcls.hCursor = LoadCursor(0,IDC_ARROW);
		wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wndcls.lpszMenuName = NULL;

		// Specify your own class name for using FindWindow later
		wndcls.lpszClassName = _T("DrxEditorClass");

		// Register the new class and exit if it fails
		if (!AfxRegisterClass(&wndcls))
		{
			TRACE("Class Registration Failed\n");
			return FALSE;
		}
		//		bClassRegistered = TRUE;

		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::InitFromCommandLine(CEditCommandLineInfo& cmdInfo)
{
	//! Setup flags from command line
	if (cmdInfo.m_bPrecacheShaders || cmdInfo.m_bPrecacheShadersLevels || cmdInfo.m_bMergeShaders
	    || cmdInfo.m_bPrecacheShaderList || cmdInfo.m_bStatsShaderList || cmdInfo.m_bStatsShaders)
	{
		m_bPreviewMode = true;
		m_bConsoleMode = true;
		m_bTestMode = true;
	}

	m_bConsoleMode |= cmdInfo.m_bConsoleMode;
	m_bTestMode |= cmdInfo.m_bTest;

	m_bPrecacheShaderList = cmdInfo.m_bPrecacheShaderList;
	m_bStatsShaderList = cmdInfo.m_bStatsShaderList;
	m_bStatsShaders = cmdInfo.m_bStatsShaders;
	m_bPrecacheShaders = cmdInfo.m_bPrecacheShaders;
	m_bPrecacheShadersLevels = cmdInfo.m_bPrecacheShadersLevels;
	m_bMergeShaders = cmdInfo.m_bMergeShaders;
	m_bExportMode = cmdInfo.m_bExport;
	m_bRunPythonScript = cmdInfo.m_bRunPythonScript;

	m_pEditor->SetMatEditMode(cmdInfo.m_bMatEditMode);

	if (m_bExportMode)
	{
		m_exportFile = cmdInfo.m_file;
		m_bTestMode = true;
	}

	// Do we have a passed filename ?
	if (!cmdInfo.m_strFileName.IsEmpty())
	{
		if (!cmdInfo.m_bRunPythonScript && CModelViewport::IsPreviewableFileType(cmdInfo.m_strFileName.GetString()))
		{
			m_bPreviewMode = true;
			drx_strcpy(m_sPreviewFile, cmdInfo.m_strFileName);
		}
	}

	if (cmdInfo.m_bAutoLoadLevel)
	{
		m_bLevelLoadTestMode = true;
		gEnv->bUnattendedMode = true;
		CEditorAutoLevelLoadTest::Instance();
	}

	if (cmdInfo.m_bMemReplay)
	{
		DrxGetIMemReplay()->Start();
	}

}

/////////////////////////////////////////////////////////////////////////////
namespace
{

// Callback class for initialization messages from system.
struct SInitializeUIInfo : IInitializeUIInfo
{
	virtual void SetInfoText(tukk text) override
	{
		SplashScreen::SetText(text);
	}

	static SInitializeUIInfo& GetInstance()
	{
		static SInitializeUIInfo theInstance;
		return theInstance;
	}
};

} //endns

std::unique_ptr<CGameEngine> CDrxEditApp::InitGameSystem()
{
	bool bShaderCacheGen = m_bPrecacheShaderList | m_bPrecacheShaders | m_bPrecacheShadersLevels;

	std::unique_ptr<CGameEngine> pGameEngine = stl::make_unique<CGameEngine>();
	if (!pGameEngine->Init(m_bPreviewMode, m_bTestMode, bShaderCacheGen, DrxStringUtils::ANSIToUTF8(GetCommandLineA()).c_str(), &SInitializeUIInfo::GetInstance()))
	{
		return nullptr;
	}

	return pGameEngine;
}

/////////////////////////////////////////////////////////////////////////////
bool CDrxEditApp::CheckIfAlreadyRunning()
{
	bool bForceNewInstance = false;

	if (!m_bPreviewMode)
	{
		m_mutexApplication = CreateMutex(nullptr, TRUE, "DinrusPro 3DApplication");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			const QString title = "CRYENGINE Sandbox is already running";
			const QString message = "Running multiple instances of the CRYENGINE Sandbox at once is not supported.\nDo you want to open a new instance anyway?";

			if (CQuestionDialog::SWarning(title, message, QDialogButtonBox::Yes | QDialogButtonBox::No) != QDialogButtonBox::Yes)
			{
				return false;
			}

			bForceNewInstance = true;
		}
	}

	// Shader precaching may start multiple editor copies
	if (!FirstInstance(bForceNewInstance) && !m_bPrecacheShaderList)
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CDrxEditApp::InitPlugins()
{
	LOADING_TIME_PROFILE_SECTION;
	SplashScreen::SetText("Loading Plugins...");
	// Load the plugins
	{
		Command_LoadPlugins();
	}
}

////////////////////////////////////////////////////////////////////////////
void CDrxEditApp::InitLevel(CEditCommandLineInfo& cmdInfo)
{
	LOADING_TIME_PROFILE_SECTION;

	if (m_bPreviewMode)
	{
		GetIEditorImpl()->EnableAcceleratos(false);

		// Load geometry object.
		if (!cmdInfo.m_strFileName.IsEmpty())
		{
			LoadFile(cmdInfo.m_strFileName);
		}
	}
	else if (m_bExportMode && !m_exportFile.IsEmpty())
	{
		GetIEditorImpl()->SetModifiedFlag(FALSE);
		CDrxEditDoc* pDocument = LoadLevel(m_exportFile);
		if (pDocument)
		{
			GetIEditorImpl()->SetModifiedFlag(FALSE);
			if (cmdInfo.m_bExportAI)
			{
				GetIEditorImpl()->GetGameEngine()->GenerateAiAll(eExp_AI_All);
			}
			ExportLevel(cmdInfo.m_bExport, cmdInfo.m_bExportTexture, true);
			// Terminate process.
			CLogFile::WriteLine("Editor: Terminate Process after export");
		}
		exit(0);
	}
	else if (stricmp(PathUtil::GetExt(cmdInfo.m_strFileName.GetString()), CLevelType::GetFileExtensionStatic()) == 0)
	{
		CDrxEditDoc* pDocument = LoadLevel(cmdInfo.m_strFileName);
		if (pDocument)
		{
			GetIEditorImpl()->SetModifiedFlag(false);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL CDrxEditApp::InitConsole()
{
	if (m_bPrecacheShaderList)
	{
		GetIEditorImpl()->GetSystem()->GetIConsole()->ExecuteString("r_PrecacheShaderList");
		return FALSE;
	}
	else if (m_bStatsShaderList)
	{
		GetIEditorImpl()->GetSystem()->GetIConsole()->ExecuteString("r_StatsShaderList");
		return FALSE;
	}
	else if (m_bStatsShaders)
	{
		GetIEditorImpl()->GetSystem()->GetIConsole()->ExecuteString("r_StatsShaders");
		return FALSE;
	}
	else if (m_bPrecacheShaders)
	{
		GetIEditorImpl()->GetSystem()->GetIConsole()->ExecuteString("r_PrecacheShaders");
		return FALSE;
	}
	else if (m_bPrecacheShadersLevels)
	{
		GetIEditorImpl()->GetSystem()->GetIConsole()->ExecuteString("r_PrecacheShadersLevels");
		return FALSE;
	}
	else if (m_bMergeShaders)
	{
		GetIEditorImpl()->GetSystem()->GetIConsole()->ExecuteString("r_MergeShaders");
		return FALSE;
	}

	// Execute editor specific config.
	gEnv->pConsole->ExecuteString("exec editor.cfg");

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CDrxEditApp::RunInitPythonScript(CEditCommandLineInfo& cmdInfo)
{
	LOADING_TIME_PROFILE_SECTION;
	if (cmdInfo.m_bRunPythonScript)
	{
		GetIEditorImpl()->ExecuteCommand("general.run_file '%s'", cmdInfo.m_strFileName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDrxEditApp initialization
bool CDrxEditApp::InitInstance()
{
	////////////////////////////////////////////////////////////////////////
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	////////////////////////////////////////////////////////////////////////
	CProcessInfo::LoadPSApi();

	InitCommonControls();    // initialize common control library

	// Init COM services
	CoInitialize(NULL);
	// Initialize RichEditCtrl.
	AfxInitRichEdit2();

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusstartupinput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusstartupinput, NULL);
	//////////////////////////////////////////////////////////////////////////

	// Check for 32bpp
	if (::GetDeviceCaps(GetDC(NULL), BITSPIXEL) != 32)
		CQuestionDialog::SWarning(QObject::tr("Warning"), QObject::tr("WARNING: Your desktop is not set to 32bpp, this might result in unexpected behavior" \
		  "of the editor. Please set your desktop to 32bpp !"));

	if (!CheckIfAlreadyRunning())
	{
		return false;
	}

	if (!CResourceCompilerVersion::CheckIfValid())
	{
		return false;
	}

	CEditCommandLineInfo cmdInfo;
	///ParseCommandLine(cmdInfo);

	auto pGameEngine = InitGameSystem();
	if (!pGameEngine)
	{
		return false;
	}

	LOADING_TIME_PROFILE_SECTION_NAMED("CDrxEditApp::InitInstance() after system");

	m_pEditor = new CEditorImpl(pGameEngine.release());

	const IDevManager* pDevManager = GetIEditorImpl()->GetDevManager();
	if (!pDevManager->HasValidDefaultLicense())
	{
		return false;
	}

	InitFromCommandLine(cmdInfo);

	m_pEditor->Init();

	SplashScreen::SetVersion(m_pEditor->GetFileVersion());

	// check here again for standard dev manager which requires the console to be initialized
	if (!pDevManager->HasValidDefaultLicense())
	{
		return false;
	}

	// Create Sandbox user folder if necessary
	GetISystem()->GetIPak()->MakeDir("%USER%/Sandbox");

	InitPlugins();

	//Must be initialized after plugins
	GetIEditorImpl()->GetAssetManager()->Init();

	m_pChangeMonitor = new CMannequinChangeMonitor();

	CDrxEditDoc* pDoc = new CDrxEditDoc;
	GetIEditorImpl()->SetDocument(pDoc);

	if (IsInRegularEditorMode())
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("CDrxEditApp::InitInstance() - File Indexing");

		CIndexedFiles::Create();

		if (gEnv->pConsole->GetCVar("ed_indexfiles")->GetIVal())
		{
			Log("Started game resource files indexing...");
			CIndexedFiles::StartFileIndexing();
		}
		else
		{
			Log("Game resource files indexing is disabled.");
		}
	}

	if (!GetIEditorImpl()->IsInMatEditMode())
		m_pEditor->InitFinished();

	InitLevel(cmdInfo);

	const ICmdLineArg* pCommandArg = GetISystem()->GetICmdLine()->FindArg(eCLAT_Pre, "edCommand");
	if (nullptr != pCommandArg)
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("CDrxEditApp::InitInstance() - Execute Command");
		GetIEditorImpl()->ExecuteCommand(pCommandArg->GetValue());
	}

	if (!m_bConsoleMode && !m_bPreviewMode)
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("CDrxEditApp::InitInstance() - Update Views");
		GetIEditorImpl()->UpdateViews();
	}

	{
		LOADING_TIME_PROFILE_SECTION_NAMED("CDrxEditApp::InitInstance() - Init Console");
		if (!InitConsole())
		{
			return true;
		}
	}

	{
		LOADING_TIME_PROFILE_SECTION_NAMED("CDrxEditApp::InitInstance() - Load Python Plugins");
		RunInitPythonScript(cmdInfo);
		PyScript::LoadPythonPlugins();
	}

	return true;
}

bool CDrxEditApp::PostInit()
{
	GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_SANDBOX_POST_INIT_DONE, 0, 0);
	return true;
}

void CDrxEditApp::DiscardLevelChanges()
{
	GetIEditorImpl()->GetDocument()->SetModifiedFlag(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::LoadFile(const string& fileName)
{
	//CEditCommandLineInfo cmdLine;
	//ProcessCommandLine(cmdinfo);

	//bool bBuilding = false;
	//string file = cmdLine.SpanExcluding()
	if (GetIEditorImpl()->GetViewManager()->GetViewCount() == 0)
		return;
	CViewport* vp = GetIEditorImpl()->GetViewManager()->GetView(0);
	if (vp->GetType() == ET_ViewportModel)
	{
		((CModelViewport*)vp)->LoadObject(fileName, 1);
	}

	LoadTagLocations();

	GetIEditorImpl()->SetModifiedFlag(FALSE);
}

//////////////////////////////////////////////////////////////////////////
inline void ExtractMenuName(string& str)
{
	// eliminate &
	i32 pos = str.Find('&');
	if (pos >= 0)
	{
		str = str.Left(pos) + str.Right(str.GetLength() - pos - 1);
	}
	// cut the string
	for (i32 i = 0; i < str.GetLength(); i++)
		if (str[i] == 9)
			str = str.Left(i);
}

i32 CDrxEditApp::ExitInstance()
{
	SAFE_DELETE(m_pChangeMonitor);

	CIndexedFiles::Destroy();

	if (GetIEditorImpl())
		GetIEditorImpl()->Notify(eNotify_OnQuit);

	m_bExiting = true;

	HEAP_CHECK

	//////////////////////////////////////////////////////////////////////////
	// Quick end for editor.
	if (gEnv && gEnv->pSystem)
		gEnv->pSystem->Quit();
	//////////////////////////////////////////////////////////////////////////

	if (m_pEditor)
	{
		delete m_pEditor;
		m_pEditor = 0;
	}

	CoUninitialize();

	// save accelerator manager configuration.
	//m_AccelManager.SaveOnExit();

	CProcessInfo::UnloadPSApi();

	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	if (m_mutexApplication)
		CloseHandle(m_mutexApplication);

	return 0;
}

bool CDrxEditApp::IsAnyWindowActive()
{
	return CEditorMainFrame::GetInstance()->GetToolManager()->isAnyWindowActive() ||
	       QFullScreenWidget::IsFullScreenWindowActive() ||
	       nullptr != QApplication::activeWindow() ||
	       CEditorMainFrame::GetInstance()->isActiveWindow();
}

bool CDrxEditApp::IdleProcessing(bool bBackgroundUpdate)
{
	if (m_suspendUpdate)
		return false;

	////////////////////////////////////////////////////////////////////////
	// Call the update function of the engine
	////////////////////////////////////////////////////////////////////////
	if (m_bTestMode && !bBackgroundUpdate)
	{
		// Terminate process.
		CLogFile::WriteLine("Editor: Terminate Process");
		exit(0);
	}

	bool bIsAppWindow = IsAnyWindowActive();
	bool bActive = false;
	bool res = false;
	if (bIsAppWindow || m_bForceProcessIdle || m_bKeepEditorActive)
	{
		res = true;
		bActive = true;
	}
	m_bForceProcessIdle = false;

	// focus changed
	if (m_bPrevActive != bActive)
		GetIEditorImpl()->GetSystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_CHANGE_FOCUS, bActive, 0);

	// process the work schedule - regardless if the app is active or not
	GetIEditorImpl()->GetBackgroundScheduleManager()->Update();

	// if there are active schedules keep updating the application
	if (GetIEditorImpl()->GetBackgroundScheduleManager()->GetNumSchedules() > 0)
	{
		bActive = true;
	}

	// Process main thread tasks, regardless whether the app is active or not.
	if (CMainThreadWorker::GetInstance().TryExecuteNextTask())
	{
		bActive = true;
	}

	m_bPrevActive = bActive;

	if (bActive || (bBackgroundUpdate && !bIsAppWindow))
	{
		// Start profiling frame.
		GetIEditorImpl()->GetSystem()->GetIProfileSystem()->StartFrame();

		if (GetIEditorImpl()->IsInGameMode())
		{
			// Update Game
			GetIEditorImpl()->GetGameEngine()->Update();
		}
		else
		{
			if (!GetIEditorImpl()->IsInMatEditMode())
			{
				//Engine update is disabled during level load
				//This condition will not always hold true and we should aim to remove this,
				//as many things may depend on the engine systems being updated during editor time, no matter what we are doing with level load
				//This was added to temporarily prevent an assert in the entity system
				//TODO: Remove this when the engine supports update during loading better
				if (!GetIEditorImpl()->GetDocument()->IsLevelBeingLoaded())
				{
					GetIEditorImpl()->GetGameEngine()->Update();
				}

				if (m_pEditor)
				{
					m_pEditor->Update();
				}

				// synchronize all animations to ensure that their computation has finished
				GetIEditorImpl()->GetSystem()->GetIAnimationSystem()->SyncAllAnimations();
			}
			GetIEditorImpl()->Notify(eNotify_OnIdleUpdate);
		}

		GetIEditorImpl()->GetSystem()->GetIProfileSystem()->EndFrame();
	}
	else if (GetIEditorImpl()->GetSystem() && GetIEditorImpl()->GetSystem()->GetILog())
		GetIEditorImpl()->GetSystem()->GetILog()->Update(); // print messages from other threads

	return res;
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::ExportLevel(bool bExportToGame, bool bExportTexture, bool bAutoExport)
{
	if (bExportTexture)
	{
		CGameExporter gameExporter;
		gameExporter.SetAutoExportMode(bAutoExport);
		gameExporter.Export(eExp_SurfaceTexture);
	}
	else if (bExportToGame)
	{
		CGameExporter gameExporter;
		gameExporter.SetAutoExportMode(bAutoExport);
		gameExporter.Export();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CDrxEditApp::UserExportToGame(bool bNoMsgBox)
{
	if (!GetIEditorImpl()->GetGameEngine()->IsLevelLoaded())
	{
		if (bNoMsgBox == false)
			CQuestionDialog::SWarning(QObject::tr(""), QObject::tr("Please load a level before attempting to export."));
		return false;
	}
	if (!GetIEditorImpl()->GetDocument()->IsDocumentReady())
	{
		if (bNoMsgBox == false)
			CQuestionDialog::SWarning(QObject::tr(""), QObject::tr("Please wait until previous operation will be finished."));
		return false;
	}

	SGameExporterSettings settings;
	settings.eExportEndian = eLittleEndian;

	return DoExportToGame(eExp_AI_All, settings, !bNoMsgBox);
}

bool CDrxEditApp::DoExportToGame(u32 flags, const SGameExporterSettings& exportSettings, bool showMsgBox /*= false*/)
{
	// Temporarily disable auto backup.
	CScopedVariableSetter<bool> autoBackupEnabledChange(gEditorFilePreferences.autoSaveEnabled, false);

	CGameExporter gameExporter(exportSettings);
	if (gameExporter.Export(flags, "."))
	{
		if (showMsgBox)
		{
			CLogFile::WriteLine("$3Export to the game was successfully done.");
		}
		return true;
	}
	return false;
}

void CDrxEditApp::OnExportToEngine()
{
	UserExportToGame(false);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditSelectNone()
{
	CUndo undo("Unselect All");
	////////////////////////////////////////////////////////////////////////
	// Remove the selection from all map objects
	////////////////////////////////////////////////////////////////////////
	GetIEditorImpl()->ClearSelection();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditInvertselection()
{
	GetIEditorImpl()->GetObjectManager()->InvertSelection();
}

void CDrxEditApp::OnEditDuplicate()
{
	if (GetIEditorImpl()->GetObjectManager()->GetSelection()->IsEmpty())
	{
		GetIEditorImpl()->GetNotificationCenter()->ShowInfo(QObject::tr("Command failed"), QObject::tr("Cannot duplicate, there are no selected objects"));
		return;
	}

	CEditTool* tool = GetIEditorImpl()->GetEditTool();
	if (tool && tool->IsKindOf(RUNTIME_CLASS(CObjectCloneTool)))
	{
		((CObjectCloneTool*)tool)->Accept();
	}

	GetIEditorImpl()->SetEditTool(new CObjectCloneTool);
	GetIEditorImpl()->SetModifiedFlag();
}

void CDrxEditApp::OnScriptCompileScript()
{
	////////////////////////////////////////////////////////////////////////
	// Use the Lua compiler to compile a script
	////////////////////////////////////////////////////////////////////////

	std::vector<string> files;
	if (CFileUtil::SelectMultipleFiles(EFILE_TYPE_ANY, files, "Lua Files (*.lua)|*.lua||", "Scripts"))
	{
		//////////////////////////////////////////////////////////////////////////
		// Lock resources.
		// Speed ups loading a lot.
		ISystem* pSystem = GetIEditorImpl()->GetSystem();
		pSystem->GetI3DEngine()->LockCGFResources();
		//////////////////////////////////////////////////////////////////////////
		for (i32 i = 0; i < files.size(); i++)
		{
			if (!CFileUtil::CompileLuaFile(files[i]))
				return;

			// No errors
			// Reload this lua file.
			GetIEditorImpl()->GetSystem()->GetIScriptSystem()->ReloadScript(files[i], false);
		}
		//////////////////////////////////////////////////////////////////////////
		// Unlock resources.
		// Some unneeded resources that were locked before may get released here.
		pSystem->GetI3DEngine()->UnlockCGFResources();
		//////////////////////////////////////////////////////////////////////////
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnScriptEditScript()
{
	// Let the user choose a LUA script file to edit
	string file;
	if (CFileUtil::SelectSingleFile(EFILE_TYPE_ANY, file, "Lua scripts (*.lua)|*.lua"))
	{
		CFileUtil::EditTextFile(file);
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditmodeMove()
{
	// TODO: Add your command handler code here
	GetIEditorImpl()->SetEditMode(eEditModeMove);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditmodeRotate()
{
	// TODO: Add your command handler code here
	GetIEditorImpl()->SetEditMode(eEditModeRotate);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditmodeScale()
{
	// TODO: Add your command handler code here
	GetIEditorImpl()->SetEditMode(eEditModeScale);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditToolLink()
{
	// TODO: Add your command handler code here
	if (GetIEditorImpl()->GetEditTool() && GetIEditorImpl()->GetEditTool()->IsKindOf(RUNTIME_CLASS(CLinkTool)))
		GetIEditorImpl()->SetEditTool(0);
	else
		GetIEditorImpl()->SetEditTool(new CLinkTool());
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditToolUnlink()
{
	CUndo undo("Unlink Object(s)");
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetObjectManager()->GetSelection();
	for (i32 i = 0; i < pSelection->GetCount(); i++)
	{
		CBaseObject* pBaseObj = pSelection->GetObject(i);
		pBaseObj->UnLink();
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditmodeSelect()
{
	// TODO: Add your command handler code here
	GetIEditorImpl()->SetEditMode(eEditModeSelect);
}

void CDrxEditApp::OnViewSwitchToGame()
{
	if (IsInPreviewMode()) 
		return;

	if (!GetIEditor()->GetViewManager()->GetViewport(ET_ViewportCamera))
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Cannot switch to game without opened viewport.");
		return;
	}

	// TODO: Add your command handler code here
	bool inGame = !GetIEditorImpl()->IsInGameMode();
	GetIEditorImpl()->SetInGameMode(inGame);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnExportSelectedObjects()
{
	CExportManager* pExportManager = static_cast<CExportManager*>(GetIEditorImpl()->GetExportManager());
	string filename = "untitled";
	CBaseObject* pObj = GetIEditorImpl()->GetSelectedObject();
	if (pObj)
	{
		filename = pObj->GetName();
	}
	else
	{
		string levelName = GetIEditorImpl()->GetGameEngine()->GetLevelName();
		if (!levelName.IsEmpty())
			filename = levelName;
	}
	string levelPath = GetIEditorImpl()->GetGameEngine()->GetLevelPath();
	pExportManager->Export(filename, "obj", levelPath);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnFileExportOcclusionMesh()
{
	CExportManager* pExportManager = static_cast<CExportManager*>(GetIEditorImpl()->GetExportManager());
	string levelName = GetIEditorImpl()->GetGameEngine()->GetLevelName();
	string levelPath = GetIEditorImpl()->GetGameEngine()->GetLevelPath();
	pExportManager->Export(levelName, "ocm", levelPath, false, false, false, true);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSelectionSave()
{
	CSystemFileDialog::RunParams runParams;
	runParams.extensionFilters << CExtensionFilter("Object Group Files (*.grp)", "grp");

	QString filePath = CSystemFileDialog::RunExportFile(runParams, nullptr);
	if (!filePath.isEmpty())
	{
		CWaitCursor wait;
		const CSelectionGroup* sel = GetIEditorImpl()->GetSelection();

		XmlNodeRef root = XmlHelpers::CreateXmlNode("Objects");
		CObjectArchive ar(GetIEditorImpl()->GetObjectManager(), root, false);
		// Save all objects to XML.
		for (i32 i = 0; i < sel->GetCount(); i++)
		{
			ar.SaveObject(sel->GetObject(i));
		}
		XmlHelpers::SaveXmlNode(root, filePath.toLocal8Bit().data());
	}
}

//////////////////////////////////////////////////////////////////////////
struct SDuplicatedObject
{
	SDuplicatedObject(const string& name, const DrxGUID& id)
	{
		m_name = name;
		m_id = id;
	}
	string  m_name;
	DrxGUID m_id;
};

void GatherAllObjects(XmlNodeRef node, std::vector<SDuplicatedObject>& outDuplicatedObjects)
{
	if (!stricmp(node->getTag(), "Object"))
	{
		DrxGUID guid;
		if (node->getAttr("Id", guid))
		{
			if (GetIEditorImpl()->GetObjectManager()->FindObject(guid))
			{
				string name;
				node->getAttr("Name", name);
				outDuplicatedObjects.push_back(SDuplicatedObject(name, guid));
			}
		}
	}

	for (i32 i = 0, nChildCount(node->getChildCount()); i < nChildCount; ++i)
	{
		XmlNodeRef childNode = node->getChild(i);
		if (childNode == NULL)
			continue;
		GatherAllObjects(childNode, outDuplicatedObjects);
	}
}

void CDrxEditApp::OnSelectionLoad()
{
	// Load objects from .grp file.
	CSystemFileDialog::RunParams runParams;
	runParams.extensionFilters << CExtensionFilter("Object Group Files (*.grp)", "grp");

	QString filePath = CSystemFileDialog::RunImportFile(runParams, nullptr);
	if (filePath.isEmpty())
	{
		return;
	}

	CWaitCursor wait;

	XmlNodeRef root = XmlHelpers::LoadXmlFromFile(filePath.toLocal8Bit().data());
	if (!root)
	{
		CQuestionDialog::SCritical(QObject::tr(""), QObject::tr("Error at loading group file."));
		return;
	}

	std::vector<SDuplicatedObject> duplicatedObjects;
	GatherAllObjects(root, duplicatedObjects);

	CDuplicatedObjectsHandlerDlg::EResult result(CDuplicatedObjectsHandlerDlg::eResult_None);
	i32 nDuplicatedObjectSize(duplicatedObjects.size());

	if (!duplicatedObjects.empty())
	{
		string msg;
		msg.Format("The following object(s) already exist(s) in the level.\r\n\r\n");

		for (i32 i = 0; i < nDuplicatedObjectSize; ++i)
		{
			msg += "\t";
			msg += duplicatedObjects[i].m_name;
			if (i < nDuplicatedObjectSize - 1)
				msg += "\r\n";
		}

		CDuplicatedObjectsHandlerDlg dlg(msg);
		if (dlg.DoModal() == IDCANCEL)
			return;
		result = dlg.GetResult();
	}

	CUndo undo("Load Objects");
	GetIEditorImpl()->ClearSelection();

	CObjectArchive ar(GetIEditorImpl()->GetObjectManager(), root, true);

	CRandomUniqueGuidProvider guidProvider;
	if (result == CDuplicatedObjectsHandlerDlg::eResult_Override)
	{
		for (i32 i = 0; i < nDuplicatedObjectSize; ++i)
		{
			CBaseObject* pObj = GetIEditorImpl()->GetObjectManager()->FindObject(duplicatedObjects[i].m_id);
			if (pObj)
				GetIEditorImpl()->GetObjectManager()->DeleteObject(pObj);
		}
	}
	else if (result == CDuplicatedObjectsHandlerDlg::eResult_CreateCopies)
	{
		ar.SetGuidProvider(&guidProvider);
	}

	ar.LoadInCurrentLayer(true);
	GetIEditorImpl()->GetObjectManager()->LoadObjects(ar, true);
	GetIEditorImpl()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnGotoSelected()
{
	CViewport* vp = GetIEditorImpl()->GetActiveView();
	if (vp)
		vp->CenterOnSelection();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnAlignObject()
{
	// Align pick callback will release itself.
	CAlignPickCallback* alignCallback = new CAlignPickCallback;
	GetIEditorImpl()->PickObject(alignCallback);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnAlignToGrid()
{
	const CSelectionGroup* sel = GetIEditorImpl()->GetSelection();
	if (!sel->IsEmpty())
	{
		CUndo undo("Align To Grid");
		Matrix34 tm;
		for (i32 i = 0; i < sel->GetCount(); i++)
		{
			CBaseObject* obj = sel->GetObject(i);
			tm = obj->GetWorldTM();
			// Force snapping on
			Vec3 snappedPos = gSnappingPreferences.Snap(tm.GetTranslation(), true);
			tm.SetTranslation(snappedPos);
			obj->SetWorldTM(tm);
			obj->OnEvent(EVENT_ALIGN_TOGRID);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Groups.
//////////////////////////////////////////////////////////////////////////

void CDrxEditApp::OnGroupAttach()
{
	GetIEditorImpl()->GetSelection()->AttachToGroup();
}

void CDrxEditApp::OnGroupClose()
{
	// Close _all_ selected open groups.
	const CSelectionGroup* selected = GetIEditorImpl()->GetSelection();
	for (u32 i = 0; i < selected->GetCount(); i++)
	{
		CBaseObject* obj = selected->GetObject(i);
		if (obj && obj->GetRuntimeClass() == RUNTIME_CLASS(CGroup))
		{
			if (((CGroup*)obj)->IsOpen())
			{
				GetIEditorImpl()->GetIUndoManager()->Begin();
				((CGroup*)obj)->Close();
				GetIEditorImpl()->GetIUndoManager()->Accept("Group Close");
				GetIEditorImpl()->SetModifiedFlag();
			}
		}
	}
}

void CDrxEditApp::OnGroupDetach()
{
	CUndo undo("Remove Selection from Group");
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	for (i32 i = 0, cnt = pSelection->GetCount(); i < cnt; ++i)
	{
		if (pSelection->GetObject(i)->GetGroup())
		{
			CGroup* pGroup = static_cast<CGroup*>(pSelection->GetObject(i)->GetGroup());
			pGroup->RemoveMember(pSelection->GetObject(i));
		}
	}
}

void CDrxEditApp::OnGroupDetachToRoot()
{
	CUndo undo("Remove Selection from Hierarchy");
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	for (i32 i = 0, cnt = pSelection->GetCount(); i < cnt; ++i)
	{
		if (pSelection->GetObject(i)->GetGroup())
		{
			CGroup* pGroup = static_cast<CGroup*>(pSelection->GetObject(i)->GetGroup());
			pGroup->RemoveMember(pSelection->GetObject(i), true, true);
		}
	}
}

void CDrxEditApp::ToggleHelpersDisplay()
{
	GetIEditor()->EnableHelpersDisplay(!(GetIEditor()->IsHelpersDisplayed()));
	GetIEditorImpl()->GetObjectManager()->SendEvent(GetIEditor()->IsHelpersDisplayed() ? EVENT_SHOW_HELPER : EVENT_HIDE_HELPER);
	GetIEditorImpl()->Notify(eNotify_OnDisplayRenderUpdate);
}

void CDrxEditApp::OnCycleDisplayInfo()
{
	i32 currentDisplayInfo = gEnv->pConsole->GetCVar("r_displayInfo")->GetIVal();
	switch (currentDisplayInfo)
	{
	case 0:
		gEnv->pConsole->GetCVar("r_displayInfo")->Set(3);
		break;
	case 1:
		gEnv->pConsole->GetCVar("r_displayInfo")->Set(2);
		break;
	case 2:
		gEnv->pConsole->GetCVar("r_displayInfo")->Set(0);
		break;
	case 3:
		gEnv->pConsole->GetCVar("r_displayInfo")->Set(1);
		break;
	default:
		break;
	}
}

void CDrxEditApp::OnGroupMake()
{
	const CSelectionGroup* selection = GetIEditorImpl()->GetSelection();
	selection->FilterParents();

	i32 i;
	std::vector<CBaseObject*> objects;
	for (i = 0; i < selection->GetFilteredCount(); i++)
	{
		objects.push_back(selection->GetFilteredObject(i));
	}

	if (objects.size())
	{
		const auto lastIdx = objects.size() - 1;
		IObjectLayer* pDestLayer = objects[lastIdx]->GetLayer();

		for (i = 0; i < objects.size(); ++i)
		{
			if (!objects[i]->AreLinkedDescendantsInLayer(pDestLayer))
			{
				string message;
				message.Format("The objects you are grouping are on different layers. All objects will be moved to %s layer\n\n"
				               "Do you want to continue?", pDestLayer->GetName());

				if (QDialogButtonBox::StandardButton::Yes != CQuestionDialog::SQuestion(QObject::tr(""), QObject::tr(message)))
				{
					return;
				}

				break;
			}
		}
	}

	GetIEditorImpl()->GetIUndoManager()->Begin();
	CGroup* group = (CGroup*)GetIEditorImpl()->NewObject("Group");
	if (!group)
	{
		GetIEditorImpl()->GetIUndoManager()->Cancel();
		return;
	}

	// Snap center to grid.
	Vec3 center = gSnappingPreferences.Snap(selection->GetCenter());
	group->SetPos(center);

	// Clear selection
	GetIEditorImpl()->GetObjectManager()->ClearSelection();

	// Put the newly created group on the last selected object's layer
	if (objects.size())
	{
		CBaseObject* pLastSelectedObject = objects[objects.size() - 1];
		GetIEditorImpl()->GetIUndoManager()->Suspend();
		group->SetLayer(pLastSelectedObject->GetLayer());
		GetIEditorImpl()->GetIUndoManager()->Resume();

		if (CBaseObject* pLastParent = pLastSelectedObject->GetGroup())
			pLastParent->AddMember(group);
	}

	// Prefab support
	CPrefabObject* pPrefabToCompareAgainst = nullptr;
	CPrefabObject* pObjectPrefab = nullptr;

	for (auto pObject : objects)
	{
		// Prefab handling
		pObjectPrefab = (CPrefabObject*)pObject->GetPrefab();

		// Sanity check if user is trying to group objects from different prefabs
		if (pPrefabToCompareAgainst && pObjectPrefab)
		{
			GetIEditorImpl()->GetIUndoManager()->Cancel();
			GetIEditorImpl()->DeleteObject(group);
			return;
		}

		if (!pPrefabToCompareAgainst)
			pPrefabToCompareAgainst = pObjectPrefab;
	}

	group->AddMembers(objects);

	// Signal that we added a group to the prefab
	if (pPrefabToCompareAgainst)
		pPrefabToCompareAgainst->AddMember(group);

	GetIEditorImpl()->GetObjectManager()->SelectObject(group);
	GetIEditorImpl()->GetIUndoManager()->Accept("Group Make");
	GetIEditorImpl()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnGroupOpen()
{
	// Ungroup all groups in selection.
	const CSelectionGroup* sel = GetIEditorImpl()->GetSelection();
	if (!sel->IsEmpty())
	{
		CUndo undo("Group Open");
		for (i32 i = 0; i < sel->GetCount(); i++)
		{
			CBaseObject* obj = sel->GetObject(i);
			if (obj && obj->GetRuntimeClass() == RUNTIME_CLASS(CGroup))
			{
				((CGroup*)obj)->Open();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnGroupUngroup()
{
	// Ungroup all groups in selection.
	std::vector<CBaseObjectPtr> objects;

	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	for (i32 i = 0; i < pSelection->GetCount(); i++)
	{
		objects.push_back(pSelection->GetObject(i));
	}

	if (objects.size())
	{
		CUndo undo("Ungroup");

		for (i32 i = 0; i < objects.size(); i++)
		{
			CBaseObject* obj = objects[i];
			if (obj && obj->GetRuntimeClass() == RUNTIME_CLASS(CGroup))
			{
				static_cast<CGroup*>(obj)->Ungroup();
				// Signal prefab if part of any
				if (CPrefabObject* pPrefab = (CPrefabObject*)obj->GetPrefab())
					pPrefab->RemoveMember(obj);
				GetIEditorImpl()->DeleteObject(obj);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnLockSelection()
{
	// Invert selection lock.
	GetIEditorImpl()->LockSelection(!GetIEditorImpl()->IsSelectionLocked());
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnFileEditLogFile()
{
	CFileUtil::EditTextFile(CLogFile::GetLogFileName(), 0, CFileUtil::FILE_TYPE_SCRIPT, false);
}

void CDrxEditApp::OnReloadTextures()
{
	CWaitCursor wait;
	CLogFile::WriteLine("Reloading Static objects textures and shaders.");
	GetIEditorImpl()->GetObjectManager()->SendEvent(EVENT_RELOAD_TEXTURES);
	GetIEditorImpl()->GetRenderer()->EF_ReloadTextures();
}

void CDrxEditApp::OnReloadArchetypes()
{
	if (gEnv->pEntitySystem)
	{
		gEnv->pEntitySystem->RefreshEntityArchetypesInRegistry();
	}
}

void CDrxEditApp::OnReloadAllScripts()
{
	OnReloadEntityScripts();
	OnReloadItemScripts();
	OnReloadAIScripts();
	OnReloadUIScripts();
}

void CDrxEditApp::OnReloadEntityScripts()
{
	LOADING_TIME_PROFILE_AUTO_SESSION("reload_entity_scripts");

	CEntityScriptRegistry::Instance()->Reload();

	SEntityEvent event;
	event.event = ENTITY_EVENT_RELOAD_SCRIPT;
	gEnv->pEntitySystem->SendEventToAll(event);

	GetIEditorImpl()->GetObjectManager()->SendEvent(EVENT_RELOAD_ENTITY);

	// an entity node can be changed either by editing a script or in schematyc.
	// Any graph can use an entity node, so reload everything.
	//FIXME: Schematyc is reloading all entities on editor idle, which is slow and potentially crashes here. Uncomment when that behaviour is reviewed
	//GetIEditorImpl()->GetFlowGraphManager()->ReloadGraphs();
}

void CDrxEditApp::OnReloadItemScripts()
{
	gEnv->pConsole->ExecuteString("i_reload");
}

void CDrxEditApp::OnReloadAIScripts()
{
	GetIEditorImpl()->GetAI()->ReloadScripts();

	// grab the entity IDs of all AI objects
	std::vector<EntityId> entityIDsUsedByAIObjects;
	IAIObjectIter* pIter = GetIEditorImpl()->GetAI()->GetAISystem()->GetAIObjectManager()->GetFirstAIObject(OBJFILTER_TYPE, 0);
	for (IAIObject* pAI = pIter->GetObject(); pAI != NULL; pIter->Next(), pAI = pIter->GetObject())
	{
		entityIDsUsedByAIObjects.push_back(pAI->GetEntityID());
	}
	pIter->Release();

	// find the AI objects among all editor objects
	CBaseObjectsArray allEditorObjects;
	GetIEditorImpl()->GetObjectManager()->GetObjects(allEditorObjects);
	for (CBaseObjectsArray::const_iterator it = allEditorObjects.begin(); it != allEditorObjects.end(); ++it)
	{
		CBaseObject* obj = *it;

		if (obj->IsKindOf(RUNTIME_CLASS(CEntityObject)))
		{
			CEntityObject* entity = static_cast<CEntityObject*>(obj);
			const EntityId entityId = entity->GetEntityId();

			// is this entity using AI? -> reload the entity and its script
			if (std::find(entityIDsUsedByAIObjects.begin(), entityIDsUsedByAIObjects.end(), entityId) != entityIDsUsedByAIObjects.end())
			{
				if (CEntityScript* script = entity->GetScript())
				{
					script->Reload();
				}
				entity->Reload(true);

				if (IEntity* pEnt = gEnv->pEntitySystem->GetEntity(entityId))
				{
					SEntityEvent event;
					event.event = ENTITY_EVENT_RELOAD_SCRIPT;
					pEnt->SendEvent(event);
				}
			}
		}
	}
}

void CDrxEditApp::OnReloadUIScripts()
{
	GetIEditorImpl()->GetUIManager()->ReloadScripts();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnReloadGeometry()
{
	CWaitProgress wait("Reloading static geometry");

	CVegetationMap* pVegetationMap = GetIEditorImpl()->GetVegetationMap();
	if (pVegetationMap)
		pVegetationMap->ReloadGeometry();

	CLogFile::WriteLine("Reloading Static objects geometries.");
	CEdMesh::ReloadAllGeometries();

	// Reload CHRs
	GetIEditorImpl()->GetSystem()->GetIAnimationSystem()->ReloadAllModels();

	//GetIEditorImpl()->Get3DEngine()->UnlockCGFResources();

	if (gEnv->pGameFramework->GetIItemSystem() != NULL)
	{
		gEnv->pGameFramework->GetIItemSystem()->ClearGeometryCache();
	}
	//GetIEditorImpl()->Get3DEngine()->LockCGFResources();
	// Force entity system to collect garbage.
	GetIEditorImpl()->GetSystem()->GetIEntitySystem()->Update();
	GetIEditorImpl()->GetObjectManager()->SendEvent(EVENT_RELOAD_GEOM);

	// Rephysicalize viewport meshes
	for (i32 i = 0; i < GetIEditorImpl()->GetViewManager()->GetViewCount(); ++i)
	{
		CViewport* vp = GetIEditorImpl()->GetViewManager()->GetView(i);
		if (vp->GetType() == ET_ViewportModel)
		{
			((CModelViewport*)vp)->RePhysicalize();
		}
	}

	OnReloadEntityScripts();
	IRenderNode** plist = new IRenderNode*[max(gEnv->p3DEngine->GetObjectsByType(eERType_Vegetation, 0), gEnv->p3DEngine->GetObjectsByType(eERType_Brush, 0))];
	for (i32 i = 0; i < 2; i++)
		for (i32 j = gEnv->p3DEngine->GetObjectsByType(i ? eERType_Brush : eERType_Vegetation, plist) - 1; j >= 0; j--)
			plist[j]->Physicalize(true);
	delete[] plist;
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnUndo()
{
	//GetIEditorImpl()->GetObjectManager()->UndoLastOp();
	GetIEditorImpl()->GetIUndoManager()->Undo();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnRedo()
{
	GetIEditorImpl()->GetIUndoManager()->Redo();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSwitchPhysics()
{
	CWaitCursor wait;
	bool bSimulationEnabled = !GetIEditorImpl()->GetGameEngine()->GetSimulationMode();

	QAction* pAction = GetIEditorImpl()->GetICommandManager()->GetAction("ui_action.actionEnable_Physics_AI");
	if (bSimulationEnabled)
		pAction->setIcon(DrxIcon("icons:common/general_physics_stop.ico"));
	else
		pAction->setIcon(DrxIcon("icons:common/general_physics_play.ico"));

	GetIEditorImpl()->GetGameEngine()->SetSimulationMode(bSimulationEnabled);
	GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_EDITOR_SIMULATION_MODE_CHANGED, bSimulationEnabled, 0);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSyncPlayer()
{
	GetIEditorImpl()->GetGameEngine()->SyncPlayerPosition(!GetIEditorImpl()->GetGameEngine()->IsSyncPlayerPosition());
}

//////////////////////////////////////////////////////////////////////////
CDrxEditApp::ECreateLevelResult CDrxEditApp::CreateLevel(const string& levelName, i32 resolution, float unitSize, bool bUseTerrain)
{
	IDrxPak* const pDrxPak = GetIEditorImpl()->GetSystem()->GetIPak();

	LOADING_TIME_PROFILE_AUTO_SESSION("sanbox_create_new_level");
	const string currentLevel = GetIEditorImpl()->GetLevelFolder();
	if (!currentLevel.IsEmpty())
	{
		pDrxPak->ClosePacks(PathUtil::Make(currentLevel, "*.*"));
	}

	const string levelFileName = PathUtil::GetFile(PathUtil::RemoveSlash(levelName));
	const string levelPath = string().Format("%s/%s", PathUtil::GetGameFolder().c_str(), levelName.c_str());
	const string filename = PathUtil::Make(levelPath, levelFileName, CLevelType::GetFileExtensionStatic());

	if (pDrxPak->IsFolder(levelPath.c_str()))
	{
		return ECLR_ALREADY_EXISTS;
	}

	CLogFile::WriteLine("Creating level directory");
	if (!pDrxPak->MakeDir(levelPath.c_str()))
	{
		return ECLR_DIR_CREATION_FAILED;
	}

	// First cleanup previous document
	GetIEditorImpl()->CloseDocument();
	// Create empty document
	GetIEditorImpl()->SetDocument(new CDrxEditDoc);

	if (CViewport* pViewPort = GetIEditorImpl()->GetActiveView())
	{
		pViewPort->Update();
	}

	GetIEditorImpl()->GetGameEngine()->SetLevelPath(levelPath);
	gEnv->pGameFramework->SetEditorLevel(levelName, levelPath);
	GetIEditorImpl()->GetDocument()->InitEmptyLevel(resolution, unitSize, bUseTerrain);
	CProgressNotification notification("Creating Level", QtUtil::ToQString(levelName));

	if (bUseTerrain)
	{
		GetIEditorImpl()->GetTerrainManager()->SetTerrainSize(resolution, unitSize);
	}

	// Save the document to this folder
	GetIEditorImpl()->GetDocument()->SetPathName(filename.GetBuffer());

	if (GetIEditorImpl()->GetDocument()->Save())
	{
		// Create sample Objectives.xml in level data.
		CreateSampleMissionObjectives();

		CGameExporter gameExporter;
		gameExporter.Export();

		GetIEditorImpl()->GetGameEngine()->LoadLevel(levelPath, GetIEditorImpl()->GetGameEngine()->GetMissionName(), true, true);

		if (auto* pEditorGame = GetIEditorImpl()->GetGameEngine()->GetIEditorGame())
		{
			pEditorGame->OnAfterLevelLoad(GetIEditorImpl()->GetGameEngine()->GetLevelName(), GetIEditorImpl()->GetGameEngine()->GetLevelPath());
		}

		GetIEditorImpl()->GetHeightmap()->InitTerrain();

		//GetIEditorImpl()->GetGameEngine()->LoadAINavigationData();
		if (!bUseTerrain)
		{
			XmlNodeRef m_node = GetIEditorImpl()->GetDocument()->GetEnvironmentTemplate();
			if (m_node)
			{
				XmlNodeRef envState = m_node->findChild("EnvState");
				if (envState)
				{
					XmlNodeRef showTerrainSurface = envState->findChild("ShowTerrainSurface");
					showTerrainSurface->setAttr("value", "false");
					GetIEditorImpl()->GetGameEngine()->ReloadEnvironment();
				}
			}
		}
		else
		{
			// we need to reload environment after terrain creation in order for water to be initialized
			GetIEditorImpl()->GetGameEngine()->ReloadEnvironment();
		}
	}

	GetIEditorImpl()->GetDocument()->SetDocumentReady(true);

	return ECLR_OK;
}

//////////////////////////////////////////////////////////////////////////
CDrxEditDoc* CDrxEditApp::LoadLevel(tukk lpszFileName)//this path is a pak file path but make this more robust
{
	CDrxEditDoc* doc = 0;
	if (GetIEditorImpl()->GetLevelIndependentFileMan()->PromptChangedFiles() && GetIEditorImpl()->GetDocument()->CanClose())
	{
		CProgressNotification notification("Loading Level", QString(lpszFileName));
		// disable commands while loading. Since we process events this can trigger actions
		// that tweak incomplete object state.
		GetIEditorImpl()->EnableAcceleratos(false);

		string newPath = PathUtil::AbsolutePathToDrxPakPath(lpszFileName);
		PathUtil::ToUnixPath(newPath.GetBuffer());

		doc = GetIEditorImpl()->GetDocument();
		if (doc)
		{
			delete doc;
		}
		doc = new CDrxEditDoc;
		doc->OnOpenDocument(newPath);
		LoadTagLocations();

		// Reenable accelerators here
		GetIEditorImpl()->EnableAcceleratos(true);

		doc->SetLastLoadedLevelName(newPath);
	}

	return doc;
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnResourcesReduceworkingset()
{
	SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnToolsUpdateProcVegetation()
{
	GetIEditorImpl()->GetTerrainManager()->ReloadSurfaceTypes();

	CVegetationMap* pVegetationMap = GetIEditorImpl()->GetVegetationMap();
	if (pVegetationMap)
		pVegetationMap->SetEngineObjectsParams();

	GetIEditorImpl()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditHide()
{
	// Hide selection.
	const CSelectionGroup* sel = GetIEditorImpl()->GetSelection();
	if (!sel->IsEmpty())
	{
		CUndo undo("Hide");
		for (i32 i = 0; i < sel->GetCount(); i++)
		{
			// Duplicated object names can exist in the case of prefab objects so passing a name as a script parameter and processing it couldn't be exact.
			sel->GetObject(i)->SetHidden(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditUnhideall()
{
	// Unhide all.
	CUndo undo("Unhide All");
	GetIEditorImpl()->GetObjectManager()->UnhideAll();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditFreeze()
{
	// Freeze selection.
	const CSelectionGroup* sel = GetIEditorImpl()->GetSelection();
	std::vector<CBaseObject*> objects;
	if (!sel->IsEmpty())
	{
		// save selection in temporary list because unfreezing will remove objects from selection
		for (i32 i = 0; i < sel->GetCount(); i++)
		{
			objects.push_back(sel->GetObject(i));
		}

		CUndo undo("Freeze");
		for (CBaseObject* pObj : objects)
		{
			pObj->SetFrozen(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnEditUnfreezeall()
{
	// Unfreeze all.
	CUndo undo("Unfreeze All");
	GetIEditorImpl()->GetObjectManager()->UnfreezeAll();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnWireframe()
{
	i32 nWireframe(R_SOLID_MODE);
	ICVar* r_wireframe(gEnv->pConsole->GetCVar("r_wireframe"));

	if (r_wireframe)
	{
		nWireframe = r_wireframe->GetIVal();
	}

	if (nWireframe != R_WIREFRAME_MODE)
	{
		nWireframe = R_WIREFRAME_MODE;
	}
	else
	{
		nWireframe = R_SOLID_MODE;
	}

	if (r_wireframe)
	{
		r_wireframe->Set(nWireframe);
	}
}

void CDrxEditApp::OnPointmode()
{
	i32 nWireframe(R_SOLID_MODE);
	ICVar* r_wireframe(gEnv->pConsole->GetCVar("r_wireframe"));

	if (r_wireframe)
	{
		nWireframe = r_wireframe->GetIVal();
	}

	if (nWireframe != R_POINT_MODE)
	{
		nWireframe = R_POINT_MODE;
	}
	else
	{
		nWireframe = R_SOLID_MODE;
	}

	if (r_wireframe)
	{
		r_wireframe->Set(nWireframe);
	}
}

//////////////////////////////////////////////////////////////////////////

void CDrxEditApp::CreateSampleMissionObjectives()
{
	CTempFileHelper helper(GetIEditorImpl()->GetLevelDataFolder() + "Objectives.xml");

	XmlNodeRef root = XmlHelpers::CreateXmlNode("Root");

	XmlNodeRef levelNode = XmlHelpers::CreateXmlNode("Level");
	levelNode->setAttr("Name", GetIEditorImpl()->GetGameEngine()->GetLevelName());
	root->addChild(levelNode);

	XmlNodeRef missionNode = XmlHelpers::CreateXmlNode("Mission_01");
	missionNode->setAttr("Name", "Example Primary Mission");
	missionNode->setAttr("Description", "Description of Example Primary Mission.");
	levelNode->addChild(missionNode);

	missionNode = XmlHelpers::CreateXmlNode("Mission_01b");
	missionNode->setAttr("Name", "Example Secondary Mission");
	missionNode->setAttr("Description", "Description of Example Secondary Mission.");
	missionNode->setAttr("Secondary", "true");
	levelNode->addChild(missionNode);

	root->saveToFile(helper.GetTempFilePath());
	helper.UpdateFile(false);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::TagLocation(i32 index)
{
	CViewport* pRenderViewport = GetIEditorImpl()->GetViewManager()->GetGameViewport();
	if (!pRenderViewport)
		return;

	Vec3 vPosVec = pRenderViewport->GetViewTM().GetTranslation();

	m_tagLocations[index - 1] = vPosVec;
	m_tagAngles[index - 1] = Ang3::GetAnglesXYZ(Matrix33(pRenderViewport->GetViewTM()));

	string sTagConsoleText("");
	sTagConsoleText.Format("Camera Tag Point %d set to the position: x=%.2f, y=%.2f, z=%.2f ", index, vPosVec.x, vPosVec.y, vPosVec.z);
	GetIEditorImpl()->WriteToConsole(sTagConsoleText);

	SaveTagLocations();
}

void CDrxEditApp::SaveTagLocations()
{
	// Save to file.
	char filename[_MAX_PATH];
	drx_strcpy(filename, GetIEditorImpl()->GetDocument()->GetPathName());
	QFileInfo fileInfo(filename);
	QString fullPath(fileInfo.absolutePath() + "/tags.txt");
	SetFileAttributes(fullPath.toStdString().c_str(), FILE_ATTRIBUTE_NORMAL);
	FILE* f = fopen(fullPath.toStdString().c_str(), "wt");
	if (f)
	{
		for (i32 i = 0; i < 12; i++)
		{
			fprintf(f, "%f,%f,%f,%f,%f,%f\n",
			        m_tagLocations[i].x, m_tagLocations[i].y, m_tagLocations[i].z,
			        m_tagAngles[i].x, m_tagAngles[i].y, m_tagAngles[i].z);
		}
		fclose(f);
	}

	// Save to .json with IArchive as well
	fullPath = fileInfo.absolutePath() + "/tags.json";
	SetFileAttributes(fullPath.toStdString().c_str(), FILE_ATTRIBUTE_NORMAL);

	CTagLocationsSerializer tagsSerializer;
	for (i32 i = 0; i < 12; ++i)
	{
		tagsSerializer.AddTag(m_tagLocations[i], m_tagAngles[i], m_tagSegmentsXY[i]);
	}

	Serialization::SaveJsonFile(fullPath.toStdString().c_str(), tagsSerializer);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::GotoTagLocation(i32 index)
{
	string sTagConsoleText("");
	Vec3 pos = m_tagLocations[index - 1];

	if (!IsVectorsEqual(m_tagLocations[index - 1], Vec3(0, 0, 0)))
	{
		// Change render viewport view TM to the stored one.
		CViewport* pRenderViewport = GetIEditorImpl()->GetViewManager()->GetGameViewport();
		if (pRenderViewport)
		{

			if (gEnv->pStatoscope && gEnv->IsEditorGameMode())
			{
				Vec3 oldPos = pRenderViewport->GetViewTM().GetTranslation();
				char buffer[100];
				drx_sprintf(buffer, "Teleported from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)", oldPos.x, oldPos.y, oldPos.z, pos.x, pos.y, pos.z);
				gEnv->pStatoscope->AddUserMarker("Player", buffer);
			}

			Matrix34 tm = Matrix34::CreateRotationXYZ(m_tagAngles[index - 1]);
			tm.SetTranslation(pos);
			pRenderViewport->SetViewTM(tm);

			GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_BEAM_PLAYER_TO_CAMERA_POS, (UINT_PTR)&tm, 0);

			sTagConsoleText.Format("Moved Camera To Tag Point %d (x=%.2f, y=%.2f, z=%.2f)", index, pos.x, pos.y, pos.z);
		}
	}
	else
	{
		sTagConsoleText.Format("Camera Tag Point %d not set", index);
	}

	if (!sTagConsoleText.IsEmpty())
		GetIEditorImpl()->WriteToConsole(sTagConsoleText);
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::LoadTagLocations()
{
	char filename[_MAX_PATH];
	drx_strcpy(filename, GetIEditorImpl()->GetDocument()->GetPathName());
	QFileInfo fileInfo(filename);
	QString fullPath(fileInfo.absolutePath() + "/tags.txt");
	// Load tag locations from file.

	ZeroStruct(m_tagLocations);

	FILE* f = fopen(fullPath.toStdString().c_str(), "rt");
	if (f)
	{
		for (i32 i = 0; i < 12; i++)
		{
			float x = 0, y = 0, z = 0, ax = 0, ay = 0, az = 0;
			fscanf(f, "%f,%f,%f,%f,%f,%f\n", &x, &y, &z, &ax, &ay, &az);
			m_tagLocations[i] = Vec3(x, y, z);
			m_tagAngles[i] = Ang3(ax, ay, az);
		}
		fclose(f);
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnToolsLogMemoryUsage()
{
	gEnv->pConsole->ExecuteString("SaveLevelStats");
	//GetIEditorImpl()->GetHeightmap()->LogLayerSizes();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnTagLocation1()  { TagLocation(1); }
void CDrxEditApp::OnTagLocation2()  { TagLocation(2); }
void CDrxEditApp::OnTagLocation3()  { TagLocation(3); }
void CDrxEditApp::OnTagLocation4()  { TagLocation(4); }
void CDrxEditApp::OnTagLocation5()  { TagLocation(5); }
void CDrxEditApp::OnTagLocation6()  { TagLocation(6); }
void CDrxEditApp::OnTagLocation7()  { TagLocation(7); }
void CDrxEditApp::OnTagLocation8()  { TagLocation(8); }
void CDrxEditApp::OnTagLocation9()  { TagLocation(9); }
void CDrxEditApp::OnTagLocation10() { TagLocation(10); }
void CDrxEditApp::OnTagLocation11() { TagLocation(11); }
void CDrxEditApp::OnTagLocation12() { TagLocation(12); }

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnGotoLocation1()  { GotoTagLocation(1); }
void CDrxEditApp::OnGotoLocation2()  { GotoTagLocation(2); }
void CDrxEditApp::OnGotoLocation3()  { GotoTagLocation(3); }
void CDrxEditApp::OnGotoLocation4()  { GotoTagLocation(4); }
void CDrxEditApp::OnGotoLocation5()  { GotoTagLocation(5); }
void CDrxEditApp::OnGotoLocation6()  { GotoTagLocation(6); }
void CDrxEditApp::OnGotoLocation7()  { GotoTagLocation(7); }
void CDrxEditApp::OnGotoLocation8()  { GotoTagLocation(8); }
void CDrxEditApp::OnGotoLocation9()  { GotoTagLocation(9); }
void CDrxEditApp::OnGotoLocation10() { GotoTagLocation(10); }
void CDrxEditApp::OnGotoLocation11() { GotoTagLocation(11); }
void CDrxEditApp::OnGotoLocation12() { GotoTagLocation(12); }

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnExportIndoors()
{
	//CBrushIndoor *indoor = GetIEditorImpl()->GetObjectManager()->GetCurrentIndoor();
	//CBrushExporter exp;
	//exp.Export( indoor, "C:\\MasterCD\\Objects\\Indoor.bld" );
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnViewCycle2dviewport()
{
	GetIEditorImpl()->GetViewManager()->Cycle2DViewport();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnDisplayGotoPosition()
{
	CGotoPositionDialog dlg;
	dlg.exec();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnRuler()
{
	CRuler* pRuler = GetIEditorImpl()->GetRuler();
	pRuler->SetActive(!pRuler->IsActive());
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnRotateselectionXaxis()
{
	CUndo undo("Rotate X");
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	pSelection->Rotate(Ang3(m_fastRotateAngle, 0, 0), GetIEditorImpl()->GetReferenceCoordSys());
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnRotateselectionYaxis()
{
	CUndo undo("Rotate Y");
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	pSelection->Rotate(Ang3(0, m_fastRotateAngle, 0), GetIEditorImpl()->GetReferenceCoordSys());
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnRotateselectionZaxis()
{
	CUndo undo("Rotate Z");
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	pSelection->Rotate(Ang3(0, 0, m_fastRotateAngle), GetIEditorImpl()->GetReferenceCoordSys());
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnRotateselectionRotateangle()
{
	QNumericBoxDialog dlg(QObject::tr("Angle"), m_fastRotateAngle);
	dlg.SetRange(-359.99, 359.99);
	dlg.SetStep(5);

	if (dlg.exec() == QDialog::Accepted)
	{
		m_fastRotateAngle = dlg.GetValue();
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnPhysicsGetState()
{
	GetIEditorImpl()->GetCommandManager()->Execute("physics.get_state_selection");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnPhysicsResetState()
{
	GetIEditorImpl()->GetCommandManager()->Execute("physics.reset_state_selection");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnPhysicsSimulateObjects()
{
	GetIEditorImpl()->GetCommandManager()->Execute("physics.simulate_objects");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnPhysicsGenerateJoints()
{
	GetIEditorImpl()->GetCommandManager()->Execute("physics.generate_joints");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnFileSavelevelresources()
{
	CGameResourcesExporter saver;
	saver.GatherAllLoadedResources();
	saver.ChooseDirectoryAndSave();
}

void CDrxEditApp::OnValidateLevel()
{
	//TODO : There are two different code paths to validate objects, most likely this should be moved somewhere else or unified at least.
	CWaitCursor cursor;

	// Validate all objects
	CBaseObjectsArray objects;
	GetIEditorImpl()->GetObjectManager()->GetObjects(objects);

	i32 i;

	CLogFile::WriteLine("Validating Objects...");
	for (i = 0; i < objects.size(); i++)
	{
		CBaseObject* pObject = objects[i];

		pObject->Validate();

		CUsedResources rs;
		pObject->GatherUsedResources(rs);
		rs.Validate();
	}

	CLogFile::WriteLine("Validating Duplicate Objects...");
	//////////////////////////////////////////////////////////////////////////
	// Find duplicate objects, Same objects with same transform.
	// Use simple grid to speed up the check.
	//////////////////////////////////////////////////////////////////////////
	i32 gridSize = 256;

	SSectorInfo si;
	GetIEditorImpl()->GetHeightmap()->GetSectorsInfo(si);
	float worldSize = si.numSectors * si.sectorSize;
	float fGridToWorld = worldSize / gridSize;

	// Put all objects into parition grid.
	std::vector<std::list<CBaseObject*>> grid;
	grid.resize(gridSize * gridSize);
	// Put objects to grid.
	for (i = 0; i < objects.size(); i++)
	{
		CBaseObject* pObject = objects[i];
		Vec3 pos = pObject->GetWorldPos();
		i32 px = i32(pos.x / fGridToWorld);
		i32 py = i32(pos.y / fGridToWorld);
		if (px < 0) px = 0;
		if (py < 0) py = 0;
		if (px >= gridSize) px = gridSize - 1;
		if (py >= gridSize) py = gridSize - 1;
		grid[py * gridSize + px].push_back(pObject);
	}

	std::list<CBaseObject*>::iterator it1, it2;
	// Check objects in grid.
	for (i = 0; i < gridSize * gridSize; i++)
	{
		std::list<CBaseObject*>::iterator first = grid[i].begin();
		std::list<CBaseObject*>::iterator last = grid[i].end();
		for (it1 = first; it1 != last; ++it1)
		{
			for (it2 = first; it2 != it1; ++it2)
			{
				// Check if same object.
				CBaseObject* p1 = *it1;
				CBaseObject* p2 = *it2;
				if (p1 != p2 && p1->GetClassDesc() == p2->GetClassDesc())
				{
					// Same class.
					if (p1->GetWorldPos() == p2->GetWorldPos() && p1->GetRotation() == p2->GetRotation() && p1->GetScale() == p2->GetScale())
					{
						// Same transformation
						// Check if objects are really same.
						if (p1->IsSimilarObject(p2))
						{
							const Vec3 pos = p1->GetWorldPos();
							// Report duplicate objects.
							DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Found multiple objects in the same location (class %s): %s and %s are located at (%.2f, %.2f, %.2f) %s",
							           p1->GetClassDesc()->ClassName(), (tukk)p1->GetName(), (tukk)p2->GetName(), pos.x, pos.y, pos.z,
							           DrxLinkService::CDrxLinkUriFactory::GetUriV("Editor", "general.select_and_go_to_object %s", p1->GetName()));
						}
					}
				}
			}
		}
	}

	//Validate materials
	GetIEditorImpl()->GetMaterialManager()->Validate();
}

/////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnValidateObjectPositions()
{
	IObjectManager* objMan = GetIEditorImpl()->GetObjectManager();

	if (!objMan)
		return;

	i32 objCount = objMan->GetObjectCount();
	AABB bbox1;
	AABB bbox2;
	i32 bugNo = 0;

	std::vector<CBaseObject*> objects;
	objMan->GetObjects(objects);

	std::vector<CBaseObject*> foundObjects;

	std::vector<DrxGUID> objIDs;
	bool reportVeg = false;

	for (i32 i1 = 0; i1 < objCount; ++i1)
	{
		CBaseObject* pObj1 = objects[i1];

		if (!pObj1)
			continue;

		// Ignore groups in search
		if (pObj1->GetType() == OBJTYPE_GROUP)
			continue;

		// Object must have geometry
		if (!pObj1->GetGeometry())
			continue;

		// Ignore solids
		if (pObj1->GetType() == OBJTYPE_SOLID)
			continue;

		pObj1->GetBoundBox(bbox1);

		// Check if object has vegetation inside its bbox
		CVegetationMap* pVegetationMap = GetIEditorImpl()->GetVegetationMap();
		if (pVegetationMap && !pVegetationMap->IsAreaEmpty(bbox1))
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "%s has vegetation object(s) inside %s", pObj1->GetName(),
			           DrxLinkService::CDrxLinkUriFactory::GetUriV("Editor", "general.select_and_go_to_object %s", pObj1->GetName()));
		}

		// Check if object has other objects inside its bbox
		foundObjects.clear();
		objMan->FindObjectsInAABB(bbox1, foundObjects);

		for (i32 i2 = 0; i2 < foundObjects.size(); ++i2)
		{
			CBaseObject* pObj2 = objects[i2];
			if (!pObj2)
				continue;

			if (pObj2->GetId() == pObj1->GetId())
				continue;

			if (pObj2->GetParent())
				continue;

			if (pObj2->GetType() == OBJTYPE_SOLID)
				continue;

			if (stl::find(objIDs, pObj2->GetId()))
				continue;

			if ((!pObj2->GetGeometry() || pObj2->GetType() == OBJTYPE_SOLID) && (pObj2->GetType()))
				continue;

			pObj2->GetBoundBox(bbox2);

			if (!bbox1.IsContainPoint(bbox2.max))
				continue;

			if (!bbox1.IsContainPoint(bbox2.min))
				continue;

			objIDs.push_back(pObj2->GetId());
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "%s inside %s object %s", (tukk)pObj2->GetName(), pObj1->GetName(),
			           DrxLinkService::CDrxLinkUriFactory::GetUriV("Editor", "general.select_and_go_to_object %s", pObj2->GetName()));
			++bugNo;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSwitchToDefaultCamera()
{
	CViewport* vp = GetIEditorImpl()->GetViewManager()->GetSelectedViewport();
	if (vp && vp->IsRenderViewport() && vp->GetType() == ET_ViewportCamera)
		((CLevelEditorViewport*)vp)->SetDefaultCamera();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSwitchToSequenceCamera()
{
	/*
	   CViewport *vp = GetIEditorImpl()->GetViewManager()->GetSelectedViewport();
	   if (vp && vp->IsRenderViewport())
	    ((CRenderViewport*)vp)->SetSequenceCamera();
	 */
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSwitchToSelectedcamera()
{
	CViewport* vp = GetIEditorImpl()->GetViewManager()->GetSelectedViewport();
	if (vp && vp->IsRenderViewport() && vp->GetType() == ET_ViewportCamera)
		((CLevelEditorViewport*)vp)->SetSelectedCamera();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnSwitchcameraNext()
{
	CViewport* vp = GetIEditorImpl()->GetActiveView();
	if (vp && vp->IsRenderViewport() && vp->GetType() == ET_ViewportCamera)
		((CLevelEditorViewport*)vp)->CycleCamera();
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnMaterialAssigncurrent()
{
	GetIEditorImpl()->ExecuteCommand("material.assign_current_to_selection");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnMaterialResettodefault()
{
	GetIEditorImpl()->ExecuteCommand("material.reset_selection");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnMaterialGetmaterial()
{
	GetIEditorImpl()->ExecuteCommand("material.set_current_from_object");
}

void CDrxEditApp::OnMaterialPicktool()
{
	GetIEditorImpl()->SetEditTool("Material.PickTool");
}

//////////////////////////////////////////////////////////////////////////
void CDrxEditApp::OnResolveMissingObjects()
{
	GetIEditorImpl()->GetObjectManager()->ResolveMissingObjects();
}

//////////////////////////////////////////////////////////////////////////
bool CDrxEditApp::IsInRegularEditorMode()
{
	return !IsInTestMode() && !IsInPreviewMode()
	       && !IsInExportMode() && !IsInConsoleMode() && !IsInLevelLoadTestMode();
}

namespace
{
bool g_runScriptResult = false;     // true -> success, false -> failure
}

namespace
{
void PySetResultToSuccess()
{
	g_runScriptResult = true;
}

void PySetResultToFailure()
{
	g_runScriptResult = false;
}

void PyIdleWait(double timeInSec)
{
	clock_t start = clock();
	do
	{
		MSG msg;
		while (FALSE != ::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	while ((double)(clock() - start) / CLOCKS_PER_SEC < timeInSec);
}
}

bool CDrxEditApp::Command_ExportToEngine()
{
	return CDrxEditApp::GetInstance()->UserExportToGame(true);
}

void CDrxEditApp::SubObjectModeVertex()
{
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	if (pSelection->GetCount() != 1)
		return;

	CBaseObject* pSelObject = pSelection->GetObject(0);

	if (pSelObject->GetType() == OBJTYPE_BRUSH)
	{
		GetIEditorImpl()->ExecuteCommand("edit_mode.select_vertex");
	}
	else if (pSelObject->GetType() == OBJTYPE_SOLID)
	{
		PyScript::Execute("designer.select_vertexmode()");
	}
	else
	{
		CQuestionDialog::SWarning(QObject::tr(""), QObject::tr("The current selected object(s) do(es)n't support the mesh editing"));
	}
}

void CDrxEditApp::SubObjectModeEdge()
{
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	if (pSelection->GetCount() != 1)
		return;

	CBaseObject* pSelObject = pSelection->GetObject(0);

	if (pSelObject->GetType() == OBJTYPE_BRUSH)
	{
		GetIEditorImpl()->ExecuteCommand("edit_mode.select_edge");
	}
	else if (pSelObject->GetType() == OBJTYPE_SOLID)
	{
		PyScript::Execute("designer.select_edgemode()");
	}
	else
	{
		CQuestionDialog::SWarning(QObject::tr(""), QObject::tr("The current selected object(s) do(es)n't support the mesh editing"));
	}
}

void CDrxEditApp::SubObjectModeFace()
{
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	if (pSelection->GetCount() != 1)
		return;

	CBaseObject* pSelObject = pSelection->GetObject(0);

	if (pSelObject->GetType() == OBJTYPE_BRUSH)
	{
		GetIEditorImpl()->ExecuteCommand("edit_mode.select_face");
	}
	else if (pSelObject->GetType() == OBJTYPE_SOLID)
	{
		PyScript::Execute("designer.select_facemode()");
	}
	else
	{
		CQuestionDialog::SWarning(QObject::tr(""), QObject::tr("The current selected object(s) do(es)n't support the mesh editing"));
	}
}

void CDrxEditApp::SubObjectModePivot()
{
	const CSelectionGroup* pSelection = GetIEditorImpl()->GetSelection();
	if (pSelection->GetCount() != 1)
		return;

	CBaseObject* pSelObject = pSelection->GetObject(0);

	if (pSelObject->GetType() == OBJTYPE_SOLID)
	{
		PyScript::Execute("designer.select_pivotmode()");
	}
	else
	{
		CQuestionDialog::SWarning(QObject::tr(""), QObject::tr("The current selected object(s) do(es)n't support the mesh editing"));
	}
}

REGISTER_ONLY_PYTHON_COMMAND_WITH_EXAMPLE(PySetResultToSuccess, general, set_result_to_success,
                                          "Sets the result of a script execution to success. Used only for Sandbox AutoTests.",
                                          "general.set_result_to_success()");
REGISTER_ONLY_PYTHON_COMMAND_WITH_EXAMPLE(PySetResultToFailure, general, set_result_to_failure,
                                          "Sets the result of a script execution to failure. Used only for Sandbox AutoTests.",
                                          "general.set_result_to_failure()");
REGISTER_ONLY_PYTHON_COMMAND_WITH_EXAMPLE(PyIdleWait, general, idle_wait,
                                          "Waits idling for a given seconds. Primarily used for auto-testing.",
                                          "general.idle_wait(double time)");

REGISTER_PYTHON_COMMAND_WITH_EXAMPLE(CDrxEditApp::Command_ExportToEngine, general, export_to_engine,
                                     "Exports the current level to the engine.",
                                     "general.export_to_engine()");

