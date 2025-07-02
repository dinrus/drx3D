// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvPackage.h>
#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/IEnvRegistry.h>

namespace sxema
{
// Forward declare interfaces.
struct IEnvAction;
struct IEnvClass;
struct IEnvComponent;
struct IEnvDataType;
struct IEnvElement;
struct IEnvFunction;
struct IEnvInterface;
struct IEnvInterfaceFunction;
struct IEnvModule;
struct IEnvSignal;

// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(IEnvAction)
DECLARE_SHARED_POINTERS(IEnvClass)
DECLARE_SHARED_POINTERS(IEnvComponent)
DECLARE_SHARED_POINTERS(IEnvDataType)
DECLARE_SHARED_POINTERS(IEnvElement)
DECLARE_SHARED_POINTERS(IEnvFunction)
DECLARE_SHARED_POINTERS(IEnvInterface)
DECLARE_SHARED_POINTERS(IEnvInterfaceFunction)
DECLARE_SHARED_POINTERS(IEnvModule)
DECLARE_SHARED_POINTERS(IEnvSignal)

struct SEnvPackageElement
{
	inline SEnvPackageElement(const DrxGUID& _elementGUID, const IEnvElementPtr& _pElement, const DrxGUID& _scopeGUID)
		: elementGUID(_elementGUID)
		, pElement(_pElement)
		, scopeGUID(_scopeGUID)
	{}

	DrxGUID          elementGUID;
	IEnvElementPtr   pElement;
	DrxGUID          scopeGUID;
};

typedef std::vector<SEnvPackageElement> EnvPackageElements;

struct IEnvRoot : public IEnvElementBase<EEnvElementType::Root>
{
	virtual ~IEnvRoot() {}
};

class CEnvRoot : public CEnvElementBase<IEnvRoot>
{
public:

	CEnvRoot();

	// IEnvElement
	virtual bool IsValidScope(IEnvElement& scope) const override;
	// ~IEnvElement
};

class CEnvRegistry : public IEnvRegistry
{
private:

	struct SPackageInfo
	{
		IEnvPackagePtr pPackage;
		EnvPackageElements elements;
	};

	typedef std::unordered_map<DrxGUID, SPackageInfo>                  Packages;
	typedef std::unordered_map<DrxGUID, IEnvElementPtr>                Elements;
	typedef std::unordered_map<DrxGUID, IEnvModulePtr>                 Modules;
	typedef std::unordered_map<DrxGUID, IEnvDataTypePtr>               DataTypes;
	typedef std::unordered_map<DrxGUID, IEnvSignalPtr>                 Signals;
	typedef std::unordered_map<DrxGUID, IEnvFunctionPtr>               Functions;
	typedef std::unordered_map<DrxGUID, IEnvClassPtr>                  Classes;
	typedef std::unordered_map<DrxGUID, IEnvInterfacePtr>              Interfaces;
	typedef std::unordered_map<DrxGUID, IEnvInterfaceFunctionConstPtr> InterfaceFunctions;
	typedef std::unordered_map<DrxGUID, IEnvComponentPtr>              Components;
	typedef std::unordered_map<DrxGUID, IEnvActionPtr>                 Actions;
	typedef std::unordered_set<DrxGUID>                                Blacklist;

public:

	// IEnvRegistry

	virtual bool                         RegisterPackage(IEnvPackagePtr&& pPackage) override;
	virtual void                         DeregisterPackage(const DrxGUID& guid) override;
	virtual const IEnvPackage*           GetPackage(const DrxGUID& guid) const override;
	virtual void                         VisitPackages(const EnvPackageConstVisitor& visitor) const override;

	virtual void                         RegisterListener(IEnvRegistryListener* pListener) override;
	virtual void                         UnregisterListener(IEnvRegistryListener* pListener) override;

	virtual const IEnvElement&           GetRoot() const override;
	virtual const IEnvElement*           GetElement(const DrxGUID& guid) const override;

	virtual const IEnvModule*            GetModule(const DrxGUID& guid) const override;
	virtual void                         VisitModules(const EnvModuleConstVisitor& visitor) const override;

	virtual const IEnvDataType*          GetDataType(const DrxGUID& guid) const override;
	virtual void                         VisitDataTypes(const EnvDataTypeConstVisitor& visitor) const override;

	virtual const IEnvSignal*            GetSignal(const DrxGUID& guid) const override;
	virtual void                         VisitSignals(const EnvSignalConstVisitor& visitor) const override;

	virtual const IEnvFunction*          GetFunction(const DrxGUID& guid) const override;
	virtual void                         VisitFunctions(const EnvFunctionConstVisitor& visitor) const override;

	virtual const IEnvClass*             GetClass(const DrxGUID& guid) const override;
	virtual void                         VisitClasses(const EnvClassConstVisitor& visitor) const override;

	virtual const IEnvInterface*         GetInterface(const DrxGUID& guid) const override;
	virtual void                         VisitInterfaces(const EnvInterfaceConstVisitor& visitor) const override;

	virtual const IEnvInterfaceFunction* GetInterfaceFunction(const DrxGUID& guid) const override;
	virtual void                         VisitInterfaceFunctions(const EnvInterfaceFunctionConstVisitor& visitor) const override;

	virtual const IEnvComponent*         GetComponent(const DrxGUID& guid) const override;
	virtual void                         VisitComponents(const EnvComponentConstVisitor& visitor) const override;

	virtual const IEnvAction*            GetAction(const DrxGUID& guid) const override;
	virtual void                         VisitActions(const EnvActionConstVisitor& visitor) const override;

	virtual void                         BlacklistElement(const DrxGUID& guid) override;
	virtual bool                         IsBlacklistedElement(const DrxGUID& guid) const override;

	// ~IEnvRegistry

	void Refresh();

private:

	bool         RegisterPackageElements(const EnvPackageElements& packageElements);
	void         ReleasePackageElements(const EnvPackageElements& packageElements);

	bool         ValidateComponentDependencies() const;

	IEnvElement* GetElement(const DrxGUID& guid);

private:

	TEnvRegistryListeners m_listeners;

	Packages           m_packages;

	CEnvRoot           m_root;
	Elements           m_elements;

	Modules            m_modules;
	DataTypes          m_dataTypes;
	Signals            m_signals;
	Functions          m_functions;
	Classes            m_classes;
	Interfaces         m_interfaces;
	InterfaceFunctions m_interfaceFunctions;
	Components         m_components;
	Actions            m_actions;

	Blacklist          m_blacklist;
};
} // sxema
