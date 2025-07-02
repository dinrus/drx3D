// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MFXFlowGraphEffect.cpp
//  Version:     v1.00
//  Created:     29/11/2006 by AlexL-Benito GR
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: IMFXEffect implementation
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MFXFlowGraphEffect.h>
#include <drx3D/Act/MaterialEffects.h>
#include <drx3D/Act/MaterialFGUpr.h>
#include <drx3D/Act/MaterialEffectsCVars.h>
#include <drx3D/Act/DinrusAction.h>

namespace
{
CMaterialFGUpr* GetFGUpr()
{
	CMaterialEffects* pMFX = static_cast<CMaterialEffects*>(CDrxAction::GetDrxAction()->GetIMaterialEffects());
	if (pMFX == 0)
		return 0;
	CMaterialFGUpr* pMFXFGMgr = pMFX->GetFGUpr();
	assert(pMFXFGMgr != 0);
	return pMFXFGMgr;
}
};

CMFXFlowGraphEffect::CMFXFlowGraphEffect()
	: CMFXEffectBase(eMFXPF_Flowgraph)
{
}

CMFXFlowGraphEffect::~CMFXFlowGraphEffect()
{
	CMaterialFGUpr* pMFXFGMgr = GetFGUpr();
	if (pMFXFGMgr && m_flowGraphParams.fgName.empty() == false)
		pMFXFGMgr->EndFGEffect(m_flowGraphParams.fgName);
}

void CMFXFlowGraphEffect::LoadParamsFromXml(const XmlNodeRef& paramsNode)
{
	// Xml data format
	/*
	   <FlowGraph name="..." maxdist="..." param1="..." param2="..." />
	 */

	m_flowGraphParams.fgName = paramsNode->getAttr("name");
	if (paramsNode->haveAttr("maxdist"))
	{
		paramsNode->getAttr("maxdist", m_flowGraphParams.maxdistSq);
		m_flowGraphParams.maxdistSq *= m_flowGraphParams.maxdistSq;
	}
	paramsNode->getAttr("param1", m_flowGraphParams.params[0]); //MFX custom param 1
	paramsNode->getAttr("param2", m_flowGraphParams.params[1]); //MFX custom param 2
	m_flowGraphParams.params[2] = 1.0f;                         //Intensity (set dynamically from game code)
	m_flowGraphParams.params[3] = 0.0f;                         //Blend out time
}

void CMFXFlowGraphEffect::Execute(const SMFXRunTimeEffectParams& params)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	if (CMaterialEffectsCVars::Get().mfx_EnableFGEffects == 0)
		return;

	const float distToCameraSq = (gEnv->pSystem->GetViewCamera().GetPosition() - params.pos).GetLengthSquared();

	//Check max distance
	if (m_flowGraphParams.maxdistSq == 0.f || distToCameraSq <= m_flowGraphParams.maxdistSq)
	{
		CMaterialFGUpr* pMFXFGMgr = GetFGUpr();
		pMFXFGMgr->StartFGEffect(m_flowGraphParams, sqrt_tpl(distToCameraSq));
		//if(pMFXFGMgr->StartFGEffect(m_flowGraphParams.fgName))
		//	DrxLogAlways("Starting FG HUD effect %s (player distance %f)", m_flowGraphParams.fgName.c_str(),distToPlayer);
	}
}

void CMFXFlowGraphEffect::SetCustomParameter(tukk customParameter, const SMFXCustomParamValue& customParameterValue)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	if (CMaterialEffectsCVars::Get().mfx_EnableFGEffects == 0)
		return;

	CMaterialFGUpr* pMFXFGMgr = GetFGUpr();
	pMFXFGMgr->SetFGCustomParameter(m_flowGraphParams, customParameter, customParameterValue);
}

void CMFXFlowGraphEffect::GetResources(SMFXResourceList& resourceList) const
{
	SMFXFlowGraphListNode* listNode = SMFXFlowGraphListNode::Create();
	listNode->m_flowGraphParams.name = m_flowGraphParams.fgName;

	SMFXFlowGraphListNode* next = resourceList.m_flowGraphList;

	if (!next)
		resourceList.m_flowGraphList = listNode;
	else
	{
		while (next->pNext)
			next = next->pNext;

		next->pNext = listNode;
	}
}

void CMFXFlowGraphEffect::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_flowGraphParams.fgName);
}
