// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include <drx3D/CoreX/BoostHelpers.h>

#include <drx3D/Schema2/ILib.h>

#include <drx3D/Schema2/Signal.h>
#include <drx3D/Schema2/Deprecated/VMOps.h>
#include <drx3D/Schema2/EnvRegistry.h>

namespace sxema2
{
	typedef std::vector<size_t> TSizeTVector;

	// Library abstract interface function.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibAbstractInterfaceFunction : public ILibAbstractInterfaceFunction
	{
	public:

		CLibAbstractInterfaceFunction(const SGUID& guid, tukk szName);

		// ILibAbstractInterfaceFunction
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual TVariantConstArray GetVariantInputs() const override;
		virtual TVariantConstArray GetVariantOutputs() const override;
		// ~ILibAbstractInterfaceFunction

		void AddInput(const IAny& value);
		void AddOutput(const IAny& value);

	private:

		SGUID          m_guid;
		string         m_name;
		TVariantVector m_variantInputs;
		TVariantVector m_variantOutputs;
	};

	DECLARE_SHARED_POINTERS(CLibAbstractInterfaceFunction)

	// Library abstract interface.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibAbstractInterface : public ILibAbstractInterface
	{
	public:

		CLibAbstractInterface(const SGUID& guid, tukk name);

		// ILibAbstractInterface
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		// ~ILibAbstractInterface

	private:

		SGUID		m_guid;
		string	m_name;
	};

	DECLARE_SHARED_POINTERS(CLibAbstractInterface)

	// Library state machine.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibStateMachine : public ILibStateMachine
	{
	public:

		CLibStateMachine(const SGUID& guid, tukk name, ELibStateMachineLifetime lifetime);

		// ILibStateMachine
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual ELibStateMachineLifetime GetLifetime() const override;
		virtual size_t GetPartner() const override;
		virtual size_t GetListenerCount() const override;
		virtual size_t GetListener(size_t iListener) const override;
		virtual size_t GetVariableCount() const override;
		virtual size_t GetVariable(const size_t iVariable) const override;
		virtual size_t GetContainerCount() const override;
		virtual size_t GetContainer(const size_t iContainer) const override;
		virtual void SetBeginFunction(const LibFunctionId& functionId) override;
		virtual LibFunctionId GetBeginFunction() const override;
		// ~ILibStateMachine

		void SetPartner(size_t iPartner);
		void AddListener(size_t iListener);
		void AddVariable(const size_t iVariable);
		void AddContainer(const size_t iContainer);

	private:

		SGUID											m_guid;
		string										m_name;
		ELibStateMachineLifetime	m_lifetime;
		size_t										m_iPartner;
		TSizeTVector							m_listeners;
		TSizeTVector							m_variables;
		TSizeTVector							m_containers;
		LibFunctionId							m_beginFunctionId;
	};

	// Library state.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibState : public ILibState
	{
	public:

		CLibState(const SGUID& guid, tukk name);

		// ILibState
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual size_t GetParent() const override;
		virtual size_t GetPartner() const override;
		virtual size_t GetStateMachine() const override;
		virtual size_t GetVariableCount() const override;
		virtual size_t GetVariable(size_t iVariable) const override;
		virtual size_t GetContainerCount() const override;
		virtual size_t GetContainer(size_t iContainer) const override;
		virtual size_t GetTimerCount() const override;
		virtual size_t GetTimer(size_t iTimer) const override;
		virtual size_t GetActionInstanceCount() const override;
		virtual size_t GetActionInstance(size_t iActionInstance) const override;
		virtual size_t GetConstructorCount() const override;
		virtual size_t GetConstructor(size_t iConstructor) const override;
		virtual size_t GetDestructorCount() const override;
		virtual size_t GetDestructor(size_t iDestructor) const override;
		virtual size_t GetSignalReceiverCount() const override;
		virtual size_t GetSignalReceiver(size_t iSignalReceiver) const override;
		virtual size_t GetTransitionCount() const override;
		virtual size_t GetTransition(size_t iTransition) const override;
		// ~ILibState

		void SetParent(size_t iParent);
		void SetPartner(size_t iPartner);
		void SetStateMachine(size_t iStateMachine);
		void AddVariable(size_t iVariable);
		void AddContainer(size_t iContainer);
		void AddTimer(size_t iTimer);
		void AddActionInstance(size_t iActionInstance);
		void AddConstructor(size_t iConstructor);
		void AddDestructor(size_t iDestructor);
		void AddSignalReceiver(size_t iSignalReceiver);
		void AddTransition(size_t iTransition);

	private:

		SGUID					m_guid;
		string				m_name;
		size_t				m_iParent;
		size_t				m_iPartner;
		size_t				m_iStateMachine;
		TSizeTVector	m_variables;
		TSizeTVector	m_containers;
		TSizeTVector	m_timers;
		TSizeTVector	m_actionInstances;
		TSizeTVector	m_constructors;
		TSizeTVector	m_destructors;
		TSizeTVector	m_signalReceivers;
		TSizeTVector	m_transitions;
	};

	// Library variable.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibVariable : public ILibVariable
	{
	public:

		CLibVariable(const SGUID& guid, tukk szName, EOverridePolicy overridePolicy, const IAny& value, size_t variantPos, size_t variantCount, ELibVariableFlags flags);

		// ILibVariable
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual EOverridePolicy GetOverridePolicy() const override;
		virtual IAnyConstPtr GetValue() const override;
		virtual size_t GetVariantPos() const override;
		virtual size_t GetVariantCount() const override;
		virtual ELibVariableFlags GetFlags() const override;
		// ~ILibVariable

		void SetOverridePolicy(EOverridePolicy overridePolicy);

	private:

		SGUID             m_guid;
		string            m_name;
		EOverridePolicy   m_overridePolicy;
		IAnyPtr           m_pValue;
		size_t            m_variantPos;
		size_t            m_variantCount;
		ELibVariableFlags m_flags;
	};

	typedef std::vector<CLibVariable> LibVariables;

	// Library container.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibContainer : public ILibContainer
	{
	public:

		CLibContainer(const SGUID& guid, tukk name, const SGUID& typeGUID);

		// ILibContainer
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual SGUID GetTypeGUID() const override;
		// ~ILibVariable

	private:

		SGUID		m_guid;
		string	m_name;
		SGUID		m_typeGUID;
	};

	// Library timer.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibTimer : public ILibTimer
	{
	public:

		CLibTimer(const SGUID& guid, tukk szName, const STimerParams& params);

		// ILibTimer
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual const STimerParams& GetParams() const override;
		// ~ILibTimer

	private:

		SGUID					m_guid;
		string				m_name;
		STimerParams	m_params;
	};

	// Library abstract interface instance.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibAbstractInterfaceImplementation : public ILibAbstractInterfaceImplementation
	{
	public:

		CLibAbstractInterfaceImplementation(const SGUID& guid, tukk name, const SGUID& interfaceGUID);

		// ILibAbstractInterfaceImplementation
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual SGUID GetInterfaceGUID() const override;
		virtual size_t GetFunctionCount() const override;
		virtual SGUID GetFunctionGUID(size_t iFunction) const override;
		virtual LibFunctionId GetFunctionId(size_t iFunction) const override;
		// ~ILibAbstractInterfaceImplementation

		void AddFunction(const SGUID& guid, const LibFunctionId& functionId);

	private:

		struct SFunction
		{
			SFunction(const SGUID& _guid, const LibFunctionId& _functionId);

			SGUID					guid;
			LibFunctionId	functionId;
		};

		typedef std::vector<SFunction> TFunctionVector;

		SGUID						m_guid;
		string					m_name;
		SGUID						m_interfaceGUID;
		TFunctionVector	m_functions;
	};

	// Library component instance.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibComponentInstance : public ILibComponentInstance
	{
	public:

		CLibComponentInstance(const SGUID& guid, tukk szName, const SGUID& componentGUID, const IPropertiesConstPtr& pProperties, u32 propertyFunctionIdx, u32 parentIdx);

		// ILibComponentInstance
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual SGUID GetComponentGUID() const override;
		virtual IPropertiesConstPtr GetProperties() const override;
		virtual u32 GetPropertyFunctionIdx() const override;
		virtual u32 GetParentIdx() const override;
		// ~ILibComponentInstance

	private:

		SGUID               m_guid;
		string              m_name;
		SGUID               m_componentGUID;
		IPropertiesConstPtr m_pProperties;
		u32              m_propertyFunctionIdx;
		u32              m_parentIdx;
	};

	// Library action instance.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibActionInstance : public ILibActionInstance
	{
	public:

		CLibActionInstance(const SGUID& guid, tukk szName, const SGUID& actionGUID, size_t iComponentInstance, const IPropertiesPtr& pProperties);

		// ILibActionInstance
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual SGUID GetActionGUID() const override;
		virtual size_t GetComponentInstance() const override;
		virtual IPropertiesConstPtr GetProperties() const override;
		// ~ILibActionInstance

	private:

		SGUID          m_guid;
		string         m_name;
		SGUID          m_actionGUID;
		size_t         m_iComponentInstance;
		IPropertiesPtr m_pProperties;
	};

	// Library constructor.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibConstructor : public ILibConstructor
	{
	public:

		CLibConstructor(const SGUID& guid, const LibFunctionId& functionId);

		// ILibConstructor
		virtual SGUID GetGUID() const override;
		virtual LibFunctionId GetFunctionId() const override;
		// ~ILibConstructor

	private:

		SGUID					m_guid;
		LibFunctionId	m_functionId;
	};

	// Library destructor.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibDestructor : public ILibDestructor
	{
	public:

		CLibDestructor(const SGUID& guid, const LibFunctionId& functionId);

		// ILibDestructor
		virtual SGUID GetGUID() const override;
		virtual LibFunctionId GetFunctionId() const override;
		// ~ILibDestructor

	private:

		SGUID					m_guid;
		LibFunctionId	m_functionId;
	};

	// Library signal receiver.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibSignalReceiver : public ILibSignalReceiver
	{
	public:

		CLibSignalReceiver(const SGUID& guid, const SGUID& contextGUID, const LibFunctionId& functionId);

		// ILibSignalReceiver
		virtual SGUID GetGUID() const override;
		virtual SGUID GetContextGUID() const override;
		virtual LibFunctionId GetFunctionId() const override;
		// ~ILibSignalReceiver

	private:

		SGUID					m_guid;
		SGUID					m_contextGUID;
		LibFunctionId	m_functionId;
	};

	// Library transition.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibTransition : public ILibTransition
	{
	public:

		CLibTransition(const SGUID& guid, const SGUID& contextGUID, const LibFunctionId& functionId);

		// ILibTransition
		virtual SGUID GetGUID() const override;
		virtual SGUID GetContextGUID() const override;
		virtual LibFunctionId GetFunctionId() const override;
		// ~ILibTransition

	private:

		SGUID					m_guid;
		SGUID					m_contextGUID;
		LibFunctionId	m_functionId;
	};

	// Library function.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibFunction : public ILibFunction
	{
	public:

		CLibFunction();

		~CLibFunction();

		// ILibFunction
		virtual SGUID GetGUID() const override;
		virtual SGUID GetClassGUID() const override;
		virtual void SetName(tukk name) override;
		virtual tukk GetName() const override;
		virtual void SetScope(tukk scope) override;
		virtual tukk GetScope() const override;
		virtual void SetFileName(tukk fileName) override;
		virtual tukk GetFileName() const override;
		virtual void SetAuthor(tukk author) override;
		virtual tukk GetAuthor() const override;
		virtual void SetDescription(tukk textDesc) override;
		virtual tukk GetDescription() const override;
		virtual void AddInput(tukk name, tukk textDesc, const IAny& value) override;
		virtual size_t GetInputCount() const override;
		virtual tukk GetInputName(size_t iInput) const override;
		virtual tukk GetInputTextDesc(size_t iInput) const override;
		virtual TVariantConstArray GetVariantInputs() const override;
		virtual void AddOutput(tukk name, tukk textDesc, const IAny& value) override;
		virtual size_t GetOutputCount() const override;
		virtual tukk GetOutputName(size_t iOutput) const override;
		virtual tukk GetOutputTextDesc(size_t iOutput) const override;
		virtual TVariantConstArray GetVariantOutputs() const override;
		virtual TVariantConstArray GetVariantConsts() const override;
		virtual GlobalFunctionConstTable GetGlobalFunctionTable() const override;
		virtual ComponentMemberFunctionConstTable GetComponentMemberFunctionTable() const override;
		virtual ActionMemberFunctionConstTable GetActionMemberFunctionTable() const override;
		virtual size_t GetSize() const override;
		virtual const SVMOp* GetOp(size_t pos) const override;
		// ~ILibFunction

		void SetGUID(const SGUID& guid);
		void SetClassGUID(const SGUID& classGUID);
		size_t AddConstValue(const CVariant& value);
		size_t AddGlobalFunction(const IGlobalFunctionConstPtr& pGlobalFunction);
		size_t AddComponentMemberFunction(const IComponentMemberFunctionConstPtr& pComponentMemberFunction);
		size_t AddActionMemberFunction(const IActionMemberFunctionConstPtr& pActionMemberFunction);
		size_t AddOp(const SVMOp& op);
		SVMOp* GetOp(size_t pos);
		SVMOp* GetLastOp();
		
	private:

		struct SParam
		{
			SParam(tukk _name, tukk _textDesc);

			string	name;
			string	textDesc;
		};

		typedef std::vector<SParam>														TParamVector;
		typedef std::vector<IGlobalFunctionConstPtr>					GlobalFunctionTable;
		typedef std::vector<IComponentMemberFunctionConstPtr>	ComponentMemberFunctionTable;
		typedef std::vector<IActionMemberFunctionConstPtr>		ActionMemberFunctionTable;

		void Release();

		static const size_t	MIN_CAPACITY;
		static const size_t	GROWTH_FACTOR;

		SGUID													m_guid;
		SGUID													m_classGUID;
		string												m_name;
		string												m_scope;
		string												m_fileName;
		string												m_author;
		string												m_textDesc;
		TParamVector									m_inputs;
		TVariantVector								m_variantInputs;
		TParamVector									m_outputs;
		TVariantVector								m_variantOutputs;
		TVariantVector								m_variantConsts;
		GlobalFunctionTable						m_globalFunctionTable;
		ComponentMemberFunctionTable	m_componentMemberFunctionTable;
		ActionMemberFunctionTable			m_actionMemberFunctionTable;
		size_t												m_capacity;
		size_t												m_size;
		size_t												m_lastOpPos;
		u8*												m_pBegin;
	};

	// Library class properties.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibClassProperties : public ILibClassProperties
	{
	public:

		enum class EInternalOverridePolicy // #SchematycTODO : Replace with global EOverridePolicy enum!
		{
			UseDefault,
			OverrideDefault
		};

	private:

		struct SProperty
		{
			SProperty();
			SProperty(tukk _szLabel, const IAnyPtr& _pValue, EInternalOverridePolicy _overridePolicy);
			SProperty(const SProperty& rhs);

			void Serialize(Serialization::IArchive& archive);

			string                  label;
			sxema2::IAnyPtr      pValue;
			EInternalOverridePolicy overridePolicy;
		};

		typedef std::map<SGUID, SProperty>                                         Properties; // #SchematycTODO : Replace with unordered map once YASLI support is enabled?
		typedef std::map<tukk , SProperty&, stl::less_stricmp<tukk > > PropertiesByLabel;

	public:

		CLibClassProperties(const LibVariables& variables);
		CLibClassProperties(const CLibClassProperties& rhs);

		// ILibClassProperties
		virtual ILibClassPropertiesPtr Clone() const override;
		virtual void Serialize(Serialization::IArchive& archive) override;
		virtual IAnyConstPtr GetProperty(const SGUID& guid) const override;
		virtual void VisitProperties(const LibClassPropertyVisitor& visitor) const override;
		virtual void OverrideProperty(const SGUID& guid, const IAny& value) override;
		// ~ILibClassProperties

	private:

		Properties m_properties;
	};

	// Library class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLibClass : public ILibClass
	{
	public:

		CLibClass(const ILib& lib, const SGUID& guid, tukk szName, const SGUID& foundationGUID, const IPropertiesConstPtr& pFoundationProperties);

		// ILibClass
		virtual const ILib& GetLib() const override;
		virtual SGUID GetGUID() const override;
		virtual tukk GetName() const override;
		virtual ILibClassPropertiesPtr CreateProperties() const override;
		virtual SGUID GetFoundationGUID() const override;
		virtual IPropertiesConstPtr GetFoundationProperties() const override;
		virtual size_t GetStateMachineCount() const override;
		virtual const ILibStateMachine* GetStateMachine(size_t iStateMachine) const override;
		virtual size_t GetStateCount() const override;
		virtual const ILibState* GetState(size_t iState) const override;
		virtual size_t GetVariableCount() const override;
		virtual const ILibVariable* GetVariable(size_t iVariable) const override;
		virtual TVariantConstArray GetVariants() const override;
		virtual size_t GetContainerCount() const override;
		virtual const ILibContainer* GetContainer(size_t iContainer) const override;
		virtual size_t GetTimerCount() const override;
		virtual const ILibTimer* GetTimer(size_t iTimer) const override;
		virtual size_t GetPersistentTimerCount() const override;
		virtual size_t GetPersistentTimer(size_t iPersistentTimer) const override;
		virtual size_t GetAbstractInterfaceImplementationCount() const override;
		virtual const ILibAbstractInterfaceImplementation* GetAbstractInterfaceImplementation(size_t iAbstractInterfaceImplementation) const override;
		virtual size_t GetComponentInstanceCount() const override;
		virtual const ILibComponentInstance* GetComponentInstance(size_t iComponentInstance) const override;
		virtual size_t GetActionInstanceCount() const override;
		virtual const ILibActionInstance* GetActionInstance(size_t iActionInstance) const override;
		virtual size_t GetPersistentActionInstanceCount() const override;
		virtual size_t GetPersistentActionInstance(size_t iPersistentActionInstance) const override;
		virtual size_t GetConstructorCount() const override;
		virtual const ILibConstructor* GetConstructor(size_t iConstructor) const override;
		virtual size_t GetPersistentConstructorCount() const override;
		virtual size_t GetPersistentConstructor(size_t iPersistentConstructor) const override;
		virtual size_t GetDestructorCount() const override;
		virtual const ILibDestructor* GetDestructor(size_t iDestructor) const override;
		virtual size_t GetPersistentDestructorCount() const override;
		virtual size_t GetPersistentDestructor(size_t iPersistentDestructor) const override;
		virtual size_t GetSignalReceiverCount() const override;
		virtual const ILibSignalReceiver* GetSignalReceiver(size_t iSignalReceiver) const override;
		virtual size_t GetPersistentSignalReceiverCount() const override;
		virtual size_t GetPersistentSignalReceiver(size_t iPersistentSignalReceiver) const override;
		virtual size_t GetTransitionCount() const override;
		virtual const ILibTransition* GetTransition(size_t iTransition) const override;
		virtual LibFunctionId GetFunctionId(const SGUID& guid) const override;
		virtual const ILibFunction* GetFunction(const LibFunctionId& functionId) const override;
		virtual const CRuntimeFunction* GetFunction_New(u32 functionIdx) const override;
		virtual void PreviewGraphFunctions(const SGUID& graphGUID, const StringStreamOutCallback& stringStreamOutCallback) const override;
		virtual void AddPrecacheResource(IAnyConstPtr resource) override;
		virtual size_t GetPrecacheResourceCount() const override;
		virtual IAnyConstPtr GetPrecacheResource(size_t ressourceIdx) const override;
		// ~ILibClass

		size_t AddStateMachine(const SGUID& guid, tukk name, ELibStateMachineLifetime lifetime);
		CLibStateMachine* GetStateMachine(size_t iStateMachine);
		size_t AddState(const SGUID& guid, tukk name);
		CLibState* GetState(size_t iState);
		size_t AddVariable(const SGUID& guid, tukk szName, EOverridePolicy overridePolicy, const IAny& value, size_t variantPos, size_t variantCount, ELibVariableFlags flags);
		CLibVariable* GetVariable(size_t iVariable);
		size_t AddContainer(const SGUID& guid, tukk name, const SGUID& typeGUID);
		CLibContainer* GetContainer(size_t iContainer);
		TVariantVector& GetVariants();
		size_t AddTimer(const SGUID& guid, tukk szName, const STimerParams& params);
		size_t AddPersistentTimer(const SGUID& guid, tukk szName, const STimerParams& params);
		CLibTimer* GetTimer(size_t iTimer);
		size_t AddAbstractInterfaceImplementation(const SGUID& guid, tukk name, const SGUID& interfaceGUID);
		CLibAbstractInterfaceImplementation* GetAbstractInterfaceImplementation(size_t iAbstractInterfaceImplementation);
		size_t AddComponentInstance(const SGUID& guid, tukk szName, const SGUID& componentGUID, const IPropertiesPtr& pProperties, u32 propertyFunctionIdx, u32 parentIdx);
		CLibComponentInstance* GetComponentInstance(size_t iComponentInstance);
		void SortComponentInstances();
		void ValidateSingletonComponentInstances() const;
		size_t AddActionInstance(const SGUID& guid, tukk szName, const SGUID& actionGUID, size_t iComponentInstance, const IPropertiesPtr& pProperties);
		size_t AddPersistentActionInstance(const SGUID& guid, tukk szName, const SGUID& actionGUID, size_t iComponentInstance, const IPropertiesPtr& pProperties);
		CLibActionInstance* GetActionInstance(size_t iActionInstance);
		size_t AddConstructor(const SGUID& guid, const LibFunctionId& functionId);
		size_t AddPersistentConstructor(const SGUID& guid, const LibFunctionId& functionId);
		CLibConstructor* GetConstructor(size_t iConstructor);
		size_t AddDestructor(const SGUID& guid, const LibFunctionId& functionId);
		size_t AddPersistentDestructor(const SGUID& guid, const LibFunctionId& functionId);
		CLibDestructor* GetDestructor(size_t iDestructor);
		size_t AddSignalReceiver(const SGUID& guid, const SGUID& contextGUID, const LibFunctionId& functionId);
		size_t AddPersistentSignalReceiver(const SGUID& guid, const SGUID& contextGUID, const LibFunctionId& functionId);
		CLibSignalReceiver* GetSignalReceiver(size_t iSignalReceiver);
		size_t AddTransition(const SGUID& guid, const SGUID& contextGUID, const LibFunctionId& functionId);
		CLibTransition* GetTransition(size_t iTransition);
		LibFunctionId AddFunction(const SGUID& guid);
		CLibFunction* GetFunction(const LibFunctionId& functionId);
		u32 AddFunction_New(const SGUID& guid);
		CRuntimeFunction* GetFunction_New(u32 functionIdx);
		bool BindFunctionToGUID(const LibFunctionId& functionId, const SGUID& guid);

	private:

		struct SFunction // #SchematycTODO : Can't we just store the id and graph guid in the function itself?
		{
			SFunction(const LibFunctionId& _functionId, const SGUID& _graphGUID);

			LibFunctionId functionId;
			SGUID         graphGUID;
			CLibFunction  function;
		};

		DECLARE_SHARED_POINTERS(CRuntimeFunction)

		typedef std::vector<CLibStateMachine>                     TStateMachineVector;
		typedef std::vector<CLibState>                            TStateVector;
		typedef std::vector<CLibContainer>                        TContainerVector;
		typedef std::vector<CLibTimer>                            TTimerVector;
		typedef std::vector<CLibAbstractInterfaceImplementation>  TAbstractInterfaceImplementationVector;
		typedef std::vector<CLibComponentInstance>                TComponentInstanceVector;
		typedef std::vector<CLibActionInstance>                   TActionInstanceVector;
		typedef std::vector<CLibConstructor>                      TConstructorVector;
		typedef std::vector<CLibDestructor>                       TDestructorVector;
		typedef std::vector<CLibSignalReceiver>                   TSignalReceiverVector;
		typedef std::vector<CLibTransition>                       TTransitionVector;
		typedef std::map<LibFunctionId, SFunction>                TFunctionMap;
		typedef std::unordered_map<SGUID, LibFunctionId>          TFunctionIdByGUIDMap;
		typedef std::vector<CRuntimeFunctionPtr>                  Functions_New;
		typedef std::vector<IAnyConstPtr>                         TAnyConstPtrVector;

		void PreviewFunction(const CLibFunction& function, StringStreamOutCallback stringStreamOutCallback) const;

		const ILib&                            m_lib;
		SGUID                                  m_guid;
		string                                 m_name;
		SGUID                                  m_foundationGUID;
		IPropertiesPtr                         m_pFoundationProperties;
		TStateMachineVector                    m_stateMachines;
		TStateVector                           m_states;
		LibVariables                           m_variables;
		TContainerVector                       m_containers;
		TVariantVector                         m_variants;
		TTimerVector                           m_timers;
		TSizeTVector                           m_persistentTimers;
		TAbstractInterfaceImplementationVector m_abstractInterfaceImplementations;
		TComponentInstanceVector               m_componentInstances;
		TActionInstanceVector                  m_actionInstances;
		TSizeTVector                           m_persistentActionInstances;
		TConstructorVector                     m_constructors;
		TSizeTVector                           m_persistentConstructors;
		TDestructorVector                      m_destructors;
		TSizeTVector                           m_persistentDestructors;
		TSignalReceiverVector                  m_signalReceivers;
		TSizeTVector                           m_persistentSignalReceivers;
		TTransitionVector                      m_transitions;
		TFunctionMap                           m_functions;
		TFunctionIdByGUIDMap                   m_functionIdsByGUID;
		Functions_New                          m_functions_New;
		LibFunctionId                          m_nextFunctionId;
		TAnyConstPtrVector                     m_resourcesToPrecache;
	};

	DECLARE_SHARED_POINTERS(CLibClass)

	// Library.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLib : public ILib
	{
	public:

		CLib(tukk name);

		// ILib
		virtual tukk GetName() const override;
		virtual ISignalConstPtr GetSignal(const SGUID& guid) const override;
		virtual void VisitSignals(const LibSignalVisitor& visitor) override;
		virtual ILibAbstractInterfaceConstPtr GetAbstractInterface(const SGUID& guid) const override;
		virtual void VisitAbstractInterfaces(const LibAbstractInterfaceVisitor& visitor) override;
		virtual ILibAbstractInterfaceFunctionConstPtr GetAbstractInterfaceFunction(const SGUID& guid) const override;
		virtual void VisitAbstractInterfaceFunctions(const LibAbstractInterfaceFunctionVisitor& visitor) override;
		virtual ILibClassConstPtr GetClass(const SGUID& guid) const override;
		virtual void VisitClasses(const LibClassVisitor& visitor) const override;
		virtual void PreviewGraphFunctions(const SGUID& graphGUID, const StringStreamOutCallback& stringStreamOutCallback) const override;
		// ~ILib

		CSignalPtr AddSignal(const SGUID& guid, const SGUID& senderGUID, tukk name);
		CSignalPtr GetSignal(const SGUID& guid);
		CLibAbstractInterfacePtr AddAbstractInterface(const SGUID& guid, tukk name);
		CLibAbstractInterfacePtr GetAbstractInterface(const SGUID& guid);
		CLibAbstractInterfaceFunctionPtr AddAbstractInterfaceFunction(const SGUID& guid, tukk name);
		CLibAbstractInterfaceFunctionPtr GetAbstractInterfaceFunction(const SGUID& guid);
		CLibClassPtr AddClass(const SGUID& guid, tukk name, const SGUID& foundationGUID, const IPropertiesConstPtr& pFoundationProperties);
		CLibClassPtr GetClass(const SGUID& guid);

	private:

		typedef std::unordered_map<SGUID, CSignalPtr>												TSignalMap;
		typedef std::unordered_map<SGUID, CLibAbstractInterfacePtr>					TAbstractInterfaceMap;
		typedef std::unordered_map<SGUID, CLibAbstractInterfaceFunctionPtr>	TAbstractInterfaceFunctionMap;
		typedef std::unordered_map<SGUID, CLibClassPtr>											TClassMap;

		string												m_name;
		TSignalMap										m_signals;
		TAbstractInterfaceMap					m_abstractInterfaces;
		TAbstractInterfaceFunctionMap	m_abstractInterfaceFunctions;
		TClassMap											m_classes;
	};

	DECLARE_SHARED_POINTERS(CLib)
}
