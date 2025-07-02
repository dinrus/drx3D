// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   FlowCustomActionNodes.cpp
   $Id$
   Описание: All nodes related to the Custom Action flow graphs

   -------------------------------------------------------------------------
   История:
   - 30:8:2011   15:24 : Created by Dean Claassen

 *********************************************************************/

#pragma once

#include <drx3D/FlowGraph/IFlowBaseNode.h>
#include <drx3D/Act/ICustomActions.h>

// Forward declarations
struct ICustomAction;

//////////////////////////////////////////////////////////////////////////
// CustomAction:Control node.
// This node is used to control a custom action from a specific instance
//////////////////////////////////////////////////////////////////////////
class CFlowNode_CustomActionControl : public CFlowBaseNode<eNCT_Instanced>, ICustomActionListener
{
public:
	enum INPUTS
	{
		EIP_Start = 0,
		EIP_Succeed,
		EIP_SucceedWait,
		EIP_SucceedWaitComplete,
		EIP_Abort,
		EIP_EndSuccess,
		EIP_EndFailure,
	};

	enum OUTPUTS
	{
		EOP_Started = 0,
		EOP_Succeeded,
		EOP_SucceededWait,
		EOP_SucceededWaitComplete,
		EOP_Aborted,
		EOP_EndedSuccess,
		EOP_EndedFailure,
	};

	CFlowNode_CustomActionControl(SActivationInfo* pActInfo)
	{
	}

	~CFlowNode_CustomActionControl();

	virtual void GetMemoryUsage(IDrxSizer* s) const
	{
		s->Add(*this);
	}

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) { return new CFlowNode_CustomActionControl(pActInfo); }

	void                 GetConfiguration(SFlowNodeConfig& config);
	void                 ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo);

	// ICustomActionListener
	virtual void OnCustomActionEvent(ECustomActionEvent event, ICustomAction& customAction);
	// ~ICustomActionListener

private:
	SActivationInfo m_actInfo;
};
