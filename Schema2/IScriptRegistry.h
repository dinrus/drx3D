// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_Delegate.h>
#include <drx3D/Schema2/TemplateUtils_Signalv2.h>

#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/GUID.h>
#include <drx3D/Schema2/IScriptFile.h>

namespace sxema2
{
	struct IScriptClass;
	struct IScriptEnumeration;
	struct IScriptFunction;
	struct IScriptModule;

	typedef TemplateUtils::CDelegate<EVisitStatus (IScriptFile&)>          ScriptFileVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IScriptFile&)>    ScriptFileConstVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (IScriptElement&)>       ScriptElementVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IScriptElement&)> ScriptElementConstVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (INewScriptFile&)>       NewScriptFileVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const INewScriptFile&)> NewScriptFileConstVisitor;

	enum class EScriptRegistryChange
	{
		ElementAdded,
		ElementRemoved
	};

	typedef TemplateUtils::CSignalv2<void (EScriptRegistryChange, IScriptElement*)> ScriptRegistryChangeSignal;

	struct SScriptRegistrySignals
	{
		ScriptRegistryChangeSignal change;
	};

	struct IScriptRegistry
	{
		virtual ~IScriptRegistry() {}

		// Compatibility interface.
		//////////////////////////////////////////////////
		virtual IScriptFile* LoadFile(tukk szFileName) = 0;
		virtual IScriptFile* CreateFile(tukk szFileName, EScriptFileFlags flags = EScriptFileFlags::None) = 0;
		virtual IScriptFile* GetFile(const SGUID& guid) = 0;
		virtual IScriptFile* GetFile(tukk szFileName) = 0;
		virtual void VisitFiles(const ScriptFileVisitor& visitor, tukk szFilePath = nullptr) = 0;
		virtual void VisitFiles(const ScriptFileConstVisitor& visitor, tukk szFilePath = nullptr) const = 0;
		virtual void RefreshFiles(const SScriptRefreshParams& params) = 0;
		virtual bool Load() = 0;
		virtual void Save(bool bAlwaysSave = false) = 0;

		// New interface.
		//////////////////////////////////////////////////
		virtual IScriptModule* AddModule(tukk szName, IScriptElement* pScope = nullptr) = 0;
		virtual IScriptEnumeration* AddEnumeration(tukk szName, IScriptElement* pScope = nullptr) = 0;
		virtual IScriptFunction* AddFunction(tukk szName, IScriptElement* pScope = nullptr) = 0;
		virtual IScriptClass* AddClass(tukk szName, const SGUID& foundationGUID, IScriptElement* pScope = nullptr) = 0;
		virtual IScriptElement* GetElement(const SGUID& guid) = 0;
		virtual void RemoveElement(const SGUID& guid) = 0;
		virtual EVisitStatus VisitElements(const ScriptElementVisitor& visitor, IScriptElement* pScope = nullptr, EVisitFlags flags = EVisitFlags::None) = 0;
		virtual EVisitStatus VisitElements(const ScriptElementConstVisitor& visitor, const IScriptElement* pScope = nullptr, EVisitFlags flags = EVisitFlags::None) const = 0;
		virtual bool IsElementNameUnique(tukk szName, IScriptElement* pScope = nullptr) const = 0; // #SchematycTODO : Should we also validate the name here?
		virtual SScriptRegistrySignals& Signals() = 0;
	};
}
