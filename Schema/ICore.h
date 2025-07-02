// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Sys/IDrxPlugin.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/Delegate.h>

namespace sxema
{

// Forward declare interfaces.
struct ICompiler;
struct IScriptView;
struct IEnvRegistrar;
struct IEnvRegistry;
struct ILog;
struct ILogRecorder;
struct IObject;
struct IRuntimeRegistry;
struct IScriptRegistry;
struct ISerializationContext;
struct ISettingsUpr;
struct ITimerSystem;
struct IUpdateScheduler;
struct IValidatorArchive;
struct IObjectProperties;
// Forward declare structures.
struct SObjectParams;
struct SObjectSignal;
struct SSerializationContextParams;
struct SValidatorArchiveParams;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(IScriptView)
DECLARE_SHARED_POINTERS(ISerializationContext)
DECLARE_SHARED_POINTERS(IValidatorArchive)
DECLARE_SHARED_POINTERS(IObjectProperties);

typedef std::function<DrxGUID()> GUIDGenerator;

struct SObjectParams
{
	DrxGUID              classGUID;
	uk                pCustomData = nullptr;
	IObjectPropertiesPtr pProperties;
	IEntity*             pEntity = nullptr;
};

} // sxema

struct IDrxSchematycCore : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IDrxSchematycCore, "041b8bda-35d7-4341-bde7-f0ca69be2595"_drx_guid)

	virtual void                                SetGUIDGenerator(const sxema::GUIDGenerator& guidGenerator) = 0;
	virtual DrxGUID                             CreateGUID() const = 0;

	virtual tukk                         GetRootFolder() const = 0;
	virtual tukk                         GetScriptsFolder() const = 0;     // #SchematycTODO : Do we really need access to this outside script registry?
	virtual tukk                         GetSettingsFolder() const = 0;    // #SchematycTODO : Do we really need access to this outside env registry?
	virtual bool                                IsExperimentalFeatureEnabled(tukk szFeatureName) const = 0;

	virtual sxema::IEnvRegistry&            GetEnvRegistry() = 0;
	virtual sxema::IScriptRegistry&         GetScriptRegistry() = 0;
	virtual sxema::IRuntimeRegistry&        GetRuntimeRegistry() = 0;
	virtual sxema::ICompiler&               GetCompiler() = 0;
	virtual sxema::ILog&                    GetLog() = 0;
	virtual sxema::ILogRecorder&            GetLogRecorder() = 0;
	virtual sxema::ISettingsUpr&        GetSettingsUpr() = 0;
	virtual sxema::IUpdateScheduler&        GetUpdateScheduler() = 0;
	virtual sxema::ITimerSystem&            GetTimerSystem() = 0;

	virtual sxema::IValidatorArchivePtr     CreateValidatorArchive(const sxema::SValidatorArchiveParams& params) const = 0;
	virtual sxema::ISerializationContextPtr CreateSerializationContext(const sxema::SSerializationContextParams& params) const = 0;
	virtual sxema::IScriptViewPtr           CreateScriptView(const DrxGUID& scopeGUID) const = 0;

	virtual bool                                CreateObject(const sxema::SObjectParams& params, sxema::IObject*& pObjectOut) = 0;
	virtual sxema::IObject*                 GetObject(sxema::ObjectId objectId) = 0;
	virtual void                                DestroyObject(sxema::ObjectId objectId) = 0;
	virtual void                                SendSignal(sxema::ObjectId objectId, const sxema::SObjectSignal& signal) = 0;
	virtual void                                BroadcastSignal(const sxema::SObjectSignal& signal) = 0;

	virtual void                                RefreshLogFileSettings() = 0;
	virtual void                                RefreshEnv() = 0;

	virtual void                                PrePhysicsUpdate() = 0;
	virtual void                                Update() = 0;
};
