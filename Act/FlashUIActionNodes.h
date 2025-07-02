// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlashUIActionNodes.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FlashUIActionNodes_H__
#define __FlashUIActionNodes_H__

#include <drx3D/Sys/IFlashUI.h>
#include "FlashUIBaseNode.h"

//--------------------------------------------------------------------------------------------
class CFlashUIActionBaseNode : public CFlowBaseNode<eNCT_Instanced>, public IUIActionListener
{
public:
	CFlashUIActionBaseNode(tukk name);
	virtual ~CFlashUIActionBaseNode();

	virtual void         GetConfiguration(SFlowNodeConfig& config) = 0;
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo) = 0;
	virtual void         GetMemoryUsage(IDrxSizer* s) const { s->Add(*this); }
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) = 0;

	virtual void         OnStart(IUIAction* pAction, const SUIArguments& args) {};
	virtual void         OnEnd(IUIAction* pAction, const SUIArguments& args)   {};

protected:
	void UpdateAction(tukk sName, bool bStrict);
	void UpdateAction(IFlowGraph* pGraph);

protected:
	IUIAction* m_pAction;
};

//--------------------------------------------------------------------------------------------
class CFlashUIStartActionNode : public CFlashUIActionBaseNode
{
public:
	CFlashUIStartActionNode(SActivationInfo* pActInfo) : CFlashUIActionBaseNode("CFlashUIStartActionNode") {};
	~CFlashUIStartActionNode();

	virtual void         GetConfiguration(SFlowNodeConfig& config);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) { return new CFlashUIStartActionNode(pActInfo); }

	virtual void         OnStart(IUIAction* pAction, const SUIArguments& args);

private:
	void FlushNextAction(SActivationInfo* pActInfo);

private:
	enum InputPorts
	{
		eI_UseAsState = 0,
	};

	enum OutputPorts
	{
		eO_OnActionStart = 0,
		eO_Args,
	};

	typedef CUIStack<std::pair<IUIAction*, SUIArguments>> TActionStack;
	TActionStack m_stack;
};

//--------------------------------------------------------------------------------------------
class CFlashUIEndActionNode : public CFlashUIActionBaseNode
{
public:
	CFlashUIEndActionNode(SActivationInfo* pActInfo) : CFlashUIActionBaseNode("CFlashUIEndActionNode") {}

	virtual void         GetConfiguration(SFlowNodeConfig& config);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual void         GetMemoryUsage(IDrxSizer* s) const { s->Add(*this); }
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)   { return new CFlashUIEndActionNode(pActInfo); }

private:
	enum InputPorts
	{
		eI_OnActionEnd = 0,
		eI_UseAsState,
		eI_Args,
	};
};

//--------------------------------------------------------------------------------------------
class CFlashUIActionNode : public CFlashUIActionBaseNode
{
public:
	CFlashUIActionNode(SActivationInfo* pActInfo)
		: CFlashUIActionBaseNode("CFlashUIActionNode")
		, m_bWasStarted(false)
	{};
	~CFlashUIActionNode();

	virtual void         GetConfiguration(SFlowNodeConfig& config);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);
	virtual void         GetMemoryUsage(IDrxSizer* s) const { s->Add(*this); }
	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo)   { return new CFlashUIActionNode(pActInfo); }

	virtual void         OnStart(IUIAction* pAction, const SUIArguments& args);
	virtual void         OnEnd(IUIAction* pAction, const SUIArguments& args);

private:
	void FlushNextAction(SActivationInfo* pActInfo);

private:
	enum EInputs
	{
		eI_UIAction = 0,
		eI_Strict,
		eI_StartAction,
		eI_Args,
	};

	enum EOutputs
	{
		eO_UIOnStart = 0,
		eO_UIOnEnd,
		eO_UIOnStartAll,
		eO_UIOnEndAll,
		eO_Args,
	};

	bool m_bWasStarted;

	typedef CUIStack<std::pair<IUIAction*, SUIArguments>> TActionStack;
	TActionStack m_startStack;
	TActionStack m_endStack;
	TActionStack m_selfEndStack;
};

#endif
