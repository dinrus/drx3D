// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*
 * Interfaces for the FlowGraph Modules System.
 *
 * A Upr (IFlowGraphModuleUpr) contains Modules (IFlowGraphModule) who run instances (IModuleInstance).
 * DinrusAction has an implementation of these components for runtime and the Editor contains an implementation of the ModuleListener for additional event handling.
 *
 * A module has its graph definition that will be cloned when creating instances.
 * A module manages its own instances (creates, updates, destroys) without communicating with the manager.
 * The module keeps track of the instance IDs that will be requested by parsing the graph at startup.
 * An instance with a specific ID can be created and destroyed multiple times and reused if existing.
 * Automatically generated IDs will never take one of the possible startup IDs.
 *
 * An instance is created when a module is called and instantiates the module's graph nodes, including a Start and End node.
 * The same instance can be referenced by multiple call nodes.
 */

	#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>

//! Module Ids.
typedef uint TModuleId;
static const TModuleId MODULEID_INVALID = -1;

//! Instance Ids.
typedef uint TModuleInstanceId;
static const TModuleInstanceId MODULEINSTANCE_INVALID = -1;

//! Parameter passing object.
typedef DynArray<TFlowInputData>             TModuleParams;

//  предварительные объявления
struct IFlowGraphModule;

// Module Instance Interface
struct IModuleInstance
{
	TModuleInstanceId m_instanceId;
	IFlowGraphModule* m_pModule; //! pointer to module that this instance belongs to
	IFlowGraphPtr     m_pGraph;
	bool              m_bUsed;

	TFlowGraphId      m_callerGraph;
	TFlowNodeId       m_callerNode;

	IModuleInstance(IFlowGraphModule* pModule, TModuleInstanceId id)
		: m_instanceId(id),
		m_pModule(pModule),
		m_pGraph(nullptr),
		m_bUsed(false),
		m_callerGraph(InvalidFlowGraphId),
		m_callerNode(InvalidFlowNodeId)
	{}
	virtual ~IModuleInstance() {}

	bool operator==(const IModuleInstance& other)
	{
		return (m_pGraph == other.m_pGraph && m_callerGraph == other.m_callerGraph && m_callerNode == other.m_callerNode
		        && m_instanceId == other.m_instanceId && m_pModule == other.m_pModule && m_bUsed == other.m_bUsed);
	}
};

// Instance Iterator
struct IFlowGraphModuleInstanceIterator
{
	virtual ~IFlowGraphModuleInstanceIterator(){}
	virtual size_t           Count() = 0;
	virtual IModuleInstance* Next() = 0;
	virtual void             AddRef() = 0;
	virtual void             Release() = 0;
};
typedef _smart_ptr<IFlowGraphModuleInstanceIterator> IModuleInstanceIteratorPtr;

// Module Interface
struct IFlowGraphModule
{
	enum EType
	{
		eT_Global = 0,
		eT_Level
	};

	virtual ~IFlowGraphModule() {}

	struct SModulePortConfig
	{
		SModulePortConfig() : type(eFDT_Void), input(true) {}

		inline bool operator==(const SModulePortConfig& other)
		{
			return (name == other.name) && (type == other.type) && (input == other.input);
		}

		string         name;
		EFlowDataTypes type;
		bool           input;
	};

	virtual tukk                                GetName() const = 0;
	virtual tukk                                GetPath() const = 0;
	virtual TModuleId                                  GetId() const = 0;

	virtual IFlowGraphModule::EType                    GetType() const = 0;
	virtual void                                       SetType(IFlowGraphModule::EType type) = 0;

	virtual IFlowGraph*                                GetRootGraph() const = 0;
	virtual bool                                       HasInstanceGraph(IFlowGraphPtr pGraph) = 0; //! Returns true if the specified graph belongs to an instance of this module

	virtual size_t                                     GetModuleInputPortCount() const = 0;
	virtual size_t                                     GetModuleOutputPortCount() const = 0;
	virtual const IFlowGraphModule::SModulePortConfig* GetModuleInputPort(size_t index) const = 0;
	virtual const IFlowGraphModule::SModulePortConfig* GetModuleOutputPort(size_t index) const = 0;
	virtual bool                                       AddModulePort(const SModulePortConfig& port) = 0;
	virtual void                                       RemoveModulePorts() = 0;

	virtual IModuleInstanceIteratorPtr                 CreateInstanceIterator() = 0;
	virtual size_t                                     GetRunningInstancesCount() const = 0;

	virtual void                                       CallDefaultInstanceForEntity(IEntity* pEntity) = 0;
};
typedef _smart_ptr<IFlowGraphModule> IFlowGraphModulePtr;

// Module Iterator
struct IFlowGraphModuleIterator
{
	virtual ~IFlowGraphModuleIterator(){}
	virtual size_t            Count() = 0;
	virtual IFlowGraphModule* Next() = 0;
	virtual void              AddRef() = 0;
	virtual void              Release() = 0;
};
typedef _smart_ptr<IFlowGraphModuleIterator> IModuleIteratorPtr;

// Module Listener
struct IFlowGraphModuleListener
{
	enum class ERootGraphChangeReason
	{
		ScanningForModules,
		LoadModuleFile
	};

	//! Called once a new module instance was created.
	virtual void OnModuleInstanceCreated(IFlowGraphModule* module, TModuleInstanceId instanceID) = 0;

	//! Called once a new module instance was destroyed.
	virtual void OnModuleInstanceDestroyed(IFlowGraphModule* module, TModuleInstanceId instanceID) = 0;

	//! Called just before a module is destroyed and deleted.
	virtual void OnModuleDestroyed(IFlowGraphModule* module) = 0;

	//! Called once a module's root graph has changed.
	virtual void OnRootGraphChanged(IFlowGraphModule* module, ERootGraphChangeReason reason) = 0;

	//! Called after new all modules are reloaded
	virtual void OnModulesScannedAndReloaded() = 0;

protected:
	virtual ~IFlowGraphModuleListener() {};
};

// Module Upr
struct IFlowGraphModuleUpr
{
	virtual ~IFlowGraphModuleUpr() {}

	virtual bool               RegisterListener(IFlowGraphModuleListener* pListener, tukk name) = 0;
	virtual void               UnregisterListener(IFlowGraphModuleListener* pListener) = 0;


	//! Delete a module definition from disk and the internal registry
	virtual bool               DeleteModuleXML(tukk moduleName) = 0;

	//! Rename a module definition in disk and in the internal registry
	virtual bool               RenameModuleXML(tukk moduleName, tukk newName) = 0;

	//! Get the path on disk were the module definition is saved
	virtual tukk        GetModulePath(tukk name) = 0;
	        tukk        GetGlobalModulesPath() const { return s_globalModulesFolder; }
	        tukk        GetLevelModulesPath() const { return s_levelModulesFolder; }

	virtual bool               SaveModule(tukk moduleName, XmlNodeRef saveTo) = 0;
	virtual IFlowGraphModule*  LoadModuleFile(tukk moduleName, tukk fileName, bool bGlobal) = 0;

	//! Getter methods
	virtual IFlowGraphModule*  GetModule(IFlowGraphPtr pFlowgraph) const = 0;
	virtual IFlowGraphModule*  GetModule(tukk moduleName) const = 0;
	virtual IFlowGraphModule*  GetModule(TModuleId id) const = 0;

	virtual tukk        GetStartNodeName(tukk moduleName) const = 0;
	virtual tukk        GetReturnNodeName(tukk moduleName) const = 0;
	virtual tukk        GetCallerNodeName(tukk moduleName) const = 0;


	//! Unload all loaded modules
	virtual void               ClearModules() = 0;
	//! Unload only the loaded level modules, not the global ones
	virtual void               ClearLevelModules() = 0;

	virtual void               ScanAndReloadModules(bool bScanGlobalModules, bool bScanLevelModules, tukk szLoadedLevelName) = 0;

	virtual bool               CreateModuleNodes(tukk moduleName, TModuleId moduleId) = 0;

	virtual IModuleIteratorPtr CreateModuleIterator() = 0;

private:
	static constexpr tukk s_globalModulesFolder = "Libs/FlowgraphModules";
	static constexpr tukk s_levelModulesFolder = "FlowgraphModules";
};
