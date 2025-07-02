// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptRegistry.h>

#include <drx3D/Schema/ScriptSerializers.h>

namespace sxema
{
// Forward declare interfaces.
struct IString;
// Forward declare classes.
class CScript;
class CScriptRoot;
// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(CScript)

class CScriptRegistry : public IScriptRegistry
{
private:
	friend class CScript;

	typedef std::unordered_map<DrxGUID, CScriptPtr>                 ScriptsByGuid;
	typedef std::unordered_map<u32 /* Crc32 hash */, CScriptPtr> ScriptsByFileName;
	typedef std::unordered_map<DrxGUID, IScriptElementPtr>          Elements; // #SchematycTODO : Would it make more sense to store by raw pointer here and make ownership exclusive to script file?
	typedef std::vector<SScriptRegistryChange>                      ChangeQueue;

	struct SSignals
	{
		ScriptRegistryChangeSignal change;
	};

public:

	CScriptRegistry();

	// IScriptRegistry

	// Compatibility interface.
	//////////////////////////////////////////////////

	virtual void ProcessEvent(const SScriptEvent& event) override;
	virtual bool Load() override;
	virtual void Save(bool bAlwaysSave = false) override;

	// New interface.
	//////////////////////////////////////////////////

	virtual bool                               IsValidScope(EScriptElementType elementType, IScriptElement* pScope) const override;
	virtual bool                               IsValidName(tukk szName, IScriptElement* pScope, tukk & szErrorMessage) const override;

	virtual IScriptModule*                     AddModule(tukk szName, tukk szFilePath) override;
	virtual IScriptEnum*                       AddEnum(tukk szName, IScriptElement* pScope) override;
	virtual IScriptStruct*                     AddStruct(tukk szName, IScriptElement* pScope) override;
	virtual IScriptSignal*                     AddSignal(tukk szName, IScriptElement* pScope) override;
	virtual IScriptConstructor*                AddConstructor(tukk szName, IScriptElement* pScope) override;
	virtual IScriptFunction*                   AddFunction(tukk szNddfuncme, IScriptElement* pScope) override;
	virtual IScriptInterface*                  AddInterface(tukk szName, IScriptElement* pScope) override;
	virtual IScriptInterfaceFunction*          AddInterfaceFunction(tukk szName, IScriptElement* pScope) override;
	virtual IScriptInterfaceTask*              AddInterfaceTask(tukk szName, IScriptElement* pScope) override;
	virtual IScriptClass*                      AddClass(tukk szName, const SElementId& baseId, tukk szFilePath) override;
	virtual IScriptBase*                       AddBase(const SElementId& baseId, IScriptElement* pScope) override;
	virtual IScriptStateMachine*               AddStateMachine(tukk szName, EScriptStateMachineLifetime lifetime, const DrxGUID& contextGUID, const DrxGUID& partnerGUID, IScriptElement* pScope) override;
	virtual IScriptState*                      AddState(tukk szName, IScriptElement* pScope) override;
	virtual IScriptVariable*                   AddVariable(tukk szName, const SElementId& typeId, const DrxGUID& baseGUID, IScriptElement* pScope) override;
	virtual IScriptTimer*                      AddTimer(tukk szName, IScriptElement* pScope) override;
	virtual IScriptSignalReceiver*             AddSignalReceiver(tukk szName, EScriptSignalReceiverType type, const DrxGUID& signalGUID, IScriptElement* pScope) override;
	virtual IScriptInterfaceImpl*              AddInterfaceImpl(EDomain domain, const DrxGUID& refGUID, IScriptElement* pScope) override;
	virtual IScriptComponentInstance*          AddComponentInstance(tukk szName, const DrxGUID& typeGUID, IScriptElement* pScope) override;
	virtual IScriptActionInstance*             AddActionInstance(tukk szName, const DrxGUID& actionGUID, const DrxGUID& contextGUID, IScriptElement* pScope) override;

	virtual void                               RemoveElement(const DrxGUID& guid) override;

	virtual IScriptElement&                    GetRootElement() override;
	virtual const IScriptElement&              GetRootElement() const override;
	virtual IScriptElement*                    GetElement(const DrxGUID& guid) override;
	virtual const IScriptElement*              GetElement(const DrxGUID& guid) const override;

	virtual bool                               CopyElementsToXml(XmlNodeRef& output, IScriptElement& scope) const override;
	virtual bool                               PasteElementsFromXml(const XmlNodeRef& input, IScriptElement* pScope) override;

	virtual bool                               SaveUndo(XmlNodeRef& output, IScriptElement& scope) const override;
	virtual IScriptElement*                    RestoreUndo(const XmlNodeRef& input, IScriptElement* pScope) override;

	virtual bool                               IsElementNameUnique(tukk szName, IScriptElement* pScope) const override;
	virtual void                               MakeElementNameUnique(IString& name, IScriptElement* pScope) const override;
	virtual void                               ElementModified(IScriptElement& element) override;

	virtual ScriptRegistryChangeSignal::Slots& GetChangeSignalSlots() override;

	virtual IScript*                           GetScriptByGuid(const DrxGUID& guid) const override;
	virtual IScript*                           GetScriptByFileName(tukk szFilePath) const override;

	virtual IScript*                           LoadScript(tukk szFilePath) override;
	virtual void                               SaveScript(IScript& script) override;

	virtual void                               OnScriptRenamed(IScript& script, tukk szFilePath) override;
	// ~IScriptRegistry

private:
	CScript* CreateScript(tukk szFilePath, const IScriptElementPtr& pRoot);
	CScript* GetScript(const DrxGUID& guid);

	void     ProcessInputBlocks(ScriptInputBlocks& inputBlocks, IScriptElement& scope, EScriptEventId eventId);
	void     AddElement(const IScriptElementPtr& pElement, IScriptElement& scope, tukk szFilePath = nullptr);
	void     RemoveElement(IScriptElement& element);
	void     SaveScript(CScript& script);

protected:
	void BeginChange();
	void EndChange();
	void ProcessChange(const SScriptRegistryChange& change);
	void ProcessChangeDependencies(EScriptRegistryChangeType changeType, const DrxGUID& elementGUID);           // #SchematycTODO : Should we be able to queue dependency changes?

	bool IsUniqueElementName(tukk szName, IScriptElement* pScope) const;

private:

	std::shared_ptr<CScriptRoot> m_pRoot;   // #SchematycTODO : Why can't we use std::unique_ptr?
	ScriptsByGuid                m_scriptsByGuid;
	ScriptsByFileName            m_scriptsByFileName;
	Elements                     m_elements;
	u32                       m_changeDepth;
	ChangeQueue                  m_changeQueue;
	SSignals                     m_signals;
};
} // sxema
