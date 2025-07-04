// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowBaseXmlNode.cpp
//  Описание: Flowgraph nodes to read/write Xml files
// -------------------------------------------------------------------------
//  История:
//    - 8/16/08 : File created - Kevin Kirst
//    - 09/06/2011: Added to SDK - Sascha Hoba
////////////////////////////////////////////////////////////////////////////

#include <drx3D/FlowGraph/StdAfx.h>
#include <drx3D/FlowGraph/FlowBaseXmlNode.h>

CGraphDocUpr* CGraphDocUpr::m_instance = NULL;
CGraphDocUpr* GDM = NULL;

////////////////////////////////////////////////////
CFlowXmlNode_Base::CFlowXmlNode_Base(SActivationInfo* pActInfo) : m_initialized(false)
{

}

////////////////////////////////////////////////////
CFlowXmlNode_Base::~CFlowXmlNode_Base(void)
{
	if (GDM == NULL)
		return;

	if (m_actInfo.pGraph == NULL)
		return;

	GDM->DeleteXmlDocument(m_actInfo.pGraph);
}

////////////////////////////////////////////////////
void CFlowXmlNode_Base::ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
{
	m_actInfo = *pActInfo;
	if (eFE_Initialize == event && !m_initialized)
	{
		if (GDM == NULL)
		{
			CGraphDocUpr::Create();
		}

		m_initialized = true;
		PREFAST_SUPPRESS_WARNING(6011); // gets initialized above if NULL
		GDM->MakeXmlDocument(pActInfo->pGraph);
	}
	else if (eFE_Activate == event && IsPortActive(pActInfo, EIP_Execute))
	{
		const bool bResult = Execute(pActInfo);
		if (bResult) ActivateOutput(pActInfo, EOP_Success, true);
		else ActivateOutput(pActInfo, EOP_Fail, true);
		ActivateOutput(pActInfo, EOP_Done, bResult);
	}
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
CGraphDocUpr::CGraphDocUpr()
{

}

////////////////////////////////////////////////////
CGraphDocUpr::~CGraphDocUpr()
{

}

////////////////////////////////////////////////////
void CGraphDocUpr::Create()
{
	if (!m_instance)
	{
		m_instance = new CGraphDocUpr;
		GDM = m_instance;
	}
}

////////////////////////////////////////////////////
CGraphDocUpr* CGraphDocUpr::Get()
{
	return m_instance;
}

////////////////////////////////////////////////////
void CGraphDocUpr::MakeXmlDocument(IFlowGraph* pGraph)
{
	GraphDocMap::iterator graph = m_GraphDocMap.find(pGraph);
	if (graph == m_GraphDocMap.end())
	{
		SXmlDocument doc;
		doc.root = doc.active = NULL;
		doc.refCount = 1;
		m_GraphDocMap[pGraph] = doc;
	}
	else
	{
		++graph->second.refCount;
	}
}

////////////////////////////////////////////////////
void CGraphDocUpr::DeleteXmlDocument(IFlowGraph* pGraph)
{
	GraphDocMap::iterator graph = m_GraphDocMap.find(pGraph);
	if (graph != m_GraphDocMap.end() && --graph->second.refCount <= 0)
	{
		m_GraphDocMap.erase(graph);
	}
}

////////////////////////////////////////////////////
bool CGraphDocUpr::GetXmlDocument(IFlowGraph* pGraph, SXmlDocument** document)
{
	bool bResult = false;

	GraphDocMap::iterator graph = m_GraphDocMap.find(pGraph);
	if (document && graph != m_GraphDocMap.end())
	{
		*document = &(graph->second);
		bResult = true;
	}

	return bResult;
}
