// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MaterialFGUpr
//  Version:     v1.00
//  Created:     29/11/2006 by AlexL-Benito GR
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: This class manage all the FlowGraph HUD effects related to a
//								material FX.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MATERIALFGMANAGER_H__
#define __MATERIALFGMANAGER_H__

#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>
#include "MFXFlowGraphEffect.h"

class CMaterialFGUpr
{
public:
	CMaterialFGUpr();
	virtual ~CMaterialFGUpr();

	// load FlowGraphs from specified path
	bool LoadLibs(tukk path = "Libs/MaterialEffects/FlowGraphs");

	// reset (deactivate all FlowGraphs)
	void Reset(bool bCleanUp);

	// serialize
	void          Serialize(TSerialize ser);

	bool          StartFGEffect(const SMFXFlowGraphParams& fgParams, float curDistance);
	bool          EndFGEffect(const string& fgName);
	bool          EndFGEffect(IFlowGraphPtr pFG);
	bool          IsFGEffectRunning(const string& fgName);
	void          SetFGCustomParameter(const SMFXFlowGraphParams& fgParams, tukk customParameter, const SMFXCustomParamValue& customParameterValue);

	void          ReloadFlowGraphs();

	size_t        GetFlowGraphCount() const;
	IFlowGraphPtr GetFlowGraph(i32 index, string* pFileName = NULL) const;
	bool          LoadFG(const string& filename, IFlowGraphPtr* pGraphRet = NULL);

	void          GetMemoryUsage(IDrxSizer* s) const;

	void          PreLoad();

protected:
	struct SFlowGraphData
	{
		SFlowGraphData()
		{
			m_startNodeId = InvalidFlowNodeId;
			m_endNodeId = InvalidFlowNodeId;
			m_bRunning = false;
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(m_name);
		}
		string        m_name;
		string        m_fileName;
		IFlowGraphPtr m_pFlowGraph;
		TFlowNodeId   m_startNodeId;
		TFlowNodeId   m_endNodeId;
		bool          m_bRunning;
	};

	SFlowGraphData* FindFG(const string& fgName);
	SFlowGraphData* FindFG(IFlowGraphPtr pFG);
	bool            InternalEndFGEffect(SFlowGraphData* pFGData, bool bInitialize);

protected:

	typedef std::vector<SFlowGraphData> TFlowGraphData;
	TFlowGraphData m_flowGraphVector;     //List of FlowGraph Effects
};

#endif
