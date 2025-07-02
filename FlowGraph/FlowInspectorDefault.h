// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowInspectorDefault.h
//  Version:     v1.00
//  Created:     1/12/2005 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: A FlowGraph inspector which draws information of all
//               flowgraphs as 2D overlay
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __FLOWINSPECTORDEFAULT_H__
#define __FLOWINSPECTORDEFAULT_H__

#include <drx3D/FlowGraph/IFlowSystem.h>

class CFlowInspectorDefault : public IFlowGraphInspector
{
public:
	CFlowInspectorDefault(IFlowSystem* pFlowSystem);
	virtual ~CFlowInspectorDefault();

	// IFlowGraphInspector interface
	virtual void AddRef();
	virtual void Release();
	virtual void PreUpdate(IFlowGraph* pGraph);    // called once per frame before IFlowSystem::Update
	virtual void PostUpdate(IFlowGraph* pGraph);   // called once per frame after  IFlowSystem::Update
	virtual void NotifyFlow(IFlowGraph* pGraph, const SFlowAddress from, const SFlowAddress to);
	virtual void NotifyProcessEvent(IFlowNode::EFlowEvent event, IFlowNode::SActivationInfo* pActInfo, IFlowNode* pImpl);
	virtual void AddFilter(IFlowGraphInspector::IFilterPtr pFilter);
	virtual void RemoveFilter(IFlowGraphInspector::IFilterPtr pFilter);
	virtual void GetMemoryUsage(IDrxSizer* s) const;
	// ~IFlowGraphInspector interface

	enum ERecType
	{
		eRTUnknown,
		eRTEntity,
		eRTAction,
		eRTLast
	};

	struct TFlowRecord
	{
		IFlowGraph*    m_pGraph;         // only valid during NotifyFlow, afterwards DON'T de-reference!
		SFlowAddress   m_from;
		SFlowAddress   m_to;
		TFlowInputData m_data;
		CTimeValue     m_tstamp;
		ERecType       m_type;
		string         m_message;        // complete message with graph, action, nodes, ports, value

		bool operator==(const TFlowRecord& lhs) const { const TFlowRecord& rhs = *this; return lhs.m_from == rhs.m_from && lhs.m_to == rhs.m_to && lhs.m_pGraph == rhs.m_pGraph; }
		bool operator!=(const TFlowRecord& rc) const  { return !(*this == rc); }

		void GetMemoryUsage(IDrxSizer* pSizer) const  {}
	};

protected:
	virtual void UpdateRecords();
	virtual void DrawRecords(const std::deque<TFlowRecord>& inRecords, i32 inBaseRow, float inMaxAge) const;
	void         DrawLabel(float col, float row, const ColorF& color, float glow, tukk szText, float fScale = 1.0f) const;
	// return true if flow should be recorded, false otherwise
	bool         RunFilters(IFlowGraph* pGraph, const SFlowAddress from, const SFlowAddress to);

	IRenderer*               m_pRenderer;
	IFlowSystem*             m_pFlowSystem;
	i32                      m_refs;
	i32                      m_newOneTime;
	i32                      m_newCont;
	std::vector<TFlowRecord> m_curRecords;
	std::deque<TFlowRecord>  m_oneTimeRecords; // all records which occurred once updated every frame
	std::deque<TFlowRecord>  m_contRecords;    // records which already occurred are put into here
	IFilter_AutoArray        m_filters;
	CTimeValue               m_currentTime;
	bool                     m_bProcessing;
	bool                     m_bPaused;
};

#endif
