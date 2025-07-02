// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : How can we reduce duplication of functionality between persistent state and intermittent states?
// #SchematycTODO : Create lightweight runtime element id system to replace guids? We can always build an id to guid map (or vice-versa) if we need access to guids at runtime (e.g. for debugging).

#pragma once

#include <drx3D/Schema/ObjectProperties.h>

#include <drx3D/Schema/IRuntimeClass.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/ITimerSystem.h>
#include <drx3D/Schema/ClassProperties.h>
#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/Scratchpad.h>
#include <drx3D/Schema/Transform.h>

namespace sxema
{

// Forward declare classes.
class CAnyConstPtr;
class CAnyConstRef;
class CAnyValue;
class CObjectProperties;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(CAnyValue)
DECLARE_SHARED_POINTERS(CRuntimeGraph)

struct SRuntimeFunction
{
	SRuntimeFunction();
	SRuntimeFunction(u32 _graphIdx, const SRuntimeActivationParams& _activationParams);

	u32                   graphIdx;
	SRuntimeActivationParams activationParams;
};

struct SRuntimeClassFunction : public SRuntimeFunction
{
	SRuntimeClassFunction(const DrxGUID& _guid);
	SRuntimeClassFunction(u32 _graphIdx, const SRuntimeActivationParams& _activationParams);

	DrxGUID guid;
};

typedef std::vector<SRuntimeClassFunction> RuntimeClassFunctions;

struct SRuntimeClassConstructor : public SRuntimeFunction
{
	SRuntimeClassConstructor(u32 _graphIdx, const SRuntimeActivationParams& _activationParams);
};

typedef std::vector<SRuntimeClassConstructor> RuntimeClassConstructors;

struct SRuntimeClassStateMachine
{
	SRuntimeClassStateMachine(const DrxGUID& _guid, tukk _szName);

	DrxGUID            guid;
	string           name;
	SRuntimeFunction beginFunction;
};

typedef std::vector<SRuntimeClassStateMachine> RuntimeClassStateMachines;

struct SRuntimeClassStateTimer : public SRuntimeFunction
{
	SRuntimeClassStateTimer(const DrxGUID& _guid, tukk _szName, const STimerParams& _params);

	DrxGUID        guid;
	string       name;
	STimerParams params;
};

typedef std::vector<SRuntimeClassStateTimer> RuntimeClassStateTimers;

struct SRuntimeClassStateSignalReceiver : public SRuntimeFunction // #SchematycTODO : Rename signal receiver to signal mapping / connection?
{
	SRuntimeClassStateSignalReceiver(const DrxGUID& _signalGUID, const DrxGUID& _senderGUID, u32 _graphIdx, const SRuntimeActivationParams& _activationParams);

	DrxGUID signalGUID;
	DrxGUID senderGUID;
};

typedef std::vector<SRuntimeClassStateSignalReceiver> RuntimeClassStateSignalReceivers;

struct SRuntimeClassStateTransition : public SRuntimeFunction
{
	SRuntimeClassStateTransition(const DrxGUID& _signalGUID, u32 _graphIdx, const SRuntimeActivationParams& _activationParams);

	DrxGUID signalGUID;
};

typedef std::vector<SRuntimeClassStateTransition> RuntimeClassStateTransitions;

struct SRuntimeStateActionDesc
{
	SRuntimeStateActionDesc(const DrxGUID& _guid, const DrxGUID& _typeGUID);

	DrxGUID guid;
	DrxGUID typeGUID; // #SchematycTODO : Can we store a raw pointer to the env action rather than referencing by GUID?
};

typedef std::vector<SRuntimeStateActionDesc> RuntimeStateActionDescs;

struct SRuntimeClassState
{
	SRuntimeClassState(const DrxGUID& _guid, tukk _szName);

	DrxGUID                            guid;
	string                           name;
	RuntimeClassStateTimers          timers;
	RuntimeClassStateSignalReceivers signalReceivers;
	RuntimeClassStateTransitions     transitions;
	RuntimeStateActionDescs          actions;
};

typedef std::vector<SRuntimeClassState> RuntimeClassStates;

struct SRuntimeClassVariable
{
	SRuntimeClassVariable(const DrxGUID& _guid, tukk _szName, bool _bPublic, u32 _pos);

	DrxGUID  guid;
	string name;
	bool   bPublic;
	u32 pos;
};

typedef std::vector<SRuntimeClassVariable> RuntimeClassVariables;

struct SRuntimeClassTimer
{
	SRuntimeClassTimer(const DrxGUID& _guid, tukk _szName, const STimerParams& _params);

	DrxGUID        guid;
	string       name;
	STimerParams params;
};

typedef std::vector<SRuntimeClassTimer> RuntimeClassTimers;

struct SRuntimeClassComponentInstance
{
	SRuntimeClassComponentInstance(const DrxGUID& _guid, tukk _szName, bool _bPublic, const DrxGUID& _componentTypeGUID, const CTransformPtr& _transform, const CClassProperties& _properties, u32 _parentIdx);

	DrxGUID          guid;
	string           name;
	bool             bPublic;
	DrxGUID          componentTypeGUID;    // #SchematycTODO : Can we store a raw pointer to the env component rather than referencing by GUID?
	CTransformPtr    transform;
	CClassProperties properties;
	u32           parentIdx;
};

typedef std::vector<SRuntimeClassComponentInstance> RuntimeClassComponentInstances;

struct SRuntimeActionDesc
{
	SRuntimeActionDesc(const DrxGUID& _guid, const DrxGUID& _typeGUID);

	DrxGUID guid;
	DrxGUID typeGUID; // #SchematycTODO : Can we store a raw pointer to the env action rather than referencing by GUID?
};

typedef std::vector<SRuntimeActionDesc> RuntimeActionDescs;

struct SRuntimeClassSignalReceiver : public SRuntimeFunction // #SchematycTODO : Rename signal receiver to signal mapping / connection?
{
	SRuntimeClassSignalReceiver(const DrxGUID& _signalGUID, const DrxGUID& _senderGUID, u32 _graphIdx, const SRuntimeActivationParams& _activationParams);

	DrxGUID signalGUID;
	DrxGUID senderGUID;
};

typedef std::vector<SRuntimeClassSignalReceiver> RuntimeClassSignalReceivers;

class CRuntimeClass : public IRuntimeClass
{
private:

	typedef std::unique_ptr<CObjectProperties> PropertiesPtr;
	typedef std::vector<CRuntimeGraphPtr>      Graphs;

public:

	CRuntimeClass(time_t timeStamp, const DrxGUID& guid, tukk szName, const DrxGUID& envClassGUID, const CAnyConstPtr& pEnvClassProperties);

	// IRuntimeClass

	virtual time_t                   GetTimeStamp() const override;
	virtual DrxGUID                  GetGUID() const override;
	virtual tukk              GetName() const override;

	virtual const IObjectProperties& GetDefaultProperties() const override;
	virtual DrxGUID                  GetEnvClassGUID() const override;
	virtual CAnyConstPtr             GetEnvClassProperties() const override;
	virtual const CScratchpad&       GetScratchpad() const override;

	// ~IRuntimeClass

	u32                                AddGraph(const DrxGUID& guid, tukk szName);
	u32                                FindGraph(const DrxGUID& guid) const;
	u32                                GetGraphCount() const;
	CRuntimeGraph*                        GetGraph(u32 graphIdx);
	const CRuntimeGraph*                  GetGraph(u32 graphIdx) const;

	u32                                AddFunction(u32 graphIdx, const SRuntimeActivationParams& activationParams);
	u32                                AddFunction(const DrxGUID& guid, u32 graphIdx, const SRuntimeActivationParams& activationParams);
	u32                                FindOrReserveFunction(const DrxGUID& guid);
	const RuntimeClassFunctions&          GetFunctions() const;

	void                                  AddConstructor(u32 graphIdx, const SRuntimeActivationParams& activationParams);
	const RuntimeClassConstructors&       GetConstructors() const;

	u32                                AddStateMachine(const DrxGUID& guid, tukk szName);
	u32                                FindStateMachine(const DrxGUID& guid) const;
	void                                  SetStateMachineBeginFunction(u32 stateMachineIdx, u32 graphIdx, const SRuntimeActivationParams& activationParams);
	const RuntimeClassStateMachines&      GetStateMachines() const;

	u32                                AddState(const DrxGUID& guid, tukk szName);
	u32                                FindState(const DrxGUID& guid) const;
	void                                  AddStateTimer(u32 stateIdx, const DrxGUID& guid, tukk szName, const STimerParams& params);
	void                                  AddStateSignalReceiver(u32 stateIdx, const DrxGUID& signalGUID, const DrxGUID& senderGUID, u32 graphIdx, const SRuntimeActivationParams& activationParams);
	u32                                AddStateAction(u32 stateIdx, const DrxGUID& guid, const DrxGUID& typeGUID);
	void                                  AddStateTransition(u32 stateIdx, const DrxGUID& signalGUID, u32 graphIdx, const SRuntimeActivationParams& activationParams);
	const RuntimeClassStates&             GetStates() const;

	u32                                AddVariable(const DrxGUID& guid, tukk szName, bool bPublic, const CAnyConstRef& value);
	const RuntimeClassVariables&          GetVariables() const;
	u32                                GetVariablePos(const DrxGUID& guid) const;

	u32                                AddTimer(const DrxGUID& guid, tukk szName, const STimerParams& params);
	const RuntimeClassTimers&             GetTimers() const;

	u32                                AddComponentInstance(const DrxGUID& guid, tukk szName, bool bPublic, const DrxGUID& componentTypeGUID, const CTransformPtr& transform, const CClassProperties& properties, u32 parentIdx);
	u32                                FindComponentInstance(const DrxGUID& guid) const;
	const RuntimeClassComponentInstances& GetComponentInstances() const;

	u32                                AddSignalReceiver(const DrxGUID& signalGUID, const DrxGUID& senderGUID, u32 graphIdx, const SRuntimeActivationParams& activationParams);
	const RuntimeClassSignalReceivers&    GetSignalReceivers() const;

	u32                                AddAction(const DrxGUID& guid, const DrxGUID& typeGUID);
	const RuntimeActionDescs&             GetActions() const;

	u32                                CountSignalReceviers(const DrxGUID& signalGUID) const;
	void                                  FinalizeComponentInstances();
	void                                  Finalize();

private:

	time_t                         m_timeStamp;
	DrxGUID                          m_guid;
	string                         m_name;
	PropertiesPtr                  m_pDefaultProperties;

	DrxGUID                          m_envClassGUID;
	CAnyValuePtr                   m_pEnvClassProperties;

	HeapScratchpad                 m_scratchpad;
	Graphs                         m_graphs;

	RuntimeClassFunctions          m_functions;
	RuntimeClassConstructors       m_constructors;
	RuntimeClassStateMachines      m_stateMachines;
	RuntimeClassStates             m_states;
	RuntimeClassVariables          m_variables;
	RuntimeClassTimers             m_timers;
	RuntimeClassComponentInstances m_componentInstances;
	RuntimeClassSignalReceivers    m_signalReceivers;
	RuntimeActionDescs             m_actions;
};

} // sxema
