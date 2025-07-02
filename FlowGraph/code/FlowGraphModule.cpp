// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxDB.cpp : Defines the exported functions for the DLL application.
//

#include <drx3D/FlowGraph/StdAfx.h>

// Included only once here
//#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/FlowGraph/FlowSystem.h>
#include <drx3D/FlowGraph/ModuleUpr.h>
#include <drx3D/FlowGraph/GameTokenSystem.h>



// Drx::IDefaultModule -----------------------------------

class CEngineModule_FlowGraph : public IFlowSystemEngineModule
{
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(IFlowSystemEngineModule)
		DRXINTERFACE_ADD(Drx::IDefaultModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_FlowGraph, "EngineModule_FlowGraph", "8d22d250-cbf2-4dba-adcc-a656c06752d7"_drx_guid)

	CEngineModule_FlowGraph();
	~CEngineModule_FlowGraph();

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override { return "DinrusFlowGraph"; };
	virtual tukk GetCategory() const override { return "drx3D"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize( SSysGlobEnv &env,const SSysInitParams &initParams ) override
	{
		ISystem* pSystem = env.pSystem;
		
		CFlowSystem* pFlowSystem = new CFlowSystem();
		env.pFlowSystem = pFlowSystem;
		pFlowSystem->PreInit();
		pFlowSystem->Init();

		DrxRegisterFlowNodes();
		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_FlowGraph)

CEngineModule_FlowGraph::CEngineModule_FlowGraph()
{
}

CEngineModule_FlowGraph::~CEngineModule_FlowGraph()
{
	if (gEnv->pFlowSystem)
	{
		gEnv->pFlowSystem->Release();
		gEnv->pFlowSystem = nullptr;
	}
}

