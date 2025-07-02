// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Action manager and dispatcher.

   -------------------------------------------------------------------------
   История:
   - 7:9:2004   17:36 : Created by Márcio Martins
   - 15:9:2010  12:30 : Revised by Dean Claassen

*************************************************************************/
#ifndef __ACTIONMAPMANAGER_H__
#define __ACTIONMAPMANAGER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IActionMapUpr.h"

class CActionMapAction;

typedef std::map<string, class CActionMap*>    TActionMapMap;
typedef std::map<string, class CActionFilter*> TActionFilterMap;
typedef std::map<string, XmlNodeRef>           TControllerLayouts;

//------------------------------------------------------------------------

class CConsoleActionListener : public IActionListener
{
public:
	void OnAction(const ActionId& actionId, i32 activationMode, float value)
	{
		IConsole* pCons = gEnv->pConsole;
		pCons->ExecuteString(actionId.c_str());
	}
};

class CActionMapUpr :
	public IActionMapUpr,
	public IInputEventListener
{
public:
	CActionMapUpr(IInput* pInput);

	void Release() { delete this; };

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& event);
	// ~IInputEventListener

	// IActionMapUpr
	virtual void                          Update();
	virtual void                          Reset();
	virtual void                          Clear();

	virtual bool                          InitActionMaps(tukk filename);
	virtual void                          SetLoadFromXMLPath(tukk szPath) { m_loadedXMLPath = szPath; };
	virtual tukk                   GetLoadFromXMLPath() const             { return m_loadedXMLPath; };
	virtual bool                          LoadFromXML(const XmlNodeRef& node);
	virtual bool                          LoadRebindDataFromXML(const XmlNodeRef& node);
	virtual bool                          SaveRebindDataToXML(XmlNodeRef& node);

	virtual bool                          AddExtraActionListener(IActionListener* pExtraActionListener, tukk actionMap = NULL);
	virtual bool                          RemoveExtraActionListener(IActionListener* pExtraActionListener, tukk actionMap = NULL);
	virtual const TActionListeners&       GetExtraActionListeners() const;

	virtual bool                          AddFlowgraphNodeActionListener(IActionListener* pExtraActionListener, tukk actionMap = NULL);
	virtual bool                          RemoveFlowgraphNodeActionListener(IActionListener* pExtraActionListener, tukk actionMap = NULL);

	virtual void                          AddAlwaysActionListener(TBlockingActionListener pActionListener); // TODO: Remove always action listeners and integrate into 1 prioritized type
	virtual void                          RemoveAlwaysActionListener(TBlockingActionListener pActionListener);
	virtual void                          RemoveAllAlwaysActionListeners();

	virtual IActionMap*                   CreateActionMap(tukk name);
	virtual bool                          RemoveActionMap(tukk name);
	virtual void                          RemoveAllActionMaps();
	virtual IActionMap*                   GetActionMap(tukk name);
	virtual const IActionMap*             GetActionMap(tukk name) const;
	virtual IActionFilter*                CreateActionFilter(tukk name, EActionFilterType type = eAFT_ActionFail);
	virtual IActionFilter*                GetActionFilter(tukk name);
	virtual IActionMapIteratorPtr         CreateActionMapIterator();
	virtual IActionFilterIteratorPtr      CreateActionFilterIterator();

	virtual void                          Enable(const bool enable, const bool resetStateOnDisable = false);
	virtual void                          EnableActionMap(tukk name, bool enable);
	virtual void                          EnableFilter(tukk name, bool enable);
	virtual bool                          IsFilterEnabled(tukk name);
	virtual void                          RemoveAllFilters();
	virtual void                          ReleaseFilteredActions();
	virtual void                          ClearStoredCurrentInputData();

	virtual bool                          ReBindActionInput(tukk actionMapName, const ActionId& actionId, tukk szCurrentInput, tukk szNewInput);

	virtual const SActionInput*           GetActionInput(tukk actionMapName, const ActionId& actionId, const EActionInputDevice device, i32k iByDeviceIndex) const;

	virtual i32                           GetVersion() const      { return m_version; }
	virtual void                          SetVersion(i32 version) { m_version = version; }
	virtual void                          EnumerateActions(IActionMapPopulateCallBack* pCallBack) const;
	virtual i32                           GetActionsCount() const;
	virtual i32                           GetActionMapsCount() const { return m_actionMaps.size(); }

	virtual bool                          AddInputDeviceMapping(const EActionInputDevice deviceType, tukk szDeviceTypeStr);
	virtual bool                          RemoveInputDeviceMapping(const EActionInputDevice deviceType);
	virtual void                          ClearInputDevicesMappings();
	virtual i32                           GetNumInputDeviceData() const;
	virtual const SActionInputDeviceData* GetInputDeviceDataByIndex(i32k iIndex);
	virtual const SActionInputDeviceData* GetInputDeviceDataByType(const EActionInputDevice deviceType);
	virtual const SActionInputDeviceData* GetInputDeviceDataByType(tukk szDeviceType);

	virtual void                          RemoveAllRefireData();
	virtual bool                          LoadControllerLayoutFile(tukk szLayoutKeyName);

	virtual void                          SetDefaultActionEntity(EntityId id, bool bUpdateAll = true);
	virtual EntityId                      GetDefaultActionEntity() const { return m_defaultEntityId; };

	virtual void                          RegisterActionMapEventListener(IActionMapEventListener* pActionMapEventListener);
	virtual void                          UnregisterActionMapEventListener(IActionMapEventListener* pActionMapEventListener);
	// ~IActionMapUpr

	void         BroadcastActionMapEvent(const SActionMapEvent& event);

	bool         ActionFiltered(const ActionId& action);

	void         RemoveActionFilter(CActionFilter* pActionFilter);

	void         ReleaseActionIfActive(const ActionId& actionId);

	bool         AddBind(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput);
	bool         RemoveBind(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput);
	void         RemoveBind(CActionMap* pActionMap);
	void         RemoveBind(CActionMapAction* pAction);
	bool         HasBind(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput) const;

	bool         UpdateRefireData(const SInputEvent& event, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput);
	bool         RemoveRefireData(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput);
	void         RemoveRefireData(CActionMap* pActionMap);
	void         RemoveRefireData(CActionMapAction* pAction);
	bool         SetRefireDataDelayedPressNeedsRelease(const SInputEvent& event, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput, const bool bDelayedPressNeedsRelease);
	void         RemoveAllDelayedPressRefireData();
	i32          GetHighestPressDelayPriority() const;

	static void  ReloadActionMaps(IConsoleCmdArgs* pArgs);

	void         GetMemoryStatistics(IDrxSizer* s);

	ILINE bool   IsCurrentlyRefiringInput() const                    { return m_bRefiringInputs; }
	ILINE bool   IsIncomingInputRepeated() const                     { return m_bIncomingInputRepeated; }
	ILINE EKeyId GetIncomingInputKeyID() const                       { return m_currentInputKeyID; }
	ILINE void   SetRepeatedInputHoldTriggerFired(const bool bFired) { m_bRepeatedInputHoldTriggerFired = bFired; }
	ILINE bool   IsRepeatedInputHoldTriggerFired() const             { return m_bRepeatedInputHoldTriggerFired; }

protected:
	virtual ~CActionMapUpr();

private:

	TActionListeners m_ExtraActionListeners;

	bool PreloadControllerLayout(const XmlNodeRef& controllerLayout);

	struct SBindData
	{
		SBindData(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
			: m_pActionMap(pActionMap)
			, m_pAction(pAction)
			, m_pActionInput(pActionInput)
		{
		}

		SActionInput*     m_pActionInput;
		CActionMapAction* m_pAction;
		CActionMap*       m_pActionMap;
	};

	struct SRefireBindData
	{
		SRefireBindData(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
			: m_bindData(pActionMap, pAction, pActionInput)
			, m_bIgnoreNextUpdate(false)
			, m_bDelayedPressNeedsRelease(false)
		{
		}

		SBindData m_bindData;
		bool      m_bIgnoreNextUpdate;         // Only used for refiring data since don't want to fire right after was added
		bool      m_bDelayedPressNeedsRelease;
	};

	typedef std::vector<SRefireBindData> TRefireBindData;
	struct SRefireData
	{
		SRefireData(const SInputEvent& event, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
			: m_inputCRC(pActionInput->inputCRC)
			, m_inputEvent(event)
		{
			m_refireBindData.push_back(SRefireBindData(pActionMap, pAction, pActionInput));
		}

		u32          m_inputCRC;
		SInputEvent     m_inputEvent;   // Copy of actual event
		TRefireBindData m_refireBindData;
	};

	typedef std::multimap<u32, SBindData>    TInputCRCToBind;
	typedef std::list<const SBindData*>         TBindPriorityList;
	typedef std::list<TBlockingActionListener>  TBlockingActionListeners;
	typedef std::vector<SActionInputDeviceData> TInputDeviceData;
	typedef std::map<u32, SRefireData*>      TInputCRCToRefireData;

	struct SRefireReleaseListData
	{
		SInputEvent       m_inputEvent;
		TBindPriorityList m_inputsList;
	};

	bool       HandleAcceptedEvents(const SInputEvent& event, TBindPriorityList& priorityList);
	void       HandleInputBlocking(const SInputEvent& event, const SActionInput* pActionInput, const float fCurrTime);
	SBindData* GetBindData(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput);
	bool       CreateEventPriorityList(const SInputEvent& inputEvent, TBindPriorityList& priorityList);
	bool       CreateRefiredEventPriorityList(SRefireData* pRefireData,
	                                          TBindPriorityList& priorityList,
	                                          TBindPriorityList& removeList,
	                                          TBindPriorityList& delayPressNeedsReleaseList);
	bool ProcessAlwaysListeners(const ActionId& action, i32 activationMode, float value, const SInputEvent& inputEvent);
	void SetCurrentlyRefiringInput(bool bRefiringInput) { m_bRefiringInputs = bRefiringInput; }
	void UpdateRefiringInputs();

	string                   m_loadedXMLPath;
	IInput*                  m_pInput;
	TActionMapMap            m_actionMaps;
	TActionFilterMap         m_actionFilters;
	TInputCRCToBind          m_inputCRCToBind;
	TInputCRCToRefireData    m_inputCRCToRefireData;
	TBlockingActionListeners m_alwaysActionListeners;
	TControllerLayouts       m_preloadedControllerLayouts;
	TInputDeviceData         m_inputDeviceData;
	EKeyId                   m_currentInputKeyID;  // Keep track to determine if is a repeated input
	i32                      m_version;

#ifndef _RELEASE
	i32 i_listActionMaps;                       // cvar
#endif

	bool m_enabled;
	bool m_bRefiringInputs;
	bool m_bDelayedRemoveAllRefiringData;               // This only gets set true while m_bRefiringInputs == true and something is disabling an action filter based on a refired input (Crashes if try to do this while iterating data)
	bool m_bIncomingInputRepeated;                      // Input currently incoming is a repeated input
	bool m_bRepeatedInputHoldTriggerFired;              // Input currently incoming already fired initial hold trigger

	typedef std::list<IActionMapEventListener*> TActionMapEventListeners;
	TActionMapEventListeners  m_actionMapEventListeners;
	EntityId                  m_defaultEntityId;

	static CActionMapUpr* s_pThis;
};

#endif //__ACTIONMAPMANAGER_H__
