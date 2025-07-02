// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/ITimerSystem.h>
#include <drx3D/Schema/Transform.h>
#include <drx3D/Schema/IEnvComponent.h>

#include <drx3D/Schema/RuntimeClass.h>

namespace sxema
{

// Forward declare interfaces.

struct IObjectProperties;
// Forward declare structures.
struct SUpdateContext;
// Forward declare classes.
class CAction;
class CActionDesc;

class CRuntimeParamMap;
class CScratchpad;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(IObjectProperties)
DECLARE_SHARED_POINTERS(CAction)

DECLARE_SHARED_POINTERS(CRuntimeClass)

struct SQueuedObjectSignal
{
	inline SQueuedObjectSignal() {}

	inline SQueuedObjectSignal(const SQueuedObjectSignal& rhs)
		: signal(rhs.signal)
		, scratchpad(rhs.scratchpad)
	{}

	inline SQueuedObjectSignal(const SObjectSignal& rhs)
		: signal(rhs.typeGUID, rhs.senderGUID)
	{
		auto visitInput = [this](const CUniqueId& id, const CAnyConstRef& value)
		{
			u32k pos = scratchpad.Add(value);
			const CAnyPtr pValue = scratchpad.Get(pos);
			signal.params.BindInput(id, pValue);
		};
		rhs.params.VisitInputs(visitInput);

		auto visitOutput = [this](const CUniqueId& id, const CAnyConstRef& value)
		{
			u32k pos = scratchpad.Add(value);
			const CAnyPtr pValue = scratchpad.Get(pos);
			signal.params.BindOutput(id, pValue);
		};
		rhs.params.VisitOutputs(visitOutput);
	}

	SObjectSignal   signal;
	StackScratchpad scratchpad;
};

typedef std::deque<SQueuedObjectSignal> ObjectSignalQueue;

class CObject : public IObject
{
private:

	typedef std::vector<CRuntimeGraphInstance> Graphs;

	struct SStateMachine
	{
		SStateMachine();

		u32 stateIdx;
	};

	typedef std::vector<SStateMachine> StateMachines;

	struct SComponent
	{
		SComponent(const SRuntimeClassComponentInstance& inst, const IEnvComponent* pEnvComponent)
			: classComponentInstance(inst)
			, classDesc(pEnvComponent->GetDesc())
			, pComponent(pEnvComponent->CreateFromPool())
		{};
		const SRuntimeClassComponentInstance& classComponentInstance;
		const CClassDesc&                     classDesc;
		std::shared_ptr<IEntityComponent>     pComponent;
	};
	typedef std::vector<SComponent> Components;

	struct SAction
	{
		SAction(const CActionDesc& _desc, const SRuntimeActionDesc& _runtimeDesc, const CActionPtr& _ptr);

		const CActionDesc& desc;
		SRuntimeActionDesc runtimeDesc;
		CActionPtr         ptr;
		bool               bRunning = false;
	};

	typedef std::vector<SAction> Actions;

	struct STimer
	{
		STimer(CObject* _pObject, const DrxGUID& _guid, TimerFlags _flags);

		void Activate();

		CObject*   pObject;
		DrxGUID    guid;
		TimerFlags flags;
		TimerId    id;
	};

	typedef std::vector<STimer> Timers;

	struct SState
	{
		Timers timers;
	};

	typedef std::vector<SState> States;

public:
	CObject(IEntity& entity, ObjectId id, uk pCustomData);
	~CObject();

	bool Init(DrxGUID classGUID, const IObjectPropertiesPtr& pProperties);

	// IObject
	virtual ObjectId             GetId() const override;
	virtual const IRuntimeClass& GetClass() const override;
	virtual tukk          GetScriptFile() const override;
	virtual uk                GetCustomData() const override;
	virtual ESimulationMode      GetSimulationMode() const override;

	virtual bool                 SetSimulationMode(ESimulationMode simulationMode, EObjectSimulationUpdatePolicy updatePolicy) override;
	virtual void                 ProcessSignal(const SObjectSignal& signal) override;
	virtual void                 StopAction(CAction& action) override;

	virtual EVisitResult         VisitComponents(const ObjectComponentConstVisitor& visitor) const override;
	virtual void                 Dump(IObjectDump& dump, const ObjectDumpFlags& flags = EObjectDumpFlags::All) const override;

	virtual IEntity*             GetEntity() const final           { return m_pEntity; };

	virtual IObjectPropertiesPtr GetObjectProperties() const final { return m_pProperties; };
	// ~IObject

	CScratchpad&      GetScratchpad();

	bool              ExecuteFunction(u32 functionIdx, CRuntimeParamMap& params);
	bool              StartAction(u32 actionIdx, CRuntimeParamMap& params);

	IEntityComponent* GetComponent(u32 componentId);

private:
	bool InitClass();

	void StopSimulation();
	void Update(const SUpdateContext& updateContext);

	void CreateGraphs();
	void ResetGraphs();

	bool ReadProperties();

	void ExecuteConstructors(ESimulationMode simulationMode);

	bool CreateStateMachines();
	void StartStateMachines(ESimulationMode simulationMode);
	void StopStateMachines();
	void DestroyStateMachines();

	bool CreateComponents();
	void ShutdownComponents();
	void DestroyComponents();

	bool CreateActions();
	bool InitActions();
	void ShutdownActions();
	void DestroyActions();

	bool CreateTimers();
	void StartTimers(ESimulationMode simulationMode);
	void StopTimers();
	void DestroyTimers();

	void RegisterForUpdate();

	void ExecuteSignalReceivers(const SObjectSignal& signal);

	void StartStateTimers(u32 stateMachineIdx);
	void StopStateTimers(u32 stateMachineIdx);
	void ExecuteStateSignalReceivers(u32 stateMachineIdx, const SObjectSignal& signal);
	bool EvaluateStateTransitions(u32 stateMachineIdx, const SObjectSignal& signal);
	void ChangeState(u32 stateMachineIdx, u32 stateIdx);

	void ExecuteFunction(const SRuntimeFunction& function, CRuntimeParamMap& params);
	void ExecuteFunction(u32 graphIdx, CRuntimeParamMap& params, SRuntimeActivationParams activationParams);
	void ProcessSignalQueue();

private:
	ObjectId              m_id;

	CRuntimeClassConstPtr m_pClass;
	uk                 m_pCustomData;
	IObjectPropertiesPtr  m_pProperties;
	ESimulationMode       m_simulationMode;

	HeapScratchpad        m_scratchpad;
	Graphs                m_graphs;

	StateMachines         m_stateMachines;

	Actions               m_actions;
	Timers                m_timers;
	States                m_states;

	bool                  m_bQueueSignals;
	ObjectSignalQueue     m_signalQueue;

	CConnectionScope      m_connectionScope;

	// Components created by sxema object
	Components m_components;

	IEntity*   m_pEntity;
};

} // sxema
