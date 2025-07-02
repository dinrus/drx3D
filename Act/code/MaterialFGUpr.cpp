// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MaterialFGUpr
//  Version:     v1.00
//  Created:     29/11/2006 by AlexL-Benito GR
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MaterialFGUpr.h>
#include <drx3D/Act/MaterialEffectsCVars.h>

CMaterialFGUpr::CMaterialFGUpr()
{
}

CMaterialFGUpr::~CMaterialFGUpr()
{
	m_flowGraphVector.clear();
}

//------------------------------------------------------------------------
void CMaterialFGUpr::Reset(bool bCleanUp)
{
	for (i32 i = 0; i < m_flowGraphVector.size(); i++)
	{
		CMaterialFGUpr::SFlowGraphData& current = m_flowGraphVector[i];
		InternalEndFGEffect(&current, !bCleanUp);
	}

	if (bCleanUp)
	{
		stl::free_container(m_flowGraphVector);
	}
}

//------------------------------------------------------------------------
void CMaterialFGUpr::Serialize(TSerialize ser)
{
	for (i32 i = 0; i < m_flowGraphVector.size(); i++)
	{
		CMaterialFGUpr::SFlowGraphData& current = m_flowGraphVector[i];
		if (ser.BeginOptionalGroup(current.m_name, true))
		{
			ser.Value("run", current.m_bRunning);
			if (ser.BeginOptionalGroup("fg", true))
			{
				current.m_pFlowGraph->Serialize(ser);
				ser.EndGroup();
			}
			ser.EndGroup();
		}
	}
}

//------------------------------------------------------------------------
// CMaterialFGUpr::LoadLibs()
// Iterates through all the files in the folder and load the graphs
// PARAMS
// - path : Folder where the FlowGraph xml files are located
//------------------------------------------------------------------------
bool CMaterialFGUpr::LoadLibs(tukk path)
{
	LOADING_TIME_PROFILE_SECTION;
	if (gEnv->pFlowSystem == 0)
		return false;

	Reset(true);

	IDrxPak* pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;
	i32 numLoaded = 0;

	string realPath(path);
	realPath.TrimRight("/\\");
	string search(realPath);
	search += "/*.xml";

	intptr_t handle = pDrxPak->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			// fd.name contains the profile name
			string filename = realPath;
			filename += "/";
			filename += fd.name;
			bool ok = LoadFG(filename);
			if (ok)
				++numLoaded;
			SLICE_AND_SLEEP();
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}

	return numLoaded > 0;
}

//------------------------------------------------------------------------
// CMaterialFGUpr::LoadFG()
// Here is where the FlowGraph is loaded, storing a pointer to it, its name
// and also the IDs of the special "start" and "end" nodes
// PARAMS
// - filename: ...
//------------------------------------------------------------------------
bool CMaterialFGUpr::LoadFG(const string& filename, IFlowGraphPtr* pGraphRet /*= NULL*/)
{
	//Create FG from the XML file
	XmlNodeRef rootNode = gEnv->pSystem->LoadXmlFromFile(filename);
	if (rootNode == 0)
		return false;
	IFlowGraphPtr pFlowGraph = gEnv->pFlowSystem->CreateFlowGraph();
	if (pFlowGraph->SerializeXML(rootNode, true) == false)
	{
		// give warning
		GameWarning("MaterialFGUpr on LoadFG(%s)==> FlowGraph->SerializeXML failed", filename.c_str());
		return false;
	}

	//Deactivated it by default
	pFlowGraph->SetEnabled(false);
	pFlowGraph->SetType(IFlowGraph::eFGT_MaterialFx);

#ifndef _RELEASE
	stack_string debugName = "[Material FX] ";
	debugName.append(PathUtil::GetFileName(filename).c_str());
	pFlowGraph->SetDebugName(debugName);
#endif

	const TFlowNodeId nodeTypeId_StartFX = gEnv->pFlowSystem->GetTypeId("MaterialFX:HUDStartFX");
	const TFlowNodeId nodeTypeId_EndFX = gEnv->pFlowSystem->GetTypeId("MaterialFX:HUDEndFX");

	//Store needed data...
	SFlowGraphData fgData;
	fgData.m_pFlowGraph = pFlowGraph;

	// search for start and end nodes
	IFlowNodeIteratorPtr pNodeIter = pFlowGraph->CreateNodeIterator();
	TFlowNodeId nodeId;
	while (IFlowNodeData* pNodeData = pNodeIter->Next(nodeId))
	{
		if (pNodeData->GetNodeTypeId() == nodeTypeId_StartFX)
		{
			fgData.m_startNodeId = nodeId;
		}
		else if (pNodeData->GetNodeTypeId() == nodeTypeId_EndFX)
		{
			fgData.m_endNodeId = nodeId;
		}
	}

	if (fgData.m_startNodeId == InvalidFlowNodeId || fgData.m_endNodeId == InvalidFlowNodeId)
	{
		// warning no start/end node found
		GameWarning("MaterialFGUpr on LoadFG(%s) ==> No Start/End node found", filename.c_str());
		return false;
	}

	//Finally store the name
	fgData.m_name = PathUtil::GetFileName(filename);
	fgData.m_fileName = filename;
	PathUtil::RemoveExtension(fgData.m_name);
	m_flowGraphVector.push_back(fgData);

	// send initialize event to allow for resource caching in game
	if (gEnv->pDrxPak->GetLvlResStatus() && !gEnv->IsEditor())
	{
		SFlowGraphData* pFGData = FindFG(pFlowGraph);
		if (pFGData)
			InternalEndFGEffect(pFGData, true);
	}
	if (pGraphRet)
		*pGraphRet = m_flowGraphVector.rbegin()->m_pFlowGraph;
	return true;
}

//------------------------------------------------------------------------
// CMaterialFGUpr::FindFG(const string& fgName)
// Find a FlowGraph by name
// PARAMS
// - fgName: Name of the FlowGraph
//------------------------------------------------------------------------
CMaterialFGUpr::SFlowGraphData* CMaterialFGUpr::FindFG(const string& fgName)
{
	std::vector<SFlowGraphData>::iterator iter = m_flowGraphVector.begin();
	std::vector<SFlowGraphData>::iterator iterEnd = m_flowGraphVector.end();
	while (iter != iterEnd)
	{
		if (fgName.compareNoCase(iter->m_name) == 0)
			return &*iter;
		++iter;
	}
	return 0;
}

//------------------------------------------------------------------------
// CMaterialFGUpr::FindFG(IFlowGraphPtr pFG)
// Find a FlowGraph "by pointer"
// PARAMS
// - pFG: FlowGraph pointer
//------------------------------------------------------------------------
CMaterialFGUpr::SFlowGraphData* CMaterialFGUpr::FindFG(IFlowGraphPtr pFG)
{
	std::vector<SFlowGraphData>::iterator iter = m_flowGraphVector.begin();
	std::vector<SFlowGraphData>::iterator iterEnd = m_flowGraphVector.end();
	while (iter != iterEnd)
	{
		if (iter->m_pFlowGraph == pFG)
			return &*iter;
		++iter;
	}
	return 0;
}

//------------------------------------------------------------------------
// CMaterialFGUpr::StartFGEffect(const string& fgName)
// Activate the MaterialFX:StartHUDEffect node, this way the FG will be executed
// PARAMS
// - fgName: Name of the flowgraph effect
//------------------------------------------------------------------------
bool CMaterialFGUpr::StartFGEffect(const SMFXFlowGraphParams& fgParams, float curDistance)
{
	SFlowGraphData* pFGData = FindFG(fgParams.fgName);
	if (pFGData == 0)
	{
		GameWarning("CMaterialFXUpr::StartFXEffect: Can't execute FG '%s'", fgParams.fgName.c_str());
	}
	else if (/*pFGData && */ !pFGData->m_bRunning)
	{
		IFlowGraphPtr pFG = pFGData->m_pFlowGraph;
		pFG->SetEnabled(true);
		pFG->InitializeValues();

		TFlowInputData data;

		// set distance output port [1]
		pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 1, true), curDistance);
		// set custom params [2] - [3]
		pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 2, true), fgParams.params[0]);
		pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 3, true), fgParams.params[1]);

		// set intensity (dynamically adjusted from game if needed) [4]
		pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 4, true), 1.0f);

		// set input port [0]
		pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 0, false), data);  // port0: InputPortConfig_Void ("Trigger")
		//data = fgName;
		//pFG->ActivatePort(SFlowAddress(pFGData->m_endNodeId, 0, false), data);		// port0: InputPortConfig<string> ("FGName")
		pFGData->m_bRunning = true;

		if (CMaterialEffectsCVars::Get().mfx_DebugFlowGraphFX != 0)
		{
			GameWarning("Material FlowGraphFX manager: Effect %s was triggered.", fgParams.fgName.c_str());
		}

		return true;
	}
	return false;
}

//------------------------------------------------------------------------
// CMaterialFGUpr::EndFGEffect(const string& fgName)
// This method will be automatically called when the effect finish
// PARAMS
// - fgName: Name of the FlowGraph
//------------------------------------------------------------------------
bool CMaterialFGUpr::EndFGEffect(const string& fgName)
{
	SFlowGraphData* pFGData = FindFG(fgName);
	if (pFGData)
		return InternalEndFGEffect(pFGData, false);
	return false;
}

//===========================================================
// CMaterialFGUpr::SetFGCustomParameter(const SMFXFlowGraphParams& fgParams, tukk customParameter, const SMFXCustomParamValue& customParameterValue)
// This method allow for setting some custom, and dynamically updated outputs from the game, to adjust the FX
// PARAMS
//  - fgParams: Flow graph parameters
//  - customParameter: Name of the custom parameter to adjust
//  - customParameterValue: Contains new value for customParameter
//------------------------------------------------------------------------
void CMaterialFGUpr::SetFGCustomParameter(const SMFXFlowGraphParams& fgParams, tukk customParameter, const SMFXCustomParamValue& customParameterValue)
{
	SFlowGraphData* pFGData = FindFG(fgParams.fgName);
	if (pFGData == 0)
	{
		GameWarning("CMaterialFXUpr::StartFXEffect: Can't execute FG '%s'", fgParams.fgName.c_str());
	}
	else if (pFGData->m_bRunning)
	{
		IFlowGraphPtr pFG = pFGData->m_pFlowGraph;

		if (stricmp(customParameter, "Intensity") == 0)
		{
			// set intensity (dynamically adjusted from game if needed) [4]
			pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 4, true), customParameterValue.fValue);
		}
		else if (stricmp(customParameter, "BlendOutTime") == 0)
		{
			//Activate blend out timer [5]
			pFG->ActivatePort(SFlowAddress(pFGData->m_startNodeId, 5, true), customParameterValue.fValue);
		}

		if (CMaterialEffectsCVars::Get().mfx_DebugFlowGraphFX != 0)
		{
			GameWarning("Material FlowGraphFX manager: Effect '%s' .Dynamic parameter '%s' set to value %.3f", fgParams.fgName.c_str(), customParameter, customParameterValue.fValue);
		}
	}
}
//------------------------------------------------------------------------
// CMaterialFGUpr::EndFGEffect(IFlowGraphPtr pFG)
// This method will be automatically called when the effect finish
// PARAMS
// - pFG: Pointer to the FlowGraph
//------------------------------------------------------------------------
bool CMaterialFGUpr::EndFGEffect(IFlowGraphPtr pFG)
{
	SFlowGraphData* pFGData = FindFG(pFG);
	if (pFGData)
		return InternalEndFGEffect(pFGData, false);
	return false;
}

//------------------------------------------------------------------------
// internal method to end effect. can also initialize values
//------------------------------------------------------------------------
bool CMaterialFGUpr::InternalEndFGEffect(SFlowGraphData* pFGData, bool bInitialize)
{
	if (pFGData == 0)
		return false;
	if (pFGData->m_bRunning || bInitialize)
	{
		IFlowGraphPtr pFG = pFGData->m_pFlowGraph;
		if (bInitialize)
		{
			if (pFG->IsEnabled() == false)
			{
				pFG->SetEnabled(true);
				pFG->InitializeValues();
			}
		}
		pFG->SetEnabled(false);
		pFGData->m_bRunning = false;
		return true;
	}
	return false;
}

void CMaterialFGUpr::PreLoad()
{
#ifndef _RELEASE

	// need to preload the missing FX for CScriptBind_Entity::PreLoadParticleEffect
	//	gEnv->pParticleUpr->FindEffect( "MissingFGEffect", "MissingFGEffect" );

#endif

	TFlowGraphData::iterator iBegin = m_flowGraphVector.begin();
	TFlowGraphData::const_iterator iEnd = m_flowGraphVector.end();
	for (TFlowGraphData::iterator i = iBegin; i != iEnd; ++i)
	{
		i->m_pFlowGraph->PrecacheResources();
	}
}

//------------------------------------------------------------------------
// CMaterialFGUpr::EndFGEffect(IFlowGraphPtr pFG)
// This method will be automatically called when the effect finish
// PARAMS
// - pFG: Pointer to the FlowGraph
//------------------------------------------------------------------------
bool CMaterialFGUpr::IsFGEffectRunning(const string& fgName)
{
	SFlowGraphData* pFGData = FindFG(fgName);
	if (pFGData)
		return pFGData->m_bRunning;
	return false;
}

//------------------------------------------------------------------------
// CMaterialFGUpr::ReloadFlowGraphs
// Reload the FlowGraphs (invoked through console command, see MaterialEffectsCVars.cpp)
//------------------------------------------------------------------------
void CMaterialFGUpr::ReloadFlowGraphs()
{
	LoadLibs();
}

void CMaterialFGUpr::GetMemoryUsage(IDrxSizer* s) const
{
	SIZER_SUBCOMPONENT_NAME(s, "flowgraphs");
	s->AddObject(this, sizeof(*this));
	s->AddObject(m_flowGraphVector);
}

size_t CMaterialFGUpr::GetFlowGraphCount() const
{
	return m_flowGraphVector.size();
}

IFlowGraphPtr CMaterialFGUpr::GetFlowGraph(i32 index, string* pFileName /*= NULL*/) const
{
	if (index >= 0 && index < GetFlowGraphCount())
	{
		const SFlowGraphData& fgData = m_flowGraphVector[index];
		if (pFileName)
			*pFileName = fgData.m_fileName;
		return fgData.m_pFlowGraph;
	}
	return NULL;
}
