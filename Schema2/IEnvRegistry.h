// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Move element interfaces to separate headers and forward declare?
// #SchematycTODO : Should all elements share a common interface?
// #SchematycTODO : Move utilities to a separate header?
// #SchematycTODO : Restrict IEnvRegistry access to read only and use IEnvRegistrar interface passed during refresh to register elements.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/IAbstractInterface.h>
#include <drx3D/Schema2/IActionFactory.h> // We can remove this include once we've moved utilities to a separate header!
#include <drx3D/Schema2/IComponentFactory.h> // We can remove this include once we've moved utilities to a separate header!
#include <drx3D/Schema2/INetworkSpawnParams.h>
#include <drx3D/Schema2/IProperties.h>
#include <drx3D/Schema2/Deprecated/Variant.h>
#include <drx3D/Schema2/EnvTypeId.h>

struct SRendParams;
struct SRenderingPassInfo;

namespace sxema2
{
	struct IAction;
	struct IActionFactory;
	struct IComponent;
	struct IComponentFactory;
	struct IEnvFunctionDescriptor;
	struct IEnvTypeDesc;
	struct IFoundation;
	struct IGlobalFunction;
	struct IObject;
	struct ISignal;
	
	DECLARE_SHARED_POINTERS(IActionFactory)
	DECLARE_SHARED_POINTERS(IComponentFactory)
	DECLARE_SHARED_POINTERS(IEnvFunctionDescriptor)
	DECLARE_SHARED_POINTERS(IEnvTypeDesc)
	DECLARE_SHARED_POINTERS(IFoundation)
	DECLARE_SHARED_POINTERS(IGlobalFunction)
	DECLARE_SHARED_POINTERS(ISignal)

	struct SComponentParams
	{
		inline SComponentParams(IObject& _object, const IProperties& _properties, INetworkSpawnParamsPtr _pNetworkSpawnParams = INetworkSpawnParamsPtr(), IComponent* _pParent = nullptr)
			: object(_object)
			, properties(_properties)
			, pNetworkSpawnParams(_pNetworkSpawnParams)
			, pParent(_pParent)
		{}

		IObject&               object;
		const IProperties&     properties;
		INetworkSpawnParamsPtr pNetworkSpawnParams;
		IComponent*            pParent;
	};

	struct IComponentPreviewProperties
	{
		virtual ~IComponentPreviewProperties() {}

		virtual void Serialize(Serialization::IArchive& archive) = 0;
	};

	DECLARE_SHARED_POINTERS(IComponentPreviewProperties)

	struct IComponent
	{
		virtual ~IComponent() {}

		virtual bool Init(const SComponentParams& params) = 0;
		virtual void Run(ESimulationMode simulationMode) = 0;
		virtual IComponentPreviewPropertiesPtr GetPreviewProperties() const { return IComponentPreviewPropertiesPtr(); } // #SchematycTODO : Make this pure virtual?
		virtual void Preview(const SRendParams& params, const SRenderingPassInfo& passInfo, const IComponentPreviewProperties& previewProperties) const {} // #SchematycTODO : Make this pure virtual?
		virtual void Shutdown() = 0;
		virtual INetworkSpawnParamsPtr GetNetworkSpawnParams() const = 0;
	};

	DECLARE_SHARED_POINTERS(IComponent)

	struct IComponentMemberFunction
	{
		virtual ~IComponentMemberFunction() {}

		virtual SGUID GetGUID() const = 0;
		virtual SGUID GetComponentGUID() const = 0;
		virtual void SetName(tukk szName) = 0;
		virtual tukk GetName() const = 0;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) = 0;
		virtual tukk GetFileName() const = 0;
		virtual void SetAuthor(tukk szAuthor) = 0;
		virtual tukk GetAuthor() const = 0;
		virtual void SetDescription(tukk szDescription) = 0;
		virtual tukk GetDescription() const = 0;
		virtual void SetWikiLink(tukk szWikiLink) = 0;
		virtual tukk GetWikiLink() const = 0;
		virtual bool BindInput(u32 paramIdx, tukk szName, tukk szDescription) = 0;
		virtual u32 GetInputCount() const = 0;
		virtual IAnyConstPtr GetInputValue(u32 inputIdx) const = 0;
		virtual tukk GetInputName(u32 inputIdx) const = 0;
		virtual tukk GetInputDescription(u32 inputIdx) const = 0;
		virtual TVariantConstArray GetVariantInputs() const = 0;
		virtual bool BindOutput(u32 paramIdx, tukk szName, tukk szDescription) = 0;
		virtual u32 GetOutputCount() const = 0;
		virtual IAnyConstPtr GetOutputValue(u32 outputIdx) const = 0;
		virtual tukk GetOutputName(u32 outputIdx) const = 0;
		virtual tukk GetOutputDescription(u32 outputIdx) const = 0;
		virtual TVariantConstArray GetVariantOutputs() const = 0;
		virtual void Call(IComponent& component, const TVariantConstArray& inputs, const TVariantArray& outputs) const = 0;

		template <typename TYPE> inline void BindInput(u32 paramIdx, tukk szName, tukk szDescription, const TYPE& value)
		{
			BindInput_Protected(paramIdx, szName, szDescription, MakeAny(value));
		}

		inline void BindInput(u32 paramIdx, tukk szName, tukk szDescription, tukk value)
		{
			BindInput_Protected(paramIdx, szName, szDescription, MakeAny(CPoolString(value)));
		}

	protected:

		virtual bool BindInput_Protected(u32 paramIdx, tukk szName, tukk szDescription, const IAny& value) = 0;
	};

	DECLARE_SHARED_POINTERS(IComponentMemberFunction)

	struct SActionParams
	{
		inline SActionParams(IObject& _object, IComponent* _pComponent, const IProperties& _properties)
			: object(_object)
			, pComponent(_pComponent)
			, properties(_properties)
		{}

		IObject&           object;
		IComponent*        pComponent;
		const IProperties& properties;
	};

	struct IAction
	{
		virtual ~IAction() {}

		virtual bool Init(const SActionParams& params) = 0;
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void Shutdown() = 0;
	};

	DECLARE_SHARED_POINTERS(IAction)

	struct IActionMemberFunction
	{
		virtual ~IActionMemberFunction() {}

		virtual SGUID GetGUID() const = 0;
		virtual SGUID GetActionGUID() const = 0;
		virtual void SetName(tukk szName) = 0;
		virtual tukk GetName() const = 0;
		virtual void SetNamespace(tukk szNamespace) = 0;
		virtual tukk GetNamespace() const = 0;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) = 0;
		virtual tukk GetFileName() const = 0;
		virtual void SetAuthor(tukk szAuthor) = 0;
		virtual tukk GetAuthor() const = 0;
		virtual void SetDescription(tukk szDescription) = 0;
		virtual tukk GetDescription() const = 0;
		virtual void SetWikiLink(tukk szWikiLink) = 0;
		virtual tukk GetWikiLink() const = 0;
		virtual bool BindInput(u32 paramIdx, tukk szName, tukk szDescription) = 0;
		virtual u32 GetInputCount() const = 0;
		virtual IAnyConstPtr GetInputValue(u32 inputIdx) const = 0;
		virtual tukk GetInputName(u32 inputIdx) const = 0;
		virtual tukk GetInputDescription(u32 inputIdx) const = 0;
		virtual TVariantConstArray GetVariantInputs() const = 0;
		virtual bool BindOutput(u32 paramIdx, tukk szName, tukk szDescription) = 0;
		virtual u32 GetOutputCount() const = 0;
		virtual IAnyConstPtr GetOutputValue(u32 outputIdx) const = 0;
		virtual tukk GetOutputName(u32 outputIdx) const = 0;
		virtual tukk GetOutputDescription(u32 outputIdx) const = 0;
		virtual TVariantConstArray GetVariantOutputs() const = 0;
		virtual void Call(IAction& action, const TVariantConstArray& inputs, const TVariantArray& outputs) const = 0;

		template <typename TYPE> inline void BindInput(u32 paramIdx, tukk szName, tukk szDescription, const TYPE& value)
		{
			BindInput_Protected(paramIdx, szName, szDescription, MakeAny(value));
		}

		inline void BindInput(u32 paramIdx, tukk szName, tukk szDescription, tukk value)
		{
			BindInput_Protected(paramIdx, szName, szDescription, MakeAny(CPoolString(value)));
		}

	protected:

		virtual bool BindInput_Protected(u32 paramIdx, tukk szName, tukk szDescription, const IAny& value) = 0;
	};

	DECLARE_SHARED_POINTERS(IActionMemberFunction)

	struct IEnvSettings
	{
		virtual ~IEnvSettings() {}

		virtual void Serialize(Serialization::IArchive& archive) = 0;
	};

	DECLARE_SHARED_POINTERS(IEnvSettings)

	typedef TemplateUtils::CDelegate<EVisitStatus (const IEnvTypeDesc&)>                       EnvTypeDescVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const ISignalConstPtr&)>                    EnvSignalVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IGlobalFunctionConstPtr&)>            EnvGlobalFunctionVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IEnvFunctionDescriptor&)>             EnvFunctionVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IFoundationConstPtr&)>                EnvFoundationVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IAbstractInterfaceConstPtr&)>         EnvAbstractInterfaceVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IAbstractInterfaceFunctionConstPtr&)> EnvAbstractInterfaceFunctionVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IComponentFactoryConstPtr&)>          EnvComponentFactoryVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IComponentMemberFunctionConstPtr&)>   EnvComponentMemberFunctionVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IActionFactoryConstPtr&)>             EnvActionFactoryVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IActionMemberFunctionConstPtr&)>      EnvActionMemberFunctionVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (tukk , const IEnvSettingsPtr&)>       EnvSettingsVisitor;

	struct IEnvRegistry
	{
		virtual ~IEnvRegistry() {}

		virtual bool RegisterTypeDesc(const IEnvTypeDescPtr& pTypeDesc) = 0;
		virtual const IEnvTypeDesc* GetTypeDesc(const EnvTypeId& id) const = 0;
		virtual const IEnvTypeDesc* GetTypeDesc(const SGUID& guid) const = 0;
		virtual void VisitTypeDescs(const EnvTypeDescVisitor& visitor) const = 0;
		virtual ISignalPtr CreateSignal(const SGUID& guid) = 0;
		virtual ISignalConstPtr GetSignal(const SGUID& guid) const = 0;
		virtual void VisitSignals(const EnvSignalVisitor& visitor) const = 0;
		virtual bool RegisterGlobalFunction(const IGlobalFunctionPtr& pFunction) = 0;
		virtual IGlobalFunctionConstPtr GetGlobalFunction(const SGUID& guid) const = 0;
		virtual void VisitGlobalFunctions(const EnvGlobalFunctionVisitor& visitor) const = 0;
		virtual void RegisterFunction(const IEnvFunctionDescriptorPtr& pFunctionDescriptor) = 0;
		virtual const IEnvFunctionDescriptor* GetFunction(const SGUID& guid) const = 0;
		virtual void VisitFunctions(const EnvFunctionVisitor& visitor) const = 0;
		virtual IFoundationPtr CreateFoundation(const SGUID& guid, tukk szName) = 0;
		virtual IFoundationConstPtr GetFoundation(const SGUID& guid) const = 0;
		virtual void VisitFoundations(const EnvFoundationVisitor& visitor, EVisitFlags flags = EVisitFlags::None) const = 0;
		virtual IAbstractInterfacePtr CreateAbstractInterface(const SGUID& guid) = 0;
		virtual bool RegisterAbstractInterface(const IAbstractInterfacePtr& pAbstractInterface) = 0;
		virtual IAbstractInterfaceConstPtr GetAbstractInterface(const SGUID& guid) const = 0;
		virtual void VisitAbstractInterfaces(const EnvAbstractInterfaceVisitor& visitor) const = 0;
		virtual IAbstractInterfaceFunctionConstPtr GetAbstractInterfaceFunction(const SGUID& guid) const = 0;
		virtual void VisitAbstractInterfaceFunctions(const EnvAbstractInterfaceFunctionVisitor& visitor) const = 0;
		virtual bool RegisterComponentFactory(const IComponentFactoryPtr& pComponentFactory) = 0;
		virtual IComponentFactoryConstPtr GetComponentFactory(const SGUID& guid) const = 0;
		virtual void VisitComponentFactories(const EnvComponentFactoryVisitor& visitor) const = 0;
		virtual bool RegisterComponentMemberFunction(const IComponentMemberFunctionPtr& pFunction) = 0;
		virtual IComponentMemberFunctionConstPtr GetComponentMemberFunction(const SGUID& guid) const = 0;
		virtual void VisitComponentMemberFunctions(const EnvComponentMemberFunctionVisitor& visitor) const = 0;
		virtual bool RegisterActionFactory(const IActionFactoryPtr& pActionFactory) = 0;
		virtual IActionFactoryConstPtr GetActionFactory(const SGUID& guid) const = 0;
		virtual void VisitActionFactories(const EnvActionFactoryVisitor& visitor) const = 0;
		virtual bool RegisterActionMemberFunction(const IActionMemberFunctionPtr& pFunction) = 0;
		virtual IActionMemberFunctionConstPtr GetActionMemberFunction(const SGUID& guid) const = 0;
		virtual void VisitActionMemberFunctions(const EnvActionMemberFunctionVisitor& visitor) const = 0;

		virtual void BlacklistElement(const SGUID& guid) = 0;
		virtual bool IsBlacklistedElement(const SGUID& guid) const = 0;

		virtual bool RegisterSettings(tukk szName, const IEnvSettingsPtr& pSettings) = 0;
		virtual IEnvSettingsPtr GetSettings(tukk szName) const = 0;
		virtual void VisitSettings(const EnvSettingsVisitor& visitor) const = 0;
		virtual void LoadAllSettings() = 0;
		virtual void SaveAllSettings() = 0;
		
		virtual void Validate() const = 0;
	};

	namespace EnvRegistryUtils // #SchematycTODO : Rename EnvUtils?
	{
		//////////////////////////////////////////////////////////////////////////
		// #SchematycTODO : Validate inputs?
		inline void GetFullName(tukk szName, tukk szNamespace, stack_string& output)
		{
			if(szNamespace[0])
			{
				output.append(szNamespace);
				output.append("::");
			}
			output.append(szName);
		}

		//////////////////////////////////////////////////////////////////////////
		// #SchematycTODO : Validate inputs?
		// #SchematycTODO : Split this up into separate functions for abstract
		//                  interfaces, components, actions and so on.
		inline void GetFullName(tukk szName, tukk szNamespace, const SGUID& scopeGUID, stack_string& output)
		{
			if(scopeGUID)
			{
				IEnvRegistry&              envRegistry = gEnv->pSchematyc2->GetEnvRegistry();
				IAbstractInterfaceConstPtr pAbstractInterface = envRegistry.GetAbstractInterface(scopeGUID);
				if(pAbstractInterface)
				{
					output = pAbstractInterface->GetNamespace();
					if(!output.empty())
					{
						output.append("::");
					}
					output.append(pAbstractInterface->GetName());
				}
				else
				{
					IComponentFactoryConstPtr pComponentFactory = envRegistry.GetComponentFactory(scopeGUID);
					if(pComponentFactory)
					{
						output = pComponentFactory->GetNamespace();
						if(!output.empty())
						{
							output.append("::");
						}
						output.append(pComponentFactory->GetName());
					}
					else
					{
						IActionFactoryConstPtr pActionFactory = envRegistry.GetActionFactory(scopeGUID);
						if(pActionFactory)
						{
							GetFullName(pActionFactory->GetName(), pActionFactory->GetNamespace(), pActionFactory->GetComponentGUID(), output);
						}
					}
				}
				if(!output.empty())
				{
					output.append("::");
				}
				output.append(szName);
			}
			else
			{
				GetFullName(szName, szNamespace, output);
			}
		}
	}
}
