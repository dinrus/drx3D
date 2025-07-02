// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IDrxPlugin.h>

class CRootMonoDomain;
class CAppDomain;
class CMonoDomain;
class CMonoLibrary;

struct IMonoNativeToManagedInterface;

namespace BehaviorTree { class Node; }

//! Represents a C# plug-in dll, used by Drx::IPluginUpr to communicate with Mono
struct IManagedPlugin
{
	virtual ~IManagedPlugin() {}

	virtual void Load(CAppDomain* pDomain) = 0;
	virtual void OnCoreLibrariesDeserialized() = 0;
	virtual void OnPluginLibrariesDeserialized() = 0;
	virtual void SetLoadIndex(i32 order) = 0;
};

struct IMonoListener
{
	virtual ~IMonoListener() {}

	//! Forwarded from DinrusSystem
	//! \param flags One or more flags from ESystemUpdateFlags structure.
	//! \param nPauseMode 0=normal(no pause), 1=menu/pause, 2=cutscene.
	virtual void OnUpdate(i32 updateFlags, i32 nPauseMode) = 0;
};

struct IMonoCompileListener
{
	virtual ~IMonoCompileListener() {}

	virtual void OnCompileFinished(tukk szCompileMessage) = 0;
};

struct IManagedNodeCreator
{
	IManagedNodeCreator() {}

	virtual BehaviorTree::Node* Create() = 0;
};

//! Main interface for the Mono/C# engine module, responsible for initializing and managing the Mono API and interacting with C# code
struct IMonoEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IMonoEngineModule, "ae47c989-0ffa-4876-b0b5-fbb833c2b4ef"_drx_guid);

	virtual void                        Shutdown() = 0;

	virtual std::shared_ptr<Drx::IEnginePlugin> LoadBinary(tukk szBinaryPath) = 0;
	
	virtual void                        Update(i32 updateFlags = 0, i32 nPauseMode = 0) = 0;

	virtual void                        RegisterListener(IMonoListener* pListener) = 0;
	virtual void                        UnregisterListener(IMonoListener* pListener) = 0;

	virtual CRootMonoDomain*            GetRootDomain() = 0;
	virtual CMonoDomain*                GetActiveDomain() = 0;
	virtual CAppDomain*                 CreateDomain(tuk name, bool bActivate = false) = 0;
	virtual void                        ReloadPluginDomain() = 0;

	virtual CMonoLibrary*               GetDrxCommonLibrary() const = 0;
	virtual CMonoLibrary*               GetDrxCoreLibrary() const = 0;

	virtual void                        RegisterNativeToManagedInterface(IMonoNativeToManagedInterface& managedInterface) = 0;

	virtual void                        RegisterManagedNodeCreator(tukk szClassName, IManagedNodeCreator* pCreator) = 0;

	virtual void                        RegisterCompileListener(IMonoCompileListener* pListener) = 0;
	virtual void                        UnregisterCompileListener(IMonoCompileListener* pListener) = 0;

	virtual tukk                 GetLatestCompileMessage() const = 0;
	virtual tukk                 GetGeneratedAssemblyName() const = 0;
};