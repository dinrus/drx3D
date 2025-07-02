// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/ICore.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Sys/ISystem.h>

namespace sxema
{

// Forward declare interfaces.
struct ILogOutput;
// Forward declare classes.
class CCompiler;
class CEnvRegistry;
class CLog;
class CLogRecorder;
class CObjectPool;
class CRuntimeRegistry;
class CScriptRegistry;
class CSettingsUpr;
class CTimerSystem;
class CUpdateScheduler;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(ILogOutput)

class CCore : public IDrxSchematycCore, public ISystemEventListener
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(Drx::IDefaultModule)
	DRXINTERFACE_ADD(IDrxSchematycCore)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CCore, "EngineModule_SchemaCore", "96d98d98-35aa-4fb6-830b-53dbfe71908d"_drx_guid)

public:

	CCore();

	virtual ~CCore();

	// Drx::IDefaultModule
	virtual tukk GetName() const override;
	virtual tukk GetCategory() const override;
	virtual bool        Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override;
	// ~Drx::IDefaultModule

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IDrxSchematycCore
	virtual void                     SetGUIDGenerator(const GUIDGenerator& guidGenerator) override;
	virtual DrxGUID                  CreateGUID() const override;

	virtual tukk              GetRootFolder() const override;
	virtual tukk              GetScriptsFolder() const override;
	virtual tukk              GetSettingsFolder() const override;
	virtual bool                     IsExperimentalFeatureEnabled(tukk szFeatureName) const override;

	virtual IEnvRegistry&            GetEnvRegistry() override;
	virtual IScriptRegistry&         GetScriptRegistry() override;
	virtual IRuntimeRegistry&        GetRuntimeRegistry() override;
	virtual ICompiler&               GetCompiler() override;
	virtual ILog&                    GetLog() override;
	virtual ILogRecorder&            GetLogRecorder() override;
	virtual ISettingsUpr&        GetSettingsUpr() override;
	virtual IUpdateScheduler&        GetUpdateScheduler() override;
	virtual ITimerSystem&            GetTimerSystem() override;

	virtual IValidatorArchivePtr     CreateValidatorArchive(const SValidatorArchiveParams& params) const override;
	virtual ISerializationContextPtr CreateSerializationContext(const SSerializationContextParams& params) const override;
	virtual IScriptViewPtr           CreateScriptView(const DrxGUID& scopeGUID) const override;

	virtual bool                     CreateObject(const sxema::SObjectParams& params, IObject*& pObjectOut) override;
	virtual IObject*                 GetObject(ObjectId objectId) override;
	virtual void                     DestroyObject(ObjectId objectId) override;
	virtual void                     SendSignal(ObjectId objectId, const SObjectSignal& signal) override;
	virtual void                     BroadcastSignal(const SObjectSignal& signal) override;

	virtual void                     RefreshLogFileSettings() override;
	virtual void                     RefreshEnv() override;

	virtual void                     PrePhysicsUpdate() override;
	virtual void                     Update() override;
	// ~IDrxSchematycCore

	void              RefreshLogFileStreams();
	void              RefreshLogFileMessageTypes();

	CRuntimeRegistry& GetRuntimeRegistryImpl();
	CCompiler&        GetCompilerImpl();

	static CCore&     GetInstance();

private:

	void LoadProjectFiles();

private:

	static CCore*                     s_pInstance;

	GUIDGenerator                     m_guidGenerator;
	mutable string                    m_scriptsFolder;    // #SchematycTODO : How can we avoid making this mutable?
	mutable string                    m_settingsFolder;   // #SchematycTODO : How can we avoid making this mutable?
	std::unique_ptr<CEnvRegistry>     m_pEnvRegistry;
	std::unique_ptr<CScriptRegistry>  m_pScriptRegistry;
	std::unique_ptr<CRuntimeRegistry> m_pRuntimeRegistry;
	std::unique_ptr<CCompiler>        m_pCompiler;
	std::unique_ptr<CObjectPool>      m_pObjectPool;
	std::unique_ptr<CTimerSystem>     m_pTimerSystem;
	std::unique_ptr<CLog>             m_pLog;
	ILogOutputPtr                     m_pLogFileOutput;
	std::unique_ptr<CLogRecorder>     m_pLogRecorder;
	std::unique_ptr<CSettingsUpr> m_pSettingsUpr;
	std::unique_ptr<CUpdateScheduler> m_pUpdateScheduler;
};

} // sxema
