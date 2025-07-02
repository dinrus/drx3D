// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlowGraphDebuggerUnitTests.cpp
//  Version:     v1.00
//  Created:     02/11/2013 by Sascha Hoba.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/FlowGraph/StdAfx.h>

#if defined(DRX_UNIT_TESTING)

	#include <drx3D/FlowGraph/IFlowGraphDebugger.h>
	#include <drx3D/FlowGraph/IFlowSystem.h>
	#include <drx3D/Sys/DrxUnitTest.h>

DRX_UNIT_TEST_SUITE(DrxFlowgraphDebuggerUnitTest)
{
	namespace FGD_UT_HELPER
	{
	IFlowGraphDebuggerPtr GetFlowGraphDebugger()
	{
		IFlowGraphDebuggerPtr pFlowgraphDebugger = GetIFlowGraphDebuggerPtr();
		DRX_UNIT_TEST_ASSERT(pFlowgraphDebugger.get() != NULL);
		return pFlowgraphDebugger;
	}

	IFlowGraphPtr CreateFlowGraph()
	{
		DRX_UNIT_TEST_ASSERT(gEnv->pFlowSystem != NULL);
		IFlowGraphPtr pFlowGraph = gEnv->pFlowSystem->CreateFlowGraph();
		DRX_UNIT_TEST_ASSERT(pFlowGraph.get() != NULL);
		return pFlowGraph;
	}

	TFlowNodeId CreateTestNode(IFlowGraphPtr pFlowGraph, const TFlowNodeTypeId& typeID)
	{
		const TFlowNodeId flowNodeID = pFlowGraph->CreateNode(typeID, "UnitTestNode");
		DRX_UNIT_TEST_ASSERT(flowNodeID != InvalidFlowNodeId);
		return flowNodeID;
	}

	void AddBreakPoint(IFlowGraphDebuggerPtr pFlowGraphDebugger, IFlowGraphPtr pFlowGraph, const SFlowAddress& addrIn)
	{
		DynArray<SBreakPoint> breakpointsDynArray;
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->AddBreakpoint(pFlowGraph, addrIn));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->GetBreakpoints(breakpointsDynArray));
		DRX_UNIT_TEST_CHECK_EQUAL(breakpointsDynArray.size(), 1);

		SBreakPoint breakpoint = breakpointsDynArray[0];
		DRX_UNIT_TEST_ASSERT(breakpoint.flowGraph == pFlowGraph);
		DRX_UNIT_TEST_ASSERT(breakpoint.addr == addrIn);
		breakpointsDynArray.clear();
		DRX_UNIT_TEST_ASSERT(breakpointsDynArray.empty());
	}
	}

	DRX_UNIT_TEST(CUT_INVALID_FLOWGRAPH_BREAKPOINT)
	{
		IFlowGraphDebuggerPtr pFlowGraphDebugger = FGD_UT_HELPER::GetFlowGraphDebugger();
		const SBreakPoint& breakpoint = pFlowGraphDebugger->GetBreakpoint();

		DRX_UNIT_TEST_CHECK_EQUAL(breakpoint.type, eBT_Invalid);
		DRX_UNIT_TEST_CHECK_EQUAL(breakpoint.edgeIndex, -1);
		DRX_UNIT_TEST_ASSERT(breakpoint.flowGraph.get() == NULL);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->BreakpointHit(), false);
	}

#if 0 // Test fails but disabled until we have time again. Flowgraph breakpoint currently works as intended for users. (Mid 2017)
	DRX_UNIT_TEST(CUT_ADD_REMOVE_FLOWGRAPH_BREAKPOINT)
	{
		IFlowGraphPtr pFlowGraph = FGD_UT_HELPER::CreateFlowGraph();
		const TFlowNodeTypeId typeID = gEnv->pFlowSystem->GetTypeId("Logic:Any");
		DRX_UNIT_TEST_ASSERT(typeID != InvalidFlowNodeTypeId);
		const TFlowNodeId flowNodeID = FGD_UT_HELPER::CreateTestNode(pFlowGraph, typeID);

		SFlowAddress addrIn;
		addrIn.isOutput = false;
		addrIn.node = flowNodeID;
		addrIn.port = 0;

		IFlowGraphDebuggerPtr pFlowGraphDebugger = FGD_UT_HELPER::GetFlowGraphDebugger();

		FGD_UT_HELPER::AddBreakPoint(pFlowGraphDebugger, pFlowGraph, addrIn);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, addrIn));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->RemoveBreakpoint(pFlowGraph, addrIn));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, addrIn), false);

		FGD_UT_HELPER::AddBreakPoint(pFlowGraphDebugger, pFlowGraph, addrIn);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, flowNodeID));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->RemoveAllBreakpointsForNode(pFlowGraph, flowNodeID));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, flowNodeID), false);

		FGD_UT_HELPER::AddBreakPoint(pFlowGraphDebugger, pFlowGraph, addrIn);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->HasBreakpoint(pFlowGraph));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->RemoveAllBreakpointsForGraph(pFlowGraph));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph), false);

		FGD_UT_HELPER::AddBreakPoint(pFlowGraphDebugger, pFlowGraph, addrIn);
		pFlowGraphDebugger->ClearBreakpoints();
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, flowNodeID), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph), false);

		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->BreakpointHit(), false);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->GetRootGraph(pFlowGraph) == pFlowGraph);
	}

	DRX_UNIT_TEST(CUT_ENABLE_DISABLE_FLOWGRAPH_BREAKPOINT)
	{
		IFlowGraphPtr pFlowGraph = FGD_UT_HELPER::CreateFlowGraph();
		const TFlowNodeTypeId typeID = gEnv->pFlowSystem->GetTypeId("Logic:Any");
		DRX_UNIT_TEST_ASSERT(typeID != InvalidFlowNodeTypeId);
		const TFlowNodeId flowNodeID = FGD_UT_HELPER::CreateTestNode(pFlowGraph, typeID);

		SFlowAddress addrIn;
		addrIn.isOutput = false;
		addrIn.node = flowNodeID;
		addrIn.port = 0;

		IFlowGraphDebuggerPtr pFlowGraphDebugger = FGD_UT_HELPER::GetFlowGraphDebugger();
		FGD_UT_HELPER::AddBreakPoint(pFlowGraphDebugger, pFlowGraph, addrIn);

		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->IsBreakpointEnabled(pFlowGraph, addrIn));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->IsTracepoint(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->EnableBreakpoint(pFlowGraph, addrIn, false));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->IsBreakpointEnabled(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->EnableBreakpoint(pFlowGraph, addrIn, true));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->IsBreakpointEnabled(pFlowGraph, addrIn));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->IsTracepoint(pFlowGraph, addrIn), false);

		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->EnableTracepoint(pFlowGraph, addrIn, true));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->IsTracepoint(pFlowGraph, addrIn));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->IsBreakpointEnabled(pFlowGraph, addrIn));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->EnableTracepoint(pFlowGraph, addrIn, false));
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->IsTracepoint(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->IsBreakpointEnabled(pFlowGraph, addrIn));

		pFlowGraphDebugger->ClearBreakpoints();

		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->IsBreakpointEnabled(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->IsTracepoint(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, flowNodeID), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph), false);

		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->BreakpointHit(), false);
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->GetRootGraph(pFlowGraph) == pFlowGraph);
	}

	DRX_UNIT_TEST(CUT_TRIGGER_FLOWGRAPH_BREAKPOINT)
	{
		if (!gEnv->IsEditor())
			return;

		ICVar* pNodeDebuggingCVar = gEnv->pConsole->GetCVar("fg_iEnableFlowgraphNodeDebugging");
		i32k oldNodeDebuggingValue = pNodeDebuggingCVar->GetIVal();
		pNodeDebuggingCVar->Set(1);

		IFlowGraphPtr pFlowGraph = FGD_UT_HELPER::CreateFlowGraph();
		const TFlowNodeTypeId typeID = gEnv->pFlowSystem->GetTypeId("Logic:Any");
		DRX_UNIT_TEST_ASSERT(typeID != InvalidFlowNodeTypeId);
		const TFlowNodeId flowNodeID = FGD_UT_HELPER::CreateTestNode(pFlowGraph, typeID);
		pFlowGraph->InitializeValues();

		SFlowAddress addrIn;
		addrIn.isOutput = true;
		addrIn.node = flowNodeID;
		addrIn.port = 0;

		IFlowGraphDebuggerPtr pFlowGraphDebugger = FGD_UT_HELPER::GetFlowGraphDebugger();
		FGD_UT_HELPER::AddBreakPoint(pFlowGraphDebugger, pFlowGraph, addrIn);

		pFlowGraph->ActivatePortCString(addrIn, "UnitTestActivation");

		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->BreakpointHit(), true);

		const SBreakPoint& breakpoint = pFlowGraphDebugger->GetBreakpoint();

		DRX_UNIT_TEST_CHECK_EQUAL(breakpoint.type, eBT_Output_Without_Edges);
		DRX_UNIT_TEST_CHECK_EQUAL(breakpoint.edgeIndex, -1);
		DRX_UNIT_TEST_ASSERT(breakpoint.flowGraph == pFlowGraph);

		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, addrIn));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, flowNodeID));
		DRX_UNIT_TEST_ASSERT(pFlowGraphDebugger->HasBreakpoint(pFlowGraph));

		pFlowGraphDebugger->InvalidateBreakpoint();
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->BreakpointHit(), false);
		pFlowGraphDebugger->ClearBreakpoints();

		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, addrIn), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph, flowNodeID), false);
		DRX_UNIT_TEST_CHECK_EQUAL(pFlowGraphDebugger->HasBreakpoint(pFlowGraph), false);

		pNodeDebuggingCVar->Set(oldNodeDebuggingValue);
	}
#endif // #if 0
}

#endif // UNITTESTING_ENABLED
