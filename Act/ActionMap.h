// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Action Map implementation. Maps Actions to Keys.

   -------------------------------------------------------------------------
   История:
   - 7:9:2004   17:47 : Created by Márcio Martins
   - 15:9:2010  12:30 : Revised by Dean Claassen

*************************************************************************/
#ifndef __ACTIONMAP_H__
#define __ACTIONMAP_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include "IActionMapUpr.h"

class CActionMapUpr;
class CActionMap;

typedef std::vector<SActionInput*> TActionInputs;

class CActionMapAction : public IActionMapAction
{
public:
	CActionMapAction();
	virtual ~CActionMapAction();

	// IActionMapAction
	virtual void                GetMemoryUsage(IDrxSizer* pSizer) const;
	virtual void                Release()                  { delete this; };
	virtual i32                 GetNumActionInputs() const { return m_actionInputs.size(); };
	virtual const SActionInput* FindActionInput(tukk szInput) const;
	virtual const SActionInput* GetActionInput(i32k iIndex) const;
	virtual const SActionInput* GetActionInput(const EActionInputDevice device, i32k iIndexByDevice) const;
	virtual const ActionId& GetActionId() const             { return m_actionId; }
	virtual tukk     GetTriggeredActionInput() const { return m_triggeredInput; };
	// IActionMapAction

	bool          AddActionInput(const SActionInput& actionInput, i32k iByDeviceIndex = -1);
	bool          RemoveActionInput(u32 inputCRC);
	void          RemoveAllActionInputs();
	SActionInput* AddAndGetActionInput(const SActionInput& actionInput);
	void          SetParentActionMap(CActionMap* pParentActionMap) { m_pParentActionMap = pParentActionMap; }
	SActionInput* FindActionInput(u32 inputCRC);
	SActionInput* GetActionInput(i32k iIndex);
	SActionInput* GetActionInput(const EActionInputDevice device, i32k iIndexByDevice);
	void          SetActionId(const ActionId& actionId)              { m_actionId = actionId; }
	void          SetNumRebindedInputs(i32k iNumRebindedInputs) { m_iNumRebindedInputs = iNumRebindedInputs; }
	i32           GetNumRebindedInputs() const                       { return m_iNumRebindedInputs; }

private:
	TActionInputString m_triggeredInput;
	ActionId           m_actionId;
	TActionInputs      m_actionInputs;
	CActionMap*        m_pParentActionMap;
	i32                m_iNumRebindedInputs;
};

class CActionMap :
	public IActionMap
{
public:
	CActionMap(CActionMapUpr* pActionMapUpr, tukk name);
	virtual ~CActionMap();

	// IActionMap
	virtual void                    GetMemoryUsage(IDrxSizer* pSizer) const override;
	virtual void                    Release() override;
	virtual void                    Clear() override;
	virtual const IActionMapAction* GetAction(const ActionId& actionId) const override;
	virtual IActionMapAction*       GetAction(const ActionId& actionId) override;
	virtual bool                    CreateAction(const ActionId& actionId) override;
	virtual bool                    RemoveAction(const ActionId& actionId) override;
	virtual i32                     GetActionsCount() const override { return m_actions.size(); };
	virtual bool                    AddActionInput(const ActionId& actionId, const SActionInput& actionInput, i32k iByDeviceIndex = -1) override;
	virtual bool                    AddAndBindActionInput(const ActionId& actionId, const SActionInput& actionInput) override;
	virtual bool                    RemoveActionInput(const ActionId& actionId, tukk szInput) override;
	virtual bool                    ReBindActionInput(const ActionId& actionId, tukk szCurrentInput, tukk szNewInput) override;
	virtual bool                    ReBindActionInput(const ActionId& actionId,
	                                                  tukk szNewInput,
	                                                  const EActionInputDevice device,
	                                                  i32k iByDeviceIndex) override;
	virtual i32                         GetNumRebindedInputs() override { return m_iNumRebindedInputs; }
	virtual bool                        Reset() override;
	virtual bool                        LoadFromXML(const XmlNodeRef& actionMapNode) override;
	virtual bool                        LoadRebindingDataFromXML(const XmlNodeRef& actionMapNode) override;
	virtual bool                        SaveRebindingDataToXML(XmlNodeRef& actionMapNode) const override;
	virtual IActionMapActionIteratorPtr CreateActionIterator() override;
	virtual void                        SetActionListener(EntityId id) override;
	virtual EntityId                    GetActionListener() const override;
	virtual tukk                 GetName() override       { return m_name.c_str(); }
	virtual void                        Enable(bool enable) override;
	virtual bool                        Enabled() const override { return m_enabled; };
	// ~IActionMap

	void EnumerateActions(IActionMapPopulateCallBack* pCallBack) const;
	bool CanProcessInput(const SInputEvent& inputEvent, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput);
	bool IsActionInputTriggered(const SInputEvent& inputEvent, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput) const;
	void InputProcessed();
	void ReleaseActionsIfActive();
	void ReleaseActionIfActive(const ActionId& actionId);
	void ReleaseFilteredActions();
	void AddExtraActionListener(IActionListener* pExtraActionListener);
	void RemoveExtraActionListener(IActionListener* pExtraActionListener);
	void NotifyExtraActionListeners(const ActionId& action, i32 currentState, float value);
	void AddFlowNodeActionListener(IActionListener* pExtraActionListener);
	void RemoveFlowNodeActionListener(IActionListener* pExtraActionListener);
	void NotifyFlowNodeActionListeners(const ActionId& action, i32 currentState, float value);

private:
	CActionMapAction* CreateAndGetAction(const ActionId& actionId);
	bool              AddAndBindActionInput(CActionMapAction* pAction, const SActionInput& actionInput);
	bool              ReBindActionInput(CActionMapAction* pAction, tukk szCurrentInput, tukk szNewInput);
	bool              ReBindActionInput(CActionMapAction* pAction, SActionInput& actionInput, tukk szNewInput);
	bool              ReBindActionInput(CActionMapAction* pAction,
	                                    tukk szNewInput,
	                                    const EActionInputDevice device,
	                                    i32k iByDeviceIndex);
	void                          ReleaseActionIfActiveInternal(CActionMapAction& action);
	EActionAnalogCompareOperation GetAnalogCompareOpTypeFromStr(tukk szTypeStr);
	tukk                   GetAnalogCompareOpStr(EActionAnalogCompareOperation compareOpType) const;
	void                          SetNumRebindedInputs(i32k iNumRebindedInputs) { m_iNumRebindedInputs = iNumRebindedInputs; }
	bool                          LoadActionInputAttributesFromXML(const XmlNodeRef& actionInputNode, SActionInput& actionInput);
	bool                          SaveActionInputAttributesToXML(XmlNodeRef& actionInputNode, const SActionInput& actionInput) const;
	void                          LoadActivationModeBitAttributeFromXML(const XmlNodeRef& attributeNode, i32& activationFlags, tukk szActivationModeName, EActionActivationMode activationMode);
	void                          SaveActivationModeBitAttributeToXML(XmlNodeRef& attributeNode, i32k activationFlags, tukk szActivationModeName, EActionActivationMode activationMode) const;

	typedef std::map<ActionId, CActionMapAction> TActionMap;
	typedef CListenerSet<IActionListener*>       TActionMapListeners;

	bool                m_enabled;
	CActionMapUpr*  m_pActionMapUpr;
	TActionMap          m_actions;
	EntityId            m_listenerId;
	TActionMapListeners m_actionMapListeners;
	TActionMapListeners m_flownodesListeners;
	string              m_name;
	i32                 m_iNumRebindedInputs;
};

#endif //__ACTIONMAP_H__
