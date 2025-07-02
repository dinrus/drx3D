// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/Core.h>

#include <drx3D/CoreX/Platform/platform_impl.inl>
#include <drx3D/Sys/IDrxPluginUpr.h>
#include <drx3D/CoreX/Game/IGameFramework.h>

#include <drx3D/Schema/EnvPackage.h>
#include <drx3D/Schema/EnvDataType.h>
#include <drx3D/Schema/SerializationEnums.inl>
#include <drx3D/Schema/SerializationUtils.h>
#include <drx3D/Schema/StackString.h>

#include <drx3D/Schema/CVars.h>
#include <drx3D/Schema/ObjectPool.h>
#include <drx3D/Schema/Compiler.h>
#include <drx3D/Schema/CoreEnv.h>
#include <drx3D/Schema/EnvRegistry.h>
#include <drx3D/Schema/RuntimeRegistry.h>
#include <drx3D/Schema/ScriptRegistry.h>
#include <drx3D/Schema/ScriptView.h>
#include <drx3D/Schema/SerializationContext.h>
#include <drx3D/Schema/ValidatorArchive.h>
#include <drx3D/Schema/Log.h>
#include <drx3D/Schema/LogRecorder.h>
#include <drx3D/Schema/SettingsUpr.h>
#include <drx3D/Schema/TimerSystem.h>
#include <drx3D/Schema/UpdateScheduler.h>
#include <drx3D/Schema/UnitTestRegistrar.h>

namespace sxema
{
namespace
{

static void OnLogFileStreamsChange(ICVar* pCVar)
{
	CCore::GetInstance().RefreshLogFileStreams();
}

static void OnLogFileMessageTypesChange(ICVar* pCVar)
{
	CCore::GetInstance().RefreshLogFileMessageTypes();
}

inline bool WantPrePhysicsUpdate()
{
	return !gEnv->pGameFramework->IsGamePaused() && (gEnv->pSystem->GetSystemGlobalState() == ESYSTEM_GLOBAL_STATE_RUNNING) && !gEnv->IsEditing();
}

inline bool WantUpdate()
{
	return !gEnv->pGameFramework->IsGamePaused() && (gEnv->pSystem->GetSystemGlobalState() == ESYSTEM_GLOBAL_STATE_RUNNING);
}

} // Anonymous

static tukk g_szSettingsFolder = "DrxSchematycSettings";

CCore::CCore()
	: m_pEnvRegistry(new CEnvRegistry())
	, m_pScriptRegistry(new CScriptRegistry())
	, m_pRuntimeRegistry(new CRuntimeRegistry())
	, m_pObjectPool(new CObjectPool())
	, m_pCompiler(new CCompiler())
	, m_pTimerSystem(new CTimerSystem())
	, m_pLog(new CLog())
	, m_pLogRecorder(new CLogRecorder())
	, m_pSettingsUpr(new CSettingsUpr())
	, m_pUpdateScheduler(new CUpdateScheduler())
{
	gEnv->pSchematyc = this;
}

CCore::~CCore()
{
	// TODO: This should be moved to a proper shutdown function later.
	m_pEnvRegistry->DeregisterPackage(g_coreEnvPackageGuid);
	// ~TODO

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	m_pLog->Shutdown();

	sxema::CVars::Unregister();

	s_pInstance = nullptr;
	gEnv->pSchematyc = nullptr;
}

tukk CCore::GetName() const
{
	return "DrxSchematycCore";
}

tukk CCore::GetCategory() const
{
	return "Plugin";
}

bool CCore::Initialize(SSysGlobEnv& env, const SSysInitParams& initParams)
{
	SXEMA_CORE_ASSERT(!s_pInstance);

	s_pInstance = this;
	env.pSchematyc = this;

	sxema::CVars::Register();

	m_pLog->Init();
	if (CVars::sc_LogToFile)
	{
		CStackString logFileName;
		i32k applicationInstance = gEnv->pSystem->GetApplicationInstance();
		if (applicationInstance)
		{
			logFileName.Format("DrxSchematyc(%d).log", applicationInstance);
		}
		else
		{
			logFileName = "DrxSchematyc.log";
		}
		m_pLogFileOutput = m_pLog->CreateFileOutput(logFileName.c_str());
		SXEMA_CORE_ASSERT(m_pLogFileOutput);
		RefreshLogFileStreams();
		CVars::sc_LogFileStreams->SetOnChangeCallback(OnLogFileStreamsChange);
		RefreshLogFileMessageTypes();
		CVars::sc_LogFileMessageTypes->SetOnChangeCallback(OnLogFileMessageTypesChange);
	}

	if (CVars::sc_RunUnitTests)
	{
		CUnitTestRegistrar::RunUnitTests();
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CCore");

	env.pSchematyc = this;

	if (!m_pEnvRegistry->RegisterPackage(SXEMA_MAKE_ENV_PACKAGE(g_coreEnvPackageGuid, "CoreEnv", g_szDinrus, "Core sxema environment", SXEMA_DELEGATE(&RegisterCoreEnvPackage))))
	{
		env.pSchematyc = nullptr;
		return false;
	}

	return true;
}

void CCore::RefreshEnv()
{
	// #SchematycTODO : Ensure that no objects exist?
	m_pRuntimeRegistry->Reset();
	m_pEnvRegistry->Refresh();
	// #SchematycTODO : Re-load and re-compile scripts?
}

void CCore::SetGUIDGenerator(const GUIDGenerator& guidGenerator)
{
	m_guidGenerator = guidGenerator;
}

DrxGUID CCore::CreateGUID() const
{
	SXEMA_CORE_ASSERT(m_guidGenerator);
	return m_guidGenerator ? m_guidGenerator() : DrxGUID();
}

tukk CCore::GetRootFolder() const
{
	return CVars::sc_RootFolder->GetString();
}

tukk CCore::GetScriptsFolder() const
{
	m_scriptsFolder = PathUtil::GetGameFolder();
	return m_scriptsFolder.c_str();
}

tukk CCore::GetSettingsFolder() const
{
	m_settingsFolder = PathUtil::GetGameFolder();
	m_settingsFolder.append("/");
	m_settingsFolder.append(g_szSettingsFolder);
	return m_settingsFolder.c_str();
}

bool CCore::IsExperimentalFeatureEnabled(tukk szFeatureName) const
{
	return DrxStringUtils::stristr(CVars::sc_ExperimentalFeatures->GetString(), szFeatureName) != nullptr;
}

IEnvRegistry& CCore::GetEnvRegistry()
{
	SXEMA_CORE_ASSERT(m_pEnvRegistry);
	return *m_pEnvRegistry;
}

IScriptRegistry& CCore::GetScriptRegistry()
{
	SXEMA_CORE_ASSERT(m_pScriptRegistry);
	return *m_pScriptRegistry;
}

IRuntimeRegistry& CCore::GetRuntimeRegistry()
{
	SXEMA_CORE_ASSERT(m_pRuntimeRegistry);
	return *m_pRuntimeRegistry;
}

ICompiler& CCore::GetCompiler()
{
	SXEMA_CORE_ASSERT(m_pCompiler);
	return *m_pCompiler;
}

ILog& CCore::GetLog()
{
	return *m_pLog;
}

ILogRecorder& CCore::GetLogRecorder()
{
	return *m_pLogRecorder;
}

ISettingsUpr& CCore::GetSettingsUpr()
{
	return *m_pSettingsUpr;
}

IUpdateScheduler& CCore::GetUpdateScheduler()
{
	return *m_pUpdateScheduler;
}

ITimerSystem& CCore::GetTimerSystem()
{
	return *m_pTimerSystem;
}

IValidatorArchivePtr CCore::CreateValidatorArchive(const SValidatorArchiveParams& params) const
{
	return std::make_shared<CValidatorArchive>(params);
}

ISerializationContextPtr CCore::CreateSerializationContext(const SSerializationContextParams& params) const
{
	return std::make_shared<CSerializationContext>(params);
}

IScriptViewPtr CCore::CreateScriptView(const DrxGUID& scopeGUID) const
{
	return std::make_shared<CScriptView>(scopeGUID);
}

bool CCore::CreateObject(const sxema::SObjectParams& params, IObject*& pObjectOut)
{
	return m_pObjectPool->CreateObject(params, pObjectOut);
}

IObject* CCore::GetObject(ObjectId objectId)
{
	return m_pObjectPool->GetObject(objectId);
}

void CCore::DestroyObject(ObjectId objectId)
{
	m_pObjectPool->DestroyObject(objectId);
}

void CCore::SendSignal(ObjectId objectId, const SObjectSignal& signal)
{
	m_pObjectPool->SendSignal(objectId, signal);
}

void CCore::BroadcastSignal(const SObjectSignal& signal)
{
	m_pObjectPool->BroadcastSignal(signal);
}

void CCore::PrePhysicsUpdate()
{
	if (WantPrePhysicsUpdate())
	{
		m_pUpdateScheduler->BeginFrame(gEnv->pTimer->GetFrameTime());
		m_pUpdateScheduler->Update(EUpdateStage::PrePhysics | EUpdateDistribution::Earliest, EUpdateStage::PrePhysics | EUpdateDistribution::End);
	}
}

void CCore::Update()
{
	if (WantUpdate())
	{
		if (!m_pUpdateScheduler->InFrame())
		{
			m_pUpdateScheduler->BeginFrame(gEnv->pTimer->GetFrameTime());
		}

		if (gEnv->IsEditing())
		{
			m_pUpdateScheduler->Update(EUpdateStage::Editing | EUpdateDistribution::Earliest, EUpdateStage::Editing | EUpdateDistribution::End);
			m_pUpdateScheduler->EndFrame();
		}
		else
		{
			m_pTimerSystem->Update();

			m_pUpdateScheduler->Update(EUpdateStage::Default | EUpdateDistribution::Earliest, EUpdateStage::Post | EUpdateDistribution::End);
			m_pUpdateScheduler->EndFrame();
		}

		m_pLog->Update();
	}
}

void CCore::LoadProjectFiles()
{
	GetLogRecorder().Begin();

	DrxLogAlways("[sxema]: Loading...");
	DrxLogAlways("[sxema]: Loading settings");
	GetSettingsUpr().LoadAllSettings();
	DrxLogAlways("[sxema]: Loading scripts");
	GetScriptRegistry().Load();
	DrxLogAlways("[sxema]: Compiling scripts");
	GetCompiler().CompileAll();
	DrxLogAlways("[sxema]: Loading complete");

	RefreshLogFileSettings();
	if (gEnv->IsEditor())
	{
		GetLogRecorder().End();
	}
}

void CCore::RefreshLogFileSettings()
{
	RefreshLogFileStreams();
	RefreshLogFileMessageTypes();
}

void CCore::RefreshLogFileStreams()
{
	if (m_pLogFileOutput)
	{
		m_pLogFileOutput->DisableAllStreams();
		CStackString logFileStreams = CVars::sc_LogFileStreams->GetString();
		u32k length = logFileStreams.length();
		i32 pos = 0;
		do
		{
			CStackString token = logFileStreams.Tokenize(" ", pos);
			const LogStreamId logStreamId = gEnv->pSchematyc->GetLog().GetStreamId(token.c_str());
			if (logStreamId != LogStreamId::Invalid)
			{
				m_pLogFileOutput->EnableStream(logStreamId);
			}
		}
		while (pos < length);
	}
}

void CCore::RefreshLogFileMessageTypes()
{
	if (m_pLogFileOutput)
	{
		m_pLogFileOutput->DisableAllMessageTypes();
		CStackString logFileMessageTypes = CVars::sc_LogFileMessageTypes->GetString();
		u32k length = logFileMessageTypes.length();
		i32 pos = 0;
		do
		{
			CStackString token = logFileMessageTypes.Tokenize(" ", pos);
			const ELogMessageType logMessageType = gEnv->pSchematyc->GetLog().GetMessageType(token.c_str());
			if (logMessageType != ELogMessageType::Invalid)
			{
				m_pLogFileOutput->EnableMessageType(logMessageType);
			}
		}
		while (pos < length);
	}
}

CRuntimeRegistry& CCore::GetRuntimeRegistryImpl()
{
	SXEMA_CORE_ASSERT(m_pRuntimeRegistry);
	return *m_pRuntimeRegistry;
}

CCompiler& CCore::GetCompilerImpl()
{
	SXEMA_CORE_ASSERT(m_pCompiler);
	return *m_pCompiler;
}

CCore& CCore::GetInstance()
{
	SXEMA_CORE_ASSERT(s_pInstance);
	return *s_pInstance;
}

void CCore::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	if (event == ESYSTEM_EVENT_GAME_FRAMEWORK_INIT_DONE)
	{
		LoadProjectFiles();
	}
}

CCore* CCore::s_pInstance = nullptr;

DRXREGISTER_SINGLETON_CLASS(CCore)

} // sxema
